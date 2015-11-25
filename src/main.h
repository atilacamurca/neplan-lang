/**
 * Declarations for neplan-lang
 */

/* interface to lexer */
extern int yylineno;
extern int iteractive_mode;
void yyerror(char *s, ...);

/* symbol table struct */
struct symbol {
    char *name;     /* variable name */
    double value;
};

/**
 * Simple symbol table of fixed size
 */
#define NHASH 9997

struct symbol symtab[NHASH];

struct symbol *lookup(char*);

/**
 * Enum of Built-in Functions
 */
enum bifs {
    B_print = 1,
    B_get_num
};

/**
 * Struct for Abstract Syntax Tree (AST)
 */
struct ast {
    int node_type;
    struct ast *left;
    struct ast *right;
};

struct num_val {
    /* type K */
    int node_type;
    double number;
};

struct sym_asign {
    /* type = */
    int node_type;
    struct symbol *_symbol;
    struct ast *value;
};

struct sym_ref {
    /* type N */
    int node_type;
    struct symbol *_symbol;
};

/**
 * Build an AST
 */

/**
 * Create a new AST
 * @param  node_type
 * @param  left
 * @param  right
 * @return
 */
struct ast *new_ast(int node_type, struct ast *left, struct ast *right);

/**
 * Create a new reference and put in the symbols table
 * @param  _symbol
 * @return
 */
struct ast *new_ref(struct symbol *_symbol);

/**
 * Create a new asignment instruction
 * @param  _symbol
 * @param  value
 * @return
 */
struct ast *new_asign(struct symbol *_symbol, struct ast *value);

/**
 * Create a new variable type number
 * @param  value
 * @return
 */
struct ast *new_number(double value);

/**
 * Evaluate an AST
 * TODO: return something else instead double.
 * @param  tree
 * @return
 */
double eval(struct ast * tree);

/**
 * Release me from my spell, let me go!
 * @param tree
 */
void free_tree(struct ast * tree);

/**
 * Dump an AST for the dumbs
 * @param tree
 * @param level
 */
void dump_ast(struct ast *tree, int level);