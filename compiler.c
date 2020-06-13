#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "y.tab.h"

node *root;
static int vi=0, ii=0, wi=0, fi=0;

// Add all T_ID symbols below n to the current symbol table
void add_to_symbol_table(node *n, expr_type et) {
    if (n->nt == T_ID) {
        if (search_symbol(n, n->svalue, 1) != NULL) {
            die("This variable has been declared before! IDIOT!");
        }
        n->et = et;
        push_pointer_array(&n->pcs->st, n);
        // Number variables for assembly generation
        n->vi = vi++;
    } else {
        for (int i = 0; i < n->cn.length; i++) {
            n->cn.nodes[i]->pcs = n->pcs;       // Parent compound statement isn't set yet so low, set it
            add_to_symbol_table(n->cn.nodes[i], et);
        }
    }
}

void assemble(node *n) {
    char buf1[100] = "";

    switch (n->nt) {
        case PROGRAM:
            printf(
                "* PROGRAM %s\n"
                "* TOLERANCE OF FCMP\n"
                "EPSILON\tCON\t0\n"
                "* STACK WITH I1 AS THE TOP POINTER\n"
                "STACK\tORIG\t*+32\n"
                "* PRINTER AND ITS OUTPUT BUFFER\n"
                "LP\tEQU\t18\n"
                "LPLINE\tORIG\t*+24\n"
                "* A TEMPORARY WORD\n"
                "TMP\tCON\t0\n"
                "* CONSTANTS\n"
                "FLTP\tALF\tFLT +\n"
                "FLTN\tALF\tFLT -\n"
                "INTP\tALF\tINT +\n"
                "INTN\tALF\tINT -\n"
                "* VARIABLES\n"
                , n->cn.nodes[0]->svalue);
            for (int i = 0; i < vi; i++)
                printf("V%d\tEQU\t%d\n", i, 2000+i);
            printf(
                "\tORIG\t100\n"
                "MAIN\tENT1\t0\n"
                );
            assemble(n->cn.nodes[1]);
            printf(
                "\tHLT\n"
                "* STANDARD LIBRARY, I2 HOLDS THE RETURN ADDRESS\n"
                "* VOID PRINTI(INT LDA)\n"
                "PRINTI\tSTJ\tTMP\n"
                "\tLD2\tTMP(1:2)\n"
                "\tLDX\tINTP\n"
                "\tJAP\t1F\n"
                "\tLDX\tINTN\n"
                "1H\tSTX\tLPLINE\n"
                "\tCHAR\n"
                "\tSTA\tLPLINE+1\n"
                "\tSTX\tLPLINE+2\n"
                "\tOUT\tLPLINE(LP)\n"
                "\tJBUS\t*(LP)\n"
                "\tJSJ\t0,2\n"
                "* VOID PRINTF(FLOAT LDA)\n"
                "PRINTF\tSTJ\tTMP\n"
                "\tLD2\tTMP(1:2)\n"
                "\tLDX\tFLTP\n"
                "\tJAP\t1F\n"
                "\tLDX\tFLTN\n"
                "1H\tSTX\tLPLINE\n"
                "\tSTA\tTMP\n"
                "\tFIX\n"
                "\tCHAR\n"
                "\tSTA\tLPLINE+1\n"
                "\tSTX\tLPLINE+2\n"
                "\tOUT\tLPLINE(LP)\n"
                "\tJBUS\t*(LP)\n"
                "\tJSJ\t0,2\n"
                "\tEND\tMAIN\n"
                );
            break;
        case DECLARATION:
            break;
        case T_PRINTLN:
            assemble(n->cn.nodes[0]);
            if (n->cn.nodes[0]->et == ET_INT)
                printf("\tJMP\tPRINTI\n");
            else
                printf("\tJMP\tPRINTF\n");
            break;
        case ASSIGN_EXPR:
            assemble(n->cn.nodes[1]);
            printf("\tSTA\tV%d\n", n->cn.nodes[0]->vi);
            break;
        case WHILE_STMT:
            printf("W%dA\tNOP\n", wi);
            assemble(n->cn.nodes[0]);
            printf(
                "\tCMPA\t=1=\n"
                "\tJNE\tW%dB\n"
                , wi);
            assemble(n->cn.nodes[1]);
            printf("\tJMP\tW%dA\n", wi);
            printf("W%dB\tNOP\n", wi);
            wi++;
            break;
        case IF_STMT:
            assemble(n->cn.nodes[0]);  // BOOL_EXPR
            printf(
                "\tCMPA\t=1=\n"
                "\tJNE\tI%dA\n"  // jump to ELSE_PART if BOOL_EXPR is false
                , ii);
            assemble(n->cn.nodes[1]);  // STMT
            printf("\tJMP\tI%dB\n", ii);  // jump to if-end cause 'else' won't be executed
            printf("I%dA\tNOP\n", ii);  // LABEL for ELSE_PART
            assemble(n->cn.nodes[2]);  // ELSE_PART
            printf("I%dB\tNOP\n", ii);  // LABEL for if-end
            ii++;
            break;
        case BOOL_EXPR:
            assemble(n->cn.nodes[0]);
            printf("\tINC1\t1\n");
            printf("\tSTA\tSTACK,1\n");
            assemble(n->cn.nodes[2]);
            printf("\tCMPA\tSTACK,1\n");
            // We do the opposite comparison, 2 op 1
            switch (n->cn.nodes[1]->nt) {
                case T_EQEQ:
                    sprintf(buf1, "JE");
                    break;
                case '<':
                    sprintf(buf1, "JG");
                    break;
                case '>':
                    sprintf(buf1, "JL");
                    break;
                case T_LEQ:
                    sprintf(buf1, "JGE");
                    break;
                case T_GE:
                    sprintf(buf1, "JLE");
                    break;
                case T_NE:
                    sprintf(buf1, "JNE");
                    break;
            }
            printf(
                "\t%s\t1F\n"
                "\tENTA\t0\n"
                "\tJMP\t2F\n"
                "1H\tENTA\t1\n"
                "2H\tNOP\n"
                , buf1);
            break;
        case '+':
            if (!buf1[0])
                sprintf(buf1, "\tADD\tSTACK,1\n");
        case '-':
            if (!buf1[0])
                sprintf(buf1, "\tSUB\tSTACK,1\n");
        case '*':
            if (!buf1[0])
                sprintf(buf1,
                    "\tMUL\tSTACK,1\n"
                    "\tSLC\t5\n");
        case '/':
            if (!buf1[0])
                sprintf(buf1,
                    "\tSLC\t5\n"
                    "\tDIV\tSTACK,1\n");
        case '2':  // All expression nodes with 2 childs
            assemble(n->cn.nodes[0]);
            printf("\tINC1\t1\n");
            printf("\tSTA\tSTACK,1\n");
            assemble(n->cn.nodes[1]);
            printf("%s", buf1);
            printf("\tDEC1\t1\n");
            break;
        case T_ID:
            printf("\tLDA\tV%d\n", n->vi);
            break;
        case T_NUM:
            if (n->et == ET_INT)
                printf("\tLDA\t=%d=\n", n->ivalue);
            else
                printf("\tLDA\t=%f=\n", n->fvalue);
            break;
        default:
            for (int i = 0; i < n->cn.length; i++)
                assemble(n->cn.nodes[i]);
    }
}

node *create_node(node_type nt, node *c0, node *c1, node *c2, node *c3) {
    node *n;

    n = (node *)malloc(sizeof(node));
    memset(n, 0, sizeof(node));
    n->nt = nt;
    if (c0) push_pointer_array(&n->cn, c0);
    if (c1) push_pointer_array(&n->cn, c1);
    if (c2) push_pointer_array(&n->cn, c2);
    if (c3) push_pointer_array(&n->cn, c3);

    return n;
}

node *create_num_node(char *str) {
    node *n;
    int i;
    float f;

    n = create_node(T_NUM, NULL, NULL, NULL, NULL);
    if (strstr(str, ".") == NULL) {
        n->et = ET_INT;
        n->ivalue = atoi(str);
    } else {
        n->et = ET_FLOAT;
        n->fvalue = atof(str);
    }

    return n;
}

node *create_str_node(char *str) {
    node *n;

    n = create_node(T_ID, NULL, NULL, NULL, NULL);
    n->svalue = strdup(str);

    return n;
}

int debugl(int i, char *s) {
    //printf("[%d,%s]", i, s);
}

void die(char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

char *et_str(expr_type et)
{
    static char *expr_type_strs[]={"none", "int", "float"};

    assert(et >= ET_NONE && et <= ET_FLOAT);

    return expr_type_strs[et];
}

char *nt_str(int nt)
{
    static char *token_type_strs[]={"T_ID", "T_NUM", "T_MAINCLASS",
        "T_PUBLIC", "T_STATIC", "T_VOID", "T_MAIN", "T_INT", "T_FLOAT",
        "T_PRINTLN", "T_ELSE", "T_FOR", "T_IF", "T_WHILE", "T_EQEQ",
        "T_LEQ", "T_NE", "T_GE", "UMINUS"};
    static char *node_type_strs[]={"PROGRAM", "COMP_STMT", "STMT_LIST",
    "STMT", "DECLARATION", "TYPE", "ID_LIST", "NULL_STMT", "ASSIGN_STMT",
    "ASSIGN_EXPR", "EXPR", "FOR_STMT", "OPASSIGN_EXPR", "OPBOOL_EXPR",
    "WHILE_STMT", "IF_STMT", "ELSE_PART", "BOOL_EXPR", "C_OP", "RVAL",
    "TERM", "FACTOR"};
    static char result[10];

    if (nt >= T_ID && nt <= UMINUS)
        return token_type_strs[nt - T_ID];
    else if (nt >= PROGRAM && nt <= FACTOR)
        return node_type_strs[nt - PROGRAM];
    else if (nt >= 32 && nt < 128)
        sprintf(result, "%c", nt);
    else
        sprintf(result, "%d", nt);

    return result;
}

void *pop_pointer_array(pointer_array *pa) {
    assert(pa != NULL);
    assert(pa->length > 0);
    return pa->nodes[--pa->length];
}

void print_tree(node *n, int depth) {
    int i;

    if (depth == 0)
        printf(
            "* --------------------\n"
            "* ABSTRACT SYNTAX TREE\n"
            "* --------------------\n"
            );
    if (n == NULL)
        return;
    printf("* ");
    for (i = 0; i < depth; i++)
        printf("  ");
    printf("%s(%s): ", nt_str(n->nt), et_str(n->et));
    switch (n->nt) {
        case PROGRAM:
            printf("%s\n", n->cn.nodes[0]->svalue);
            break;
        case COMP_STMT:
            printf("st=(");
            if (n->st.length > 0)
                printf("%s %s", et_str(n->st.nodes[0]->et), n->st.nodes[0]->svalue);
            for (int i = 1; i < n->st.length; i++)
                printf(" ,%s %s", et_str(n->st.nodes[i]->et), n->st.nodes[i]->svalue);
            printf(")\n");
            break;
        case '+':
        case '-':
        case '*':
        case '/':
            printf("\n");
            break;
        case UMINUS:
            printf("\n");
            break;
        case T_ID:
            printf("%s,V%d\n", n->svalue, n->vi);
            break;
        case T_NUM:
            if (n->et == ET_INT)
                printf("%d\n", n->ivalue);
            else
                printf("%f\n", n->fvalue);
            break;
        default:
            printf("\n");
    };
    for (i = 0; i < n->cn.length; i++)
        print_tree(n->cn.nodes[i], depth+1);
}

void push_pointer_array(pointer_array *pa, void *p) {
    assert(pa != NULL);
    if (pa->length == pa->size) {
        pa->size = 2*pa->size + 4;
        pa->nodes = realloc(pa->nodes, pa->size*sizeof(void *));
        assert(pa->nodes != NULL);
    }
    pa->nodes[pa->length++] = p;
}

node *search_symbol(node *n, char *s, char local) {
    pointer_array *st;

    if (n->nt == COMP_STMT)
        st = &n->st;
    else
        st = &n->pcs->st;

    for (int i = 0; i < st->length; i++) {
        if (strcmp(s, st->nodes[i]->svalue) == 0) {
            return st->nodes[i];
        }
    }
    if (local || n->pcs == NULL)
        return NULL;
    else
        return search_symbol(n->pcs, s, local);
}

void semantic_bottom_up(node *n) {
    int i;

    for (i = 0; i < n->cn.length; i++)
        semantic_bottom_up(n->cn.nodes[i]);
    switch (n->nt) {
        case ASSIGN_EXPR:
            if (n->cn.nodes[0]->et == ET_INT && n->cn.nodes[1]->et == ET_FLOAT)
                die("Can't assign float to int! UwU");
            n->et = n->cn.nodes[0]->et;
            break;
        case '+':
        case '-':
        case '*':
        case '/':
            if (n->cn.nodes[0]->et == ET_INT && n->cn.nodes[1]->et == ET_INT)
                n->et = ET_INT;
            else
                n->et = ET_FLOAT;
            break;
        case UMINUS:
            n->et = n->cn.nodes[0]->et;
            break;
    }
}

void semantic_top_down(node *n) {
    int i;
    expr_type et;

    if (n == NULL)
        return;
    for (i = 0; i < n->cn.length; i++) {
        n->cn.nodes[i]->pn = n;
        if (n->nt == COMP_STMT)
            n->cn.nodes[i]->pcs = n;
        else
            n->cn.nodes[i]->pcs = n->pcs;
    }
    switch (n->nt) {
        case COMP_STMT:
            memset(&n->st, 0, sizeof(n->st));
            break;
        case DECLARATION:
            if (n->cn.nodes[0]->nt == T_INT)
                et = ET_INT;
            else {
                assert(n->cn.nodes[0]->nt == T_FLOAT);
                et = ET_FLOAT;
            }
            add_to_symbol_table(n->cn.nodes[1], et);
            break;
        case T_ID:
            if (n->pcs != NULL) {    // Don't process program name
                n->ps = search_symbol(n, n->svalue, 0);
                n->et = n->ps->et;
                n->vi = n->ps->vi;
            }
            break;
    };
    for (i = 0; i < n->cn.length; i++)
        semantic_top_down(n->cn.nodes[i]);
}

void yyerror(const char *msg)
{
    fprintf(stderr, "McChris language error: %s\n", msg);
    exit(1);
}

int yywrap(void) {
    return 1;
}

int main ()
{
    yyparse();
    semantic_top_down(root);
    semantic_bottom_up(root);
    assemble(root);
    print_tree(root, 0);
}
