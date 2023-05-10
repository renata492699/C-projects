#ifndef XPARSER_H
#define XPARSER_H

#include "managed_string.h"
#include "parser.h"

#include <stdio.h>
#include <string.h>


/*****************************************************************************
 *  DATA STRUCTURES
 *****************************************************************************/

struct attribute {
    mchar *key;
    mchar *value;
};

struct node {
    mchar *name;
    struct vector *attributes;
    mchar *text;
    struct vector *children;
};

struct node *node_create(mchar *name, struct vector *attributes,
                         mchar *text, struct vector *children);

void node_destroy(struct node *node);

void attribute_destroy(struct attribute *attribute);

/*****************************************************************************
 *  MISCELLANEOUS
 *****************************************************************************/

typedef int (*predicate) (int);

mchar *parse_string(struct parsing_state *state, predicate valid_chars);

mchar *parse_word(struct parsing_state *state, predicate start,
                  predicate rest, const char *error_message);

void exp_ptr_destroy(char **expression);

/*****************************************************************************
 *  PARSING
 *****************************************************************************/

mchar *parse_name(struct parsing_state *state);

mchar *parse_text(struct parsing_state *state, bool normalize);

bool parse_equals(struct parsing_state *state);

mchar *parse_value(struct parsing_state *state);

bool parse_attributes(struct parsing_state *state, struct vector *attributes);

void attr_ptr_destroy(struct attribute **attributes);

struct node *parse_xnode(struct parsing_state *state);

void node_ptr_destroy(struct node **node);

struct vector *parse_xnodes(struct parsing_state *state);

struct node *parse_root(struct parsing_state *state);

struct node *parse_xml(FILE *file);

#endif /* XPARSER_H */
