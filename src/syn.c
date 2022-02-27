// TODO: Increment error count when no list elements are specified for a list declaration
// TODO: Implement rest of operators, including unary

#ifndef GLOBALS_H
#define GLOBALS_H
#include "globals.h"
#endif

#ifndef TOKENS_H
#define TOKENS_H
#include "tokens.h"
#endif

#ifndef TOKEN_LIST_H
#define TOKEN_LIST_H
#include "token_list.h"
#endif

#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H
#include "syntax_tree.h"
#endif

#ifndef STDIO_H
#define STDIO_H
#include <stdio.h>
#endif

// Prototypes
void syn_init();
void parse_tokens_into_list();
void get_next_token();
void get_prev_token();
void write_syntax_file();

void flatten_syntax_tree(tree_node*);
void print_expr(tree_node*);
void print_expr_child(tree_node*);

// Process Productions
bool prog();
bool stmts(tree_node*);
bool stmt(tree_node*);
bool prim_decl_stmt(tree_node*);
bool list_decl_stmt(tree_node*);
bool list_elems(tree_node*);
bool list_elem(tree_node*);
bool assign_stmt(tree_node*);
bool in_stmt(tree_node*);
bool out_stmt(tree_node*);
bool decision_stmt(tree_node*);
bool if_stmt(tree_node*);
bool elif_stmts(tree_node*);
bool elif_stmt(tree_node*);
bool else_stmt(tree_node*);
bool switch_stmt(tree_node*);
bool cases(tree_node*);
bool case_(tree_node*);
bool regular_case(tree_node*);
bool default_case(tree_node*);
bool while_stmt(tree_node*);
bool break_stmt(tree_node*);
bool continue_stmt(tree_node*);

bool parse_expr(tree_node*);
bool e(expr_stack*, expr_stack*);
bool p(expr_stack*, expr_stack*);
void pop_operator(expr_stack*, expr_stack*);
void push_operator(tree_node*, expr_stack*, expr_stack*);
bool compare_operators(node_type, node_type);
int8_t precedence_level(node_type);
bool is_recognizable_as_binary(int8_t);
bool is_recognizable_as_unary(int8_t);
tree_node *trasnform_into_binary_node(int8_t);
bool is_unary(node_type);
bool is_left_associative(node_type);

bool identifier_list(tree_node*);

bool optional_expr_list(tree_node*);
bool expr_list(tree_node*);

bool parse_token(int8_t);
bool expect_token(int8_t, char*);
bool parse_identifier(char*);
bool parse_constant(char*);
bool parse_type(int8_t*, char*);

/**
 * @brief Number of errors encountered in syntax phase.
  */
uint8_t syn_error_count;

// Globals for Syntax Analysis
// The input symbol table file
FILE *sym_file;
// The output syntax tree file
FILE *syn_tree_file;
// Placeholder for all tokens in LinkedList
token_list *all_tokens;
// Placeholder for current token
token_list_item *curr_token;
// Root of the syntax tree
tree_node *root;

char *curr_lexeme;
int8_t curr_token_rep;

void start_syn() {
    // Attempt to open symbol table file in read mode
    if((sym_file = fopen(filename_symboltable, "r")) == NULL) {
        printf("Error: the symbol table file does not exist.\n");
        exit(EXIT_FAILURE);
    }

    // Parse all tokens
    parse_tokens_into_list();
    // Start at sentinel intentionally placed
    curr_token = all_tokens -> head -> next;
    syn_error_count = 0;

    // Start parsing
    prog();

    // If no errors, output syntax tree
    if(!syn_error_count) {
        write_syntax_file();
    }

    // Close files used by syntax analysis phase
    fclose(sym_file);
}

void parse_tokens_into_list() {
    // Initialize list of tokens
    all_tokens = construct_token_list();
    // Extract all tokens, put into token list
    size_t len = 0;
    char *line = NULL;
    ssize_t read;
    add_token_to_list(all_tokens, "SENTINEL", -2);
    while((read = getline(&line, &len, sym_file)) != -1) {
        char *lexeme = strtok(line, ";");
        int8_t token_rep = atoi(strtok(NULL, ";"));
        add_token_to_list(all_tokens, lexeme, token_rep);
    }
}

void write_syntax_file() {
    strcat(filename_syntaxtree, separated_name_extension[0]);
    strcat(filename_syntaxtree, ".wojaktree");

    // Attempt to open syntax tree file in read mode
    if((syn_tree_file = fopen(filename_syntaxtree, "w")) == NULL) {
        printf("Error: cannot create syntax tree file with name %s."
               "Please make sure that the compiler has proper access.",
               filename_syntaxtree);
        exit(EXIT_FAILURE);
    }

    // Write to the syntax tree file
    flatten_syntax_tree(root);

    // Close file
    fclose(syn_tree_file);
}

bool prog() {
    root = create_node(PROG, "PROG");
    return stmts(root);
}

bool stmts(tree_node* parent) {
    tree_node *stmts_node = create_node(STMTS, "STMTS");
    token_list_item *before = curr_token;

    bool success = stmt(stmts_node);

    if(success) {
        add_ready_to_parent(parent, stmts_node);
        stmts(stmts_node);
    } else {
        destroy_node(stmts_node);
        curr_token = before -> prev;
        get_next_token();
    }

    return success;
}

bool stmt(tree_node* parent) {
    // if eof, stop
    get_next_token();
    if(curr_token_rep == EOF) return false;
    get_prev_token();

    tree_node *stmt_node = create_node(STMT, "STMT");
    token_list_item *before = curr_token;

    bool success =
        prim_decl_stmt(stmt_node) ||
        list_decl_stmt(stmt_node) ||
        assign_stmt(stmt_node) ||
        in_stmt(stmt_node) ||
        out_stmt(stmt_node) ||
        decision_stmt(stmt_node) ||
        switch_stmt(stmt_node) ||
        while_stmt(stmt_node) ||
        break_stmt(stmt_node) ||
        continue_stmt(stmt_node);

    if(success) {
        add_ready_to_parent(parent, stmt_node);
    } else {
        destroy_node(stmt_node);
        curr_token = before -> prev;
        get_next_token();
    }

    return success;
}

bool prim_decl_stmt(tree_node* parent) {
    tree_node *prim_decl_node = create_node(PRIM_DECL_STMT, "PRIM_DECL_STMT");
    token_list_item *before = curr_token;

    if(parse_token(LANGLE) &&
       parse_token(PRIM_DECLARE_KW)) {

        int8_t type_declared = 0;
        char type_declared_str[20];

        bool success =
            expect_token(TYPE_KW, "type keyword") &&
            expect_token(EQ_SIGN, "=") &&
            expect_token(LCBRACE, "{") &&
            parse_type(&type_declared, type_declared_str) &&
            expect_token(RCBRACE, "}") &&
            expect_token(IDENTIFIER_KW, "identifier keyword") &&
            expect_token(EQ_SIGN, "=") &&
            expect_token(LCBRACE, "{") &&
            identifier_list(prim_decl_node) &&
            expect_token(RCBRACE, "}") &&
            optional_expr_list(prim_decl_node) &&
            expect_token(FW_SLASH, "/") &&
            expect_token(RANGLE, ">");

        if(success) {
            add_node_to_parent(prim_decl_node, TYPE_NODE, type_declared_str);
            add_ready_to_parent(parent, prim_decl_node);
            
            // printf("Primitive Declaration: [Type: %s, Identifier: %s]\n",
            //        type_declared_str,
            //        identifier_str);
            return true;
        } else {
            printf("incorrect primitive declaration statement.\n"
                   "Must be <prim_declare type={type} identifier={identifier}/>\n");
        }
    } else {
        curr_token = before -> prev;
        get_next_token();
    }

    destroy_node(prim_decl_node);
    return false;
}

bool list_decl_stmt(tree_node *parent) {
    tree_node *list_decl_node = create_node(LIST_DECL_STMT, "LIST_DECL_STMT");
    token_list_item *before = curr_token;

    if(parse_token(LANGLE) &&
       parse_token(LIST_DECLARE_KW)) {

        int8_t type_declared = 0;
        char type_declared_str[20];
        char identifier_str[80];

        bool success =
            expect_token(TYPE_KW, "type keyword") &&
            expect_token(EQ_SIGN, "=") &&
            expect_token(LCBRACE, "{") &&
            parse_type(&type_declared, type_declared_str) &&
            expect_token(RCBRACE, "}") &&
            expect_token(IDENTIFIER_KW, "identifier keyword") &&
            expect_token(EQ_SIGN, "=") &&
            expect_token(LCBRACE, "{") &&
            parse_identifier(identifier_str) &&
            expect_token(RCBRACE, "}") &&
            expect_token(RANGLE, ">") &&
            list_elems(list_decl_node) &&
            expect_token(LANGLE, "<") &&
            expect_token(FW_SLASH, "/") &&
            expect_token(LIST_DECLARE_KW, "list_declare keyword") &&
            expect_token(RANGLE, ">");

        if(success) {
            add_node_to_parent(list_decl_node, TYPE_NODE, type_declared_str);
            add_node_to_parent(list_decl_node, IDENTIFIER_NODE, identifier_str);
            add_ready_to_parent(parent, list_decl_node);
            return true;
        } else {
            printf("incorrect list declaration statement.\n"
                   "Must be <list type={type} identifier={identifier}> elems </list_declare>\n");
        }
    } else {
        curr_token = before -> prev;
        get_next_token();
    }

    destroy_node(list_decl_node);
    return false;
}

bool list_elems(tree_node *parent) {
    tree_node *list_elems_node = create_node(LIST_ELEMS_NODE, "LIST_ELEMS");
    token_list_item *before = curr_token;

    bool success = list_elem(list_elems_node);
    if(success) {
        add_ready_to_parent(parent, list_elems_node);
        list_elems(list_elems_node);
    } else {
        destroy_node(list_elems_node);
        curr_token = before -> prev;
        get_next_token();
    }
    
    return success;
}

bool list_elem(tree_node *parent) {
    tree_node *list_elem_node = create_node(LIST_ELEM_NODE, "LIST_ELEM");
    token_list_item *before = curr_token;

    if(parse_token(LANGLE) &&
       parse_token(ELEM_KW)) {

        bool success =
            expect_token(EXPR_KW, "expr keyword") &&
            expect_token(EQ_SIGN, "=") &&
            expect_token(LCBRACE, "{") &&
            parse_expr(list_elem_node) &&
            expect_token(RCBRACE, "}") &&
            expect_token(FW_SLASH, "/") &&
            expect_token(RANGLE, ">");

        if(success) {
            add_ready_to_parent(parent, list_elem_node);
            return true;
        } else {
            printf("incorrect elem statement.\n"
                   "Must be <elem expr={expr}/>\n");
        }
    } else {
        curr_token = before -> prev;
        get_next_token();
    }

    destroy_node(list_elem_node);
    return false;
}

bool assign_stmt(tree_node *parent) {
    tree_node *assign_stmt_node = create_node(ASSIGN_STMT, "ASSIGN_STMT");
    token_list_item *before = curr_token;

    if(parse_token(LANGLE) &&
       parse_token(ASSIGN_KW)) {

        char identifier_str[80];

        bool success =
            expect_token(IDENTIFIER_KW, "identifier keyword") &&
            expect_token(EQ_SIGN, "=") &&
            expect_token(LCBRACE, "{") &&
            parse_identifier(identifier_str) &&
            expect_token(RCBRACE, "}") &&
            expect_token(EXPR_KW, "expr keyword") &&
            expect_token(EQ_SIGN, "=") &&
            expect_token(LCBRACE, "{") &&
            parse_expr(assign_stmt_node) &&
            expect_token(RCBRACE, "}") &&
            expect_token(FW_SLASH, "/") &&
            expect_token(RANGLE, ">");

        if(success) {
            add_node_to_parent(assign_stmt_node, IDENTIFIER_NODE, identifier_str);
            add_ready_to_parent(parent, assign_stmt_node);
            // printf("Assignment Statement\n");
            return true;
        } else {
            printf("incorrect assign statement.\n"
                   "Must be <assign identifier={identifier} expr={expr}/>\n");
        }
    } else {
        curr_token = before -> prev;
        get_next_token();
    }

    destroy_node(assign_stmt_node);
    return false;
}

bool in_stmt(tree_node *parent) {
    tree_node *in_node = create_node(IN_STMT, "IN_STMT");
    token_list_item *before = curr_token;

    if(parse_token(LANGLE) &&
       parse_token(IN_KW)) {

        char identifier_str[80];

        bool success =
            expect_token(IDENTIFIER_KW, "identifier keyword") &&
            expect_token(EQ_SIGN, "=") &&
            expect_token(LCBRACE, "{") &&
            parse_identifier(identifier_str) &&
            expect_token(RCBRACE, "}") &&
            expect_token(FW_SLASH, "/") &&
            expect_token(RANGLE, ">");

        if(success) {
            add_node_to_parent(in_node, IDENTIFIER_NODE, identifier_str);
            add_ready_to_parent(parent, in_node);
            
            // printf("Input Statement: [Identifier: %s]\n",
            //        identifier_str);
            return true;
        } else {
            printf("incorrect input statement.\n"
                   "Must be <in identifier={identifier}/>\n");
        }
    } else {
        curr_token = before -> prev;
        get_next_token();
    }

    destroy_node(in_node);
    return false;
}

bool out_stmt(tree_node *parent) {
    tree_node *out_node = create_node(OUT_STMT, "OUT_STMT");
    token_list_item *before = curr_token;

    if(parse_token(LANGLE) &&
       parse_token(OUT_KW)) {

        char identifier_str[80];

        bool success =
            expect_token(EXPR_KW, "expr keyword") &&
            expect_token(EQ_SIGN, "=") &&
            expect_token(LCBRACE, "{") &&
            parse_expr(out_node) &&
            expect_token(RCBRACE, "}") &&
            expect_token(FW_SLASH, "/") &&
            expect_token(RANGLE, ">");

        if(success) {
            add_ready_to_parent(parent, out_node);
            
            // printf("Output Statement\n");
            return true;
        } else {
            printf("incorrect output statement.\n"
                   "Must be <out expr={expr}/>\n");
        }
    } else {
        curr_token = before -> prev;
        get_next_token();
    }

    destroy_node(out_node);
    return false;
}

bool decision_stmt(tree_node *parent) {
    tree_node *decision_node = create_node(DECISION_STMT, "DECISION_STMT");
    token_list_item *before = curr_token;

    bool success = if_stmt(decision_node);
    if(success) {
        elif_stmts(decision_node);
        else_stmt(decision_node);
    }

    if(success) {
        add_ready_to_parent(parent, decision_node);
    } else {
        destroy_node(decision_node);
        curr_token = before -> prev;
        get_next_token();
    }

    return success;
}

bool if_stmt(tree_node *parent) {
    tree_node *if_node = create_node(IF_STMT, "IF_STMT");
    token_list_item *before = curr_token;

    if(parse_token(LANGLE) &&
       parse_token(IF_KW)) {
        
        bool success =
            expect_token(EXPR_KW, "expr keyword") &&
            expect_token(EQ_SIGN, "=") &&
            expect_token(LCBRACE, "{") &&
            parse_expr(if_node) &&
            expect_token(RCBRACE, "}") &&
            expect_token(RANGLE, ">") &&
            stmts(if_node) &&
            expect_token(LANGLE, "<") &&
            expect_token(FW_SLASH, "/") &&
            expect_token(IF_KW, "if") &&
            expect_token(RANGLE, ">");

        if(success) {
            add_ready_to_parent(parent, if_node);
            // printf("Output Statement\n");
            return true;
        } else {
            printf("incorrect if statement.\n"
                   "Must be <if expr={expr}>stmts</if>\n");
        }
    } else {
        curr_token = before -> prev;
        get_next_token();
    }

    destroy_node(if_node);
    return false;
}

bool elif_stmts(tree_node *parent) {
    tree_node *elif_stmts_node = create_node(ELIF_STMTS, "ELIF_STMTS");
    token_list_item *before = curr_token;

    bool success = elif_stmt(elif_stmts_node);

    if(success) {
        add_ready_to_parent(parent, elif_stmts_node);
        elif_stmts(elif_stmts_node);
    } else {
        destroy_node(elif_stmts_node);
        curr_token = before -> prev;
        get_next_token();
    }

    return true;
}

bool elif_stmt(tree_node *parent) {
    tree_node *elif_node = create_node(ELIF_STMT, "ELIF_STMT");
    token_list_item *before = curr_token;

    if(parse_token(LANGLE) &&
       parse_token(ELIF_KW)) {
        
        bool success =
            expect_token(EXPR_KW, "expr keyword") &&
            expect_token(EQ_SIGN, "=") &&
            expect_token(LCBRACE, "{") &&
            parse_expr(elif_node) &&
            expect_token(RCBRACE, "}") &&
            expect_token(RANGLE, ">") &&
            stmts(elif_node) &&
            expect_token(LANGLE, "<") &&
            expect_token(FW_SLASH, "/") &&
            expect_token(ELIF_KW, "elif keyword") &&
            expect_token(RANGLE, ">");

        if(success) {
            add_ready_to_parent(parent, elif_node);
            // printf("Output Statement\n");
            return true;
        } else {
            printf("incorrect elif statement.\n"
                   "Must be <elif expr={expr}>stmts</elif>\n");
        }
    } else {
        curr_token = before -> prev;
        get_next_token();
    }

    destroy_node(elif_node);
    return false;
}

bool else_stmt(tree_node *parent) {
    tree_node *else_node = create_node(ELSE_STMT, "ELSE_STMT");
    token_list_item *before = curr_token;

    if(parse_token(LANGLE) &&
       parse_token(ELSE_KW)) {
        
        bool success =
            expect_token(RANGLE, ">") &&
            stmts(else_node) &&
            expect_token(LANGLE, "<") &&
            expect_token(FW_SLASH, "/") &&
            expect_token(ELSE_KW, "else keyword") &&
            expect_token(RANGLE, ">");

        if(success) {
            add_ready_to_parent(parent, else_node);
            // printf("Output Statement\n");
            return true;
        } else {
            printf("incorrect else statement.\n"
                   "Must be <else>stmts</else>\n");
        }
    } else {
        curr_token = before -> prev;
        get_next_token();
    }

    destroy_node(else_node);
    return false;
}

bool switch_stmt(tree_node *parent) {
    tree_node *switch_node = create_node(SWITCH_STMT, "SWITCH_STMT");
    token_list_item *before = curr_token;

    if(parse_token(LANGLE) &&
       parse_token(SWITCH_KW)) {

        char identifier_str[80];

        bool success =
            expect_token(EVAL_KW, "eval keyword") &&
            expect_token(EQ_SIGN, "=") &&
            expect_token(LCBRACE, "{") &&
            parse_identifier(identifier_str) &&
            expect_token(RCBRACE, "}") &&
            expect_token(RANGLE, ">") &&
            cases(switch_node) &&
            expect_token(LANGLE, "<") &&
            expect_token(FW_SLASH, "/") &&
            expect_token(SWITCH_KW, "switch") &&
            expect_token(RANGLE, ">");

        if(success) {
            add_node_to_parent(switch_node, EVAL, identifier_str);
            add_ready_to_parent(parent, switch_node);
            
            // printf("Input Statement: [Identifier: %s]\n",
            //        identifier_str);
            return true;
        } else {
            printf("incorrect switch statement.\n"
                   "Must be <switch eval={eval}>cases</switch>\n");
        }
    } else {
        curr_token = before -> prev;
        get_next_token();
    }

    destroy_node(switch_node);
    return false;
}

bool cases(tree_node *parent) {
    tree_node *cases_node = create_node(CASES, "CASES");
    token_list_item *before = curr_token;

    bool success = case_(cases_node);
    if(success) {
        add_ready_to_parent(parent, cases_node);
        cases(cases_node);
    } else {
        destroy_node(cases_node);
        curr_token = before -> prev;
        get_next_token();
    }
    
    return success;
}

bool case_(tree_node *parent) {
    tree_node *case_node = create_node(CASE_, "CASE");
    token_list_item *before = curr_token;

    bool success = regular_case(case_node) ||
                   default_case(case_node);
    
    if(success) {
        add_ready_to_parent(parent, case_node);
    } else {
        destroy_node(case_node);
        curr_token = before -> prev;
        get_next_token();
    }
    
    return success;
}

bool regular_case(tree_node *parent) {
    tree_node *regular_case_node = create_node(REGULAR_CASE, "REGULAR_CASE");
    token_list_item *before = curr_token;

    if(parse_token(LANGLE) &&
       parse_token(CASE_KW)) {

        char constant[80];

        bool success =
            expect_token(CONST_KW, "const keyword") &&
            expect_token(EQ_SIGN, "=") &&
            expect_token(LCBRACE, "{") &&
            parse_constant(constant) &&
            expect_token(RCBRACE, "}") &&
            expect_token(RANGLE, ">") &&
            stmts(regular_case_node) &&
            expect_token(LANGLE, "<") &&
            expect_token(FW_SLASH, "/") &&
            expect_token(CASE_KW, "case keyword") &&
            expect_token(RANGLE, ">");

        if(success) {
            add_node_to_parent(regular_case_node, CONST, constant);
            add_ready_to_parent(parent, regular_case_node);
            
            // printf("Input Statement: [Identifier: %s]\n",
            //        identifier_str);
            return true;
        } else {
            printf("incorrect case statement.\n"
                   "Must be <case const={constant}>stmts</case>\n");
        }
    } else {
        curr_token = before -> prev;
        get_next_token();
    }

    destroy_node(regular_case_node);
    return false;
}

bool default_case(tree_node *parent) {
    tree_node *default_case_node = create_node(DEFAULT_CASE, "DEFAULT_CASE");
    token_list_item *before = curr_token;

    if(parse_token(LANGLE) &&
       parse_token(DEFAULT_KW)) {

        bool success =
            expect_token(RANGLE, ">") &&
            stmts(default_case_node) &&
            expect_token(LANGLE, "<") &&
            expect_token(FW_SLASH, "/") &&
            expect_token(DEFAULT_KW, "default keyword") &&
            expect_token(RANGLE, ">");

        if(success) {
            add_ready_to_parent(parent, default_case_node);
            
            // printf("Default Case:");
            return true;
        } else {
            printf("incorrect default statement.\n"
                   "Must be <default>stmts</case>\n");
        }
    } else {
        curr_token = before -> prev;
        get_next_token();
    }

    destroy_node(default_case_node);
    return false;
}

bool while_stmt(tree_node *parent) {
    tree_node *while_stmt_node = create_node(WHILE_STMT, "WHILE_STMT");
    token_list_item *before = curr_token;

    if(parse_token(LANGLE) &&
       parse_token(WHILE_KW)) {
        
        bool success =
            expect_token(EXPR_KW, "expr keyword") &&
            expect_token(EQ_SIGN, "=") &&
            expect_token(LCBRACE, "{") &&
            parse_expr(while_stmt_node) &&
            expect_token(RCBRACE, "}") &&
            expect_token(RANGLE, ">") &&
            stmts(while_stmt_node) &&
            expect_token(LANGLE, "<") &&
            expect_token(FW_SLASH, "/") &&
            expect_token(WHILE_KW, "while keyword") &&
            expect_token(RANGLE, ">");

        if(success) {
            add_ready_to_parent(parent, while_stmt_node);
            // printf("Default Case:");
            return true;
        } else {
            printf("incorrect while statement.\n"
                   "Must be <while expr={expr}>stmts</while>\n");
        }
    } else {
        curr_token = before -> prev;
        get_next_token();
    }

    destroy_node(while_stmt_node);
    return false;
}

bool break_stmt(tree_node *parent) {
    tree_node *break_stmt_node = create_node(BREAK_STMT, "BREAK_STMT");
    token_list_item *before = curr_token;

    if(parse_token(LANGLE) &&
       parse_token(BREAK_KW)) {

        bool success =
            expect_token(FW_SLASH, "/") &&
            expect_token(RANGLE, ">");

        if(success) {
            add_ready_to_parent(parent, break_stmt_node);
            printf("Break Statement\n");
            return true;
        } else {
            printf("incorrect break statement.\n"
                   "Must be <break/>\n");
        }
    } else {
        curr_token = before -> prev;
        get_next_token();
    }

    destroy_node(break_stmt_node);
    return false;
}

bool continue_stmt(tree_node *parent) {
    tree_node *continue_stmt_node = create_node(CONTINUE_STMT, "CONTINUE_STMT");
    token_list_item *before = curr_token;

    if(parse_token(LANGLE) &&
       parse_token(CONTINUE_KW)) {

        bool success =
            expect_token(FW_SLASH, "/") &&
            expect_token(RANGLE, ">");

        if(success) {
            add_ready_to_parent(parent, continue_stmt_node);
            printf("Continue Statement\n");
            return true;
        } else {
            printf("incorrect continue statement.\n"
                   "Must be <continue/>\n");
        }
    } else {
        curr_token = before -> prev;
        get_next_token();
    }

    destroy_node(continue_stmt_node);
    return false;
}

bool parse_expr(tree_node *parent) {
    tree_node *expr_node = create_node(EXPR_NODE, "EXPR");
    token_list_item *before = curr_token;

    expr_stack *operators = create_expr_stack(),
               *operands = create_expr_stack();
    
    expr_stack_push(operators, create_node(SENTINEL_EXPR_NODE, NULL));
    bool success = e(operators, operands);

    if(success) {
        add_ready_to_parent(expr_node, expr_stack_peek(operands));
        add_ready_to_parent(parent, expr_node);
    } else {
        curr_token = before -> prev;
        get_next_token();
    }

    return success;
}

bool e(expr_stack *operators, expr_stack *operands) {
    p(operators, operands);
    get_next_token();
    while(is_recognizable_as_binary(curr_token_rep)) {
        tree_node *binary_node = trasnform_into_binary_node(curr_token_rep);
        push_operator(binary_node, operators, operands);
        p(operators, operands);
        get_next_token();
    }
    get_prev_token();
    while(expr_stack_peek(operators) -> type != SENTINEL_EXPR_NODE)
        pop_operator(operators, operands);
    return true;
}

bool p(expr_stack *operators, expr_stack *operands) {
    char placeholder[80];

    if(parse_identifier(placeholder)) {
        tree_node *identifier_node = create_node(IDENTIFIER_NODE, placeholder);
        expr_stack_push(operands, identifier_node);
    } else if(parse_constant(placeholder)) {
        tree_node *literal_node;
        switch(curr_token_rep) {
            case INT_LITERAL:
                literal_node = create_node(INT_LITERAL_NODE, curr_lexeme);
                break;
            case FLOAT_LITERAL:
                literal_node = create_node(FLOAT_LITERAL_NODE, curr_lexeme);
                break;
            case STR_LITERAL:
                literal_node = create_node(STR_LITERAL_NODE, curr_lexeme);
                break;
            case TRUE_KW:
                literal_node = create_node(TRUE_KW_NODE, curr_lexeme);
                break;
            case FALSE_KW:
                literal_node = create_node(FALSE_KW_NODE, curr_lexeme);
                break;
        }
        expr_stack_push(operands, literal_node);
    } else {
        syn_error_count++;
        printf("Syntax error: expected value, unary operator, or parenthesis.");
    }
}

void pop_operator(expr_stack *operators, expr_stack *operands) {
    tree_node *operator = expr_stack_pop(operators);
    if(!is_unary(operator -> type)) {
        // if binary operator, pop two operands then attach
        tree_node *second_operand = expr_stack_pop(operands),
                  *first_operand = expr_stack_pop(operands);
        add_ready_to_parent(operator, first_operand);
        add_ready_to_parent(operator, second_operand);
        expr_stack_push(operands, operator);
    } else {
        // if unary, just pop one operand
        tree_node *operand = expr_stack_pop(operands);
        add_ready_to_parent(operator, operand);
        expr_stack_push(operands, operator);
    }
}

void push_operator(tree_node *operator, expr_stack *operators, expr_stack *operands) {
    while(compare_operators(expr_stack_peek(operators) -> type,
                            operator -> type))
        pop_operator(operators, operands);
    expr_stack_push(operators, operator);
}

bool compare_operators(node_type first, node_type second) {
    // if both binary, check left associativity also
    if(!is_unary(first) && !is_unary(second))
        return precedence_level(first) > precedence_level(second) ||
               (precedence_level(first) == precedence_level(second) &&
                is_left_associative(first));
    
    else
        return precedence_level(first) > precedence_level(second);
}

int8_t precedence_level(node_type operator) {
    switch(operator) {
        case SENTINEL_EXPR_NODE:
            return 255;
        case LOGIC_OR_NODE:
            return 12;
        case LOGIC_AND_NODE:
            return 11;
        case BITWISE_OR_NODE:
            return 10;
        case BITWISE_NOT_NODE:
            return 9;
        case BITWISE_AND_NODE:
            return 8;
        case RELATIONAL_EQ_NODE:
        case RELATIONAL_NOT_EQ_NODE:
            return 7;
        case RELATIONAL_GT_NODE:
        case RELATIONAL_LT_NODE:
        case RELATIONAL_GT_EQ_NODE:
        case RELATIONAL_LT_EQ_NODE:
            return 6;
        case BITWISE_LEFT_NODE:
        case BITWISE_RIGHT_NODE:
            return 5;
        case ADD_NODE:
        case SUB_NODE:
            return 4;
        case MUL_NODE:
        case DIV_NODE:
        case FLOOR_NODE:
        case MOD_NODE:
            return 3;
        case UNARY_ADD_NODE:
        case UNARY_SUB_NODE:
            return 2;
        default:
            return -1;
    }
}

bool is_recognizable_as_binary(int8_t token_rep) {
    return token_rep == LOGIC_OR_OP ||
           token_rep == LOGIC_AND_OP ||
           token_rep == BITWISE_OR_OP ||
           token_rep == BITWISE_XOR_OP ||
           token_rep == BITWISE_AND_OP ||
           token_rep == EQ_OP ||
           token_rep == NOT_EQ_OP ||
           token_rep == LANGLE ||
           token_rep == LT_EQ_OP ||
           token_rep == RANGLE ||
           token_rep == GT_EQ_OP ||
           token_rep == BITWISE_LEFT_OP ||
           token_rep == BITWISE_RIGHT_OP ||
           token_rep == ADD_OP ||
           token_rep == HYPHEN ||
           token_rep == MUL_OP ||
           token_rep == FW_SLASH ||
           token_rep == FLR_OP ||
           token_rep == MOD_OP;
}

bool is_recognizable_as_unary(int8_t token_rep) {
    return token_rep == ADD_OP ||
           token_rep == HYPHEN;
}

tree_node *trasnform_into_binary_node(int8_t token_rep) {
    switch(token_rep) {
        case LOGIC_OR_OP: return create_node(LOGIC_OR_NODE, "||");
        case LOGIC_AND_OP: return create_node(LOGIC_AND_NODE, "&&");
        case BITWISE_OR_OP: return create_node(BITWISE_OR_NODE, "|");
        case BITWISE_XOR_OP: return create_node(BITWISE_XOR_NODE, "^");
        case BITWISE_AND_OP: return create_node(BITWISE_AND_NODE, "&");
        case EQ_OP: return create_node(RELATIONAL_EQ_NODE, "==");
        case NOT_EQ_OP: return create_node(RELATIONAL_NOT_EQ_NODE, "!=");
        case LANGLE: return create_node(RELATIONAL_LT_NODE, "<");
        case LT_EQ_OP: return create_node(RELATIONAL_LT_EQ_NODE, "<=");
        case RANGLE: return create_node(RELATIONAL_GT_NODE, ">");
        case GT_EQ_OP: return create_node(RELATIONAL_GT_EQ_NODE, ">=");
        case BITWISE_LEFT_OP: return create_node(BITWISE_LEFT_NODE, "<<");
        case BITWISE_RIGHT_OP: return create_node(BITWISE_RIGHT_NODE, ">>");
        case ADD_OP: return create_node(ADD_NODE, "+");
        case HYPHEN: return create_node(SUB_NODE, "-");
        case MUL_OP: return create_node(MUL_NODE, "*");
        case FW_SLASH: return create_node(DIV_NODE, "/");
        case FLR_OP: return create_node(FLOOR_NODE, "//");
        case MOD_OP: return create_node(MOD_NODE, "%");
        default: return NULL;
    }
}

bool is_unary(node_type type) {
    return type == UNARY_ADD_NODE ||
           type == UNARY_SUB_NODE;
}

bool is_left_associative(node_type type) {
    return type == LOGIC_OR_NODE ||
           type == LOGIC_AND_NODE ||
           type == BITWISE_OR_NODE ||
           type == BITWISE_XOR_NODE ||
           type == BITWISE_AND_NODE ||
           type == RELATIONAL_EQ_NODE ||
           type == RELATIONAL_NOT_EQ_NODE ||
           type == RELATIONAL_LT_NODE ||
           type == RELATIONAL_LT_EQ_NODE ||
           type == RELATIONAL_GT_NODE ||
           type == RELATIONAL_GT_EQ_NODE ||
           type == BITWISE_LEFT_NODE ||
           type == BITWISE_RIGHT_NODE ||
           type == ADD_NODE ||
           type == SUB_NODE ||
           type == MUL_NODE ||
           type == DIV_NODE ||
           type == FLOOR_NODE ||
           type == MOD_NODE;
}

bool parse_type(int8_t *placeholder, char *placeholder_lex) {
    get_next_token();
    strcpy(placeholder_lex, curr_lexeme);
    bool success =
        curr_token_rep == INT_KW ||
        curr_token_rep == FLOAT_KW ||
        curr_token_rep == STRING_KW ||
        curr_token_rep == BOOL_KW;
    if(!success) printf("Error: expected data type.");
    return success;
}

bool identifier_list(tree_node *parent) {
    tree_node *identifier_list_node = create_node(IDENTIFIER_LIST_NODE, "IDENTIFIER_LIST");
    token_list_item *before = curr_token;

    char placeholder_identifier[80];
    bool success = parse_identifier(placeholder_identifier);

    if(success) {
        add_node_to_parent(identifier_list_node, IDENTIFIER_NODE, placeholder_identifier);
        add_ready_to_parent(parent, identifier_list_node);
        // if a comma was encountered, expect another identifier
        if(parse_token(COMMA)) {
            bool another_success = identifier_list(identifier_list_node);
            if(!another_success) {
                syn_error_count++;
                printf("Syntax error: expected another identifier"
                       "after comma 'id , <another_id>'\n");
                destroy_node(identifier_list_node);
                return false;
            }
        }
    } else {
        destroy_node(identifier_list_node);
        curr_token = before -> prev;
        get_next_token();
    }

    return success;
}

bool optional_expr_list(tree_node *parent) {
    token_list_item *before = curr_token;

    if(parse_token(EXPR_KW)) {
        bool success =
            expect_token(EQ_SIGN, "=") &&
            expect_token(LCBRACE, "{") &&
            expr_list(parent) &&
            expect_token(RCBRACE, "}");
        if(!success) {
            printf("incorrect expr attribute.\n"
                   "must be expr={<list_of_expressions>}\n");
            return false;
        }
    } else {
        curr_token = before -> prev;
        get_next_token();
    }

    return true;
}

bool expr_list(tree_node *parent) {
    tree_node *expr_list_node = create_node(EXPR_LIST_NODE, "EXPR_LIST");
    token_list_item *before = curr_token;

    bool success = parse_expr(expr_list_node);

    if(success) {
        add_ready_to_parent(parent, expr_list_node);
        // if a comma was encountered, expect another identifier
        if(parse_token(COMMA)) {
            bool another_success = expr_list(expr_list_node);
            if(!another_success) {
                printf("Syntax error: expected another expression"
                       "after comma 'expr , <another_expr>'\n");
                syn_error_count++;
                destroy_node(expr_list_node);
                return false;
            }
        }
    } else {
        destroy_node(expr_list_node);
        curr_token = before -> prev;
        get_next_token();
    }

    return success;
}

bool parse_identifier(char *placeholder_lex) {
    token_list_item *before = curr_token;
    get_next_token();
    strcpy(placeholder_lex, curr_lexeme);
    bool success = curr_token_rep == IDENTIFIER;
    if(!success) {
        curr_token = before -> prev;
        get_next_token();
    }
    return success;
}

bool parse_token(int8_t token) {
    token_list_item *before = curr_token;
    if(curr_token_rep == EOF) { return false; }
    get_next_token();
    bool success = token == curr_token_rep;
    if(!success) {
        curr_token = before -> prev;
        get_next_token();
    }
    return success;
}

bool parse_constant(char *placeholder_lex) {
    token_list_item *before = curr_token;
    if(curr_token_rep == EOF) { return false; }
    get_next_token();
    strcpy(placeholder_lex, curr_lexeme);
    bool success =
        curr_token_rep == INT_LITERAL ||
        curr_token_rep == FLOAT_LITERAL ||
        curr_token_rep == STR_LITERAL ||
        curr_token_rep == TRUE_KW ||
        curr_token_rep == FALSE_KW;
    if(!success) {
        curr_token = before -> prev;
        get_next_token();
    }
    return success;
}

bool expect_token(int8_t expected_token, char *expected_token_str) {
    bool success = parse_token(expected_token);
    if(!success) { 
        printf("Error: expected %s but got %s\n",
               expected_token_str,
               curr_token -> next -> token -> lexeme);
        syn_error_count++;
    }
    return success;
}

void get_next_token() {
    curr_token = curr_token -> next;
    if(curr_token -> token != NULL) {
        curr_lexeme = curr_token -> token -> lexeme;
        curr_token_rep = curr_token -> token -> token_rep;
    }
}

void get_prev_token() {
    curr_token = curr_token -> prev;
    if(curr_token -> token != NULL) {
        curr_lexeme = curr_token -> token -> lexeme;
        curr_token_rep = curr_token -> token -> token_rep;
    }
}

void flatten_syntax_tree(tree_node *current) {
    if(current == NULL) return;

    node_type type = current -> type;
    if(type == IDENTIFIER_NODE) {
        fprintf(syn_tree_file, "IDENTIFIER\t%s\n", current -> value); return; }
    else if(type == INT_LITERAL_NODE) {
        fprintf(syn_tree_file, "INT_LITERAL\t%s\n", current -> value); return; }
    else if(type == FLOAT_LITERAL_NODE) {
        fprintf(syn_tree_file, "FLOAT_LITERAL\t%s\n", current -> value); return; }
    else if(type == STR_LITERAL_NODE) {
        fprintf(syn_tree_file, "STR_LITERAL\t%s\n", current -> value); return; }
    else if(type == TRUE_KW_NODE) {
        fprintf(syn_tree_file, "TRUE_KW\t%s\n", current -> value); return; }
    else if(type == FALSE_KW_NODE) {
        fprintf(syn_tree_file, "FALSE_KW\t%s\n", current -> value); return; }
    
    else if(type == TYPE_NODE) {
        fprintf(syn_tree_file, "TYPE\t%s\n", current -> value); return; }
    else if(type == EXPR_NODE) {
        print_expr(current); fprintf(syn_tree_file, "\n"); return; }

    // Print the current node
    fprintf(syn_tree_file, "%s\n", current -> value);
    // Recurse for each child
    if(current -> children_head != NULL) {
        tree_node *current_child = current -> children_head;
        while(current_child != NULL) {
            flatten_syntax_tree(current_child);
            current_child = current_child -> next_sibling;
        }
    }
}

void print_expr(tree_node *expr_node) {
    // Print the "expr" string
    fprintf(syn_tree_file, "%s\t", expr_node -> value);
    // For each child, output the string value
    print_expr_child(expr_node -> children_head);
}

void print_expr_child(tree_node *current) {
    if(current == NULL) return;

    // If single value, print immediately then return
    node_type type = current -> type;
    if(type == IDENTIFIER_NODE ||
       type == INT_LITERAL_NODE ||
       type == FLOAT_LITERAL_NODE ||
       type == STR_LITERAL_NODE ||
       type == TRUE_KW_NODE ||
       type == FALSE_KW_NODE) {
        fprintf(syn_tree_file, "%s", current -> value);
        return;
    }

    // Print the value, a left paren (, and all children inside recursively, and right paren )
    fprintf(syn_tree_file, "%s(", current -> value);
    // Recurse for each child
    if(current -> children_head != NULL) {
        tree_node *current_child = current -> children_head;
        print_expr_child(current_child);
        while((current_child = current_child -> next_sibling) != NULL) {
            fprintf(syn_tree_file, ",");
            print_expr_child(current_child);
        }
    }
    fprintf(syn_tree_file, ")");
}