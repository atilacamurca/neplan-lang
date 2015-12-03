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
    struct symlist *_symlist;
    int fn; /* which function */
}

/* Declare tokens */
%token <value> NUMBER
%token <_symbol> NAME
%token <fn> FUNC
%token EOL

%nonassoc <fn> CMP
%right '='
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS
%right POW

%type <tree> exp stmt explist multiple_assign conditional_assign
%type <_symlist> symlist

%start start

%%

stmt:
    /* TODO: latter we must have if, else, while */
     conditional_assign
    | multiple_assign
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
    | NAME '=' exp          { $$ = new_asign($1, $3); }
    | FUNC '(' explist ')'  { $$ = new_built_in_function($1, $3); }
;

explist: /* empty */        { $$ = NULL; }
    | exp
    | exp ',' explist       { $$ = new_ast(TYPE_STMT_LIST, $1, $3); }
;

symlist: NAME               { $$ = new_symlist($1, NULL); }
    | NAME ',' symlist      { $$ = new_symlist($1, $3); }
;

multiple_assign:
    symlist '=' explist     { $$ = new_multiple_assign($1, $3); }
;

conditional_assign:
    NAME '=' or_condition_list     { $$ = new_conditional_assign($1, $3); }
;

or_condition_list:
      exp                           { $$ = $1; } // TODO: se der certo fazer o mesmo em explist!
    | exp 'or' or_condition_list    { $$ = new_ast(TYPE_STMT_LIST, $1, $3); }
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
