#pragma once
#include "machine.h"

char *fv_strdup(const char *string);

void program_romfile(const char *filepath, bool is_disc);

byte program_loadrom(Machine *machine);
