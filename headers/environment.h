#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Variable in the environment
 * 
 * Structure to store a variable with its name and value.
 * Uses a linked list to allow multiple variables.
 */
typedef struct s_variable {
    char* name;                 // variable name
    int value;                  // variable value (integer)
    struct s_variable* next;    // pointer to next variable
} t_variable;

/**
 * Environment (symbol table)
 * 
 * Linked list of variables representing the execution environment.
 * Allows storing and retrieving variable values.
 */
typedef struct s_environment {
    t_variable* head;   // first variable in the list
} t_environment;

/* Environment management functions */

/**
 * Creates a new empty environment
 */
t_environment* create_environment();

/**
 * Defines or updates an integer variable in the environment
 */
void set_variable(t_environment* env, const char* name, int value);

/**
 * Retrieves a variable's value from the environment
 * Returns the value if the variable exists, otherwise prints an error and returns 0
 */
int get_variable(t_environment* env, const char* name);

/**
 * Checks if a variable exists in the environment
 * Returns 1 if it exists, 0 otherwise
 */
int has_variable(t_environment* env, const char* name);

/**
 * Prints all environment variables (for debugging)
 */
void print_environment(t_environment* env);

/**
 * Frees the environment memory
 */
void free_environment(t_environment* env);

#endif
