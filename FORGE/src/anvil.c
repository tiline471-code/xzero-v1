#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "frame.h"
#include "anvil.h"

static void emit_expression(FILE *out, ASTNode *node) {
    if (!node) return;
    if (node->type == NODE_LITERAL) {
        if (node->string_value) {
            fprintf(out, "%s", node->string_value);
        }
    } else if (node->type == NODE_IDENTIFIER) {
        if (node->name) {
            fprintf(out, "%s", node->name);
        }
    } else if (node->type == NODE_BINARY_EXPR) {
        fprintf(out, "(");
        if (node->child_count > 0) emit_expression(out, node->children[0]);
        if (node->name) fprintf(out, " %s ", node->name);
        if (node->child_count > 1) emit_expression(out, node->children[1]);
        fprintf(out, ")");
    }
}

static void emit_node(FILE *out, ASTNode *node) {
    if (!node) return;

    if (node->type == NODE_PROGRAM) {
        fprintf(out, "// Generado automáticamente por FORGE (ANVIL Engine)\n");
        fprintf(out, "#include <stdio.h>\n\n");

        for (int i = 0; i < node->child_count; i++) {
            emit_node(out, node->children[i]);
        }
    } 
    else if (node->type == NODE_FUNCTION) {
        if (node->name && strcmp(node->name, "start") == 0) {
            fprintf(out, "int main(void) {\n");
        } else {
            fprintf(out, "void %s(void) {\n", node->name ? node->name : "func");
        }

        for (int i = 0; i < node->child_count; i++) {
            emit_node(out, node->children[i]);
        }

        if (node->name && strcmp(node->name, "start") == 0) {
            fprintf(out, "    return 0;\n");
        }
        fprintf(out, "}\n\n");
    } 
    else if (node->type == NODE_VAR_DECL) {
        if (node->name && node->child_count > 0) {
            fprintf(out, "    int %s = ", node->name);
            emit_expression(out, node->children[0]);
            fprintf(out, ";\n");
        }
    }
    else if (node->type == NODE_OUTPUT) {
        if (node->child_count > 0) {
            // Verificamos si la expresión es un string literal o número/variable numérica
            // Por simplicidad inicial para output, soportamos tanto cadenas como números/enteros con %s o %d según corresponda, o un formateador genérico.
            // Hagámoslo robusto detectando el tipo de expresión.
            ASTNode *expr = node->children[0];
            if (expr->type == NODE_LITERAL && expr->string_value && expr->string_value[0] == '"') {
                fprintf(out, "    printf(\"%%s\\n\", ");
                emit_expression(out, expr);
                fprintf(out, ");\n");
            } else {
                fprintf(out, "    printf(\"%%d\\n\", ");
                emit_expression(out, expr);
                fprintf(out, ");\n");
            }
        }
    }
}

void anvil_generate_code(ASTNode *root, const char *output_name) {
    if (!root) return;

    const char *temp_c_file = "_forge_temp.c";
    FILE *out = fopen(temp_c_file, "w");
    if (!out) {
        printf("ANVIL Error: No se pudo crear el archivo temporal de compilación.\n");
        return;
    }

    emit_node(out, root);
    fclose(out);

    char compile_command[512];
    snprintf(compile_command, sizeof(compile_command), "clang %s -o %s", temp_c_file, output_name);
    
    printf("[ANVIL] Compilando binario nativo con Clang...\n");
    int result = system(compile_command);

    if (result == 0) {
        printf("¡Éxito! Binario generado correctamente: ./%s\n", output_name);
    } else {
        printf("ANVIL Error: Falló la compilación del código traducido.\n");
    }

    remove(temp_c_file);
}
