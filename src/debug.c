
#include "main.h"
#include "debug.h"

void debug(int level, const char *message, ...) {
    switch (level) {
        case LEVEL_INFO:
            printf(ansi_bgcolor_blue "[INFO]" ansi_bgcolor_reset " ");
            break;
        case LEVEL_DEBUG:
            printf(ansi_bgcolor_yellow "[DEBUG]" ansi_bgcolor_reset " ");
            break;
        case LEVEL_ERROR:
            printf(ansi_bgcolor_red "[ERROR]" ansi_bgcolor_reset " line:%d: ", yylineno);
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
