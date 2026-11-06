#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raster.h"

#define FORGE_VERSION "0.2.0-alpha"

void print_help() {
    printf("FORGE v%s - Compilador oficial del lenguaje Kryon (Proyecto XZERO)\n\n", FORGE_VERSION);
    printf("Uso:\n  forge <comando> [archivo.kry]\n\n");
    printf("Comandos disponibles:\n");
    printf("  version    Muestra la versión actual de FORGE\n");
    printf("  build      Compila un archivo .kry\n");
    printf("  run        Ejecuta un archivo .kry (Muestra tokens con RASTER)\n");
    printf("  help       Muestra esta ayuda\n");
}

// Función auxiliar para leer un archivo completo a memoria
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
    else if (strcmp(command, "build") == 0 || strcmp(command, "run") == 0) {
        if (argc < 3) {
            printf("FORGE Error: Falta especificar el archivo fuente (.kry).\n");
            return 1;
        }
        
        const char *filepath = argv[2];
        printf("[FORGE RASTER] Analizando archivo fuente: %s\n", filepath);

        char *source = read_file(filepath);
        if (!source) return 1;

        // Inicializar el analizador léxico RASTER
        raster_init(source);

        printf("\n--- SECUENCIA DE TOKENS DETECTADOS ---\n");
        Token token;
        do {
            token = raster_next_token();
            const char *type_name = "UNKNOWN";
            switch (token.type) {
                case TOKEN_PROGRAM: type_name = "PROGRAM"; break;
                case TOKEN_FN: type_name = "FN"; break;
                case TOKEN_OUTPUT: type_name = "OUTPUT"; break;
                case TOKEN_IDENTIFIER: type_name = "IDENTIFIER"; break;
                case TOKEN_STRING: type_name = "STRING_LITERAL"; break;
                case TOKEN_NUMBER: type_name = "NUMBER"; break;
                case TOKEN_LBRACE: type_name = "LBRACE '{'"; break;
                case TOKEN_RBRACE: type_name = "RBRACE '}'"; break;
                case TOKEN_LPAREN: type_name = "LPAREN '('"; break;
                case TOKEN_RPAREN: type_name = "RPAREN ')'"; break;
                case TOKEN_SEMICOLON: type_name = "SEMICOLON ';'"; break;
                case TOKEN_ERROR: type_name = "ERROR"; break;
                case TOKEN_EOF: type_name = "EOF"; break;
            }

            if (token.type != TOKEN_EOF) {
                printf("Línea %d:%d -> [%s] : \"%s\"\n", token.line, token.column, type_name, token.lexeme);
            }

            // Liberar memoria del lexema del token
            free(token.lexeme);
        } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);

        printf("--------------------------------------\n");
        free(source);
    } 
    else {
        printf("FORGE Error: Comando desconocido '%s'. Prueba 'forge help'.\n", command);
        return 1;
    }

    return 0;
}

