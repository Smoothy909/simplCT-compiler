# Detailed test feedback

| Total | Passed | Failed |
| ---: | ---: | ---: |
| 56 | 56 | 0 |

## Test checklist

- :white_check_mark: `advance_pos.does_not_change_line_on_regular_char`
- :white_check_mark: `advance_pos.increments_line_on_newline`
- :white_check_mark: `advance_pos.increments_position_by_one`
- :white_check_mark: `advance_pos.returns_current_char`
- :white_check_mark: `check_keyword.if_keyword`
- :white_check_mark: `check_keyword.let`
- :white_check_mark: `check_keyword.partial_keyword_is_identifier`
- :white_check_mark: `check_keyword.print`
- :white_check_mark: `check_keyword.single_letter_is_identifier`
- :white_check_mark: `check_keyword.unknown_word_is_identifier`
- :white_check_mark: `check_keyword.while_keyword`
- :white_check_mark: `peek_char.does_not_advance_position`
- :white_check_mark: `peek_char.empty_source_returns_null`
- :white_check_mark: `peek_char.returns_char_at_arbitrary_position`
- :white_check_mark: `peek_char.returns_char_at_pos_zero`
- :white_check_mark: `peek_char.returns_null_at_end_of_source`
- :white_check_mark: `peek_next_char.does_not_advance_position`
- :white_check_mark: `peek_next_char.empty_source_returns_null`
- :white_check_mark: `peek_next_char.returns_char_at_arbitrary_position`
- :white_check_mark: `peek_next_char.returns_char_at_pos_plus_one`
- :white_check_mark: `peek_next_char.returns_null_when_next_is_end`
- :white_check_mark: `scan_identifier.multi_letter_identifier`
- :white_check_mark: `scan_identifier.recognizes_if_keyword`
- :white_check_mark: `scan_identifier.recognizes_let_keyword`
- :white_check_mark: `scan_identifier.single_letter_identifier`
- :white_check_mark: `scan_identifier.starts_at_current_position`
- :white_check_mark: `scan_identifier.stops_at_first_non_letter`
- :white_check_mark: `scan_number.multi_digit_number`
- :white_check_mark: `scan_number.single_digit`
- :white_check_mark: `scan_number.starts_at_current_position`
- :white_check_mark: `scan_number.stops_at_first_non_digit`
- :white_check_mark: `scan_number.zero_is_valid`
- :white_check_mark: `scan_string.consumes_closing_quote`
- :white_check_mark: `scan_string.empty_string`
- :white_check_mark: `scan_string.simple_string`
- :white_check_mark: `scan_string.string_with_spaces`
- :white_check_mark: `scan_string.unterminated_string_produces_error`
- :white_check_mark: `tokenize.all_other_keywords`
- :white_check_mark: `tokenize.assign_is_single_equals`
- :white_check_mark: `tokenize.empty_source_gives_only_eof`
- :white_check_mark: `tokenize.equality_is_double_equals`
- :white_check_mark: `tokenize.identifier`
- :white_check_mark: `tokenize.if_condition_with_equality`
- :white_check_mark: `tokenize.integer_literal`
- :white_check_mark: `tokenize.keyword_let`
- :white_check_mark: `tokenize.last_token_is_always_eof`
- :white_check_mark: `tokenize.line_number_is_tracked_across_newlines`
- :white_check_mark: `tokenize.print_string_statement`
- :white_check_mark: `tokenize.simple_declaration`
- :white_check_mark: `tokenize.single_char_comparison_operators`
- :white_check_mark: `tokenize.single_char_operators_and_delimiters`
- :white_check_mark: `tokenize.string_literal`
- :white_check_mark: `tokenize.two_char_comparison_operators`
- :white_check_mark: `tokenize.unknown_character_produces_error`
- :white_check_mark: `tokenize.unterminated_string_produces_error`
- :white_check_mark: `tokenize.whitespace_is_skipped`

:white_check_mark: All tests passed — no failures to detail.
