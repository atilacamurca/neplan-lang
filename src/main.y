/**
 * Main Bison File
 */

%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "main.h"
    #include "opcodes.h"
%}

%union {
    struct ast *tree;
    double value;
    struct symbol *_symbol; /* which symbol */
}

/* Declare tokens */
%token <value> NUMBER
%token <_symbol> NAME
%token EOL

%right '='
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS

%type <tree> exp stmt

%start start

%%

stmt:
    /* TODO: latter we must have if, else, while */
    exp
;

exp:
      exp '+' exp           { $$ = new_ast(OP_ADD, $1, $3); }
    | exp '-' exp           { $$ = new_ast(OP_SUB, $1, $3); }
    | exp '*' exp           { $$ = new_ast(OP_MUL, $1, $3); }
    | exp '/' exp           { $$ = new_ast(OP_DIV, $1, $3); }
    | '(' exp ')'           { $$ = $2; }
    | '-' exp %prec UMINUS  { $$ = new_ast(OP_UMINUS, $2, NULL); }
    | NUMBER                { $$ = new_number($1); }
    | NAME                  { $$ = new_ref($1); }
    | NAME '=' exp          { $$ = new_asign($1, $3); }
;

start:      /* empty */
    | start stmt EOL /* for now print the result */ {
        if (iteractive_mode > 0) {
            printf("= %4.4g\n> ", eval($2));
        }
        free_tree($2);
    }
    | start error EOL       {
        yyerrok;
        if (iteractive_mode > 0) {
            printf("> ");
        }
    }
;
