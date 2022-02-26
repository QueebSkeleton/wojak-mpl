#ifndef TOKENS_H
#define TOKENS_H
#include "tokens.h"
#endif

#ifndef TOKEN_LIST_H
#define TOKEN_LIST_H
#include "token_list.h"
#endif

// Prototypes
void syn_init();
// Process Productions
void prog();
void stmts();
void stmt();
void prim_declare_stmt();

// Globals for Syntax Analysis
token_list *all_tokens_in_file;

void syn_init() {
    // Initialize the token list
}

void prog() {

}

void stmts() {

}

void stmt() {

}

void prim_declare_stmt() {
    int expected_tokens[] = { LANGLE,
                              PRIM_DECLARE_KW,
                              TYPE_KW, EQ_SIGN, LCBRACE, RCBRACE,
                              IDENTIFIER_KW, EQ_SIGN, LCBRACE, IDENTIFIER, RCBRACE,
                              };
}