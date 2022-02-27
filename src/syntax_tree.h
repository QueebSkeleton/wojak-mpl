#ifndef STDLIB_H
#define STDLIB_H
#include "stdlib.h"
#endif

#ifndef STRING_H
#define STRING_H
#include "string.h"
#endif

typedef enum node_type {
    PROG, STMTS, STMT,

    PRIM_DECL_STMT,

    LIST_DECL_STMT, LIST_ELEMS_NODE, LIST_ELEM_NODE,

    ASSIGN_STMT,

    IN_STMT,

    OUT_STMT,

    DECISION_STMT, IF_STMT, ELIF_STMTS, ELIF_STMT, ELSE_STMT,

    SWITCH_STMT, CASES, CASE_, REGULAR_CASE, DEFAULT_CASE,
    EVAL, CONST,

    WHILE_STMT,

    BREAK_STMT, CONTINUE_STMT,

    LOGIC_OR_NODE,
    LOGIC_AND_NODE,
    LOGIC_NOT_NODE,

    BITWISE_OR_NODE,
    BITWISE_XOR_NODE,
    BITWISE_NOT_NODE,
    BITWISE_AND_NODE,
    BITWISE_RIGHT_NODE,
    BITWISE_LEFT_NODE,

    RELATIONAL_EQ_NODE,
    RELATIONAL_NOT_EQ_NODE,
    RELATIONAL_GT_NODE,
    RELATIONAL_GT_EQ_NODE,
    RELATIONAL_LT_NODE,
    RELATIONAL_LT_EQ_NODE,

    ADD_NODE,
    SUB_NODE,
    MUL_NODE,
    DIV_NODE,
    FLOOR_NODE,
    MOD_NODE,

    UNARY_ADD_NODE,
    UNARY_SUB_NODE,

    SENTINEL_EXPR_NODE,

    IDENTIFIER_NODE,
    IDENTIFIER_LIST_NODE,
    EXPR_NODE,
    EXPR_LIST_NODE,

    STR_LITERAL_NODE,
    INT_LITERAL_NODE,
    FLOAT_LITERAL_NODE,
    TRUE_KW_NODE,
    FALSE_KW_NODE,


    TYPE
} node_type;

struct tree_node {
    node_type type;
    struct tree_node *children_head;
    struct tree_node *next_sibling;
    char value[255];
};

typedef struct tree_node tree_node;

struct tree_node* create_node(node_type type, char *value) {
    // allocate new node
    struct tree_node *new_node =
        (struct tree_node*) malloc(sizeof(struct tree_node));
    new_node -> type = type;
    if(value != NULL) strcpy(new_node -> value, value);
    new_node -> children_head = NULL;
    new_node -> next_sibling = NULL;
    return new_node;
}

struct tree_node* add_ready_to_parent(tree_node *parent, tree_node *child) {
    if(parent == NULL) return NULL;
    // search parent for the last child in list
    if(parent -> children_head == NULL)
        parent -> children_head = child;
    // traverse for the last child
    else {
        struct tree_node *current = parent -> children_head;
        while(current -> next_sibling != NULL)
            current = current -> next_sibling;
        current -> next_sibling = child;
    }
    return child;
}

struct tree_node* add_node_to_parent(tree_node *parent, node_type type, char *value) {
    struct tree_node *new_node = create_node(type, value);
    return add_ready_to_parent(parent, new_node);
}

void destroy_node(tree_node *to_destroy) {
    if(to_destroy == NULL) return;

    // First loop on all children
    if(to_destroy -> children_head != NULL) {
        struct tree_node *current = to_destroy -> children_head;
        struct tree_node *next = NULL;

        while(current != NULL) {
            next = current -> next_sibling;
            destroy_node(current);
            current = next;
        }
    }

    // Then destroy itself
    free(to_destroy);
}

struct expr_stack_node {
    struct tree_node *actual_node;
    struct expr_stack_node *next;
};
typedef struct expr_stack_node expr_stack_node;

struct expr_stack {
    struct expr_stack_node *top;
    uint32_t members;
};
typedef struct expr_stack expr_stack;

struct expr_stack* create_expr_stack() {
    struct expr_stack *new_stack =
        (struct expr_stack *) malloc(sizeof(expr_stack));
    new_stack -> top = NULL;
    new_stack -> members = 0;
    return new_stack;
}

void expr_stack_push(struct expr_stack *stack, struct tree_node *value) {
    if(value == NULL) return;

    // Allocate for a new node
    struct expr_stack_node *new_node =
        (struct expr_stack_node *) malloc(sizeof(struct expr_stack_node));
    new_node -> actual_node = value;
    new_node -> next = NULL;

    // If empty, make this top
    if(stack -> top == NULL)
        stack -> top = new_node;
    
    // else, replace the current top
    else {
        new_node -> next = stack -> top;
        stack -> top = new_node;
    }

    // increment size
    stack -> members++;
}

struct tree_node* expr_stack_pop(struct expr_stack *stack) {
    if(stack == NULL) return NULL;

    if(stack -> top != NULL) {
        struct expr_stack_node *current_top = stack -> top;
        stack -> top = current_top -> next;
        stack -> members--;
        return current_top -> actual_node;
    } else return NULL;
}

struct tree_node* expr_stack_peek(struct expr_stack *stack) {
    if(stack == NULL) return NULL;
    return stack -> top -> actual_node;
}