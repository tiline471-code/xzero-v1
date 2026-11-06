#ifndef FRAME_H
#define FRAME_H

#include "raster.h"

typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION,
    NODE_OUTPUT,
    NODE_VAR_DECL
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    char *name;                  // Nombre del programa, función o variable
    char *string_value;          // Valor o contenido asociado
    struct ASTNode **children;   
    int child_count;
} ASTNode;

void frame_init(void);
ASTNode *frame_parse(void);
void free_ast(ASTNode *node);
void print_ast(ASTNode *node, int indent);

#endif
