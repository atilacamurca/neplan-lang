/**
 * Debug functions
 */

#include <stdarg.h>
#include <stdio.h>

#define LEVEL_INFO 0
#define LEVEL_DEBUG 1
#define LEVEL_ERROR 2

void debug(int level, const char *message, ...);
