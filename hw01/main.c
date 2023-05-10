#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_error_message(char *message)
{
    fprintf(stderr, "%s\n", message);
}

int64_t to_binary(int64_t accumulator)
{
    int64_t n = accumulator;
    int64_t binary = 0;
    while (n > 0) {
        int i = 1;
        while (n != 0) {
            binary += (n % 2) * i;
            n /= 2;
            i *= 10;
        }
    }
    return binary;
}

void number_function(int input, int64_t *number)
{
    if (*number == INT64_MIN) {
        *number = 0;
    }
    int64_t input_num = input - '0';
    *number *= 10;
    *number += input_num;
}

bool safe_division(int operation, int64_t number)
{
    if (((operation == '/') || (operation == '%')) && (number == 0)) {
        print_error_message("Division by zero");
        return false;
    }
    return true;
}

bool op_w_parameter(int64_t *number, int64_t *accumulator, int operation)
{
    if ((*number == INT64_MIN) && (operation != 0)) {
        print_error_message("Syntax error");
        return false;
    }
    if (!safe_division(operation, *number)) {
        return false;
    }
    if (operation == 'P') {
        *accumulator = *number;
    } else if (operation == '+') {
        *accumulator += *number;
    } else if (operation == '-') {
        *accumulator -= *number;
    } else if (operation == '*') {
        *accumulator *= *number;
    } else if (operation == '/') {
        *accumulator /= *number;
    } else if (operation == '%') {
        *accumulator %= *number;
    } else if (operation == '<') {
        *accumulator <<= *number;
    } else if (operation == '>') {
        *accumulator >>= *number;
    } else if (operation == 0) {
        return true;
    } else {
        print_error_message("Syntax error");
        return false;
    }
    if (strchr("P+-/%><*", operation) != NULL) {
        printf("# %ld\n", *accumulator);
        *number = INT64_MIN;
    }
    return true;
}

void from_int_to_tox(int operation, int64_t accumulator)
{
    if (operation == 'T') {
        int64_t result = to_binary(accumulator);
        printf("# %ld\n", result);
    } else if (operation == 'O') {
        printf("# %lo\n", accumulator);
    } else if (operation == 'X') {
        printf("# %lX\n", accumulator);
    }
}

bool valid(int operation, int ch)
{
    if (isalpha(ch) && (operation != 'X')) {
        print_error_message("Syntax error");
        return false;
    }
    if ((operation == 'T') && ((ch != '1') && (ch != '0'))) {
        print_error_message("Syntax error");
        return false;
    }
    if ((operation == 'O') && ((ch == '8') || (ch == '9'))) {
        print_error_message("Syntax error");
        return false;
    }
    return true;
}

bool tox_operations(int *ch, int64_t *number, int64_t accumulator)
{
    int operation = *ch;
    from_int_to_tox(operation, accumulator);

    // prepis argumentu za operaci
    *ch = getchar();
    char base_number[50] = { 0 };
    while (isdigit(*ch) || (strchr("abcdefABCDEF", *ch) != NULL) || isspace(*ch)) {
        if (isspace(*ch)) {
            *ch = getchar();
            continue;
        }
        if (!valid(operation, *ch)) {
            return false;
        }
        char ch_char = (char) *ch;
        if (isalpha(*ch)) {
            char to_upper_char = (char) toupper(*ch);
            strncat(base_number, &to_upper_char, 1);
        } else {
            strncat(base_number, &ch_char, 1);
        }
        *ch = getchar();
    }

    // number se nesmi prepsat na 0, pokud operation je bez argumentu
    if (base_number[0] == '\0') {
        return true;
    }
    if (operation == 'T') {
        *number = strtoll(base_number, NULL, 2);
    } else if (operation == 'O') {
        *number = strtoll(base_number, NULL, 8);
    } else if (operation == 'X') {
        *number = strtoll(base_number, NULL, 16);
    }
    return true;
}

void memory_opt(int64_t *memory, int64_t *accumulator, int operation)
{
    if (operation == 'N') {
        *accumulator = 0;
        printf("# %ld\n", *accumulator);
    } else if (operation == 'M') {
        *memory = *memory + *accumulator;
    } else if (operation == 'R') {
        *memory = 0;
    }
}

bool calculate(int *ch, int64_t *accumulator, int64_t *memory)
{
    int operation = 0;
    int64_t number = INT64_MIN;
    bool success;

    while ((operation != ';') && (operation != EOF)) {
        if (isspace(*ch)) {
            *ch = getchar();
            continue;
        }
        if (isdigit(*ch)) {
            number_function(*ch, &number);
            *ch = getchar();
            continue;
        }
        if (*ch == 'm') {
            number = *memory;
            *ch = getchar();
            continue;
        }
        if ((*ch != 'X') && (*ch != 'T') && (*ch != 'O')) {
            success = op_w_parameter(&number, accumulator, operation);
            if (!success) {
                return false;
            }
            operation = *ch;
        }
        if ((*ch == 'X') || (*ch == 'T') || (*ch == 'O')) {
            success = tox_operations(ch, &number, *accumulator);
            if (!success) {
                return false;
            }
            continue;
        }
        if ((operation == 'N') || (operation == 'R') || (operation == 'M')) {
            memory_opt(memory, accumulator, operation);
            operation = 0;
        }
        if (operation == '=') {
            operation = 0;
            printf("# %ld\n", *accumulator);
        }
        *ch = getchar();
    }
    if (number != INT64_MIN) {
        print_error_message("Syntax error");
        return false;
    }
    if (operation == EOF) {
        *ch = operation;
        return true;
    }

    // pokud se na vstupu nachazi komentar
    while ((*ch != EOF) && (*ch != '\n')) {
        *ch = getchar();
    }
    return true;
}

int main(void)
{
    int ch = getchar();
    bool calc = true;
    int64_t accumulator = 0;
    int64_t memory = 0;
    while (ch != EOF) {
        calc = calculate(&ch, &accumulator, &memory);
        if (!calc) {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}
