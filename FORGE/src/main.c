#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raster.h"
#include "frame.h"
#include "anvil.h"

static void print_help(void) {
    printf("FORGE v0.2.0-alpha - Compilador oficial del lenguaje Kryon (Proyecto XZERO)\n\n");
    printf("Uso:\n");
    printf("  forge <comando> [archivo.kry] [-o salida]\n\n");
    printf("Comandos disponibles:\n");
    printf("  version    Muestra la versión actual de FORGE\n");
    printf("  build      Compila un archivo .kry a binario nativo (.kyb)\n");
    printf("  run        Ejecuta un archivo .kry mostrando tokens y AST\n");
    printf("  help       Muestra esta ayuda\n");
}

static char *read_file(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = '\0';
    fclose(f);
    return buf;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "FORGE Error: Faltan argumentos. Prueba 'forge help'.\n");
        return 1;
    }

    if (strcmp(argv[1], "help") == 0) {
        print_help();
        return 0;
    }

    if (strcmp(argv[1], "version") == 0) {
        printf("FORGE v0.2.0-alpha\n");
        return 0;
    }

    if (strcmp(argv[1], "build") == 0 || strcmp(argv[1], "run") == 0) {
        if (argc < 3) {
            fprintf(stderr, "FORGE Error: Falta el archivo fuente .kry\n");
            return 1;
        }

        const char *input_file = argv[2];
        const char *output_file = "output.kyb";

        if (argc >= 5 && strcmp(argv[3], "-o") == 0) {
            output_file = argv[4];
        }

        char *source = read_file(input_file);
        if (!source) {
            fprintf(stderr, "FORGE Error: No se pudo abrir el archivo '%s'\n", input_file);
            return 1;
        }

        printf("[FORGE PIPELINE] Compilando '%s'...\n", input_file);

        int token_count = 0;
        Token *tokens = raster_tokenize(source, &token_count);
        if (!tokens) {
            fprintf(stderr, "FORGE Error: Fallo en el análisis léxico (Raster).\n");
            free(source);
            return 1;
        }

        if (strcmp(argv[1], "run") == 0) {
            printf("--- TOKENS ---\n");
            for (int i = 0; i < token_count; i++) {
                printf("Token(%d): type=%d, lexeme='%s'\n", i, tokens[i].type, tokens[i].lexeme);
            }
        }

        ASTNode *root = frame_parse(tokens, token_count);
        if (!root) {
            fprintf(stderr, "FORGE Error: Fallo en el análisis sintáctico (AST).\n");
            free(tokens);
            free(source);
            return 1;
        }

        if (strcmp(argv[1], "run") == 0) {
            printf("--- AST parseado exitosamente ---\n");
        }

        if (!anvil_generate_code(root, output_file)) {
            fprintf(stderr, "FORGE Error: Fallo en la generación de código (Anvil).\n");
            free_ast(root);
            free(tokens);
            free(source);
            return 1;
        }

        printf("[FORGE PIPELINE] Compilación exitosa -> '%s'\n", output_file);

        free_ast(root);
        free(tokens);
        free(source);
        return 0;
    }

    fprintf(stderr, "FORGE Error: Comando desconocido '%s'. Prueba 'forge help'.\n", argv[1]);
    return 1;
}
