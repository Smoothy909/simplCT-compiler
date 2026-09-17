#ifndef BYTECODE_H
#define BYTECODE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Opcodes: Virtual Machine Instructions
 * 
 * These instructions form the intermediate language of our compiler.
 * They are executed by the virtual machine (VM) which uses a stack.
 */
typedef enum e_opcode {
    // Stack operations
    OP_PUSH,            // Push a constant value onto the stack
    OP_PUSH_STR,        // Push a reference to a string
    
    // Arithmetic operations
    OP_ADD,             // Pop 2 values, add them, push result
    OP_SUB,             // Pop 2 values, subtract, push result
    OP_MUL,             // Pop 2 values, multiply, push result
    OP_DIV,             // Pop 2 values, divide, push result
    
    // Comparison operations
    OP_EQ,              // Pop 2 values, test ==, push 1 or 0
    OP_NEQ,             // Pop 2 values, test !=, push 1 or 0
    OP_LT,              // Pop 2 values, test <, push 1 or 0
    OP_GT,              // Pop 2 values, test >, push 1 or 0
    OP_LTE,             // Pop 2 values, test <=, push 1 or 0
    OP_GTE,             // Pop 2 values, test >=, push 1 or 0
    
    // Variable management
    OP_STORE_VAR,       // Pop a value and store it in a variable
    OP_LOAD_VAR,        // Load a variable's value and push it onto the stack
    
    // Output instructions
    OP_PRINT,           // Pop a value and print it with newline
    OP_PRINT_NO_NEWLINE,// Pop a value and print it without newline
    
    // Control flow instructions
    OP_JUMP,            // Unconditional jump to an address
    OP_JUMP_IF_FALSE,   // Pop a value, jump if it's false (0)
    
    // Termination instruction
    OP_HALT             // Stop execution
} t_opcode;

/**
 * Bytecode instruction
 * 
 * Represents a single instruction in the bytecode.
 * Each instruction has an opcode and may have an operand (value or variable name).
 */
typedef struct s_bytecode_instruction {
    t_opcode opcode;                    // instruction type
    int operand;                        // numeric operand (for PUSH, JUMP, etc.)
    char* operand_str;                  // string operand (for STORE_VAR, LOAD_VAR)
    struct s_bytecode_instruction* next; // pointer to next instruction
} t_bytecode_instruction;

/**
 * Bytecode structure
 * 
 * Linked list of bytecode instructions with counter.
 */
typedef struct s_bytecode {
    t_bytecode_instruction* head;   // first instruction
    t_bytecode_instruction* tail;   // last instruction
    int count;                      // number of instructions
} t_bytecode;

/* Bytecode creation and manipulation functions */

/**
 * Creates a new empty bytecode list
 */
t_bytecode* create_bytecode();

/**
 * Creates an instruction with a numeric operand
 * Example: create_instruction(OP_PUSH, 42, NULL)
 */
t_bytecode_instruction* create_instruction(t_opcode opcode, int operand, const char* operand_str);

/**
 * Adds an instruction to the end of the bytecode
 */
void add_instruction(t_bytecode* code, t_bytecode_instruction* instruction);

/**
 * Adds a simple instruction (without operand)
 * Example: add_simple_instruction(code, OP_ADD)
 */
void add_simple_instruction(t_bytecode* code, t_opcode opcode);

/**
 * Adds an instruction with numeric operand
 * Example: add_push_instruction(code, 42)
 */
void add_push_instruction(t_bytecode* code, int value);

/**
 * Adds an instruction with string operand (variable name)
 * Example: add_var_instruction(code, OP_STORE_VAR, "x")
 */
void add_var_instruction(t_bytecode* code, t_opcode opcode, const char* var_name);

/**
 * Prints the bytecode (for debugging)
 */
void print_bytecode(t_bytecode* code);

/**
 * Prints an opcode (for debugging)
 */
void print_opcode(t_opcode opcode);

/**
 * Retrieves an instruction by its index (for patching jumps)
 */
t_bytecode_instruction* get_instruction_at(t_bytecode* code, int index);

/**
 * Frees the bytecode memory
 */
void free_bytecode(t_bytecode* code);

#endif
