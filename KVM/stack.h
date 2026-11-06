#ifndef STACK_H
#define STACK_H

#include "../COMMON/bytecode.h"

typedef struct {
    Value *data;
    int top;
    int capacity;
} Stack;

void stack_init(Stack *s);
void stack_push(Stack *s, Value val);
Value stack_pop(Stack *s);
Value stack_peek(Stack *s);
int stack_is_empty(Stack *s);
void stack_free(Stack *s);

#endif
