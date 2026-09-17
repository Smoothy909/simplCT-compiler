#include <stdio.h>
#include <stdlib.h>
#include "../headers/io_utils.h"
#include "../headers/lexer.h"
#include "../headers/compiler.h"
#include "../headers/vm.h"

/* Nombre réel de programmes (on passe de 19 à 4) */
#define PROGRAM_COUNT 19

static const char* program_names[] = {
    "blank-space",
    "digit-sum",
    "fibonacci",
    "gcd",
    "if-in-while",
    "nested-while",
    "no-parenthesis",
    "primes",
    "print-error",
    "sujet",
    "test",
    "test_game_output",
    "test_if",
    "test_multi_string",
    "test_simple",
    "test_string_var",
    "test_strings",
    "test_while",
    "unclosed-str",
};

/* Chemins relatifs simplifiés */
static const char* program_files[] = {
    "../programs/blank-space.simpl",
    "../programs/digit-sum.simpl",
    "../programs/fibonacci.simpl",
    "../programs/gcd.simpl",
    "../programs/if-in-while.simpl",
    "../programs/nested-while.simpl",
    "../programs/no-parenthesis.simpl",
    "../programs/primes.simpl",
    "../programs/print-error.simpl",
    "../programs/sujet.simpl",
    "../programs/test.simpl",
    "../programs/test_game_output.simpl",
    "../programs/test_if.simpl",
    "../programs/test_multi_string.simpl",
    "../programs/test_simple.simpl",
    "../programs/test_string_var.simpl",
    "../programs/test_strings.simpl",
    "../programs/test_while.simpl",
    "../programs/unclosed-str.simpl",
};

/* Lance le pipeline complet */
void run_pipeline(const char* filename) {
    printf("\nChargement du fichier : %s\n", filename);

    char* source = read_file(filename);
    if (!source) {
        printf("Erreur : Impossible de lire le fichier %s\n", filename);
        return;
    }

    printf("\n=== SOURCE CODE ===\n%s\n=== END SOURCE CODE ===\n", source);

    t_token_list* tokens = tokenize(source);
    printf("\n=== TOKEN LIST ===\n");
    print_tokens(tokens);

    t_bytecode* code = compile(tokens);
    if (!code) {
        printf("\nErreur de compilation !\n");
        free(source);
        free_token_list(tokens);
        return;
    }

    printf("\n=== BYTECODE ===\n");
    print_bytecode(code);

    printf("\n=== EXECUTION OUTPUT ===\n");
    t_vm* vm = create_vm();
    execute(vm, code);
    printf("=== END EXECUTION ===\n");

    /* Ménage */
    free_vm(vm);
    free_bytecode(code);
    free_token_list(tokens);
    free(source);
}

int show_menu() {
    int choice = -1;
    printf("\n+--------------------------------------+\n");
    printf("|      SimplCT - Menu Principal        |\n");
    printf("+--------------------------------------+\n");
    for (int i = 0; i < PROGRAM_COUNT; i++) {
        printf("|  %d. %-32s |\n", i + 1, program_names[i]);
    }
    printf("|  0. Quitter                          |\n");
    printf("+--------------------------------------+\n");
    printf("Votre choix : ");

    // Le " %d" avec un espace permet de mieux gérer les entrées clavier
    if (scanf("%d", &choice) != 1) {
        // Si l'utilisateur tape une lettre, on vide le tampon
        while(getchar() != '\n');
        return -1;
    }
    return choice;
}

int main() {
    int choice;
    while (1) {
        choice = show_menu();

        if (choice == 0) {
            printf("Au revoir !\n");
            break;
        }

        if (choice < 1 || choice > PROGRAM_COUNT) {
            printf("\n[!] Choix invalide. Entrez un nombre entre 0 et %d.\n", PROGRAM_COUNT);
            continue;
        }

        printf("\n>>> Lancement de : %s\n", program_names[choice - 1]);
        run_pipeline(program_files[choice - 1]);

        printf("\nAppuyez sur Entrer pour revenir au menu...");
        getchar(); getchar(); // Pause pour laisser le temps de lire
    }
    return 0;
}