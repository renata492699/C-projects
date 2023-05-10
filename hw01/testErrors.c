#include "testErrors.h"

void print_error_message(char *message)
{
    fprintf(stderr, "%s\n", message);
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

bool safe_division(int operation, int64_t number)
{
    if (((operation == '/') || (operation == '%')) && (number == 0)) {
        print_error_message("Division by zero");
        return false;
    }
    return true;
}
