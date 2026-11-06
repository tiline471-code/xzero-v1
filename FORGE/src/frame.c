#include "frame.h"
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
static ASTNode *parse_statement(Parser *p);
static ASTNode *parse_block(Parser *p);

static ASTNode *parse_primary(Parser *p) {
    Token t = parser_advance(p);
    if (t.type == TOKEN_INT) {
        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = NODE_LITERAL;
        node->data.literal.value = atoi(t.lexeme);
        return node;
    }
    if (t.type == TOKEN_STRING) {
        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = NODE_STRING_LITERAL;
        node->data.string_literal.value = strdup(t.lexeme);
        return node;
    }
    if (t.type == TOKEN_IDENTIFIER) {
        char *name = strdup(t.lexeme);
        if (parser_match(p, TOKEN_LPAREN)) {
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
            node->data.variable.name = name;
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

static ASTNode *parse_comparison(Parser *p) {
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
        } else if (t.type == TOKEN_LESS || t.type == TOKEN_EQUAL_EQUAL) {
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

static ASTNode *parse_expression(Parser *p) {
    return parse_comparison(p);
}

static ASTNode *parse_block(Parser *p) {
    parser_match(p, TOKEN_LBRACE);
    ASTNode **statements = NULL;
    int count = 0;

    while (parser_peek(p).type != TOKEN_RBRACE && parser_peek(p).type != TOKEN_EOF) {
        ASTNode *stmt = parse_statement(p);
        if (stmt) {
            statements = realloc(statements, sizeof(ASTNode *) * (count + 1));
            statements[count++] = stmt;
        }
    }
    parser_match(p, TOKEN_RBRACE);

    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_BLOCK;
    node->data.block.statements = statements;
    node->data.block.count = count;
    return node;
}

static ASTNode *parse_statement(Parser *p) {
    Token t = parser_peek(p);

    if (t.type == TOKEN_FN || (t.type == TOKEN_IDENTIFIER && strcmp(t.lexeme, "fn") == 0)) {
        parser_advance(p);
        Token name_tok = parser_advance(p);
        parser_match(p, TOKEN_LPAREN);

        char **params = NULL;
        int param_count = 0;
        if (!parser_match(p, TOKEN_RPAREN)) {
            do {
                Token param_tok = parser_advance(p);
                params = realloc(params, sizeof(char *) * (param_count + 1));
                params[param_count++] = strdup(param_tok.lexeme);
            } while (parser_match(p, TOKEN_COMMA));
            parser_match(p, TOKEN_RPAREN);
        }

        ASTNode *body = parse_block(p);

        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = NODE_FN_DECL;
        node->data.fn_decl.name = strdup(name_tok.lexeme);
        node->data.fn_decl.params = params;
        node->data.fn_decl.param_count = param_count;
        node->data.fn_decl.body = body;
        return node;
    }

    if (t.type == TOKEN_IDENTIFIER && strcmp(t.lexeme, "return") == 0) {
        parser_advance(p);
        ASTNode *expr = NULL;
        if (parser_peek(p).type != TOKEN_SEMICOLON) {
            expr = parse_expression(p);
        }
        parser_match(p, TOKEN_SEMICOLON);

        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = NODE_RETURN;
        node->data.return_stmt.expr = expr;
        return node;
    }

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

    if (t.type == TOKEN_OUTPUT || (t.type == TOKEN_IDENTIFIER && strcmp(t.lexeme, "print") == 0)) {
        parser_advance(p);
        ASTNode *expr = parse_expression(p);
        parser_match(p, TOKEN_SEMICOLON);

        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = NODE_PRINT;
        node->data.print.expr = expr;
        return node;
    }

    if (t.type == TOKEN_IF || (t.type == TOKEN_IDENTIFIER && strcmp(t.lexeme, "if") == 0)) {
        parser_advance(p);
        parser_match(p, TOKEN_LPAREN);
        ASTNode *cond = parse_expression(p);
        parser_match(p, TOKEN_RPAREN);

        ASTNode *then_branch = parse_block(p);
        ASTNode *else_branch = NULL;

        Token next = parser_peek(p);
        if (next.type == TOKEN_ELSE || (next.type == TOKEN_IDENTIFIER && strcmp(next.lexeme, "else") == 0)) {
            parser_advance(p);
            else_branch = parse_block(p);
        }

        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = NODE_IF;
        node->data.if_stmt.condition = cond;
        node->data.if_stmt.then_branch = then_branch;
        node->data.if_stmt.else_branch = else_branch;
        return node;
    }

    if (t.type == TOKEN_WHILE || (t.type == TOKEN_IDENTIFIER && strcmp(t.lexeme, "while") == 0)) {
        parser_advance(p);
        parser_match(p, TOKEN_LPAREN);
        ASTNode *cond = parse_expression(p);
        parser_match(p, TOKEN_RPAREN);

        ASTNode *body = parse_block(p);

        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = NODE_WHILE;
        node->data.while_stmt.condition = cond;
        node->data.while_stmt.body = body;
        return node;
    }

    Token next_t = p->current + 1 < p->count ? p->tokens[p->current + 1] : (Token){TOKEN_EOF, "", 0};
    if (t.type == TOKEN_IDENTIFIER && next_t.type == TOKEN_ASSIGN) {
        char *name = strdup(parser_advance(p).lexeme);
        parser_advance(p);
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

ASTNode *frame_parse(Token *tokens, int count) {
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

void free_ast(ASTNode *node) {
    if (!node) return;
    switch (node->type) {
        case NODE_PROGRAM:
            for (int i = 0; i < node->data.program.count; i++) {
                free_ast(node->data.program.statements[i]);
            }
            free(node->data.program.statements);
            break;
        case NODE_BLOCK:
            for (int i = 0; i < node->data.block.count; i++) {
                free_ast(node->data.block.statements[i]);
            }
            free(node->data.block.statements);
            break;
        case NODE_EXPR_STMT:
            free_ast(node->data.expr_stmt.expr);
            break;
        case NODE_PRINT:
            free_ast(node->data.print.expr);
            break;
        case NODE_VAR_DECL:
            free(node->data.var_decl.name);
            free_ast(node->data.var_decl.value);
            break;
        case NODE_ASSIGN:
            free(node->data.assign.name);
            free_ast(node->data.assign.value);
            break;
        case NODE_BINARY:
            free_ast(node->data.binary.left);
            free_ast(node->data.binary.right);
            break;
        case NODE_VARIABLE:
            free(node->data.variable.name);
            break;
        case NODE_STRING_LITERAL:
            free(node->data.string_literal.value);
            break;
        case NODE_CALL:
            free(node->data.call.name);
            for (int i = 0; i < node->data.call.arg_count; i++) {
                free_ast(node->data.call.args[i]);
            }
            free(node->data.call.args);
            break;
        case NODE_IF:
            free_ast(node->data.if_stmt.condition);
            free_ast(node->data.if_stmt.then_branch);
            if (node->data.if_stmt.else_branch) free_ast(node->data.if_stmt.else_branch);
            break;
        case NODE_WHILE:
            free_ast(node->data.while_stmt.condition);
            free_ast(node->data.while_stmt.body);
            break;
        case NODE_FN_DECL:
            free(node->data.fn_decl.name);
            for (int i = 0; i < node->data.fn_decl.param_count; i++) {
                free(node->data.fn_decl.params[i]);
            }
            free(node->data.fn_decl.params);
            free_ast(node->data.fn_decl.body);
            break;
        case NODE_RETURN:
            if (node->data.return_stmt.expr) free_ast(node->data.return_stmt.expr);
            break;
        default:
            break;
    }
    free(node);
}
