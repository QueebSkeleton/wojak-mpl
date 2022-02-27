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
bool out_stmt(tree_node*);
bool switch_stmt(tree_node*);
bool cases(tree_node*);
bool case_(tree_node*);
bool regular_case(tree_node*);
bool default_case(tree_node*);
bool break_stmt(tree_node*);
bool continue_stmt(tree_node*);

bool parse_expr(tree_node*);
bool e(expr_stack*, expr_stack*);
bool p(expr_stack*, expr_stack*);
void pop_operator(expr_stack*, expr_stack*);
void push_operator(tree_node*, expr_stack*, expr_stack*);
bool compare_operators(node_type, node_type);
int8_t precedence_level(node_type);
bool is_recognizable_as_binary(int8_t);
bool is_recognizable_as_unary(int8_t);
tree_node *trasnform_into_binary_node(int8_t);
bool is_unary(node_type);
bool is_left_associative(node_type);


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
        out_stmt(stmt_node) ||
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

bool out_stmt(tree_node *parent) {
    tree_node *out_node = create_node(OUT_STMT, NULL);
    token_list_item *before = curr_token;

    if(parse_token(LANGLE) &&
       parse_token(OUT_KW)) {

        char identifier_str[80];

        bool success =
            expect_token(EXPR_KW, "expr keyword") &&
            expect_token(EQ_SIGN, "=") &&
            expect_token(LCBRACE, "{") &&
            parse_expr(out_node) &&
            expect_token(RCBRACE, "}") &&
            expect_token(FW_SLASH, "/") &&
            expect_token(RANGLE, ">");

        if(success) {
            add_ready_to_parent(parent, out_node);
            
            printf("Output Statement\n");
            return true;
        } else {
            printf("incorrect output statement.\n"
                   "Must be <out expr={expr}/>\n");
        }
    } else {
        curr_token = before -> prev;
        get_next_token();
    }

    destroy_node(out_node);
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

bool parse_expr(tree_node *parent) {
    tree_node *expr_node = create_node(EXPR_NODE, NULL);
    token_list_item *before = curr_token;

    expr_stack *operators = create_expr_stack(),
               *operands = create_expr_stack();
    
    expr_stack_push(operators, create_node(SENTINEL_EXPR_NODE, NULL));
    bool success = e(operators, operands);

    if(success) {
        add_ready_to_parent(expr_node, expr_stack_peek(operands));
        add_ready_to_parent(parent, expr_node);
    } else {
        curr_token = before -> prev;
        get_next_token();
    }

    return success;
}

bool e(expr_stack *operators, expr_stack *operands) {
    p(operators, operands);
    get_next_token();
    while(is_recognizable_as_binary(curr_token_rep)) {
        tree_node *binary_node = trasnform_into_binary_node(curr_token_rep);
        push_operator(binary_node, operators, operands);
        p(operators, operands);
        get_next_token();
    }
    get_prev_token();
    while(expr_stack_peek(operators) -> type != SENTINEL_EXPR_NODE)
        pop_operator(operators, operands);
    return true;
}

bool p(expr_stack *operators, expr_stack *operands) {
    char placeholder[80];

    if(parse_identifier(placeholder)) {
        tree_node *identifier_node = create_node(IDENTIFIER_NODE, placeholder);
        expr_stack_push(operands, identifier_node);
    } else if(parse_constant(placeholder)) {
        tree_node *literal_node;
        switch(curr_token_rep) {
            case INT_LITERAL:
                literal_node = create_node(INT_LITERAL_NODE, curr_lexeme);
                break;
            case FLOAT_LITERAL:
                literal_node = create_node(FLOAT_LITERAL_NODE, curr_lexeme);
                break;
            case STR_LITERAL:
                literal_node = create_node(STR_LITERAL_NODE, curr_lexeme);
                break;
            case TRUE_KW:
                literal_node = create_node(TRUE_KW_NODE, curr_lexeme);
                break;
            case FALSE_KW:
                literal_node = create_node(FALSE_KW_NODE, curr_lexeme);
                break;
        }
        expr_stack_push(operands, literal_node);
    } else {
        printf("Syntax error: expected value, unary operator, or parenthesis.");
    }
}

void pop_operator(expr_stack *operators, expr_stack *operands) {
    tree_node *operator = expr_stack_pop(operators);
    if(!is_unary(operator -> type)) {
        // if binary operator, pop two operands then attach
        tree_node *second_operand = expr_stack_pop(operands),
                  *first_operand = expr_stack_pop(operands);
        add_ready_to_parent(operator, first_operand);
        add_ready_to_parent(operator, second_operand);
        expr_stack_push(operands, operator);
    } else {
        // if unary, just pop one operand
        tree_node *operand = expr_stack_pop(operands);
        add_ready_to_parent(operator, operand);
        expr_stack_push(operands, operator);
    }
}

void push_operator(tree_node *operator, expr_stack *operators, expr_stack *operands) {
    while(compare_operators(expr_stack_peek(operators) -> type,
                            operator -> type))
        pop_operator(operators, operands);
    expr_stack_push(operators, operator);
}

bool compare_operators(node_type first, node_type second) {
    // if both binary, check left associativity also
    if(!is_unary(first) && !is_unary(second))
        return precedence_level(first) > precedence_level(second) ||
               (precedence_level(first) == precedence_level(second) &&
                is_left_associative(first));
    
    else
        return precedence_level(first) > precedence_level(second);
}

int8_t precedence_level(node_type operator) {
    switch(operator) {
        case SENTINEL_EXPR_NODE:
            return 255;
        case LOGIC_OR_NODE:
            return 12;
        case LOGIC_AND_NODE:
            return 11;
        case BITWISE_OR_NODE:
            return 10;
        case BITWISE_NOT_NODE:
            return 9;
        case BITWISE_AND_NODE:
            return 8;
        case RELATIONAL_EQ_NODE:
        case RELATIONAL_NOT_EQ_NODE:
            return 7;
        case RELATIONAL_GT_NODE:
        case RELATIONAL_LT_NODE:
        case RELATIONAL_GT_EQ_NODE:
        case RELATIONAL_LT_EQ_NODE:
            return 6;
        case BITWISE_LEFT_NODE:
        case BITWISE_RIGHT_NODE:
            return 5;
        case ADD_NODE:
        case SUB_NODE:
            return 4;
        case MUL_NODE:
        case DIV_NODE:
        case FLOOR_NODE:
        case MOD_NODE:
            return 3;
        case UNARY_ADD_NODE:
        case UNARY_SUB_NODE:
            return 2;
        default:
            return -1;
    }
}

bool is_recognizable_as_binary(int8_t token_rep) {
    return token_rep == LOGIC_OR_OP ||
           token_rep == LOGIC_AND_OP ||
           token_rep == BITWISE_OR_OP ||
           token_rep == BITWISE_XOR_OP ||
           token_rep == BITWISE_AND_OP ||
           token_rep == EQ_OP ||
           token_rep == NOT_EQ_OP ||
           token_rep == LANGLE ||
           token_rep == LT_EQ_OP ||
           token_rep == RANGLE ||
           token_rep == GT_EQ_OP ||
           token_rep == BITWISE_LEFT_OP ||
           token_rep == BITWISE_RIGHT_OP ||
           token_rep == ADD_OP ||
           token_rep == HYPHEN ||
           token_rep == MUL_OP ||
           token_rep == FW_SLASH ||
           token_rep == FLR_OP ||
           token_rep == MOD_OP;
}

bool is_recognizable_as_unary(int8_t token_rep) {
    return token_rep == ADD_OP ||
           token_rep == HYPHEN;
}

tree_node *trasnform_into_binary_node(int8_t token_rep) {
    switch(token_rep) {
        case LOGIC_OR_OP: return create_node(LOGIC_OR_NODE, "||");
        case LOGIC_AND_OP: return create_node(LOGIC_AND_NODE, "&&");
        case BITWISE_OR_OP: return create_node(BITWISE_OR_NODE, "|");
        case BITWISE_XOR_OP: return create_node(BITWISE_XOR_NODE, "^");
        case BITWISE_AND_OP: return create_node(BITWISE_AND_NODE, "&");
        case EQ_OP: return create_node(RELATIONAL_EQ_NODE, "==");
        case NOT_EQ_OP: return create_node(RELATIONAL_NOT_EQ_NODE, "!=");
        case LANGLE: return create_node(RELATIONAL_LT_NODE, "<");
        case LT_EQ_OP: return create_node(RELATIONAL_LT_EQ_NODE, "<=");
        case RANGLE: return create_node(RELATIONAL_GT_NODE, ">");
        case GT_EQ_OP: return create_node(RELATIONAL_GT_EQ_NODE, ">=");
        case BITWISE_LEFT_OP: return create_node(BITWISE_LEFT_NODE, "<<");
        case BITWISE_RIGHT_OP: return create_node(BITWISE_RIGHT_NODE, ">>");
        case ADD_OP: return create_node(ADD_NODE, "+");
        case HYPHEN: return create_node(SUB_NODE, "-");
        case MUL_OP: return create_node(MUL_NODE, "*");
        case FW_SLASH: return create_node(DIV_NODE, "/");
        case FLR_OP: return create_node(FLOOR_NODE, "//");
        case MOD_OP: return create_node(MOD_NODE, "%");
        default: return NULL;
    }
}

bool is_unary(node_type type) {
    return type == UNARY_ADD_NODE ||
           type == UNARY_SUB_NODE;
}

bool is_left_associative(node_type type) {
    return type == LOGIC_OR_NODE ||
           type == LOGIC_AND_NODE ||
           type == BITWISE_OR_NODE ||
           type == BITWISE_XOR_NODE ||
           type == BITWISE_AND_NODE ||
           type == RELATIONAL_EQ_NODE ||
           type == RELATIONAL_NOT_EQ_NODE ||
           type == RELATIONAL_LT_NODE ||
           type == RELATIONAL_LT_EQ_NODE ||
           type == RELATIONAL_GT_NODE ||
           type == RELATIONAL_GT_EQ_NODE ||
           type == BITWISE_LEFT_NODE ||
           type == BITWISE_RIGHT_NODE ||
           type == ADD_NODE ||
           type == SUB_NODE ||
           type == MUL_NODE ||
           type == DIV_NODE ||
           type == FLOOR_NODE ||
           type == MOD_NODE;
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
    token_list_item *before = curr_token;
    get_next_token();
    strcpy(placeholder_lex, curr_lexeme);
    bool success = curr_token_rep == IDENTIFIER;
    if(!success) {
        curr_token = before -> prev;
        get_next_token();
    }
    return success;
}

bool parse_token(int8_t token) {
    token_list_item *before = curr_token;
    if(curr_token_rep == EOF) { return false; }
    get_next_token();
    bool success = token == curr_token_rep;
    if(!success) {
        curr_token = before -> prev;
        get_next_token();
    }
    return success;
}

bool parse_constant(char *placeholder_lex) {
    token_list_item *before = curr_token;
    if(curr_token_rep == EOF) { return false; }
    get_next_token();
    strcpy(placeholder_lex, curr_lexeme);
    bool success =
        curr_token_rep == INT_LITERAL ||
        curr_token_rep == FLOAT_LITERAL ||
        curr_token_rep == STR_LITERAL ||
        curr_token_rep == TRUE_KW ||
        curr_token_rep == FALSE_KW;
    if(!success) {
        curr_token = before -> prev;
        get_next_token();
    }
    return success;
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