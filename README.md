<a name="readme-top"></a>

<!-- PROJECT SHIELDS -->
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]

<div align="center">
  <h3 align="center">Wojak Markup Programming Language</h3>
  <p align="center">
    A programming language with markup syntax.
  </p>
</div>

## About the Project

Wojak.MPL is an attempt to create a compiled language with markup-style syntax,
like HTML.

This project is made in fulfillment of the requirements for the subject
**Principles of Programming Languages** under the *Computer Science* program
in the Polytechnic University of the Philippines.

## Built With

The compiler is made with:

[![C][C-shield]][C-docs]
[![SH][SH-shield]][SH-docs]

## Todo for the Project

- [x] Lexical Analysis
- [x] Grammar Rules, Syntax Analysis
- [x] Line and Column Error Information
- [ ] Semantic Analysis
- [ ] Compilation to x86_64 Linux ELF format

## Basic Language Syntax

Here is a sample program to calculate the sum of two numbers using Wojak.MPL:

<pre>
&lt;!-- Declare variables a, b as integers --&gt;
&lt;prim_declare type='int' identifier='a, b'/&gt;

&lt;!-- Input a, b --&gt;
&lt;out expr='"Enter first number: "'/&gt;
&lt;in identifier='a'/&gt;
&lt;out expr='"Enter second number: "'/&gt;
&lt;in identifier='b'/&gt;

&lt;!-- Declare sum variable, assign the total of a and b --&gt;
&lt;prim_declare type='int' identifier='sum' expr='a + b'/&gt;

&lt;!-- Print result --&gt;
&lt;out expr='"The sum is: " + sum + "\n"'/&gt;
</pre>

The language allows for typical generic programming language features such as:

1. Variables
<pre>
&lt;!-- Declare primitives --&gt;
&lt;prim_declare type={string} identifier={myStr}/&gt;
&lt;prim_declare type={int} identifier={a} expr={5+5-10}/&gt;

&lt;!-- Array type declaration --&gt;
&lt;list_declare type={int} identifier={b}&gt;
  &lt;elem expr={1}/&gt;
  &lt;elem expr={2}/&gt;
&lt;/list_declare&gt;

&lt;!-- Assignment operation --&gt;
&lt;assign identifier={a} expr={500} /&gt;
</pre>

2. Input and Output (I/O)
<pre>
&lt;!-- Input statement (standard io) --&gt;
&lt;in identifier={a}/&gt;

&lt;!-- Output statement (standard io) --&gt;
&lt;out expr={"Enter first number: "}/&gt;
</pre>

3. Conditional Statements (if-else, switch-case)
<pre>
&lt;!-- If-Else --&gt;
&lt;if expr={grade >= 75}&gt;
  &lt;out expr={"You passed!\n"}/&gt;
&lt;/if&gt;
&lt;elif expr={grade >= 70}&gt;
  &lt;out expr={"Please see your instructor.\n"}/&gt;
&lt;/elif&gt;
&lt;else&gt;
  &lt;out expr={"You failed!\n"}/&gt;
&lt;/else&gt;

&lt;!-- Switch --&gt;
&lt;switch eval={a}&gt;
  <&lt;case const={10}&gt;
    &lt;!-- statements --&gt;
    &lt;break/&gt;
  &lt;/case&gt;
  &lt;default&gt;
    &lt;!-- statements --&gt;
  &lt;/default&gt;
&lt;/switch&gt;
</pre>

1. Iterative Statement
<pre>
&lt;while expr={a > 10}&gt;
  &lt;!-- statements --&gt;
&lt;/while&gt;
</pre>

## Grammar Rules

<pre>
WOJAK_GRAMMAR = (NT, T, P, S)

where:
NT is the set of non-terminals;
T is the set of terminal symbols, which include the tokens in the language;
P is the set of production rules, and;
S is the start symbol, which is the PROG non-terminal
</pre>

### Start Symbol

<pre>
S = PROG
</pre>

### Non-Terminals

<pre>
NT = { PROG, STMTS, STMT, PRIM_DECL_STMT, LIST_DECL_STMT, LIST_ELEMS, LIST_ELEM,
       ASSIGN_STMT, IN_STMT, OUT_STMT, DECISION_STMT, IF_STMT, ELIF_STMTS,
       ELIF_STMT, ELSE_STMT, SWITCH_STMT, CASES, CASE, REGULAR_CASE, DEFAULT_CASE,
       WHILE_STMT, CONTINUE_STMT, BREAK_STMT, EXPR_LIST, EXPR, PREC_12, PREC_11,
       PREC_10, PREC_9, PREC_8, PREC_7, PREC_6, PREC_5, PREC_4, PREC_3, PREC_2,
       PREC_1, IDENTIFIER_LIST, IDENTIFIER, TYPE, ARR_IDX, LITERAL,
       STRING_LITERAL, INTEGER_LITERAL, FLOAT_LITERAL, BOOL_LITERAL }
</pre>

### Terminals

<pre>
T = { “+”, “++”, “-”, “--”, “*”, “**”, “/” “//”, “%”, “&”, “|”, “~”, “^”,
      “<<”, “>>”, “&&”, “||”, “!”, “=”, “==”, “!=”, “>”, “>=”, “<”, “<=”,
      “(“, “)”, “\’”, “\””, “,”, “{“, “}”, “.”, “[“, “]”, “prim_declare”,
      “list_declare”, “elem”, “assign”, “if”, “elif”, “else”, “break”,
      “continue”, “switch”, “case”, “while”, “type”, “identifier”, “expr”,
      “size”, “eval”, “const”, “int”, “float”, “string”, “bool”, “true”,
      “false”, “and”, “or”, “not”, “sizeof”, “begin”, “end”, “list”,
      “then”, “to”, “initialize”, “with”, “proceed” }
</pre>

### Identifiers and Literals Definition

<pre>
IDENTIFIER      = (underscore + lowercase + uppercase)
                  (underscore + digit + lowercase + uppercase)*

STRING_LITERAL  = (\” (Σ-{\”})* \”)

INTEGER_LITERAL = (digit+)

FLOAT_LITERAL   = (digit*).(digit+)

BOOL_LITERAL    = “true” | “false”
</pre>

### Production Rules

<pre>
PROG  = STMTS
STMTS = STMT { STMT }
STMT  = PRIM_DECL_STMT
      | LIST_DECL_STMT
      | ASSIGN_STMT
      | IN_STMT
      | OUT_STMT
      | DECISION_STMT
      | SWITCH_STMT
      | WHILE_STMT
      | BREAK_STMT
      | CONTINUE_STMT

IDENTIFIER_LIST = IDENTIFIER { “,” IDENTIFIER }
EXPR_LIST       = EXPR { “,” EXPR }

PRIM_DECL_STMT  = “<” “prim_declare”
                  “type” “=” “{“ TYPE “}”
                  “identifier” “=” “{“ IDENTIFIER_LIST “}”
                  [ “expr” “=” “{“ EXPR_LIST “}” ] “/” “>”

LIST_DECL_STMT  = “<” “list_declare”
                  “type” “=” “{“ TYPE “}”
                  “identifier” “=” “{“ IDENTIFIER “}” “>”
                  LIST_ELEMS 
                  “<” “/” “list_declare” “>”
LIST_ELEMS      = LIST_ELEM { LIST_ELEM }
LIST_ELEM       = “<” “elem” “expr” “=” “{“ “EXPR” “}” “/” “>”

ASSIGN_STMT     = “<” “assign”
                  “identifier” “=” “{“ IDENTIFIER “}”
                  “expr” “=” “{“ EXPR “}” “/” “>”

IN_STMT         = “<” “in”
                  “identifier” “=” “{“ IDENTIFIER “}” “/” “>”

OUT_STMT        = “<” “out” “expr” “=” “{“ EXPR “}” “/” “>”

DECISION_STMT   = IF_STMT ELIF_STMTS ELSE_STMT
IF_STMT         = “<” “if” “expr” “=” “{“ EXPR “}” “>”
                  STMTS
                  “<” “/” “if” “>”
ELIF_STMTS      = [ ELIF_STMT { ELIF_STMT } ]

ELIF_STMT       = “<” “elif” “expr” “=” “{“ EXPR “}” “>”
                  STMTS
                  “<” “/” “elif” “>”
ELSE_STMT       = [ “<” “else” “>” STMTS “<” “/” “else” “>” ]

SWITCH_STMT     = “<” “switch” “eval” “=” “{“ IDENTIFIER “}” “>”
                  CASES
                  “<” “/” “switch” “>”
CASES           = CASE { CASE }
CASE            = REGULAR_CASE
                | DEFAULT_CASE
REGULAR_CASE    = “<” “case”
                      “const” “=” “{“ LITERAL “}” “>”
                  STMTS
                  “<” “/” “case” “>”
DEFAULT_CASE    = “<” “default” “>”
                  STMTS
                  “<” “/” “default” “>”

WHILE_STMT      = “<” “while” “expr” “=” “{“ EXPR “}” “>”
                  STMTS
                  “<” “/” “while” “>”

CONTINUE_STMT   = “<” “continue” “/” “>”
BREAK_STMT      = “<” “break” “/” “>”

EXPR    = PREC_12
PREC_12 = [ PREC_12 “||” ] PREC_11
PREC_11 = [ PREC_11 “&&” ] PREC_10
PREC_10 = [ PREC_10 “|” ] PREC_9
PREC_9  = [ PREC_9 “^” ] PREC_8
PREC_8  = [ PREC_8 “&” ] PREC_7
PREC_7  = [ PREC ( “==” | “!=” ) ] PREC_6
PREC_6  = [ PREC_6 ( “>” | “>=” | “<” | “<=” ) ] PREC_5
PREC_5  = [ PREC_5 ( “<<” | “>>” ) ] PREC_4
PREC_4  = [ PREC_4 ( “+” | “-” ) ] PREC_3
PREC_3  = [ PREC_3 ( “*” | “/” | “//” | “%” ) ] PREC_2 .
PREC_2  = ( “++” | “--” |
            “+”  | “-”  | 
            “!”  | “~”  |
            ( “(“ TYPE “)” ) ) PREC_2
        | PREC_1 [ “**” PREC_2 ]
PREC_1  = IDENTIFIER [ “++” | “--” | ARR_IDX ]
        | LITERAL
        | “(“ EXPR “)”

LITERAL = STRING_LITERAL
        | INTEGER_LITERAL
        | FLOAT_LITERAL
        | BOOL_LITERAL

TYPE    = “string”
        | “integer”
        | “float”
        | “bool”

ARR_IDX = “[“ EXPR “]”
</pre>

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- Markdown Links & Images -->
[stars-shield]: https://img.shields.io/github/stars/QueebSkeleton/Wojak-MPL?style=for-the-badge
[stars-url]: https://github.com/QueebSkeleton/Wojak-MPL/stargazers
[issues-shield]: https://img.shields.io/github/issues/QueebSkeleton/Wojak-MPL?style=for-the-badge
[issues-url]: https://github.com/QueebSkeleton/Wojak-MPL/issues

[C-shield]: https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white
[C-docs]: https://devdocs.io/c/
[SH-shield]: https://img.shields.io/badge/shell_script-%23121011.svg?style=for-the-badge&logo=gnu-bash&logoColor=white
[SH-docs]: https://www.gnu.org/software/bash/manual/bash.html
