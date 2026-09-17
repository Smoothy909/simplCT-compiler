#include "../headers/lexer.h"

// Retourne le caractère actuel sans avancer
char peek_char(t_lexer* lexer) {
    if (lexer->pos >= lexer->length) return '\0';
    return lexer->source[lexer->pos];
}

// Retourne le caractère suivant sans avancer
char peek_next_char(t_lexer* lexer) {
    if (lexer->pos + 1 >= lexer->length) return '\0';
    return lexer->source[lexer->pos + 1];
}

// Avance d'un caractère et le retourne
char advance_pos(t_lexer* lexer) {
    char c = peek_char(lexer);
    if (c != '\0') {
        lexer->pos++;
        if (c == '\n') lexer->line++;
    }
    return c;
}

// Compare le mot trouvé avec les mots-clés du langage
t_token_type check_keyword(const char* lexeme) {
    if (strcmp(lexeme, "let") == 0) return TOKEN_LET;
    if (strcmp(lexeme, "print") == 0) return TOKEN_PRINT;
    if (strcmp(lexeme, "if") == 0) return TOKEN_IF;
    if (strcmp(lexeme, "while") == 0) return TOKEN_WHILE;
if (strcmp(lexeme, "for") == 0)   return TOKEN_FOR;
    if (strcmp(lexeme, "room") == 0) return TOKEN_ROOM;
    if (strcmp(lexeme, "connect") == 0) return TOKEN_CONNECT;
    if (strcmp(lexeme, "on") == 0) return TOKEN_ON;
    if (strcmp(lexeme, "enter") == 0) return TOKEN_ENTER;
    return TOKEN_ID;
}

// Scanne un nom de variable ou un mot-clé
t_token* scan_identifier(t_lexer* lexer) {
    int start = lexer->pos;
    while (isalnum(peek_char(lexer)) || peek_char(lexer) == '_') {
        advance_pos(lexer);
    }

    int len = lexer->pos - start;
    char* lexeme = (char*)malloc(len + 1);
    strncpy(lexeme, &lexer->source[start], len);
    lexeme[len] = '\0';

    t_token_type type = check_keyword(lexeme);
    t_token* token = create_token(type, lexeme, lexer->line);
    free(lexeme);
    return token;
}

// Scanne un nombre entier
t_token* scan_number(t_lexer* lexer) {
    int start = lexer->pos;
    while (isdigit(peek_char(lexer))) {
        advance_pos(lexer);
    }

    int len = lexer->pos - start;
    char* lexeme = (char*)malloc(len + 1);
    strncpy(lexeme, &lexer->source[start], len);
    lexeme[len] = '\0';

    t_token* token = create_token(TOKEN_INT, lexeme, lexer->line);
    token->value = atoi(lexeme);
    free(lexeme);
    return token;
}

// scan_string(): scans a string literal
t_token* scan_string(t_lexer* lexer) {
    advance_pos(lexer);
    int start = lexer->pos;

    while (peek_char(lexer) != '"' && peek_char(lexer) != '\0') {
        advance_pos(lexer);
    }
    if (peek_char(lexer) == '\0') {
        return create_token(TOKEN_ERROR, "Unterminated string", lexer->line);
    }

    int len = lexer->pos - start;
    char lexeme[len + 1];
    strncpy(lexeme, &lexer->source[start], len);
    lexeme[len] = '\0';

    if (peek_char(lexer) == '"') advance_pos(lexer);

    return create_token(TOKEN_STRING, lexeme, lexer->line);
}

t_token_list* tokenize(const char* source) {
    t_lexer lexer;
    lexer.source = source;
    lexer.length = strlen(source);
    lexer.pos = 0;
    lexer.line = 1;

    t_token_list* list = create_token_list();

    while (peek_char(&lexer) != '\0') {
        char c = peek_char(&lexer);

        //espaces
        if (isspace(c)) {
            advance_pos(&lexer);
            continue;
        }

        //mot ?
        if (isalpha(c) || c == '_') {
            append_token(list, scan_identifier(&lexer));
            continue;
        }

        //trv nbr
        if (isdigit(c)) {
            append_token(list, scan_number(&lexer));
            continue;
        }

        //trv str
        if (c == '"') {
            append_token(list, scan_string(&lexer));
            continue;
        }


        t_token* token = NULL;
        char lx[3] = {c, '\0', '\0'};

        switch (c) {
            case '+': token = create_token(TOKEN_PLUS, "+", lexer.line); advance_pos(&lexer); break;
            case '-': token = create_token(TOKEN_MINUS, "-", lexer.line); advance_pos(&lexer); break;
            case '*': token = create_token(TOKEN_MULT, "*", lexer.line); advance_pos(&lexer); break;
            case '/': token = create_token(TOKEN_DIV, "/", lexer.line); advance_pos(&lexer); break;
            case ';': token = create_token(TOKEN_SEMICOLON, ";", lexer.line); advance_pos(&lexer); break;
            case '(': token = create_token(TOKEN_LPAREN, "(", lexer.line); advance_pos(&lexer); break;
            case ')': token = create_token(TOKEN_RPAREN, ")", lexer.line); advance_pos(&lexer); break;
            case '{': token = create_token(TOKEN_LBRACE, "{", lexer.line); advance_pos(&lexer); break;
            case '}': token = create_token(TOKEN_RBRACE, "}", lexer.line); advance_pos(&lexer); break;
            case ',': token = create_token(TOKEN_COMMA, ",", lexer.line); advance_pos(&lexer); break;


            case '=':
                if (peek_next_char(&lexer) == '=') {
                    token = create_token(TOKEN_EQ, "==", lexer.line);
                    advance_pos(&lexer);
                } else {
                    token = create_token(TOKEN_ASSIGN, "=", lexer.line);
                }
                advance_pos(&lexer);
                break;
            case '!':
                if (peek_next_char(&lexer) == '=') {
                    token = create_token(TOKEN_NEQ, "!=", lexer.line);
                    advance_pos(&lexer);
                    advance_pos(&lexer);
                } else {
                    advance_pos(&lexer);
                }
                break;
            case '<':
                if (peek_next_char(&lexer) == '=') {
                    token = create_token(TOKEN_LTE, "<=", lexer.line);
                    advance_pos(&lexer);
                } else {
                    token = create_token(TOKEN_LT, "<", lexer.line);
                }
                advance_pos(&lexer);
                break;
            case '>':
                if (peek_next_char(&lexer) == '=') {
                    token = create_token(TOKEN_GTE, ">=", lexer.line);
                    advance_pos(&lexer);
                } else {
                    token = create_token(TOKEN_GT, ">", lexer.line);
                }
                advance_pos(&lexer);
                break;

            default:
                token = create_token(TOKEN_ERROR, &c, lexer.line); advance_pos(&lexer); break;
        }

        if (token) append_token(list, token);
    }


    append_token(list, create_token(TOKEN_EOF, "EOF", lexer.line));
    return list;
}