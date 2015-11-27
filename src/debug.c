
#include "debug.h"

void debug(int level, const char *message, ...) {
    switch (level) {
        case LEVEL_INFO:
            printf("[INFO] ");
            break;
        case LEVEL_DEBUG:
            printf("[DEBUG] ");
            break;
        case LEVEL_ERROR:
            printf("[ERROR] line:%d: ", yylineno);
            break;
        default:
            printf("[UNSET] ");
    }

    va_list args;
    va_start(args, message);
    vprintf(message, args);
    printf("\n");
    va_end(args);
}
