#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <stdio.h>
#include <stdlib.h>

/**
 * Reads the entire content of a file into a string, utility function for the lexer and compiler.
 * Returns NULL on error
 * The caller must free the returned string
 */
char* read_file(const char* filename);

#endif