#include "../headers/token.h"

t_token* create_token(t_token_type type, 
    const char* lexeme, int line) {
    t_token* token = (t_token*)malloc(sizeof(t_token));
    token->type = type;
    token->lexeme = (char*) malloc(strlen(lexeme) + 1);
    strcpy(token->lexeme, lexeme);
    token->line = line;
    token->next = NULL;
    return token;
}

void free_token(t_token* token) {
    if (token) {
        free(token->lexeme);
        free(token);
    }
}

void append_token(t_token_list* list, t_token* token) {
    if (!list->head) {
        list->head = token;
        list->tail = token;
    } else {
        list->tail->next = token;
        list->tail = token;
    }
}

t_token_list* create_token_list() {
    t_token_list* list = (t_token_list*)malloc(sizeof(t_token_list));
    list->head = NULL;
    list->tail = NULL;
    return list;
}

void free_token_list(t_token_list* list) {
    t_token* current = list->head;
    while (current) {
        t_token* next = current->next;
        free_token(current);
        current = next;
    }
    list->head = NULL;
    list->tail = NULL;    
}

void print_token_type(t_token_type type) {
    switch (type) {
        case TOKEN_LET: printf("LET"); break;
        case TOKEN_PRINT: printf("PRINT"); break;
        case TOKEN_IF: printf("IF"); break;
        case TOKEN_WHILE: printf("WHILE"); break;
        case TOKEN_FOR:   printf("FOR"); break;
        case TOKEN_ROOM: printf("ROOM"); break;
        case TOKEN_CONNECT: printf("CONNECT"); break;
        case TOKEN_ON: printf("ON"); break;
        case TOKEN_ENTER: printf("ENTER"); break;
        case TOKEN_ID: printf("ID"); break;
        case TOKEN_INT: printf("INT"); break;
        case TOKEN_STRING: printf("STRING"); break;
        case TOKEN_PLUS: printf("PLUS"); break;
        case TOKEN_MINUS: printf("MINUS"); break;
        case TOKEN_MULT: printf("MULT"); break;
        case TOKEN_DIV: printf("DIV"); break;
        case TOKEN_ASSIGN: printf("ASSIGN"); break;
        case TOKEN_EQ: printf("EQ"); break;
        case TOKEN_NEQ: printf("NEQ"); break;
        case TOKEN_LT: printf("LT"); break;
        case TOKEN_GT: printf("GT"); break;
        case TOKEN_LTE: printf("LTE"); break;
        case TOKEN_GTE: printf("GTE"); break;
        case TOKEN_LPAREN: printf("LPAREN"); break;
        case TOKEN_RPAREN: printf("RPAREN"); break;
        case TOKEN_LBRACE: printf("LBRACE"); break;
        case TOKEN_RBRACE: printf("RBRACE"); break;
        case TOKEN_SEMICOLON: printf("SEMICOLON"); break;
        case TOKEN_COMMA: printf("COMMA"); break;
        case TOKEN_EOF: printf("EOF"); break;
        case TOKEN_ERROR: printf("ERROR"); break;
    }
}

void print_tokens(t_token_list* tokens) {
    t_token* current = tokens->head;
    while (current) {
        printf("Line %d: %s (", current->line, current->lexeme);
        print_token_type(current->type);
        printf(")\n");
        current = current->next;
    }
}