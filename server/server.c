#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMMAND_LEN 100
#define MAX_ARG_LEN 100
#define MAX_ARG_COUNT 10

void moveLeft(int distance) {
    printf("Eseguo movimento a sinistra di: %d\n", distance);
}

void moveRight(int distance) {
    printf("Eseguo movimento a destra di: %d\n", distance);
}

void divide_input(const char* input, char* command, char arguments[][MAX_ARG_LEN], int* arg_count) {

    // Trova la prima virgola e la separa dagli argomenti
    const char* separator_pos = strchr(input, ',');
    if (separator_pos == NULL) {
        // Nessun argomento
        strncpy(command, input, MAX_COMMAND_LEN - 1);
        command[MAX_COMMAND_LEN - 1] = '\0'; // Termina la stringa
        *arg_count = 0;
        return;
    }

    // Estrazione comando
    size_t command_len = separator_pos - input;
    strncpy(command, input, command_len);
    command[command_len] = '\0';

    // Estrazione argomenti
    *arg_count = 0;
    const char* start = separator_pos + 1;
    while (*start != '\0' && *arg_count < MAX_ARG_COUNT) {
        const char* end = strchr(start, ',');
        if (end == NULL) {
            end = start + strlen(start);
        }

        size_t arg_len = end - start;
        if (arg_len >= MAX_ARG_LEN) {
            arg_len = MAX_ARG_LEN - 1; // Tronca la stringa se troppo lunga
        }
        strncpy(arguments[*arg_count], start, arg_len);
        arguments[*arg_count][arg_len] = '\0';
        *arg_count += 1;

        if (*end == '\0') {
            break;
        }
        start = end + 1;
    }
}

void parse_command(const char* input) {
    char command[MAX_COMMAND_LEN];
    char arguments[MAX_ARG_COUNT][MAX_ARG_LEN];
    int arg_count;

    // Divisione dell'input in comando e argomenti
    divide_input(input, command, arguments, &arg_count);

    // Parsing del comando e ricerca di una funzione appropriata per gestirlo
    if (strcmp(command, "moveLeft") == 0) {
        int distance = atoi(arguments[0]);
        moveLeft(distance);
    }
    else if (strcmp(command, "moveRight") == 0) {
        int distance = atoi(arguments[0]);
        moveRight(distance);
    }
    else {
        printf("Invalid command: %s\n", command);
    }
}

int main(int argc, char** argv) {

    if (argc != 2) {
        printf("No argument found!");
        return 1;
    }

    char* input = strdup(argv[1]);
    parse_command(input);

    return 0;
}
