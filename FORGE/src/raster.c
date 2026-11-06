#include "raster.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static int is_digit(char c) {
    return c >= '0' && c <= '9';
}

Token *raster_tokenize(const char *source, int *out_count) {
    int capacity = 64;
    int count = 0;
    Token *tokens = malloc(sizeof(Token) * capacity);

    int length = strlen(source);
    int i = 0;

    while (i < length) {
        char c = source[i];

        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            i++;
            continue;
        }

        if (c == '/' && i + 1 < length && source[i+1] == '/') {
            while (i < length && source[i] != '\n') i++;
            continue;
        }

        if (is_alpha(c)) {
            int start = i;
            while (i < length && (is_alpha(source[i]) || is_digit(source[i]))) {
                i++;
            }
            int len = i - start;
            char *lexeme = malloc(len + 1);
            memcpy(lexeme, &source[start], len);
            lexeme[len] = '\0';

            TokenType type = TOKEN_IDENTIFIER;
            if (strcmp(lexeme, "var") == 0) type = TOKEN_VAR;
            else if (strcmp(lexeme, "print") == 0) type = TOKEN_OUTPUT;
            else if (strcmp(lexeme, "fn") == 0) type = TOKEN_FN;

            if (count >= capacity) {
                capacity *= 2;
                tokens = realloc(tokens, sizeof(Token) * capacity);
            }
            tokens[count].type = type;
            tokens[count].lexeme = lexeme;
            tokens[count].line = 1;
            count++;
            continue;
        }

        if (is_digit(c)) {
            int start = i;
            while (i < length && is_digit(source[i])) {
                i++;
            }
            int len = i - start;
            char *lexeme = malloc(len + 1);
            memcpy(lexeme, &source[start], len);
            lexeme[len] = '\0';

            if (count >= capacity) {
                capacity *= 2;
                tokens = realloc(tokens, sizeof(Token) * capacity);
            }
            tokens[count].type = TOKEN_INT;
            tokens[count].lexeme = lexeme;
            tokens[count].line = 1;
            count++;
            continue;
        }

        TokenType type = TOKEN_UNKNOWN;
        char lexeme[2] = {c, '\0'};

        if (c == '=') type = TOKEN_ASSIGN;
        else if (c == '+') type = TOKEN_PLUS;
        else if (c == '-') type = TOKEN_MINUS;
        else if (c == '*') type = TOKEN_STAR;
        else if (c == '/') type = TOKEN_SLASH;
        else if (c == ';') type = TOKEN_SEMICOLON;
        else if (c == '(') type = TOKEN_LPAREN;
        else if (c == ')') type = TOKEN_RPAREN;
        else if (c == '{') type = TOKEN_LBRACE;
        else if (c == '}') type = TOKEN_RBRACE;
        else if (c == ',') type = TOKEN_COMMA;

        i++;
        if (count >= capacity) {
            capacity *= 2;
            tokens = realloc(tokens, sizeof(Token) * capacity);
        }
        tokens[count].type = type;
        tokens[count].lexeme = strdup(lexeme);
        tokens[count].line = 1;
        count++;
    }

    if (count >= capacity) {
        capacity += 1;
        tokens = realloc(tokens, sizeof(Token) * capacity);
    }
    tokens[count].type = TOKEN_EOF;
    tokens[count].lexeme = strdup("");
    tokens[count].line = 1;
    count++;

    *out_count = count;
    return tokens;
}

void raster_free_tokens(Token *tokens, int count) {
    if (!tokens) return;
    for (int i = 0; i < count; i++) {
        free(tokens[i].lexeme);
    }
    free(tokens);
}
