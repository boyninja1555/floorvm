#include "parser.h"
#include <stdlib.h>
#include <stdio.h>

ParserState parser_new(LexerState *lexer)
{
    const u32 cap = 8;
    return (ParserState){
        .tokens = lexer->tokens,
        .tokens_length = lexer->tokens_length,
        .idx = 0,
        .nodes = malloc(cap * sizeof(AST *)),
        .nodes_length = 0,
        .nodes_cap = cap,
    };
}

void parser_node(ParserState *parser, AST *node)
{
    if (parser->nodes_length >= parser->nodes_cap)
    {
        parser->nodes_cap *= 2;
        AST **new = realloc(parser->nodes, parser->nodes_cap * sizeof(AST *));
        if (!new)
            return;

        parser->nodes = new;
    }

    parser->nodes[parser->nodes_length++] = node;
}

static bool parser_expect(ParserState *parser, TokenType type)
{
    if (parser->idx >= parser->tokens_length)
    {
        fprintf(stderr, "Reached EOF while expecting %s!\n", tokentype_string(type));
        return false;
    }

    if (parser->tokens[parser->idx].type != type)
        return false;

    return true;
}

bool parser_parse(ParserState *parser)
{
    if (parser->idx >= parser->tokens_length)
    {
        fprintf(stderr, "Did not reach EOF before running out of tokens!\n");
        return false;
    }

    Token token = parser->tokens[parser->idx++];

    if (token.type == TOKEN_EOF)
    {
        parser->reached_eof = true;
        return true;
    }

    if (token.type == TOKEN_TYPE)
    {
        if (!parser_expect(parser, TOKEN_IDENTIFIER))
        {
            fprintf(stderr, "Expected name after the %s of a variable declaration!\n", string_to_cstr(token.value));
            return false;
        }

        String name = parser->tokens[parser->idx].value;
        parser->idx++;

        if (!parser_expect(parser, TOKEN_AT))
        {
            fprintf(stderr, "Expected '@' symbol after the name of a variable declaration!\n");
            return false;
        }

        parser->idx++;

        if (!parser_expect(parser, TOKEN_INT))
        {
            fprintf(stderr, "Expected integer address after the '@' symbol of a variable declaration!\n");
            return false;
        }

        String address_raw = parser->tokens[parser->idx].value;
        parser->idx++;

        AstUvarDeclare *var = malloc(sizeof(AstUvarDeclare));
        var->base.type = AST_UVAR_DECLARE;
        var->type = string_dup(token.value);
        var->name = name;
        var->address = atoi(string_to_cstr(address_raw));

        if (!parser_expect(parser, TOKEN_SEP))
        {
            fprintf(stderr, "Expected semicolon at the end of a variable declaration!\n");
            return false;
        }

        parser->idx++;
        parser_node(parser, (AST *)var);
        return true;
    }

    if (token.type == TOKEN_IDENTIFIER)
    {
        if (parser_expect(parser, TOKEN_EQUALS))
        {
            parser->idx++;

            if (!parser_expect(parser, TOKEN_INT))
            {
                fprintf(stderr, "Expected integer value after the '=' symbol of a variable assignment!\n");
                return false;
            }

            String value_raw = parser->tokens[parser->idx].value;
            parser->idx++;

            AstUvarAssign *assignment = malloc(sizeof(AstUvarAssign));
            assignment->base.type = AST_UVAR_ASSIGN;
            assignment->name = token.value;
            assignment->value = atoi(string_to_cstr(value_raw));

            if (!parser_expect(parser, TOKEN_SEP))
            {
                fprintf(stderr, "Expected semicolon at the end of a variable assignment!\n");
                return false;
            }

            parser->idx++;
            parser_node(parser, (AST *)assignment);
            return true;
        }

        return true;
    }

    return true;
}

void parser_free(ParserState *parser)
{
    for (u32 i = 0; i < parser->nodes_length; i++)
    {
        AST *node = parser->nodes[i];
        switch (node->type)
        {
        case AST_UVAR_DECLARE:
        {
            AstUvarDeclare *var = (AstUvarDeclare *)node;
            string_free(&var->type);
            string_free(&var->name);
            free(var);
            break;
        }

        case AST_LITERAL_INT:
        {
            free((AstLiteralInt *)node);
            break;
        }

        case AST_LITERAL_STRING:
        {
            AstLiteralString *str = (AstLiteralString *)node;
            string_free(&str->value);
            free(str);
            break;
        }

        case AST_LITERAL_IDENTIFIER:
        {
            AstLiteralIdentifier *id = (AstLiteralIdentifier *)node;
            string_free(&id->value);
            free(id);
            break;
        }
        }
    }

    free(parser->nodes);
    parser->nodes = NULL;
    parser->nodes_length = 0;
    parser->nodes_cap = 0;
    parser->idx = 0;
}
