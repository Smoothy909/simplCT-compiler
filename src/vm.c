#include "../headers/vm.h"

/**
 * Creates a new virtual machine
 */
t_vm* create_vm() {
    t_vm* vm = (t_vm*)malloc(sizeof(t_vm));
    if (!vm) {
        fprintf(stderr, "Error: Memory allocation failed for VM\n");
        exit(1);
    }
    
    vm->stack_top = -1; // empty stack
    vm->env = create_environment();
    vm->instructions = NULL;
    vm->instruction_count = 0;
    vm->ip = 0;
    
    return vm;
}

/**
 * Pushes an integer value onto the stack
 */
void push(t_vm* vm, int value) {
    if (vm->stack_top >= STACK_SIZE - 1) {
        fprintf(stderr, "Runtime Error: Stack overflow\n");
        exit(1);
    }
    vm->stack_top++;
    vm->stack[vm->stack_top].type = VALUE_INT;
    vm->stack[vm->stack_top].data.int_val = value;
}

/**
 * Pushes a string onto the stack
 */
void push_string(t_vm* vm, const char* value) {
    if (vm->stack_top >= STACK_SIZE - 1) {
        fprintf(stderr, "Runtime Error: Stack overflow\n");
        exit(1);
    }
    vm->stack_top++;
    vm->stack[vm->stack_top].type = VALUE_STRING;
    vm->stack[vm->stack_top].data.str_val = (char*)value;
}

/**
 * Pops and returns the value from the top of the stack
 */
t_stack_value pop_value(t_vm* vm) {
    if (vm->stack_top < 0) {
        fprintf(stderr, "Runtime Error: Stack underflow\n");
        exit(1);
    }
    return vm->stack[vm->stack_top--];
}

/**
 * Pops an integer (for compatibility with existing code)
 */
int pop(t_vm* vm) {
    t_stack_value val = pop_value(vm);
    if (val.type != VALUE_INT) {
        fprintf(stderr, "Runtime Error: Expected integer value\n");
        exit(1);
    }
    return val.data.int_val;
}

/**
 * Peeks at the value on top of the stack without popping it
 */
t_stack_value peek_value(t_vm* vm) {
    if (vm->stack_top < 0) {
        fprintf(stderr, "Runtime Error: Stack is empty\n");
        exit(1);
    }
    return vm->stack[vm->stack_top];
}

/**
 * Peeks at the integer on top (for compatibility)
 */
int peek(t_vm* vm) {
    t_stack_value val = peek_value(vm);
    if (val.type != VALUE_INT) {
        fprintf(stderr, "Runtime Error: Expected integer value\n");
        exit(1);
    }
    return val.data.int_val;
}

/**
 * Prints the stack state (for debugging)
 */
void print_stack(t_vm* vm) {
    printf("Stack [%d]: ", vm->stack_top + 1);
    for (int i = 0; i <= vm->stack_top; i++) {
        if (vm->stack[i].type == VALUE_INT) {
            printf("%d ", vm->stack[i].data.int_val);
        } else {
            printf("\"%s\" ", vm->stack[i].data.str_val);
        }
    }
    printf("\n");
}

/**
 * Converts the linked list of instructions to an array (for jumps)
 */
static void build_instruction_array(t_vm* vm, t_bytecode* code) {
    vm->instruction_count = code->count;
    vm->instructions = (t_bytecode_instruction**)malloc(sizeof(t_bytecode_instruction*) * code->count);
    
    if (!vm->instructions) {
        fprintf(stderr, "Error: Memory allocation failed for instruction array\n");
        exit(1);
    }
    
    t_bytecode_instruction* current = code->head;
    int index = 0;
    while (current) {
        vm->instructions[index++] = current;
        current = current->next;
    }
}

/**
 * Executes the bytecode
 */
int execute(t_vm* vm, t_bytecode* code) {
    // Build an instruction array to allow jumps
    build_instruction_array(vm, code);
    
    // Reset VM state
    vm->ip = 0;
    vm->stack_top = -1;
    
    // Main execution loop
    while (vm->ip < vm->instruction_count) {
        t_bytecode_instruction* instruction = vm->instructions[vm->ip];
        
        // Debug output (optional - keep commented if not needed)
        // printf("IP=%d: ", vm->ip);
        // print_opcode(instruction->opcode);
        // printf("\n");
        
        switch (instruction->opcode) {
            case OP_PUSH: {
                push(vm, instruction->operand);
                vm->ip++;
                break;
            }
            
            case OP_PUSH_STR: {
                // Push a reference to the string
                push_string(vm, instruction->operand_str);
                vm->ip++;
                break;
            }
            
            case OP_ADD: {
                int b = pop(vm);
                int a = pop(vm);
                push(vm, a + b);
                vm->ip++;
                break;
            }
            
            case OP_SUB: {
                int b = pop(vm);
                int a = pop(vm);
                push(vm, a - b);
                vm->ip++;
                break;
            }
            
            case OP_MUL: {
                int b = pop(vm);
                int a = pop(vm);
                push(vm, a * b);
                vm->ip++;
                break;
            }
            
            case OP_DIV: {
                int b = pop(vm);
                int a = pop(vm);
                if (b == 0) {
                    fprintf(stderr, "Runtime Error: Division by zero\n");
                    return -1;
                }
                push(vm, a / b);
                vm->ip++;
                break;
            }
            
            case OP_EQ: {
                int b = pop(vm);
                int a = pop(vm);
                push(vm, a == b ? 1 : 0);
                vm->ip++;
                break;
            }
            
            case OP_NEQ: {
                int b = pop(vm);
                int a = pop(vm);
                push(vm, a != b ? 1 : 0);
                vm->ip++;
                break;
            }
            
            case OP_LT: {
                int b = pop(vm);
                int a = pop(vm);
                push(vm, a < b ? 1 : 0);
                vm->ip++;
                break;
            }
            
            case OP_GT: {
                int b = pop(vm);
                int a = pop(vm);
                push(vm, a > b ? 1 : 0);
                vm->ip++;
                break;
            }
            
            case OP_LTE: {
                int b = pop(vm);
                int a = pop(vm);
                push(vm, a <= b ? 1 : 0);
                vm->ip++;
                break;
            }
            
            case OP_GTE: {
                int b = pop(vm);
                int a = pop(vm);
                push(vm, a >= b ? 1 : 0);
                vm->ip++;
                break;
            }
            
            case OP_STORE_VAR: {
                t_stack_value value = pop_value(vm);
                // Store the value in the environment
                // For strings, we store the pointer as an integer
                if (value.type == VALUE_INT) {
                    set_variable(vm->env, instruction->operand_str, value.data.int_val);
                } else {
                    // Store string pointer as integer
                    set_variable(vm->env, instruction->operand_str, (int)(long)value.data.str_val);
                }
                vm->ip++;
                break;
            }
            
            case OP_LOAD_VAR: {
                // Get the value from the environment
                int stored_value = get_variable(vm->env, instruction->operand_str);
                
                // We need to determine if this is a string pointer or an integer
                // For now, use heuristic: values > 1000 are likely pointers, < 1000 are integers
                // This is a simplification - in production you'd use a proper type system
                if (stored_value > 1000 || (long)stored_value < -1000) {
                    push_string(vm, (char*)(long)stored_value);
                } else {
                    push(vm, stored_value);
                }
                vm->ip++;
                break;
            }
            
            case OP_PRINT: {
                t_stack_value value = pop_value(vm);
                if (value.type == VALUE_INT) {
                    printf("%d\n", value.data.int_val);
                } else {
                    printf("%s\n", value.data.str_val);
                }
                vm->ip++;
                break;
            }
            
            case OP_PRINT_NO_NEWLINE: {
                t_stack_value value = pop_value(vm);
                if (value.type == VALUE_INT) {
                    printf("%d", value.data.int_val);
                } else {
                    printf("%s", value.data.str_val);
                }
                vm->ip++;
                break;
            }
            
            case OP_JUMP: {
                vm->ip = instruction->operand;
                break;
            }
            
            case OP_JUMP_IF_FALSE: {
                int condition = pop(vm);
                if (condition == 0) {
                    vm->ip = instruction->operand;
                } else {
                    vm->ip++;
                }
                break;
            }
            
            case OP_HALT: {
                return 0; // Execution completed successfully
            }
            
            default: {
                fprintf(stderr, "Runtime Error: Unknown opcode %d\n", instruction->opcode);
                return -1;
            }
        }
    }
    
    return 0; // Execution completed successfully
}

/**
 * Frees the VM memory
 */
void free_vm(t_vm* vm) {
    if (!vm) return;
    
    if (vm->env) {
        free_environment(vm->env);
    }
    
    if (vm->instructions) {
        free(vm->instructions);
    }
    
    free(vm);
}
