#include "../headers/environment.h"

/**
 * Creates a new empty environment
 */
t_environment* create_environment() {
    t_environment* env = (t_environment*)malloc(sizeof(t_environment));
    if (!env) {
        fprintf(stderr, "Error: Memory allocation failed for environment\n");
        exit(1);
    }
    env->head = NULL;
    return env;
}

/**
 * Searches for a variable by name
 * Returns a pointer to the variable if it exists, NULL otherwise
 */
static t_variable* find_variable(t_environment* env, const char* name) {
    t_variable* current = env->head;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * Defines or updates a variable in the environment
 */
void set_variable(t_environment* env, const char* name, int value) {
    // Check if the variable already exists
    t_variable* var = find_variable(env, name);
    
    if (var) {
        // Update the existing value
        var->value = value;
    } else {
        // Create a new variable
        t_variable* new_var = (t_variable*)malloc(sizeof(t_variable));
        if (!new_var) {
            fprintf(stderr, "Error: Memory allocation failed for variable\n");
            exit(1);
        }
        
        new_var->name = (char*)malloc(strlen(name) + 1);
        if (!new_var->name) {
            fprintf(stderr, "Error: Memory allocation failed for variable name\n");
            exit(1);
        }
        strcpy(new_var->name, name);
        new_var->value = value;
        
        // Add to the beginning of the list
        new_var->next = env->head;
        env->head = new_var;
    }
}

/**
 * Retrieves a variable's value from the environment
 */
int get_variable(t_environment* env, const char* name) {
    t_variable* var = find_variable(env, name);
    
    if (var) {
        return var->value;
    } else {
        fprintf(stderr, "Runtime Error: Variable '%s' not defined\n", name);
        return 0; // Return 0 by default
    }
}

/**
 * Checks if a variable exists in the environment
 */
int has_variable(t_environment* env, const char* name) {
    return find_variable(env, name) != NULL;
}

/**
 * Prints all environment variables (for debugging)
 */
void print_environment(t_environment* env) {
    printf("\n=== ENVIRONMENT ===\n");
    
    t_variable* current = env->head;
    if (!current) {
        printf("(empty)\n");
    } else {
        while (current) {
            printf("%s = %d\n", current->name, current->value);
            current = current->next;
        }
    }
    
    printf("=== END ENVIRONMENT ===\n\n");
}

/**
 * Frees the environment memory
 */
void free_environment(t_environment* env) {
    if (!env) return;
    
    t_variable* current = env->head;
    while (current) {
        t_variable* next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
    
    free(env);
}
