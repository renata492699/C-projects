#ifndef TOX_OPERATIONS_H
#define TOX_OPERATIONS_H

#include "ctype.h"
#include "stdbool.h"
#include "stdlib.h"
#include "string.h"
#include "stdint.h"
#include "stdio.h"

int64_t to_binary(int64_t accumulator);
void from_int_to_tox(int operation, int64_t accumulator);
bool valid(int operation, int ch);
bool tox_operations(int *ch, int64_t *number, int64_t accumulator);

#endif
