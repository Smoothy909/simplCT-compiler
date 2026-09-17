#ifndef LEXER_H
#define LEXER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "token.h"
#include "io_utils.h"

/**
 * Lexer structure
 * Maintains the state during lexical analysis
 */
typedef struct s_lexer {
    const char* source;     // Source code to analyze
    int length;             // Length of source code
    int pos;                // Current position in source
    int line;               // Current line number (for error messages)
} t_lexer;


/* Lexer helper functions */

/**
* Peeks at the current character without consuming it
*/
char peek_char(t_lexer* lexer);

/**
* Peeks at the next character without consuming it
*/
char peek_next_char(t_lexer* lexer);

/**
* Advances the lexer by one character and returns it
*/
char advance_pos(t_lexer* lexer);

/** 
* Checks if the given lexeme is a keyword and returns its token type
*/
t_token_type check_keyword(const char* lexeme);

/**
 * Scans an identifier or a keyword
 */
t_token* scan_identifier(t_lexer* lexer);

/**
 * Scans a number literal
 */
t_token* scan_number(t_lexer* lexer);

/**
 * Scans a string literal
 */
t_token* scan_string(t_lexer* lexer);


/**
 * Tokenizes source code into a list of tokens
 */
t_token_list* tokenize(const char* source);

#endif