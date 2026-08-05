#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "lexer.h"
#include "parser.h"

int main(int argc, const char *argv[])
{
    LexerState lexer = lexer_new(string_from_cstr("u8 name 9393; name = 12;"));
    while (!lexer.reached_eof)
        if (!lexer_lex(&lexer))
            return 1;

    for (u32 i = 0; i < lexer.tokens_length; i++)
    {
        Token token = lexer.tokens[i];
        printf("%s => \"%s\"\n", tokentype_string(token.type), string_to_cstr(token.value));
    }

    printf("\n");
    ParserState parser = parser_new(&lexer);
    while (!parser.reached_eof)
        if (!parser_parse(&parser))
            return 1;

    for (u32 i = 0; i < parser.nodes_length; i++)
    {
        AST *node = parser.nodes[i];
        switch (node->type)
        {
        case AST_UVAR_DECLARE:
        {
            const AstUvarDeclare *var = (const AstUvarDeclare *)node;
            printf("UVAR_DECLARE\n");
            printf("  type    : %s\n", string_to_cstr(var->type));
            printf("  name    : %s\n", string_to_cstr(var->name));
            printf("  address : %u\n", var->address);
            break;
        }

        case AST_UVAR_ASSIGN:
        {
            const AstUvarAssign *var = (const AstUvarAssign *)node;
            printf("UVAR_ASSIGN\n");
            printf("  name  : %s\n", string_to_cstr(var->name));
            printf("  value : %u\n", var->value);
            break;
        }

        case AST_LITERAL_INT:
        {
            const AstLiteralInt *lit = (const AstLiteralInt *)node;
            printf("LITERAL_INT\n");
            printf("  value : %u\n", lit->value);
            break;
        }

        case AST_LITERAL_STRING:
        {
            const AstLiteralString *lit = (const AstLiteralString *)node;
            char *str = string_to_cstr(lit->value);
            printf("LITERAL_STRING\n");
            printf("  value : \"%s\"\n", str);
            free(str);
            break;
        }

        case AST_LITERAL_IDENTIFIER:
        {
            const AstLiteralIdentifier *lit = (const AstLiteralIdentifier *)node;
            char *str = string_to_cstr(lit->value);
            printf("LITERAL_IDENTIFIER\n");
            printf("  value : %s\n", str);
            free(str);
            break;
        }
        }
    }

    parser_free(&parser);
    lexer_free(&lexer);
    return 0;
}
