#include <stdio.h>

#include "xpath.h"

int parse_expressions(char *expressions, struct vector **expr_parsed)
{
    if (expressions == NULL) {
        return -1;
    }
    char *ptr = strtok(expressions, "/");
    while (ptr != NULL) {
        vec_push_back(*expr_parsed, &ptr);
        ptr = strtok(NULL, "/");
    }
    return 0;
}

int print_text(struct node *root, FILE *output)
{
    if (root->children == NULL || vec_empty(root->children)) {
        fprintf(output, "%s\n", root->text);
        return 0;
    }
    size_t size_v = vec_size(root->children);
    for (size_t i = 0; i < size_v; i++) {
        struct vector *ch = root->children;
        struct node **child = vec_get(ch, i);
        print_text(*child, output);
    }
    return 0;
}

void print_level(FILE *output, int level)
{
    for (int i = 0; i < level; i++) {
        fprintf(output, "    ");
    }
}

int print_attributes(FILE *output, struct vector *attributes)
{
    if (attributes == NULL) {
        return 0;
    }
    size_t size_v = vec_size(attributes);
    for (size_t i = 0; i < size_v; i++) {
        struct attribute **attribute = vec_get(attributes, i);
        fprintf(output, "%s=\"%s\" ",
                (*attribute)->key, (*attribute)->value);
    }
    return 0;
}

int print_xml(struct node *root, FILE *output, int level)
{
    print_level(output, level);
    fprintf(output, "<%s ", root->name);
    print_attributes(output, root->attributes);
    fprintf(output, ">\n");

    if (root->children == NULL || vec_empty(root->children)) {
        print_level(output, level+1);
        fprintf(output, "%s\n", root->text);
        print_level(output, level);
        fprintf(output, "</%s>\n", root->name);
        return 0;
    }

    size_t size_v = vec_size(root->children);
    for (size_t i = 0; i < size_v; i++) {
        struct node **child = vec_get(root->children, i);
        print_xml(*child, output, level+1);
    }
    print_level(output, level);
    fprintf(output, "</%s>\n", root->name);
    return 0;
}

int close_input_output(FILE *input, FILE *output)
{
    if (input != NULL) {
        fclose(input);
    }
    if ((output != NULL) && (output != stdout)) {
        fclose(output);
    }
    return 0;
}

int main(int argc, char **argv)
{
    bool text = false;
    bool xml = false;
    char expressions[500];
    FILE *output = NULL;
    FILE *input = NULL;

    int i = 1;
    while (i < argc) {
        char *arg = argv[i];
        if ((strcmp("-i", arg) == 0) || (strcmp("--input", arg) == 0)) {

            if (input != NULL) {
                close_input_output(input, output);
                fprintf(stderr, "Option input can be used only once.\n");
                return -1;
            }
            input = fopen(argv[i + 1], "r");
            if (input == NULL) {
                fprintf(stderr, "Open input file error.\n");
                return -1;
            }
            i += 2;
            continue;
        }
        if ((strcmp("-o", arg) == 0) || (strcmp("--output", arg) == 0)) {
            if (output != NULL) {
                close_input_output(input, output);
                fprintf(stderr, "Option output can be used only once.\n");
                return -1;
            }
            output = fopen(argv[i + 1], "w");
            if (output == NULL) {
                fprintf(stderr, "Open output file error.\n");
                return -1;
            }
            i += 2;
            continue;
        }
        if ((strcmp("-t", arg) == 0) || (strcmp("--text", arg) == 0)) {
            if (xml) {
                close_input_output(input, output);
                fprintf(stderr,
                        "Can not define text output and xml output at the same time.\n");
                return -1;
            }
            text = true;

        } else if ((strcmp("-x", arg) == 0) || (strcmp("--xml", arg) == 0)) {
            if (text) {
                fprintf(stderr,
                        "Can not define text output and xml output at the same time.\n");
                return -1;
            }
            xml = true;

        } else {
            strcpy(expressions, arg);
        }
        i++;
    }
    if (input == NULL) {
        input = stdin;
    }
    if (output == NULL) {
        output = stdout;
    }

    struct node *root;
    root = parse_xml(input);
    if (root == NULL) {
        fprintf(stderr, "Parse xml failed.\n");
        close_input_output(input, output);
        return -1;
    }

    if (!xml) {
        text = true;
    }

    struct vector *expr_vec = vec_create(sizeof(char **));
    parse_expressions(expressions, &expr_vec);

    int success = -1;
    char **root_name = vec_get(expr_vec, 0);
    if (strcmp(root->name, *root_name) == 0) {
        success = result_node(&root, expr_vec, 1);
    }
    if (success == 0) {
        if (text) {
            print_text(root, output);
        } else {
            print_xml(root, output, 0);
        }
    }
    close_input_output(input, output);
    vec_destroy(expr_vec, DESTRUCTOR(exp_ptr_destroy));
    node_destroy(root);
    return (success == -2) ? -1 : 0;
}
