#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raster.h"
#include "frame.h"
#include "anvil.h"

#define FORGE_VERSION "0.2.0-alpha"

void print_help() {
    printf("FORGE v%s - Compilador oficial del lenguaje Kryon (Proyecto XZERO)\n\n", FORGE_VERSION);
    printf("Uso:\n  forge <comando> [archivo.kry] [-o salida]\n\n");
    printf("Comandos disponibles:\n");
    printf("  version    Muestra la versión actual de FORGE\n");
    printf("  build      Compila un archivo .kry a binario nativo\n");
    printf("  run        Ejecuta un archivo .kry mostrando tokens y AST\n");
    printf("  help       Muestra esta ayuda\n");
}

char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        printf("FORGE Error: No se pudo abrir el archivo '%s'.\n", path);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(length + 1);
    if (!buffer) {
        printf("FORGE Error: Memoria insuficiente.\n");
        fclose(file);
        return NULL;
    }
    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    fclose(file);
    return buffer;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("FORGE Error: Faltan argumentos. Prueba 'forge help'.\n");
        return 1;
    }

    const char *command = argv[1];

    if (strcmp(command, "version") == 0) {
        printf("FORGE v%s (ARM64 Native Target)\n", FORGE_VERSION);
    } 
    else if (strcmp(command, "help") == 0 || strcmp(command, "--help") == 0) {
        print_help();
    } 
    else if (strcmp(command, "build") == 0) {
        if (argc < 3) {
            printf("FORGE Error: Falta especificar el archivo fuente (.kry).\n");
            return 1;
        }

        const char *filepath = argv[2];
        const char *output_name = "programa"; // Nombre por defecto del binario
        if (argc >= 5 && strcmp(argv[3], "-o") == 0) {
            output_name = argv[4];
        }

        printf("[FORGE PIPELINE] Compilando '%s'...\n", filepath);

        char *source = read_file(filepath);
        if (!source) return 1;

        raster_init(source);
        frame_init();
        ASTNode *root = frame_parse();

        if (root) {
            // ANVIL genera el código ejecutable final
            anvil_generate_code(root, output_name);
            free_ast(root);
        } else {
            printf("FORGE Error: Fallo en el análisis sintáctico (AST).\n");
        }

        free(source);
    }
    else if (strcmp(command, "run") == 0) {
        if (argc < 3) {
            printf("FORGE Error: Falta especificar el archivo fuente (.kry).\n");
            return 1;
        }
        
        const char *filepath = argv[2];
        char *source = read_file(filepath);
        if (!source) return 1;

        raster_init(source);
        frame_init();
        ASTNode *root = frame_parse();

        if (root) {
            printf("\n--- ÁRBOL DE SINTAXIS ABSTRACTA (AST) ---\n");
            print_ast(root, 0);
            printf("-----------------------------------------\n");
            free_ast(root);
        }
        free(source);
    } 
    else {
        printf("FORGE Error: Comando desconocido '%s'. Prueba 'forge help'.\n", command);
        return 1;
    }

    return 0;
}
