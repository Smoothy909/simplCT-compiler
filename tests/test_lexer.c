/**
 * test_lexer.c — Part 1 validation tests
 *
 * Tests are based strictly on the Part 1 specification.
 * Helper functions build all required state directly (no call to any
 * function under test), following the same principle as the TP labs.
 *
 * Run locally:
 *   ./run_tests.sh
 * or manually:
 *   cmake -B cmake-build-debug -DENABLE_VALIDATION_TESTS=ON
 *   cmake --build cmake-build-debug
 *   ctest --test-dir cmake-build-debug --output-on-failure
 */

#include "utest.h"
#include "headers/lexer.h"
#include "headers/token.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Feedback macros ─────────────────────────────────────────────────────── */

/* Suppress sign-compare warnings from utest.h internals when comparing enums. */
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif

#define EXPECT_EQ_FEEDBACK(expected, actual, checks, hint) \
    EXPECT_EQ_MSG((expected), (actual), "Checks: " checks " | Hint: " hint)

#define EXPECT_TRUE_FEEDBACK(condition, checks, hint) \
    EXPECT_TRUE_MSG((condition), "Checks: " checks " | Hint: " hint)

#define EXPECT_STREQ_FEEDBACK(expected, actual, checks, hint) \
    EXPECT_STREQ_MSG((expected), (actual), "Checks: " checks " | Hint: " hint)

/* ASSERT variant: aborts the test immediately on failure (use for NULL guards). */
#define ASSERT_TRUE_FEEDBACK(condition, checks, hint) \
    ASSERT_TRUE_MSG((condition), "Checks: " checks " | Hint: " hint)

/* ── Helpers ─────────────────────────────────────────────────────────────── */

/*
 * Build a t_lexer at pos=0, line=1 without calling any tested function.
 * Sets length from strlen(source).
 */
static t_lexer make_lexer(const char *source)
{
    t_lexer l;
    l.source = source;
    l.length = (int)strlen(source);
    l.pos    = 0;
    l.line   = 1;
    return l;
}

/*
 * Build a t_lexer at a specific position without calling any tested function.
 */
static t_lexer make_lexer_at(const char *source, int pos)
{
    t_lexer l;
    l.source = source;
    l.length = (int)strlen(source);
    l.pos    = pos;
    l.line   = 1;
    return l;
}

/*
 * Free a single token directly (without using free_token from token.c).
 * create_token() strdup's the lexeme, so we free it explicitly.
 */
static void cleanup_token(t_token *t)
{
    if (!t) return;
    free(t->lexeme);
    free(t);
}

/*
 * Free a token list and all its tokens directly (without using free_token_list).
 */
static void cleanup_token_list(t_token_list *list)
{
    if (!list) return;
    t_token *cur = list->head;
    while (cur) {
        t_token *next = cur->next;
        free(cur->lexeme);
        free(cur);
        cur = next;
    }
    free(list);
}

/* Count all tokens in the list (including TOKEN_EOF). */
static int count_tokens(t_token_list *list)
{
    if (!list) return 0;
    int n = 0;
    for (t_token *cur = list->head; cur; cur = cur->next) n++;
    return n;
}

/* Return the nth token (0-indexed). Returns NULL if out of range. */
static t_token *get_nth_token(t_token_list *list, int n)
{
    if (!list) return NULL;
    t_token *cur = list->head;
    for (int i = 0; i < n && cur; i++) cur = cur->next;
    return cur;
}


/* ═══════════════════════════════════════════════════════════════════════════
 * check_keyword
 *
 * Spec: returns TOKEN_LET/PRINT/IF/WHILE for the four reserved keywords,
 *       TOKEN_ID for everything else.
 * ═══════════════════════════════════════════════════════════════════════════ */

UTEST(check_keyword, let)
{
    EXPECT_EQ_FEEDBACK(TOKEN_LET, check_keyword("let"),
        "check_keyword(\"let\") == TOKEN_LET",
        "\"let\" is a reserved keyword.");
}

UTEST(check_keyword, print)
{
    EXPECT_EQ_FEEDBACK(TOKEN_PRINT, check_keyword("print"),
        "check_keyword(\"print\") == TOKEN_PRINT",
        "\"print\" is a reserved keyword.");
}

UTEST(check_keyword, if_keyword)
{
    EXPECT_EQ_FEEDBACK(TOKEN_IF, check_keyword("if"),
        "check_keyword(\"if\") == TOKEN_IF",
        "\"if\" is a reserved keyword.");
}

UTEST(check_keyword, while_keyword)
{
    EXPECT_EQ_FEEDBACK(TOKEN_WHILE, check_keyword("while"),
        "check_keyword(\"while\") == TOKEN_WHILE",
        "\"while\" is a reserved keyword.");
}

UTEST(check_keyword, unknown_word_is_identifier)
{
    EXPECT_EQ_FEEDBACK(TOKEN_ID, check_keyword("myvar"),
        "check_keyword(\"myvar\") == TOKEN_ID",
        "Anything that is not a keyword must return TOKEN_ID.");
}

UTEST(check_keyword, partial_keyword_is_identifier)
{
    EXPECT_EQ_FEEDBACK(TOKEN_ID, check_keyword("letme"),
        "check_keyword(\"letme\") == TOKEN_ID",
        "\"letme\" starts like \"let\" but is not an exact match; must return TOKEN_ID.");
}

UTEST(check_keyword, single_letter_is_identifier)
{
    EXPECT_EQ_FEEDBACK(TOKEN_ID, check_keyword("x"),
        "check_keyword(\"x\") == TOKEN_ID",
        "A single letter that is not a keyword must return TOKEN_ID.");
}


/* ═══════════════════════════════════════════════════════════════════════════
 * peek_char
 *
 * Spec: returns the character at lexer->pos without advancing pos.
 *       Returns '\0' when pos is at or beyond the end of source.
 * ═══════════════════════════════════════════════════════════════════════════ */

UTEST(peek_char, returns_char_at_pos_zero)
{
    t_lexer l = make_lexer("hello");
    EXPECT_EQ_FEEDBACK('h', peek_char(&l),
        "peek_char at pos=0 on \"hello\" == 'h'",
        "peek_char must return the character at the current position.");
}

UTEST(peek_char, returns_char_at_arbitrary_position)
{
    t_lexer l = make_lexer_at("hello", 3);
    EXPECT_EQ_FEEDBACK('l', peek_char(&l),
        "peek_char at pos=3 on \"hello\" == 'l'",
        "peek_char must return source[pos], not always source[0].");
}

UTEST(peek_char, does_not_advance_position)
{
    t_lexer l = make_lexer("hello");
    peek_char(&l);
    EXPECT_EQ_FEEDBACK(0, l.pos,
        "l.pos == 0 after peek_char",
        "peek_char must not modify lexer->pos.");
}

UTEST(peek_char, returns_null_at_end_of_source)
{
    t_lexer l = make_lexer("hi");
    l.pos = 2; /* exactly at end */
    EXPECT_EQ_FEEDBACK('\0', peek_char(&l),
        "peek_char at pos==length returns '\\0'",
        "peek_char must return '\\0' when pos is at or beyond the end of source.");
}

UTEST(peek_char, empty_source_returns_null)
{
    t_lexer l = make_lexer("");
    EXPECT_EQ_FEEDBACK('\0', peek_char(&l),
        "peek_char on empty source == '\\0'",
        "peek_char on an empty source must return '\\0'.");
}


/* ═══════════════════════════════════════════════════════════════════════════
 * peek_next_char
 *
 * Spec: returns the character at lexer->pos+1 without advancing pos.
 *       Returns '\0' when pos+1 is at or beyond the end of source.
 * ═══════════════════════════════════════════════════════════════════════════ */

UTEST(peek_next_char, returns_char_at_pos_plus_one)
{
    t_lexer l = make_lexer("hello");
    EXPECT_EQ_FEEDBACK('e', peek_next_char(&l),
        "peek_next_char at pos=0 on \"hello\" == 'e'",
        "peek_next_char must return source[pos+1].");
}

UTEST(peek_next_char, returns_char_at_arbitrary_position)
{
    t_lexer l = make_lexer_at("hello", 1); /* 'e' is at 1, 'l' is at 2 */
    EXPECT_EQ_FEEDBACK('l', peek_next_char(&l),
        "peek_next_char at pos=1 on \"hello\" == 'l'",
        "peek_next_char must return source[pos+1] for any pos.");
}

UTEST(peek_next_char, does_not_advance_position)
{
    t_lexer l = make_lexer("hello");
    peek_next_char(&l);
    EXPECT_EQ_FEEDBACK(0, l.pos,
        "l.pos == 0 after peek_next_char",
        "peek_next_char must not modify lexer->pos.");
}

UTEST(peek_next_char, returns_null_when_next_is_end)
{
    t_lexer l = make_lexer("hi"); /* length=2; at pos=1, pos+1=2 == length */
    l.pos = 1;
    EXPECT_EQ_FEEDBACK('\0', peek_next_char(&l),
        "peek_next_char at pos=1 on \"hi\" (length=2) == '\\0'",
        "peek_next_char must return '\\0' when pos+1 is at or beyond end of source.");
}

UTEST(peek_next_char, empty_source_returns_null)
{
    t_lexer l = make_lexer("");
    EXPECT_EQ_FEEDBACK('\0', peek_next_char(&l),
        "peek_next_char on empty source == '\\0'",
        "peek_next_char on empty source must return '\\0'.");
}


/* ═══════════════════════════════════════════════════════════════════════════
 * advance_pos
 *
 * Spec: returns source[pos] and increments pos by 1.
 *       Increments lexer->line when the consumed character is '\n'.
 * ═══════════════════════════════════════════════════════════════════════════ */

UTEST(advance_pos, returns_current_char)
{
    t_lexer l = make_lexer("hello");
    EXPECT_EQ_FEEDBACK('h', advance_pos(&l),
        "advance_pos on \"hello\" at pos=0 returns 'h'",
        "advance_pos must return the character at the current position before advancing.");
}

UTEST(advance_pos, increments_position_by_one)
{
    t_lexer l = make_lexer("hello");
    advance_pos(&l);
    EXPECT_EQ_FEEDBACK(1, l.pos,
        "l.pos == 1 after advance_pos on \"hello\"",
        "advance_pos must increment pos by exactly 1.");
}

UTEST(advance_pos, does_not_change_line_on_regular_char)
{
    t_lexer l = make_lexer("hello");
    advance_pos(&l);
    EXPECT_EQ_FEEDBACK(1, l.line,
        "l.line == 1 after advancing over a regular character",
        "advance_pos must only increment line when the consumed character is '\\n'.");
}

UTEST(advance_pos, increments_line_on_newline)
{
    t_lexer l = make_lexer("\nhello");
    advance_pos(&l); /* consumes '\n' */
    EXPECT_EQ_FEEDBACK(2, l.line,
        "l.line == 2 after advancing over '\\n'",
        "advance_pos must increment lexer->line when the consumed character is a newline.");
}


/* ═══════════════════════════════════════════════════════════════════════════
 * scan_identifier
 *
 * Spec: called when lexer is at a letter. Reads consecutive letters,
 *       calls check_keyword to determine type, returns the token.
 *       pos ends at the first non-letter character.
 * ═══════════════════════════════════════════════════════════════════════════ */

UTEST(scan_identifier, single_letter_identifier)
{
    t_lexer l = make_lexer("x");
    t_token *tok = scan_identifier(&l);

    ASSERT_TRUE_FEEDBACK(tok != NULL,
        "scan_identifier(\"x\") returns non-NULL",
        "scan_identifier must always return a valid token.");
    EXPECT_EQ_FEEDBACK(TOKEN_ID, tok->type,
        "token type == TOKEN_ID for \"x\"",
        "\"x\" is not a keyword; its type must be TOKEN_ID.");
    EXPECT_STREQ_FEEDBACK("x", tok->lexeme,
        "token lexeme == \"x\"",
        "The lexeme must contain the full identifier text.");
    cleanup_token(tok);
}

UTEST(scan_identifier, multi_letter_identifier)
{
    t_lexer l = make_lexer("myvar");
    t_token *tok = scan_identifier(&l);
    ASSERT_TRUE_FEEDBACK(tok != NULL, "scan_identifier(\"myvar\") returns non-NULL", "scan_identifier must always return a valid token.");

    EXPECT_EQ_FEEDBACK(TOKEN_ID, tok->type,
        "token type == TOKEN_ID for \"myvar\"",
        "\"myvar\" is not a keyword.");
    EXPECT_STREQ_FEEDBACK("myvar", tok->lexeme,
        "token lexeme == \"myvar\"",
        "The lexeme must contain all consecutive letters.");
    cleanup_token(tok);
}

UTEST(scan_identifier, recognizes_if_keyword)
{
    t_lexer l = make_lexer("if");
    t_token *tok = scan_identifier(&l);
    ASSERT_TRUE_FEEDBACK(tok != NULL, "scan_identifier(\"if\") returns non-NULL", "scan_identifier must always return a valid token.");

    EXPECT_EQ_FEEDBACK(TOKEN_IF, tok->type,
        "token type == TOKEN_IF for \"if\"",
        "scan_identifier must use check_keyword to detect reserved words.");
    EXPECT_STREQ_FEEDBACK("if", tok->lexeme,
        "token lexeme == \"if\"",
        "The lexeme must still contain the keyword text.");
    cleanup_token(tok);
}

UTEST(scan_identifier, recognizes_let_keyword)
{
    t_lexer l = make_lexer("let");
    t_token *tok = scan_identifier(&l);
    ASSERT_TRUE_FEEDBACK(tok != NULL, "scan_identifier(\"let\") returns non-NULL", "scan_identifier must always return a valid token.");

    EXPECT_EQ_FEEDBACK(TOKEN_LET, tok->type,
        "token type == TOKEN_LET for \"let\"",
        "\"let\" is a reserved keyword.");
    cleanup_token(tok);
}

UTEST(scan_identifier, stops_at_first_non_letter)
{
    t_lexer l = make_lexer("abc ");
    t_token *tok = scan_identifier(&l);
    ASSERT_TRUE_FEEDBACK(tok != NULL, "scan_identifier(\"abc \") returns non-NULL", "scan_identifier must always return a valid token.");

    EXPECT_STREQ_FEEDBACK("abc", tok->lexeme,
        "scan_identifier on \"abc \" reads only \"abc\"",
        "scan_identifier must stop when it encounters a non-letter character.");
    EXPECT_EQ_FEEDBACK(3, l.pos,
        "l.pos == 3 after scanning \"abc\" from \"abc \"",
        "pos must point to the first non-letter character after scanning.");
    cleanup_token(tok);
}

UTEST(scan_identifier, starts_at_current_position)
{
    t_lexer l = make_lexer_at("xyz", 1); /* pos at 'y' */
    t_token *tok = scan_identifier(&l);
    ASSERT_TRUE_FEEDBACK(tok != NULL, "scan_identifier at pos=1 returns non-NULL", "scan_identifier must always return a valid token.");

    EXPECT_STREQ_FEEDBACK("yz", tok->lexeme,
        "scan_identifier at pos=1 of \"xyz\" reads \"yz\"",
        "scan_identifier must start reading from lexer->pos, not from 0.");
    cleanup_token(tok);
}


/* ═══════════════════════════════════════════════════════════════════════════
 * scan_number
 *
 * Spec: called when lexer is at a digit. Reads consecutive digits,
 *       returns a TOKEN_INT token whose lexeme is the digit string.
 *       pos ends at the first non-digit character.
 * ═══════════════════════════════════════════════════════════════════════════ */

UTEST(scan_number, single_digit)
{
    t_lexer l = make_lexer("7");
    t_token *tok = scan_number(&l);

    ASSERT_TRUE_FEEDBACK(tok != NULL,
        "scan_number(\"7\") returns non-NULL",
        "scan_number must always return a valid token.");
    EXPECT_EQ_FEEDBACK(TOKEN_INT, tok->type,
        "token type == TOKEN_INT",
        "scan_number must always produce a TOKEN_INT token.");
    EXPECT_STREQ_FEEDBACK("7", tok->lexeme,
        "token lexeme == \"7\"",
        "The lexeme must contain the digit string.");
    cleanup_token(tok);
}

UTEST(scan_number, multi_digit_number)
{
    t_lexer l = make_lexer("42");
    t_token *tok = scan_number(&l);
    ASSERT_TRUE_FEEDBACK(tok != NULL, "scan_number(\"42\") returns non-NULL", "scan_number must always return a valid token.");

    EXPECT_EQ_FEEDBACK(TOKEN_INT, tok->type,
        "token type == TOKEN_INT for \"42\"",
        "scan_number must produce TOKEN_INT.");
    EXPECT_STREQ_FEEDBACK("42", tok->lexeme,
        "token lexeme == \"42\"",
        "The lexeme must contain all consecutive digits.");
    cleanup_token(tok);
}

UTEST(scan_number, stops_at_first_non_digit)
{
    t_lexer l = make_lexer("123+");
    t_token *tok = scan_number(&l);
    ASSERT_TRUE_FEEDBACK(tok != NULL, "scan_number(\"123+\") returns non-NULL", "scan_number must always return a valid token.");

    EXPECT_STREQ_FEEDBACK("123", tok->lexeme,
        "scan_number on \"123+\" reads only \"123\"",
        "scan_number must stop when it encounters a non-digit character.");
    EXPECT_EQ_FEEDBACK(3, l.pos,
        "l.pos == 3 after scanning \"123\" from \"123+\"",
        "pos must point to the first non-digit character after scanning.");
    cleanup_token(tok);
}

UTEST(scan_number, starts_at_current_position)
{
    t_lexer l = make_lexer_at("1234", 2); /* pos at '3' */
    t_token *tok = scan_number(&l);
    ASSERT_TRUE_FEEDBACK(tok != NULL, "scan_number at pos=2 returns non-NULL", "scan_number must always return a valid token.");

    EXPECT_STREQ_FEEDBACK("34", tok->lexeme,
        "scan_number at pos=2 of \"1234\" reads \"34\"",
        "scan_number must start reading from lexer->pos, not from 0.");
    cleanup_token(tok);
}

UTEST(scan_number, zero_is_valid)
{
    t_lexer l = make_lexer("0");
    t_token *tok = scan_number(&l);
    ASSERT_TRUE_FEEDBACK(tok != NULL, "scan_number(\"0\") returns non-NULL", "scan_number must always return a valid token.");

    EXPECT_EQ_FEEDBACK(TOKEN_INT, tok->type,
        "token type == TOKEN_INT for \"0\"",
        "\"0\" is a valid integer literal.");
    EXPECT_STREQ_FEEDBACK("0", tok->lexeme,
        "token lexeme == \"0\"",
        "The lexeme for the integer zero must be \"0\".");
    cleanup_token(tok);
}


/* ═══════════════════════════════════════════════════════════════════════════
 * scan_string
 *
 * Spec: called when lexer->pos is at the opening '"'. Reads until the
 *       closing '"'. Returns TOKEN_STRING with lexeme = content (no quotes).
 *       Returns TOKEN_ERROR if EOF is reached before the closing '"'.
 *       Closing '"' is consumed (pos ends past it).
 * ═══════════════════════════════════════════════════════════════════════════ */

UTEST(scan_string, simple_string)
{
    t_lexer l = make_lexer("\"hello\"");
    t_token *tok = scan_string(&l);

    ASSERT_TRUE_FEEDBACK(tok != NULL,
        "scan_string on \"\\\"hello\\\"\" returns non-NULL",
        "scan_string must always return a valid token.");
    EXPECT_EQ_FEEDBACK(TOKEN_STRING, tok->type,
        "token type == TOKEN_STRING",
        "A valid string literal must produce TOKEN_STRING.");
    EXPECT_STREQ_FEEDBACK("hello", tok->lexeme,
        "token lexeme == \"hello\" (without surrounding quotes)",
        "The lexeme must contain the string content without the quotes.");
    cleanup_token(tok);
}

UTEST(scan_string, empty_string)
{
    t_lexer l = make_lexer("\"\"");
    t_token *tok = scan_string(&l);
    ASSERT_TRUE_FEEDBACK(tok != NULL, "scan_string(\"\\\"\\\"\") returns non-NULL", "scan_string must always return a valid token.");

    EXPECT_EQ_FEEDBACK(TOKEN_STRING, tok->type,
        "token type == TOKEN_STRING for empty string \"\\\"\\\"\"",
        "An empty string literal must produce TOKEN_STRING.");
    EXPECT_STREQ_FEEDBACK("", tok->lexeme,
        "token lexeme == \"\" for empty string",
        "The lexeme for an empty string literal must be the empty string.");
    cleanup_token(tok);
}

UTEST(scan_string, string_with_spaces)
{
    t_lexer l = make_lexer("\"hello world\"");
    t_token *tok = scan_string(&l);
    ASSERT_TRUE_FEEDBACK(tok != NULL, "scan_string(\"hello world\") returns non-NULL", "scan_string must always return a valid token.");

    EXPECT_EQ_FEEDBACK(TOKEN_STRING, tok->type,
        "token type == TOKEN_STRING for \"\\\"hello world\\\"\"",
        "Strings with spaces must produce TOKEN_STRING.");
    EXPECT_STREQ_FEEDBACK("hello world", tok->lexeme,
        "token lexeme == \"hello world\"",
        "Spaces inside a string literal must be preserved in the lexeme.");
    cleanup_token(tok);
}

UTEST(scan_string, unterminated_string_produces_error)
{
    t_lexer l = make_lexer("\"hello");
    t_token *tok = scan_string(&l);

    ASSERT_TRUE_FEEDBACK(tok != NULL,
        "scan_string on unterminated string returns non-NULL",
        "scan_string must return a token even for unterminated strings.");
    EXPECT_EQ_FEEDBACK(TOKEN_ERROR, tok->type,
        "token type == TOKEN_ERROR for unterminated string",
        "A string with no closing '\"' before EOF must produce TOKEN_ERROR.");
    cleanup_token(tok);
}

UTEST(scan_string, consumes_closing_quote)
{
    /* "\"hi\"" has 4 chars: '"', 'h', 'i', '"'  (indices 0-3)
     * After scanning, pos must be 4 (past the closing quote). */
    t_lexer l = make_lexer("\"hi\"");
    scan_string(&l);
    EXPECT_EQ_FEEDBACK(4, l.pos,
        "l.pos == 4 after scanning \"\\\"hi\\\"\" (length=4)",
        "scan_string must consume the closing '\"', leaving pos past it so "
        "the tokenize loop does not re-process it.");
}


/* ═══════════════════════════════════════════════════════════════════════════
 * tokenize  (integration)
 *
 * Spec: builds the complete token list from a source string.
 *       Skips whitespace, tracks line numbers, appends TOKEN_EOF at the end.
 * ═══════════════════════════════════════════════════════════════════════════ */

UTEST(tokenize, empty_source_gives_only_eof)
{
    t_token_list *list = tokenize("");

    ASSERT_TRUE_FEEDBACK(list != NULL,
        "tokenize(\"\") != NULL",
        "tokenize must always return a valid (non-NULL) token list.");
    ASSERT_TRUE_FEEDBACK(list->head != NULL,
        "list->head != NULL for tokenize(\"\")",
        "Even an empty source must produce at least TOKEN_EOF.");
    EXPECT_EQ_FEEDBACK(TOKEN_EOF, list->head->type,
        "first (and only content) token == TOKEN_EOF for empty source",
        "The only token for an empty source must be TOKEN_EOF.");
    EXPECT_EQ_FEEDBACK(1, count_tokens(list),
        "count == 1 for empty source (only TOKEN_EOF)",
        "An empty source must produce exactly one token: TOKEN_EOF.");
    cleanup_token_list(list);
}

UTEST(tokenize, last_token_is_always_eof)
{
    t_token_list *list = tokenize("let x");
    ASSERT_TRUE_FEEDBACK(list != NULL,
        "tokenize(\"let x\") returns non-NULL",
        "tokenize must never return NULL.");
    t_token *cur = list->head;
    t_token *last = NULL;
    while (cur) { last = cur; cur = cur->next; }

    ASSERT_TRUE_FEEDBACK(last != NULL, "token list is not empty", "last token must be TOKEN_EOF");
    EXPECT_EQ_FEEDBACK(TOKEN_EOF, last->type,
        "last token of any source == TOKEN_EOF",
        "tokenize must always append TOKEN_EOF as the final token.");
    cleanup_token_list(list);
}

UTEST(tokenize, keyword_let)
{
    t_token_list *list = tokenize("let");
    ASSERT_TRUE_FEEDBACK(list != NULL, "tokenize(\"let\") returns non-NULL", "tokenize must never return NULL.");
    t_token *first = get_nth_token(list, 0);
    ASSERT_TRUE_FEEDBACK(first != NULL, "first token is non-NULL", "Token list must contain at least one token.");

    EXPECT_EQ_FEEDBACK(TOKEN_LET, first->type,
        "first token == TOKEN_LET for source \"let\"",
        "\"let\" must be tokenized as TOKEN_LET.");
    EXPECT_STREQ_FEEDBACK("let", first->lexeme,
        "TOKEN_LET lexeme == \"let\"",
        "The lexeme must match the source text.");
    cleanup_token_list(list);
}

UTEST(tokenize, all_other_keywords)
{
    /* print, if, while each produce their keyword token */
    const char      *sources[]  = { "print", "if", "while" };
    t_token_type     expected[] = { TOKEN_PRINT, TOKEN_IF, TOKEN_WHILE };

    for (int i = 0; i < 3; i++) {
        t_token_list *list = tokenize(sources[i]);
        if (!list) continue; /* skip if tokenize not yet implemented */
        t_token *first = get_nth_token(list, 0);
        if (!first) { cleanup_token_list(list); continue; }
        EXPECT_EQ_FEEDBACK(expected[i], first->type,
            "keyword token type matches expected value",
            "Each keyword (print, if, while) must produce its corresponding token type.");
        cleanup_token_list(list);
    }
}

UTEST(tokenize, identifier)
{
    t_token_list *list = tokenize("myvar");
    ASSERT_TRUE_FEEDBACK(list != NULL, "tokenize(\"myvar\") returns non-NULL", "tokenize must never return NULL.");
    t_token *first = get_nth_token(list, 0);
    ASSERT_TRUE_FEEDBACK(first != NULL, "first token is non-NULL", "");

    EXPECT_EQ_FEEDBACK(TOKEN_ID, first->type,
        "first token == TOKEN_ID for \"myvar\"",
        "A non-keyword word must produce TOKEN_ID.");
    EXPECT_STREQ_FEEDBACK("myvar", first->lexeme,
        "TOKEN_ID lexeme == \"myvar\"",
        "The lexeme must match the identifier text.");
    cleanup_token_list(list);
}

UTEST(tokenize, integer_literal)
{
    t_token_list *list = tokenize("42");
    ASSERT_TRUE_FEEDBACK(list != NULL, "tokenize(\"42\") returns non-NULL", "tokenize must never return NULL.");
    t_token *first = get_nth_token(list, 0);
    ASSERT_TRUE_FEEDBACK(first != NULL, "first token is non-NULL", "");

    EXPECT_EQ_FEEDBACK(TOKEN_INT, first->type,
        "first token == TOKEN_INT for \"42\"",
        "An integer literal must produce TOKEN_INT.");
    EXPECT_STREQ_FEEDBACK("42", first->lexeme,
        "TOKEN_INT lexeme == \"42\"",
        "The lexeme must contain the digit string.");
    cleanup_token_list(list);
}

UTEST(tokenize, string_literal)
{
    t_token_list *list = tokenize("\"hello\"");
    ASSERT_TRUE_FEEDBACK(list != NULL, "tokenize returns non-NULL", "tokenize must never return NULL.");
    t_token *first = get_nth_token(list, 0);
    ASSERT_TRUE_FEEDBACK(first != NULL, "first token is non-NULL", "");

    EXPECT_EQ_FEEDBACK(TOKEN_STRING, first->type,
        "first token == TOKEN_STRING for \"\\\"hello\\\"\"",
        "A string literal must produce TOKEN_STRING.");
    EXPECT_STREQ_FEEDBACK("hello", first->lexeme,
        "TOKEN_STRING lexeme == \"hello\" (without quotes)",
        "The string lexeme must not include the surrounding quotes.");
    cleanup_token_list(list);
}

UTEST(tokenize, single_char_operators_and_delimiters)
{
    struct { const char *src; t_token_type expected; } cases[] = {
        { "+", TOKEN_PLUS      },
        { "-", TOKEN_MINUS     },
        { "*", TOKEN_MULT      },
        { "/", TOKEN_DIV       },
        { "(", TOKEN_LPAREN    },
        { ")", TOKEN_RPAREN    },
        { "{", TOKEN_LBRACE    },
        { "}", TOKEN_RBRACE    },
        { ";", TOKEN_SEMICOLON },
        { ",", TOKEN_COMMA     },
    };
    for (int i = 0; i < 10; i++) {
        t_token_list *list = tokenize(cases[i].src);
        if (!list) continue;
        t_token *first = get_nth_token(list, 0);
        if (!first) { cleanup_token_list(list); continue; }
        EXPECT_EQ_FEEDBACK(cases[i].expected, first->type,
            "single-character operator/delimiter token type",
            "Each single-character operator/delimiter must map to its token type.");
        cleanup_token_list(list);
    }
}

UTEST(tokenize, assign_is_single_equals)
{
    t_token_list *list = tokenize("=");
    ASSERT_TRUE_FEEDBACK(list != NULL, "tokenize(\"=\") returns non-NULL", "tokenize must never return NULL.");
    t_token *first = get_nth_token(list, 0);
    ASSERT_TRUE_FEEDBACK(first != NULL, "first token is non-NULL", "");
    EXPECT_EQ_FEEDBACK(TOKEN_ASSIGN, first->type,
        "tokenize(\"=\") first token == TOKEN_ASSIGN",
        "A lone '=' must produce TOKEN_ASSIGN, not TOKEN_EQ.");
    cleanup_token_list(list);
}

UTEST(tokenize, equality_is_double_equals)
{
    t_token_list *list = tokenize("==");
    ASSERT_TRUE_FEEDBACK(list != NULL, "tokenize(\"==\") returns non-NULL", "tokenize must never return NULL.");
    EXPECT_EQ_FEEDBACK(TOKEN_EQ, get_nth_token(list, 0)->type,
        "tokenize(\"==\") first token == TOKEN_EQ",
        "\"==\" must produce a single TOKEN_EQ, not two TOKEN_ASSIGN.");
    /* TOKEN_EQ + TOKEN_EOF = 2 tokens, not 3 */
    EXPECT_EQ_FEEDBACK(2, count_tokens(list),
        "tokenize(\"==\") produces exactly 2 tokens (TOKEN_EQ + TOKEN_EOF)",
        "\"==\" is one token; receiving 3 means it was split into two TOKEN_ASSIGN.");
    cleanup_token_list(list);
}

UTEST(tokenize, two_char_comparison_operators)
{
    struct { const char *src; t_token_type expected; } cases[] = {
        { "!=", TOKEN_NEQ },
        { "<=", TOKEN_LTE },
        { ">=", TOKEN_GTE },
    };
    for (int i = 0; i < 3; i++) {
        t_token_list *list = tokenize(cases[i].src);
        if (!list) continue;
        t_token *first = get_nth_token(list, 0);
        if (!first) { cleanup_token_list(list); continue; }
        EXPECT_EQ_FEEDBACK(cases[i].expected, first->type,
            "two-char operator produces its token type",
            "Two-character operators must be tokenized as a single token.");
        EXPECT_EQ_FEEDBACK(2, count_tokens(list),
            "two-char operator + EOF = 2 tokens total",
            "A two-character operator must produce exactly one token (plus TOKEN_EOF).");
        cleanup_token_list(list);
    }
}

UTEST(tokenize, single_char_comparison_operators)
{
    struct { const char *src; t_token_type expected; } cases[] = {
        { "<", TOKEN_LT },
        { ">", TOKEN_GT },
    };
    for (int i = 0; i < 2; i++) {
        t_token_list *list = tokenize(cases[i].src);
        if (!list) continue;
        t_token *first = get_nth_token(list, 0);
        if (!first) { cleanup_token_list(list); continue; }
        EXPECT_EQ_FEEDBACK(cases[i].expected, first->type,
            "single comparison operator token type",
            "A lone '<' or '>' must produce TOKEN_LT or TOKEN_GT respectively.");
        cleanup_token_list(list);
    }
}

UTEST(tokenize, whitespace_is_skipped)
{
    /* "let x" → TOKEN_LET, TOKEN_ID, TOKEN_EOF (no whitespace token) */
    t_token_list *list = tokenize("let x");
    ASSERT_TRUE_FEEDBACK(list != NULL, "tokenize(\"let x\") returns non-NULL", "tokenize must never return NULL.");

    EXPECT_EQ_FEEDBACK(3, count_tokens(list),
        "tokenize(\"let x\") produces 3 tokens",
        "Whitespace between tokens must be ignored and not produce any token.");
    EXPECT_EQ_FEEDBACK(TOKEN_LET, get_nth_token(list, 0)->type, "token 0 == TOKEN_LET", "");
    EXPECT_EQ_FEEDBACK(TOKEN_ID,  get_nth_token(list, 1)->type, "token 1 == TOKEN_ID",  "");
    EXPECT_EQ_FEEDBACK(TOKEN_EOF, get_nth_token(list, 2)->type, "token 2 == TOKEN_EOF", "");
    cleanup_token_list(list);
}

UTEST(tokenize, line_number_is_tracked_across_newlines)
{
    /* "let\nx" — 'let' is on line 1, 'x' is on line 2 */
    t_token_list *list = tokenize("let\nx");
    ASSERT_TRUE_FEEDBACK(list != NULL, "tokenize(\"let\\nx\") returns non-NULL", "tokenize must never return NULL.");
    t_token *let_tok = get_nth_token(list, 0);
    t_token *x_tok   = get_nth_token(list, 1);
    ASSERT_TRUE_FEEDBACK(let_tok != NULL, "first token is non-NULL", "");
    ASSERT_TRUE_FEEDBACK(x_tok != NULL, "second token is non-NULL", "");

    EXPECT_EQ_FEEDBACK(1, let_tok->line,
        "TOKEN_LET is on line 1",
        "Tokens before any newline must have line == 1.");
    EXPECT_EQ_FEEDBACK(2, x_tok->line,
        "TOKEN_ID after '\\n' is on line 2",
        "After consuming a newline, the line counter must have incremented to 2.");
    cleanup_token_list(list);
}

UTEST(tokenize, unknown_character_produces_error)
{
    t_token_list *list = tokenize("@");
    ASSERT_TRUE_FEEDBACK(list != NULL, "tokenize(\"@\") returns non-NULL", "tokenize must never return NULL.");
    t_token *first = get_nth_token(list, 0);
    ASSERT_TRUE_FEEDBACK(first != NULL, "first token is non-NULL", "");
    EXPECT_EQ_FEEDBACK(TOKEN_ERROR, first->type,
        "tokenize(\"@\") first token == TOKEN_ERROR",
        "An unrecognized character must produce TOKEN_ERROR.");
    cleanup_token_list(list);
}

UTEST(tokenize, unterminated_string_produces_error)
{
    t_token_list *list = tokenize("\"hello");
    ASSERT_TRUE_FEEDBACK(list != NULL, "tokenize(unterminated string) returns non-NULL", "tokenize must never return NULL.");
    t_token *first = get_nth_token(list, 0);
    ASSERT_TRUE_FEEDBACK(first != NULL, "first token is non-NULL", "");
    EXPECT_EQ_FEEDBACK(TOKEN_ERROR, first->type,
        "tokenize(\"\\\"hello\") first token == TOKEN_ERROR",
        "A string with no closing '\"' must produce TOKEN_ERROR.");
    cleanup_token_list(list);
}

UTEST(tokenize, simple_declaration)
{
    /* "let x = 42;" → TOKEN_LET, TOKEN_ID, TOKEN_ASSIGN, TOKEN_INT,
     *                 TOKEN_SEMICOLON, TOKEN_EOF  (6 tokens) */
    t_token_list *list = tokenize("let x = 42;");
    ASSERT_TRUE_FEEDBACK(list != NULL, "tokenize(\"let x = 42;\") returns non-NULL", "tokenize must never return NULL.");

    EXPECT_EQ_FEEDBACK(6, count_tokens(list),
        "tokenize(\"let x = 42;\") produces 6 tokens",
        "Expected: TOKEN_LET TOKEN_ID TOKEN_ASSIGN TOKEN_INT TOKEN_SEMICOLON TOKEN_EOF.");

    EXPECT_EQ_FEEDBACK(TOKEN_LET,       get_nth_token(list, 0)->type, "token 0 == TOKEN_LET",       "");
    EXPECT_EQ_FEEDBACK(TOKEN_ID,        get_nth_token(list, 1)->type, "token 1 == TOKEN_ID",        "");
    EXPECT_EQ_FEEDBACK(TOKEN_ASSIGN,    get_nth_token(list, 2)->type, "token 2 == TOKEN_ASSIGN",    "");
    EXPECT_EQ_FEEDBACK(TOKEN_INT,       get_nth_token(list, 3)->type, "token 3 == TOKEN_INT",       "");
    EXPECT_EQ_FEEDBACK(TOKEN_SEMICOLON, get_nth_token(list, 4)->type, "token 4 == TOKEN_SEMICOLON", "");
    EXPECT_EQ_FEEDBACK(TOKEN_EOF,       get_nth_token(list, 5)->type, "token 5 == TOKEN_EOF",       "");

    EXPECT_STREQ_FEEDBACK("x",  get_nth_token(list, 1)->lexeme, "TOKEN_ID lexeme == \"x\"",  "");
    EXPECT_STREQ_FEEDBACK("42", get_nth_token(list, 3)->lexeme, "TOKEN_INT lexeme == \"42\"", "");
    cleanup_token_list(list);
}

UTEST(tokenize, if_condition_with_equality)
{
    /* "if (x == 0)" → TOKEN_IF, TOKEN_LPAREN, TOKEN_ID, TOKEN_EQ,
     *                 TOKEN_INT, TOKEN_RPAREN, TOKEN_EOF  (7 tokens) */
    t_token_list *list = tokenize("if (x == 0)");
    ASSERT_TRUE_FEEDBACK(list != NULL, "tokenize(\"if (x == 0)\") returns non-NULL", "tokenize must never return NULL.");

    EXPECT_EQ_FEEDBACK(7, count_tokens(list),
        "tokenize(\"if (x == 0)\") produces 7 tokens",
        "Expected: TOKEN_IF TOKEN_LPAREN TOKEN_ID TOKEN_EQ TOKEN_INT TOKEN_RPAREN TOKEN_EOF.");

    EXPECT_EQ_FEEDBACK(TOKEN_IF,     get_nth_token(list, 0)->type, "token 0 == TOKEN_IF",     "");
    EXPECT_EQ_FEEDBACK(TOKEN_LPAREN, get_nth_token(list, 1)->type, "token 1 == TOKEN_LPAREN", "");
    EXPECT_EQ_FEEDBACK(TOKEN_ID,     get_nth_token(list, 2)->type, "token 2 == TOKEN_ID",     "");
    EXPECT_EQ_FEEDBACK(TOKEN_EQ,     get_nth_token(list, 3)->type, "token 3 == TOKEN_EQ",     "");
    EXPECT_EQ_FEEDBACK(TOKEN_INT,    get_nth_token(list, 4)->type, "token 4 == TOKEN_INT",    "");
    EXPECT_EQ_FEEDBACK(TOKEN_RPAREN, get_nth_token(list, 5)->type, "token 5 == TOKEN_RPAREN", "");
    EXPECT_EQ_FEEDBACK(TOKEN_EOF,    get_nth_token(list, 6)->type, "token 6 == TOKEN_EOF",    "");
    cleanup_token_list(list);
}

UTEST(tokenize, print_string_statement)
{
    /* "print(\"hello\");" → TOKEN_PRINT, TOKEN_LPAREN, TOKEN_STRING,
     *                       TOKEN_RPAREN, TOKEN_SEMICOLON, TOKEN_EOF  (6 tokens) */
    t_token_list *list = tokenize("print(\"hello\");");
    ASSERT_TRUE_FEEDBACK(list != NULL, "tokenize(\"print(\\\"hello\\\");\") returns non-NULL", "tokenize must never return NULL.");

    EXPECT_EQ_FEEDBACK(6, count_tokens(list),
        "tokenize(\"print(\\\"hello\\\");\" ) produces 6 tokens",
        "Expected: TOKEN_PRINT TOKEN_LPAREN TOKEN_STRING TOKEN_RPAREN TOKEN_SEMICOLON TOKEN_EOF.");

    EXPECT_EQ_FEEDBACK(TOKEN_PRINT,     get_nth_token(list, 0)->type, "token 0 == TOKEN_PRINT",     "");
    EXPECT_EQ_FEEDBACK(TOKEN_LPAREN,    get_nth_token(list, 1)->type, "token 1 == TOKEN_LPAREN",    "");
    EXPECT_EQ_FEEDBACK(TOKEN_STRING,    get_nth_token(list, 2)->type, "token 2 == TOKEN_STRING",    "");
    EXPECT_EQ_FEEDBACK(TOKEN_RPAREN,    get_nth_token(list, 3)->type, "token 3 == TOKEN_RPAREN",    "");
    EXPECT_EQ_FEEDBACK(TOKEN_SEMICOLON, get_nth_token(list, 4)->type, "token 4 == TOKEN_SEMICOLON", "");
    EXPECT_STREQ_FEEDBACK("hello", get_nth_token(list, 2)->lexeme,
        "TOKEN_STRING lexeme == \"hello\"",
        "String content must not include the surrounding quotes.");
    cleanup_token_list(list);
}

UTEST_MAIN();
