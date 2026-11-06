#ifndef FRAME_H
#define FRAME_H

#include "raster.h"

typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION,
    NODE_VAR_DECL,
    NODE_OUTPUT,
    NODE_BINARY_EXPR,
    NODE_LITERAL,
    NODE_IDENTIFIER
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    char *name;
    char *string_value;
    struct ASTNode **children;
    int child_count;
} ASTNode;

void frame_init(void);
ASTNode *frame_parse(void);
void free_ast(ASTNode *node);
void print_ast(ASTNode *node, int indent);

#endif
