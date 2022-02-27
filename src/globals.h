#ifndef STDIO_H
#define STDIO_H
#include <stdio.h>
#endif

#ifndef INTTYPES_H
#define INTTYPES_h
#include <inttypes.h>
#endif

/**
 * @brief File name, raw.
 */
char filename[255];

/**
 * @brief File name, separated name and extension.
 */
char **separated_name_extension;

/**
 * @brief File name, with .symwojak\
 */
char filename_symboltable[255];

/**
 * @brief Number of errors encountered in lexer phase.
  */
uint8_t lex_error_count;