
#ifndef STDLIB_H
#define STDLIB_H
#include <stdlib.h>
#endif

#ifndef STRING_H
#define STRING_H
#include <string.h>
#endif

#ifndef INTTYPES_H
#define INTTYPES_H
#include <inttypes.h>
#endif

#ifndef STDBOOL_H
#define STDBOOL_H
#include <stdbool.h>
#endif

struct token {
    char lexeme[80];
    int8_t token_rep;
};

struct token_list_item {
    struct token *token;
    struct token_list_item *next;
    struct token_list_item *prev;
};
typedef struct token_list_item token_list_item;

struct token_list {
    struct token_list_item *head;
    struct token_list_item *tail;
    int current_size;
};
typedef struct token_list token_list;

/**
 * @brief Creates a token list.
 * 
 * @return struct token_list* the list createds
 */
struct token_list* construct_token_list() {
    struct token_list *token_list_created = (struct token_list*)
        malloc(sizeof(struct token_list));

    // Initialize sentinels
    token_list_created -> head =
        (struct token_list_item *) malloc(sizeof(struct token_list_item));
    token_list_created -> tail =
        (struct token_list_item *) malloc(sizeof(struct token_list_item));
    token_list_created -> head -> next = token_list_created -> tail;
    token_list_created -> head -> token = NULL;
    token_list_created -> tail -> prev = token_list_created -> head;
    token_list_created -> tail -> token = NULL;
    token_list_created -> current_size = 0;
    return token_list_created;
}

/**
 * @brief Adds a token to the specitied list.
 * 
 * @param token_list 
 * @param lexeme 
 * @param token 
 */
void add_token_to_list(struct token_list *token_list_,
                       const char *lexeme,
                       int8_t token_rep) {
    
    if(token_list_ == NULL || lexeme == NULL)
        return;

    // Allocate the token
    struct token_list_item *new_item = (struct token_list_item*)
        malloc(sizeof(struct token_list_item));
    new_item -> next = NULL;
    new_item -> prev = NULL;
    new_item -> token = (struct token*) malloc(sizeof(struct token));
    strcpy(new_item -> token -> lexeme, lexeme);
    new_item -> token -> token_rep = token_rep;

    // Attach the token to the list
    struct token_list_item *before_new_item = token_list_ -> tail -> prev;
    before_new_item -> next = new_item;
    new_item -> prev = before_new_item;
    new_item -> next = token_list_ -> tail;
    token_list_ -> tail -> prev = new_item;

    // Increment the size of list
    token_list_ -> current_size++;
}

/**
 * @brief Frees the memory allocated for the specified token list.
 * 
 * @param token_list_ 
 */
void destroy_token_list(struct token_list *token_list_) {
    // Destroy the individual items first
    if(token_list_ -> current_size > 0) {
        struct token_list_item *current = token_list_ -> head -> next;
        while(current != token_list_ -> tail) {
            struct token_list_item *next = current -> next;
            free(current -> token);
            free(current);
            current = next;
        }
    }

    // Destroy the actual list
    free(token_list_ -> head);
    free(token_list_ -> tail);
    free(token_list_);
}

/*
int main() {
    struct token_list *token_list_ = construct_token_list();
    add_token_to_list(token_list_, "abcd", ADD_OP);
    add_token_to_list(token_list_, "asdasd", ADD_OP);
    token_list_item *current_item = get_current_from_token_list(token_list_);
    printf("Lexeme: %s, Token: %d\n", current_item -> token -> lexeme,
                                      current_item -> token -> token_rep);
    next_item_in_token_list(token_list_);
    current_item = get_current_from_token_list(token_list_);
    printf("Lexeme: %s, Token: %d\n", current_item -> token -> lexeme,
                                      current_item -> token -> token_rep);
    destroy_token_list(token_list_);
}
*/