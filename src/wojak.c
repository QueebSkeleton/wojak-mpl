// TODO: Adhere to DRY Principles, TOO MUCH REPETITION IN SWITCH!
// Maybe consider doing the transitions with a one or two-dimensional array
// and make the machine a generic code that applies to all
// IF gonna use array or hashmap as transition table, minimize the states
// especially for keywords


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_LENGTH 1024

// MACRO for a transition given that the current input
// matches the specified character, heads to got-o state.
#define TRANSITION(input, to_state) \
    TRANSITION_NONSINGLE(next_char == input, to_state)

// MACRO for a transition provided a boolean condition
// and a goto state. Reads the given character into the lexeme.
#define TRANSITION_NONSINGLE(boolean_condition, to_state) \
    if(boolean_condition) \
        do { \
            lexeme[forward_lexeme_ptr - begin_lexeme_ptr] = next_char; \
            current_state = to_state; \
        } while(0)

// MACRO to accept the current input
// and create a token out of it
#define ACCEPT(token_str, rep) \
    return create_token(lexeme, token_str, rep)

// MACRO for an accept action for a token,
// while retracting the forward pointer by 1
#define RETRACT_THEN_ACCEPT(token_str, rep) \
    do { \
        retract_char(1); \
        ACCEPT(token_str, rep); \
    } while(0)

// MACRO for keywords, each letter state
// Given that keywords follow the same principle for a machine;
// 1: When it reads an appropriate keyword letter, heads to next state
// 2: Else, when it reads another character/number/underscore, heads
//    to identifier state.
// 3: Last case, when a diff. character is encountered, retract then
//    head to identifier state.
#define WORD_STATE(transitions) \
    FINAL_STATE_WITH_TRANSITION( \
        transitions; \
        else TRANSITION_NONSINGLE(isalnum(next_char) || '_', 1), \
        do { retract_char(1); current_state = 1; } while(0))

// MACRO for a generic final state that has transitions
// when reading appropriate input
// otherwise, the accept will be executed.
#define FINAL_STATE_WITH_TRANSITION(transitions, accept) \
    do { \
        get_next_char(); \
        transitions; \
        else accept; \
    } while(0); break

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
#define COMMENT_BEGIN 34
#define COMMENT_END 35
#define INT_LITERAL 36
#define FLOAT_LITERAL 37

#define PRIM_DECLARE_KW 38
#define LIST_DECLARE_KW 39
#define ELEM_KW 40
#define ASSIGN_KW 41
#define IF_KW 42
#define ELIF_KW 43
#define ELSE_KW 44
#define BREAK_KW 45
#define CONTINUE_KW 46
#define SWITCH_KW 47
#define CASE_KW 48
#define WHILE_KW 49
#define TYPE_KW 50
#define IDENTIFIER_KW 51
#define EXPR_KW 52
#define SIZE_KW 53
#define EVAL_KW 54
#define CONST_KW 55

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
        printf("Error: cannot open file with name %s. Please check if this file exists"
            " or if the program cannot access it.\n", argv[1]);
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
                // ??? Implement a generic transition table, so code repetition happens minimally
                // Operators
                TRANSITION('+', 2);
                else TRANSITION('-', 4);
                else TRANSITION('*', 6);
                else TRANSITION('/', 8);
                else TRANSITION('%', 10);
                else TRANSITION('&', 11);
                else TRANSITION('|', 13);
                else TRANSITION('~', 15);
                else TRANSITION('!', 16);
                else TRANSITION('^', 17);
                else TRANSITION('=', 19);
                else TRANSITION('>', 21);
                else TRANSITION('<', 24);
                else TRANSITION('(', 167);
                else TRANSITION(')', 168);
                else TRANSITION('\'', 169);
                else TRANSITION('\"', 170);
                else TRANSITION(',', 171);
                else TRANSITION('[', 172);
                else TRANSITION(']', 173);
                else TRANSITION('.', 174);
                // Start of keywords
                else TRANSITION('p', 27);
                else TRANSITION('l', 39);
                else TRANSITION('e', 51);
                else TRANSITION('a', 55);
                else TRANSITION('i', 61);
                else TRANSITION('b', 67);
                else TRANSITION('c', 72);
                else TRANSITION('s', 80);
                else TRANSITION('w', 90);
                else TRANSITION('t', 95);
                // Start of identifier
                else TRANSITION_NONSINGLE(isalpha(next_char) || next_char == '_', 1);
                // Start of integer or float literal
                else TRANSITION_NONSINGLE(isdigit(next_char), 175);
                // End of File
                else RETRACT_THEN_ACCEPT("End of File", EOF);
                break;
            case 1: FINAL_STATE_WITH_TRANSITION(
                        TRANSITION_NONSINGLE(
                            isalnum(next_char) || next_char == '_', 1),
                        RETRACT_THEN_ACCEPT("Identifier", IDENTIFIER));
            case 2: FINAL_STATE_WITH_TRANSITION(
                        TRANSITION('+', 3),
                        RETRACT_THEN_ACCEPT("Plus Sign", ADD_OP));
            case 3: ACCEPT("Increment Operator", INC_OP);
            case 4: FINAL_STATE_WITH_TRANSITION(
                        TRANSITION('-', 5),
                        RETRACT_THEN_ACCEPT("Hyphen", HYPHEN));
            case 5: FINAL_STATE_WITH_TRANSITION(
                        TRANSITION('>', 166),
                        RETRACT_THEN_ACCEPT("Decrement Operator", DEC_OP));
            case 6: FINAL_STATE_WITH_TRANSITION(
                        TRANSITION('*', 7),
                        RETRACT_THEN_ACCEPT("Asterisk Symbol (or Multiplication Operator)", MUL_OP));
            case 7: ACCEPT("Exponentiation Operator", EXP_OP);
            case 8: FINAL_STATE_WITH_TRANSITION(
                        TRANSITION('/', 9),
                        RETRACT_THEN_ACCEPT("Forward Slash Symbol", FW_SLASH));
            case 9: ACCEPT("Floor Division Operator", FLR_OP);
            case 10: ACCEPT("Modulus Operator", MOD_OP);
            case 11: FINAL_STATE_WITH_TRANSITION(
                        TRANSITION('&', 12),
                        RETRACT_THEN_ACCEPT("Bitwise AND Operator", BITWISE_AND_OP));
            case 12: ACCEPT("Logical AND Operator", LOGIC_AND_OP);
            case 13: FINAL_STATE_WITH_TRANSITION(
                        TRANSITION('|', 14),
                        RETRACT_THEN_ACCEPT("Bitwise OR Operator", BITWISE_OR_OP));
            case 14: ACCEPT("Logical OR Operator", LOGIC_OR_OP);
            case 15: ACCEPT("Bitwise NOT Operator", BITWISE_NOT_OP);
            case 16: FINAL_STATE_WITH_TRANSITION(
                        TRANSITION('=', 18),
                        RETRACT_THEN_ACCEPT("Logical NOT Operator", LOGIC_NOT_OP));
            case 17: ACCEPT("Bitwise XOR Operator", BITWISE_XOR_OP);
            case 18: ACCEPT("Relational Not Equal Operator", NOT_EQ_OP);
            case 19: FINAL_STATE_WITH_TRANSITION(
                        TRANSITION('=', 20),
                        RETRACT_THEN_ACCEPT("Equal Sign", EQ_SIGN));
            case 20: ACCEPT("Relational Equal Operator", EQ_OP);
            case 21: FINAL_STATE_WITH_TRANSITION(
                        TRANSITION('=', 22);
                        else TRANSITION('>', 23),
                        RETRACT_THEN_ACCEPT("Right Angle Bracket", RBRACKET));
            case 22: ACCEPT("Greater Than or Equal Operator", GT_EQ_OP);
            case 23: ACCEPT("Bitwise Shift Right Operator", BITWISE_RIGHT_OP);
            case 24: FINAL_STATE_WITH_TRANSITION(
                        TRANSITION('=', 25);
                        else TRANSITION('<', 26);
                        else TRANSITION('!', 163),
                        RETRACT_THEN_ACCEPT("Left Angle Bracket", LBRACKET));
            case 25: ACCEPT("Less Than or Equal Operator", LT_EQ_OP);
            case 26: ACCEPT("Bitwise Shift Left Operator", BITWISE_LEFT_OP);
            case 27: WORD_STATE(TRANSITION('r', 28));
            case 28: WORD_STATE(TRANSITION('i', 29));
            case 29: WORD_STATE(TRANSITION('m', 30));
            case 30: WORD_STATE(TRANSITION('_', 31));
            case 31: WORD_STATE(TRANSITION('d', 32));
            case 32: WORD_STATE(TRANSITION('e', 33));
            case 33: WORD_STATE(TRANSITION('c', 34));
            case 34: WORD_STATE(TRANSITION('l', 35));
            case 35: WORD_STATE(TRANSITION('a', 36));
            case 36: WORD_STATE(TRANSITION('r', 37));
            case 37: WORD_STATE(TRANSITION('e', 38));
            case 38: ACCEPT("Primitive Declaration Keyword", PRIM_DECLARE_KW);
            case 39: WORD_STATE(TRANSITION('i', 40));
            case 40: WORD_STATE(TRANSITION('s', 41));
            case 41: WORD_STATE(TRANSITION('t', 42));
            case 42: WORD_STATE(TRANSITION('_', 43));
            case 43: WORD_STATE(TRANSITION('d', 44));
            case 44: WORD_STATE(TRANSITION('e', 45));
            case 45: WORD_STATE(TRANSITION('c', 46));
            case 46: WORD_STATE(TRANSITION('l', 47));
            case 47: WORD_STATE(TRANSITION('a', 48));
            case 48: WORD_STATE(TRANSITION('r', 49));
            case 49: WORD_STATE(TRANSITION('e', 50));
            case 50: ACCEPT("List Declaration Keyword", LIST_DECLARE_KW);
            case 51: WORD_STATE(
                        TRANSITION('l', 52);
                        else TRANSITION('x', 109);
                        else TRANSITION('v', 115));
            case 52: WORD_STATE(
                        TRANSITION('e', 53);
                        else TRANSITION('i', 63);
                        else TRANSITION('s', 65));
            case 53: WORD_STATE(TRANSITION('m', 54));
            case 54: ACCEPT("Element Keyword", ELEM_KW);
            case 55: WORD_STATE(TRANSITION('s', 56));
            case 56: WORD_STATE(TRANSITION('s', 57));
            case 57: WORD_STATE(TRANSITION('i', 58));
            case 58: WORD_STATE(TRANSITION('g', 59));
            case 59: WORD_STATE(TRANSITION('n', 60));
            case 60: ACCEPT("Assign Keyword", ASSIGN_KW);
            case 61: WORD_STATE(
                        TRANSITION('f', 62);
                        else TRANSITION('d', 100));
            case 62: ACCEPT("If Keyword", IF_KW);
            case 63: WORD_STATE(TRANSITION('f', 64));
            case 64: ACCEPT("Else If Keyword", ELIF_KW);
            case 65: WORD_STATE(TRANSITION('e', 66));
            case 66: ACCEPT("Else Keyword", ELSE_KW);
            case 67: WORD_STATE(TRANSITION('r', 68));
            case 68: WORD_STATE(TRANSITION('e', 69));
            case 69: WORD_STATE(TRANSITION('a', 70));
            case 70: WORD_STATE(TRANSITION('k', 71));
            case 71: ACCEPT("Break Keyword", BREAK_KW);
            case 72: WORD_STATE(
                        TRANSITION('o', 73);
                        else TRANSITION('a', 86));
            case 73: WORD_STATE(TRANSITION('n', 74));
            case 74: WORD_STATE(
                        TRANSITION('t', 75);
                        else TRANSITION('s', 118));
            case 75: WORD_STATE(TRANSITION('i', 76));
            case 76: WORD_STATE(TRANSITION('n', 77));
            case 77: WORD_STATE(TRANSITION('u', 78));
            case 78: WORD_STATE(TRANSITION('e', 79));
            case 79: ACCEPT("Continue", CONTINUE_KW);
            case 80: WORD_STATE(
                        TRANSITION('w', 81);
                        else TRANSITION('i', 112));
            case 81: WORD_STATE(TRANSITION('i', 82));
            case 82: WORD_STATE(TRANSITION('t', 83));
            case 83: WORD_STATE(TRANSITION('c', 84));
            case 84: WORD_STATE(TRANSITION('h', 85));
            case 85: ACCEPT("Switch Keyword", SWITCH_KW);;
            case 86: WORD_STATE(TRANSITION('s', 87));
            case 87: WORD_STATE(TRANSITION('e', 88));
            case 89: ACCEPT("Case Keyword", CASE_KW);
            case 90: WORD_STATE(TRANSITION('h', 91));
            case 91: WORD_STATE(TRANSITION('i', 92));
            case 92: WORD_STATE(TRANSITION('l', 93));
            case 93: WORD_STATE(TRANSITION('e', 94));
            case 94: ACCEPT("While Keyword", WHILE_KW);
            case 95: WORD_STATE(TRANSITION('y', 96));
            case 96: WORD_STATE(TRANSITION('p', 97));
            case 97: WORD_STATE(TRANSITION('e', 98));
            case 98: ACCEPT("Type Keyword", TYPE_KW);
            // state 99 skipped
            case 100: WORD_STATE(TRANSITION('e', 101));
            case 101: WORD_STATE(TRANSITION('n', 102));
            case 102: WORD_STATE(TRANSITION('t', 103));
            case 103: WORD_STATE(TRANSITION('i', 104));
            case 104: WORD_STATE(TRANSITION('f', 105));
            case 105: WORD_STATE(TRANSITION('i', 106));
            case 106: WORD_STATE(TRANSITION('e', 107));
            case 107: WORD_STATE(TRANSITION('r', 108));
            case 108: ACCEPT("Identifier Keyword", IDENTIFIER_KW);
            case 109: WORD_STATE(TRANSITION('p', 110));
            case 110: WORD_STATE(TRANSITION('r', 111));
            case 111: ACCEPT("Expression Keyword", EXPR_KW);
            case 112: WORD_STATE(TRANSITION('z', 113));
            case 113: WORD_STATE(TRANSITION('e', 114));
            case 114: ACCEPT("Size Keyword", SIZE_KW);
            case 115: WORD_STATE(TRANSITION('a', 116));
            case 116: WORD_STATE(TRANSITION('l', 117));
            case 117: ACCEPT("Evaluate Keyword", EVAL_KW);
            case 118: WORD_STATE(TRANSITION('t', 119));
            case 119: ACCEPT("Constant", CONST_KW);
            case 163: TRANSITION('-', 164);
            case 164: TRANSITION('-', 165);
            case 165: ACCEPT("Comment Begin", COMMENT_BEGIN);
            case 166: ACCEPT("Comment End", COMMENT_END);
            case 167: ACCEPT("Left Parenthesis", LPAREN);
            case 168: ACCEPT("Right Parenthesis", RPAREN);
            case 169: ACCEPT("Single Quote", SINGLE_QUOTE);
            case 170: ACCEPT("Double Quote", DOUBLE_QUOTE);
            case 171: ACCEPT("Comma", COMMA);
            case 172: ACCEPT("Left Square Bracket", LBRACKET);
            case 173: ACCEPT("Right Square Bracket", RBRACKET);
            case 174: ACCEPT("Dot", DOT);
            case 175: FINAL_STATE_WITH_TRANSITION(
                        TRANSITION_NONSINGLE(isdigit(next_char), 175);
                        else TRANSITION('.', 176),
                        RETRACT_THEN_ACCEPT("Integer Literal", INT_LITERAL));
            case 176: FINAL_STATE_WITH_TRANSITION(
                        TRANSITION_NONSINGLE(isdigit(next_char), 177),
                        { retract_char(1); lexeme[forward_lexeme_ptr - begin_lexeme_ptr] = '0'; current_state = 177; });
            case 177: FINAL_STATE_WITH_TRANSITION(
                        TRANSITION_NONSINGLE(isdigit(next_char), 145),
                        RETRACT_THEN_ACCEPT("Float Literal", FLOAT_LITERAL));
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