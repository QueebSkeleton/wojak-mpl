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
#define MUL_OP 5
#define EXP_OP 6
#define DIV_OP 7
#define FLR_OP 8

typedef struct {
    char lexeme[80];
    char token_str[80];
    int rep;
} token;

void init();
token* lex();
token* create_token();
void get_next_char();
void get_next_char_nonblank();
void refresh_buffer(int);

/**
 * @brief The input file, assumingly in .wojak language rules.
 * 
 */
FILE *input_file;

/**
 * @brief Placeholder for the lexer's current character to consider.
 * 
 */
char next_char;

/**
 * @brief The current state for the lexical analyzer.
 * 
 */
int current_state;

/**
 * @brief Input buffer for the file. Used in accordance with the two-buffer scheme.
 * 
 */
char buffer[BUFFER_LENGTH * 2];

/**
 * @brief A pointer to the beginning of the currently scanned lexeme.
 * 
 * Utilized in the buffer, and is used for backtracking when a path fails.
 * 
 */
int begin_lexeme_ptr;

/**
 * @brief A pointer to scan a lexeme character by character.
 * 
 * Utilized in the buffer.
 * 
 */
int forward_lexeme_ptr;

/**
 * @brief Entry point of the compiler. For now, it prints all tokens in the input file.
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
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

    // Print all tokens in the file
    token *current_token;
    while((current_token = lex()) -> rep != EOF) {
        printf("Lexeme is: %s, Token is: %s\n", current_token->lexeme, current_token->token_str);
        free(current_token);
    }

    // Close the file
    fclose(input_file);

}

/**
 * @brief Initializes the lexical analyzer
 * 
 */
void init() {
    // Initialize pointer at end of second half, which will reset later for convenience
    forward_lexeme_ptr = BUFFER_LENGTH * 2 - 2;

    // BUFFER STUFF
    // Initialize EOF sentinels
    buffer[BUFFER_LENGTH - 1] = EOF;
    buffer[BUFFER_LENGTH * 2 - 1] = EOF;
}

/**
 * @brief Lexes the input and returns the first token encountered.
 * 
 * The function is modeled as an implementation of a Deterministic
 * Finite Automaton (DFA).
 * 
 * @return token the parsed token
 */
token* lex() {
    current_state = 0;

    while(1) {
        switch(current_state) {
            case 0:
                get_next_char_nonblank();

                // OPERATORS
                if(next_char == '+') current_state = 2;
                else if(next_char == '-') current_state = 4;
                else if(next_char == '*') current_state = 6;
                else if(next_char == '/') current_state = 8;

                // EOF encountered
                else if(next_char == EOF) return create_token("EOF", "End-of-File", EOF);

                break;

            case 2:
                get_next_char();

                // Another + encountered
                if(next_char == '+') current_state = 3;
                // Else, return + token
                else return create_token("+", "Plus Sign (or Addition Operator)", ADD_OP);

                break;

            case 3:
                // return ++ token
                return create_token("++", "Increment Operator", INC_OP);

            case 4:
                get_next_char();

                // Another - encountered
                if(next_char == '-') current_state = 5;
                // Else, return - token
                else return create_token("-", "Dash Symbol (or Subtraction Operator)", SUB_OP);

                break;

            case 5:
                // return -- token
                return create_token("--", "Decrement Operator", DEC_OP);

            case 6:
                get_next_char();

                // Another * encountered
                if(next_char == '*') current_state = 7;
                // Else, return * token
                else return create_token("*", "Asterisk Symbol (or Multiplication Operator)", MUL_OP);

                break;
            case 7:
                // return ** token
                return create_token("**", "Exponentiation Operator", EXP_OP);

            case 8:
                get_next_char();

                // Another / encountered
                if(next_char == '/') current_state = 9;
                // Else, return / token
                else return create_token("/", "Forward Slash Symbol (or Division Operator)", DIV_OP);

                break;

            case 9:
                // return ** token
                return create_token("//", "Floor Division Operator", FLR_OP);
            
            default:
                // Placeholder, will not be reached
                return create_token("EOF", "End-of-File", EOF);
        }
    }
}

/**
 * @brief Create a token object
 * 
 * @param lexeme the lexeme string
 * @param token_str a string representation of the token
 * @param rep actual representation of the token
 * @return token 
 */
token* create_token(char *lexeme, char *token_str, int rep) {
    token *new_token = (token*) malloc(sizeof(token));
    strcpy(new_token -> lexeme, lexeme);
    strcpy(new_token -> token_str, token_str);
    new_token -> rep = rep;
    return new_token;
}

/**
 * @brief Gets the next character from the file.
 * 
 * The Lexical Analyzer utilizes the two-buffer scheme. EOF sentinels
 * are placed on the end of each half of the buffer to reduce if-else
 * comparisons (see Two-Buffer Schemes, EOF Sentinels).
 * 
 */
void get_next_char() {
    next_char = buffer[++forward_lexeme_ptr];
    // If EOF is encountered
    if(next_char == EOF) {
        // First check if EOF detected is the one
        // intentionally placed at end of buffer first half
        if(forward_lexeme_ptr == BUFFER_LENGTH - 1) {
            // Refresh second half
            refresh_buffer(2);
            // Reset pointer to the first character of the second buffer half
            next_char = buffer[(forward_lexeme_ptr = BUFFER_LENGTH)];
        }

        // Then, check if it's the second half
        else if(forward_lexeme_ptr == BUFFER_LENGTH * 2 - 1) {
            // Refresh first half
            refresh_buffer(1);
            // Reset pointer to the first character of the first buffer half
            next_char = buffer[(forward_lexeme_ptr = 0)];
        }
    }
}

/**
 * @brief Repetitively calls get_next_char() until a non-space is encountered.
 * 
 */
void get_next_char_nonblank() {
    get_next_char();
    while(next_char == ' ' || next_char == '\n')
        get_next_char();
}

/**
 * @brief Refreshes a part of the buffer.
 * 
 * @param half whether the first half (1) or the second (2) is to be refreshed.
 */
void refresh_buffer(int half) {
    // Attempt to read a chunk of characters from the file
    // with the specified BUFFER_LENGTH
    // And store how many characters were actually read.
    size_t chars_read = fread(buffer + (BUFFER_LENGTH * (half - 1)),
                              sizeof(char),
                              sizeof(char) * (BUFFER_LENGTH - 1),
                              input_file);
    // If characters read were less than the buffer size,
    // then the actual end-of-file is encountered.
    // Place an EOF there to signify.
    // NOTE: this is intentionally done because fread() does not place
    // EOF on the string, but instead uses \0
    buffer[BUFFER_LENGTH * (half - 1) + (int) chars_read] = EOF;
}