/**
 * test_compiler.c — Part 2 validation tests
 *
 * Tests are based strictly on the Part 2 specification.
 * Helper functions build all required state directly (no call to any
 * function under test), following the same principle as the TP labs.
 *
 * Run locally:
 *   ./run_tests.sh part2
 * or manually:
 *   cmake -B cmake-build-debug -DENABLE_VALIDATION_TESTS=ON
 *   cmake --build cmake-build-debug
 *   ctest --test-dir cmake-build-debug -R "^test_compiler\." --output-on-failure
 */

#include "utest.h"
#include "headers/compiler.h"
#include "headers/bytecode.h"
#include "headers/token.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Feedback macros ─────────────────────────────────────────────────────── */

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif

#define EXPECT_EQ_FEEDBACK(expected, actual, checks, hint) \
    EXPECT_EQ_MSG((expected), (actual), "Checks: " checks " | Hint: " hint)

#define EXPECT_TRUE_FEEDBACK(condition, checks, hint) \
    EXPECT_TRUE_MSG((condition), "Checks: " checks " | Hint: " hint)

#define EXPECT_STREQ_FEEDBACK(expected, actual, checks, hint) \
    EXPECT_STREQ_MSG((expected), (actual), "Checks: " checks " | Hint: " hint)

#define ASSERT_TRUE_FEEDBACK(condition, checks, hint) \
    ASSERT_TRUE_MSG((condition), "Checks: " checks " | Hint: " hint)

/* ── Helpers ─────────────────────────────────────────────────────────────── */

/*
 * Build a linked token chain from parallel arrays of types and lexemes.
 * Uses create_token() from token.c (not under test).
 * The last element of `types` must be TOKEN_EOF.
 */
static t_token *make_token_chain(t_token_type types[], const char *lexemes[], int count)
{
    if (count == 0) return NULL;
    t_token *head = NULL;
    t_token *tail = NULL;
    for (int i = 0; i < count; i++) {
        t_token *tok = create_token(types[i], lexemes[i], 1);
        if (!head) { head = tok; tail = tok; }
        else        { tail->next = tok; tail = tok; }
    }
    return head;
}

/*
 * Free a token chain without calling free_token_list.
 * create_token() strdup's the lexeme, so we free it explicitly.
 */
static void free_chain(t_token *head)
{
    while (head) {
        t_token *next = head->next;
        free(head->lexeme);
        free(head);
        head = next;
    }
}

/*
 * Build a t_compiler with a fresh empty bytecode, positioned at `head`.
 * Does NOT call compile() or any function under test.
 */
static t_compiler make_compiler(t_token *head)
{
    t_compiler c;
    c.current   = head;
    c.previous  = NULL;
    c.code      = create_bytecode();
    c.had_error = 0;
    return c;
}

/*
 * Free the bytecode owned by a compiler (not the token chain).
 */
static void cleanup_compiler(t_compiler *c)
{
    if (c->code) { free_bytecode(c->code); c->code = NULL; }
}

/*
 * Build a t_token_list (for compile() integration tests).
 * Uses create_token_list() + append_token() — neither is under test.
 */
static t_token_list *make_token_list(t_token_type types[],
                                     const char *lexemes[], int count)
{
    t_token_list *list = create_token_list();
    for (int i = 0; i < count; i++)
        append_token(list, create_token(types[i], lexemes[i], 1));
    return list;
}

/*
 * Shortcut: get instruction at index i and assert it is non-NULL.
 * Returns NULL (and lets the calling test fail via ASSERT) if out of range.
 */
static t_bytecode_instruction *get_instr(t_bytecode *code, int i,
                                          const char *ctx)
{
    t_bytecode_instruction *instr = get_instruction_at(code, i);
    (void)ctx; /* used only for clarity in call sites */
    return instr;
}


/* ═══════════════════════════════════════════════════════════════════════════
   SUITE: compile_primary
   ═══════════════════════════════════════════════════════════════════════════ */

UTEST(compile_primary, int_literal)
{
    t_token_type  types[]   = { TOKEN_INT, TOKEN_EOF };
    const char   *lexemes[] = { "42",      "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 2);
    t_compiler    c         = make_compiler(chain);

    compile_primary(&c);

    ASSERT_TRUE_FEEDBACK(c.code != NULL,
        "compile_primary(42) produces non-NULL bytecode",
        "compile_primary must always initialise the bytecode.");
    EXPECT_EQ_FEEDBACK(1, c.code->count,
        "compile_primary(42): exactly 1 instruction emitted",
        "An integer literal must emit exactly one OP_PUSH instruction.");
    t_bytecode_instruction *instr = get_instr(c.code, 0, "compile_primary/int");
    ASSERT_TRUE_FEEDBACK(instr != NULL,
        "instruction at index 0 is non-NULL",
        "compile_primary must emit an instruction for an integer literal.");
    EXPECT_EQ_FEEDBACK(OP_PUSH, instr->opcode,
        "compile_primary(42): opcode == OP_PUSH",
        "An integer literal must produce OP_PUSH.");
    EXPECT_EQ_FEEDBACK(42, instr->operand,
        "compile_primary(42): operand == 42",
        "OP_PUSH operand must equal the value of the integer literal.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_primary, int_zero)
{
    t_token_type  types[]   = { TOKEN_INT, TOKEN_EOF };
    const char   *lexemes[] = { "0",       "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 2);
    t_compiler    c         = make_compiler(chain);

    compile_primary(&c);

    ASSERT_TRUE_FEEDBACK(c.code != NULL,
        "compile_primary(0): non-NULL bytecode",
        "compile_primary must handle zero.");
    t_bytecode_instruction *instr = get_instr(c.code, 0, "compile_primary/zero");
    ASSERT_TRUE_FEEDBACK(instr != NULL,
        "instruction at index 0 is non-NULL",
        "compile_primary must emit an instruction for 0.");
    EXPECT_EQ_FEEDBACK(OP_PUSH, instr->opcode,
        "compile_primary(0): opcode == OP_PUSH",
        "Zero is a valid integer literal and must produce OP_PUSH.");
    EXPECT_EQ_FEEDBACK(0, instr->operand,
        "compile_primary(0): operand == 0",
        "OP_PUSH operand must equal 0.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_primary, string_literal)
{
    t_token_type  types[]   = { TOKEN_STRING, TOKEN_EOF };
    const char   *lexemes[] = { "hello",      "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 2);
    t_compiler    c         = make_compiler(chain);

    compile_primary(&c);

    ASSERT_TRUE_FEEDBACK(c.code != NULL,
        "compile_primary(\"hello\"): non-NULL bytecode",
        "compile_primary must handle string literals.");
    EXPECT_EQ_FEEDBACK(1, c.code->count,
        "compile_primary(\"hello\"): exactly 1 instruction",
        "A string literal must emit exactly one instruction.");
    t_bytecode_instruction *instr = get_instr(c.code, 0, "compile_primary/str");
    ASSERT_TRUE_FEEDBACK(instr != NULL,
        "instruction at index 0 is non-NULL",
        "compile_primary must emit an instruction for a string literal.");
    EXPECT_EQ_FEEDBACK(OP_PUSH_STR, instr->opcode,
        "compile_primary(\"hello\"): opcode == OP_PUSH_STR",
        "A string literal must produce OP_PUSH_STR.");
    ASSERT_TRUE_FEEDBACK(instr->operand_str != NULL,
        "compile_primary(\"hello\"): operand_str is non-NULL",
        "OP_PUSH_STR must have a non-NULL string operand.");
    EXPECT_STREQ_FEEDBACK("hello", instr->operand_str,
        "compile_primary(\"hello\"): operand_str == \"hello\"",
        "OP_PUSH_STR operand_str must equal the string lexeme.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_primary, identifier)
{
    t_token_type  types[]   = { TOKEN_ID, TOKEN_EOF };
    const char   *lexemes[] = { "x",      "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 2);
    t_compiler    c         = make_compiler(chain);

    compile_primary(&c);

    ASSERT_TRUE_FEEDBACK(c.code != NULL,
        "compile_primary(x): non-NULL bytecode",
        "compile_primary must handle identifiers.");
    EXPECT_EQ_FEEDBACK(1, c.code->count,
        "compile_primary(x): exactly 1 instruction",
        "An identifier must emit exactly one OP_LOAD_VAR instruction.");
    t_bytecode_instruction *instr = get_instr(c.code, 0, "compile_primary/id");
    ASSERT_TRUE_FEEDBACK(instr != NULL,
        "instruction at index 0 is non-NULL",
        "compile_primary must emit an instruction for an identifier.");
    EXPECT_EQ_FEEDBACK(OP_LOAD_VAR, instr->opcode,
        "compile_primary(x): opcode == OP_LOAD_VAR",
        "An identifier must produce OP_LOAD_VAR.");
    ASSERT_TRUE_FEEDBACK(instr->operand_str != NULL,
        "compile_primary(x): operand_str is non-NULL",
        "OP_LOAD_VAR must carry the variable name.");
    EXPECT_STREQ_FEEDBACK("x", instr->operand_str,
        "compile_primary(x): operand_str == \"x\"",
        "OP_LOAD_VAR operand_str must equal the identifier lexeme.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_primary, parenthesized_expression)
{
    /* ( 7 ) — should produce exactly OP_PUSH 7 */
    t_token_type  types[]   = { TOKEN_LPAREN, TOKEN_INT, TOKEN_RPAREN, TOKEN_EOF };
    const char   *lexemes[] = { "(",          "7",       ")",          "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 4);
    t_compiler    c         = make_compiler(chain);

    compile_primary(&c);

    ASSERT_TRUE_FEEDBACK(c.code != NULL,
        "compile_primary((7)): non-NULL bytecode",
        "compile_primary must handle parenthesised expressions.");
    EXPECT_EQ_FEEDBACK(1, c.code->count,
        "compile_primary((7)): exactly 1 instruction",
        "A parenthesised integer must still emit one OP_PUSH.");
    t_bytecode_instruction *instr = get_instr(c.code, 0, "compile_primary/paren");
    ASSERT_TRUE_FEEDBACK(instr != NULL,
        "instruction at index 0 is non-NULL",
        "compile_primary must emit an instruction for (7).");
    EXPECT_EQ_FEEDBACK(OP_PUSH, instr->opcode,
        "compile_primary((7)): opcode == OP_PUSH",
        "Parentheses do not add extra instructions.");
    EXPECT_EQ_FEEDBACK(7, instr->operand,
        "compile_primary((7)): operand == 7",
        "The value inside parentheses must be preserved.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_primary, advances_current_token)
{
    /* After compiling an integer, current must have moved past it */
    t_token_type  types[]   = { TOKEN_INT, TOKEN_EOF };
    const char   *lexemes[] = { "5",       "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 2);
    t_compiler    c         = make_compiler(chain);

    compile_primary(&c);

    EXPECT_TRUE_FEEDBACK(c.current != NULL,
        "compile_primary(5): current is non-NULL after compile",
        "After consuming the integer, current must point to the next token.");
    EXPECT_EQ_FEEDBACK(TOKEN_EOF, c.current->type,
        "compile_primary(5): current == TOKEN_EOF after compile",
        "compile_primary must advance past the consumed token.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_primary, no_error_on_valid_input)
{
    t_token_type  types[]   = { TOKEN_INT, TOKEN_EOF };
    const char   *lexemes[] = { "1",       "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 2);
    t_compiler    c         = make_compiler(chain);

    compile_primary(&c);

    EXPECT_EQ_FEEDBACK(0, c.had_error,
        "compile_primary(1): had_error == 0",
        "compile_primary must not set had_error on valid input.");

    cleanup_compiler(&c);
    free_chain(chain);
}


/* ═══════════════════════════════════════════════════════════════════════════
   SUITE: compile_factor
   ═══════════════════════════════════════════════════════════════════════════ */

UTEST(compile_factor, single_primary)
{
    t_token_type  types[]   = { TOKEN_INT, TOKEN_EOF };
    const char   *lexemes[] = { "3",       "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 2);
    t_compiler    c         = make_compiler(chain);

    compile_factor(&c);

    EXPECT_EQ_FEEDBACK(1, c.code->count,
        "compile_factor(3): 1 instruction emitted",
        "A single primary with no * or / must emit exactly 1 instruction.");
    t_bytecode_instruction *i0 = get_instr(c.code, 0, "factor/single");
    ASSERT_TRUE_FEEDBACK(i0 != NULL, "instruction 0 non-NULL", "compile_factor must emit instruction.");
    EXPECT_EQ_FEEDBACK(OP_PUSH, i0->opcode,
        "compile_factor(3): opcode[0] == OP_PUSH",
        "A lone integer in factor position must produce OP_PUSH.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_factor, multiplication)
{
    /* 3 * 4 → PUSH 3, PUSH 4, MUL */
    t_token_type  types[]   = { TOKEN_INT, TOKEN_MULT, TOKEN_INT, TOKEN_EOF };
    const char   *lexemes[] = { "3",       "*",        "4",       "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 4);
    t_compiler    c         = make_compiler(chain);

    compile_factor(&c);

    EXPECT_EQ_FEEDBACK(3, c.code->count,
        "compile_factor(3*4): 3 instructions emitted",
        "a * b must emit: PUSH a, PUSH b, MUL.");
    t_bytecode_instruction *i2 = get_instr(c.code, 2, "factor/mul");
    ASSERT_TRUE_FEEDBACK(i2 != NULL, "instruction 2 non-NULL", "compile_factor must emit MUL.");
    EXPECT_EQ_FEEDBACK(OP_MUL, i2->opcode,
        "compile_factor(3*4): opcode[2] == OP_MUL",
        "Multiplication must emit OP_MUL as the third instruction.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_factor, division)
{
    /* 6 / 2 → PUSH 6, PUSH 2, DIV */
    t_token_type  types[]   = { TOKEN_INT, TOKEN_DIV, TOKEN_INT, TOKEN_EOF };
    const char   *lexemes[] = { "6",       "/",       "2",       "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 4);
    t_compiler    c         = make_compiler(chain);

    compile_factor(&c);

    EXPECT_EQ_FEEDBACK(3, c.code->count,
        "compile_factor(6/2): 3 instructions emitted",
        "a / b must emit: PUSH a, PUSH b, DIV.");
    t_bytecode_instruction *i2 = get_instr(c.code, 2, "factor/div");
    ASSERT_TRUE_FEEDBACK(i2 != NULL, "instruction 2 non-NULL", "compile_factor must emit DIV.");
    EXPECT_EQ_FEEDBACK(OP_DIV, i2->opcode,
        "compile_factor(6/2): opcode[2] == OP_DIV",
        "Division must emit OP_DIV as the third instruction.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_factor, left_associativity)
{
    /* 2 * 3 * 4 → PUSH 2, PUSH 3, MUL, PUSH 4, MUL (5 instructions) */
    t_token_type  types[]   = { TOKEN_INT, TOKEN_MULT, TOKEN_INT, TOKEN_MULT, TOKEN_INT, TOKEN_EOF };
    const char   *lexemes[] = { "2",       "*",        "3",       "*",        "4",       "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 6);
    t_compiler    c         = make_compiler(chain);

    compile_factor(&c);

    EXPECT_EQ_FEEDBACK(5, c.code->count,
        "compile_factor(2*3*4): 5 instructions emitted",
        "a*b*c must emit 5 instructions (left-associative).");
    t_bytecode_instruction *i2 = get_instr(c.code, 2, "factor/la/0");
    t_bytecode_instruction *i4 = get_instr(c.code, 4, "factor/la/1");
    ASSERT_TRUE_FEEDBACK(i2 != NULL && i4 != NULL, "instructions 2 and 4 non-NULL",
        "compile_factor must emit two MUL instructions for a*b*c.");
    EXPECT_EQ_FEEDBACK(OP_MUL, i2->opcode,
        "compile_factor(2*3*4): opcode[2] == OP_MUL",
        "First MUL must appear at index 2.");
    EXPECT_EQ_FEEDBACK(OP_MUL, i4->opcode,
        "compile_factor(2*3*4): opcode[4] == OP_MUL",
        "Second MUL must appear at index 4 (left-associative).");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_factor, no_error_on_valid_input)
{
    t_token_type  types[]   = { TOKEN_INT, TOKEN_MULT, TOKEN_INT, TOKEN_EOF };
    const char   *lexemes[] = { "2",       "*",        "3",       "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 4);
    t_compiler    c         = make_compiler(chain);

    compile_factor(&c);

    EXPECT_EQ_FEEDBACK(0, c.had_error,
        "compile_factor(2*3): had_error == 0",
        "compile_factor must not set had_error on valid input.");

    cleanup_compiler(&c);
    free_chain(chain);
}


/* ═══════════════════════════════════════════════════════════════════════════
   SUITE: compile_term
   ═══════════════════════════════════════════════════════════════════════════ */

UTEST(compile_term, addition)
{
    /* 3 + 4 → PUSH 3, PUSH 4, ADD */
    t_token_type  types[]   = { TOKEN_INT, TOKEN_PLUS, TOKEN_INT, TOKEN_EOF };
    const char   *lexemes[] = { "3",       "+",        "4",       "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 4);
    t_compiler    c         = make_compiler(chain);

    compile_term(&c);

    EXPECT_EQ_FEEDBACK(3, c.code->count,
        "compile_term(3+4): 3 instructions emitted",
        "a + b must emit: PUSH a, PUSH b, ADD.");
    t_bytecode_instruction *i2 = get_instr(c.code, 2, "term/add");
    ASSERT_TRUE_FEEDBACK(i2 != NULL, "instruction 2 non-NULL", "compile_term must emit ADD.");
    EXPECT_EQ_FEEDBACK(OP_ADD, i2->opcode,
        "compile_term(3+4): opcode[2] == OP_ADD",
        "Addition must emit OP_ADD.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_term, subtraction)
{
    /* 5 - 2 → PUSH 5, PUSH 2, SUB */
    t_token_type  types[]   = { TOKEN_INT, TOKEN_MINUS, TOKEN_INT, TOKEN_EOF };
    const char   *lexemes[] = { "5",       "-",         "2",       "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 4);
    t_compiler    c         = make_compiler(chain);

    compile_term(&c);

    EXPECT_EQ_FEEDBACK(3, c.code->count,
        "compile_term(5-2): 3 instructions",
        "a - b must emit: PUSH a, PUSH b, SUB.");
    t_bytecode_instruction *i2 = get_instr(c.code, 2, "term/sub");
    ASSERT_TRUE_FEEDBACK(i2 != NULL, "instruction 2 non-NULL", "compile_term must emit SUB.");
    EXPECT_EQ_FEEDBACK(OP_SUB, i2->opcode,
        "compile_term(5-2): opcode[2] == OP_SUB",
        "Subtraction must emit OP_SUB.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_term, precedence_mult_over_add)
{
    /* 2 + 3 * 4 → PUSH 2, PUSH 3, PUSH 4, MUL, ADD (5 instructions) */
    t_token_type  types[]   = { TOKEN_INT, TOKEN_PLUS, TOKEN_INT, TOKEN_MULT, TOKEN_INT, TOKEN_EOF };
    const char   *lexemes[] = { "2",       "+",        "3",       "*",        "4",       "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 6);
    t_compiler    c         = make_compiler(chain);

    compile_term(&c);

    EXPECT_EQ_FEEDBACK(5, c.code->count,
        "compile_term(2+3*4): 5 instructions",
        "* has higher precedence than +: emit PUSH 2, PUSH 3, PUSH 4, MUL, ADD.");
    t_bytecode_instruction *i3 = get_instr(c.code, 3, "term/prec/mul");
    t_bytecode_instruction *i4 = get_instr(c.code, 4, "term/prec/add");
    ASSERT_TRUE_FEEDBACK(i3 != NULL && i4 != NULL, "instructions 3 and 4 non-NULL",
        "compile_term must produce MUL then ADD for 2+3*4.");
    EXPECT_EQ_FEEDBACK(OP_MUL, i3->opcode,
        "compile_term(2+3*4): opcode[3] == OP_MUL",
        "MUL must appear before ADD (higher precedence).");
    EXPECT_EQ_FEEDBACK(OP_ADD, i4->opcode,
        "compile_term(2+3*4): opcode[4] == OP_ADD",
        "ADD must be the last instruction for 2+3*4.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_term, left_associativity)
{
    /* 1 + 2 + 3 → PUSH 1, PUSH 2, ADD, PUSH 3, ADD (5 instructions) */
    t_token_type  types[]   = { TOKEN_INT, TOKEN_PLUS, TOKEN_INT, TOKEN_PLUS, TOKEN_INT, TOKEN_EOF };
    const char   *lexemes[] = { "1",       "+",        "2",       "+",        "3",       "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 6);
    t_compiler    c         = make_compiler(chain);

    compile_term(&c);

    EXPECT_EQ_FEEDBACK(5, c.code->count,
        "compile_term(1+2+3): 5 instructions",
        "1+2+3 must be left-associative: PUSH 1, PUSH 2, ADD, PUSH 3, ADD.");
    t_bytecode_instruction *i2 = get_instr(c.code, 2, "term/la/0");
    t_bytecode_instruction *i4 = get_instr(c.code, 4, "term/la/1");
    ASSERT_TRUE_FEEDBACK(i2 != NULL && i4 != NULL, "instructions 2 and 4 non-NULL",
        "compile_term must emit two ADD instructions for 1+2+3.");
    EXPECT_EQ_FEEDBACK(OP_ADD, i2->opcode,
        "compile_term(1+2+3): opcode[2] == OP_ADD",
        "First ADD at index 2.");
    EXPECT_EQ_FEEDBACK(OP_ADD, i4->opcode,
        "compile_term(1+2+3): opcode[4] == OP_ADD",
        "Second ADD at index 4 (left-associative).");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_term, single_factor)
{
    t_token_type  types[]   = { TOKEN_INT, TOKEN_EOF };
    const char   *lexemes[] = { "9",       "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 2);
    t_compiler    c         = make_compiler(chain);

    compile_term(&c);

    EXPECT_EQ_FEEDBACK(1, c.code->count,
        "compile_term(9): 1 instruction",
        "A single integer with no operator must emit 1 instruction.");

    cleanup_compiler(&c);
    free_chain(chain);
}


/* ═══════════════════════════════════════════════════════════════════════════
   SUITE: compile_comparison
   ═══════════════════════════════════════════════════════════════════════════ */

UTEST(compile_comparison, less_than)
{
    /* a < b → LOAD_VAR a, LOAD_VAR b, LT */
    t_token_type  types[]   = { TOKEN_ID, TOKEN_LT, TOKEN_ID, TOKEN_EOF };
    const char   *lexemes[] = { "a",      "<",      "b",      "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 4);
    t_compiler    c         = make_compiler(chain);

    compile_comparison(&c);

    EXPECT_EQ_FEEDBACK(3, c.code->count,
        "compile_comparison(a<b): 3 instructions",
        "a < b must emit LOAD_VAR a, LOAD_VAR b, LT.");
    t_bytecode_instruction *i2 = get_instr(c.code, 2, "cmp/lt");
    ASSERT_TRUE_FEEDBACK(i2 != NULL, "instruction 2 non-NULL", "compile_comparison must emit LT.");
    EXPECT_EQ_FEEDBACK(OP_LT, i2->opcode,
        "compile_comparison(a<b): opcode[2] == OP_LT",
        "< must produce OP_LT.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_comparison, greater_than)
{
    t_token_type  types[]   = { TOKEN_ID, TOKEN_GT, TOKEN_ID, TOKEN_EOF };
    const char   *lexemes[] = { "a",      ">",      "b",      "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 4);
    t_compiler    c         = make_compiler(chain);

    compile_comparison(&c);

    t_bytecode_instruction *i2 = get_instr(c.code, 2, "cmp/gt");
    ASSERT_TRUE_FEEDBACK(i2 != NULL, "instruction 2 non-NULL", "compile_comparison must emit GT.");
    EXPECT_EQ_FEEDBACK(OP_GT, i2->opcode,
        "compile_comparison(a>b): opcode[2] == OP_GT",
        "> must produce OP_GT.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_comparison, less_equal)
{
    t_token_type  types[]   = { TOKEN_ID, TOKEN_LTE, TOKEN_ID, TOKEN_EOF };
    const char   *lexemes[] = { "a",      "<=",      "b",      "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 4);
    t_compiler    c         = make_compiler(chain);

    compile_comparison(&c);

    t_bytecode_instruction *i2 = get_instr(c.code, 2, "cmp/lte");
    ASSERT_TRUE_FEEDBACK(i2 != NULL, "instruction 2 non-NULL", "compile_comparison must emit LTE.");
    EXPECT_EQ_FEEDBACK(OP_LTE, i2->opcode,
        "compile_comparison(a<=b): opcode[2] == OP_LTE",
        "<= must produce OP_LTE.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_comparison, greater_equal)
{
    t_token_type  types[]   = { TOKEN_ID, TOKEN_GTE, TOKEN_ID, TOKEN_EOF };
    const char   *lexemes[] = { "a",      ">=",      "b",      "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 4);
    t_compiler    c         = make_compiler(chain);

    compile_comparison(&c);

    t_bytecode_instruction *i2 = get_instr(c.code, 2, "cmp/gte");
    ASSERT_TRUE_FEEDBACK(i2 != NULL, "instruction 2 non-NULL", "compile_comparison must emit GTE.");
    EXPECT_EQ_FEEDBACK(OP_GTE, i2->opcode,
        "compile_comparison(a>=b): opcode[2] == OP_GTE",
        ">= must produce OP_GTE.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_comparison, single_term)
{
    t_token_type  types[]   = { TOKEN_INT, TOKEN_EOF };
    const char   *lexemes[] = { "1",       "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 2);
    t_compiler    c         = make_compiler(chain);

    compile_comparison(&c);

    EXPECT_EQ_FEEDBACK(1, c.code->count,
        "compile_comparison(1): 1 instruction (no operator)",
        "A lone value with no comparison operator must emit 1 instruction.");

    cleanup_compiler(&c);
    free_chain(chain);
}


/* ═══════════════════════════════════════════════════════════════════════════
   SUITE: compile_equality
   ═══════════════════════════════════════════════════════════════════════════ */

UTEST(compile_equality, equal)
{
    /* a == b → LOAD_VAR a, LOAD_VAR b, EQ */
    t_token_type  types[]   = { TOKEN_ID, TOKEN_EQ, TOKEN_ID, TOKEN_EOF };
    const char   *lexemes[] = { "a",      "==",     "b",      "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 4);
    t_compiler    c         = make_compiler(chain);

    compile_equality(&c);

    EXPECT_EQ_FEEDBACK(3, c.code->count,
        "compile_equality(a==b): 3 instructions",
        "a == b must emit LOAD_VAR a, LOAD_VAR b, EQ.");
    t_bytecode_instruction *i2 = get_instr(c.code, 2, "eq/eq");
    ASSERT_TRUE_FEEDBACK(i2 != NULL, "instruction 2 non-NULL", "compile_equality must emit EQ.");
    EXPECT_EQ_FEEDBACK(OP_EQ, i2->opcode,
        "compile_equality(a==b): opcode[2] == OP_EQ",
        "== must produce OP_EQ.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_equality, not_equal)
{
    t_token_type  types[]   = { TOKEN_ID, TOKEN_NEQ, TOKEN_ID, TOKEN_EOF };
    const char   *lexemes[] = { "a",      "!=",      "b",      "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 4);
    t_compiler    c         = make_compiler(chain);

    compile_equality(&c);

    t_bytecode_instruction *i2 = get_instr(c.code, 2, "eq/neq");
    ASSERT_TRUE_FEEDBACK(i2 != NULL, "instruction 2 non-NULL", "compile_equality must emit NEQ.");
    EXPECT_EQ_FEEDBACK(OP_NEQ, i2->opcode,
        "compile_equality(a!=b): opcode[2] == OP_NEQ",
        "!= must produce OP_NEQ.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_equality, single_comparison)
{
    t_token_type  types[]   = { TOKEN_INT, TOKEN_EOF };
    const char   *lexemes[] = { "5",       "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 2);
    t_compiler    c         = make_compiler(chain);

    compile_equality(&c);

    EXPECT_EQ_FEEDBACK(1, c.code->count,
        "compile_equality(5): 1 instruction",
        "A lone value with no equality operator must emit 1 instruction.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_equality, int_equal_id)
{
    /* 42 == x → PUSH 42, LOAD_VAR x, EQ */
    t_token_type  types[]   = { TOKEN_INT, TOKEN_EQ, TOKEN_ID, TOKEN_EOF };
    const char   *lexemes[] = { "42",      "==",     "x",      "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 4);
    t_compiler    c         = make_compiler(chain);

    compile_equality(&c);

    t_bytecode_instruction *i0 = get_instr(c.code, 0, "eq/mixed/0");
    t_bytecode_instruction *i1 = get_instr(c.code, 1, "eq/mixed/1");
    t_bytecode_instruction *i2 = get_instr(c.code, 2, "eq/mixed/2");
    ASSERT_TRUE_FEEDBACK(i0 != NULL && i1 != NULL && i2 != NULL,
        "instructions 0-2 non-NULL",
        "compile_equality(42==x) must emit 3 instructions.");
    EXPECT_EQ_FEEDBACK(OP_PUSH, i0->opcode,
        "compile_equality(42==x): opcode[0] == OP_PUSH",
        "Left operand 42 must produce OP_PUSH.");
    EXPECT_EQ_FEEDBACK(OP_LOAD_VAR, i1->opcode,
        "compile_equality(42==x): opcode[1] == OP_LOAD_VAR",
        "Right operand x must produce OP_LOAD_VAR.");
    EXPECT_EQ_FEEDBACK(OP_EQ, i2->opcode,
        "compile_equality(42==x): opcode[2] == OP_EQ",
        "== must produce OP_EQ.");

    cleanup_compiler(&c);
    free_chain(chain);
}


/* ═══════════════════════════════════════════════════════════════════════════
   SUITE: compile_expression
   ═══════════════════════════════════════════════════════════════════════════ */

UTEST(compile_expression, simple_integer)
{
    t_token_type  types[]   = { TOKEN_INT, TOKEN_EOF };
    const char   *lexemes[] = { "42",      "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 2);
    t_compiler    c         = make_compiler(chain);

    compile_expression(&c);

    EXPECT_EQ_FEEDBACK(1, c.code->count,
        "compile_expression(42): 1 instruction",
        "compile_expression must delegate to the precedence chain.");
    t_bytecode_instruction *i0 = get_instr(c.code, 0, "expr/int");
    ASSERT_TRUE_FEEDBACK(i0 != NULL, "instruction 0 non-NULL",
        "compile_expression must emit an instruction.");
    EXPECT_EQ_FEEDBACK(OP_PUSH, i0->opcode,
        "compile_expression(42): opcode[0] == OP_PUSH",
        "An integer at expression level must produce OP_PUSH.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_expression, precedence_mult_over_add)
{
    /* 1 + 2 * 3 → PUSH 1, PUSH 2, PUSH 3, MUL, ADD */
    t_token_type  types[]   = { TOKEN_INT, TOKEN_PLUS, TOKEN_INT, TOKEN_MULT, TOKEN_INT, TOKEN_EOF };
    const char   *lexemes[] = { "1",       "+",        "2",       "*",        "3",       "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 6);
    t_compiler    c         = make_compiler(chain);

    compile_expression(&c);

    EXPECT_EQ_FEEDBACK(5, c.code->count,
        "compile_expression(1+2*3): 5 instructions",
        "* binds tighter than +: PUSH 1, PUSH 2, PUSH 3, MUL, ADD.");
    t_bytecode_instruction *i3 = get_instr(c.code, 3, "expr/prec/mul");
    t_bytecode_instruction *i4 = get_instr(c.code, 4, "expr/prec/add");
    ASSERT_TRUE_FEEDBACK(i3 != NULL && i4 != NULL, "instructions 3 and 4 non-NULL",
        "compile_expression must emit MUL then ADD for 1+2*3.");
    EXPECT_EQ_FEEDBACK(OP_MUL, i3->opcode,
        "compile_expression(1+2*3): opcode[3] == OP_MUL",
        "MUL must appear before ADD.");
    EXPECT_EQ_FEEDBACK(OP_ADD, i4->opcode,
        "compile_expression(1+2*3): opcode[4] == OP_ADD",
        "ADD is the outermost operation.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_expression, comparison_chain)
{
    /* x > 0 → LOAD_VAR x, PUSH 0, GT */
    t_token_type  types[]   = { TOKEN_ID, TOKEN_GT, TOKEN_INT, TOKEN_EOF };
    const char   *lexemes[] = { "x",      ">",      "0",       "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 4);
    t_compiler    c         = make_compiler(chain);

    compile_expression(&c);

    t_bytecode_instruction *i2 = get_instr(c.code, 2, "expr/cmp");
    ASSERT_TRUE_FEEDBACK(i2 != NULL, "instruction 2 non-NULL", "compile_expression must emit GT.");
    EXPECT_EQ_FEEDBACK(OP_GT, i2->opcode,
        "compile_expression(x>0): opcode[2] == OP_GT",
        "Comparison operators must be reachable from compile_expression.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_expression, no_error_on_valid_input)
{
    t_token_type  types[]   = { TOKEN_INT, TOKEN_EOF };
    const char   *lexemes[] = { "7",       "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 2);
    t_compiler    c         = make_compiler(chain);

    compile_expression(&c);

    EXPECT_EQ_FEEDBACK(0, c.had_error,
        "compile_expression(7): had_error == 0",
        "compile_expression must not set had_error on valid input.");

    cleanup_compiler(&c);
    free_chain(chain);
}


/* ═══════════════════════════════════════════════════════════════════════════
   SUITE: compile_let_statement
   Note: TOKEN_LET has already been consumed by compile_statement;
         current starts at TOKEN_ID.
   ═══════════════════════════════════════════════════════════════════════════ */

UTEST(compile_let_statement, simple_int_assignment)
{
    /* x = 42 ; → PUSH 42, STORE_VAR "x" */
    t_token_type  types[]   = { TOKEN_ID, TOKEN_ASSIGN, TOKEN_INT, TOKEN_SEMICOLON, TOKEN_EOF };
    const char   *lexemes[] = { "x",      "=",          "42",      ";",             "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 5);
    t_compiler    c         = make_compiler(chain);

    compile_let_statement(&c);

    ASSERT_TRUE_FEEDBACK(c.code != NULL,
        "compile_let_statement: non-NULL bytecode",
        "compile_let_statement must produce bytecode.");
    EXPECT_EQ_FEEDBACK(2, c.code->count,
        "compile_let_statement(let x=42;): 2 instructions",
        "let x = expr; must emit the expression instructions + OP_STORE_VAR.");
    t_bytecode_instruction *i0 = get_instr(c.code, 0, "let/int/0");
    t_bytecode_instruction *i1 = get_instr(c.code, 1, "let/int/1");
    ASSERT_TRUE_FEEDBACK(i0 != NULL && i1 != NULL, "instructions 0-1 non-NULL",
        "compile_let_statement must emit 2 instructions.");
    EXPECT_EQ_FEEDBACK(OP_PUSH, i0->opcode,
        "compile_let_statement(let x=42;): opcode[0] == OP_PUSH",
        "The RHS expression must be compiled before STORE_VAR.");
    EXPECT_EQ_FEEDBACK(42, i0->operand,
        "compile_let_statement(let x=42;): operand[0] == 42",
        "OP_PUSH operand must equal 42.");
    EXPECT_EQ_FEEDBACK(OP_STORE_VAR, i1->opcode,
        "compile_let_statement(let x=42;): opcode[1] == OP_STORE_VAR",
        "The last instruction must be OP_STORE_VAR.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_let_statement, var_name_stored)
{
    /* y = 0 ; — check STORE_VAR carries the variable name */
    t_token_type  types[]   = { TOKEN_ID, TOKEN_ASSIGN, TOKEN_INT, TOKEN_SEMICOLON, TOKEN_EOF };
    const char   *lexemes[] = { "y",      "=",          "0",       ";",             "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 5);
    t_compiler    c         = make_compiler(chain);

    compile_let_statement(&c);

    t_bytecode_instruction *i1 = get_instr(c.code, 1, "let/name/1");
    ASSERT_TRUE_FEEDBACK(i1 != NULL, "instruction 1 non-NULL",
        "compile_let_statement must emit STORE_VAR.");
    ASSERT_TRUE_FEEDBACK(i1->operand_str != NULL,
        "compile_let_statement(let y=0;): STORE_VAR operand_str non-NULL",
        "OP_STORE_VAR must carry the variable name.");
    EXPECT_STREQ_FEEDBACK("y", i1->operand_str,
        "compile_let_statement(let y=0;): STORE_VAR operand_str == \"y\"",
        "STORE_VAR must carry the exact variable name from the let statement.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_let_statement, expression_rhs)
{
    /* z = 3 + 4 ; → PUSH 3, PUSH 4, ADD, STORE_VAR "z" */
    t_token_type  types[]   = { TOKEN_ID, TOKEN_ASSIGN, TOKEN_INT, TOKEN_PLUS, TOKEN_INT, TOKEN_SEMICOLON, TOKEN_EOF };
    const char   *lexemes[] = { "z",      "=",          "3",       "+",        "4",       ";",             "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 7);
    t_compiler    c         = make_compiler(chain);

    compile_let_statement(&c);

    EXPECT_EQ_FEEDBACK(4, c.code->count,
        "compile_let_statement(let z=3+4;): 4 instructions",
        "let z = 3+4; must emit PUSH 3, PUSH 4, ADD, STORE_VAR z.");
    t_bytecode_instruction *i2 = get_instr(c.code, 2, "let/expr/add");
    t_bytecode_instruction *i3 = get_instr(c.code, 3, "let/expr/store");
    ASSERT_TRUE_FEEDBACK(i2 != NULL && i3 != NULL, "instructions 2-3 non-NULL",
        "compile_let_statement must emit ADD + STORE_VAR for 3+4.");
    EXPECT_EQ_FEEDBACK(OP_ADD, i2->opcode,
        "compile_let_statement(let z=3+4;): opcode[2] == OP_ADD",
        "The expression 3+4 must produce OP_ADD.");
    EXPECT_EQ_FEEDBACK(OP_STORE_VAR, i3->opcode,
        "compile_let_statement(let z=3+4;): opcode[3] == OP_STORE_VAR",
        "STORE_VAR must follow the compiled expression.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_let_statement, no_error_on_valid_input)
{
    t_token_type  types[]   = { TOKEN_ID, TOKEN_ASSIGN, TOKEN_INT, TOKEN_SEMICOLON, TOKEN_EOF };
    const char   *lexemes[] = { "n",      "=",          "1",       ";",             "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 5);
    t_compiler    c         = make_compiler(chain);

    compile_let_statement(&c);

    EXPECT_EQ_FEEDBACK(0, c.had_error,
        "compile_let_statement(let n=1;): had_error == 0",
        "compile_let_statement must not set had_error on valid input.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_let_statement, id_rhs)
{
    /* a = b ; → LOAD_VAR "b", STORE_VAR "a" */
    t_token_type  types[]   = { TOKEN_ID, TOKEN_ASSIGN, TOKEN_ID, TOKEN_SEMICOLON, TOKEN_EOF };
    const char   *lexemes[] = { "a",      "=",          "b",      ";",             "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 5);
    t_compiler    c         = make_compiler(chain);

    compile_let_statement(&c);

    EXPECT_EQ_FEEDBACK(2, c.code->count,
        "compile_let_statement(let a=b;): 2 instructions",
        "let a = b; must emit LOAD_VAR b, STORE_VAR a.");
    t_bytecode_instruction *i0 = get_instr(c.code, 0, "let/id/0");
    ASSERT_TRUE_FEEDBACK(i0 != NULL, "instruction 0 non-NULL",
        "compile_let_statement must emit LOAD_VAR for identifier RHS.");
    EXPECT_EQ_FEEDBACK(OP_LOAD_VAR, i0->opcode,
        "compile_let_statement(let a=b;): opcode[0] == OP_LOAD_VAR",
        "An identifier RHS must produce OP_LOAD_VAR.");

    cleanup_compiler(&c);
    free_chain(chain);
}


/* ═══════════════════════════════════════════════════════════════════════════
   SUITE: compile_print_statement
   Note: TOKEN_PRINT has already been consumed; current starts at TOKEN_LPAREN.
   ═══════════════════════════════════════════════════════════════════════════ */

UTEST(compile_print_statement, single_int)
{
    /* ( 42 ) ; → PUSH 42, PRINT */
    t_token_type  types[]   = { TOKEN_LPAREN, TOKEN_INT, TOKEN_RPAREN, TOKEN_SEMICOLON, TOKEN_EOF };
    const char   *lexemes[] = { "(",          "42",      ")",          ";",             "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 5);
    t_compiler    c         = make_compiler(chain);

    compile_print_statement(&c);

    EXPECT_EQ_FEEDBACK(2, c.code->count,
        "compile_print_statement(print(42);): 2 instructions",
        "print(expr); must emit the expression + OP_PRINT.");
    t_bytecode_instruction *i1 = get_instr(c.code, 1, "print/int/1");
    ASSERT_TRUE_FEEDBACK(i1 != NULL, "instruction 1 non-NULL",
        "compile_print_statement must emit OP_PRINT.");
    EXPECT_EQ_FEEDBACK(OP_PRINT, i1->opcode,
        "compile_print_statement(print(42);): opcode[1] == OP_PRINT",
        "A single-argument print must end with OP_PRINT (with newline).");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_print_statement, single_identifier)
{
    /* ( x ) ; → LOAD_VAR x, PRINT */
    t_token_type  types[]   = { TOKEN_LPAREN, TOKEN_ID, TOKEN_RPAREN, TOKEN_SEMICOLON, TOKEN_EOF };
    const char   *lexemes[] = { "(",          "x",      ")",          ";",             "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 5);
    t_compiler    c         = make_compiler(chain);

    compile_print_statement(&c);

    t_bytecode_instruction *i0 = get_instr(c.code, 0, "print/id/0");
    t_bytecode_instruction *i1 = get_instr(c.code, 1, "print/id/1");
    ASSERT_TRUE_FEEDBACK(i0 != NULL && i1 != NULL, "instructions 0-1 non-NULL",
        "compile_print_statement must emit 2 instructions for print(x);.");
    EXPECT_EQ_FEEDBACK(OP_LOAD_VAR, i0->opcode,
        "compile_print_statement(print(x);): opcode[0] == OP_LOAD_VAR",
        "An identifier argument must produce OP_LOAD_VAR.");
    EXPECT_EQ_FEEDBACK(OP_PRINT, i1->opcode,
        "compile_print_statement(print(x);): opcode[1] == OP_PRINT",
        "Single-argument print must end with OP_PRINT.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_print_statement, single_string)
{
    /* ( "hello" ) ; → PUSH_STR "hello", PRINT */
    t_token_type  types[]   = { TOKEN_LPAREN, TOKEN_STRING, TOKEN_RPAREN, TOKEN_SEMICOLON, TOKEN_EOF };
    const char   *lexemes[] = { "(",          "hello",      ")",          ";",             "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 5);
    t_compiler    c         = make_compiler(chain);

    compile_print_statement(&c);

    t_bytecode_instruction *i0 = get_instr(c.code, 0, "print/str/0");
    ASSERT_TRUE_FEEDBACK(i0 != NULL, "instruction 0 non-NULL",
        "compile_print_statement must emit OP_PUSH_STR for a string argument.");
    EXPECT_EQ_FEEDBACK(OP_PUSH_STR, i0->opcode,
        "compile_print_statement(print(\"hello\");): opcode[0] == OP_PUSH_STR",
        "A string literal must produce OP_PUSH_STR.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_print_statement, two_args_uses_no_newline)
{
    /* ( 1 , 2 ) ; → PUSH 1, PRINT_NO_NEWLINE, PUSH_STR " ", PRINT_NO_NEWLINE, PUSH 2, PRINT */
    t_token_type  types[]   = { TOKEN_LPAREN, TOKEN_INT, TOKEN_COMMA, TOKEN_INT, TOKEN_RPAREN, TOKEN_SEMICOLON, TOKEN_EOF };
    const char   *lexemes[] = { "(",          "1",       ",",         "2",       ")",          ";",             "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 7);
    t_compiler    c         = make_compiler(chain);

    compile_print_statement(&c);

    EXPECT_EQ_FEEDBACK(6, c.code->count,
        "compile_print_statement(print(1,2);): 6 instructions",
        "print(a,b); must emit: expr_a, PRINT_NO_NEWLINE, PUSH_STR \" \", PRINT_NO_NEWLINE, expr_b, PRINT.");
    t_bytecode_instruction *i1 = get_instr(c.code, 1, "print/2arg/1");
    ASSERT_TRUE_FEEDBACK(i1 != NULL, "instruction 1 non-NULL",
        "compile_print_statement must emit PRINT_NO_NEWLINE after first arg.");
    EXPECT_EQ_FEEDBACK(OP_PRINT_NO_NEWLINE, i1->opcode,
        "compile_print_statement(print(1,2);): opcode[1] == OP_PRINT_NO_NEWLINE",
        "When there is a comma, the first argument must use OP_PRINT_NO_NEWLINE.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_print_statement, last_arg_has_newline)
{
    /* ( 1 , 2 ) ; — last instruction must be OP_PRINT */
    t_token_type  types[]   = { TOKEN_LPAREN, TOKEN_INT, TOKEN_COMMA, TOKEN_INT, TOKEN_RPAREN, TOKEN_SEMICOLON, TOKEN_EOF };
    const char   *lexemes[] = { "(",          "1",       ",",         "2",       ")",          ";",             "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 7);
    t_compiler    c         = make_compiler(chain);

    compile_print_statement(&c);

    ASSERT_TRUE_FEEDBACK(c.code->count > 0, "bytecode count > 0",
        "compile_print_statement must emit instructions.");
    t_bytecode_instruction *last = get_instr(c.code, c.code->count - 1, "print/last");
    ASSERT_TRUE_FEEDBACK(last != NULL, "last instruction non-NULL",
        "The last instruction must be reachable.");
    EXPECT_EQ_FEEDBACK(OP_PRINT, last->opcode,
        "compile_print_statement(print(1,2);): last opcode == OP_PRINT",
        "The last argument of print must always end with OP_PRINT (newline).");

    cleanup_compiler(&c);
    free_chain(chain);
}


/* ═══════════════════════════════════════════════════════════════════════════
   SUITE: compile_if_statement
   Note: TOKEN_IF has already been consumed; current starts at TOKEN_LPAREN.
   ═══════════════════════════════════════════════════════════════════════════ */

UTEST(compile_if_statement, instruction_count)
{
    /* if (x) { let y = 1; }
       Tokens (if consumed): ( x ) { let y = 1 ; }
       Expected: LOAD_VAR x, JUMP_IF_FALSE 4, PUSH 1, STORE_VAR y  → count=4 */
    t_token_type  types[]   = {
        TOKEN_LPAREN, TOKEN_ID, TOKEN_RPAREN,
        TOKEN_LBRACE,
            TOKEN_LET, TOKEN_ID, TOKEN_ASSIGN, TOKEN_INT, TOKEN_SEMICOLON,
        TOKEN_RBRACE, TOKEN_EOF
    };
    const char   *lexemes[] = {
        "(",          "x",      ")",
        "{",
            "let",     "y",      "=",         "1",       ";",
        "}",           "EOF"
    };
    t_token      *chain     = make_token_chain(types, lexemes, 11);
    t_compiler    c         = make_compiler(chain);

    compile_if_statement(&c);

    EXPECT_EQ_FEEDBACK(4, c.code->count,
        "compile_if_statement(if(x){let y=1;}): 4 instructions",
        "if(cond){body} must emit: condition, JUMP_IF_FALSE, body instructions.");
    EXPECT_EQ_FEEDBACK(0, c.had_error,
        "compile_if_statement: had_error == 0",
        "compile_if_statement must not set had_error on valid input.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_if_statement, jump_if_false_opcode)
{
    t_token_type  types[]   = {
        TOKEN_LPAREN, TOKEN_ID, TOKEN_RPAREN,
        TOKEN_LBRACE, TOKEN_RBRACE, TOKEN_EOF
    };
    const char   *lexemes[] = {
        "(",          "x",      ")",
        "{",          "}",       "EOF"
    };
    t_token      *chain     = make_token_chain(types, lexemes, 6);
    t_compiler    c         = make_compiler(chain);

    compile_if_statement(&c);

    /* if (x) {} → LOAD_VAR x (0), JUMP_IF_FALSE (1) */
    t_bytecode_instruction *i1 = get_instr(c.code, 1, "if/opcode/1");
    ASSERT_TRUE_FEEDBACK(i1 != NULL, "instruction 1 non-NULL",
        "compile_if_statement must emit JUMP_IF_FALSE after the condition.");
    EXPECT_EQ_FEEDBACK(OP_JUMP_IF_FALSE, i1->opcode,
        "compile_if_statement(if(x){}): opcode[1] == OP_JUMP_IF_FALSE",
        "The instruction after the condition must be OP_JUMP_IF_FALSE.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_if_statement, backpatch_address)
{
    /* if (x) { let y = 1; }
       JUMP_IF_FALSE operand must equal code->count after block (= 4) */
    t_token_type  types[]   = {
        TOKEN_LPAREN, TOKEN_ID, TOKEN_RPAREN,
        TOKEN_LBRACE,
            TOKEN_LET, TOKEN_ID, TOKEN_ASSIGN, TOKEN_INT, TOKEN_SEMICOLON,
        TOKEN_RBRACE, TOKEN_EOF
    };
    const char   *lexemes[] = {
        "(",          "x",      ")",
        "{",
            "let",     "y",      "=",         "1",       ";",
        "}",           "EOF"
    };
    t_token      *chain     = make_token_chain(types, lexemes, 11);
    t_compiler    c         = make_compiler(chain);

    compile_if_statement(&c);

    t_bytecode_instruction *jump = get_instr(c.code, 1, "if/patch/jump");
    ASSERT_TRUE_FEEDBACK(jump != NULL, "JUMP_IF_FALSE instruction non-NULL",
        "compile_if_statement must emit JUMP_IF_FALSE.");
    EXPECT_EQ_FEEDBACK(4, jump->operand,
        "compile_if_statement: JUMP_IF_FALSE operand == 4 (past the block)",
        "Backpatching must set the jump address to the instruction after the block.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_if_statement, empty_block_backpatch)
{
    /* if (1) {} → PUSH 1 (0), JUMP_IF_FALSE 2 (1) — jump points past empty block */
    t_token_type  types[]   = {
        TOKEN_LPAREN, TOKEN_INT, TOKEN_RPAREN,
        TOKEN_LBRACE, TOKEN_RBRACE, TOKEN_EOF
    };
    const char   *lexemes[] = {
        "(",          "1",      ")",
        "{",          "}",       "EOF"
    };
    t_token      *chain     = make_token_chain(types, lexemes, 6);
    t_compiler    c         = make_compiler(chain);

    compile_if_statement(&c);

    t_bytecode_instruction *jump = get_instr(c.code, 1, "if/empty/jump");
    ASSERT_TRUE_FEEDBACK(jump != NULL, "JUMP_IF_FALSE instruction non-NULL",
        "compile_if_statement must emit JUMP_IF_FALSE for an empty block.");
    EXPECT_EQ_FEEDBACK(2, jump->operand,
        "compile_if_statement(if(1){}): JUMP_IF_FALSE operand == 2",
        "For an empty block the jump must point to the instruction right after it (index 2).");

    cleanup_compiler(&c);
    free_chain(chain);
}


/* ═══════════════════════════════════════════════════════════════════════════
   SUITE: compile_while_statement
   Note: TOKEN_WHILE has already been consumed; current starts at TOKEN_LPAREN.
   ═══════════════════════════════════════════════════════════════════════════ */

UTEST(compile_while_statement, instruction_count)
{
    /* while (x) { let y = 1; }
       Expected: LOAD_VAR x(0), JUMP_IF_FALSE 5(1), PUSH 1(2), STORE_VAR y(3), JUMP 0(4) → count=5 */
    t_token_type  types[]   = {
        TOKEN_LPAREN, TOKEN_ID, TOKEN_RPAREN,
        TOKEN_LBRACE,
            TOKEN_LET, TOKEN_ID, TOKEN_ASSIGN, TOKEN_INT, TOKEN_SEMICOLON,
        TOKEN_RBRACE, TOKEN_EOF
    };
    const char   *lexemes[] = {
        "(",          "x",      ")",
        "{",
            "let",     "y",      "=",         "1",       ";",
        "}",           "EOF"
    };
    t_token      *chain     = make_token_chain(types, lexemes, 11);
    t_compiler    c         = make_compiler(chain);

    compile_while_statement(&c);

    EXPECT_EQ_FEEDBACK(5, c.code->count,
        "compile_while_statement(while(x){let y=1;}): 5 instructions",
        "while(cond){body} must emit: condition, JUMP_IF_FALSE, body, JUMP_back.");
    EXPECT_EQ_FEEDBACK(0, c.had_error,
        "compile_while_statement: had_error == 0",
        "compile_while_statement must not set had_error on valid input.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_while_statement, back_jump_opcode_and_address)
{
    /* while (x) { let y = 1; }
       Last instruction must be OP_JUMP with operand 0 (loop_start) */
    t_token_type  types[]   = {
        TOKEN_LPAREN, TOKEN_ID, TOKEN_RPAREN,
        TOKEN_LBRACE,
            TOKEN_LET, TOKEN_ID, TOKEN_ASSIGN, TOKEN_INT, TOKEN_SEMICOLON,
        TOKEN_RBRACE, TOKEN_EOF
    };
    const char   *lexemes[] = {
        "(",          "x",      ")",
        "{",
            "let",     "y",      "=",         "1",       ";",
        "}",           "EOF"
    };
    t_token      *chain     = make_token_chain(types, lexemes, 11);
    t_compiler    c         = make_compiler(chain);

    compile_while_statement(&c);

    t_bytecode_instruction *back = get_instr(c.code, 4, "while/back");
    ASSERT_TRUE_FEEDBACK(back != NULL, "instruction 4 non-NULL",
        "compile_while_statement must emit a back-jump as the last instruction.");
    EXPECT_EQ_FEEDBACK(OP_JUMP, back->opcode,
        "compile_while_statement: opcode[4] == OP_JUMP",
        "The last instruction of a while loop must be an unconditional OP_JUMP.");
    EXPECT_EQ_FEEDBACK(0, back->operand,
        "compile_while_statement: OP_JUMP operand == 0 (loop_start)",
        "The back-jump must point to the first instruction of the condition (index 0).");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_while_statement, exit_jump_backpatch)
{
    /* while (x) { let y = 1; }
       JUMP_IF_FALSE at index 1 must point to 5 (past the loop) */
    t_token_type  types[]   = {
        TOKEN_LPAREN, TOKEN_ID, TOKEN_RPAREN,
        TOKEN_LBRACE,
            TOKEN_LET, TOKEN_ID, TOKEN_ASSIGN, TOKEN_INT, TOKEN_SEMICOLON,
        TOKEN_RBRACE, TOKEN_EOF
    };
    const char   *lexemes[] = {
        "(",          "x",      ")",
        "{",
            "let",     "y",      "=",         "1",       ";",
        "}",           "EOF"
    };
    t_token      *chain     = make_token_chain(types, lexemes, 11);
    t_compiler    c         = make_compiler(chain);

    compile_while_statement(&c);

    t_bytecode_instruction *jump = get_instr(c.code, 1, "while/exit/jump");
    ASSERT_TRUE_FEEDBACK(jump != NULL, "JUMP_IF_FALSE instruction non-NULL",
        "compile_while_statement must emit JUMP_IF_FALSE after the condition.");
    EXPECT_EQ_FEEDBACK(OP_JUMP_IF_FALSE, jump->opcode,
        "compile_while_statement: opcode[1] == OP_JUMP_IF_FALSE",
        "The instruction after the condition must be OP_JUMP_IF_FALSE.");
    EXPECT_EQ_FEEDBACK(5, jump->operand,
        "compile_while_statement: JUMP_IF_FALSE operand == 5 (past the loop)",
        "Backpatching must set the exit-jump to point after the entire loop.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_while_statement, empty_body)
{
    /* while (1) {} → PUSH 1(0), JUMP_IF_FALSE 3(1), JUMP 0(2) */
    t_token_type  types[]   = {
        TOKEN_LPAREN, TOKEN_INT, TOKEN_RPAREN,
        TOKEN_LBRACE, TOKEN_RBRACE, TOKEN_EOF
    };
    const char   *lexemes[] = {
        "(",          "1",      ")",
        "{",          "}",       "EOF"
    };
    t_token      *chain     = make_token_chain(types, lexemes, 6);
    t_compiler    c         = make_compiler(chain);

    compile_while_statement(&c);

    EXPECT_EQ_FEEDBACK(3, c.code->count,
        "compile_while_statement(while(1){}): 3 instructions",
        "An empty while body must produce 3 instructions: cond, JUMP_IF_FALSE, JUMP.");
    t_bytecode_instruction *exit_jump = get_instr(c.code, 1, "while/empty/exit");
    ASSERT_TRUE_FEEDBACK(exit_jump != NULL, "JUMP_IF_FALSE non-NULL", "Must emit JUMP_IF_FALSE.");
    EXPECT_EQ_FEEDBACK(3, exit_jump->operand,
        "compile_while_statement(while(1){}): JUMP_IF_FALSE operand == 3",
        "Exit jump must point past the JUMP instruction at index 2.");

    cleanup_compiler(&c);
    free_chain(chain);
}


/* ═══════════════════════════════════════════════════════════════════════════
   SUITE: compile_block
   ═══════════════════════════════════════════════════════════════════════════ */

UTEST(compile_block, empty_block)
{
    t_token_type  types[]   = { TOKEN_LBRACE, TOKEN_RBRACE, TOKEN_EOF };
    const char   *lexemes[] = { "{",          "}",          "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 3);
    t_compiler    c         = make_compiler(chain);

    compile_block(&c);

    EXPECT_EQ_FEEDBACK(0, c.code->count,
        "compile_block({}): 0 instructions",
        "An empty block must emit no instructions.");
    EXPECT_EQ_FEEDBACK(0, c.had_error,
        "compile_block({}): had_error == 0",
        "An empty block must not set had_error.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_block, single_statement)
{
    /* { let x = 1; } → PUSH 1, STORE_VAR x  (2 instructions) */
    t_token_type  types[]   = {
        TOKEN_LBRACE,
            TOKEN_LET, TOKEN_ID, TOKEN_ASSIGN, TOKEN_INT, TOKEN_SEMICOLON,
        TOKEN_RBRACE, TOKEN_EOF
    };
    const char   *lexemes[] = {
        "{",
            "let",     "x",      "=",         "1",       ";",
        "}",           "EOF"
    };
    t_token      *chain     = make_token_chain(types, lexemes, 8);
    t_compiler    c         = make_compiler(chain);

    compile_block(&c);

    EXPECT_EQ_FEEDBACK(2, c.code->count,
        "compile_block({let x=1;}): 2 instructions",
        "A block with one let statement must emit 2 instructions.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_block, two_statements)
{
    /* { let x = 1; let y = 2; } → 4 instructions */
    t_token_type  types[]   = {
        TOKEN_LBRACE,
            TOKEN_LET, TOKEN_ID, TOKEN_ASSIGN, TOKEN_INT, TOKEN_SEMICOLON,
            TOKEN_LET, TOKEN_ID, TOKEN_ASSIGN, TOKEN_INT, TOKEN_SEMICOLON,
        TOKEN_RBRACE, TOKEN_EOF
    };
    const char   *lexemes[] = {
        "{",
            "let",     "x",      "=",         "1",       ";",
            "let",     "y",      "=",         "2",       ";",
        "}",           "EOF"
    };
    t_token      *chain     = make_token_chain(types, lexemes, 13);
    t_compiler    c         = make_compiler(chain);

    compile_block(&c);

    EXPECT_EQ_FEEDBACK(4, c.code->count,
        "compile_block({let x=1;let y=2;}): 4 instructions",
        "A block with two let statements must emit 4 instructions.");

    cleanup_compiler(&c);
    free_chain(chain);
}


/* ═══════════════════════════════════════════════════════════════════════════
   SUITE: compile_statement
   ═══════════════════════════════════════════════════════════════════════════ */

UTEST(compile_statement, dispatches_let)
{
    /* let x = 1 ; → PUSH 1, STORE_VAR x */
    t_token_type  types[]   = { TOKEN_LET, TOKEN_ID, TOKEN_ASSIGN, TOKEN_INT, TOKEN_SEMICOLON, TOKEN_EOF };
    const char   *lexemes[] = { "let",     "x",      "=",          "1",       ";",             "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 6);
    t_compiler    c         = make_compiler(chain);

    compile_statement(&c);

    EXPECT_EQ_FEEDBACK(2, c.code->count,
        "compile_statement(let x=1;): 2 instructions",
        "compile_statement must dispatch TOKEN_LET to compile_let_statement.");
    t_bytecode_instruction *last = get_instr(c.code, 1, "stmt/let/1");
    ASSERT_TRUE_FEEDBACK(last != NULL, "instruction 1 non-NULL", "compile_statement must emit STORE_VAR.");
    EXPECT_EQ_FEEDBACK(OP_STORE_VAR, last->opcode,
        "compile_statement(let x=1;): opcode[1] == OP_STORE_VAR",
        "Dispatching let must ultimately emit OP_STORE_VAR.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_statement, dispatches_print)
{
    /* print ( 42 ) ; → PUSH 42, PRINT */
    t_token_type  types[]   = { TOKEN_PRINT, TOKEN_LPAREN, TOKEN_INT, TOKEN_RPAREN, TOKEN_SEMICOLON, TOKEN_EOF };
    const char   *lexemes[] = { "print",     "(",          "42",      ")",          ";",             "EOF"     };
    t_token      *chain     = make_token_chain(types, lexemes, 6);
    t_compiler    c         = make_compiler(chain);

    compile_statement(&c);

    ASSERT_TRUE_FEEDBACK(c.code->count > 0, "at least 1 instruction emitted",
        "compile_statement must dispatch TOKEN_PRINT to compile_print_statement.");
    t_bytecode_instruction *last = get_instr(c.code, c.code->count - 1, "stmt/print/last");
    ASSERT_TRUE_FEEDBACK(last != NULL, "last instruction non-NULL",
        "compile_statement must emit OP_PRINT for a print statement.");
    EXPECT_EQ_FEEDBACK(OP_PRINT, last->opcode,
        "compile_statement(print(42);): last opcode == OP_PRINT",
        "Dispatching print must ultimately emit OP_PRINT.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_statement, dispatches_if)
{
    /* if ( 1 ) { } → has OP_JUMP_IF_FALSE */
    t_token_type  types[]   = {
        TOKEN_IF, TOKEN_LPAREN, TOKEN_INT, TOKEN_RPAREN,
        TOKEN_LBRACE, TOKEN_RBRACE, TOKEN_EOF
    };
    const char   *lexemes[] = {
        "if",     "(",          "1",       ")",
        "{",           "}",           "EOF"
    };
    t_token      *chain     = make_token_chain(types, lexemes, 7);
    t_compiler    c         = make_compiler(chain);

    compile_statement(&c);

    ASSERT_TRUE_FEEDBACK(c.code->count >= 2, "at least 2 instructions for if",
        "compile_statement must dispatch TOKEN_IF to compile_if_statement.");
    t_bytecode_instruction *i1 = get_instr(c.code, 1, "stmt/if/1");
    ASSERT_TRUE_FEEDBACK(i1 != NULL, "instruction 1 non-NULL",
        "compile_statement must emit JUMP_IF_FALSE for an if statement.");
    EXPECT_EQ_FEEDBACK(OP_JUMP_IF_FALSE, i1->opcode,
        "compile_statement(if(1){}): opcode[1] == OP_JUMP_IF_FALSE",
        "Dispatching if must produce OP_JUMP_IF_FALSE.");

    cleanup_compiler(&c);
    free_chain(chain);
}

UTEST(compile_statement, dispatches_while)
{
    /* while ( 1 ) { } → has OP_JUMP at end */
    t_token_type  types[]   = {
        TOKEN_WHILE, TOKEN_LPAREN, TOKEN_INT, TOKEN_RPAREN,
        TOKEN_LBRACE, TOKEN_RBRACE, TOKEN_EOF
    };
    const char   *lexemes[] = {
        "while",     "(",          "1",       ")",
        "{",          "}",          "EOF"
    };
    t_token      *chain     = make_token_chain(types, lexemes, 7);
    t_compiler    c         = make_compiler(chain);

    compile_statement(&c);

    ASSERT_TRUE_FEEDBACK(c.code->count >= 3, "at least 3 instructions for while",
        "compile_statement must dispatch TOKEN_WHILE to compile_while_statement.");
    t_bytecode_instruction *last = get_instr(c.code, c.code->count - 1, "stmt/while/last");
    ASSERT_TRUE_FEEDBACK(last != NULL, "last instruction non-NULL",
        "compile_statement must emit OP_JUMP for the back-edge of a while loop.");
    EXPECT_EQ_FEEDBACK(OP_JUMP, last->opcode,
        "compile_statement(while(1){}): last opcode == OP_JUMP",
        "Dispatching while must produce OP_JUMP as the final back-edge instruction.");

    cleanup_compiler(&c);
    free_chain(chain);
}


/* ═══════════════════════════════════════════════════════════════════════════
   SUITE: compile  (full pipeline via t_token_list)
   ═══════════════════════════════════════════════════════════════════════════ */

UTEST(compile, null_input_returns_null)
{
    t_bytecode *code = compile(NULL);
    EXPECT_TRUE_FEEDBACK(code == NULL,
        "compile(NULL) == NULL",
        "compile must return NULL when given a NULL token list.");
}

UTEST(compile, halt_at_end)
{
    /* let x = 1 ; EOF → last instruction must be OP_HALT */
    t_token_type  types[]   = { TOKEN_LET, TOKEN_ID, TOKEN_ASSIGN, TOKEN_INT, TOKEN_SEMICOLON, TOKEN_EOF };
    const char   *lexemes[] = { "let",     "x",      "=",          "1",       ";",             "EOF"     };
    t_token_list *list      = make_token_list(types, lexemes, 6);

    t_bytecode *code = compile(list);

    ASSERT_TRUE_FEEDBACK(code != NULL,
        "compile returns non-NULL for valid input",
        "compile must return a valid bytecode for a well-formed program.");
    ASSERT_TRUE_FEEDBACK(code->count > 0, "bytecode count > 0",
        "compile must emit at least one instruction.");
    t_bytecode_instruction *last = get_instr(code, code->count - 1, "compile/halt");
    ASSERT_TRUE_FEEDBACK(last != NULL, "last instruction non-NULL",
        "compile must emit at least one instruction including HALT.");
    EXPECT_EQ_FEEDBACK(OP_HALT, last->opcode,
        "compile(let x=1;): last opcode == OP_HALT",
        "compile must always emit OP_HALT as the final instruction.");

    free_bytecode(code);
    free_token_list(list);
}

UTEST(compile, single_let_statement)
{
    /* let x = 42 ; EOF → PUSH 42, STORE_VAR x, HALT */
    t_token_type  types[]   = { TOKEN_LET, TOKEN_ID, TOKEN_ASSIGN, TOKEN_INT, TOKEN_SEMICOLON, TOKEN_EOF };
    const char   *lexemes[] = { "let",     "x",      "=",          "42",      ";",             "EOF"     };
    t_token_list *list      = make_token_list(types, lexemes, 6);

    t_bytecode *code = compile(list);

    ASSERT_TRUE_FEEDBACK(code != NULL, "compile returns non-NULL", "compile must succeed.");
    EXPECT_EQ_FEEDBACK(3, code->count,
        "compile(let x=42;): 3 instructions (PUSH, STORE_VAR, HALT)",
        "A single let statement must produce exactly 3 instructions.");
    t_bytecode_instruction *i0 = get_instr(code, 0, "compile/let/0");
    t_bytecode_instruction *i1 = get_instr(code, 1, "compile/let/1");
    ASSERT_TRUE_FEEDBACK(i0 != NULL && i1 != NULL, "instructions 0-1 non-NULL",
        "compile must emit PUSH and STORE_VAR for a let statement.");
    EXPECT_EQ_FEEDBACK(OP_PUSH, i0->opcode,
        "compile(let x=42;): opcode[0] == OP_PUSH",
        "The first instruction must be OP_PUSH for the integer literal.");
    EXPECT_EQ_FEEDBACK(OP_STORE_VAR, i1->opcode,
        "compile(let x=42;): opcode[1] == OP_STORE_VAR",
        "The second instruction must be OP_STORE_VAR.");

    free_bytecode(code);
    free_token_list(list);
}

UTEST(compile, single_print_statement)
{
    /* print ( x ) ; EOF → LOAD_VAR x, PRINT, HALT */
    t_token_type  types[]   = { TOKEN_PRINT, TOKEN_LPAREN, TOKEN_ID, TOKEN_RPAREN, TOKEN_SEMICOLON, TOKEN_EOF };
    const char   *lexemes[] = { "print",     "(",          "x",      ")",          ";",             "EOF"     };
    t_token_list *list      = make_token_list(types, lexemes, 6);

    t_bytecode *code = compile(list);

    ASSERT_TRUE_FEEDBACK(code != NULL, "compile returns non-NULL", "compile must succeed.");
    EXPECT_EQ_FEEDBACK(3, code->count,
        "compile(print(x);): 3 instructions (LOAD_VAR, PRINT, HALT)",
        "A single print statement must produce exactly 3 instructions.");
    t_bytecode_instruction *i1 = get_instr(code, 1, "compile/print/1");
    ASSERT_TRUE_FEEDBACK(i1 != NULL, "instruction 1 non-NULL", "Must emit PRINT.");
    EXPECT_EQ_FEEDBACK(OP_PRINT, i1->opcode,
        "compile(print(x);): opcode[1] == OP_PRINT",
        "The second instruction must be OP_PRINT.");

    free_bytecode(code);
    free_token_list(list);
}

UTEST(compile, multi_statement)
{
    /* let x = 1 ; print ( x ) ; EOF → PUSH 1, STORE_VAR x, LOAD_VAR x, PRINT, HALT */
    t_token_type  types[]   = {
        TOKEN_LET, TOKEN_ID, TOKEN_ASSIGN, TOKEN_INT, TOKEN_SEMICOLON,
        TOKEN_PRINT, TOKEN_LPAREN, TOKEN_ID, TOKEN_RPAREN, TOKEN_SEMICOLON,
        TOKEN_EOF
    };
    const char   *lexemes[] = {
        "let",     "x",      "=",          "1",       ";",
        "print",     "(",          "x",      ")",          ";",
        "EOF"
    };
    t_token_list *list = make_token_list(types, lexemes, 11);

    t_bytecode *code = compile(list);

    ASSERT_TRUE_FEEDBACK(code != NULL, "compile returns non-NULL", "compile must succeed.");
    EXPECT_EQ_FEEDBACK(5, code->count,
        "compile(let x=1; print(x);): 5 instructions",
        "Two statements must produce 5 instructions: PUSH, STORE_VAR, LOAD_VAR, PRINT, HALT.");

    free_bytecode(code);
    free_token_list(list);
}

UTEST(compile, no_error_on_valid_program)
{
    /* Verify compile() returns non-NULL (no error) for a valid program */
    t_token_type  types[]   = { TOKEN_LET, TOKEN_ID, TOKEN_ASSIGN, TOKEN_INT, TOKEN_SEMICOLON, TOKEN_EOF };
    const char   *lexemes[] = { "let",     "v",      "=",          "5",       ";",             "EOF"     };
    t_token_list *list      = make_token_list(types, lexemes, 6);

    t_bytecode *code = compile(list);

    EXPECT_TRUE_FEEDBACK(code != NULL,
        "compile(valid program) != NULL",
        "compile must return a non-NULL bytecode for any syntactically valid program.");

    if (code) free_bytecode(code);
    free_token_list(list);
}

UTEST_MAIN();
