#ifndef GLOBALS_H
#define GLOBALS_H
#include "globals.h"
#endif

#ifndef LEXER_C
#define LEXER_C
#include "lexer.c"
#endif

#ifndef SYN_C
#define SYN_C
#include "syn.c"
#endif

#ifndef STDIO_H
#define STDIO_H
#include <stdio.h>
#endif

#ifndef STRING_H
#define STRING_H
#include <string.h>
#endif

bool is_valid_file_name(const char*);
char** separate_name_extension(const char*);
int compare(char[], char[]);

// From globals
extern char **separated_name_extension;

// From other phases
extern void start_lex();
extern void start_syn();

int main(int argc, char *argv[]) {
    // Check if arguments are properly given
    if(argc < 2) {
        printf("Error: no given files.\n");
        printf("Usage is: wojak filename.wojak\n");
        exit(EXIT_FAILURE);
    }

    // Check if filename is proper
    if(!is_valid_file_name(argv[1])) {
        printf("Error: invalid file with name %s. It must have a"
            " format of name.wojak\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    strcpy(filename, argv[1]);

    // Extract file name and extension
    separated_name_extension = separate_name_extension(argv[1]);
    if(compare(separated_name_extension[1], "wojak")) {
        printf("Error: file extension must be .wojak\n");
        exit(EXIT_FAILURE);
    }

    // Start Lexical Analysis Phase
    start_lex();
    // Start Syntax Analysis Phase when no errors from lex
    if(lex_error_count > 0)
        exit(EXIT_FAILURE);
    start_syn();
}

/**
 * @brief Checks if the given string is a valid file name "name.extension"
 * 
 * @param filename the string to evaluate
 * @return the validity of the string as a filename
 */
bool is_valid_file_name(const char *filename) {
    int filename_len = strlen(filename);

    if(filename_len < 3) return false;

    for(int i = 0; i < filename_len; i++)
        if(filename[i] == '.' && (i != 0 && i != filename_len - 1))
            return true;

    return false;
}

/**
 * @brief Separate the file name and extension from a given string.
 * 
 * @param filename the filename to break down.
 * @return const char** the name (index 0) and extension (index 1)
 */
char** separate_name_extension(const char *filename) {
    char *dot_loc = strrchr(filename, '.'),
         **filename_broken = (char **) malloc(sizeof(char *) * 2);
    uint8_t name_len = dot_loc - filename + 1,
            ext_len = filename + (strlen(filename)) - dot_loc;
    filename_broken[0] = strcpy((char *) malloc(sizeof(char) * name_len),
                                filename);
    filename_broken[0][name_len - 1] = '\0';
    filename_broken[1] = strcpy((char *) malloc(sizeof(char) * ext_len),
                                dot_loc + 1);
    filename_broken[1][ext_len - 1] = '\0';
    return filename_broken;
}

/**
 * @brief Compares two character strings.
 *
 * @param a the first string
 * @param b the second string 
 * @return the ascii difference
 */
int compare(char a[], char b[]) {
    int a_len = strlen(a),
        b_len = strlen(b),
        i;

    for(i = 0; i < a_len && i < b_len; i++)
        if(a[i] != b[i])
            return a[i] - b[i];
    
    // overlaps (when one of the strings is bigger)
    if(i < a_len)
        return a[i];
    else if(i < b_len)
        return -b[i];

    return 0;
}