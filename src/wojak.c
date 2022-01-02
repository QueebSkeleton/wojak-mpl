// TODO: Document properly

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_LENGTH 1024

#define ADD_OP 1
#define INC_OP 2
#define SUB_OP 3
#define DEC_OP 4

typedef struct {
    char lexeme[80];
    int rep;
} token;

void init();
token* lex();
void get_next_char();
void get_next_char_nonblank();
void refresh_buffer_firsthalf();
void refresh_buffer_secondhalf();

// Compiler input file
FILE *input_file;

// Lexical Analyzer Stuff HERE, MOVE LATER TO HEADER FILE
char next_char;
int current_state;
// Buffer (double buffering technique)
char buffer[BUFFER_LENGTH * 2];
int begin_lexeme_ptr;
int forward_lexeme_ptr;

int main(int argc, char *argv[]) {

    if(argc < 2) {
        printf("Error: no given files.\n");
        printf("Usage is: wojak filename.wojak\n");
        exit(EXIT_FAILURE);
    }

    // Open file in read mode
    if((input_file = fopen(argv[1], "r")) == NULL) {
        printf("Error: cannot open file with name %s. Please check if this file exists\
            or if the program cannot access it.\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    // Initialize the lexer
    init();

    // Placeholder for current token
    token *token;

    while((token = lex()) -> rep != EOF) {
        printf("Lexeme is: %s, Token is: %d\n", token -> lexeme, token -> rep);
    }

}

void init() {
    // Initialize buffer
    refresh_buffer_firsthalf();
    begin_lexeme_ptr = 0;
    forward_lexeme_ptr = -1;
}

token* lex() {
    current_state = 0;

    token *token_to_return = (token *) malloc(sizeof(token));

    while(1) {
        switch(current_state) {
            case 0:
                if(next_char == '+') current_state = 2;
                else if(next_char == EOF) current_state = __INT_MAX__;

                get_next_char();

                break;
            case 2:
                if(next_char == '+') current_state = 3;
                else {
                    strcpy(token_to_return -> lexeme, "+");
                    token_to_return -> rep = ADD_OP;
                    return token_to_return;
                }

                get_next_char();

                break;
            case 3:
                strcpy(token_to_return -> lexeme, "++");
                token_to_return -> rep = INC_OP;
                return token_to_return;
            default:
                strcpy(token_to_return -> lexeme, "EOF");
                token_to_return -> rep = EOF;
                return token_to_return;
        }
    }
}

void get_next_char() {
    next_char = buffer[++forward_lexeme_ptr];
    // If EOF is encountered
    if(next_char == EOF) {
        // First check if EOF detected is the one
        // intentionally placed at end of buffer first half
        if(forward_lexeme_ptr == BUFFER_LENGTH - 1) {
            // Refresh second half
            refresh_buffer_secondhalf();
            next_char = buffer[(forward_lexeme_ptr = BUFFER_LENGTH)];
        }

        // Then, check if it's the second half
        else if(forward_lexeme_ptr == BUFFER_LENGTH * 2 - 1) {
            refresh_buffer_firsthalf();
            next_char = buffer[(forward_lexeme_ptr = 0)];
        }
    }
}

void get_next_char_nonblank() {
    get_next_char();
    while(next_char == ' ' || next_char == '\n')
        get_next_char();
}

void refresh_buffer_firsthalf() {
    size_t chars_read = fread(buffer,
                              sizeof(char),
                              sizeof(char) * (BUFFER_LENGTH - 1),
                              input_file);
    buffer[(int) chars_read] = EOF;
    buffer[BUFFER_LENGTH - 1] = EOF;
}

void refresh_buffer_secondhalf() {
    size_t chars_read = fread(buffer + BUFFER_LENGTH,
                              sizeof(char),
                              sizeof(char) * (BUFFER_LENGTH - 1),
                              input_file);
    buffer[BUFFER_LENGTH + (int) chars_read] = EOF;
    buffer[BUFFER_LENGTH * 2 - 1] = EOF;
}