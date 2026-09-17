#include "../headers/compiler.h"

/* ============================================
   UTILITY FUNCTIONS (provided — do not modify)
   ============================================ */

static int check(t_compiler* compiler, t_token_type type) {
    if (!compiler->current) return 0;
    return compiler->current->type == type;
}

static void advance(t_compiler* compiler) {
    compiler->previous = compiler->current;
    if (compiler->current) {
        compiler->current = compiler->current->next;
    }
}

static int match(t_compiler* compiler, t_token_type type) {
    if (check(compiler, type)) {
        advance(compiler);
        return 1;
    }
    return 0;
}

static void expect(t_compiler* compiler, t_token_type type, const char* message) {
    if (!check(compiler, type)) {
        fprintf(stderr, "Compilation Error at line %d: %s\n",
                compiler->current ? compiler->current->line : 0, message);
        compiler->had_error = 1;
        if (compiler->current) {
            advance(compiler);
        }
        return;
    }
    advance(compiler);
}

/* ============================================
   EXPRESSION COMPILATION
   ============================================ */

void compile_primary(t_compiler* compiler) {
    if (match(compiler, TOKEN_INT)) {
        int value = 0;
        if (compiler->previous && compiler->previous->lexeme) {
            value = atoi(compiler->previous->lexeme);
        }
        add_push_instruction(compiler->code, value);
    }
    else if (match(compiler, TOKEN_STRING)) {
        add_var_instruction(compiler->code, OP_PUSH_STR, compiler->previous->lexeme);
    }
    else if (match(compiler, TOKEN_ID)) {
        add_var_instruction(compiler->code, OP_LOAD_VAR, compiler->previous->lexeme);
    }
    else if (match(compiler, TOKEN_LPAREN)) {
        compile_expression(compiler);
        expect(compiler, TOKEN_RPAREN, "Parenthese fermante ')' attendue");
    }
    else {
        fprintf(stderr, "Compilation Error at line %d: expression primaire attendue\n",
                compiler->current ? compiler->current->line : 0);
        compiler->had_error = 1;
        if (compiler->current) {
            advance(compiler);
        }
    }
}

void compile_factor(t_compiler* compiler) {
    compile_primary(compiler);
    while (match(compiler, TOKEN_MULT) || match(compiler, TOKEN_DIV)) {
        t_token_type op = compiler->previous->type;
        compile_primary(compiler);
        if (op == TOKEN_MULT) add_simple_instruction(compiler->code, OP_MUL);
        else add_simple_instruction(compiler->code, OP_DIV);
    }
}

void compile_term(t_compiler* compiler) {
    compile_factor(compiler);
    while (match(compiler, TOKEN_PLUS) || match(compiler, TOKEN_MINUS)) {
        t_token_type op = compiler->previous->type;
        compile_factor(compiler);
        if (op == TOKEN_PLUS) add_simple_instruction(compiler->code, OP_ADD);
        else add_simple_instruction(compiler->code, OP_SUB);
    }
}

void compile_comparison(t_compiler* compiler) {
    compile_term(compiler);
    while (match(compiler, TOKEN_LT) || match(compiler, TOKEN_GT) ||
           match(compiler, TOKEN_LTE) || match(compiler, TOKEN_GTE)) {
        t_token_type op = compiler->previous->type;
        compile_term(compiler);
        if (op == TOKEN_LT) add_simple_instruction(compiler->code, OP_LT);
        else if (op == TOKEN_GT) add_simple_instruction(compiler->code, OP_GT);
        else if (op == TOKEN_LTE) add_simple_instruction(compiler->code, OP_LTE);
        else if (op == TOKEN_GTE) add_simple_instruction(compiler->code, OP_GTE);
    }
}

void compile_equality(t_compiler* compiler) {
    compile_comparison(compiler);
    while (match(compiler, TOKEN_EQ) || match(compiler, TOKEN_NEQ)) {
        t_token_type op = compiler->previous->type;
        compile_comparison(compiler);
        if (op == TOKEN_EQ) add_simple_instruction(compiler->code, OP_EQ);
        else add_simple_instruction(compiler->code, OP_NEQ);
    }
}

void compile_expression(t_compiler* compiler) {
    compile_equality(compiler);
}


/* ============================================
   STATEMENT COMPILATION
   ============================================ */

void compile_let_statement(t_compiler* compiler) {
    expect(compiler, TOKEN_ID, "Expected identifier after 'let'");
    if (compiler->had_error) return;
    const char* var_name = compiler->previous->lexeme;

    expect(compiler, TOKEN_ASSIGN, "Expected '=' after variable name");
    if (compiler->had_error) return;

    compile_expression(compiler);
    if (compiler->had_error) return;

    expect(compiler, TOKEN_SEMICOLON, "Expected ';' after let statement");
    if (compiler->had_error) return;

    add_var_instruction(compiler->code, OP_STORE_VAR, var_name);
}

void compile_print_statement(t_compiler* compiler) {
    expect(compiler, TOKEN_LPAREN, "Expected '(' after 'print'");
    if (compiler->had_error) return;

    compile_expression(compiler);
    if (compiler->had_error) return;

    if (match(compiler, TOKEN_COMMA)) {
        add_simple_instruction(compiler->code, OP_PRINT_NO_NEWLINE);

        while (1) {
            add_var_instruction(compiler->code, OP_PUSH_STR, " ");
            add_simple_instruction(compiler->code, OP_PRINT_NO_NEWLINE);

            compile_expression(compiler);
            if (compiler->had_error) return;

            if (match(compiler, TOKEN_COMMA)) {
                add_simple_instruction(compiler->code, OP_PRINT_NO_NEWLINE);
            } else {
                add_simple_instruction(compiler->code, OP_PRINT);
                break;
            }
        }
    } else {
        add_simple_instruction(compiler->code, OP_PRINT);
    }

    expect(compiler, TOKEN_RPAREN, "Expected ')' after print arguments");
    if (compiler->had_error) return;

    expect(compiler, TOKEN_SEMICOLON, "Expected ';' after print statement");
    (void)compiler;
}

void compile_if_statement(t_compiler* compiler) {
    expect(compiler, TOKEN_LPAREN, "Expected '(' after 'if'");
    if (compiler->had_error) return;

    compile_expression(compiler);
    if (compiler->had_error) return;

    expect(compiler, TOKEN_RPAREN, "Expected ')' afte if condition");
    if (compiler->had_error) return;

    int jump_index = compiler->code->count;
    add_instruction(compiler->code, create_instruction(OP_JUMP_IF_FALSE, 0, NULL));

    compile_block(compiler);
    if (compiler->had_error) return;

    get_instruction_at(compiler->code, jump_index)->operand = compiler->code->count;
    (void)compiler;
}

void compile_while_statement(t_compiler* compiler) {
    int loop_start = compiler->code->count;

    expect(compiler, TOKEN_LPAREN, "Expected '(' after 'while'");
    if (compiler->had_error) return;

    compile_expression(compiler);
    if (compiler->had_error) return;

    expect(compiler, TOKEN_RPAREN, "Expected ')' after while condition");
    if (compiler->had_error) return;

    int jump_index = compiler->code->count;
    add_instruction(compiler->code, create_instruction(OP_JUMP_IF_FALSE, 0, NULL));

    compile_block(compiler);
    if (compiler->had_error) return;

    add_instruction(compiler->code, create_instruction(OP_JUMP, loop_start, NULL));

    get_instruction_at(compiler->code, jump_index)->operand = compiler->code->count;
}

void compile_block(t_compiler* compiler) {
    expect(compiler, TOKEN_LBRACE, "Expected '{' to start block");
    if (compiler->had_error) return;

    while (!check(compiler, TOKEN_RBRACE) && !check(compiler, TOKEN_EOF) && !compiler->had_error) {
        compile_statement(compiler);
    }

    expect(compiler, TOKEN_RBRACE, "Expected '}' to end block");
}

void compile_for_statement(t_compiler* compiler) {
    expect(compiler, TOKEN_LPAREN, "Expected '(' after 'for'");
    if (compiler->had_error) return;

    // 1. Initialisation : let i = 0
    if (match(compiler, TOKEN_LET)) {
        compile_let_statement(compiler);
    }
    if (compiler->had_error) return;

    // 2. Condition : i < 10
    int loop_start = compiler->code->count;
    compile_expression(compiler);
    if (compiler->had_error) return;

    expect(compiler, TOKEN_SEMICOLON, "Expected ';' after for condition");
    if (compiler->had_error) return;

    int jump_index = compiler->code->count;
    add_instruction(compiler->code, create_instruction(OP_JUMP_IF_FALSE, 0, NULL));

    // 3. Sauter l'increment pour l'instant, on y reviendra
    // On sauvegarde où commence l'incrément
    // D'abord on compile le corps, puis l'incrément

    // Incrément : i = i + 1 — on le stocke token par token
    // Astuce : on saute l'incrément au premier passage
    int skip_inc_index = compiler->code->count;
    add_instruction(compiler->code, create_instruction(OP_JUMP, 0, NULL));

    // Position de l'incrément
    int inc_start = compiler->code->count;

    // Compiler l'incrément : "i = i + 1"
    expect(compiler, TOKEN_ID, "Expected identifier in for increment");
    if (compiler->had_error) return;
    const char* var_name = compiler->previous->lexeme;
    expect(compiler, TOKEN_ASSIGN, "Expected '=' in for increment");
    if (compiler->had_error) return;
    compile_expression(compiler);
    if (compiler->had_error) return;
    add_var_instruction(compiler->code, OP_STORE_VAR, var_name);

    // Retourner à la condition
    add_instruction(compiler->code, create_instruction(OP_JUMP, loop_start, NULL));

    // Patcher le saut par-dessus l'incrément
    get_instruction_at(compiler->code, skip_inc_index)->operand = compiler->code->count;

    expect(compiler, TOKEN_RPAREN, "Expected ')' after for increment");
    if (compiler->had_error) return;

    // Corps de la boucle
    compile_block(compiler);
    if (compiler->had_error) return;

    // Aller à l'incrément après le corps
    add_instruction(compiler->code, create_instruction(OP_JUMP, inc_start, NULL));

    // Patcher le saut de sortie
    get_instruction_at(compiler->code, jump_index)->operand = compiler->code->count;
}

void compile_statement(t_compiler* compiler) {
    if (match(compiler, TOKEN_LET)) {
        compile_let_statement(compiler);
        return;
    }

    if (match(compiler, TOKEN_IF)) {
        compile_if_statement(compiler);
        return;
    }

    if (match(compiler, TOKEN_WHILE)) {
        compile_while_statement(compiler);
        return;
    }

    if (match(compiler, TOKEN_FOR)) {
        compile_for_statement(compiler);
        return;
    }
    
    if (match(compiler, TOKEN_PRINT)) {
        compile_print_statement(compiler);
        return;
    }

    if (check(compiler, TOKEN_ID) &&
        compiler->current &&
        compiler->current->next &&
        compiler->current->next->type == TOKEN_ASSIGN) {
        expect(compiler, TOKEN_ID, "Expected identifier");
        if (compiler->had_error) return;
        const char* var_name = compiler->previous->lexeme;

        expect(compiler, TOKEN_ASSIGN, "Expected '=' after identifier");
        if (compiler->had_error) return;

        compile_expression(compiler);
        if (compiler->had_error) return;

        expect(compiler, TOKEN_SEMICOLON, "Expected ';' after assignment");
        if (compiler->had_error) return;

        add_var_instruction(compiler->code, OP_STORE_VAR, var_name);
        return;
    }

    fprintf(stderr, "Compilation Error at line %d: unexpected token in statement\n",
            compiler->current ? compiler->current->line : 0);
    compiler->had_error = 1;
    if (compiler->current) {
        advance(compiler);
    }
}

/* ============================================
   MAIN COMPILATION FUNCTION
   ============================================ */

t_bytecode* compile(t_token_list* tokens) {
    if (!tokens) return NULL;

    t_compiler compiler;
    compiler.current = tokens->head;
    compiler.previous = NULL;
    compiler.code = create_bytecode();
    compiler.had_error = 0;

    while (compiler.current && compiler.current->type != TOKEN_EOF && !compiler.had_error) {
        compile_statement(&compiler);
    }

    if (compiler.had_error) {
        free_bytecode(compiler.code);
        return NULL;
    }

    add_simple_instruction(compiler.code, OP_HALT);
    return compiler.code;
}
