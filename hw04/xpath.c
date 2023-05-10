#include "xpath.h"

bool cmp_key(struct attribute *attribute, mchar *key)
{
    return strcmp(attribute->key, key) == 0;
}

bool cmp_value(struct attribute *attribute, mchar *value)
{
    return strcmp(attribute->value, value) == 0;
}

bool get_node(char *expression)
{
    return (strchr(expression, '[') == NULL) &&
           (strchr(expression, ']') == NULL) &&
           (strchr(expression, '@') == NULL);
}

bool get_n_node(char *expression)
{
    return (strchr(expression, '[') != NULL) &&
           (strchr(expression, ']') != NULL) &&
           (strchr(expression, '@') == NULL);
}

bool get_attribute_node(char *expression)
{
    return (strchr(expression, '[') != NULL) &&
           (strchr(expression, ']') != NULL) &&
           (strchr(expression, '@') != NULL) &&
           (strchr(expression, '=') == NULL);
}

bool get_attribute_value_node(char *expression)
{
    return (strchr(expression, '[') != NULL) &&
           (strchr(expression, ']') != NULL) &&
           (strchr(expression, '@') != NULL) &&
           (strchr(expression, '=') != NULL);
}

int parse_exp_name(char *string, char *name)
{
    if (sscanf(string, "%[^[]s", name) != 1) {
        fprintf(stderr, "Could not parse the expression with name.\n");
        return -1;
    }
    return 0;
}

int parse_exp_index(char *string, int *index)
{
    if (sscanf(string, "%d", index) != 1) {
        fprintf(stderr, "Could not parse the expression with index.\n");
        return -1;
    }
    return 0;
}

int parse_exp_key(char *string, char *key)
{
    if (sscanf(string, "@%[^]]s", key) != 1) {
        fprintf(stderr, "Could not parse the expression with key.\n");
        return -1;
    }
    return 0;
}

int parse_exp_value(char *string, char *key, char *value)
{
    if (sscanf(string, "@%[^=]s", key) != 1) {
        fprintf(stderr, "Could not parse the expression with key.\n");
        return -1;
    }
    string += (int) (strchr(string, '"') - string) + 1;
    if (sscanf(string, "%[^\"]s", value) != 1) {
        fprintf(stderr, "Could not parse the expression with value.\n");
        return -1;
    }
    return 0;
}

int scan_name_index(char *string, char *name, int *index)
{
    if (parse_exp_name(string, name) != 0) {
        return -1;
    }
    string += (strchr(string, '[') - string) + 1;
    if (parse_exp_index(string, index) != 0) {
        return -1;
    }
    return 0;
}

int scan_name_key(char *string, char *name, char *key)
{
    if (parse_exp_name(string, name) != 0) {
        return -1;
    }
    string += (int) (strchr(string, '@') - string);
    if (parse_exp_key(string, key) != 0) {
        return -1;
    }
    return 0;
}

int scan_name_value(char *string, char *name, char *key, char *value)
{
    if (parse_exp_name(string, name) != 0) {
        return -1;
    }
    string += (int) (strchr(string, '@') - string);
    if (parse_exp_value(string, key, value) != 0) {
        return -1;
    }
    return 0;
}

bool match_key_value(struct vector *attributes, mchar *data,
                     bool(*cmp_function)(struct attribute*, mchar *data))
{
    size_t size_v = vec_size(attributes);
    for (size_t i = 0; i < size_v; i++) {
        struct attribute **attribute = vec_get(attributes, i);
        if (cmp_function(*attribute, data)) {
            return true;
        }
    }
    return false;
}

int match_expr(struct node *node, struct vector *expressions, size_t n)
{
    char **expr = vec_get(expressions, n);

    if (get_node(*expr)) {
        return (strcmp(node->name, *expr) == 0) ? 0 : -1;

    } else if (get_n_node(*expr)) {
        char name[100];
        int index;

        if (scan_name_index(*expr, name, &index) != 0) {
            return -2;
        }
        if (index <= 0) {
            fprintf(stderr, "Index can not be negative number or zero.\n");
            return -2;
        }
        return (strcmp(node->name, name) == 0) ? 0 : -1;

    } else if (get_attribute_node(*expr)) {
        char name[100];
        char key[100];

        if (scan_name_key(*expr, name, key) != 0) {
            return -2;
        }
        return ((strcmp(node->name, name) == 0) &&
                match_key_value(node->attributes,
                                key, cmp_key)) ? 0 : -1;

    } else if (get_attribute_value_node(*expr)) {
        char name[100];
        char key[100];
        char value[100];

        if (scan_name_value(*expr, name, key, value) != 0) {
            return -2;
        }
        return ((strcmp(name, node->name) == 0) &&
                match_key_value(node->attributes, key, cmp_key) &&
                match_key_value(node->attributes,
                                value, cmp_value)) ? 0 : -1;
    } else {
        return -1;
    }
}

struct node *return_node(struct vector **result)
{
    struct node *node;
    if (vec_empty(*result)) {
        return NULL;
    }
    if (vec_size(*result) == 1) {
        struct node **one_node = vec_get(*result, 0);
        node = *one_node;
        *one_node = NULL;
        vec_destroy(*result, DESTRUCTOR(node_ptr_destroy));
    } else {
        mchar *name = str_create("result");
        struct node *new_node =
                node_create(name, NULL, NULL, *result);
        node = new_node;
    }
    return node;
}

struct vector *filter_children(struct vector **children,
        struct vector *expressions, size_t n)
{
    size_t size_v = vec_size(*children);
    struct vector *result = vec_create(sizeof(struct node *));
    int count = 0;
    int index = 0;
    char **exp = vec_get(expressions, n);
    if (get_n_node(*exp)) {
        char name[100];
        if (scan_name_index(*exp, name, &index) != 0) {
            vec_destroy(result, DESTRUCTOR(node_ptr_destroy));
            return NULL;
        }
    }

    for (size_t i = 0; i < size_v; i++) {
        struct node **child = vec_get(*children, i);
        int scs = match_expr(*child, expressions, n);
        if (scs == 0) {
            count += 1;
            if (get_n_node(*exp) && (index != count)) {
                continue;
            }
            result_node(child, expressions, n+1);
            struct node *child_ = *child;
            vec_push_back(result, &child_);
            *child = NULL;
        }
        if (scs == -2) {
            vec_destroy(result, DESTRUCTOR(node_ptr_destroy));
            return NULL;
        }
    }
    return result;
}

int result_node(struct node **root, struct vector *expressions, size_t n)
{
    if (root == NULL) {
        return -2;
    }
    if (((*root)->children == NULL) || (vec_size(expressions) < (n + 1))) {
        return 0;
    }
    struct vector *result = filter_children(&(*root)->children, expressions, n);
    if (result == NULL) {
        node_destroy(*root);
        *root = NULL;
        return -2;
    }
    struct node *return_node_ = return_node(&result);
    if (return_node_ == NULL) {
        vec_destroy(result, DESTRUCTOR(node_ptr_destroy));
        return -1;
    }
    node_destroy(*root);
    *root = return_node_;
    return 0;
}
