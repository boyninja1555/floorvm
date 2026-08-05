#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "lexer.h"
#include "parser.h"
#include "compiler.h"

static String read_file(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (!file)
        return string_create();

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (size <= 0)
    {
        fclose(file);
        return string_create();
    }

    char *buffer = malloc(size);
    if (!buffer)
    {
        fclose(file);
        return string_create();
    }

    fread(buffer, 1, size, file);
    fclose(file);
    String result = {.data = buffer, .length = size, .cap = size};
    return result;
}

int main(int argc, const char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <source.fvmc> <program.txt>\n", argv[0]);
        return 1;
    }

    String source = read_file(argv[1]);
    if (source.data == NULL)
    {
        fprintf(stderr, "Failed to read %s!\n", argv[1]);
        return 1;
    }

    LexerState lexer = lexer_new(source);
    string_free(&source);
    while (!lexer.reached_eof)
        if (!lexer_lex(&lexer))
        {
            lexer_free(&lexer);
            return 1;
        }

    for (u32 i = 0; i < lexer.tokens_length; i++)
    {
        Token token = lexer.tokens[i];
        printf("%s => \"%s\"\n", tokentype_string(token.type), string_to_cstr(token.value));
    }

    printf("\n");
    ParserState parser = parser_new(&lexer);
    while (!parser.reached_eof)
        if (!parser_parse(&parser))
        {
            parser_free(&parser);
            lexer_free(&lexer);
            return 1;
        }

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

    printf("\n");

    FILE *out = fopen(argv[2], "w");
    if (!out)
    {
        fprintf(stderr, "Failed to open %s!\n", argv[2]);
        parser_free(&parser);
        lexer_free(&lexer);
        return 1;
    }

    CompilerState compiler = compiler_new(&parser, out);
    if (compiler_compile(&compiler))
        printf("Compiled %s to %s!\n", argv[1], argv[2]);

    compiler_free(&compiler);
    parser_free(&parser);
    lexer_free(&lexer);
    return 0;
}
