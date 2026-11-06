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

        // Cadenas de texto entre comillas dobles
        if (c == '"') {
            int start = ++i;
            while (i < length && source[i] != '"') {
                i++;
            }
            int len = i - start;
            char *lexeme = malloc(len + 1);
            memcpy(lexeme, &source[start], len);
            lexeme[len] = '\0';
            if (i < length) i++; // saltar comilla de cierre

            if (count >= capacity) {
                capacity *= 2;
                tokens = realloc(tokens, sizeof(Token) * capacity);
            }
            tokens[count].type = TOKEN_STRING;
            tokens[count].lexeme = lexeme;
            tokens[count].line = 1;
            count++;
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
            else if (strcmp(lexeme, "if") == 0) type = TOKEN_IF;
            else if (strcmp(lexeme, "else") == 0) type = TOKEN_ELSE;
            else if (strcmp(lexeme, "while") == 0) type = TOKEN_WHILE;
            else if (strcmp(lexeme, "fn") == 0) type = TOKEN_FN;
            else if (strcmp(lexeme, "return") == 0) type = TOKEN_IDENTIFIER; // se maneja por lexema

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
        char lexeme[3] = {c, '\0', '\0'};

        if (c == '=' && i + 1 < length && source[i+1] == '=') {
            type = TOKEN_EQUAL_EQUAL;
            lexeme[0] = '='; lexeme[1] = '='; lexeme[2] = '\0';
            i += 2;
        } else if (c == '=') { type = TOKEN_ASSIGN; i++; }
        else if (c == '<') { type = TOKEN_LESS; i++; }
        else if (c == '+') { type = TOKEN_PLUS; i++; }
        else if (c == '-') { type = TOKEN_MINUS; i++; }
        else if (c == '*') { type = TOKEN_STAR; i++; }
        else if (c == '/') { type = TOKEN_SLASH; i++; }
        else if (c == ';') { type = TOKEN_SEMICOLON; i++; }
        else if (c == '(') { type = TOKEN_LPAREN; i++; }
        else if (c == ')') { type = TOKEN_RPAREN; i++; }
        else if (c == '{') { type = TOKEN_LBRACE; i++; }
        else if (c == '}') { type = TOKEN_RBRACE; i++; }
        else if (c == ',') { type = TOKEN_COMMA; i++; }
        else { i++; }

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
