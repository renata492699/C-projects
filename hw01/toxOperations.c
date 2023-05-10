#include "toxOperations.h"

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
