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
    #include <string.h>
    #include "debug.h"
    #include "main.h"
    #include "main.tab.h"
    #include "opcodes.h"

    int debug_flag = 0; // FALSE
    int iteractive_mode = 0; // FALSE
%}

/* float exponent */
EXP     ([Ee][-+]?[0-9]+)

%%

 /* single character operators */
"+"     |
"-"     |
"*"     |
"/"     |
"="     |
","     |
"("     |
")"     {
    if (debug_flag > 0) debug(LEVEL_DEBUG, "Operator found: %c", yytext[0]);
    return yytext[0];
}

"**"            {
    if (debug_flag > 0) debug(LEVEL_DEBUG, "Operator found: **");
    return POW;
}

"true"  {
    if (debug_flag > 0) debug(LEVEL_DEBUG, "Type found: true");
    yylval.bool = 1; return BOOL;
}

"false" {
    if (debug_flag > 0) debug(LEVEL_DEBUG, "Type found: false");
    yylval.bool = 0; return BOOL;
}

"and"   {
    if (debug_flag > 0) debug(LEVEL_DEBUG, "Operator found: and");
    return AND;
}

"or"    {
    if (debug_flag > 0) debug(LEVEL_DEBUG, "Operator found: or");
    return OR;
}

"not"   {
    if (debug_flag > 0) debug(LEVEL_DEBUG, "Operator found: not");
    return NOT;
}

 /* Built-in Functions */
"print"         {
    if (debug_flag > 0) debug(LEVEL_DEBUG, "Built-in function found: print");
    yylval.fn = B_print; return FUNC;
}
"quit"          {
    if (debug_flag > 0) debug(LEVEL_DEBUG, "Built-in function found: quit");
    yylval.fn = B_quit; return FUNC;
}

"getnum"        {
    if (debug_flag > 0) debug(LEVEL_DEBUG, "Built-in function found: getnum");
    yylval.fn = B_get_num; return FUNC_GETNUM;
}

 /* Names */
[a-zA-Z][a-zA-Z0-9]*    {
    if (debug_flag > 0) debug(LEVEL_DEBUG, "Reference found: %c", *yytext);
    yylval._symbol = lookup(yytext);
    return NAME;
}

 /* Numbers */
[0-9]+"."[0-9]*{EXP}?   |
"."?[0-9]+{EXP}?        {
    if (debug_flag > 0) debug(LEVEL_DEBUG, "Number found: %g", atof(yytext));
    yylval.value = atof(yytext);
    return NUMBER;
}

`[^`/\\.\n]+`           {
    yylval.string = strdup(yytext+1);
    yylval.string[yyleng-2] = 0;
    if (debug_flag > 0) debug(LEVEL_DEBUG, "String found: %s", yylval.string);
    return STRING;
}

`[^`\n]*$               { debug(LEVEL_ERROR, "Unterminated string %s", yytext); }

 /* Ignore comments and whitespace. */
"#".*\n         { if (debug_flag > 0) debug(LEVEL_DEBUG, "Ignoring comments."); }
[ \t]           { if (debug_flag > 0) debug(LEVEL_DEBUG, "Ignoring whitespaces."); }

"\n"            { return EOL; }

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

            yyparse();
            fclose(yyin);
            return EXIT_SUCCESS;
        }
    } else {
        /* welcome message */
        printf("Welcome to neplan-lang. I will be your humble REPL.\n");
        printf("^C or quit() to Exit.\n> ");
        iteractive_mode = 1;
        yyparse();
        return 0;
    }
}
