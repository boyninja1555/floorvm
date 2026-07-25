#pragma once
#include <stdint.h>

#define TARGET_FPS 60
#define NS_PER_FRAME (1000000000ULL / TARGET_FPS)
#define CYCLES_PER_FRAME 16666

#define SCREEN_WIDTH 40
#define SCREEN_HEIGHT 30
#define REGISTERS_COUNT 8

#define PROGRAM_SIZE 8192
#define VRAM_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)
#define USER_SIZE 4096
#define RAM_SIZE (PROGRAM_SIZE + VRAM_SIZE + USER_SIZE)

#define PROGRAM_START 0
#define VRAM_START PROGRAM_SIZE
#define USER_START (VRAM_START + VRAM_SIZE)

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef u8 byte;

typedef enum
{
    // Control
    OP_HALT = 0x00,
    OP_JMP = 0x01,
    OP_JMPIZ = 0x02,
    OP_JMPNZ = 0x03,

    // Memory
    OP_LOAD32 = 0x10,
    OP_LOAD8 = 0x11,
    OP_LOAD32P = 0x12,
    OP_LOAD8P = 0x13,
    OP_STORE32 = 0x14,
    OP_STORE8 = 0x15,
    OP_STORE32P = 0x16,
    OP_STORE8P = 0x17,
    OP_SET32 = 0x18,
    OP_SET8 = 0x19,
    OP_COPY = 0x20,

    // ALU
    OP_ADD = 0x21,
    OP_SUB = 0x22,
    OP_MUL = 0x23,
    OP_DIV = 0x24,
} MachineOpcode;

typedef enum
{
    MS_OK = 0,
    MS_KO = 1,
} MachineStatus;

typedef struct
{
    byte status;
    byte ram[RAM_SIZE];
    u16 pc;
    u32 registers[REGISTERS_COUNT];
} Machine;

Machine machine_init();

void machine_reset(Machine *machine);

void machine_step(Machine *machine);
