// TODO: Document properly
// TODO: Might re-evaluate states later so there are no skipped numbers(?)
// NOT NECESSARY, but can be nice for OCD people

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_LENGTH 1024

#define TRANSITION(input, to_state) TRANSITION_NONSINGLE(next_char == input, to_state)
#define TRANSITION_NONSINGLE(boolean_condition, to_state) \
    if(boolean_condition) { \
        lexeme[forward_lexeme_ptr - begin_lexeme_ptr] = next_char; \
        current_state = to_state; \
    }
#define ACCEPT(token_str, rep) \
    return create_token(lexeme, token_str, rep);
#define RETRACT_THEN_ACCEPT(token_str, rep) \
    { retract_char(1); ACCEPT(token_str, rep); }

#define IDENTIFIER 0
#define ADD_OP 1
#define INC_OP 2
#define HYPHEN 3
#define DEC_OP 4
#define MUL_OP 5
#define EXP_OP 6
#define FW_SLASH 7
#define FLR_OP 8
#define MOD_OP 9
#define BITWISE_AND_OP 10
#define LOGIC_AND_OP 11
#define BITWISE_OR_OP 12
#define LOGIC_OR_OP 13
#define BITWISE_NOT_OP 14
#define LOGIC_NOT_OP 15
#define NOT_EQ_OP 16
#define BITWISE_XOR_OP 17
#define EQ_SIGN 18
#define EQ_OP 19
#define RANGLE 20
#define GT_EQ_OP 21
#define BITWISE_RIGHT_OP 22
#define LANGLE 23
#define LT_EQ_OP 24
#define BITWISE_LEFT_OP 25
#define LPAREN 26
#define RPAREN 27
#define SINGLE_QUOTE 28
#define DOUBLE_QUOTE 29
#define COMMA 30
#define LBRACKET 31
#define RBRACKET 32
#define DOT 33
#define INT_LITERAL 34
#define FLOAT_LITERAL 35

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
void retract_char();
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
 * @brief Placeholder whether to refresh a buffer or not. This is particularly
 * used with retracting the pointer.
 * 
 */
int should_refresh_buffer[2];

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
    // Initialize reset flags
    should_refresh_buffer[0] = 1;
    should_refresh_buffer[1] = 1;
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

    char lexeme[80] = { '\0' };

    while(1) {
        switch(current_state) {
            case 0:
                get_next_char_nonblank();

                begin_lexeme_ptr = forward_lexeme_ptr;

                // TODO: Implement a HashMap Data Structure
                TRANSITION('+', 2)
                else TRANSITION('-', 4)
                else TRANSITION('*', 6)
                else TRANSITION('/', 8)
                else TRANSITION('%', 10)
                else TRANSITION('&', 11)
                else TRANSITION('|', 13)
                else TRANSITION('~', 15)
                else TRANSITION('!', 16)
                else TRANSITION('^', 17)
                else TRANSITION('=', 19)
                else TRANSITION('>', 21)
                else TRANSITION('<', 24)
                else TRANSITION('(', 135)
                else TRANSITION(')', 136)
                else TRANSITION('\'', 137)
                else TRANSITION('\"', 138)
                else TRANSITION(',', 139)
                else TRANSITION('[', 140)
                else TRANSITION(']', 141)
                else TRANSITION('.', 142)
                else TRANSITION_NONSINGLE(isalpha(next_char) || next_char == '_', 1)
                else TRANSITION_NONSINGLE(isdigit(next_char), 143)
                else RETRACT_THEN_ACCEPT("End of File",
                                         EOF)
                break;

            case 1:
                get_next_char();
                TRANSITION_NONSINGLE(isalnum(next_char) || next_char == '_', 1)
                else RETRACT_THEN_ACCEPT("Identifier",
                                         IDENTIFIER)
                break;

            case 2:
                get_next_char();
                TRANSITION('+', 3)
                else RETRACT_THEN_ACCEPT("Plus Sign (or Addition Operator)",
                                         ADD_OP)
                break;

            case 3:
                ACCEPT("Increment Operator",
                       INC_OP)

            case 4:
                get_next_char();
                TRANSITION('-', 5)
                else RETRACT_THEN_ACCEPT("Hyphen",
                                         HYPHEN)
                break;

            case 5:
                ACCEPT("Decrement Operator",
                       DEC_OP)

            case 6:
                get_next_char();
                TRANSITION('*', 7)
                else RETRACT_THEN_ACCEPT("Asterisk Symbol (or Multiplication Operator)",
                                         MUL_OP)
                break;
            case 7:
                ACCEPT("Exponentiation Operator",
                       EXP_OP)

            case 8:
                get_next_char();
                TRANSITION('/', 9)
                else RETRACT_THEN_ACCEPT("Forward Slash Symbol",
                                         FW_SLASH)
                break;

            case 9:
                ACCEPT("Floor Division Operator",
                       FLR_OP)

            case 10:
                ACCEPT("Modulus Operator",
                       MOD_OP)

            case 11:
                get_next_char();
                TRANSITION('&', 12)
                else RETRACT_THEN_ACCEPT("Bitwise AND Operator",
                                         BITWISE_AND_OP)
                break;

            case 12:
                ACCEPT("Logical AND Operator",
                       LOGIC_AND_OP)

            case 13:
                get_next_char();
                TRANSITION('|', 14)
                else RETRACT_THEN_ACCEPT("Bitwise OR Operator",
                                         BITWISE_OR_OP)
                break;

            case 14:
                ACCEPT("Logical OR Operator",
                       LOGIC_OR_OP)

            case 15:
                ACCEPT("Bitwise NOT Operator",
                       BITWISE_NOT_OP)

            case 16:
                get_next_char();
                TRANSITION('=', 18)
                else RETRACT_THEN_ACCEPT("Logical NOT Operator",
                                         LOGIC_NOT_OP)
                break;

            case 17:
                ACCEPT("Bitwise XOR Operator",
                       BITWISE_XOR_OP)

            case 18:
                ACCEPT("Relational Not Equal Operator",
                       NOT_EQ_OP)

            case 19:
                get_next_char();
                TRANSITION('=', 20)
                else RETRACT_THEN_ACCEPT("Equal Sign",
                                         EQ_SIGN)
                break;

            case 20:
                ACCEPT("Relational Equal Operator",
                       EQ_OP)

            case 21:
                get_next_char();
                TRANSITION('=', 22)
                else TRANSITION('>', 23)
                else RETRACT_THEN_ACCEPT("Right Angle Bracket",
                                         RBRACKET)
                break;

            case 22:
                ACCEPT("Relational Greater Than or Equal Operator",
                       GT_EQ_OP)

            case 23:
                ACCEPT("Bitwise Shift Right Operator",
                       BITWISE_RIGHT_OP)

            case 24:
                get_next_char();
                TRANSITION('=', 25)
                else TRANSITION('<', 26)
                else RETRACT_THEN_ACCEPT("Left Angle Bracket",
                                         LBRACKET)
                break;

            case 25:
                ACCEPT("Relational Less Than or Equal Operator",
                       LT_EQ_OP)

            case 26:
                ACCEPT("Bitwise Shift Left Operator",
                       BITWISE_LEFT_OP)

            case 135:
                ACCEPT("Left Parenthesis",
                       LPAREN)

            case 136:
                ACCEPT("Right Parenthesis",
                       RPAREN)

            case 137:
                ACCEPT("Single Quote",
                       SINGLE_QUOTE)

            case 138:
                ACCEPT("Double Quote",
                       DOUBLE_QUOTE)

            case 139:
                ACCEPT("Comma",
                       COMMA)
                
            case 140:
                ACCEPT("Left Square Bracket",
                       LBRACKET)
                
            case 141:
                ACCEPT("Right Square Bracket",
                       RBRACKET)

            case 142:
                ACCEPT("Dot",
                       DOT);

            case 143:
                get_next_char();
                TRANSITION_NONSINGLE(isdigit(next_char), 143)
                else TRANSITION('.', 144)
                else ACCEPT("Integer Literal",
                            INT_LITERAL)
                break;

            case 144:
                get_next_char();
                TRANSITION_NONSINGLE(isdigit(next_char), 145)
                else {
                    lexeme[forward_lexeme_ptr - begin_lexeme_ptr] = '0';
                    RETRACT_THEN_ACCEPT("Float Literal",
                                        FLOAT_LITERAL)
                }
                break;

            case 145:
                get_next_char();
                TRANSITION_NONSINGLE(isdigit(next_char), 145)
                else RETRACT_THEN_ACCEPT("Float Literal",
                                         FLOAT_LITERAL)
                break;
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
            if(should_refresh_buffer[1]) refresh_buffer(1);
            should_refresh_buffer[1] = 1; // Reset flag
            // Reset pointer to the first character of the second buffer half
            next_char = buffer[(forward_lexeme_ptr = BUFFER_LENGTH)];
        }

        // Then, check if it's the second half
        else if(forward_lexeme_ptr == BUFFER_LENGTH * 2 - 1) {
            // Refresh first half
            if(should_refresh_buffer[0]) refresh_buffer(0);
            should_refresh_buffer[0] = 1; // Reset flag
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
 * @brief Retracts the forward pointer.
 * 
 * In the case when the pointer has to traverse another buffer upon
 * retracting, they should not be refreshed when get_next_char is called.
 * 
 * @param steps the number of steps to retract
 * @return int if an inter-buffer traversal occured
 */
void retract_char(int steps) {
    if(steps < 0)
        return;

    // If retracting from first to second buffer
    // e.g. retracting from index 0???
    if(forward_lexeme_ptr - steps < 0) {
        // 1st half buffer should not be refreshed
        should_refresh_buffer[0] = 0;
        // Retract the pointer
        forward_lexeme_ptr = (BUFFER_LENGTH * 2 - 1) - steps - forward_lexeme_ptr;
        next_char = buffer[forward_lexeme_ptr];
    }

    // Else if retracting from second to first buffer
    // e.g. retracting from BUFFER_LENGTH
    else if(forward_lexeme_ptr >= BUFFER_LENGTH && forward_lexeme_ptr - steps < BUFFER_LENGTH) {
        // 2nd half buffer should not be refreshed
        should_refresh_buffer[1] = 0;
        // Retract the pointer
        forward_lexeme_ptr = forward_lexeme_ptr - steps - 1;
        next_char = buffer[forward_lexeme_ptr];
    }

    // Else, simply retract
    else next_char = buffer[forward_lexeme_ptr -= steps];
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
    size_t chars_read = fread(buffer + (BUFFER_LENGTH * (half)),
                              sizeof(char),
                              sizeof(char) * (BUFFER_LENGTH - 1),
                              input_file);
    // If characters read were less than the buffer size,
    // then the actual end-of-file is encountered.
    // Place an EOF there to signify.
    // NOTE: this is intentionally done because fread() does not place
    // EOF on the string, but instead uses \0
    buffer[BUFFER_LENGTH * (half) + (int) chars_read] = EOF;
}