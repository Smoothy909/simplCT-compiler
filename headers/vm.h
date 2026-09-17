#ifndef VM_H
#define VM_H

#include <stdlib.h>
#include <stdio.h>
#include "bytecode.h"
#include "environment.h"

#define STACK_SIZE 256

/**
 * Type of value on the stack
 */
typedef enum e_value_type {
    VALUE_INT,
    VALUE_STRING
} t_value_type;

/**
 * Value on the stack (can be int or string)
 */
typedef struct s_stack_value {
    t_value_type type;
    union {
        int int_val;
        char* str_val;
    } data;
} t_stack_value;

/**
 * Virtual Machine (VM)
 * 
 * Executes bytecode using a stack and an environment (variables).
 * The VM maintains:
 * - A stack for computations
 * - An instruction pointer (IP) to track the current instruction
 * - An environment to store variables
 */
typedef struct s_vm {
    t_stack_value stack[STACK_SIZE];    // execution stack
    int stack_top;                      // top of the stack (index)
    t_environment* env;                 // variable environment
    t_bytecode_instruction** instructions; // array of instructions (for jumps)
    int instruction_count;              // number of instructions
    int ip;                            // instruction pointer (instruction counter)
} t_vm;

/* VM functions */

/**
 * Creates a new virtual machine
 */
t_vm* create_vm();

/**
 * Executes the bytecode
 * Returns 0 on success, -1 on error
 */
int execute(t_vm* vm, t_bytecode* code);

/**
 * Frees the VM memory
 */
void free_vm(t_vm* vm);

/* Stack functions (internal, but exposed for testing) */

/**
 * Pushes an integer value onto the stack
 */
void push(t_vm* vm, int value);

/**
 * Pushes a string onto the stack
 */
void push_string(t_vm* vm, const char* value);

/**
 * Pops and returns the value from the top of the stack
 */
t_stack_value pop_value(t_vm* vm);

/**
 * Pops an integer (for compatibility with existing code)
 */
int pop(t_vm* vm);

/**
 * Peeks at the value on top of the stack without popping it
 */
t_stack_value peek_value(t_vm* vm);

/**
 * Peeks at the integer on top (for compatibility)
 */
int peek(t_vm* vm);

/**
 * Prints the stack state (for debugging)
 */
void print_stack(t_vm* vm);

#endif
