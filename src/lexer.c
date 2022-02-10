
#ifndef GLOBALS_H
#define GLOBALS_H
#include "globals.h"
#endif

#ifndef TOKENS_H
#define TOKENS_H
#include "tokens.h"
#endif

#ifndef STDLIB_H
#define STDLIB_H
#include <stdlib.h>
#endif

#ifndef STDBOOL_H
#define STDBOOL_H
#include <stdbool.h>
#endif

#ifndef STRING_H
#define STRING_H
#include <string.h>
#endif

#ifndef CTYPE_H
#define CTYPE_H
#include <ctype.h>
#endif

#ifndef INTTYPES_H
#define INTTYPES_h
#include <inttypes.h>
#endif

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
    do { \
        strcpy(lex_text, lexeme); \
        strcpy(lex_token_desc, token_str); \
        lex_token = rep; \
        return; \
    } while(0)

// MACRO for an accept action for a token,
// while retracting the forward pointer by 1
#define RETRACT_THEN_ACCEPT(token_str, rep) \
    do { \
        retract_char(1); \
        ACCEPT(token_str, rep); \
    } while(0)

// MACRO for a generic final state that has transitions
// when reading appropriate input
// otherwise, the accept will be executed.
#define FINAL_STATE_WITH_TRANSITION(transitions, accept) \
    get_next_char(); \
    transitions; \
    else accept; \
    break

// MACRO for keywords, each letter state
// Given that keywords follow the same principle for a machine;
// 1: When it reads an appropriate keyword letter, heads to next state
// 2: When a diff. character is encountered, retract then
//    head to identifier state.
#define WORD_STATE(transitions) \
    FINAL_STATE_WITH_TRANSITION( \
        transitions, \
        do { retract_char(1); current_state = 1; } while(0))

#define ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(token_str, rep) \
    FINAL_STATE_WITH_TRANSITION( \
        TRANSITION_NONSINGLE(isalnum(next_char) || next_char == '_', 1), \
        RETRACT_THEN_ACCEPT(token_str, rep))

void init();
void lex();
void get_next_char();
void get_next_char_nonblank();
void retract_char(uint16_t);
void refresh_buffer(uint8_t);

extern FILE *input_file;
extern FILE *sym_file;

/**
 * @brief Placeholder for the current lexeme.
 */
char lex_text[80];

/**
 * @brief Placeholder for the current token.
 * For printing purposes only.
 */
char lex_token_desc[80];

/**
 * @brief The current token value.
 * @see tokens.h
 */
int8_t lex_token;

/**
 * @brief Placeholder for the lexer's current character to consider.
 * 
 */
char next_char;

/**
 * @brief The current state for the lexical analyzer.
 * 
 */
uint8_t current_state;

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
bool should_refresh_buffer[2];

/**
 * @brief A pointer to the beginning of the currently scanned lexeme.
 * 
 * Utilized in the buffer, and is used for backtracking when a path fails.
 * 
 */
uint16_t begin_lexeme_ptr;

/**
 * @brief A pointer to scan a lexeme character by character.
 * 
 * Utilized in the buffer.
 * 
 */
uint16_t forward_lexeme_ptr;

/**
 * @brief Entry point of the compiler. For now, it prints all tokens in the input file.
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[]) {
    // Check if arguments are properly given
    if(argc < 2) {
        printf("Error: no given files.\n");
        printf("Usage is: wojak filename.wojak\n");
        exit(EXIT_FAILURE);
    }

    // Open input file in read mode
    if((input_file = fopen(argv[1], "r")) == NULL) {
        printf("Error: cannot open file with name %s. Please check if this file exists"
            " or if the program cannot access it.\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    // Extract the file name only, use it for output file naming convention
    char filename[256] = { '\0' };
    for(int i = 0; i < strlen(argv[1]); i++) {
        if(argv[1][i] == '.')
            break;
        filename[i] = argv[1][i];
    }

    // Open symbol table output file
    if((sym_file = fopen(strcat(filename, ".symwojak"), "w")) == NULL) {
        printf("Error: cannot create symbol table output file %s. Please check"
            " if the program cannot access it.\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    // Initialize the lexer
    init();

    // Print all tokens in the file
    lex();
    while(lex_token != EOF) {
        printf("Lexeme is: %s, Token is: %s\n", lex_text, lex_token_desc);
        fprintf(sym_file, "%s,%d,%s\n", lex_text, lex_token, lex_token_desc);
        lex();
    }

    // Close the files
    fclose(sym_file);
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
void lex() {
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
                else TRANSITION('{', 175);
                else TRANSITION('}', 176);
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
                else TRANSITION('f', 190);
                else TRANSITION('b', 195);
                // Start of identifier
                else TRANSITION_NONSINGLE(isalpha(next_char) || next_char == '_', 1);
                // Start of integer or float literal
                else TRANSITION_NONSINGLE(isdigit(next_char), 177);
                // End of File
                else TRANSITION(EOF, 206);
                // Invalid token
                else printf("Unrecognized token %c\n.", next_char);
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
            case 28: WORD_STATE(TRANSITION('i', 29);
                        else TRANSITION('o', 158));
            case 29: WORD_STATE(TRANSITION('m', 30));
            case 30: WORD_STATE(TRANSITION('_', 31));
            case 31: WORD_STATE(TRANSITION('d', 32));
            case 32: WORD_STATE(TRANSITION('e', 33));
            case 33: WORD_STATE(TRANSITION('c', 34));
            case 34: WORD_STATE(TRANSITION('l', 35));
            case 35: WORD_STATE(TRANSITION('a', 36));
            case 36: WORD_STATE(TRANSITION('r', 37));
            case 37: WORD_STATE(TRANSITION('e', 38));
            case 38: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "prim_declare Keyword", PRIM_DECLARE_KW);
            case 39: WORD_STATE(TRANSITION('i', 40));
            case 40: WORD_STATE(TRANSITION('s', 41));
            case 41: FINAL_STATE_WITH_TRANSITION(
                        TRANSITION('t', 42);
                        else TRANSITION_NONSINGLE(isalnum(next_char) || '_', 1),
                        RETRACT_THEN_ACCEPT("list", LIST_RW)); 
            case 42: WORD_STATE(TRANSITION('_', 43));
            case 43: WORD_STATE(TRANSITION('d', 44));
            case 44: WORD_STATE(TRANSITION('e', 45));
            case 45: WORD_STATE(TRANSITION('c', 46));
            case 46: WORD_STATE(TRANSITION('l', 47));
            case 47: WORD_STATE(TRANSITION('a', 48));
            case 48: WORD_STATE(TRANSITION('r', 49));
            case 49: WORD_STATE(TRANSITION('e', 50));
            case 50: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "list_declare Keyword", LIST_DECLARE_KW);
            case 51: WORD_STATE(
                        TRANSITION('l', 52);
                        else TRANSITION('x', 109);
                        else TRANSITION('v', 115);
                        else TRANSITION('n', 133));
            case 52: WORD_STATE(
                        TRANSITION('e', 53);
                        else TRANSITION('i', 63);
                        else TRANSITION('s', 65));
            case 53: WORD_STATE(TRANSITION('m', 54));
            case 54: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "elem Keyword", ELEM_KW);
            case 55: WORD_STATE(TRANSITION('s', 56);
                        else TRANSITION('n', 120));
            case 56: WORD_STATE(TRANSITION('s', 57));
            case 57: WORD_STATE(TRANSITION('i', 58));
            case 58: WORD_STATE(TRANSITION('g', 59));
            case 59: WORD_STATE(TRANSITION('n', 60));
            case 60: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "assign Keyword", ASSIGN_KW);
            case 61: WORD_STATE(
                        TRANSITION('f', 62);
                        else TRANSITION('d', 100);
                        else TRANSITION('n', 146));
            case 62: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "if Keyword", IF_KW);
            case 63: WORD_STATE(TRANSITION('f', 64));
            case 64: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "elif Keyword", ELIF_KW);
            case 65: WORD_STATE(TRANSITION('e', 66));
            case 66: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "else Keyword", ELSE_KW);
            case 67: WORD_STATE(TRANSITION('r', 68);
                        else TRANSITION('e', 129));
            case 68: WORD_STATE(TRANSITION('e', 69));
            case 69: WORD_STATE(TRANSITION('a', 70));
            case 70: WORD_STATE(TRANSITION('k', 71));
            case 71: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "break Keyword", BREAK_KW);
            case 72: WORD_STATE(
                        TRANSITION('o', 73);
                        else TRANSITION('n', 86));
            case 73: WORD_STATE(TRANSITION('n', 74));
            case 74: WORD_STATE(
                        TRANSITION('t', 75);
                        else TRANSITION('s', 118));
            case 75: WORD_STATE(TRANSITION('i', 76));
            case 76: WORD_STATE(TRANSITION('n', 77));
            case 77: WORD_STATE(TRANSITION('u', 78));
            case 78: WORD_STATE(TRANSITION('e', 79));
            case 79: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "continue Keyword", CONTINUE_KW);
            case 80: WORD_STATE(
                        TRANSITION('w', 81);
                        else TRANSITION('i', 112);
                        else TRANSITION('t', 185));
            case 81: WORD_STATE(TRANSITION('i', 82));
            case 82: WORD_STATE(TRANSITION('t', 83));
            case 83: WORD_STATE(TRANSITION('c', 84));
            case 84: WORD_STATE(TRANSITION('h', 85));
            case 85: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "switch Keyword", SWITCH_KW);
            case 86: WORD_STATE(TRANSITION('s', 87));
            case 87: WORD_STATE(TRANSITION('e', 88));
            case 89: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "case Keyword", CASE_KW);
            case 90: WORD_STATE(TRANSITION('h', 91);
                        else TRANSITION('i', 155));
            case 91: WORD_STATE(TRANSITION('i', 92));
            case 92: WORD_STATE(TRANSITION('l', 93));
            case 93: WORD_STATE(TRANSITION('e', 94));
            case 94: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "while Keyword", WHILE_KW);
            case 95: WORD_STATE( 
                        TRANSITION('y', 96);
                        else TRANSITION('h', 142);
                        else TRANSITION('o', 145);
                        else TRANSITION('r', 199));
            case 96: WORD_STATE(TRANSITION('p', 97));
            case 97: WORD_STATE(TRANSITION('e', 98));
            case 98: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "type Keyword", TYPE_KW);
            // state 99 skipped
            case 100: WORD_STATE(TRANSITION('e', 101));
            case 101: WORD_STATE(TRANSITION('n', 102));
            case 102: WORD_STATE(TRANSITION('t', 103));
            case 103: WORD_STATE(TRANSITION('i', 104));
            case 104: WORD_STATE(TRANSITION('f', 105));
            case 105: WORD_STATE(TRANSITION('i', 106));
            case 106: WORD_STATE(TRANSITION('e', 107));
            case 107: WORD_STATE(TRANSITION('r', 108));
            case 108: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "identifier Keyword", IDENTIFIER_KW);
            case 109: WORD_STATE(TRANSITION('p', 110));
            case 110: WORD_STATE(TRANSITION('r', 111));
            case 111: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "expr Keyword", EXPR_KW);
            case 112: WORD_STATE(TRANSITION('z', 113));
            case 113: WORD_STATE(TRANSITION('e', 114));
            case 114: FINAL_STATE_WITH_TRANSITION(
                        TRANSITION('o', 127);
                        else TRANSITION_NONSINGLE(isalnum(next_char) || next_char == '_', 1),
                        RETRACT_THEN_ACCEPT("size Keyword", SIZE_KW));
            case 115: WORD_STATE(TRANSITION('a', 116));
            case 116: WORD_STATE(TRANSITION('l', 117));
            case 117: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "eval Keyword", EVAL_KW);
            case 118: WORD_STATE(TRANSITION('t', 119));
            case 119: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "const keyword", CONST_KW);
            case 120: WORD_STATE(TRANSITION('d', 121));
            case 121: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "and Reserved word", AND_RW);
            case 122: WORD_STATE(TRANSITION('r', 123));
            case 123: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "or Reserved Word", OR_RW);
            case 124: WORD_STATE(TRANSITION('o', 125));
            case 125: WORD_STATE(TRANSITION('t', 126));
            case 126: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "not Reserved Word", NOT_RW);
            case 127: WORD_STATE(TRANSITION('f', 128));
            case 128: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "sizeof Reserved Word", SIZEOF_RW);
            case 129: WORD_STATE(TRANSITION('g', 130));
            case 130: WORD_STATE(TRANSITION('i', 131));
            case 131: WORD_STATE(TRANSITION('n', 132));
            case 132: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "begin Reserved Word", BEGIN_RW);
            case 133: WORD_STATE(TRANSITION('d', 134));
            case 134: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "end Reserved Word", END_RW);
            case 142: WORD_STATE(TRANSITION('e', 143));
            case 143: WORD_STATE(TRANSITION('n', 144));
            case 144: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "then Noise Word", THEN_NW);
            case 145: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "to Noise Word", TO_NW);
            case 146: WORD_STATE(
                        TRANSITION('i', 147);
                        else TRANSITION('t', 184));
            case 147: WORD_STATE(TRANSITION('t', 148));
            case 148: WORD_STATE(TRANSITION('i', 149));
            case 149: WORD_STATE(TRANSITION('a', 150));
            case 150: WORD_STATE(TRANSITION('l', 151));
            case 151: WORD_STATE(TRANSITION('i', 152));
            case 152: WORD_STATE(TRANSITION('z', 153));
            case 153: WORD_STATE(TRANSITION('e', 154));
            case 154: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "initialize Noise Word", INITIALIZE_NW);
            case 155: WORD_STATE(TRANSITION('t', 156));
            case 156: WORD_STATE(TRANSITION('h', 157));
            case 157: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "with Noise Word", WITH_NW);
            case 158: WORD_STATE(TRANSITION('c', 159));
            case 159: WORD_STATE(TRANSITION('e', 160));
            case 160: WORD_STATE(TRANSITION('e', 161));
            case 161: WORD_STATE(TRANSITION('d', 162));
            case 162: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "proceed Noise Word", PROCEED_NW);
            case 163: TRANSITION('-', 164);
            case 164: TRANSITION('-', 165);
            case 165: {
                get_next_char();
                TRANSITION('-', 166);
                else TRANSITION(EOF, 183);
                else TRANSITION_NONSINGLE(next_char != '-', 165);
                break;
            }
            case 166: {
                get_next_char();
                TRANSITION('-', 181);
                else TRANSITION(EOF, 183);
                else TRANSITION_NONSINGLE(next_char != '-', 165);
                break;
            }
            case 167: ACCEPT("Left Parenthesis", LPAREN);
            case 168: ACCEPT("Right Parenthesis", RPAREN);
            case 169: get_next_char();
                      TRANSITION_NONSINGLE(next_char != '\'', 169);
                      else TRANSITION('\'', 180);
                      break;
            case 170: get_next_char();
                      TRANSITION_NONSINGLE(next_char != '\"', 170);
                      else TRANSITION('\"', 180);
                      break;
            case 171: ACCEPT("Comma", COMMA);
            case 172: ACCEPT("Left Square Bracket", LBRACKET);
            case 173: ACCEPT("Right Square Bracket", RBRACKET);
            case 174: ACCEPT("Dot", DOT);
            case 175: ACCEPT("Left Curly Brace", LCBRACE);
            case 176: ACCEPT("Right Curly Brace", RCBRACE);
            case 177: FINAL_STATE_WITH_TRANSITION(
                        TRANSITION_NONSINGLE(isdigit(next_char), 177);
                        else TRANSITION('.', 178),
                        RETRACT_THEN_ACCEPT("Integer Literal", INT_LITERAL));
            case 178: FINAL_STATE_WITH_TRANSITION(
                        TRANSITION_NONSINGLE(isdigit(next_char), 179),
                        { retract_char(1); lexeme[forward_lexeme_ptr - begin_lexeme_ptr] = '0'; current_state = 177; });
            case 179: FINAL_STATE_WITH_TRANSITION(
                        TRANSITION_NONSINGLE(isdigit(next_char), 179);
                        else TRANSITION('.', 207),
                        RETRACT_THEN_ACCEPT("Float Literal", FLOAT_LITERAL));
            case 180: ACCEPT("String Literal", STR_LITERAL);
            case 181: {
                get_next_char();
                TRANSITION('>', 182);
                else TRANSITION(EOF, 183);
                else TRANSITION_NONSINGLE(next_char != '>', 165);
                break;
            }
            case 182: ACCEPT("Comment", COMMENT);
            case 183: {
                // handle incompletes better
                printf("Incomplete comment encountered.\n");
                ACCEPT("End of File", EOF);
            }
            case 184: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "int Keyword (Attrib Value)", INT_KW);
            case 185: WORD_STATE(TRANSITION('r', 186));
            case 186: WORD_STATE(TRANSITION('i', 187));
            case 187: WORD_STATE(TRANSITION('n', 188));
            case 188: WORD_STATE(TRANSITION('g', 189));
            case 189: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "string Keyword (Attrib Value)", STRING_KW);
            case 190: WORD_STATE(
                        TRANSITION('l', 191);
                        else TRANSITION('a', 202));
            case 191: WORD_STATE(TRANSITION('o', 192));
            case 192: WORD_STATE(TRANSITION('a', 193));
            case 193: WORD_STATE(TRANSITION('t', 194));
            case 194: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "float Keyword (Attrib Value)", FLOAT_KW);
            case 195: WORD_STATE(TRANSITION('o', 196));
            case 196: WORD_STATE(TRANSITION('o', 197));
            case 197: WORD_STATE(TRANSITION('l', 198));
            case 198: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "bool Keyword (Attrib Value)", BOOL_KW);
            case 199: WORD_STATE(TRANSITION('u', 200));
            case 200: WORD_STATE(TRANSITION('e', 201));
            case 201: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "true Literal", TRUE_KW);
            case 202: WORD_STATE(TRANSITION('l', 203));
            case 203: WORD_STATE(TRANSITION('s', 204));
            case 204: WORD_STATE(TRANSITION('e', 205));
            case 205: ACCEPT_WORD_OR_TRANSITION_IDENTIFIER(
                        "false Literal", FALSE_KW);
            case 206: ACCEPT("End of File", EOF);

            // DEAD STATE for Invalid Float
            case 207: FINAL_STATE_WITH_TRANSITION(
                        TRANSITION_NONSINGLE(isdigit(next_char), 207),
                        RETRACT_THEN_ACCEPT("Invalid. Too Many Decimal Points", INVALID_LITERAL));
        }
    }
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
            should_refresh_buffer[1] = true; // Reset flag
            // Reset pointer to the first character of the second buffer half
            next_char = buffer[(forward_lexeme_ptr = BUFFER_LENGTH)];
        }

        // Then, check if it's the second half
        else if(forward_lexeme_ptr == BUFFER_LENGTH * 2 - 1) {
            // Refresh first half
            if(should_refresh_buffer[0]) refresh_buffer(0);
            should_refresh_buffer[0] = true; // Reset flag
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
void retract_char(uint16_t steps) {
    if(steps < 0)
        return;

    // If retracting from first to second buffer
    // e.g. retracting from index 0???
    if(forward_lexeme_ptr - steps < 0) {
        // 1st half buffer should not be refreshed
        should_refresh_buffer[0] = false;
        // Retract the pointer
        forward_lexeme_ptr = (BUFFER_LENGTH * 2 - 1) - steps - forward_lexeme_ptr;
        next_char = buffer[forward_lexeme_ptr];
    }

    // Else if retracting from second to first buffer
    // e.g. retracting from BUFFER_LENGTH
    else if(forward_lexeme_ptr >= BUFFER_LENGTH && forward_lexeme_ptr - steps < BUFFER_LENGTH) {
        // 2nd half buffer should not be refreshed
        should_refresh_buffer[1] = false;
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
void refresh_buffer(uint8_t half) {
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
    buffer[BUFFER_LENGTH * (half) + chars_read] = EOF;
}