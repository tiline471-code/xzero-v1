#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "anvil.h"

typedef struct {
    char *names[256];
    int count;
} SymbolTable;

static SymbolTable sym_table;

static int symbol_lookup(const char *name) {
    for (int i = 0; i < sym_table.count; i++) {
        if (strcmp(sym_table.names[i], name) == 0) {
            return i;
        }
    }
    if (sym_table.count < 256) {
        sym_table.names[sym_table.count] = strdup(name);
        return sym_table.count++;
    }
    return 0;
}

static int add_constant_int(ConstantPool *pool, int val) {
    for (int i = 0; i < pool->count; i++) {
        if (pool->values[i].type == VAL_INT && pool->values[i].as.as_int == val) {
            return i;
        }
    }
    Value v;
    v.type = VAL_INT;
    v.as.as_int = val;
    pool_write(pool, v);
    return pool->count - 1;
}

static void compile_node(ASTNode *node, ConstantPool *pool, BytecodeChunk *chunk) {
    if (!node) return;

    switch (node->type) {
        case NODE_PROGRAM:
            for (int i = 0; i < node->data.program.count; i++) {
                compile_node(node->data.program.statements[i], pool, chunk);
            }
            break;

        case NODE_EXPR_STMT:
            compile_node(node->data.expr_stmt.expr, pool, chunk);
            break;

        case NODE_PRINT:
            compile_node(node->data.print.expr, pool, chunk);
            chunk_write(chunk, OP_PRINT);
            break;

        case NODE_VAR_DECL:
            compile_node(node->data.var_decl.value, pool, chunk);
            {
                int var_idx = symbol_lookup(node->data.var_decl.name);
                chunk_write(chunk, OP_STORE_VAR);
                chunk_write(chunk, (uint8_t)var_idx);
            }
            break;

        case NODE_ASSIGN:
            compile_node(node->data.assign.value, pool, chunk);
            {
                int var_idx = symbol_lookup(node->data.assign.name);
                chunk_write(chunk, OP_STORE_VAR);
                chunk_write(chunk, (uint8_t)var_idx);
            }
            break;

        case NODE_BINARY:
            compile_node(node->data.binary.left, pool, chunk);
            compile_node(node->data.binary.right, pool, chunk);
            if (strcmp(node->data.binary.op, "+") == 0) {
                chunk_write(chunk, OP_ADD);
            } else if (strcmp(node->data.binary.op, "-") == 0) {
                chunk_write(chunk, OP_SUB);
            } else if (strcmp(node->data.binary.op, "*") == 0) {
                chunk_write(chunk, OP_MUL);
            } else if (strcmp(node->data.binary.op, "/") == 0) {
                chunk_write(chunk, OP_DIV);
            }
            break;

        case NODE_LITERAL:
            {
                int const_idx = add_constant_int(pool, node->data.literal.value);
                chunk_write(chunk, OP_LOAD_CONST);
                chunk_write(chunk, (uint8_t)(const_idx & 0xFF));
                chunk_write(chunk, (uint8_t)((const_idx >> 8) & 0xFF));
            }
            break;

        case NODE_VARIABLE:
            {
                int var_idx = symbol_lookup(node->data.variable.name);
                chunk_write(chunk, OP_LOAD_VAR);
                chunk_write(chunk, (uint8_t)var_idx);
            }
            break;

        default:
            break;
    }
}

int anvil_compile(ASTNode *root, const char *output_filename) {
    sym_table.count = 0;

    ConstantPool pool;
    pool_init(&pool);

    BytecodeChunk chunk;
    chunk_init(&chunk);

    compile_node(root, &pool, &chunk);
    chunk_write(&chunk, OP_HALT);

    int result = bytecode_serialize(output_filename, &pool, &chunk);

    pool_free(&pool);
    chunk_free(&chunk);

    for (int i = 0; i < sym_table.count; i++) {
        free(sym_table.names[i]);
    }

    return result;
}

int anvil_generate_code(ASTNode *root, const char *output_filename) {
    return anvil_compile(root, output_filename);
}
