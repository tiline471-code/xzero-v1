#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FORGE_VERSION "0.2.0-alpha"

void print_help() {
    printf("FORGE v%s - Compilador oficial del lenguaje Kryon (Proyecto XZERO)\n\n", FORGE_VERSION);
    printf("Uso:\n  forge <comando> [archivo.kry]\n\n");
    printf("Comandos:\n");
    printf("  version    Muestra la versión de FORGE\n");
    printf("  build      Compila un archivo .kry\n");
    printf("  run        Ejecuta un archivo .kry\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("FORGE Error: Faltan argumentos. Prueba 'forge help'.\n");
        return 1;
    }
    if (strcmp(argv[1], "version") == 0) {
        printf("FORGE v%s (ARM64 Native Target)\n", FORGE_VERSION);
    } else {
        printf("[FORGE] Procesando comando para: %s\n", argv[2] ? argv[2] : "desconocido");
    }
    return 0;
}
