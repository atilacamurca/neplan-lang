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
    int type;       /* variable type */
    double value;
};

/**
 * Simple symbol table of fixed size
 */
#define NHASH 9997

struct symbol symtab[NHASH];

struct symbol *lookup(char*);

/**
 * List of symbols, for an argument list
 */
struct symlist {
    struct symbol *_symbol;
    struct symlist *next;
};

/**
 * Create a new symbol list
 * @param  _symbol
 * @param  next
 * @return
 */
struct symlist *new_symlist(struct symbol *_symbol, struct symlist *next);

/**
 * Clean the symbol list from memory
 * @param _symlist
 */
void free_symlist(struct symlist *_symlist);

/**
 * Handle multiple assignments
 * @param  _symlist
 * @param  right
 * @return
 */
struct ast *new_multiple_assign(struct symlist *_symlist, struct ast *explist);

/**
 * Handle a dynamic assignment of a number
 * @param  _symbol
 * @param  message
 * @return
 */
struct ast *new_dynamic_number_assign(struct symbol *_symbol, char *message);

/**
 * Enum of Built-in Functions
 */
enum bifs {
    B_print = 1,
    B_get_num,
    B_quit
};

/**
 * Struct for Abstract Syntax Tree (AST)
 */
struct ast {
    int node_type;
    struct ast *left;
    struct ast *right;
};

/**
 * Struct for Built-in Functions
 */
struct fncall {
    /* type F */
    int node_type;
    struct ast *left;
    enum bifs func_type;
};

struct num_val {
    /* type K */
    int node_type;
    double number;
};

struct bool_val {
    /* type B */
    int node_type;
    int boolean;
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
 * Create new built-in function
 * @param  func_type
 * @param  left
 * @return
 */
struct ast *new_built_in_function(int func_type, struct ast *left);

// struct ast *new_call(struct symbol *_symbol, struct ast *left);

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
struct ast *new_assign(struct symbol *_symbol, struct ast *value, int type);

/**
 * Create a new variable type number
 * @param  value
 * @return
 */
struct ast *new_number(double value);

/**
 * Create a new variable type boolean
 * @param  value [description]
 * @return       [description]
 */
struct ast *new_boolean(int value);

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

#define interactive_entry "#> "

/* Hack for colors */
#define ansi_color_magenta "\x1b[35m"
#define ansi_color_reset "\x1b[0m"

#define ansi_bgcolor_red "\x1b[41m"
#define ansi_bgcolor_yellow "\x1b[43m"
#define ansi_bgcolor_blue "\x1b[44m"
#define ansi_bgcolor_reset "\x1b[49m"

#define ansi_dim "\x1b[2m"
#define ansi_dim_reset "\x1b[22m"
