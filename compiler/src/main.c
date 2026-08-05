#include <stdbool.h>
#include <stdio.h>
#include "lexer.h"

int main(int argc, const char *argv[])
{
    LexerState lexer = lexer_new(string_from_cstr("u8 name; \"Random string!!\";"));
    while (!lexer.reached_eof)
        if (!lexer_lex(&lexer))
            return 1;

    for (u64 i = 0; i < lexer.tokens_length; i++)
    {
        Token token = lexer.tokens[i];
        printf("%s => \"%s\"\n", tokentype_string(token.type), string_to_cstr(token.value));
    }

    lexer_free(&lexer);
    return 0;
}
