#ifndef RASTER_H
#define RASTER_H

typedef enum {
    TOKEN_EOF = 0,
    TOKEN_ERROR,
    TOKEN_PROGRAM,
    TOKEN_FN,
    TOKEN_OUTPUT,
    TOKEN_VAR,
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_NUMBER,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_SEMICOLON,
    TOKEN_ASSIGN,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH
} TokenType;

typedef struct {
    TokenType type;
    char *lexeme;
    int line;
    int column;
} Token;

void raster_init(const char *source_code);
Token raster_next_token();

#endif
