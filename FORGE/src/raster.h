#ifndef RASTER_H
#define RASTER_H

typedef enum {
    TOKEN_EOF = 0,
    TOKEN_ERROR,

    // Palabras clave de Kryon
    TOKEN_PROGRAM,
    TOKEN_FN,
    TOKEN_OUTPUT,
    TOKEN_VAR,      // Palabra clave: var

    // Literales e Identificadores
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_NUMBER,

    // Delimitadores y Símbolos
    TOKEN_LBRACE,   // {
    TOKEN_RBRACE,   // }
    TOKEN_LPAREN,   // (
    TOKEN_RPAREN,   // )
    TOKEN_SEMICOLON,// ;
    TOKEN_ASSIGN    // =
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
