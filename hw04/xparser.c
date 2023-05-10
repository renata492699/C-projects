#include "parser.h"
#include "vector.h"
#include "xparser.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/*****************************************************************************
 *  DATA STRUCTURES
 *****************************************************************************/

struct node *node_create(mchar *name, struct vector *attributes,
                         mchar *text, struct vector *children)
{
    assert(name != NULL);
    assert(text == NULL || children == NULL);

    struct node *node = malloc(sizeof(struct node));
    if (node != NULL) {
        *node = (struct node) {
            .name = name,
            .text = text,
            .attributes = attributes,
            .children = children
        };
    }

    return node;
}

struct attribute *attribute_create(mchar *key, mchar *value)
{
    struct attribute *attr = malloc(sizeof(struct attribute));
    if (attr != NULL) {
        *attr = (struct attribute) {
            .key = key,
            .value = value
        };
    }

    return attr;
}

void attribute_destroy(struct attribute *attribute)
{
    if (attribute == NULL) {
        return;
    }
    str_destroy(attribute->key);
    str_destroy(attribute->value);
    free(attribute);
}

void node_destroy(struct node *node)
{
    if (node == NULL) {
        return;
    }

    str_destroy(node->name);
    str_destroy(node->text);
    vec_destroy(node->attributes, DESTRUCTOR(attr_ptr_destroy));

    vec_destroy(node->children, DESTRUCTOR(node_ptr_destroy));

    free(node);
}

/*****************************************************************************
 *  MISCELLANEOUS
 *****************************************************************************/

static bool add_while(struct parsing_state *state, mchar **str,
                      predicate predicate)
{
    assert(state != NULL);
    assert(predicate != NULL);
    assert(str != NULL);
    assert(*str != NULL);

    while(predicate(peek_char(state))) {
        if (!str_add_char(str, next_char(state))) {
            return false;
        }
    }

    return true;
}

mchar *parse_string(struct parsing_state *state, predicate valid_chars)
{
    assert(state != NULL);
    assert(valid_chars != NULL);

    mchar *string = str_create("");
    if (string != NULL && add_while(state, &string, valid_chars)) {
        return string;
    }

    alloc_error(state, "string");
    free(string);

    return NULL;
}

mchar *parse_word(struct parsing_state *state, predicate start,
                  predicate rest, const char *error_message)
{
    assert(state != NULL);
    assert(start != NULL);
    assert(rest != NULL);
    assert(error_message != NULL);

    if (!start(peek_char(state))) {
        parsing_error(state, error_message);
        return NULL;
    }

    mchar *word = str_create(CHAR_TO_STR(next_char(state)));
    if (word != NULL && add_while(state, &word, rest)) {
        return word;
    }

    alloc_error(state, "word");
    free(word);

    return NULL;
}

void exp_ptr_destroy(char **expression)
{
    assert(expression != NULL);
    *expression = NULL;
}

/*****************************************************************************
 *  NAME
 *****************************************************************************/

static int name_start(int c)
{
    return c == '_' || c == ':' || isalpha(c);
}

static int name_rest(int c)
{
    return strchr("_:.-", c) != NULL || isdigit(c) || isalpha(c);
}

mchar *parse_name(struct parsing_state *state)
{
    assert(state != NULL);

    return parse_word(state, name_start, name_rest, "letters or _ or :");
}


/*****************************************************************************
 *  TEXT
 *****************************************************************************/

static bool end_of_text(int c)
{
    return c == EOF || c == '<' || c == '"';
}

static bool add_to_text(struct parsing_state *state, mchar **text,
                        bool *space, bool normalize)
{
    assert(state != NULL);
    assert(text != NULL);

    int c = peek_char(state);

    if (normalize) {
        if (isspace(c)) {
            *space = true;
            next_char(state);
            return true;
        }

        if (*space && !str_add_char(text, ' ')) {
            return false;
        }

        *space = false;
    }

    return str_add_char(text, next_char(state));
}

mchar *parse_text(struct parsing_state *state, bool normalize)
{
    assert(state != NULL);

    if (normalize) {
        read_spaces(state, 0);
    }

    mchar *text = str_create("");
    if (text == NULL) {
        alloc_error(state, "text");
        return NULL;
    }

    bool space = false;
    while (!end_of_text(peek_char(state))) {
        if (!add_to_text(state, &text, &space, normalize)) {
            alloc_error(state, "text");
            str_destroy(text);
            return NULL;
        }
    }

    return text;
}


/*****************************************************************************
 *  VALUE
 *****************************************************************************/

bool parse_equals(struct parsing_state *state)
{
    assert(state != NULL);

    return read_spaces(state, 0)
        && pattern_char(state, '=')
        && read_spaces(state, 0);
}

mchar *parse_value(struct parsing_state *state)
{
    assert(state != NULL);

    if (!pattern_char(state, '"')) {
        return NULL;
    }

    mchar *value = parse_text(state, false);
    if (value == NULL) {
        return NULL;
    }

    if (!pattern_char(state, '"')) {
        str_destroy(value);
        return NULL;
    }

    return value;
}


/*****************************************************************************
 *  ATTRIBUTES
 *****************************************************************************/

static bool parse_attribute(struct parsing_state *state,
                            mchar **key, mchar **value)
{
    assert(state != NULL);

    *key = parse_name(state);
    if (*key == NULL) {
        return false;
    }

    bool success = parse_equals(state);
    success = success && (*value = parse_value(state)) != NULL;

    if (!success) {
        str_destroy(*key);
    }

    return success;
}

void attr_ptr_destroy(struct attribute **attributes)
{
    assert(attributes != NULL);

    attribute_destroy(*attributes);
}

static bool end_of_attributes(struct parsing_state *state)
{
    assert(state != NULL);

    const int c = peek_char(state);
    return c == EOF || c == '/' || c == '>' || c == '<';
}

bool parse_attributes(struct parsing_state *state, struct vector *attributes)
{
    assert(state != NULL);
    assert(attributes != NULL);

    if (end_of_attributes(state)) {
        return true;
    }
    while (!end_of_attributes(state)) {
        const bool whitespace = read_spaces(state, 1);
        if (!whitespace) {
            return false;
        }
        if (end_of_attributes(state)) {
            return true;
        }
        mchar *key = NULL;
        mchar *value = NULL;
        if (!parse_attribute(state, &key, &value)) {
            return false;
        }
        struct attribute *attribute = attribute_create(key, value);
        vec_push_back(attributes, &attribute);
    }

    return true;
}


/*****************************************************************************
 *  XNODE
 *****************************************************************************/

static bool check_end_tag(struct parsing_state *state, const mchar *name)
{
    return pattern_str(state, "</")
        && read_spaces(state, 0)
        && pattern_str(state, name)
        && read_spaces(state, 0)
        && pattern_char(state, '>');
}

static struct node *parse_content(struct parsing_state *state, mchar *name,
                                  struct vector *attributes, bool is_empty)
{
    assert(state != NULL);

    mchar *text = NULL;
    struct vector *children = NULL;

    if (!is_empty) {
        bool is_text = next_char(state) != '<' || peek_char(state) == '/';
        return_char(state);

        if (is_text) {
            text = parse_text(state, true);
        } else {
            children = parse_xnodes(state);
        }
    }

    const bool parsed_content = is_empty || text != NULL || children != NULL;
    if (parsed_content && (is_empty || check_end_tag(state, name))) {
        struct node *node = node_create(name, attributes, text, children);
        if (node != NULL) {
            return node;
        }
    }

    str_destroy(text);
    vec_destroy(children, DESTRUCTOR(node_ptr_destroy));

    return NULL;
}

static bool parse_tag(struct parsing_state *state, mchar **name,
                      struct vector *attributes, bool *is_empty)
{
    if (!pattern_char(state, '<') || !read_spaces(state, 0)) {
        return false;
    }

    if ((*name = parse_name(state)) == NULL) {
        return false;
    }

    if (!parse_attributes(state, attributes)) {
        str_destroy(*name);
        return false;
    }

    read_spaces(state, 0);
    *is_empty = accept_char(state, '/');

    if (pattern_char(state, '>') && read_spaces(state, 0)) {
        return true;
    }

    str_destroy(*name);
    vec_destroy(attributes, DESTRUCTOR(attr_ptr_destroy));

    return false;
}

struct node *parse_xnode(struct parsing_state *state)
{
    read_spaces(state, 0);

    mchar *name = NULL;
    struct vector *attributes = vec_create(sizeof(struct attribute *));

    if (attributes == NULL) {
        return NULL;
    }
    bool is_empty = true;
    if (!parse_tag(state, &name, attributes, &is_empty)) {
        return NULL;
    }

    struct node *node = parse_content(state, name, attributes, is_empty);
    if (node != NULL) {
        read_spaces(state, 0);
        return node;
    }

    str_destroy(name);
    vec_destroy(attributes, DESTRUCTOR(attr_ptr_destroy));

    return NULL;
}

void node_ptr_destroy(struct node **node)
{
    assert(node != NULL);

    node_destroy(*node);
}

struct vector *parse_xnodes(struct parsing_state *state)
{
    struct vector *children = vec_create(sizeof(struct node *));
    if (children == NULL) {
        return NULL;
    }

    read_spaces(state, 0);

    while (accept_char(state, '<')) {
        const int next = peek_char(state);
        return_char(state);
        // If next character is '/', we know that this is an end tag,
        // so there are no children → return empty vector
        if (next == '/') {
            return children;
        }

        struct node *child = parse_xnode(state);
        if (child != NULL) {
            if (!vec_push_back(children, &child)) {
                alloc_error(state, "adding child node to vector of nodes");
                vec_destroy(children, DESTRUCTOR(node_ptr_destroy));
                return NULL;
            }
        }
    }
    read_spaces(state, 0);

    return children;
}


/*****************************************************************************
 *  XML
 *****************************************************************************/

struct node *parse_root(struct parsing_state *state)
{
    struct node *root = parse_xnode(state);
    if (root == NULL) {
        return NULL;
    }

    if (peek_char(state) == EOF) {
        return root;
    }

    parsing_error(state, "EOF");

    return NULL;
}

struct node *parse_xml(FILE *file)
{
    struct file_generator gen = { file };
    struct parsing_state state = parsing_state_init(&gen, file_fill);
    struct node *root = parse_root(&state);
    if (root == NULL) {
        print_error(&state, stderr);
    }
    return root;
}
