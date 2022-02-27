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
#include "stdio.h"
#endif

// Prototypes
void syn_init();
void parse_tokens_into_list();
void get_next_token();
void get_prev_token();

// Process Productions
bool prog();
bool stmts(tree_node*);
bool stmt(tree_node*);
bool prim_decl_stmt(tree_node*);
bool in_stmt(tree_node*);
bool switch_stmt(tree_node*);
bool cases(tree_node*);
bool case_(tree_node*);
bool regular_case(tree_node*);
bool default_case(tree_node*);
bool break_stmt(tree_node*);
bool continue_stmt(tree_node*);

bool expr(tree_node*);

bool parse_token(int8_t);
bool expect_token(int8_t, char*);
bool parse_identifier(char*);
bool parse_constant(char*);
bool parse_type(int8_t*, char*);

// Globals for Syntax Analysis
FILE *sym_file;
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
    curr_token = all_tokens -> head;

    // Start parsing
    prog();

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
    while((read = getline(&line, &len, sym_file)) != -1) {
        char *lexeme = strtok(line, ",");
        int8_t token_rep = atoi(strtok(NULL, ","));
        add_token_to_list(all_tokens, lexeme, token_rep);
    }
}

bool prog() {
    root = create_node(PROG, NULL);
    return stmts(root);
}

bool stmts(tree_node* parent) {
    tree_node *stmts_node = create_node(STMTS, NULL);

    bool success = stmt(stmts_node);

    if(success) {
        add_ready_to_parent(parent, stmts_node);
        stmts(stmts_node);
    } else
        destroy_node(stmts_node);

    return success;
}

bool stmt(tree_node* parent) {
    // if eof, stop
    get_next_token();
    if(curr_token_rep == EOF) return false;
    get_prev_token();

    tree_node *stmt_node = create_node(STMT, NULL);
    bool success =
        prim_decl_stmt(stmt_node) ||
        in_stmt(stmt_node) ||
        switch_stmt(stmt_node) ||
        break_stmt(stmt_node) ||
        continue_stmt(stmt_node);

    success ? add_ready_to_parent(parent, stmt_node) :
              destroy_node(stmt_node);

    return success;
}

bool prim_decl_stmt(tree_node* parent) {
    tree_node *prim_decl_node = create_node(PRIM_DECL_STMT, NULL);
    token_list_item *before = curr_token;

    if(parse_token(LANGLE) &&
       parse_token(PRIM_DECLARE_KW)) {

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
            expect_token(FW_SLASH, "/") &&
            expect_token(RANGLE, ">");

        if(success) {
            add_node_to_parent(prim_decl_node, TYPE, type_declared_str);
            add_node_to_parent(prim_decl_node, IDENTIFIER_NODE, identifier_str);
            add_ready_to_parent(parent, prim_decl_node);
            
            printf("Primitive Declaration: [Type: %s, Identifier: %s]\n",
                   type_declared_str,
                   identifier_str);
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

bool in_stmt(tree_node *parent) {
    tree_node *in_node = create_node(IN_STMT, NULL);
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
            
            printf("Input Statement: [Identifier: %s]\n",
                   identifier_str);
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

bool switch_stmt(tree_node *parent) {
    tree_node *switch_node = create_node(SWITCH_STMT, NULL);
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
    tree_node *cases_node = create_node(CASES, NULL);

    bool success = case_(cases_node);
    if(success) {
        add_ready_to_parent(parent, cases_node);
        cases(cases_node);
    } else
        destroy_node(cases_node);
    
    return success;
}

bool case_(tree_node *parent) {
    tree_node *case_node = create_node(CASE_, NULL);

    bool success = regular_case(case_node) ||
                   default_case(case_node);
    success ? add_ready_to_parent(parent, case_node) :
              destroy_node(case_node);
    
    return success;
}

bool regular_case(tree_node *parent) {
    tree_node *regular_case_node = create_node(REGULAR_CASE, NULL);
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
    tree_node *default_case_node = create_node(DEFAULT_CASE, NULL);
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

bool break_stmt(tree_node *parent) {
    tree_node *break_stmt_node = create_node(BREAK_STMT, NULL);
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
    tree_node *continue_stmt_node = create_node(CONTINUE_STMT, NULL);
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

bool parse_identifier(char *placeholder_lex) {
    get_next_token();
    strcpy(placeholder_lex, curr_lexeme);
    return curr_token_rep == IDENTIFIER;
}

bool parse_token(int8_t token) {
    if(curr_token_rep == EOF) { return false; }
    get_next_token();
    return token == curr_token_rep;
}

bool parse_constant(char *placeholder_lex) {
    if(curr_token_rep == EOF) { return false; }
    get_next_token();
    strcpy(placeholder_lex, curr_lexeme);
    return curr_token_rep == INT_LITERAL ||
           curr_token_rep == FLOAT_LITERAL ||
           curr_token_rep == STR_LITERAL ||
           curr_token_rep == TRUE_KW ||
           curr_token_rep == FALSE_KW;
}

bool expect_token(int8_t expected_token, char *expected_token_str) {
    bool success = parse_token(expected_token);
    if(!success) printf("Error: expected %s but got %s\n", expected_token_str, curr_lexeme);
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