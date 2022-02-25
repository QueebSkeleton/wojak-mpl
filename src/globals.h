#ifndef STDIO_H
#define STDIO_H
#include <stdio.h>
#endif

#ifndef INTTYPES_H
#define INTTYPES_h
#include <inttypes.h>
#endif

/**
 * @brief The input file, assumingly in .wojak language rules.
 * 
 */
FILE *input_file;

/**
 * @brief The symbol table file output
 * 
 */
FILE *sym_file;

/**
 * @brief Number of errors encountered in lexer phase.
  */
uint8_t lex_error_count;