#!/bin/bash
# generate_test_report.sh — Generate Markdown feedback reports from utest XML files.
#
# Usage: generate_test_report.sh <xml_dir> <prefix> <output_md>
#
#   xml_dir    — directory containing XML files (e.g. tests/reports/xml)
#   prefix     — test-file prefix to filter on (e.g. test_lexer, test_compiler)
#   output_md  — path of the concise report (e.g. tests/reports/feedback_part1.md)
#
# Produces four files:
#   <output_md>                — EN concise: grouped by function, deduplicated hints
#   <output_md>.details.md     — EN detailed: full check + got + hint for every failure
#   <output_md>.fr.md          — FR concise (translated)
#   <output_md>.fr.details.md  — FR detailed (translated)

set -e

if [ "$#" -ne 3 ]; then
    echo "Usage: generate_test_report.sh <xml_dir> <prefix> <output_md>" >&2
    exit 1
fi

REPORTS_DIR="$1"
TEST_PREFIX="$2"
OUTPUT_FILE="$3"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
# Use a prefix-specific translations file if it exists, otherwise fall back to the default.
_prefix_tsv="$SCRIPT_DIR/report_translations_${TEST_PREFIX}.fr.tsv"
if [ -f "$_prefix_tsv" ]; then
    TRANSLATIONS_FR="$_prefix_tsv"
else
    TRANSLATIONS_FR="$SCRIPT_DIR/report_translations.fr.tsv"
fi

# Build sibling filenames
if [ "${OUTPUT_FILE##*.}" = "md" ]; then
    DETAILS_OUTPUT_FILE="${OUTPUT_FILE%.md}.details.md"
    FR_OUTPUT_FILE="${OUTPUT_FILE%.md}.fr.md"
    FR_DETAILS_OUTPUT_FILE="${OUTPUT_FILE%.md}.fr.details.md"
else
    DETAILS_OUTPUT_FILE="${OUTPUT_FILE}.details.md"
    FR_OUTPUT_FILE="${OUTPUT_FILE}.fr.md"
    FR_DETAILS_OUTPUT_FILE="${OUTPUT_FILE}.fr.details.md"
fi

TMP_DIR="$(mktemp -d /tmp/test-report.XXXXXX)"
cleanup() { rm -rf "$TMP_DIR"; }
trap cleanup EXIT

# SUMMARY_FILE: test_name TAB function_name TAB status
SUMMARY_FILE="$TMP_DIR/summary.tsv"
# DETAILS_FILE: test_name TAB function_name TAB check TAB actual TAB hint
DETAILS_FILE="$TMP_DIR/details.tsv"

# ── helpers ───────────────────────────────────────────────────────────────────

has_xml_reports() {
    for f in "$REPORTS_DIR"/${TEST_PREFIX}.*.xml; do [ -e "$f" ] && return 0; done
    return 1
}

# lookup_fr <test_name> <field> <source_text>
# Returns translated text if found in report_translations.fr.tsv, else source_text.
lookup_fr() {
    local test_name="$1" field="$2" source_text="$3"
    if [ ! -f "$TRANSLATIONS_FR" ]; then
        printf '%s\n' "$source_text"; return
    fi
    local result
    result="$(awk -F'\t' -v tn="$test_name" -v f="$field" -v src="$source_text" \
        '$1==tn && $2==f && $3==src { print $4; found=1; exit }
         END { if (!found) print src }' "$TRANSLATIONS_FR")"
    printf '%s\n' "$result"
}

parse_xml() {
    local xml_file="$1"
    local test_name function_name

    test_name="$(sed -n 's/.*<testcase name="\([^"]*\)".*/\1/p' "$xml_file" | head -n 1)"
    [ -z "$test_name" ] && test_name="$(basename "$xml_file" .xml)"
    function_name="${test_name%%.*}"

    local status
    if ! grep -q "<testcase" "$xml_file"; then
        status="FAIL"
        printf '%s\t%s\t%s\t\t%s\n' \
            "$test_name" "$function_name" \
            "test binary crashed (likely SEGFAULT)" \
            "Check for NULL pointer dereferences." \
            >> "$DETAILS_FILE"
    elif grep -q "<failure" "$xml_file"; then
        status="FAIL"
        sed -n '/<system-out><!\[CDATA\[/,/\]\]><\/system-out>/p' "$xml_file" | \
        awk -v tn="$test_name" -v fn="$function_name" '
            function trim(s) { gsub(/^[[:space:]]+|[[:space:]]+$/, "", s); return s }
            {
                line = $0
                sub(/^.*<system-out><!\[CDATA\[/, "", line)
                sub(/\]\]><\/system-out>.*$/, "", line)
                line = trim(line)
                if (line ~ /^Actual[[:space:]]*:/) {
                    sub(/^Actual[[:space:]]*:[[:space:]]*/, "", line); actual = line
                } else if (line ~ /^Message[[:space:]]*:[[:space:]]*Checks:/) {
                    sub(/^Message[[:space:]]*:[[:space:]]*Checks:[[:space:]]*/, "", line)
                    checks = line; hint = ""
                    pos = index(line, " | Hint: ")
                    if (pos > 0) {
                        checks = trim(substr(line, 1, pos - 1))
                        hint   = trim(substr(line, pos + 9))
                    }
                    gsub(/\t/, " ", checks); gsub(/\t/, " ", actual); gsub(/\t/, " ", hint)
                    printf "%s\t%s\t%s\t%s\t%s\n", tn, fn, checks, actual, hint
                    actual = ""
                }
            }
        ' >> "$DETAILS_FILE"
    else
        status="PASS"
    fi

    printf '%s\t%s\t%s\n' "$test_name" "$function_name" "$status" >> "$SUMMARY_FILE"
}

# emit_concise <output_file> <locale>
emit_concise() {
    local out="$1" locale="$2"
    local title details_ref
    if [ "$locale" = "fr" ]; then
        title="# Résultats des tests"
        details_ref="_Voir \`${FR_DETAILS_OUTPUT_FILE}\` pour le détail complet._"
        all_passed=":white_check_mark: Tous les tests sont passés."
        some_failed=":x: $failed_tests test(s) échoué(s). Corrigez-les dans l'ordre ci-dessous, puis relancez \`./run_tests.sh\`."
    else
        title="# Test results"
        details_ref="_See \`${DETAILS_OUTPUT_FILE}\` for the full breakdown._"
        all_passed=":white_check_mark: All tests passed."
        some_failed=":x: $failed_tests test(s) failed. Fix them in the order shown below, then re-run \`./run_tests.sh\`."
    fi
    {
        echo "$title"
        echo
        echo "| Total | Passed | Failed |"
        echo "| ---: | ---: | ---: |"
        echo "| $total_tests | $passed_tests | $failed_tests |"
        echo
        if [ "$failed_tests" -eq 0 ]; then
            echo "$all_passed"
        else
            echo "$some_failed"
            echo
            for fn in $failing_functions; do
                echo "## \`$fn\`"
                echo
                awk -F'\t' -v fn="$fn" '$2==fn && $5!="" { print $1 "\t" $5 }' "$DETAILS_FILE" \
                    | sort -u -t$'\t' -k2,2 \
                    | while IFS=$'\t' read -r tn hint; do
                        if [ "$locale" = "fr" ]; then
                            hint="$(lookup_fr "$tn" "hint" "$hint")"
                        fi
                        echo "- $hint"
                      done \
                    | sort -u
                echo
            done
        fi
        echo "---"
        echo "$details_ref"
    } > "$out"
}

# emit_detailed <output_file> <locale>
emit_detailed() {
    local out="$1" locale="$2"
    local title checklist_heading failed_heading
    if [ "$locale" = "fr" ]; then
        title="# Détail des tests"
        checklist_heading="## Liste des tests"
        failed_heading="## Tests échoués"
        check_label="Vérification"
        got_label="Obtenu"
        hint_label="Indice"
        all_ok=":white_check_mark: Tous les tests sont passés — aucun échec à détailler."
    else
        title="# Detailed test feedback"
        checklist_heading="## Test checklist"
        failed_heading="## Failed tests"
        check_label="Check"
        got_label="Got"
        hint_label="Hint"
        all_ok=":white_check_mark: All tests passed — no failures to detail."
    fi
    {
        echo "$title"
        echo
        echo "| Total | Passed | Failed |"
        echo "| ---: | ---: | ---: |"
        echo "| $total_tests | $passed_tests | $failed_tests |"
        echo
        echo "$checklist_heading"
        echo
        sort "$SUMMARY_FILE" | while IFS=$'\t' read -r test_name _ status; do
            if [ "$status" = "PASS" ]; then
                echo "- :white_check_mark: \`$test_name\`"
            else
                echo "- :x: \`$test_name\`"
            fi
        done
        echo
        if [ "$failed_tests" -gt 0 ]; then
            echo "$failed_heading"
            echo
            for fn in $failing_functions; do
                echo "### \`$fn\`"
                echo
                awk -F'\t' -v fn="$fn" '$2==fn { print }' "$DETAILS_FILE" | \
                while IFS=$'\t' read -r tn _ chk actual hint; do
                    if [ "$locale" = "fr" ]; then
                        chk="$(lookup_fr "$tn" "check" "$chk")"
                        hint="$(lookup_fr "$tn" "hint" "$hint")"
                    fi
                    printf '**`%s`**\n' "$tn"
                    echo "- ${check_label}: ${chk}"
                    [ -n "$actual" ] && echo "  - ${got_label}: \`${actual}\`"
                    [ -n "$hint" ]   && echo "- ${hint_label}: ${hint}"
                    echo
                done
            done
        else
            echo "$all_ok"
        fi
    } > "$out"
}

# ── gather data ───────────────────────────────────────────────────────────────

if ! has_xml_reports; then
    for f in "$OUTPUT_FILE" "$DETAILS_OUTPUT_FILE" "$FR_OUTPUT_FILE" "$FR_DETAILS_OUTPUT_FILE"; do
        printf '# Unit test feedback\n\nNo XML reports found. Run `./run_tests.sh` first.\n' > "$f"
    done
    exit 0
fi

: > "$SUMMARY_FILE"
: > "$DETAILS_FILE"

for xml_file in "$REPORTS_DIR"/${TEST_PREFIX}.*.xml; do
    [ -e "$xml_file" ] || continue
    parse_xml "$xml_file"
done

total_tests="$(wc -l < "$SUMMARY_FILE" | tr -d ' ')"
passed_tests="$(awk -F'\t' '$3=="PASS"{c++} END{print c+0}' "$SUMMARY_FILE")"
failed_tests="$(awk -F'\t' '$3=="FAIL"{c++} END{print c+0}' "$SUMMARY_FILE")"

# List of distinct functions that have at least one failure, in sorted order
failing_functions="$(awk -F'\t' '$3=="FAIL"{print $2}' "$SUMMARY_FILE" | sort -u)"

# ── emit all four reports ─────────────────────────────────────────────────────
emit_concise  "$OUTPUT_FILE"          "en"
emit_detailed "$DETAILS_OUTPUT_FILE"  "en"
emit_concise  "$FR_OUTPUT_FILE"       "fr"
emit_detailed "$FR_DETAILS_OUTPUT_FILE" "fr"
