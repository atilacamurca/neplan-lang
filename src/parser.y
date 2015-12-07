/**
 * Main Bison File
 */

%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "main.h"
    #include "opcodes.h"
%}

%union {
    struct ast *tree;
    double value;
    int bool;
    struct symbol *_symbol; /* which symbol */
    struct symlist *_symlist;
    int fn; /* which function */
    char *string;
}

/* Declare tokens */
%token <value> NUMBER
%token <bool> BOOL
%token <string> STRING
%token <_symbol> NAME
%token <fn> FUNC FUNC_GETNUM
%token EOL

%nonassoc <fn> CMP
%right '='
%left OR
%left AND
%left NOT
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS PNOT
%right POW

%type <tree> exp stmt explist multiple_assign dynamic_assign_number
%type <_symlist> symlist

%start start

%%

stmt:
    /* TODO: latter we must have if, else, while */
    multiple_assign
    | dynamic_assign_number
    | exp
;

exp:
      exp '+' exp           { $$ = new_ast(OP_ADD, $1, $3); }
    | exp '-' exp           { $$ = new_ast(OP_SUB, $1, $3); }
    | exp '*' exp           { $$ = new_ast(OP_MUL, $1, $3); }
    | exp '/' exp           { $$ = new_ast(OP_DIV, $1, $3); }
    | exp POW exp           { $$ = new_ast(OP_POW, $1, $3); }
    | '(' exp ')'           { $$ = $2; }
    | '-' exp %prec UMINUS  { $$ = new_ast(OP_UMINUS, $2, NULL); }
    | NUMBER                { $$ = new_number($1); }
    | NAME                  { $$ = new_ref($1); }
    | NAME '=' exp          { $$ = new_assign($1, $3); }
    | FUNC '(' explist ')'  { $$ = new_built_in_function($1, $3); }
    | exp AND exp           { $$ = new_ast(OP_AND, $1, $3); }
    | exp OR exp            { $$ = new_ast(OP_OR, $1, $3); }
    | NOT exp %prec PNOT    { $$ = new_ast(OP_NOT, $2, NULL); }
    | BOOL                  { $$ = new_boolean($1); }
;

explist: /* empty */        { $$ = NULL; }
    | exp
    | exp ',' explist       { $$ = new_ast(TYPE_STMT_LIST, $1, $3); }
;

symlist: NAME               { $$ = new_symlist($1, NULL); }
    | NAME ',' symlist      { $$ = new_symlist($1, $3); }

multiple_assign:
    symlist '=' explist     { $$ = new_multiple_assign($1, $3); }
;

dynamic_assign_number:
    NAME '=' FUNC_GETNUM '(' STRING ')' { $$ = new_dynamic_number_assign($1, $5); }
;

start:      /* empty */
    | start stmt EOL {
        if (iteractive_mode > 0) {
            printf("= %4.4g\n> ", eval($2));
        }
        free_tree($2);
    }
    | start EOL             {
        if (iteractive_mode > 0) {
            printf("> ");
        }
    }
    | start error EOL       {
        yyerrok;
        if (iteractive_mode > 0) {
            printf("> ");
        }
    }
;
