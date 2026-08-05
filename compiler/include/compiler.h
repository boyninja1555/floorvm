#pragma once

#include <stdio.h>
#include "parser.h"

typedef struct
{
    String type;
    String name;
    u32 address;
} CompilerVariable;

typedef struct
{
    AST **nodes;
    u32 nodes_length;

    CompilerVariable *variables;
    u32 variables_length;
    u32 variables_cap;

    FILE *out;
} CompilerState;

CompilerState compiler_new(ParserState *parser, FILE *out);

bool compiler_compile(CompilerState *compiler);

void compiler_free(CompilerState *compiler);
