#pragma once

#include "typing.h"
#include <stdbool.h>
#include <libcfloor/string.h>

typedef enum
{
    TOKEN_EOF,
    TOKEN_SEP,
    TOKEN_EQUALS,
    TOKEN_AT,

    TOKEN_TYPE,
    TOKEN_IDENTIFIER,

    TOKEN_STRING,
    TOKEN_INT,
} TokenType;

char *tokentype_string(TokenType type);

typedef struct
{
    TokenType type;
    String value;
} Token;

Token token_new(TokenType type, const String value);

typedef struct
{
    String source;
    string_size idx;

    Token *tokens;
    u32 tokens_length;
    u32 tokens_cap;

    bool reached_eof;
    bool in_string;
    bool in_int;

    String temp;
} LexerState;

LexerState lexer_new(const String source);

void lexer_token(LexerState *lexer, const Token token);

bool lexer_lex(LexerState *lexer);

void lexer_free(LexerState *lexer);
