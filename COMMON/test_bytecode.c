#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "bytecode.h"

static void test_serialization_roundtrip() {
    const char *test_file = "test_output.kyb";

    ConstantPool pool_out;
    pool_init(&pool_out);

    Value v1;
    v1.type = VAL_INT;
    v1.as.as_int = 42;
    pool_write(&pool_out, v1);

    Value v2;
    v2.type = VAL_STRING;
    v2.as.as_string = strdup("k-engine");
    pool_write(&pool_out, v2);

    BytecodeChunk chunk_out;
    chunk_init(&chunk_out);
    chunk_write(&chunk_out, OP_LOAD_CONST);
    chunk_write(&chunk_out, 0x00);
    chunk_write(&chunk_out, 0x01);
    chunk_write(&chunk_out, OP_PRINT);
    chunk_write(&chunk_out, OP_HALT);

    int serialize_result = bytecode_serialize(test_file, &pool_out, &chunk_out);
    assert(serialize_result == 1);

    ConstantPool pool_in;
    BytecodeChunk chunk_in;
    int deserialize_result = bytecode_deserialize(test_file, &pool_in, &chunk_in);
    assert(deserialize_result == 1);

    assert(pool_in.count == 2);
    assert(pool_in.values[0].type == VAL_INT);
    assert(pool_in.values[0].as.as_int == 42);

    assert(pool_in.values[1].type == VAL_STRING);
    assert(strcmp(pool_in.values[1].as.as_string, "k-engine") == 0);

    assert(chunk_in.count == 5);
    assert(chunk_in.code[0] == OP_LOAD_CONST);
    assert(chunk_in.code[1] == 0x00);
    assert(chunk_in.code[2] == 0x01);
    assert(chunk_in.code[3] == OP_PRINT);
    assert(chunk_in.code[4] == OP_HALT);

    pool_free(&pool_out);
    chunk_free(&chunk_out);
    pool_free(&pool_in);
    chunk_free(&chunk_in);

    remove(test_file);
    printf("TEST_SERIALIZATION_ROUNDTRIP: OK\n");
}

static void test_invalid_files() {
    ConstantPool pool;
    BytecodeChunk chunk;

    int res1 = bytecode_deserialize("non_existent_file.kyb", &pool, &chunk);
    assert(res1 == 0);

    const char *bad_magic_file = "bad_magic.kyb";
    FILE *f = fopen(bad_magic_file, "wb");
    assert(f != NULL);
    uint8_t bad_magic[4] = { 'B', 'A', 'D', '!' };
    fwrite(bad_magic, 1, 4, f);
    uint8_t version = 0x01;
    fwrite(&version, 1, 1, f);
    fclose(f);

    int res2 = bytecode_deserialize(bad_magic_file, &pool, &chunk);
    assert(res2 == 0);
    remove(bad_magic_file);

    printf("TEST_INVALID_FILES: OK\n");
}

int main(void) {
    test_serialization_roundtrip();
    test_invalid_files();
    printf("TODOS LOS TESTS DE COMMON PASARON CORRECTAMENTE.\n");
    return 0;
}
