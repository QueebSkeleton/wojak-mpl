
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
    struct token_list_item *current;
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
    token_list_created -> tail -> prev = token_list_created -> head;

    token_list_created -> current = NULL;
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
    // Set current initially (if this is the first item added)
    if(token_list_ -> current_size == 1) token_list_ -> current = new_item;
}

/**
 * @brief Traverses the current pointer in the list to the next.
 * 
 * @param token_list_ the list to change current value.
 * @return true if the traversal was successful
 * @return false if not
 */
bool next_item_in_token_list(struct token_list *token_list_) {
    // If token list is null,
    // or no items yet are added,
    // or the current item is the last item, simply do nothing
    if(token_list_ == NULL ||
       token_list_ -> current == NULL ||
       token_list_ -> current -> next == token_list_ -> tail) return false;

    token_list_ -> current = token_list_ -> current -> next;
    return true;
}

/**
 * @brief Get the current from token list
 * 
 * @param token_list_ 
 * @return struct token_list_item* 
 */
struct token_list_item* get_current_from_token_list(const struct token_list *token_list_) {
    if(token_list_ == NULL) return NULL;
    return token_list_ -> current;
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