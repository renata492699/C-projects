#ifndef TEST_ERRORS_H
#define TEST_ERRORS_H

#include "stdio.h"
#include "stdbool.h"
#include "ctype.h"
#include "stdint.h"

void print_error_message(char *message);
bool valid(int operation, int ch);
bool safe_division(int operation, int64_t number);

#endif
