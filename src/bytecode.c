#include "../headers/bytecode.h"

/**
 * Creates a new empty bytecode list
 */
t_bytecode* create_bytecode() {
    t_bytecode* code = (t_bytecode*)malloc(sizeof(t_bytecode));
    if (!code) {
        fprintf(stderr, "Error: Memory allocation failed for bytecode\n");
        exit(1);
    }
    code->head = NULL;
    code->tail = NULL;
    code->count = 0;
    return code;
}

/**
 * Creates an instruction with opcode and operands
 */
t_bytecode_instruction* create_instruction(t_opcode opcode, int operand, const char* operand_str) {
    t_bytecode_instruction* instruction = (t_bytecode_instruction*)malloc(sizeof(t_bytecode_instruction));
    if (!instruction) {
        fprintf(stderr, "Error: Memory allocation failed for instruction\n");
        exit(1);
    }
    instruction->opcode = opcode;
    instruction->operand = operand;
    
    // Copy the string if provided
    if (operand_str) {
        instruction->operand_str = (char*)malloc(strlen(operand_str) + 1);
        if (!instruction->operand_str) {
            fprintf(stderr, "Error: Memory allocation failed for operand string\n");
            exit(1);
        }
        strcpy(instruction->operand_str, operand_str);
    } else {
        instruction->operand_str = NULL;
    }
    
    instruction->next = NULL;
    return instruction;
}

/**
 * Adds an instruction to the end of the bytecode
 */
void add_instruction(t_bytecode* code, t_bytecode_instruction* instruction) {
    if (!code->head) {
        // First element
        code->head = instruction;
        code->tail = instruction;
    } else {
        // Add to the end
        code->tail->next = instruction;
        code->tail = instruction;
    }
    code->count++;
}

/**
 * Adds a simple instruction (without operand)
 */
void add_simple_instruction(t_bytecode* code, t_opcode opcode) {
    t_bytecode_instruction* instruction = create_instruction(opcode, 0, NULL);
    add_instruction(code, instruction);
}

/**
 * Adds an instruction with numeric operand
 */
void add_push_instruction(t_bytecode* code, int value) {
    t_bytecode_instruction* instruction = create_instruction(OP_PUSH, value, NULL);
    add_instruction(code, instruction);
}

/**
 * Adds an instruction with string operand (variable name)
 */
void add_var_instruction(t_bytecode* code, t_opcode opcode, const char* var_name) {
    t_bytecode_instruction* instruction = create_instruction(opcode, 0, var_name);
    add_instruction(code, instruction);
}

/**
 * Prints an opcode (for debugging)
 */
void print_opcode(t_opcode opcode) {
    switch (opcode) {
        case OP_PUSH:          printf("PUSH"); break;        case OP_PUSH_STR:      printf("PUSH_STR"); break;        case OP_ADD:           printf("ADD"); break;
        case OP_SUB:           printf("SUB"); break;
        case OP_MUL:           printf("MUL"); break;
        case OP_DIV:           printf("DIV"); break;
        case OP_EQ:            printf("EQ"); break;
        case OP_NEQ:           printf("NEQ"); break;
        case OP_LT:            printf("LT"); break;
        case OP_GT:            printf("GT"); break;
        case OP_LTE:           printf("LTE"); break;
        case OP_GTE:           printf("GTE"); break;
        case OP_STORE_VAR:     printf("STORE_VAR"); break;
        case OP_LOAD_VAR:      printf("LOAD_VAR"); break;
        case OP_PRINT:         printf("PRINT"); break;
        case OP_PRINT_NO_NEWLINE: printf("PRINT_NO_NEWLINE"); break;
        case OP_JUMP:          printf("JUMP"); break;
        case OP_JUMP_IF_FALSE: printf("JUMP_IF_FALSE"); break;
        case OP_HALT:          printf("HALT"); break;
        default:               printf("UNKNOWN"); break;
    }
}

/**
 * Prints the bytecode (for debugging)
 */
void print_bytecode(t_bytecode* code) {
    printf("\n=== BYTECODE (%d instructions) ===\n", code->count);
    
    t_bytecode_instruction* current = code->head;
    int address = 0;
    
    while (current) {
        printf("%04d: ", address);
        print_opcode(current->opcode);
        
        // Display operand according to instruction type
        switch (current->opcode) {
            case OP_PUSH:
                printf(" %d", current->operand);
                break;
            case OP_PUSH_STR:
                printf(" \"%s\"", current->operand_str ? current->operand_str : "");
                break;
            case OP_STORE_VAR:
            case OP_LOAD_VAR:
                printf(" %s", current->operand_str);
                break;
            case OP_JUMP:
            case OP_JUMP_IF_FALSE:
                printf(" %d", current->operand);
                break;
            default:
                // No operand to display
                break;
        }
        
        printf("\n");
        current = current->next;
        address++;
    }
    
    printf("=== END BYTECODE ===\n\n");
}

/**
 * Retrieves an instruction by its index
 */
t_bytecode_instruction* get_instruction_at(t_bytecode* code, int index) {
    if (index < 0 || index >= code->count) {
        return NULL;
    }
    
    t_bytecode_instruction* current = code->head;
    for (int i = 0; i < index; i++) {
        current = current->next;
    }
    
    return current;
}

/**
 * Frees the bytecode memory
 */
void free_bytecode(t_bytecode* code) {
    if (!code) return;
    
    t_bytecode_instruction* current = code->head;
    while (current) {
        t_bytecode_instruction* next = current->next;
        if (current->operand_str) {
            free(current->operand_str);
        }
        free(current);
        current = next;
    }
    
    free(code);
}
