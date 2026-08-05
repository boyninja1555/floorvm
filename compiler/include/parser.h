#pragma once

#include <stdbool.h>
#include <libcfloor/string.h>
#include "lexer.h"

typedef enum
{
    AST_VAR_DECLARE,
    AST_LITERAL_INT,
    AST_LITERAL_STRING,
    AST_LITERAL_IDENTIFIER,
} AstType;

typedef struct AST AST;

struct AST
{
    AstType type;
};

typedef struct
{
    AST base;
    String type;
    String name;
    u32 address;
    AST *value;
} AstVarDeclare;

typedef struct
{
    AST base;
    u32 value;
} AstLiteralInt;

typedef struct
{
    AST base;
    String value;
} AstLiteralString;

typedef struct
{
    AST base;
    String value;
} AstLiteralIdentifier;

typedef struct
{
    Token *tokens;
    u32 idx;

    AST **nodes;
    u32 nodes_length;
    u32 nodes_cap;

    bool reached_eof;
} ParserState;

ParserState parser_new(LexerState *lexer);

void parser_node(ParserState *parser, AST *node);

void parser_free(ParserState *parser);
