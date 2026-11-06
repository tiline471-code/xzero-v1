#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "raster.h"

static const char *src_cursor;
static int current_line = 1;
static int current_col = 1;

void raster_init(const char *source_code) {
    src_cursor = source_code;
    current_line = 1;
    current_col = 1;
}

static Token create_token(TokenType type, const char *lexeme) {
    Token token;
    token.type = type;
    token.lexeme = strdup(lexeme);
    token.line = current_line;
    token.column = current_col;
    return token;
}

Token raster_next_token() {
    while (*src_cursor != '\0') {
        if (*src_cursor == ' ' || *src_cursor == '\r' || *src_cursor == '\t') {
            src_cursor++;
            current_col++;
            continue;
        }
        if (*src_cursor == '\n') {
            src_cursor++;
            current_line++;
            current_col = 1;
            continue;
        }

        if (*src_cursor == '{') { src_cursor++; current_col++; return create_token(TOKEN_LBRACE, "{"); }
        if (*src_cursor == '}') { src_cursor++; current_col++; return create_token(TOKEN_RBRACE, "}"); }
        if (*src_cursor == '(') { src_cursor++; current_col++; return create_token(TOKEN_LPAREN, "("); }
        if (*src_cursor == ')') { src_cursor++; current_col++; return create_token(TOKEN_RPAREN, ")"); }
        if (*src_cursor == ';') { src_cursor++; current_col++; return create_token(TOKEN_SEMICOLON, ";"); }
        if (*src_cursor == '=') { src_cursor++; current_col++; return create_token(TOKEN_ASSIGN, "="); }
        if (*src_cursor == '+') { src_cursor++; current_col++; return create_token(TOKEN_PLUS, "+"); }
        if (*src_cursor == '-') { src_cursor++; current_col++; return create_token(TOKEN_MINUS, "-"); }
        if (*src_cursor == '*') { src_cursor++; current_col++; return create_token(TOKEN_STAR, "*"); }
        if (*src_cursor == '/') { src_cursor++; current_col++; return create_token(TOKEN_SLASH, "/"); }

        if (*src_cursor == '"') {
            src_cursor++;
            current_col++;
            const char *start = src_cursor;
            while (*src_cursor != '"' && *src_cursor != '\0') {
                src_cursor++;
                current_col++;
            }
            int length = src_cursor - start;
            char *value = (char *)malloc(length + 1);
            strncpy(value, start, length);
            value[length] = '\0';
            if (*src_cursor == '"') {
                src_cursor++;
                current_col++;
            }
            Token t = create_token(TOKEN_STRING, value);
            free(value);
            return t;
        }

        if (isdigit(*src_cursor)) {
            const char *start = src_cursor;
            while (isdigit(*src_cursor)) {
                src_cursor++;
                current_col++;
            }
            int length = src_cursor - start;
            char *num = (char *)malloc(length + 1);
            strncpy(num, start, length);
            num[length] = '\0';
            Token t = create_token(TOKEN_NUMBER, num);
            free(num);
            return t;
        }

        if (isalpha(*src_cursor) || *src_cursor == '_') {
            const char *start = src_cursor;
            while (isalnum(*src_cursor) || *src_cursor == '_') {
                src_cursor++;
                current_col++;
            }
            int length = src_cursor - start;
            char *ident = (char *)malloc(length + 1);
            strncpy(ident, start, length);
            ident[length] = '\0';

            TokenType type = TOKEN_IDENTIFIER;
            if (strcmp(ident, "program") == 0) type = TOKEN_PROGRAM;
            else if (strcmp(ident, "fn") == 0) type = TOKEN_FN;
            else if (strcmp(ident, "output") == 0) type = TOKEN_OUTPUT;
            else if (strcmp(ident, "var") == 0) type = TOKEN_VAR;

            Token t = create_token(type, ident);
            free(ident);
            return t;
        }

        char unknown[2] = { *src_cursor, '\0' };
        src_cursor++;
        current_col++;
        return create_token(TOKEN_ERROR, unknown);
    }

    return create_token(TOKEN_EOF, "");
}
