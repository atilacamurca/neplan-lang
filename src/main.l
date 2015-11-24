/**
 * Main Lex File
 */

/* auto resumo scanning */
%option noyywrap

%option case-sensitive

/* keep track of the current line number */
%option yylineno

%{
    #include <stdio.h>
    #include "debug.h"
    int debug_flag = 0; // FALSE
%}

%%

 /* single character operators */
"+"     |
"-"     |
"*"     |
"/"     |
"("     |
")"     { if (debug_flag > 0) debug(LEVEL_DEBUG, "Operator found: %c", yytext[0]); }

 /* Ignore comments and whitespace. */
#[^\n]*         { if (debug_flag > 0) debug(LEVEL_DEBUG, "Ignoring comments."); }
[ \t\n]         { if (debug_flag > 0) debug(LEVEL_DEBUG, "Ignoring whitespaces."); }

 /* Ignore any other stuff */
.               { debug(LEVEL_ERROR, "Character not recognized %c", *yytext); }

 /* When reach EOF, print file statistics */
<<EOF>>         { if (debug_flag > 0) debug(LEVEL_INFO ,"File parsed, %u lines.", yylineno); return 0; }

%%

main(argc, argv)
int argc;
char **argv;
{
    if (argc > 1) {
        if (! (yyin = fopen(argv[argc - 1], "r"))) {
            perror(argv[argc - 1]);
            return EXIT_FAILURE;
        } else {
            if (strcmp("-d", argv[1]) == 0) {
                debug_flag = 1; // TRUE
            }

            yylex();
            fclose(yyin);
            return EXIT_SUCCESS;
        }
    }
}
