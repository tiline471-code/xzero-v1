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
    NODE_STRING_LITERAL,
    NODE_VARIABLE,
    NODE_CALL,
    NODE_IF,
    NODE_WHILE,
    NODE_BLOCK,
    NODE_FN_DECL,
    NODE_RETURN
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
            char *value;
        } string_literal;
        struct {
            char *name;
        } variable;
        struct {
            char *name;
            struct ASTNode **args;
            int arg_count;
        } call;
        struct {
            struct ASTNode *condition;
            struct ASTNode *then_branch;
            struct ASTNode *else_branch;
        } if_stmt;
        struct {
            struct ASTNode *condition;
            struct ASTNode *body;
        } while_stmt;
        struct {
            struct ASTNode **statements;
            int count;
        } block;
        struct {
            char *name;
            char **params;
            int param_count;
            struct ASTNode *body;
        } fn_decl;
        struct {
            struct ASTNode *expr;
        } return_stmt;
    } data;
} ASTNode;

ASTNode *frame_parse(Token *tokens, int count);
void free_ast(ASTNode *node);

#endif
