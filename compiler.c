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
    n->nt = nt;
    n->et = ET_NONE;
    n->children[0] = c0;
    n->children[1] = c1;
    n->children[2] = c2;
    n->children[3] = c3;
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

node *search_in_symbol_table(pointer_array *pa, char *s) {
    for (int i = 0; i < pa->length; i++) {
        if (strcmp(s, pa->nodes[i]->svalue) == 0) {
            return pa->nodes[i];
        }
    }
    return NULL;
}

void add_to_symbol_table(pointer_array *pa, expr_type et, node *n) {
    if (n->nt == T_ID) {
        if (search_in_symbol_table(pa, n->svalue) != NULL) {
            die("This variable has been declared before! IDIOT!");
        }
        n->et = et;
        push_pointer_array(pa, n);
    } else {
        for (int i = 0; i < 4; i++)
            if (n->children[i] != NULL)
                add_to_symbol_table(pa, et, n->children[i]);
    }
}

void semantic_analysis(node *n) {
    int i;
    expr_type et;
    node *pcs;      // Parent compount statement

    if (n == NULL)
        return;
    for (i = 0; i < 4; i++)
        if (n->children[i] != NULL)
            n->children[i]->parent = n;
    switch (n->nt) {
        case COMP_STMT:
            memset(&n->st, 0, sizeof(n->st));
            break;
        case DECLARATION:
            pcs = n->parent;
            while (pcs->nt != COMP_STMT) {
                pcs = pcs->parent;
            };
            if (n->children[0]->nt == T_INT)
                et = ET_INT;
            else {
                assert(n->children[0]->nt == T_FLOAT);
                et = ET_FLOAT;
            }
            add_to_symbol_table(&pcs->st, et, n->children[1]);
            break;
/*        case T_ID:
            comp = search_parent(type=COMP_STMT);
            n->symbol = search_in_symbol_table(&pcs->st, n->svalue);
            break;*/
    };
    for (i = 0; i < 4; i++)
        semantic_analysis(n->children[i]);
}

void print_tree(node *n, int depth) {
    int i;

    if (n == NULL)
        return;
    printf("%*s", 2*depth, "");
    switch (n->nt) {
        case PROGRAM:
            printf("mainclass %s\n", n->children[0]->svalue);
            break;
        case COMP_STMT:
            // COMP_STMT, st=(int a, int k, float x, int y)
            printf("COMPOUNT STATEMENT, st=(");
            for (int i = 0; i < n->st.length; i++) {
                printf("%s %s, ", n->st.nodes[i]->et, n->st.nodes[i]->svalue);
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
            printf("id: %s\n", n->svalue);
            break;
        default:
            if (n->nt >= 32 && n->nt < 128)
                printf("Node %c\n", n->nt);
            else
                printf("Node %d\n", n->nt);
    };
    for (i = 0; i < 4; i++) {
        print_tree(n->children[i], depth+1);
    }
}
