#include "parser.h"
#include <stdlib.h>

ParserState parser_new(LexerState *lexer)
{
    const u32 cap = 8;
    return (ParserState){
        .tokens = lexer->tokens,
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

void parser_free(ParserState *parser)
{
    for (u32 i = 0; i < parser->nodes_length; i++)
    {
        AST *node = parser->nodes[i];
        switch (node->type)
        {
        case AST_VAR_DECLARE:
        {
            AstVarDeclare *var = (AstVarDeclare *)node;
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
