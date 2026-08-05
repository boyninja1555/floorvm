#include "lexer.h"
#include <stdlib.h>

char *tokentype_string(TokenType type)
{
    switch (type)
    {
    case TOKEN_EOF:
        return "EOF";
    case TOKEN_SEP:
        return "SEP";
    case TOKEN_U8:
        return "U8";
    case TOKEN_U16:
        return "U16";
    case TOKEN_U32:
        return "U32";
    case TOKEN_IDENTIFIER:
        return "IDENTIFIER";
    case TOKEN_STRING:
        return "STRING";
    case TOKEN_INT:
        return "INT";
    }
}

Token token_new(TokenType type, const String value)
{
    return (Token){.type = type, .value = (String)value};
}

LexerState lexer_new(const String source)
{
    u32 length = 0;
    u32 cap = 8;
    return (LexerState){
        .source = string_dup(source),
        .idx = 0,
        .tokens = malloc(cap * sizeof(Token)),
        .tokens_length = length,
        .tokens_cap = cap,
        .reached_eof = false,
        .in_string = false,
        .in_int = false,
    };
}

void lexer_token(LexerState *lexer, const Token token)
{
    if (lexer->tokens_length >= lexer->tokens_cap)
    {
        lexer->tokens_cap *= 2;
        Token *new = realloc(lexer->tokens, lexer->tokens_cap * sizeof(Token));
        if (!new)
            return;

        lexer->tokens = new;
    }

    lexer->tokens[lexer->tokens_length++] = token;
}

static TokenType classify_literal(const String literal)
{
    if (string_equals_cstr(literal, "u8"))
        return TOKEN_U8;

    if (string_equals_cstr(literal, "u16"))
        return TOKEN_U16;

    if (string_equals_cstr(literal, "u32"))
        return TOKEN_U32;

    // if (string_is_integer(literal))
    //     return TOKEN_INT;

    return TOKEN_IDENTIFIER;
}

static void lexer_fillit(LexerState *lexer)
{
    if (string_equals_cstr(lexer->temp, ""))
        return;

    lexer_token(lexer, token_new(classify_literal(lexer->temp), string_dup(lexer->temp)));
    string_set(&lexer->temp, "");
}

bool lexer_lex(LexerState *lexer)
{
    if (lexer->idx >= lexer->source.length)
    {
        lexer_fillit(lexer);
        lexer_token(lexer, token_new(TOKEN_EOF, string_from_cstr("EOF")));
        lexer->reached_eof = true;
        return true;
    }

    char c = lexer->source.data[lexer->idx++];

    if (lexer->in_string)
    {
        if (c == '"')
        {
            lexer_token(lexer, token_new(TOKEN_STRING, string_dup(lexer->temp)));
            string_set(&lexer->temp, "");
            lexer->in_string = false;
            return 1;
        }

        string_append(&lexer->temp, c);
        return 1;
    }

    if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
    {
        lexer_fillit(lexer);
        return true;
    }

    if (c == ';')
    {
        lexer_fillit(lexer);
        lexer_token(lexer, token_new(TOKEN_SEP, string_from_cstr(cstr_dupwt(&c, 1))));
        return true;
    }

    if (c == '"')
    {
        lexer_fillit(lexer);
        lexer->in_string = true;
        return true;
    }

    string_append(&lexer->temp, c);
    return true;
}

void lexer_free(LexerState *lexer)
{
    string_free(&lexer->source);
    lexer->idx = 0;

    free(lexer->tokens);
    lexer->tokens_length = 0;
    lexer->tokens_cap = 8;
}
