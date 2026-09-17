#!/bin/bash
# run_tests.sh — Build and run validation tests locally.
#
# Usage:
#   ./run_tests.sh [part1|part2]   (default: part1)
#
# What it does:
#   1. Configures the CMake build with validation tests enabled.
#   2. Compiles all test executables found in tests/.
#   3. Runs only the tests for the requested part via CTest.
#   4. Writes XML reports to tests/reports/xml/ and Markdown reports
#      to tests/reports/feedback_<PART>.md (concise) and
#      tests/reports/feedback_<PART>.details.md (detailed).
#
# Requirements: cmake >= 3.24, a C11 compiler (gcc or clang).

set -e

PART="${1:-part1}"

case "$PART" in
    part1) TEST_PREFIX="test_lexer" ;;
    part2) TEST_PREFIX="test_compiler" ;;
    *) echo "Usage: $0 [part1|part2]" >&2; exit 1 ;;
esac

BUILD_DIR="cmake-build-debug"
XML_DIR="tests/reports/xml"
REPORT_FILE="tests/reports/feedback_${PART}.md"
REPORT_UTILS="tests/report_utils/generate_test_report.sh"

echo "=== Building tests (${PART}) ==="
mkdir -p "$XML_DIR"
rm -f "${XML_DIR}/${TEST_PREFIX}".*.xml \
      "$REPORT_FILE" "${REPORT_FILE%.md}.details.md"

cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Debug -DENABLE_VALIDATION_TESTS=ON
cmake --build "$BUILD_DIR"

echo ""
echo "=== Running ${PART} tests ==="
cd "$BUILD_DIR"
set +e
ctest -R "^${TEST_PREFIX}\." --output-on-failure
TEST_EXIT_CODE=$?
set -e
cd ..

echo ""
echo "=== Generating Markdown reports ==="
bash "$REPORT_UTILS" "$XML_DIR" "$TEST_PREFIX" "$REPORT_FILE"
echo "Concise : $REPORT_FILE"
echo "Detailed: ${REPORT_FILE%.md}.details.md"

echo ""
if [ "$TEST_EXIT_CODE" -eq 0 ]; then
    echo "All ${PART} tests passed."
else
    echo "Some tests failed. See $REPORT_FILE for details."
fi

exit "$TEST_EXIT_CODE"
