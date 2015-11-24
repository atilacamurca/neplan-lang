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
%}

%%

 /* Ignore comments and whitespace. */
#[^\n]*         {}
[ \t\r\n]       {}

 /* Ignore any other stuff */
.               {}

 /* When reach EOF, print file statistics */
<<EOF>>         { printf("[INFO] File parsed, %u lines.\n", yylineno); return 0; }

%%

main(argc, argv)
int argc;
char **argv;
{
    if (argc > 1) {
        if (! (yyin = fopen(argv[1], "r"))) {
            perror(argv[1]);
            return EXIT_FAILURE;
        } else {
            yylex();
            fclose(yyin);
            return EXIT_SUCCESS;
        }
    }
}
