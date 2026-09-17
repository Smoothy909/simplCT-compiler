# simplCT

<div align="center">
  <img src="https://img.shields.io/badge/C-11-0F172A?style=flat-square&logo=c&logoColor=white" alt="C" />
  <img src="https://img.shields.io/badge/bytecode-compiler-334155?style=flat-square" alt="Compiler" />
  <img src="https://img.shields.io/badge/VM-runtime-475569?style=flat-square" alt="VM" />
</div>

<p align="center">
  <strong>A compact compiler project written in C</strong><br>
  from source parsing to bytecode execution.
</p>

SimplCT is a small but complete compiler pipeline built in C. It reads a custom imperative language, tokenizes the source, compiles expressions and statements into bytecode, and executes the result with a lightweight virtual machine.

## Overview

The project demonstrates the main stages of a compiler:

- Lexical analysis: tokenization of source code
- Compilation: generation of bytecode from statements and expressions
- Runtime execution: evaluation in a virtual machine
- Sample programs: examples covering arithmetic, conditions, loops, and strings

This repository is intentionally educational and designed to keep the compilation pipeline clear and readable.

## Architecture

The codebase follows a classic compiler pipeline:

- `src/lexer.c` — tokenizes source code
- `src/compiler.c` — compiles expressions and statements into bytecode
- `src/vm.c` — executes bytecode in a virtual machine
- `src/environment.c` — stores runtime variables
- `src/io_utils.c` — reads source files from disk
- `headers/` — shared type definitions and interfaces

The execution flow is:

`source program -> tokens -> bytecode -> runtime environment`

## Supported language features

The implemented language includes:

- variable declarations with `let`
- arithmetic operators: `+`, `-`, `*`, `/`
- comparisons: `<`, `>`, `<=`, `>=`, `==`, `!=`
- conditional statements: `if (...) { ... }`
- loops: `while (...) { ... }` and `for (...) { ... }`
- output: `print(...)`
- string literals and basic string handling

## Project structure

- `main.c` — interactive launcher for sample programs
- `programs/` — example `.simpl` source files
- `src/` — compiler, lexer, VM, and runtime logic
- `headers/` — public declarations and core structures
- `tests/` — validation and reporting tools
- `CMakeLists.txt` — build configuration

## Build

The project uses CMake.

```bash
cmake -S . -B build
cmake --build build
```

This produces the `simplct` executable.

## Run

From the build directory:

```bash
./simplct
```

The application opens an interactive menu listing the available sample programs. Each selected file is read, tokenized, compiled, and executed.

## Example

```simpl
let x = 10;
let y = 5;
let result = x + y;
print(result);
```

This source is parsed, compiled, and then evaluated by the VM.

## Validation

Optional validation tests can be enabled with CMake:

```bash
cmake -S . -B build -DENABLE_VALIDATION_TESTS=ON
ctest --test-dir build --output-on-failure
```

## Notes

This project is designed to be readable and instructional. It prioritizes clarity and educational value over industrial compiler complexity, making it suitable for understanding how a compiler pipeline works from tokenization to runtime execution.

## License

This repository is provided as a project for learning and experimentation. See the repository contents for any additional usage constraints if applicable.