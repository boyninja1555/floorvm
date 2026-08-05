#include "compiler.h"
#include <stdlib.h>
#include <string.h>

static void compiler_variable(CompilerState *compiler, String type, String name, u32 address)
{
    if (compiler->variables_length >= compiler->variables_cap)
    {
        compiler->variables_cap *= 2;
        CompilerVariable *new = realloc(compiler->variables, compiler->variables_cap * sizeof(CompilerVariable));
        if (!new)
            return;

        compiler->variables = new;
    }

    compiler->variables[compiler->variables_length++] = (CompilerVariable){.type = string_dup(type), .name = string_dup(name), .address = address};
}

static CompilerVariable *compiler_find_variable(CompilerState *compiler, String name)
{
    for (u32 i = 0; i < compiler->variables_length; i++)
        if (string_equals(compiler->variables[i].name, name))
            return &compiler->variables[i];

    return NULL;
}

CompilerState compiler_new(ParserState *parser, FILE *out)
{
    return (CompilerState){
        .nodes = parser->nodes,
        .nodes_length = parser->nodes_length,
        .variables = malloc(8 * sizeof(CompilerVariable)),
        .variables_length = 0,
        .variables_cap = 8,
        .out = out,
    };
}

bool compiler_compile(CompilerState *compiler)
{
    for (u32 i = 0; i < compiler->nodes_length; i++)
    {
        AST *node = compiler->nodes[i];
        if (node->type == AST_UVAR_DECLARE)
        {
            AstUvarDeclare *var = (AstUvarDeclare *)node;
            compiler_variable(compiler, var->type, var->name, var->address);
        }
    }

    for (u32 i = 0; i < compiler->variables_length; i++)
    {
        CompilerVariable var = compiler->variables[i];
        fprintf(compiler->out, "#define %s %u\n", string_to_cstr(var.name), var.address);
    }

    fprintf(compiler->out, "\n_start:\n");
    for (u32 i = 0; i < compiler->nodes_length; i++)
    {
        AST *node = compiler->nodes[i];
        switch (node->type)
        {
        case AST_UVAR_DECLARE:
            break;

        case AST_UVAR_ASSIGN:
        {
            AstUvarAssign *assignment = (AstUvarAssign *)node;
            CompilerVariable *var = compiler_find_variable(compiler, assignment->name);
            if (var->address == 0xFFFFFFFF)
            {
                fprintf(stderr, "Unknown variable! %s\n", string_to_cstr(assignment->name));
                return false;
            }

            if (string_equals_cstr(var->type, "u8"))
            {
                if (assignment->value < 0 || assignment->value > 255)
                {
                    fprintf(stderr, "Invalid u8! Must be 0-255 (inclusive).\n");
                    return false;
                }

                fprintf(compiler->out, "\tset8 r0 %u\n", assignment->value);
                fprintf(compiler->out, "\tstore8 r0 ${%s}\n", string_to_cstr(assignment->name));
            }
            else if (string_equals_cstr(var->type, "u32"))
            {
                if (assignment->value < 0 || assignment->value > 255)
                {
                    fprintf(stderr, "Invalid u32! Must be 0-4294967295 (inclusive).\n");
                    return false;
                }

                fprintf(compiler->out, "\tset32 r0 %u\n", assignment->value);
                fprintf(compiler->out, "\tstore32 r0 ${%s}\n", string_to_cstr(assignment->name));
            }
            else
            {
                fprintf(stderr, "Invalid datatype %s!\n", var->type);
                return false;
            }

            break;
        }

        default:
        {
            fprintf(stderr, "Unsupported AST node!\n");
            return false;
        }
        }
    }

    return true;
}

void compiler_free(CompilerState *compiler)
{
    for (u32 i = 0; i < compiler->variables_length; i++)
    {
        string_free(&compiler->variables[i].type);
        string_free(&compiler->variables[i].name);
    }

    free(compiler->variables);
}
