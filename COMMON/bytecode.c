#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bytecode.h"

void chunk_init(BytecodeChunk *chunk) {
    chunk->code = NULL;
    chunk->count = 0;
    chunk->capacity = 0;
}

void chunk_write(BytecodeChunk *chunk, uint8_t byte) {
    if (chunk->capacity < chunk->count + 1) {
        int old_capacity = chunk->capacity;
        chunk->capacity = old_capacity < 8 ? 8 : old_capacity * 2;
        chunk->code = realloc(chunk->code, chunk->capacity);
    }
    chunk->code[chunk->count++] = byte;
}

void chunk_free(BytecodeChunk *chunk) {
    free(chunk->code);
    chunk_init(chunk);
}

void pool_init(ConstantPool *pool) {
    pool->values = NULL;
    pool->count = 0;
    pool->capacity = 0;
}

void pool_write(ConstantPool *pool, Value val) {
    if (pool->capacity < pool->count + 1) {
        int old_capacity = pool->capacity;
        pool->capacity = old_capacity < 8 ? 8 : old_capacity * 2;
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

int bytecode_serialize(const char *filename, const ConstantPool *pool, const BytecodeChunk *chunk) {
    FILE *f = fopen(filename, "wb");
    if (!f) return 0;

    uint8_t magic[4] = { 'K', 'R', 'Y', 'N' };
    fwrite(magic, 1, 4, f);

    uint8_t version = 0x01;
    fwrite(&version, 1, 1, f);

    uint16_t pool_count = (uint16_t)pool->count;
    fwrite(&pool_count, sizeof(uint16_t), 1, f);

    for (int i = 0; i < pool->count; i++) {
        uint8_t type = (uint8_t)pool->values[i].type;
        fwrite(&type, 1, 1, f);
        if (type == VAL_INT) {
            int32_t val = (int32_t)pool->values[i].as.as_int;
            fwrite(&val, sizeof(int32_t), 1, f);
        } else if (type == VAL_STRING) {
            uint16_t len = (uint16_t)strlen(pool->values[i].as.as_string);
            fwrite(&len, sizeof(uint16_t), 1, f);
            fwrite(pool->values[i].as.as_string, 1, len, f);
        }
    }

    uint32_t code_size = (uint32_t)chunk->count;
    fwrite(&code_size, sizeof(uint32_t), 1, f);
    fwrite(chunk->code, 1, chunk->count, f);

    fclose(f);
    return 1;
}

int bytecode_deserialize(const char *filename, ConstantPool *pool, BytecodeChunk *chunk) {
    FILE *f = fopen(filename, "rb");
    if (!f) return 0;

    uint8_t magic[4];
    if (fread(magic, 1, 4, f) != 4 || memcmp(magic, "KRYN", 4) != 0) {
        fclose(f);
        return 0;
    }

    uint8_t version;
    if (fread(&version, 1, 1, f) != 1 || version != 0x01) {
        fclose(f);
        return 0;
    }

    uint16_t pool_count;
    if (fread(&pool_count, sizeof(uint16_t), 1, f) != 1) {
        fclose(f);
        return 0;
    }

    pool_init(pool);
    for (int i = 0; i < pool_count; i++) {
        uint8_t type;
        if (fread(&type, 1, 1, f) != 1) { pool_free(pool); fclose(f); return 0; }
        Value val;
        val.type = (ConstantType)type;
        if (type == VAL_INT) {
            int32_t num;
            if (fread(&num, sizeof(int32_t), 1, f) != 1) { pool_free(pool); fclose(f); return 0; }
            val.as.as_int = (int)num;
        } else if (type == VAL_STRING) {
            uint16_t len;
            if (fread(&len, sizeof(uint16_t), 1, f) != 1) { pool_free(pool); fclose(f); return 0; }
            char *str = malloc(len + 1);
            if (fread(str, 1, len, f) != len) { free(str); pool_free(pool); fclose(f); return 0; }
            str[len] = '\0';
            val.as.as_string = str;
        } else {
            pool_free(pool);
            fclose(f);
            return 0;
        }
        pool_write(pool, val);
    }

    uint32_t code_size;
    if (fread(&code_size, sizeof(uint32_t), 1, f) != 1) {
        pool_free(pool);
        fclose(f);
        return 0;
    }

    chunk_init(chunk);
    for (uint32_t i = 0; i < code_size; i++) {
        uint8_t byte;
        if (fread(&byte, 1, 1, f) != 1) {
            chunk_free(chunk);
            pool_free(pool);
            fclose(f);
            return 0;
        }
        chunk_write(chunk, byte);
    }

    fclose(f);
    return 1;
}
