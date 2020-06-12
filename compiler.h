#ifndef COMPILER_H
#define COMPILER_H

typedef enum {ET_NONE, ET_INT, ET_FLOAT} expr_type;
typedef enum {PROGRAM=300, COMP_STMT, STMT_LIST, STMT, DECLARATION,
 TYPE, ID_LIST, NULL_STMT, ASSIGN_STMT, ASSIGN_EXPR, EXPR, FOR_STMT,
 OPASSIGN_EXPR, OPBOOL_EXPR, WHILE_STMT, IF_STMT, ELSE_PART, BOOL_EXPR,
 C_OP, RVAL, TERM, FACTOR} node_type;

typedef struct struct_pointer_array {
    int size, length;
    struct struct_node **nodes;    // Pointer to array of pointers
} pointer_array;

typedef struct struct_node {
    node_type nt;
    expr_type et;
    union {
        int ivalue;                // When nt=T_NUM, et=ET_INT
        float fvalue;              // When nt=T_NUM, ET_FLOAT
        char *svalue;              // When nt=T_ID
        pointer_array st;          // When nt=COMP_STMT; symbol table
    };
    int vi;                        // When nt=T_ID, variable index
    struct struct_node *pn, *pcs;  // Parent node, parent compound statement
    struct struct_node *ps;        // Parent symbol; if NULL, it can't be LVAL
    pointer_array cn;              // child nodes
} node;

void add_to_symbol_table(node *n, expr_type et);
void assemble(node *n);
node *create_node(node_type nt, node *c0, node *c1, node *c2, node *c3);
node *create_num_node(char *str);
node *create_str_node(char *str);
int debugl(int i, char *s);
void die(char *msg);
char *et_str(expr_type et);
char *nt_str(int nt);
void *pop_pointer_array(pointer_array *pa);
void print_tree(node *n, int depth);
void push_pointer_array(pointer_array *pa, void *p);
node *search_symbol(node *n, char *s, char local);
void semantic_bottom_up(node *n);
void semantic_top_down(node *n);
void yyerror(const char *msg);
int yylex(void);
int yyparse(void);
int yywrap(void);

extern node *root;

#endif
