#pragma once
#include "machine.h"

byte renderer_init(int gui_scale);

void renderer_frame(const Machine *machine);

void renderer_cleanup();
