#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "anvil.h"

typedef struct {
    char *name;
    int address;
    int param_count;
} FunctionSymbol;

typedef struct {
    char *names[256];
    int count;
} SymbolTable;

typedef struct {
    FunctionSymbol fns[64];
    int count;
} FunctionTable;

static SymbolTable sym_table;
static FunctionTable fn_table;

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

static int function_lookup(const char *name) {
    for (int i = 0; i < fn_table.count; i++) {
        if (strcmp(fn_table.fns[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static int function_register(const char *name, int address, int param_count) {
    int idx = function_lookup(name);
    if (idx != -1) {
        fn_table.fns[idx].address = address;
        fn_table.fns[idx].param_count = param_count;
        return idx;
    }
    if (fn_table.count < 64) {
        fn_table.fns[fn_table.count].name = strdup(name);
        fn_table.fns[fn_table.count].address = address;
        fn_table.fns[fn_table.count].param_count = param_count;
        return fn_table.count++;
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

static int add_constant_string(ConstantPool *pool, const char *val) {
    for (int i = 0; i < pool->count; i++) {
        if (pool->values[i].type == VAL_STRING && strcmp(pool->values[i].as.as_string, val) == 0) {
            return i;
        }
    }
    Value v;
    v.type = VAL_STRING;
    v.as.as_string = strdup(val);
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

        case NODE_BLOCK:
            for (int i = 0; i < node->data.block.count; i++) {
                compile_node(node->data.block.statements[i], pool, chunk);
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
            } else if (strcmp(node->data.binary.op, "<") == 0) {
                chunk_write(chunk, OP_LESS);
            } else if (strcmp(node->data.binary.op, "==") == 0) {
                chunk_write(chunk, OP_EQUAL);
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

        case NODE_STRING_LITERAL:
            {
                int const_idx = add_constant_string(pool, node->data.string_literal.value);
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

        case NODE_CALL: {
            for (int i = 0; i < node->data.call.arg_count; i++) {
                compile_node(node->data.call.args[i], pool, chunk);
            }

            // Comprobar si es una función nativa del sistema
            if (strcmp(node->data.call.name, "sys_exec") == 0) {
                chunk_write(chunk, OP_NATIVE_CALL);
                chunk_write(chunk, 0); // ID 0: sys_exec
                chunk_write(chunk, (uint8_t)node->data.call.arg_count);
                break;
            }

            int fn_idx = function_lookup(node->data.call.name);
            if (fn_idx == -1) {
                fn_idx = function_register(node->data.call.name, 0, node->data.call.arg_count);
            }
            chunk_write(chunk, OP_CALL);
            int fn_addr = fn_table.fns[fn_idx].address;
            chunk_write(chunk, (uint8_t)(fn_addr & 0xFF));
            chunk_write(chunk, (uint8_t)((fn_addr >> 8) & 0xFF));
            break;
        }

        case NODE_FN_DECL: {
            chunk_write(chunk, OP_JUMP);
            int jump_over_pos = chunk->count;
            chunk_write(chunk, 0xFF);
            chunk_write(chunk, 0xFF);

            int fn_address = chunk->count;
            function_register(node->data.fn_decl.name, fn_address, node->data.fn_decl.param_count);

            for (int i = 0; i < node->data.fn_decl.param_count; i++) {
                int var_idx = symbol_lookup(node->data.fn_decl.params[i]);
                chunk_write(chunk, OP_STORE_VAR);
                chunk_write(chunk, (uint8_t)var_idx);
            }

            compile_node(node->data.fn_decl.body, pool, chunk);

            int zero_idx = add_constant_int(pool, 0);
            chunk_write(chunk, OP_LOAD_CONST);
            chunk_write(chunk, (uint8_t)(zero_idx & 0xFF));
            chunk_write(chunk, (uint8_t)((zero_idx >> 8) & 0xFF));
            chunk_write(chunk, OP_RETURN);

            int end_address = chunk->count;
            chunk->code[jump_over_pos] = (end_address & 0xFF);
            chunk->code[jump_over_pos + 1] = ((end_address >> 8) & 0xFF);
            break;
        }

        case NODE_RETURN:
            if (node->data.return_stmt.expr) {
                compile_node(node->data.return_stmt.expr, pool, chunk);
            } else {
                int zero_idx = add_constant_int(pool, 0);
                chunk_write(chunk, OP_LOAD_CONST);
                chunk_write(chunk, (uint8_t)(zero_idx & 0xFF));
                chunk_write(chunk, (uint8_t)((zero_idx >> 8) & 0xFF));
            }
            chunk_write(chunk, OP_RETURN);
            break;

        case NODE_IF:
            compile_node(node->data.if_stmt.condition, pool, chunk);
            
            chunk_write(chunk, OP_JUMP_IF_FALSE);
            int jump_false_pos = chunk->count;
            chunk_write(chunk, 0xFF);
            chunk_write(chunk, 0xFF);

            compile_node(node->data.if_stmt.then_branch, pool, chunk);

            if (node->data.if_stmt.else_branch) {
                chunk_write(chunk, OP_JUMP);
                int jump_end_pos = chunk->count;
                chunk_write(chunk, 0xFF);
                chunk_write(chunk, 0xFF);

                int else_start = chunk->count;
                chunk->code[jump_false_pos] = (else_start & 0xFF);
                chunk->code[jump_false_pos + 1] = ((else_start >> 8) & 0xFF);

                compile_node(node->data.if_stmt.else_branch, pool, chunk);

                int end_pos = chunk->count;
                chunk->code[jump_end_pos] = (end_pos & 0xFF);
                chunk->code[jump_end_pos + 1] = ((end_pos >> 8) & 0xFF);
            } else {
                int end_pos = chunk->count;
                chunk->code[jump_false_pos] = (end_pos & 0xFF);
                chunk->code[jump_false_pos + 1] = ((end_pos >> 8) & 0xFF);
            }
            break;

        case NODE_WHILE: {
            int loop_start = chunk->count;
            compile_node(node->data.while_stmt.condition, pool, chunk);

            chunk_write(chunk, OP_JUMP_IF_FALSE);
            int jump_exit_pos = chunk->count;
            chunk_write(chunk, 0xFF);
            chunk_write(chunk, 0xFF);

            compile_node(node->data.while_stmt.body, pool, chunk);

            chunk_write(chunk, OP_JUMP);
            chunk_write(chunk, (loop_start & 0xFF));
            chunk_write(chunk, ((loop_start >> 8) & 0xFF));

            int loop_end = chunk->count;
            chunk->code[jump_exit_pos] = (loop_end & 0xFF);
            chunk->code[jump_exit_pos + 1] = ((loop_end >> 8) & 0xFF);
            break;
        }

        default:
            break;
    }
}

int anvil_compile(ASTNode *root, const char *output_filename) {
    sym_table.count = 0;
    fn_table.count = 0;

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
    for (int i = 0; i < fn_table.count; i++) {
        free(fn_table.fns[i].name);
    }

    return result;
}

int anvil_generate_code(ASTNode *root, const char *output_filename) {
    return anvil_compile(root, output_filename);
}
