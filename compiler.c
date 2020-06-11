#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "y.tab.h"

void push_pointer_array(pointer_array *pa, void *p) {
    assert(pa != NULL);
    if (pa->length == pa->size) {
        pa->size = 2*pa->size + 4;
        pa->nodes = realloc(pa->nodes, pa->size*sizeof(void *));
        assert(pa->nodes != NULL);
    }
    pa->nodes[pa->length++] = p;
}

void *pop_pointer_array(pointer_array *pa) {
    assert(pa != NULL);
    assert(pa->length > 0);
    return pa->nodes[--pa->length];
}

node *create_node(node_type nt, node *c0, node *c1, node *c2, node *c3) {
    node *n;

    // printf("Creating node (%d, %p, %p, %p, %p)\n", nt, c0, c1, c2, c3);
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
}

void die(char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(1);
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

// Add all T_ID symbols below n to the current symbol table
void add_to_symbol_table(node *n, expr_type et) {
    if (n->nt == T_ID) {
        if (search_symbol(n, n->svalue, 1) != NULL) {
            die("This variable has been declared before! IDIOT!");
        }
        n->et = et;
        push_pointer_array(&n->pcs->st, n);
    } else {
        for (int i = 0; i < n->cn.length; i++) {
            n->cn.nodes[i]->pcs = n->pcs;       // Parent compount statement isn't set yet so low, set it
            add_to_symbol_table(n->cn.nodes[i], et);
        }
    }
}

void semantic_analysis(node *n) {
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
            }

            break;
    };
    for (i = 0; i < n->cn.length; i++)
        semantic_analysis(n->cn.nodes[i]);
}

void print_tree(node *n, int depth) {
    int i;

    if (n == NULL)
        return;
    printf("%*s", 2*depth, "");
    switch (n->nt) {
        case PROGRAM:
            printf("mainclass %s\n", n->cn.nodes[0]->svalue);
            break;
        case COMP_STMT:
            printf("COMPOUNT STATEMENT, st=(");
            for (int i = 0; i < n->st.length; i++) {
                printf("%d %s, ", n->st.nodes[i]->et, n->st.nodes[i]->svalue);
            }
            printf(")\n");
            break;
        case T_NUM:
            if (n->et == ET_INT)
                printf("const-int: %d\n", n->ivalue);
            else
                printf("const-float: %f\n", n->fvalue);
            break;
        case T_ID:
            printf("id: %s,%d,%p\n", n->svalue, n->et, n->ps);
            break;
        default:
            if (n->nt >= 32 && n->nt < 128)
                printf("Node %c\n", n->nt);
            else
                printf("Node %d\n", n->nt);
    };
    for (i = 0; i < n->cn.length; i++) {
        print_tree(n->cn.nodes[i], depth+1);
    }
}
