#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kvm.h"
#include "stack.h"

typedef struct {
    Value variables[256];
    int initialized[256];
} Memory;

typedef struct {
    int return_ip;
} CallFrame;

int kvm_run_file(const char *filename) {
    ConstantPool pool;
    BytecodeChunk chunk;

    if (!bytecode_deserialize(filename, &pool, &chunk)) {
        fprintf(stderr, "KVM Error: No se pudo cargar o deserializar el archivo binario '%s'.\n", filename);
        return 0;
    }

    Stack stack;
    stack_init(&stack);

    CallFrame call_stack[256];
    int call_top = 0;

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
                    fprintf(stderr, "KVM Runtime Error [IP %d]: Bytecode truncado en OP_LOAD_CONST.\n", ip - 1);
                    running = 0;
                    break;
                }
                uint16_t const_idx = chunk.code[ip] | (chunk.code[ip + 1] << 8);
                ip += 2;

                if (const_idx >= (uint16_t)pool.count) {
                    fprintf(stderr, "KVM Runtime Error [IP %d]: Índice de constante fuera de límites (%d).\n", ip - 3, const_idx);
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
                    fprintf(stderr, "KVM Runtime Error [IP %d]: Bytecode truncado en OP_LOAD_VAR.\n", ip - 1);
                    running = 0;
                    break;
                }
                uint8_t var_idx = chunk.code[ip++];
                if (!mem.initialized[var_idx]) {
                    fprintf(stderr, "KVM Runtime Error [IP %d]: Variable no inicializada en índice %d.\n", ip - 2, var_idx);
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
                    fprintf(stderr, "KVM Runtime Error [IP %d]: Bytecode truncado en OP_STORE_VAR.\n", ip - 1);
                    running = 0;
                    break;
                }
                uint8_t var_idx = chunk.code[ip++];
                if (stack.top < 0) {
                    fprintf(stderr, "KVM Runtime Error [IP %d]: Stack underflow en OP_STORE_VAR.\n", ip - 2);
                    running = 0;
                    break;
                }
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
                if (stack.top < 1) {
                    fprintf(stderr, "KVM Runtime Error [IP %d]: Stack underflow en operación aritmética.\n", ip - 1);
                    running = 0;
                    break;
                }
                Value right = stack_pop(&stack);
                Value left = stack_pop(&stack);

                if (left.type != VAL_INT || right.type != VAL_INT) {
                    fprintf(stderr, "KVM Runtime Error [IP %d]: Tipos incompatibles para operación aritmética.\n", ip - 1);
                    running = 0;
                    break;
                }

                int res = 0;
                if (opcode == OP_ADD) res = left.as.as_int + right.as.as_int;
                else if (opcode == OP_SUB) res = left.as.as_int - right.as.as_int;
                else if (opcode == OP_MUL) res = left.as.as_int * right.as.as_int;
                else if (opcode == OP_DIV) {
                    if (right.as.as_int == 0) {
                        fprintf(stderr, "KVM Runtime Error [IP %d]: División entre cero.\n", ip - 1);
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

            case OP_LESS:
            case OP_EQUAL: {
                if (stack.top < 1) {
                    fprintf(stderr, "KVM Runtime Error [IP %d]: Stack underflow en comparación.\n", ip - 1);
                    running = 0;
                    break;
                }
                Value right = stack_pop(&stack);
                Value left = stack_pop(&stack);

                int res = 0;
                if (opcode == OP_LESS) {
                    res = (left.as.as_int < right.as.as_int) ? 1 : 0;
                } else if (opcode == OP_EQUAL) {
                    res = (left.as.as_int == right.as.as_int) ? 1 : 0;
                }

                Value out_val;
                out_val.type = VAL_INT;
                out_val.as.as_int = res;
                stack_push(&stack, out_val);
                break;
            }

            case OP_JUMP: {
                if (ip + 1 >= chunk.count) {
                    fprintf(stderr, "KVM Runtime Error [IP %d]: Bytecode truncado en OP_JUMP.\n", ip - 1);
                    running = 0;
                    break;
                }
                uint16_t target = chunk.code[ip] | (chunk.code[ip + 1] << 8);
                ip = target;
                break;
            }

            case OP_JUMP_IF_FALSE: {
                if (ip + 1 >= chunk.count) {
                    fprintf(stderr, "KVM Runtime Error [IP %d]: Bytecode truncado en OP_JUMP_IF_FALSE.\n", ip - 1);
                    running = 0;
                    break;
                }
                uint16_t target = chunk.code[ip] | (chunk.code[ip + 1] << 8);
                ip += 2;

                Value cond = stack_pop(&stack);
                if (cond.as.as_int == 0) {
                    ip = target;
                }
                break;
            }

            case OP_CALL: {
                if (ip + 1 >= chunk.count) {
                    fprintf(stderr, "KVM Runtime Error [IP %d]: Bytecode truncado en OP_CALL.\n", ip - 1);
                    running = 0;
                    break;
                }
                uint16_t target = chunk.code[ip] | (chunk.code[ip + 1] << 8);
                ip += 2;

                if (call_top >= 256) {
                    fprintf(stderr, "KVM Runtime Error: Stack overflow en llamadas a función.\n");
                    running = 0;
                    break;
                }
                call_stack[call_top++].return_ip = ip;
                ip = target;
                break;
            }

            case OP_NATIVE_CALL: {
                if (ip + 1 >= chunk.count) {
                    fprintf(stderr, "KVM Runtime Error [IP %d]: Bytecode truncado en OP_NATIVE_CALL.\n", ip - 1);
                    running = 0;
                    break;
                }
                uint8_t native_id = chunk.code[ip++];
                uint8_t arg_count = chunk.code[ip++];

                if (native_id == 0) { // sys_exec(cmd)
                    if (arg_count != 1 || stack.top < 0) {
                        fprintf(stderr, "KVM Runtime Error: sys_exec espera exactamente 1 argumento string.\n");
                        running = 0;
                        break;
                    }
                    Value arg = stack_pop(&stack);
                    int exit_code = -1;
                    if (arg.type == VAL_STRING) {
                        exit_code = system(arg.as.as_string);
                        free(arg.as.as_string);
                    }
                    Value res;
                    res.type = VAL_INT;
                    res.as.as_int = exit_code;
                    stack_push(&stack, res);
                }
                break;
            }

            case OP_RETURN: {
                if (call_top <= 0) {
                    fprintf(stderr, "KVM Runtime Error: Retorno sin marco de función activo.\n");
                    running = 0;
                    break;
                }
                Value ret_val = stack_pop(&stack);
                ip = call_stack[--call_top].return_ip;
                stack_push(&stack, ret_val);
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
                fprintf(stderr, "KVM Runtime Error [IP %d]: Opcode desconocido (0x%02X).\n", ip - 1, opcode);
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
