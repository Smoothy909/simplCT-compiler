#ifndef TOKEN_H
#define TOKEN_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Token types enumeration
 * Represents all possible token types in the language
 */
typedef enum e_token_type {
    // Keywords
    TOKEN_LET,
    TOKEN_PRINT,
    TOKEN_IF,
    TOKEN_WHILE,
    TOKEN_FOR,
    
    // Game-specific keywords (for future extension)
    TOKEN_ROOM,
    TOKEN_CONNECT,
    TOKEN_ON,
    TOKEN_ENTER,

    // Literals and identifiers
    TOKEN_ID,           // Variable names
    TOKEN_INT,          // Integer literals
    TOKEN_STRING,       // String literals

    // Operators
    TOKEN_PLUS,         // +
    TOKEN_MINUS,        // -
    TOKEN_MULT,         // *
    TOKEN_DIV,          // /
    TOKEN_ASSIGN,       // =
    TOKEN_EQ,           // ==
    TOKEN_NEQ,          // !=
    TOKEN_LT,           // <
    TOKEN_GT,           // >
    TOKEN_LTE,          // <=
    TOKEN_GTE,          // >=

    // Delimiters
    TOKEN_LPAREN,       // (
    TOKEN_RPAREN,       // )
    TOKEN_LBRACE,       // {
    TOKEN_RBRACE,       // }
    TOKEN_SEMICOLON,    // ;
    TOKEN_COMMA,        // ,

    // Special tokens
    TOKEN_EOF,          // End of file
    TOKEN_ERROR         // Lexical error
} t_token_type;

/**
 * Token structure
 * Represents a single lexical token with its type, text, and position
 */
typedef struct s_token {
    t_token_type type;      // Token type
    char* lexeme;           // Text content of the token
    int line;               // Line number (for error messages)
    int value;              // Integer value (for TOKEN_INT)
    struct s_token* next;   // Next token in the list
} t_token;

/**
 * Token list structure
 * Maintains a linked list of tokens
 */
typedef struct s_token_list {
    t_token* head;          // First token
    t_token* tail;          // Last token
} t_token_list;

/* Token functions */

/**
 * Creates a new token
 */
t_token* create_token(t_token_type type, const char* lexeme, int line);

/**
 * Frees a token's memory
 */
void free_token(t_token* token);

/**
 * Appends a token to the list
 */
void append_token(t_token_list* list, t_token* token);

/**
 * Creates a new empty token list
 */
t_token_list* create_token_list();

/**
 * Frees the entire token list 
 */
void free_token_list(t_token_list* list);

/**
 * Prints the token type name (for debugging)
 */
void print_token_type(t_token_type type);

/**
 * Prints all tokens in the list (for debugging)
 */
void print_tokens(t_token_list* tokens);

#endif