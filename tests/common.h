#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define TEST(condition)                                                                                    \
    if (!(bool)(condition)) {                                                                              \
        printf("\n=== TEST FAILED ===\nIn %s:%d: %s()\n%s\n\n", __FILE__, __LINE__, __func__, #condition); \
        exit(1);                                                                                           \
    }

