cmake_minimum_required(VERSION 3.24)
project(simplct C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Collect all project sources located in src/.
# CONFIGURE_DEPENDS lets CMake re-scan automatically when files are added/removed.
file(GLOB LIB_SOURCES CONFIGURE_DEPENDS "src/*.c")

if(NOT LIB_SOURCES)
    message(FATAL_ERROR "No source files found in src/. At least one module is required.")
endif()

list(SORT LIB_SOURCES)

add_executable(simplct main.c ${LIB_SOURCES})
target_include_directories(simplct PRIVATE headers)

if(MSVC)
    target_compile_options(simplct PRIVATE /W4)
else()
    target_compile_options(simplct PRIVATE -Wall -Wextra)
endif()

# Tests are disabled by default and can be enabled later when validation tests are added.
option(ENABLE_VALIDATION_TESTS "Build validation test binaries" OFF)
if(ENABLE_VALIDATION_TESTS)
    enable_testing()

    # Auto-discover test sources in tests/.
    file(GLOB TEST_FILES CONFIGURE_DEPENDS "tests/test_*.c")

    # Ensure XML output directory exists.
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/tests/reports/xml")

    foreach(TEST_FILE ${TEST_FILES})
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)

        add_executable(${TEST_NAME} ${TEST_FILE} ${LIB_SOURCES})
        target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_SOURCE_DIR} tests)

        # Discover UTEST() macros in each test source.
        file(READ ${TEST_FILE} TEST_SOURCE)
        string(REGEX MATCHALL "UTEST\\([^,]+,[^)]+\\)" TEST_MATCHES "${TEST_SOURCE}")

        foreach(MATCH ${TEST_MATCHES})
            string(REGEX REPLACE "UTEST\\([ ]*([^, ]+)[ ]*,[ ]*([^) ]+)[ ]*\\)" "\\1.\\2" TEST_CASE "${MATCH}")
            add_test(
                NAME ${TEST_NAME}.${TEST_CASE}
                COMMAND ${TEST_NAME} --filter=${TEST_CASE} --output=${CMAKE_SOURCE_DIR}/tests/reports/xml/${TEST_NAME}.${TEST_CASE}.xml
            )
            set_tests_properties(${TEST_NAME}.${TEST_CASE} PROPERTIES TIMEOUT 30 LABELS "unit")
        endforeach()
    endforeach()
endif()
