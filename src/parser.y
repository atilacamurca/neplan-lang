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
%token <fn> FUNC_GETNUM FUNC_QUIT FUNC_PRINT
%token EOL

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
    FUNC_QUIT '(' ')'               { $$ = new_built_in_function($1, NULL); }
    | exp '+' exp                   { $$ = new_ast(OP_ADD, $1, $3); }
    | exp '-' exp                   { $$ = new_ast(OP_SUB, $1, $3); }
    | exp '*' exp                   { $$ = new_ast(OP_MUL, $1, $3); }
    | exp '/' exp                   { $$ = new_ast(OP_DIV, $1, $3); }
    | exp POW exp                   { $$ = new_ast(OP_POW, $1, $3); }
    | '-' exp %prec UMINUS          { $$ = new_ast(OP_UMINUS, $2, NULL); }
    | exp AND exp                   { $$ = new_ast(OP_AND, $1, $3); }
    | exp OR exp                    { $$ = new_ast(OP_OR, $1, $3); }
    | NOT exp %prec PNOT            { $$ = new_ast(OP_NOT, $2, NULL); }
    | NAME '=' exp                  { $$ = new_assign($1, $3, TYPE_UNDEFINED); }
    | NUMBER                        { $$ = new_number($1); }
    | BOOL                          { $$ = new_boolean($1); }
    | '(' exp ')'                   { $$ = $2; }
    | FUNC_PRINT '(' exp ')'        { $$ = handle_print_call($3); }
    | NAME                          { $$ = new_ref($1); }
;

explist:
      exp
    | exp ',' explist               { $$ = new_ast(TYPE_STMT_LIST, $1, $3); }
;

symlist: NAME                       { $$ = new_symlist($1, NULL); }
    | NAME ',' symlist              { $$ = new_symlist($1, $3); }
;

multiple_assign:
    symlist '=' '{' explist '}'      { $$ = new_multiple_assign($1, $4); }
;

dynamic_assign_number:
    NAME '=' FUNC_GETNUM '(' STRING ')' { $$ = new_dynamic_number_assign($1, $5); }
;

start:      /* empty */
    | start stmt EOL {
        if (iteractive_mode > 0) {
            handle_stmt_return($2);
            printf("\n" interactive_entry);
        } else {
            eval($2);
        }
        free_tree($2);
    }
    | start EOL             {
        if (iteractive_mode > 0) {
            printf(interactive_entry);
        }
    }
    | start error EOL       {
        yyerrok;
        if (iteractive_mode > 0) {
            printf(interactive_entry);
        }
    }
;
