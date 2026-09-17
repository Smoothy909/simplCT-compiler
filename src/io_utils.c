#include "../headers/io_utils.h"

char* read_file(const char* filename) {
    /* MODIF : "rb" au lieu de "r" pour eviter les bugs Windows */
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(length + 1);
    if (!buffer) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    /* MODIF : On recupere le nombre exact d'octets lus */
    size_t actual_length = fread(buffer, 1, length, file);
    /* MODIF : On arrete la chaine exactement a la fin du texte lu */
    buffer[actual_length] = '\0';

    fclose(file);
    return buffer;
}