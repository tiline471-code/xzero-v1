#ifndef RASTER_H
#define RASTER_H

typedef enum {
    TOKEN_EOF = 0,
    TOKEN_UNKNOWN,
    TOKEN_FN,
    TOKEN_OUTPUT,
    TOKEN_VAR,
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_INT,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_SEMICOLON,
    TOKEN_ASSIGN,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_COMMA
} TokenType;

typedef struct {
    TokenType type;
    char *lexeme;
    int line;
} Token;

Token *raster_tokenize(const char *source, int *out_count);
void raster_free_tokens(Token *tokens, int count);

#endif
