#include <assert.h>
#include <math.h>
#include <stdarg.h>
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
        if (search_symbol(n, n->svalue, 1) != NULL)
            die("Variable %s has been declared before! IDIOT!", n->svalue);
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
    int i, myfi;
    char buf1[100] = "";

    assert(n != NULL);
    switch (n->nt) {
        case PROGRAM:
            printf(
                "* PROGRAM %s\n"
                "EPSILON\tCON\t0\tTOLERANCE OF FCMP\n"
                "* VARIABLES\n"
                , n->cn.nodes[0]->svalue);
            for (i = 0; i < vi; i++)
                printf("V%d\tCON\t0\n", i);
            printf(
                "LP\tEQU\t18\tLINE PRINTER\n"
                "LPLINE\tORIG\t*+24\tITS OUTPUT BUFFER\n"
                "STACK\tORIG\t*+32\tSTACK, I1=TOP\n"
                "* STRINGS\n"
                "FLTP\tALF\tFLT +\n"
                "FLTN\tALF\tFLT -\n"
                "INTP\tALF\tINT +\n"
                "INTN\tALF\tINT -\n"
                "OVFL\tALF\t*OVER\n"
                "\tALF\tFLOW*\n"
                "* DEFINES\n"
                "* ALWAYS USE SVx,1 NEVER SVx:\n"
                "SV1\tEQU\tSTACK+1\tSTACK VARIABLE 1\n"
                "SV2\tEQU\tSTACK+2\tSTACK VARIABLE 2\n"
                "SV3\tEQU\tSTACK+3\tSTACK VARIABLE 3\n"
                "SV4\tEQU\tSTACK+4\tSTACK VARIABLE 4\n"
                "MAIN\tENT1\t0\tSTACK POINTER=0\n"
                );
            assemble(n->cn.nodes[1]);
            printf(
                "\tHLT\n"
                "* STANDARD LIBRARY\n"
                "* J AND I6 ARE USED FOR CALL/RETURN\n"
                "* VOID PRINTI(INT A)\n"
                "PRINTI\tSTJ\tSV1,1\n"  // Store return address
                "\tLDX\tINTP\tSHOW INT +\n"
                "\tJANN\t1F\n"  // Jump if A<0
                "\tLDX\tINTN\tSHOW INT -\n"
                "1H\tSTX\tLPLINE\n"
                "\tCHAR\n"
                "\tSTA\tLPLINE+1\n"
                "\tSTX\tLPLINE+2\n"
                "\tSTZ\tLPLINE+3\n"
                "\tOUT\tLPLINE(LP)\n"
                "\tJBUS\t*(LP)\n"
                "\tLD6\tSV1,1(1:2)\n"
                "\tJSJ\t0,6\n"

                "* VOID PRINTF(FLOAT A)\n"
                "PRINTF\tSTJ\tSV1,1\n"
                "\tLDX\tFLTP\tSHOW TYPE AND SIGN\n"
                "\tJANN\t1F\n"
                "\tLDX\tFLTN\n"
                "1H\tSTX\tLPLINE\n"
                // Normally we'd use STA SV2,1(0:5) to store ABS(A),
                // but MIXBuilder has a bug with STA and negatives
                // and byte ranges, and stores 6363636363-A instead
                "\tSTA\tSV2,1\tA=ABS(A)\n"
                "\tLDA\tSV2,1(1:5)\tOMIT SIGN\n"
                "\tSTA\tSV2,1\tSV2=ABS(A)\n"
                "\tFIX\n"
                "\tSTA\tSV3,1\tSV3=ROUND(A)\n"
                "\tLDX\tSV3,1\tX=SV3\n"
                "\tFLOT\n"
                "\tFSUB\tSV2,1\tA=SV3-SV2\n"
                "\tJAN\t1F\tNO JUMP IF ROUND(A)>A\n"
                "\tDECX\t1\n"
                "\tSTX\tSV3,1\tSV3=SV3-1\n"
                "1H\tLDA\tSV3,1\n"
                "\tCHAR\n"
                "\tSTA\tLPLINE+1\n"
                "\tSTX\tLPLINE+2\n"
                "* NOW FOR THE FRACTIONAL PART\n"
                "\tLDAN\tSV3,1\n"
                "\tFLOT\n"
                "\tFADD\tSV2,1\tNOW A=FRACTION\n"
                "\tSTA\tSV2,1\n"
                "\tLDA\t=10000=\n"
                "\tFLOT\n"
                "\tFMUL\tSV2,1\n"
                "\tFIX\n"
                "\tCMPA\t=9999=\n"
                "\tJLE\t1F\n"
                "\tLDA\t=9999=\n"
                "1H\tCHAR\n"
                "\tSTX\tLPLINE+3\n"
                "\tLDA\t=40=\n"
                "\tSTA\tLPLINE+3(1:1)\n"
                "\tOUT\tLPLINE(LP)\n"
                "\tJBUS\t*(LP)\n"
                "\tLD6\tSV1,1(1:2)\n"
                "\tJSJ\t0,6\n"

                "* VOID OVERFLOW(T_DIV)\n"
                "OVERFLOW\tNOP\n"
                // We are here because of the mixal instruction JOV, means overflow happened
                "\tLDA\tOVFL\n"
                "\tSTA\tLPLINE\n"
                "\tLDA\tOVFL+1\n"
                "\tSTA\tLPLINE+1\n"
                "\tSTZ\tLPLINE+2\n"
                "\tSTZ\tLPLINE+3\n"
                "\tOUT\tLPLINE(LP)\n"
                "\tJBUS\t*(LP)\n"
                "\tHLT\n"
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
        case FOR_STMT:
            myfi=fi++;
        //if there are no optional assignment/bool/update expressions it will be an endless-loop
            if (n->cn.nodes[0] != NULL)
                assemble(n->cn.nodes[0]); //optional assignment
            printf("F%dA\tNOP\n", myfi); //LABEL to return to
            if (n->cn.nodes[1] != NULL) {
                assemble(n->cn.nodes[1]); //optional bool expr
                printf(
                    "\tCMPA\t=1=\n"
                    "\tJNE\tF%dB\n" //end-loop
                    , myfi);
            }
            assemble(n->cn.nodes[3]); // statement
            if (n->cn.nodes[2] != NULL) {
                assemble(n->cn.nodes[2]); //optional loop-update expr
            }
            printf("\tJMP\tF%dA\n", myfi); //go back to bool expr
            printf("F%dB\tNOP\n", myfi); //end-loop LABEL
            break;
        case BOOL_EXPR:
            assemble(n->cn.nodes[0]);
            if ((n->cn.nodes[0]->et == ET_INT && n->cn.nodes[2]->et == ET_FLOAT))
                // CONVERT left child TO FLOAT
                printf("\tFLOT\n");
            printf("\tINC1\t1\n");
            printf("\tSTA\tSTACK,1\n");
            assemble(n->cn.nodes[2]);
            if ((n->cn.nodes[2]->et == ET_INT && n->cn.nodes[0]->et == ET_FLOAT))
                // CONVERT right child TO FLOAT
                printf("\tFLOT\n");
            printf("\tDEC1\t1\n");
            if ((n->cn.nodes[0]->et == ET_FLOAT || n->cn.nodes[2]->et == ET_FLOAT))
                printf("\tFCMP\tSTACK+1,1\n");
            else
                printf("\tCMPA\tSTACK+1,1\n");
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
                default:
                    die("Unknown boolean operator %d", n->cn.nodes[1]->nt);
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
                if (n->cn.nodes[0]->et == ET_FLOAT || n->cn.nodes[1]->et == ET_FLOAT)
                    sprintf(buf1, "\tFADD\tSTACK,1\n");
                else
                    sprintf(buf1, "\tADD\tSTACK,1\n");
        case '-':
            if (!buf1[0])
                if (n->cn.nodes[0]->et == ET_FLOAT || n->cn.nodes[1]->et == ET_FLOAT)
                    sprintf(buf1, "\tFSUB\tSTACK,1\n");
                else
                    sprintf(buf1, "\tSUB\tSTACK,1\n");
        case '*':
            if (!buf1[0])
                if (n->cn.nodes[0]->et == ET_FLOAT || n->cn.nodes[1]->et == ET_FLOAT)
                    sprintf(buf1,
                        "\tFMUL\tSTACK,1\n");
                else
                    sprintf(buf1,
                        "\tMUL\tSTACK,1\n"
                        "\tSLC\t5\n");
        case '/':
            if (!buf1[0])
                if (n->cn.nodes[0]->et == ET_FLOAT || n->cn.nodes[1]->et == ET_FLOAT)
                    sprintf(buf1,
                        "\tFDIV\tSTACK,1\n");
                else
                    sprintf(buf1,
                        "\tSLC\t5\n"
                        "\tENTA\t0\t5\n"
                        "\tDIV\tSTACK,1\n");
        case '2':  // All expression nodes with 2 childs
            assemble(n->cn.nodes[1]);
            if ((n->cn.nodes[1]->et == ET_INT && n->cn.nodes[0]->et == ET_FLOAT))
                // CONVERT right child TO FLOAT
                printf("\tFLOT\n");
            printf("\tINC1\t1\n");
            printf("\tSTA\tSTACK,1\n");
            assemble(n->cn.nodes[0]);
            if ((n->cn.nodes[0]->et == ET_INT && n->cn.nodes[1]->et == ET_FLOAT))
                // CONVERT left child TO FLOAT
                printf("\tFLOT\n");
            printf("%s", buf1);
            printf("\tJOV\tOVERFLOW\n");
            printf("\tDEC1\t1\n");
            break;
        case UMINUS:
            assemble(n->cn.nodes[0]);
            printf("\tSTA\tSV1,1\n");
            printf("\tLDAN\tSV1,1\n");
            break;
        case T_ID:
            printf("\tLDA\tV%d\n", n->vi);
            break;
        case T_NUM:
            if (n->et == ET_FLOAT) {
                i = float_as_int(n->fvalue);
                printf("* Float literal: %f\n", n->fvalue);
            }
            else
                i = n->ivalue;
            if (i >= 0)
                printf("\tLDA\t=%d=\n", i);
            else
                printf("\tLDAN\t=%d=\n", -i);
        default:
            for (i = 0; i < n->cn.length; i++)
                assemble(n->cn.nodes[i]);
    }
}

void debuuug(char *s) {
    fprintf(stderr, "DEBUUUG: %s", s);
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
    // fprintf(stderr, "[%d,%s]", i, s);
}

void die(char *format, ...) {
    va_list args;
    char buf[1000];

    va_start(args, format);
    sprintf(buf, "%s\n", format);
    vfprintf(stderr, buf, args);
    va_end(args);

    exit(1);
}

char *et_str(expr_type et)
{
    static char *expr_type_strs[]={"none", "int", "float"};

    assert(et >= ET_NONE && et <= ET_FLOAT);

    return expr_type_strs[et];
}

// Return an integer with the same binary representation in MIX as f.
// This is because no MIX tools allow floats in CONstants or =LITERALS=.
int float_as_int(float f) {
    int sign, exponent;
    float logbase64, fraction;

    if (f == 0)
        return 0;
    else if (f > 0)
        sign = 1;
    else {
        sign = -1;
        f = -f;
    }
    logbase64 = log10(f) / log10(64);
    // Is it an integer power?
    if (floor(logbase64) == ceil(logbase64))
        exponent = ceil(logbase64 + 1);
    else
        exponent = ceil(logbase64);
    fraction = f / pow(64, exponent);
    // Add EXPBIAS and check the value
    exponent = exponent + 32;
    if (exponent < 0 || exponent >= 64)
        die("Exponent out of bounds: %d", exponent);
    if (fraction < 1.0/64.0 || fraction >= 1)
        die("Fraction out of bounds: %f", fraction);

    // 16777216 = 64^3 = FRACBIAS
    return sign*16777216*exponent + round(16777216*fraction);
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
                die("Can't assign float to int %s! UwU", n->cn.nodes[0]->svalue);
            n->et = n->cn.nodes[0]->et;
            break;
        case '+':
        case '-':
        case '*':
        case '/':
            if (n->cn.nodes[0]->et == ET_FLOAT || n->cn.nodes[1]->et == ET_FLOAT)
                n->et = ET_FLOAT;
            else
                n->et = ET_INT;
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
    die("McChris language error: %s\n", msg);
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
