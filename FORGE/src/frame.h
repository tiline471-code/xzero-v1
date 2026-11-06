#ifndef FRAME_H
#define FRAME_H

#include "raster.h"

typedef enum {
    NODE_PROGRAM,
    NODE_EXPR_STMT,
    NODE_PRINT,
    NODE_VAR_DECL,
    NODE_ASSIGN,
    NODE_BINARY,
    NODE_LITERAL,
    NODE_VARIABLE,
    NODE_CALL,
    NODE_IF,
    NODE_WHILE
} NodeType;

struct ASTNode;

typedef struct ASTNode {
    NodeType type;
    union {
        struct {
            struct ASTNode **statements;
            int count;
        } program;
        struct {
            struct ASTNode *expr;
        } expr_stmt;
        struct {
            struct ASTNode *expr;
        } print;
        struct {
            char *name;
            struct ASTNode *value;
        } var_decl;
        struct {
            char *name;
            struct ASTNode *value;
        } assign;
        struct {
            struct ASTNode *left;
            struct ASTNode *right;
            char op[4];
        } binary;
        struct {
            int value;
        } literal;
        struct {
            char *name;
        } variable;
        struct {
            char *name;
            struct ASTNode **args;
            int arg_count;
        } call;
    } data;
} ASTNode;

ASTNode *frame_parse(Token *tokens, int count);
void free_ast(ASTNode *node);

#endif
