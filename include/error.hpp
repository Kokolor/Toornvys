#pragma once

#include <cstdio>
#include <cstdlib>

#define ERROR(line, ...)                                    \
    do                                                      \
    {                                                       \
        fprintf(stderr, "\033[31mError (line %d): ", line); \
        fprintf(stderr, __VA_ARGS__);                       \
        fprintf(stderr, "\033[0m\n");                       \
        exit(EXIT_FAILURE);                                 \
    } while (0)