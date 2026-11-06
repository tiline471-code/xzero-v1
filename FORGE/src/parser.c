#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    Token *tokens;
    int count;
    int current;
} Parser;

static Token parser_peek(Parser *p) {
    if (p->current >= p->count) {
        Token t = {TOKEN_EOF, "", 0};
        return t;
    }
    return p->tokens[p->current];
}

static Token parser_advance(Parser *p) {
    if (p->current < p->count) {
        return p->tokens[p->current++];
    }
    return p->tokens[p->count - 1];
}

static int parser_match(Parser *p, TokenType type) {
    Token t = parser_peek(p);
    if (t.type == type) {
        parser_advance(p);
        return 1;
    }
    return 0;
}

static ASTNode *parse_expression(Parser *p);

static ASTNode *parse_primary(Parser *p) {
    Token t = parser_advance(p);
    if (t.type == TOKEN_INT) {
        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = NODE_LITERAL;
        node->data.literal.value = atoi(t.lexeme);
        return node;
    }
    if (t.type == TOKEN_IDENTIFIER) {
        // Check if it's a variable reference or function call
        if (parser_match(p, TOKEN_LPAREN)) {
            // Function call
            char *name = strdup(t.lexeme);
            ASTNode **args = NULL;
            int arg_count = 0;
            if (!parser_match(p, TOKEN_RPAREN)) {
                do {
                    ASTNode *arg = parse_expression(p);
                    args = realloc(args, sizeof(ASTNode *) * (arg_count + 1));
                    args[arg_count++] = arg;
                } while (parser_match(p, TOKEN_COMMA));
                parser_match(p, TOKEN_RPAREN);
            }
            ASTNode *node = malloc(sizeof(ASTNode));
            node->type = NODE_CALL;
            node->data.call.name = name;
            node->data.call.args = args;
            node->data.call.arg_count = arg_count;
            return node;
        } else {
            ASTNode *node = malloc(sizeof(ASTNode));
            node->type = NODE_VARIABLE;
            node->data.variable.name = strdup(t.lexeme);
            return node;
        }
    }
    if (t.type == TOKEN_LPAREN) {
        ASTNode *expr = parse_expression(p);
        parser_match(p, TOKEN_RPAREN);
        return expr;
    }
    return NULL;
}

static ASTNode *parse_term(Parser *p) {
    ASTNode *left = parse_primary(p);
    while (1) {
        Token t = parser_peek(p);
        if (strcmp(t.lexeme, "*") == 0 || strcmp(t.lexeme, "/") == 0) {
            parser_advance(p);
            ASTNode *right = parse_primary(p);
            ASTNode *node = malloc(sizeof(ASTNode));
            node->type = NODE_BINARY;
            node->data.binary.left = left;
            node->data.binary.right = right;
            strcpy(node->data.binary.op, t.lexeme);
            left = node;
        } else {
            break;
        }
    }
    return left;
}

static ASTNode *parse_expression(Parser *p) {
    ASTNode *left = parse_term(p);
    while (1) {
        Token t = parser_peek(p);
        if (strcmp(t.lexeme, "+") == 0 || strcmp(t.lexeme, "-") == 0) {
            parser_advance(p);
            ASTNode *right = parse_term(p);
            ASTNode *node = malloc(sizeof(ASTNode));
            node->type = NODE_BINARY;
            node->data.binary.left = left;
            node->data.binary.right = right;
            strcpy(node->data.binary.op, t.lexeme);
            left = node;
        } else {
            break;
        }
    }
    return left;
}

static ASTNode *parse_statement(Parser *p) {
    Token t = parser_peek(p);
    if (t.type == TOKEN_VAR || (t.type == TOKEN_IDENTIFIER && strcmp(t.lexeme, "var") == 0)) {
        parser_advance(p);
        Token name_tok = parser_advance(p);
        parser_match(p, TOKEN_ASSIGN);
        ASTNode *expr = parse_expression(p);
        parser_match(p, TOKEN_SEMICOLON);

        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = NODE_VAR_DECL;
        node->data.var_decl.name = strdup(name_tok.lexeme);
        node->data.var_decl.value = expr;
        return node;
    }
    if (t.type == TOKEN_PRINT || (t.type == TOKEN_IDENTIFIER && strcmp(t.lexeme, "print") == 0)) {
        parser_advance(p);
        ASTNode *expr = parse_expression(p);
        parser_match(p, TOKEN_SEMICOLON);

        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = NODE_PRINT;
        node->data.print.expr = expr;
        return node;
    }

    // Assignment or expression statement
    Token next_t = p->current + 1 < p->count ? p->tokens[p->current + 1] : (Token){TOKEN_EOF, "", 0};
    if (t.type == TOKEN_IDENTIFIER && next_t.type == TOKEN_ASSIGN) {
        char *name = strdup(parser_advance(p).lexeme);
        parser_advance(p); // consume '='
        ASTNode *expr = parse_expression(p);
        parser_match(p, TOKEN_SEMICOLON);

        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = NODE_ASSIGN;
        node->data.assign.name = name;
        node->data.assign.value = expr;
        return node;
    }

    ASTNode *expr = parse_expression(p);
    parser_match(p, TOKEN_SEMICOLON);
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_EXPR_STMT;
    node->data.expr_stmt.expr = expr;
    return node;
}

ASTNode *parse(Token *tokens, int count) {
    Parser p;
    p.tokens = tokens;
    p.count = count;
    p.current = 0;

    ASTNode **statements = NULL;
    int count_stmts = 0;

    while (p.current < p.count && parser_peek(&p).type != TOKEN_EOF) {
        ASTNode *stmt = parse_statement(&p);
        if (stmt) {
            statements = realloc(statements, sizeof(ASTNode *) * (count_stmts + 1));
            statements[count_stmts++] = stmt;
        } else {
            break;
        }
    }

    ASTNode *program = malloc(sizeof(ASTNode));
    program->type = NODE_PROGRAM;
    program->data.program.statements = statements;
    program->data.program.count = count_stmts;
    return program;
}
