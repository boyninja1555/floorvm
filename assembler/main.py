from pathlib import Path
import struct
import re
import sys
import ast


PROGRAM_SIZE = 8192


OPCODES = {
    # Control
    "HALT": 0x00,
    "JMP": 0x01,
    "JMPIZ": 0x02,
    "JMPNZ": 0x03,
    # Memory
    "LOAD32": 0x10,
    "LOAD8": 0x11,
    "LOAD32P": 0x12,
    "LOAD8P": 0x13,
    "STORE32": 0x14,
    "STORE8": 0x15,
    "STORE32P": 0x16,
    "STORE8P": 0x17,
    "SET32": 0x18,
    "SET8": 0x19,
    "COPY": 0x1A,
    # ALU
    "ADD": 0x20,
    "SUB": 0x21,
    "MUL": 0x22,
    "DIV": 0x23,
    "AND": 0x24,
    "OR": 0x25,
    "XOR": 0x26,
    "NOT": 0x27,
    "SHL": 0x28,
    "SHR": 0x29,
}


INSTRUCTION_SIZE = {
    # Control
    "HALT": 0,
    "JMP": 4,
    "JMPIZ": 5,
    "JMPNZ": 5,
    # Memory
    "LOAD32": 5,
    "LOAD8": 5,
    "LOAD32P": 2,
    "LOAD8P": 2,
    "STORE32": 5,
    "STORE8": 5,
    "STORE32P": 2,
    "STORE8P": 2,
    "SET32": 5,
    "SET8": 2,
    "COPY": 2,
    # ALU
    "ADD": 3,
    "SUB": 3,
    "MUL": 3,
    "DIV": 3,
    "AND": 3,
    "OR": 3,
    "XOR": 3,
    "NOT": 3,
    "SHL": 3,
    "SHR": 3,
}


def preprocess(source: str) -> list[str]:
    defines: dict[str, str] = {}
    lines: list[str] = []
    for line in source.splitlines():
        line = line.split(";")[0].strip()
        if not line:
            continue

        if line.startswith("#define"):
            _, key, value = line.split()
            defines[key] = value
            continue

        for key, value in defines.items():
            line = re.sub(rf"\$\{{{key}\}}", value, line)

        lines.append(line)

    return lines


def parse_number(value: str) -> int:
    return int(value.removeprefix("r"), 0)


def write_u32(output: bytearray, value: int):
    output.extend(struct.pack(">I", value & 0xFFFFFFFF))


def resolve(value: str, labels: dict[str, int]) -> int:
    if value in labels:
        return labels[value]

    try:
        return parse_number(value)
    except ValueError:
        print(f"Label {value} did not resolve!")
        sys.exit(1)


def assemble(source: str) -> bytes:
    lines = preprocess(source)
    labels: dict[str, int] = {}
    position = 0
    for line in lines:
        if line.endswith(":"):
            labels[line[:-1]] = position
            continue

        parts = line.split(maxsplit=1)
        opcode = parts[0].upper()
        if opcode in OPCODES:
            position += 1 + INSTRUCTION_SIZE[opcode]
        elif opcode == ".BYTE":
            values = parts[1].split()
            position += len(values)
        elif opcode == ".HEX":
            hex_string = parts[1].replace(" ", "")
            if len(hex_string) % 2 != 0:
                print("Directive .hex requires an even number of hex digits!")
                sys.exit(1)

            position += len(hex_string) // 2
        elif opcode == ".UTF8":
            text = ast.literal_eval(parts[1])
            position += len(text.encode("utf-8"))
        elif opcode == ".LUTF8":
            text = ast.literal_eval(parts[1])
            position += 4 + len(text.encode("utf-8"))
        else:
            print(f"Unknown opcode/directive {opcode}!")
            sys.exit(1)

    output = bytearray()
    for line in lines:
        if line.endswith(":"):
            continue

        parts = line.split(maxsplit=1)
        opcode = parts[0].upper()
        args = parts[1].split() if len(parts) > 1 else []
        if opcode in OPCODES:
            output.append(OPCODES[opcode])
            if opcode == "JMP":
                write_u32(output, resolve(args[0], labels))
            elif opcode in ("JMPIZ", "JMPNZ"):
                write_u32(output, resolve(args[0], labels))
                output.append(parse_number(args[1]) & 0xFF)
            elif opcode == "SET32":
                output.append(parse_number(args[0]) & 0xFF)
                write_u32(output, parse_number(args[1]))
            elif opcode in ("SET8", "COPY"):
                output.append(parse_number(args[0]) & 0xFF)
                output.append(parse_number(args[1]) & 0xFF)
            elif opcode in ("LOAD32", "LOAD8"):
                write_u32(output, resolve(args[0], labels))
                output.append(parse_number(args[1]) & 0xFF)
            elif opcode in ("STORE32", "STORE8"):
                output.append(parse_number(args[0]) & 0xFF)
                write_u32(output, resolve(args[1], labels))
            elif opcode in ("LOAD32P", "LOAD8P", "STORE32P", "STORE8P"):
                output.append(parse_number(args[0]) & 0xFF)
                output.append(parse_number(args[1]) & 0xFF)
            elif opcode in (
                "ADD",
                "SUB",
                "MUL",
                "DIV",
                "AND",
                "OR",
                "XOR",
                "NOT",
                "SHL",
                "SHR",
            ):
                for arg in args:
                    output.append(parse_number(arg) & 0xFF)
        elif opcode == ".BYTE":
            for value in args:
                output.append(parse_number(value) & 0xFF)
        elif opcode == ".HEX":
            output.extend(bytes.fromhex(parts[1]))
        elif opcode == ".UTF8":
            text = ast.literal_eval(parts[1])
            output.extend(text.encode("utf-8"))
        elif opcode == ".LUTF8":
            text = ast.literal_eval(parts[1])
            encoded = text.encode("utf-8")
            write_u32(output, len(encoded))
            output.extend(encoded)
        else:
            print(f"Unknown opcode/directive {opcode}!")
            sys.exit(1)

    return bytes(output).ljust(PROGRAM_SIZE, b"\0")


def main():
    if len(sys.argv) != 3:
        print("Please specify input and output files!")
        sys.exit(1)

    input_path = Path(sys.argv[1]).resolve()
    output_path = Path(sys.argv[2]).resolve()
    try:
        with open(input_path, "r") as input_file:
            with open(output_path, "wb") as output_file:
                output_file.write(assemble(input_file.read()))
                print(f"{output_path} is ready!")
    except FileNotFoundError:
        print("Input file does not exist!")
        sys.exit(1)


if __name__ == "__main__":
    main()
