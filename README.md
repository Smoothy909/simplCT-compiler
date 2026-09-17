[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/d8U1gygT)

# <span style="color:#FF5A5A">Project simplCT [EN]</span>

This repository is the base for the simplCT compilation project.

## <span style="color:#FF8B5A">Team members</span>
- `Gabrielle Deininger` (< gabrielle-efr >)
- `Gregoire D'argent` (< Smoothy909 >)
- Group: `P1 INT1`

## <span style="color:#FF8B5A">lexer.c functions</span>
10/04/2026

### <span style="color:#FFA95A">1. work repartitions</span>
- Gabrielle Deininger : `tokenize`, `scan_string`, `README.md`
- Gregoire D'argent : `scan_identifier`, `scan_number`, `check_keyword`, `advance_pos`, `peek_next_char`

### <span style="color:#FFD45A">2. Difficulties encountered</span>
- one bug corrected in `scan_string`: forgot to verify if quotes were closed before the end of the file
- improvement thanks to AI: pointed out the missing `/0` termination check, which we then implemented

## <span style="color:#FF8B5A">compiler.c functions</span>
10/05/2026

### <span style="color:#FFA95A">1. work repartitions</span>
- Gabrielle Deininger : `compile_primary`, `compile_factor`, `compile_term`, `README.md`,`compile_let_statement`, `compile_print_statement`, `compile_if_statement`
- Gregoire D'argent : `compile_comparison`, `compile_equality`, `compile_expression`,`compile_while_statement`, `compile_for_statement`,`compile_block`,`compile_statement`,`compile`

### <span style="color:#FFD45A">2. Difficulties encountered</span>
- big difficulties with git : issues encountered while trying to merge and access part 2
- bug in let compiler and primary compiler

## <span style="color:#FF8B5A">main.c and programs</span>
24/05/2026

### <span style="color:#FFA95A">1. work repartitions</span>
- Gabrielle Deininger : `main.c` (interactive menu), `io_utils.c`, `fibonacci.simpl`, `gcd.simpl`, `README.md`
- Gregoire D'argent : `main.c`, `primes.simpl`, `digit-sum.simpl`

### <span style="color:#FFD45A">2. Difficulties encountered</span>
- **Memory bug in `io_utils.c`**: On Windows, the file reader was capturing "garbage data" (like `LE_SYSMAN=`) from the system memory at the end of the source code. This caused "Primary expression expected" errors during the compilation of the very last line.
- **Resolution with AI assistance**: The issue was linked to the way Windows handles line breaks (`\r\n`). Following AI advice, we opened files in binary mode (`"rb"`) and used the return value of `fread` to manually set the null-terminator (`\0`) at the exact end of the buffer.
- **Pathing issues**: Solved "File not found" errors in CLion by using relative paths (`../programs/`) to correctly point to the source files from the build directory.