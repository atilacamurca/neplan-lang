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

    while ((c = (*sym++))) {
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
new_boolean(int value)
{
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

    a->node_type = TYPE_ASSIGN;
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

        case TYPE_ASSIGN:
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

void parse_number(struct ast *a, double *p_value);

void parse_boolean(struct ast *a, int *p_value);

struct ast *
eval(struct ast *a)
{
    double aux_value1, aux_value2;
    int aux_bool1, aux_bool2;
    struct ast *aux, *aux2;
    struct symbol *aux_sym;

    if (!a) {
        debug(LEVEL_ERROR, "Internal error, null eval.");
        exit(EXIT_FAILURE);
    }

    switch (a->node_type) {
        case TYPE_NUMBER:
            // value = ((struct num_val *)a)->number;
        case TYPE_BOOL:
            // value = (double) ((struct bool_val *)a)->boolean;
        case TYPE_REF:
            // value = ((struct sym_ref *)a)->_symbol->value;
            break;

        case TYPE_ASSIGN:
            aux = eval( ((struct sym_asign *)a)->value );
            aux_sym = ((struct sym_asign *)a)->_symbol;
            switch (aux->node_type) {
                case TYPE_NUMBER:
                    aux_sym->value = ((struct num_val *)aux)->number;
                    aux_sym->type = TYPE_NUMBER;
                    a = new_assign(aux_sym, aux, TYPE_NUMBER);
                    break;
                case TYPE_BOOL:
                    aux_sym->value = ((struct bool_val *)aux)->boolean;
                    aux_sym->type = TYPE_BOOL;
                    a = new_assign(aux_sym, aux, TYPE_BOOL);
                    break;
                case TYPE_REF:
                    aux_sym = ((struct sym_ref *)aux)->_symbol;
                    a = new_assign(aux_sym, aux, aux_sym->type);
                    break;
            }
            break;

        case OP_ADD:
            aux = eval(a->left);
            aux2 = eval(a->right);
            parse_number(aux, &aux_value1);
            parse_number(aux2, &aux_value2);
            if (aux_value1 && aux_value2) {
                a = new_number(aux_value1 + aux_value2);
            }
            break;
        case OP_SUB:
            aux = eval(a->left);
            aux2 = eval(a->right);
            parse_number(aux, &aux_value1);
            parse_number(aux2, &aux_value2);
            if (aux_value1 && aux_value2) {
                a = new_number(aux_value1 - aux_value2);
            }
            break;
        case OP_MUL:
            aux = eval(a->left);
            aux2 = eval(a->right);
            parse_number(aux, &aux_value1);
            parse_number(aux2, &aux_value2);
            if (aux_value1 && aux_value2) {
                a = new_number(aux_value1 * aux_value2);
            }
            break;
        case OP_DIV:
            aux = eval(a->left);
            aux2 = eval(a->right);
            parse_number(aux, &aux_value1);
            parse_number(aux2, &aux_value2);
            if (aux_value1 && aux_value2) {
                if (aux_value2 != 0) {
                    a = new_number(aux_value1 / aux_value2);
                } else {
                    debug(LEVEL_ERROR, "Division by zero.");
                }
            }
            break;
        case OP_UMINUS:
            aux = eval(a->left);
            parse_number(aux, &aux_value1);
            if (aux_value1) {
                a = new_number(- aux_value1);
            }
            break;
        case OP_POW:
            aux = eval(a->left);
            aux2 = eval(a->right);
            parse_number(aux, &aux_value1);
            parse_number(aux2, &aux_value2);
            if (aux_value1 && aux_value2) {
                a = new_number(pow(aux_value1, aux_value2));
            }
            break;

        /* logic operators */
        case OP_AND:
            aux = eval(a->left);
            aux2 = eval(a->right);
            parse_boolean(aux, &aux_bool1);
            parse_boolean(aux2, &aux_bool2);
            a = new_boolean(aux_bool1 && aux_bool2);
            break;
        case OP_OR:
            aux = eval(a->left);
            aux2 = eval(a->right);
            parse_boolean(aux, &aux_bool1);
            parse_boolean(aux2, &aux_bool2);
            a = new_boolean(aux_bool1 || aux_bool2);
            break;
        case OP_NOT:
            aux = eval(a->left);
            parse_boolean(aux, &aux_bool1);
            a = new_boolean(! aux_bool1);
            break;

        /* list of statements */
        case TYPE_STMT_LIST:
            eval(a->left);
            eval(a->right);
            break;

        case TYPE_FUNC:
            call_built_in_function((struct fncall *) a);
            break;

        default:
            debug(LEVEL_ERROR, "Internal error. Bad node %c.", a->node_type);
    }
    return a;
}

void
parse_number(struct ast *a, double *p_value)
{
    struct symbol *s;
    switch (a->node_type) {
        case TYPE_NUMBER:
            *p_value = ((struct num_val *) a)->number;
            break;
        case TYPE_REF:
            s = lookup(((struct sym_ref *) a)->_symbol->name);
            switch (s->type) {
                case TYPE_NUMBER:
                    *p_value = s->value;
                    break;
                default:
                    debug(LEVEL_ERROR, "Variable '%s' is not a number.", s->name);
            }
            break;
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_UMINUS:
        case OP_POW:
            *p_value = ((struct num_val *) a)->number;
            break;
        default:
            debug(LEVEL_ERROR, "Expression is not numeric.");
    }
}

void
parse_boolean(struct ast *a, int *p_value)
{
    struct symbol *s;
    switch (a->node_type) {
        case TYPE_BOOL:
            *p_value = !!((struct bool_val *)a)->boolean;
            break;
        case TYPE_REF:
            s = lookup(((struct sym_ref *) a)->_symbol->name);
            switch (s->type) {
                case TYPE_BOOL:
                    *p_value = !!s->value;
                    break;
                default:
                    debug(LEVEL_ERROR, "Variable '%s' is not a boolean.", s->name);
            }
            break;
        case OP_AND:
        case OP_OR:
        case OP_NOT:
            *p_value = !!((struct bool_val *) a)->boolean;
            break;
        default:
            debug(LEVEL_ERROR, "Expression is not boolean.");
    }
}

static double
call_built_in_function(struct fncall *fn)
{
    enum bifs func_type = fn->func_type;
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
    int num_args_symbol, index;
    struct symlist *cloned_args;
    struct ast **current_values; /* store current values for latter reference, like a, b = b, a */
    struct ast *last_value;
    // double aux_value;
    // int aux_bool;

    struct symlist *sl; /* for counting arguments */
    /* count the arguments */
    sl = _symlist;
    for(num_args_symbol = 0; sl; sl = sl->next) {
        num_args_symbol++;
    }

    current_values = malloc(num_args_symbol * sizeof(struct ast));

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
            last_value = current_values[index] = eval(explist);
        }
        cloned_args = cloned_args->next;
    }

    for (index = 0; _symlist; _symlist = _symlist->next, index++) {
        // 1st tentative, works great if the language has only one type
        // _symlist->_symbol = lookup(_symlist->_symbol->name);
        // _symlist->_symbol->value = current_values[index];

        // 2nd tentative, overrides the symbol values
        // _symlist->_symbol = lookup(_symlist->_symbol->name);
        // switch (_symlist->_symbol->type) {
        //     case TYPE_NUMBER:
        //         parse_number(current_values[index], &aux_value);
        //         if (aux_value) {
        //             _symlist->_symbol->value = aux_value;
        //         }
        //         break;
        //     case TYPE_BOOL:
        //         parse_boolean(current_values[index], &aux_bool);
        //         _symlist->_symbol->value = aux_bool;
        //         break;
        //     default:
        //         debug(LEVEL_ERROR, "Error while parsing, type not permited.");
        //}

        // 3th tentative
        switch (current_values[index]->node_type) {
            case TYPE_BOOL:
            case OP_AND:
            case OP_OR:
            case OP_NOT:
                _symlist->_symbol->value = ((struct bool_val *) current_values[index])->boolean;
                _symlist->_symbol->type = TYPE_BOOL;
                break;
            case TYPE_NUMBER:
            case OP_ADD:
            case OP_SUB:
            case OP_MUL:
            case OP_DIV:
            case OP_UMINUS:
            case OP_POW:
                _symlist->_symbol->value = ((struct num_val *) current_values[index])->number;
                _symlist->_symbol->type = TYPE_NUMBER;
                break;
            case TYPE_REF:
                _symlist->_symbol->value = ((struct sym_ref *) current_values[index])->_symbol->value;
                break;

        }
    }
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
    struct ast *value;
    struct symbol *s;
    if (param) {
        value = eval(param);
        switch (value->node_type) {
            case TYPE_BOOL:
            case OP_AND:
            case OP_OR:
            case OP_NOT:
                printf(template_for_boolean, (!!((struct bool_val *) value)->boolean ? "true" : "false"));
                break;
            case TYPE_REF:
                s = lookup(((struct sym_ref *) value)->_symbol->name);
                switch (s->type) {
                    case TYPE_NUMBER:
                        printf(template_for_number, s->value);
                        break;
                    case TYPE_BOOL:
                        printf(template_for_boolean, (!!s->value ? "true" : "false"));
                        break;
                }
                break;
            case TYPE_NUMBER:
            case OP_ADD:
            case OP_SUB:
            case OP_MUL:
            case OP_DIV:
            case OP_UMINUS:
            case OP_POW:
                printf(template_for_number, ((struct num_val *) value)->number);
                break;
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
