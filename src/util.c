#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#endif

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
static bool program_is_disc = false;

void program_romfile(const char *filepath, bool is_disc)
{
    program_is_disc = is_disc;

    if (program_filepath != NULL)
    {
        free(program_filepath);
        program_filepath = NULL;
    }

    if (filepath != NULL)
        program_filepath = fv_strdup(filepath);
}

byte program_loadisc(Machine *machine)
{
    if (program_filepath == NULL)
        return MS_KO;
#if defined(_WIN32) || defined(_WIN64)
    HANDLE hDrive = CreateFileA(program_filepath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDrive == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "Failed to open optical drive! (Error %lu)\n", GetLastError());
        return MS_KO;
    }

    DWORD bytes_read = 0;
    BOOL result = ReadFile(hDrive, machine->ram, (DWORD)PROGRAM_SIZE, &bytes_read, NULL);
    CloseHandle(hDrive);
    if (!result || bytes_read != PROGRAM_SIZE)
    {
        fprintf(stderr, "Disc read error! Expected %d bytes, got %lu. (Error %lu)\n", PROGRAM_SIZE, bytes_read, GetLastError());
        return MS_KO;
    }
#else
    int fd = open(program_filepath, O_RDONLY);
    if (fd < 0)
    {
        perror("Failed to open optical drive!");
        return MS_KO;
    }

    if (lseek(fd, 0, SEEK_SET) == (off_t)-1)
    {
        perror("Seek to LBA 0 failed!");
        close(fd);
        return MS_KO;
    }

    ssize_t bytes_read = read(fd, machine->ram, PROGRAM_SIZE);
    close(fd);
    if (bytes_read != PROGRAM_SIZE)
    {
        fprintf(stderr, "Disc read error! expected %d bytes, got %zd.\n", PROGRAM_SIZE, bytes_read);
        return MS_KO;
    }
#endif
    return MS_OK;
}

byte program_loadrom(Machine *machine)
{
    if (program_filepath == NULL)
    {
        fprintf(stderr, "ROM filepath was left unset prior to load attempts!\n");
        return MS_KO;
    }

    if (program_is_disc)
        return program_loadisc(machine);

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
