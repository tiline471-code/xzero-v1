#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kvm.h"
#include "stack.h"

typedef struct {
    Value variables[256];
    int initialized[256];
} Memory;

int kvm_run_file(const char *filename) {
    ConstantPool pool;
    BytecodeChunk chunk;

    if (!bytecode_deserialize(filename, &pool, &chunk)) {
        fprintf(stderr, "KVM Error: No se pudo cargar o deserializar el archivo binario '%s'.\n", filename);
        return 0;
    }

    Stack stack;
    stack_init(&stack);

    Memory mem;
    memset(&mem, 0, sizeof(Memory));

    int ip = 0;
    int running = 1;

    while (running && ip < chunk.count) {
        uint8_t opcode = chunk.code[ip++];

        switch (opcode) {
            case OP_HALT:
                running = 0;
                break;

            case OP_LOAD_CONST: {
                if (ip + 1 >= chunk.count) {
                    fprintf(stderr, "KVM Runtime Error: Bytecode truncado en OP_LOAD_CONST.\n");
                    running = 0;
                    break;
                }
                uint16_t const_idx = chunk.code[ip] | (chunk.code[ip + 1] << 8);
                ip += 2;

                if (const_idx >= pool.count) {
                    fprintf(stderr, "KVM Runtime Error: Índice de constante fuera de límites (%d).\n", const_idx);
                    running = 0;
                    break;
                }

                Value val = pool.values[const_idx];
                Value push_val;
                push_val.type = val.type;
                if (val.type == VAL_INT) {
                    push_val.as.as_int = val.as.as_int;
                } else if (val.type == VAL_STRING) {
                    push_val.as.as_string = strdup(val.as.as_string);
                }
                stack_push(&stack, push_val);
                break;
            }

            case OP_LOAD_VAR: {
                if (ip >= chunk.count) {
                    fprintf(stderr, "KVM Runtime Error: Bytecode truncado en OP_LOAD_VAR.\n");
                    running = 0;
                    break;
                }
                uint8_t var_idx = chunk.code[ip++];
                if (!mem.initialized[var_idx]) {
                    fprintf(stderr, "KVM Runtime Error: Variable no inicializada en índice %d.\n", var_idx);
                    running = 0;
                    break;
                }
                Value val = mem.variables[var_idx];
                Value push_val;
                push_val.type = val.type;
                if (val.type == VAL_INT) {
                    push_val.as.as_int = val.as.as_int;
                } else if (val.type == VAL_STRING) {
                    push_val.as.as_string = strdup(val.as.as_string);
                }
                stack_push(&stack, push_val);
                break;
            }

            case OP_STORE_VAR: {
                if (ip >= chunk.count) {
                    fprintf(stderr, "KVM Runtime Error: Bytecode truncado en OP_STORE_VAR.\n");
                    running = 0;
                    break;
                }
                uint8_t var_idx = chunk.code[ip++];
                Value val = stack_pop(&stack);

                if (mem.initialized[var_idx] && mem.variables[var_idx].type == VAL_STRING) {
                    free(mem.variables[var_idx].as.as_string);
                }

                mem.variables[var_idx] = val;
                mem.initialized[var_idx] = 1;
                break;
            }

            case OP_ADD:
            case OP_SUB:
            case OP_MUL:
            case OP_DIV: {
                Value right = stack_pop(&stack);
                Value left = stack_pop(&stack);

                if (left.type != VAL_INT || right.type != VAL_INT) {
                    fprintf(stderr, "KVM Runtime Error: Tipos incompatibles para operación aritmética.\n");
                    running = 0;
                    break;
                }

                int res = 0;
                if (opcode == OP_ADD) res = left.as.as_int + right.as.as_int;
                else if (opcode == OP_SUB) res = left.as.as_int - right.as.as_int;
                else if (opcode == OP_MUL) res = left.as.as_int * right.as.as_int;
                else if (opcode == OP_DIV) {
                    if (right.as.as_int == 0) {
                        fprintf(stderr, "KVM Runtime Error: División entre cero.\n");
                        running = 0;
                        break;
                    }
                    res = left.as.as_int / right.as.as_int;
                }

                Value out_val;
                out_val.type = VAL_INT;
                out_val.as.as_int = res;
                stack_push(&stack, out_val);
                break;
            }

            case OP_PRINT: {
                Value val = stack_pop(&stack);
                if (val.type == VAL_INT) {
                    printf("%d\n", val.as.as_int);
                } else if (val.type == VAL_STRING) {
                    printf("%s\n", val.as.as_string);
                    free(val.as.as_string);
                }
                break;
            }

            default:
                fprintf(stderr, "KVM Runtime Error: Opcode desconocido (0x%02X).\n", opcode);
                running = 0;
                break;
        }
    }

    for (int i = 0; i < 256; i++) {
        if (mem.initialized[i] && mem.variables[i].type == VAL_STRING) {
            free(mem.variables[i].as.as_string);
        }
    }

    stack_free(&stack);
    pool_free(&pool);
    chunk_free(&chunk);

    return 1;
}
