<h1 align="center">FloorVM - Fantasy Virtual Game Console</h1>

A lightweight, high-efficiency virtual machine with a variable-length byte-stream ISA, built for deterministic execution and low-level fantasy hardware/game development. Even though it's called a virtual machine, it's technically emulating a fantasy game console never put into production due to my brokeness and lateness.

---

## Architecture

**FloorVM** operates on a unified memory map with 16 general-purpose 32-bit registers, big-endian multi-byte serialization, and a compact variable-length instruction set architecture.

### System specs

| Component              | Value / Specification                          |
|------------------------|------------------------------------------------|
| **Target frame rate**  | 60 FPS (CYCLES_PER_FRAME = 16,666)             |
| **Display resolution** | 40 × 30 pixels (1.2KB total VRAM)              |
| **Registers**          | 16 general-purpose 32-bit registers (r0 – r15) |
| **Program counter**    | Unsigned 16-bit integer (pc)                   |
| **Endianness**         | Big-endian                                     |
| **Total memory**       | 13,489 bytes (13.17 KiB)                       |

---

## Unified memory map

Memory is mapped into four distinct linear segments across a continuous byte array:

| Bounds              | Name                       | Size   |
|---------------------|----------------------------|--------|
| `0x0000` - `0x1FFF` | Program ROM / Code segment | 8 KiB  |
| `0x2000` - `0x24AF` | VRAM display buffer        | 1.2 KB |
| `0x24B0`            | Input controller state     | 1 byte |
| `0x24B1` - `0x34B0` | Safe user work RAM         | 4 KiB  |

### Memory Addresses & Offsets

| Region            | Start address (decimal) | Start address (hex) | Size   | Description                                   |
|-------------------|-------------------------|---------------------|--------|-----------------------------------------------|
| **Program ROM**   | 0                       | 0x0000              | 8 KiB  | Executable bytecode & embedded data assets    |
| **VRAM buffer**   | 8192                    | 0x2000              | 1.2 KB | 40 x 30 byte-addressable display buffer       |
| **Input byte**    | 9392                    | 0x24B0              | 1 byte | Memory-mapped controller button bitmask       |
| **User work RAM** | 9393                    | 0x24B1              | 4 KiB  | General-purpose user-controlled free RAM      |

---

## Controller input map

Input is polled directly by reading the single memory-mapped byte at address 9392 (0x24B0). Bits represent button states (1=pressed,0=released):

| Bit       | Mask (hex) | Flag constant | Button      | Emulator button |
|-----------|------------|---------------|-------------|-----------------|
| **Bit 0** | `0x01`     | BTN_UP        | D-Pad Up    | Up arrow        |
| **Bit 1** | `0x02`     | BTN_DOWN      | D-Pad Down  | Down arrow      |
| **Bit 2** | `0x04`     | BTN_LEFT      | D-Pad Left  | Left arrow      |
| **Bit 3** | `0x08`     | BTN_RIGHT     | D-Pad Right | Right arrow     |
| **Bit 4** | `0x10`     | BTN_A         | Button A    | A               |
| **Bit 5** | `0x20`     | BTN_D         | Button D    | D               |
| **Bit 6** | `0x40`     | BTN_SELECT    | Select      | Enter/return    |
| **Bit 7** | `0x80`     | BTN_START     | Start       | Backspace       |

---

## Instruction set architecture

The ISA uses single-byte opcodes followed by inline variable-length operands.

### Control instructions

| Opcode  | Hex    | Operands           | Description                               |
|---------|--------|--------------------|-------------------------------------------|
| `HALT`  | `0x00` | *None*             | Halts execution                           |
| `JMP`   | `0x01` | `target[4]`        | Unconditionally jumps to a 32-bit address |
| `JMPIZ` | `0x02` | `target[4] reg`    | Jumps to target if `reg` == 0             |
| `JMPNZ` | `0x03` | `target[4] reg`    | Jumps to target if `reg` != 0             |

### Memory instructions

| Opcode     | Hex    | Operands         | Description                                                        |
|------------|--------|------------------|--------------------------------------------------------------------|
| `LOAD32`   | `0x10` | `addr[4] reg`    | Reads 32-bit value from literal address into register              |
| `LOAD8`    | `0x11` | `addr[4] reg`    | Reads byte from literal address into register                      |
| `LOAD32P`  | `0x12` | `addr_reg reg`   | Reads 32-bit value from pointer register into register             |
| `LOAD8P`   | `0x13` | `addr_reg reg`   | Reads byte from pointer register into register                     |
| `STORE32`  | `0x14` | `reg addr[4]`    | Writes 32-bit value from register into literal address             |
| `STORE8`   | `0x15` | `reg addr[4]`    | Writes byte from register into literal address                     |
| `STORE32P` | `0x16` | `reg addr_reg`   | Writes 32-bit value from register into address in pointer register |
| `STORE8P`  | `0x17` | `reg addr_reg`   | Writes byte from register into address in pointer register         |
| `SET32`    | `0x18` | `reg value[4]`   | Sets register to immediate 32-bit value                            |
| `SET8`     | `0x19` | `reg value`      | Sets register to immediate byte value                              |
| `COPY`     | `0x1A` | `reg target_reg` | Copies value from `reg` into `target_reg`                          |

### ALU instructions

> **Note:** All ALU operations store the result into `target_reg`

| Opcode | Hex    | Operands                 | Operation                         |
|--------|--------|--------------------------|-----------------------------------|
| `ADD`  | `0x20` | `target_reg a_reg b_reg` | `target_reg` = `a_reg` + `b_reg`  |
| `SUB`  | `0x21` | `target_reg a_reg b_reg` | `target_reg` = `a_reg` - `b_reg`  |
| `MUL`  | `0x22` | `target_reg a_reg b_reg` | `target_reg` = `a_reg` * `b_reg`  |
| `DIV`  | `0x23` | `target_reg a_reg b_reg` | `target_reg` = `a_reg` / `b_reg`  |
| `AND`  | `0x24` | `target_reg a_reg b_reg` | `target_reg` = `a_reg` & `b_reg`  |
| `OR`   | `0x25` | `target_reg a_reg b_reg` | `target_reg` = `a_reg` \| `b_reg` |
| `XOR`  | `0x26` | `target_reg a_reg b_reg` | `target_reg` = `a_reg` ^ `b_reg`  |
| `NOT`  | `0x27` | `target_reg reg`         | `target_reg` = `~reg`             |
| `SHL`  | `0x28` | `target_reg a_reg b_reg` | `target_reg` = `a_reg << b_reg`   |
| `SHR`  | `0x29` | `target_reg a_reg b_reg` | `target_reg` = `a_reg >> b_reg`   |

---

## Assembler directives and syntax

The Python assembler compiles `.asm` text files into ROMs padded to exactly 8 KiB.

### Syntax Rules

* **Comments:** Anything following a semicolon `;` is treated as a comment.
* **Labels:** Terminated with a colon (e.g. `main_loop:`). Labels resolve to their absolute byte offsets during assembling.
* **Registers:** Prefixed with an `r` (e.g. `r0` `r1` `r15`). Numbers can be hex (`0x2000`) or decimal (`8192`).
* **Macros:** `#define NAME VALUE` replaces any occurrences of `${NAME}` in subsequent lines.

### Directives

| Directive | Syntax                 | Description                                                      |
|-----------|------------------------|------------------------------------------------------------------|
| `.BYTE`   | `.BYTE 0x01 0x02 255`  | Emits raw byte values                                            |
| `.HEX`    | `.HEX 48656C6C6F`      | Emits a raw hex byte stream (must be of even length)             |
| `.UTF8`   | `.UTF8 "Hello World"`  | Emits raw UTF-8 string bytes                                     |
| `.LUTF8`  | `.LUTF8 "Hello World"` | Emits 4-byte length prefix (big-endian) followed by string bytes |

---

## Example program

```assembly
#define VRAM 0x2000
#define INPUT 0x24B0

start:
    SET32 r0 ${VRAM}   ; Loads VRAM base address into r0
    SET8 r1 0xFF       ; Color value (full intensity)
    STORE8 r1 ${VRAM}  ; Draws pixel at top-left corner (0,0)

loop:
    LOAD8 ${INPUT} r2        ; Reads controller button bitmask
    SET8 r3 0x10             ; Bit 4 = BTN_A
    AND r4 r2 r3             ; Checks if A is pressed
    JMPNZ button_pressed r4  ; Jump if BTN_A is down
    JMP loop                 ; Repeats main loop

button_pressed:
    HALT  ; Stops execution
```

---

## Building and running

### Assembling code

Run the assembler script to produce the target `.from` (FloorVM ROM) file:

```bash
python assembler/main.py programs/paint.txt programs/paint.from
```

### Testing with the VM

This repository ships the latest build of the VM as a native executable for 5 platforms:

- Windows on x86_64
- Windows on ARM64
- Linux on x86_64
- Linux on ARM64
- macOS, but only for Apple Silicon (ARM64)

You can find the ZIPs and TARs containing these executables (potentially including necessary libraries placed next to the executable) on the [Releases](https://github.com/boyninja1555/floorvm/releases) tab. Every time we push an update, though, it publishes a new release. If your release doesn't work, you can download an earlier one!

```bash
# Windows
.\FloorVM.exe programs\paint.from

# Linux/macOS
./FloorVM programs/paint.from
```
