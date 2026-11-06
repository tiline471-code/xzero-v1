#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raster.h"
#include "frame.h"

static Token current_token;

void frame_init(void) {
    current_token = raster_next_token();
}

static void consume(TokenType type) {
    if (current_token.type == type) {
        free(current_token.lexeme);
        current_token = raster_next_token();
    } else {
        printf("FRAME Error: Se esperaba el token tipo %d (Recibido tipo %d con lexema '%s')\n", type, current_token.type, current_token.lexeme ? current_token.lexeme : "NULL");
        if (current_token.type != TOKEN_EOF) {
            free(current_token.lexeme);
            current_token = raster_next_token();
        }
    }
}

static ASTNode *create_node(ASTNodeType type, const char *name, const char *string_value) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = type;
    node->name = name ? strdup(name) : NULL;
    node->string_value = string_value ? strdup(string_value) : NULL;
    node->children = NULL;
    node->child_count = 0;
    return node;
}

static void add_child(ASTNode *parent, ASTNode *child) {
    parent->child_count++;
    parent->children = (ASTNode **)realloc(parent->children, sizeof(ASTNode *) * parent->child_count);
    parent->children[parent->child_count - 1] = child;
}

static ASTNode *parse_function(void);
static ASTNode *parse_statement(void);
static ASTNode *parse_var_decl(void);
static ASTNode *parse_output(void);

static ASTNode *parse_program(void) {
    consume(TOKEN_PROGRAM);
    
    char *prog_name = strdup(current_token.lexeme);
    consume(TOKEN_IDENTIFIER);
    
    ASTNode *prog_node = create_node(NODE_PROGRAM, prog_name, NULL);
    free(prog_name);

    consume(TOKEN_LBRACE);

    while (current_token.type == TOKEN_FN) {
        add_child(prog_node, parse_function());
    }

    consume(TOKEN_RBRACE);
    return prog_node;
}

static ASTNode *parse_function(void) {
    consume(TOKEN_FN);

    char *fn_name = strdup(current_token.lexeme);
    consume(TOKEN_IDENTIFIER);

    ASTNode *fn_node = create_node(NODE_FUNCTION, fn_name, NULL);
    free(fn_name);

    consume(TOKEN_LPAREN);
    consume(TOKEN_RPAREN);
    consume(TOKEN_LBRACE);

    while (current_token.type == TOKEN_VAR || current_token.type == TOKEN_OUTPUT) {
        add_child(fn_node, parse_statement());
    }

    consume(TOKEN_RBRACE);
    return fn_node;
}

static ASTNode *parse_statement(void) {
    if (current_token.type == TOKEN_VAR) {
        return parse_var_decl();
    } else if (current_token.type == TOKEN_OUTPUT) {
        return parse_output();
    }
    return NULL;
}

static ASTNode *parse_var_decl(void) {
    consume(TOKEN_VAR);

    char *var_name = strdup(current_token.lexeme);
    consume(TOKEN_IDENTIFIER);

    consume(TOKEN_ASSIGN);

    char *var_value = strdup(current_token.lexeme);
    consume(TOKEN_STRING);

    consume(TOKEN_SEMICOLON);

    ASTNode *var_node = create_node(NODE_VAR_DECL, var_name, var_value);
    free(var_name);
    free(var_value);

    return var_node;
}

static ASTNode *parse_output(void) {
    consume(TOKEN_OUTPUT);
    consume(TOKEN_LPAREN);

    char *val = strdup(current_token.lexeme);
    if (current_token.type == TOKEN_STRING || current_token.type == TOKEN_IDENTIFIER) {
        consume(current_token.type);
    } else {
        consume(TOKEN_STRING);
    }

    ASTNode *out_node = create_node(NODE_OUTPUT, NULL, val);
    free(val);

    consume(TOKEN_RPAREN);
    consume(TOKEN_SEMICOLON);

    return out_node;
}

ASTNode *frame_parse(void) {
    if (current_token.type == TOKEN_PROGRAM) {
        return parse_program();
    }
    return NULL;
}

void free_ast(ASTNode *node) {
    if (!node) return;
    for (int i = 0; i < node->child_count; i++) {
        free_ast(node->children[i]);
    }
    free(node->children);
    free(node->name);
    free(node->string_value);
    free(node);
}

void print_ast(ASTNode *node, int indent) {
    if (!node) return;
    for (int i = 0; i < indent; i++) printf("  ");

    const char *type_str = "UNKNOWN";
    if (node->type == NODE_PROGRAM) type_str = "PROGRAM";
    else if (node->type == NODE_FUNCTION) type_str = "FUNCTION";
    else if (node->type == NODE_OUTPUT) type_str = "OUTPUT";
    else if (node->type == NODE_VAR_DECL) type_str = "VAR_DECL";

    printf("AST NODE: %s", type_str);
    if (node->name) printf(" [name: %s]", node->name);
    if (node->string_value) printf(" [value: \"%s\"]", node->string_value);
    printf("\n");

    for (int i = 0; i < node->child_count; i++) {
        print_ast(node->children[i], indent + 1);
    }
}
