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
new_asign(struct symbol *s, struct ast *v)
{
    struct sym_asign *a = malloc(sizeof(struct sym_asign));
    if (!a) {
        debug(LEVEL_ERROR, "out of space.");
        exit(EXIT_FAILURE);
    }

    a->node_type = TYPE_ASIGN;
    a->_symbol = s;
    a->value = v;
    return (struct ast *) a;
}

void
free_tree(struct ast *a)
{
    switch (a->node_type) {
        /* two subtrees */
        case '+':
        case '-':
        case '*':
        case '/':
            free_tree(a->right);
            break;

        /* no subtree */
        case TYPE_NUMBER:
        case TYPE_REF:
            break;

        case TYPE_ASIGN:
            free(((struct sym_asign *)a)->value);
            break;
        default:
            debug(LEVEL_ERROR, "Free bad node %c", a->node_type);
    }

    free(a); /* free the node itself */
}

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

        default:
            debug(LEVEL_ERROR, "Internal error. Bad node %c.", a->node_type);
    }
    return value;
}

void
yyerror(char *s, ...)
{
    va_list ap;
    va_start(ap, s);

    fprintf(stderr, "%d: error: ", yylineno);
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
}
