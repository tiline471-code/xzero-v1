#include <stdlib.h>
#include <stdio.h>
#include "stack.h"

void stack_init(Stack *s) {
    s->data = NULL;
    s->top = -1;
    s->capacity = 0;
}

void stack_push(Stack *s, Value val) {
    if (s->top + 1 >= s->capacity) {
        s->capacity = s->capacity < 8 ? 8 : s->capacity * 2;
        s->data = realloc(s->data, sizeof(Value) * s->capacity);
    }
    s->data[++s->top] = val;
}

Value stack_pop(Stack *s) {
    if (s->top < 0) {
        Value empty = {0};
        return empty;
    }
    return s->data[s->top--];
}

Value stack_peek(Stack *s) {
    if (s->top < 0) {
        Value empty = {0};
        return empty;
    }
    return s->data[s->top];
}

int stack_is_empty(Stack *s) {
    return s->top < 0;
}

void stack_free(Stack *s) {
    free(s->data);
    stack_init(s);
}
