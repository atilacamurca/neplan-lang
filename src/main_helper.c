/**
 * Helper functions for main file
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "main.h"
#include "debug.h"
#include "opcodes.h"

/**
 * hash a symbol
 */
static unsigned
symhash(char *sym)
{
    unsigned int hash = 0;
    unsigned c;

    while (c = *sym++) {
        hash = hash * 9 ^ c;
    }

    return hash;
}

struct symbol *
lookup(char *sym)
{
    struct symbol *sp = &symtab[symhash(sym) % NHASH];
    int scount = NHASH; /* how many have we looked at */

    while (--scount >= 0) {
        /* if symbol already exists */
        if (sp->name && !strcmp(sp->name, sym)) {
            return sp;
        }

        /* new entry */
        if (!sp->name) {
            sp->name = strdup(sym);
            sp->value = 0;
            sp->type = TYPE_NUMBER;
            return sp;
        }

        /* keep looking otherwise */
        if (++sp >= symtab + NHASH) {
            sp = symtab;
        }
    }

    debug(LEVEL_ERROR, "Symbol Table Overflow. Table is full.");
    abort();
}

struct ast *
new_ast(int node_type, struct ast *left, struct ast *right)
{
    struct ast *a = malloc(sizeof(struct ast));

    if (!a) {
        debug(LEVEL_ERROR, "out of space.");
        exit(EXIT_FAILURE);
    }

    a->node_type = node_type;
    a->left = left;
    a->right = right;
    return a;
}

struct ast *
new_number(double value)
{
    struct num_val *a = malloc(sizeof(struct num_val));

    if (!a) {
        debug(LEVEL_ERROR, "out of space.");
        exit(EXIT_FAILURE);
    }

    a->node_type = TYPE_NUMBER;
    a->number = value;
    return (struct ast *) a;
}

struct ast *
new_boolean(int value) {
    struct bool_val *a = malloc(sizeof(struct bool_val));

    if (!a) {
        debug(LEVEL_ERROR, "out of space.");
        exit(EXIT_FAILURE);
    }

    a->node_type = TYPE_BOOL;
    a->boolean = !!value;
    return (struct ast *) a;
}

struct ast *
new_built_in_function(int func_type, struct ast *left)
{
    struct fncall *a = malloc(sizeof(struct fncall));

    if (!a) {
        debug(LEVEL_ERROR, "out of space.");
        exit(EXIT_FAILURE);
    }

    a->node_type = TYPE_FUNC;
    a->left = left;
    a->func_type = func_type;
    return (struct ast *) a;
}

struct ast *
new_ref(struct symbol *s)
{
    struct sym_ref *a = malloc(sizeof(struct sym_ref));

    if (!a) {
        debug(LEVEL_ERROR, "out of space.");
        exit(EXIT_FAILURE);
    }

    a->node_type = TYPE_REF;
    a->_symbol = s;
    return (struct ast *) a;
}

struct ast *
new_assign(struct symbol *s, struct ast *v, int type)
{
    struct sym_asign *a = malloc(sizeof(struct sym_asign));
    if (!a) {
        debug(LEVEL_ERROR, "out of space.");
        exit(EXIT_FAILURE);
    }

    a->node_type = TYPE_ASIGN;
    a->_symbol = s;
    a->_symbol->type = type;
    a->value = v;
    return (struct ast *) a;
}

struct symlist *
new_symlist(struct symbol *_symbol, struct symlist *next)
{
    struct symlist *sl = malloc(sizeof(struct symlist));

    if(!sl) {
        debug(LEVEL_ERROR, "out of space.");
        exit(EXIT_FAILURE);
    }
    sl->_symbol = _symbol;
    sl->next = next;
    return sl;
}

void
free_symlist(struct symlist *_symlist)
{
    struct symlist *aux;

    while(_symlist) {
        aux = _symlist->next;
        free(_symlist);
        _symlist = aux;
    }
}

void
free_tree(struct ast *a)
{
    switch (a->node_type) {
        /* two subtrees */
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_POW:
        case TYPE_STMT_LIST:
        case OP_AND:
        case OP_OR:
            free_tree(a->right);
            break;

        /* one subtree */
        case TYPE_FUNC:
        case OP_UMINUS:
        case OP_NOT:
            if (a->left) {
                free_tree(a->left);
            }
            break;

        /* no subtree */
        case TYPE_NUMBER:
        case TYPE_REF:
        case TYPE_BOOL:
            break;

        case TYPE_ASIGN:
            free(((struct sym_asign *)a)->value);
            break;
        default:
            debug(LEVEL_ERROR, "Free bad node %c", a->node_type);
    }

    free(a); /* free the node itself */
}

/**
 * Call a built-in function
 */
static double call_built_in_function(struct fncall *);

double
eval(struct ast *a)
{
    double value;

    if (!a) {
        debug(LEVEL_ERROR, "Internal error, null eval.");
        exit(EXIT_FAILURE);
    }

    switch (a->node_type) {
        case TYPE_NUMBER:
            value = ((struct num_val *)a)->number;
            break;

        case TYPE_BOOL:
            value = (double) ((struct bool_val *)a)->boolean;
            break;

        case TYPE_REF:
            value = ((struct sym_ref *)a)->_symbol->value;
            break;

        case TYPE_ASIGN:
            value = ((struct sym_asign *)a)->_symbol->value = eval( ((struct sym_asign *)a)->value );
            break;

        case OP_ADD:
            value = eval(a->left) + eval(a->right);
            break;
        case OP_SUB:
            value = eval(a->left) - eval(a->right);
            break;
        case OP_MUL:
            value = eval(a->left) * eval(a->right);
            break;
        case OP_DIV:
            /* TODO: check for division by zero! */
            value = eval(a->left) / eval(a->right);
            break;
        case OP_UMINUS:
            value = -eval(a->left);
            break;
        case OP_POW:
            value = pow(eval(a->left), eval(a->right));
            break;

        /* logic operators */
        case OP_AND:
            value = eval(a->left) && eval(a->right);
            break;
        case OP_OR:
            value = eval(a->left) || eval(a->right);
            break;
        case OP_NOT:
            value = !eval(a->left);
            break;

        /* list of statements */
        case TYPE_STMT_LIST:
            eval(a->left);
            value = eval(a->right);
            break;

        case TYPE_FUNC:
            value = call_built_in_function((struct fncall *) a);
            break;

        default:
            debug(LEVEL_ERROR, "Internal error. Bad node %c.", a->node_type);
    }
    return value;
}

static double
call_built_in_function(struct fncall *fn)
{
    enum bifs func_type = fn->func_type;
    double value = 0.0;
    int type = 0;

    switch (func_type) {
        case B_quit:
            printf("See ya!\n");
            exit(EXIT_SUCCESS);
        default:
            debug(LEVEL_ERROR, "Unknown built-in function %d", func_type);
            return 0.0;
    }
}

struct ast *
new_multiple_assign(struct symlist *_symlist, struct ast *explist)
{
    int num_args_symbol, num_args_expr, index;
    struct symlist *cloned_args;
    double *current_values;
    struct ast *last_value;

    struct symlist *sl; /* for counting arguments */
    /* count the arguments */
    sl = _symlist;
    for(num_args_symbol = 0; sl; sl = sl->next) {
        num_args_symbol++;
    }

    /* store current values for latter reference, like a, b = b, a */
    current_values = (double *) malloc(num_args_symbol * sizeof(double));

    /* evaluate the arguments */
    cloned_args = _symlist;
    for (index = 0; cloned_args; index++) {
        if (explist->node_type == TYPE_STMT_LIST) {
            current_values[index] = eval(explist->left);
            explist = explist->right;
        } else {
            // if it's the end of the list
            // continue to give the last value if
            // the number of symbols is less than the number of args
            current_values[index] = eval(explist);
            last_value = explist;
        }
        cloned_args = cloned_args->next;
    }

    for (index = 0;_symlist; _symlist = _symlist->next, index++) {
        // _symlist->_symbol->value = current_values[index];
        _symlist->_symbol = lookup(_symlist->_symbol->name);
        _symlist->_symbol->value = current_values[index];
    }

    free(current_values);
    return last_value;
}

struct ast *
new_dynamic_number_assign(struct symbol *_symbol, char *message)
{
    printf("%s", message);

    char buffer[256];
    fgets(buffer, 256, stdin);
    double value = atof(buffer);

    return new_assign(_symbol, new_number(value), TYPE_NUMBER);
}

struct ast *
handle_print_call(struct ast * param)
{
    handle_output(param, ansi_color_magenta "%.4g\n" ansi_color_reset, ansi_color_magenta "%s\n" ansi_color_reset);
    return param;
}

void
handle_stmt_return(struct ast *param)
{
    handle_output(param, ansi_dim "= %.4g" ansi_dim_reset, ansi_dim "= %s" ansi_dim_reset);
}

void
handle_output(struct ast *param, char *template_for_number, char *template_for_boolean)
{
    double value = 0.0;
    int type = 0;
    struct symbol *s;
    if (param) {
        value = eval(param);
        switch (param->node_type) {
            case TYPE_BOOL:
            case OP_AND:
            case OP_OR:
            case OP_NOT:
                printf(template_for_boolean, (!!value ? "true" : "false"));
                break;
            case TYPE_REF:
                s = lookup(((struct sym_ref *) param)->_symbol->name);
                switch (s->type) {
                    case TYPE_NUMBER:
                        printf(template_for_number, s->value);
                        break;
                    case TYPE_BOOL:
                        printf(template_for_boolean, (!!s->value ? "true" : "false"));
                        break;
                }
                break;
            default:
                printf(template_for_number, value);
        }
    }
}

void
yyerror(char *s, ...)
{
    va_list ap;
    va_start(ap, s);

    debug(LEVEL_ERROR, s, ap);
}
