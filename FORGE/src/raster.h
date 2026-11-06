#ifndef RASTER_H
#define RASTER_H

typedef enum {
    TOKEN_EOF = 0, TOKEN_ERROR,
    TOKEN_PROGRAM, TOKEN_FN, TOKEN_OUTPUT,
    TOKEN_IDENTIFIER, TOKEN_STRING,
    TOKEN_LBRACE, TOKEN_RBRACE, TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_SEMICOLON
} TokenType;

typedef struct {
    TokenType type;
    char *lexeme;
    int line;
} Token;

void raster_init(const char *source_code);
Token raster_next_token();

#endif
