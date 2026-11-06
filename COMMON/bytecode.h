#ifndef BYTECODE_H
#define BYTECODE_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    VAL_INT,
    VAL_STRING
} ValueType;

typedef struct {
    ValueType type;
    union {
        int as_int;
        char *as_string;
    } as;
} Value;

typedef struct {
    Value *values;
    int count;
    int capacity;
} ConstantPool;

typedef struct {
    uint8_t *code;
    int count;
    int capacity;
} BytecodeChunk;

typedef enum {
    OP_HALT          = 0x00,
    OP_LOAD_CONST    = 0x01,
    OP_LOAD_VAR      = 0x02,
    OP_STORE_VAR     = 0x03,
    OP_ADD           = 0x04,
    OP_SUB           = 0x05,
    OP_MUL           = 0x06,
    OP_DIV           = 0x07,
    OP_PRINT         = 0x08,
    OP_LESS          = 0x09,
    OP_EQUAL         = 0x0A,
    OP_JUMP          = 0x0B,
    OP_JUMP_IF_FALSE = 0x0C,
    OP_CALL          = 0x0D,
    OP_RETURN        = 0x0E,
    OP_CLOSURE       = 0x0F,
    OP_NATIVE_CALL   = 0x10
} OpCode;

void pool_init(ConstantPool *pool);
void pool_write(ConstantPool *pool, Value val);
void pool_free(ConstantPool *pool);

void chunk_init(BytecodeChunk *chunk);
void chunk_write(BytecodeChunk *chunk, uint8_t byte);
void chunk_free(BytecodeChunk *chunk);

int bytecode_serialize(const char *filename, ConstantPool *pool, BytecodeChunk *chunk);
int bytecode_deserialize(const char *filename, ConstantPool *pool, BytecodeChunk *chunk);

#endif
