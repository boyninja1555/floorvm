#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *fv_strdup(const char *string)
{
    if (!string)
        return NULL;

    size_t len = strlen(string) + 1;
    char *dest = malloc(len);
    if (dest)
        memcpy(dest, string, len);

    return dest;
}

static char *program_filepath = NULL;

void program_romfile(const char *filepath)
{
    if (program_filepath != NULL)
    {
        free(program_filepath);
        program_filepath = NULL;
    }

    if (filepath != NULL)
        program_filepath = fv_strdup(filepath);
}

byte program_loadrom(Machine *machine)
{
    if (program_filepath == NULL)
    {
        fprintf(stderr, "ROM filepath was left unset prior to load attempts!\n");
        return MS_KO;
    }

    FILE *file = fopen(program_filepath, "rb");
    if (!file)
    {
        fprintf(stderr, "ROM does not exist!\n");
        return MS_KO;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    if (size > PROGRAM_SIZE)
    {
        fprintf(stderr, "ROM too large (%ld bytes), must be %i bytes!\n", size, PROGRAM_SIZE);
        fclose(file);
        return MS_KO;
    }

    fread(machine->ram, sizeof(byte), PROGRAM_SIZE, file);
    fclose(file);
    return MS_OK;
}
