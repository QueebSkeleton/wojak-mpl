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

bool parse_token(int8_t);
bool expect_token(int8_t, char*);
bool parse_identifier(char*);
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
    tree_node *stmts_node = add_node_to_parent(parent, STMTS, NULL);

    bool success = stmt(stmts_node);
    if(success) stmts(stmts_node);

    return true;
}

bool stmt(tree_node* parent) {
    // if eof, stop
    get_next_token();
    if(curr_token_rep == EOF) return false;
    get_prev_token();

    tree_node *stmt_node = add_node_to_parent(parent, STMT, NULL);
    return prim_decl_stmt(stmt_node) ||
           in_stmt(stmt_node);
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

    free(prim_decl_node);
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

    free(in_node);
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