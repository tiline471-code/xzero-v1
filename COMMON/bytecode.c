#include "bytecode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void pool_init(ConstantPool *pool) {
    pool->values = NULL;
    pool->count = 0;
    pool->capacity = 0;
}

void pool_write(ConstantPool *pool, Value val) {
    if (pool->count + 1 >= pool->capacity) {
        pool->capacity = pool->capacity < 8 ? 8 : pool->capacity * 2;
        pool->values = realloc(pool->values, sizeof(Value) * pool->capacity);
    }
    pool->values[pool->count++] = val;
}

void pool_free(ConstantPool *pool) {
    for (int i = 0; i < pool->count; i++) {
        if (pool->values[i].type == VAL_STRING) {
            free(pool->values[i].as.as_string);
        }
    }
    free(pool->values);
    pool_init(pool);
}

void chunk_init(BytecodeChunk *chunk) {
    chunk->code = NULL;
    chunk->count = 0;
    chunk->capacity = 0;
}

void chunk_write(BytecodeChunk *chunk, uint8_t byte) {
    if (chunk->count + 1 >= chunk->capacity) {
        chunk->capacity = chunk->capacity < 8 ? 8 : chunk->capacity * 2;
        chunk->code = realloc(chunk->code, sizeof(uint8_t) * chunk->capacity);
    }
    chunk->code[chunk->count++] = byte;
}

void chunk_free(BytecodeChunk *chunk) {
    free(chunk->code);
    chunk_init(chunk);
}

int bytecode_serialize(const char *filename, ConstantPool *pool, BytecodeChunk *chunk) {
    FILE *f = fopen(filename, "wb");
    if (!f) return 0;

    // Magic bytes for Kryon Binary (.kyb)
    fwrite("KYB1", 1, 4, f);

    // Constant Pool
    fwrite(&pool->count, sizeof(int), 1, f);
    for (int i = 0; i < pool->count; i++) {
        ValueType type = pool->values[i].type;
        fwrite(&type, sizeof(ValueType), 1, f);
        if (type == VAL_INT) {
            fwrite(&pool->values[i].as.as_int, sizeof(int), 1, f);
        } else if (type == VAL_STRING) {
            int len = strlen(pool->values[i].as.as_string);
            fwrite(&len, sizeof(int), 1, f);
            fwrite(pool->values[i].as.as_string, 1, len, f);
        }
    }

    // Bytecode Chunk
    fwrite(&chunk->count, sizeof(int), 1, f);
    fwrite(chunk->code, 1, chunk->count, f);

    fclose(f);
    return 1;
}

int bytecode_deserialize(const char *filename, ConstantPool *pool, BytecodeChunk *chunk) {
    FILE *f = fopen(filename, "rb");
    if (!f) return 0;

    char magic[4];
    if (fread(magic, 1, 4, f) != 4 || memcmp(magic, "KYB1", 4) != 0) {
        fclose(f);
        return 0;
    }

    pool_init(pool);
    int pool_count = 0;
    if (fread(&pool_count, sizeof(int), 1, f) != 1) {
        fclose(f);
        return 0;
    }

    for (int i = 0; i < pool_count; i++) {
        ValueType type;
        if (fread(&type, sizeof(ValueType), 1, f) != 1) {
            fclose(f);
            return 0;
        }
        Value val;
        val.type = type;
        if (type == VAL_INT) {
            if (fread(&val.as.as_int, sizeof(int), 1, f) != 1) {
                fclose(f);
                return 0;
            }
        } else if (type == VAL_STRING) {
            int len = 0;
            if (fread(&len, sizeof(int), 1, f) != 1) {
                fclose(f);
                return 0;
            }
            char *str = malloc(len + 1);
            if (fread(str, 1, len, f) != (size_t)len) {
                free(str);
                fclose(f);
                return 0;
            }
            str[len] = '\0';
            val.as.as_string = str;
        }
        pool_write(pool, val);
    }

    chunk_init(chunk);
    int chunk_count = 0;
    if (fread(&chunk_count, sizeof(int), 1, f) != 1) {
        fclose(f);
        return 0;
    }

    for (int i = 0; i < chunk_count; i++) {
        uint8_t byte;
        if (fread(&byte, 1, 1, f) != 1) {
            fclose(f);
            return 0;
        }
        chunk_write(chunk, byte);
    }

    fclose(f);
    return 1;
}
