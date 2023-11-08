#ifndef M68HC11_ASSEMBLER_H
#define M68HC11_ASSEMBLER_H

#include "addressingmode.h"
#include "m68hc11x.h"
#include <algorithm>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <sstream>
#include <map>
#include <functional>
#include <format>
#include <iomanip>

class CPUState {
public:
    union {
        union {
            u8 A;
            u8 B;
        };

        u16 D;
    };

    u16 IX;
    u16 IY;
    u16 SP;
    u16 PC;
    u8 Flags;
};

struct Operation {
    std::vector<u8> opcodes;
    u8 byteCount;
};

struct Instruction {
    using Ptr = std::shared_ptr<Instruction>;
    using OpcodeMap = std::unordered_map<Assembler_AddressingMode, Operation>;
    using ExecuteFn = std::function<void(CPUState &)>;

    std::string mnemonic;
    std::string description;
    OpcodeMap opcodes;
    ExecuteFn execute;

    bool IsAddressingModeSupported(Assembler_AddressingMode mode) const {
        return opcodes.count(mode);
    }

    static Ptr Create(std::string mnemonic, std::string description, OpcodeMap opcodes, ExecuteFn execute) {
        return std::make_shared<Instruction>(Instruction{
            std::move(mnemonic),
            std::move(description),
            std::move(opcodes),
            std::move(execute)
        });
    }

    static Ptr Create(std::string mnemonic, OpcodeMap opcodes) {
        return Create(mnemonic, "", opcodes);
    }

    static Ptr Create(std::string mnemonic, std::string description, OpcodeMap opcodes) {
        return Create(mnemonic, description, opcodes, [](CPUState&) {
            throw std::runtime_error("Unimplemented instruction!");
        });
    }
};

using InstructionRef = std::shared_ptr<const Instruction>;

inline std::vector<std::shared_ptr<Instruction>> AllInstructions = {
        Instruction::Create(
                "ORG",
                {
                        {Assembler_AddressingMode::EXTENDED, { {}, 0 }}
                }
        ),
        Instruction::Create(
                "ABA",
                "Add accumulators",
                {
                    { Assembler_AddressingMode::INHERENT, { { 0x1B }, 0 } }
                },
                [](CPUState &state) {
                    state.A = state.A + state.B;
                }
        ),
        Instruction::Create(
                "ABX",
                "Add B to X",
                {
                        { Assembler_AddressingMode::INHERENT, { { 0x3A }, 0 } }
                }
        ),
        Instruction::Create(
                "ABY",
                "Add B to Y",
                {
                        { Assembler_AddressingMode::INHERENT, { { 0x18, 0x3A }, 0 } }
                }
        ),
        Instruction::Create (
                "ADCA",
                "Add with Carry to A",
                {
                        { Assembler_AddressingMode::IMMEDIATE, { { 0x89 },          1 } },
                        { Assembler_AddressingMode::DIRECT,    { { 0x99 },          1 } },
                        { Assembler_AddressingMode::EXTENDED,  { { 0xB9 },          2 } },
                        { Assembler_AddressingMode::INDEXED_X, { { 0xA9 },          1 } },
                        { Assembler_AddressingMode::INDEXED_Y, { { 0x18, 0xA9 },    1 } },
                }
        ),
        Instruction::Create (
                "ADCB",
                "Add with Carry to B",
                {
                        { Assembler_AddressingMode::IMMEDIATE, { { 0xC9 },          1 } },
                        { Assembler_AddressingMode::DIRECT,    { { 0xD9 },          1 } },
                        { Assembler_AddressingMode::EXTENDED,  { { 0xF9 },          2 } },
                        { Assembler_AddressingMode::INDEXED_X, { { 0xE9 },          1 } },
                        { Assembler_AddressingMode::INDEXED_Y, { { 0x18, 0xE9 },    1 } },
                }
        ),
        Instruction::Create (
                "ADDA",
                "Add Memory to A",
                {
                        { Assembler_AddressingMode::IMMEDIATE, { { 0x8B },          1 } },
                        { Assembler_AddressingMode::DIRECT,    { { 0x9B },          1 } },
                        { Assembler_AddressingMode::EXTENDED,  { { 0xBB },          2 } },
                        { Assembler_AddressingMode::INDEXED_X, { { 0xAB },          1 } },
                        { Assembler_AddressingMode::INDEXED_Y, { { 0x18, 0xAB },    1 } },
                }
        ),
        Instruction::Create (
                "ADDB",
                "Add Memory to B",
                {
                        { Assembler_AddressingMode::IMMEDIATE, { { 0xCB },          1 } },
                        { Assembler_AddressingMode::DIRECT,    { { 0xDB },          1 } },
                        { Assembler_AddressingMode::EXTENDED,  { { 0xFB },          2 } },
                        { Assembler_AddressingMode::INDEXED_X, { { 0xEB },          1 } },
                        { Assembler_AddressingMode::INDEXED_Y, { { 0x18, 0xEB },    1 } },
                }
        ),
        Instruction::Create (
                "ADDD",
                "Add 16-Bit to D",
                {
                        { Assembler_AddressingMode::IMMEDIATE, { { 0xC3 },          2 } },
                        { Assembler_AddressingMode::DIRECT,    { { 0xD3 },          1 } },
                        { Assembler_AddressingMode::EXTENDED,  { { 0xF3 },          2 } },
                        { Assembler_AddressingMode::INDEXED_X, { { 0xE3 },          1 } },
                        { Assembler_AddressingMode::INDEXED_Y, { { 0x18, 0xE3 },    1 } },
                }
        ),
        Instruction::Create (
                "ANDA",
                "AND A with Memory",
                {
                        { Assembler_AddressingMode::IMMEDIATE, { { 0x84 },          1 } },
                        { Assembler_AddressingMode::DIRECT,    { { 0x94 },          1 } },
                        { Assembler_AddressingMode::EXTENDED,  { { 0xB4 },          2 } },
                        { Assembler_AddressingMode::INDEXED_X, { { 0xA4 },          1 } },
                        { Assembler_AddressingMode::INDEXED_Y, { { 0x18, 0xA4 },    1 } },
                }
        ),
        Instruction::Create (
                "ANDB",
                "AND B with Memory",
                {
                        { Assembler_AddressingMode::IMMEDIATE, { { 0xC4 },          1 } },
                        { Assembler_AddressingMode::DIRECT,    { { 0xD4 },          1 } },
                        { Assembler_AddressingMode::EXTENDED,  { { 0xF4 },          2 } },
                        { Assembler_AddressingMode::INDEXED_X, { { 0xE4 },          1 } },
                        { Assembler_AddressingMode::INDEXED_Y, { { 0x18, 0xE4 },    1 } },
                }
        ),
        Instruction::Create (
                "ASL",
                "Arithmetic Shift Left",
                {
                        { Assembler_AddressingMode::EXTENDED,   { { 0x78 },          2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { { 0x68 },          1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { { 0x18, 0x68 },    1 } },
                }
        ),
        Instruction::Create(
                "ASLA",
                "Arithmetic Shift Left A",
                {
                        { Assembler_AddressingMode::INHERENT, { {0x48}, 0 } }
                }
        ),
        Instruction::Create(
                "ASLB",
                "Arithmetic Shift Left B",
                {
                        { Assembler_AddressingMode::INHERENT, { {0x58}, 0 } }
                }
        ),
        Instruction::Create(
                "ASLD",
                "Arithmetic Shift Left D",
                {
                        { Assembler_AddressingMode::INHERENT, { {0x05}, 0 } }
                }
        ),
        Instruction::Create(
            "ASR",
            "Arithmetic Shift Right",
            {
                { Assembler_AddressingMode::EXTENDED,   { { 0x77 },         2 } },
                { Assembler_AddressingMode::INDEXED_X,  { { 0x67 },         1 } },
                { Assembler_AddressingMode::INDEXED_Y,  { { 0x18, 0x67 },   1 } }
            }
        ),
        Instruction::Create(
                "ASRA",
                "Arithmetic Shift Right A",
                {
                        { Assembler_AddressingMode::INHERENT, { {0x47}, 0 } }
                }
        ),
        Instruction::Create(
                "ASRB",
                "Arithmetic Shift Right B",
                {
                        { Assembler_AddressingMode::INHERENT, { {0x57}, 0 } }
                }
        ),
        Instruction::Create(
                "BCC",
                "Branch if Carry Clear",
                {
                        { Assembler_AddressingMode::RELATIVE, { {0x24}, 1 } }
                }
        ),
        Instruction::Create(
                "BCLR",
                "Clear Bit(s)",
                {
                        { Assembler_AddressingMode::DIRECT,     { {0x15}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0x1D}, 2 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0x1D}, 2 } },
                }
        ),
        Instruction::Create(
                "BCS",
                "Branch if Carry Set",
                {
                        { Assembler_AddressingMode::RELATIVE, { {0x25}, 1 } }
                }
        ),
        Instruction::Create(
                "BEQ",
                "Branch If Equal",
                {
                        { Assembler_AddressingMode::RELATIVE, { {0x27}, 1 } }
                }
        ),
        Instruction::Create(
                "BGE",
                "Branch If Greater Than or Equal (Signed)",
                {
                        { Assembler_AddressingMode::RELATIVE, { {0x2C}, 1 } }
                }
        ),
        Instruction::Create(
                "BGT",
                "Branch If Greater Than (Signed)",
                {
                        { Assembler_AddressingMode::RELATIVE, { {0x2E}, 1 } }
                }
        ),
        Instruction::Create(
                "BHI",
                "Branch if Higher (Unsigned)",
                {
                        { Assembler_AddressingMode::RELATIVE, { {0x22}, 1 } }
                }
        ),
        Instruction::Create(
                "BHS",
                "Branch if Higher or Same (Unsigned)",
                {
                        { Assembler_AddressingMode::RELATIVE, { {0x24}, 1 } }
                }
        ),
        Instruction::Create(
                "BITA",
                "Bit(s) Test A with Memory",
                {
                        { Assembler_AddressingMode::IMMEDIATE,  { {0x85}, 1 } },
                        { Assembler_AddressingMode::DIRECT,     { {0x95}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0xB5}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0xA5}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xA5}, 1 } },
                }
        ),
        Instruction::Create(
                "BITB",
                "Bit(s) Test B with Memory",
                {
                        { Assembler_AddressingMode::IMMEDIATE,  { {0xC5}, 1 } },
                        { Assembler_AddressingMode::DIRECT,     { {0xD5}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0xF5}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0xE5}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xE5}, 1 } },
                }
        ),
        Instruction::Create(
                "BLE",
                "Branch if Less Than or Equal (Signed)",
                {
                        { Assembler_AddressingMode::RELATIVE, { {0x2F}, 1 } }
                }
        ),
        Instruction::Create(
                "BLO",
                "Branch if Lower (Unsigned)",
                {
                        { Assembler_AddressingMode::RELATIVE, { {0x25}, 1 } }
                }
        ),
        Instruction::Create(
                "BLS",
                "Branch if Lower or Same (Unsigned)",
                {
                        { Assembler_AddressingMode::RELATIVE, { {0x23}, 1 } }
                }
        ),
        Instruction::Create(
                "BLT",
                "Branch if Less Than (Signed)",
                {
                        { Assembler_AddressingMode::RELATIVE, { {0x2D}, 1 } }
                }
        ),
        Instruction::Create(
                "BMI",
                "Branch if Minus",
                {
                        { Assembler_AddressingMode::RELATIVE, { {0x2B}, 1 } }
                }
        ),
        Instruction::Create(
                "BNE",
                "Branch if Not Equal",
                {
                        { Assembler_AddressingMode::RELATIVE, { {0x2B}, 1 } }
                }
        ),
        Instruction::Create(
                "BPL",
                "Branch if Plus",
                {
                        { Assembler_AddressingMode::RELATIVE, { {0x2A}, 1 } }
                }
        ),
        Instruction::Create(
                "BRA",
                "Branch Always",
                {
                        { Assembler_AddressingMode::RELATIVE, { {0x20}, 1 } }
                }
        ),
        Instruction::Create(
                "BRCLR",
                "Branch if Bit(s) Clear",
                {
                        { Assembler_AddressingMode::DIRECT,     { {0x13}, 3 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0x1F}, 3 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0x1F}, 3 } }
                }
        ),
        Instruction::Create(
                "BRN",
                "Branch Never", // NOTE(alex): Isn't this the exact same as NOP?
                {
                        { Assembler_AddressingMode::RELATIVE, { {0x21}, 1 } }
                }
        ),
        Instruction::Create(
                "BRSET",
                "Branch if Bit(s) Set",
                {
                        { Assembler_AddressingMode::DIRECT,     { {0x12}, 3 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0x1E}, 3 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0x1E}, 3 } }
                }
        ),
        Instruction::Create(
                "BSET",
                "Set Bit(s)",
                {
                        { Assembler_AddressingMode::DIRECT,     { {0x14}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0x1C}, 2 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0x1C}, 2 } }
                }
        ),
        Instruction::Create(
                "BSR",
                "Branch to Subroutine",
                {
                        { Assembler_AddressingMode::RELATIVE, { {0x8D}, 1 } }
                }
        ),
        Instruction::Create(
                "BVC",
                "Branch if Overflow Clear",
                {
                        { Assembler_AddressingMode::RELATIVE, { {0x28}, 1 } }
                }
        ),
        Instruction::Create(
                "BVS",
                "Branch if Overflow Set",
                {
                        { Assembler_AddressingMode::RELATIVE, { {0x29}, 1 } }
                }
        ),
        Instruction::Create(
                "CBA",
                "Compare A to B",
                {
                        { Assembler_AddressingMode::INHERENT, { {0x11}, 0 } }
                }
        ),
        Instruction::Create(
                "CLC",
                "Clear Carry Bit",
                {
                        { Assembler_AddressingMode::INHERENT, { {0x0C}, 0 } }
                }
        ),
        Instruction::Create(
                "CLI",
                "Clear Interrupt Mask",
                {
                        { Assembler_AddressingMode::INHERENT, { {0x0E}, 0 } }
                }
        ),
        Instruction::Create(
                "CLR",
                "Clear Memory Byte",
                {
                        { Assembler_AddressingMode::EXTENDED,   { {0x7F}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0x6F}, 2 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0x6F}, 2 } }
                }
        ),
        Instruction::Create(
                "CLRA",
                "Clear Accumulator A",
                {
                        { Assembler_AddressingMode::INHERENT, { {0x4F}, 0 } }
                }
        ),
        Instruction::Create(
                "CLRB",
                "Clear Accumulator B",
                {
                        { Assembler_AddressingMode::INHERENT, { {0x5F}, 0 } }
                }
        ),
        Instruction::Create(
                "CLV",
                "Clear Accumulator B",
                {
                        { Assembler_AddressingMode::INHERENT, { {0x0A}, 0 } }
                }
        ),
        Instruction::Create(
                "CMPA",
                "Compare A to Memory",
                {
                        { Assembler_AddressingMode::IMMEDIATE,  { {0x81}, 1 } },
                        { Assembler_AddressingMode::DIRECT,     { {0x91}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0xB1}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0xA1}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xA1}, 1 } }
                }
        ),
        Instruction::Create(
                "CMPB",
                "Compare B to Memory",
                {
                        { Assembler_AddressingMode::IMMEDIATE,  { {0xC1}, 1 } },
                        { Assembler_AddressingMode::DIRECT,     { {0xD1}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0xF1}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0xE1}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xE1}, 1 } }
                }
        ),
        Instruction::Create(
                "COM",
                "1's Complement Memory Byte",
                {
                        { Assembler_AddressingMode::EXTENDED,   { {0x73}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0x63}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0x63}, 1 } }
                }
        ),
        Instruction::Create(
                "COMA",
                "1's Complement A",
                {
                        { Assembler_AddressingMode::INHERENT, { {0x43}, 0 } }
                }
        ),
        Instruction::Create(
                "COMB",
                "1's Complement B",
                {
                        { Assembler_AddressingMode::INHERENT, { {0x53}, 0 } }
                }
        ),
        Instruction::Create(
                "CPD",
                "Compare D to Memory 16-Bit",
                {
                        { Assembler_AddressingMode::IMMEDIATE,  { {0x1A, 0x83}, 2 } },
                        { Assembler_AddressingMode::DIRECT,     { {0x1A, 0x93}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0x1A, 0xB3}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0x1A, 0xA3}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0xCD, 0xA3}, 1 } }
                }
        ),
        Instruction::Create(
                "CPX",
                "Compare X to Memory 16-Bit",
                {
                        { Assembler_AddressingMode::IMMEDIATE,  { {0x8C}, 2 } },
                        { Assembler_AddressingMode::DIRECT,     { {0x9C}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0xBC}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0xAC}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0xCD, 0xAC}, 1 } }
                }
        ),
        Instruction::Create(
                "CPY",
                "Compare Y to Memory 16-Bit",
                {
                        { Assembler_AddressingMode::IMMEDIATE,  { {0x18, 0x8C}, 2 } },
                        { Assembler_AddressingMode::DIRECT,     { {0x18, 0x9C}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0x18, 0xBC}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0x1A, 0xAC}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xAC}, 1 } }
                }
        ),
        Instruction::Create(
                "DAA",
                "Decimal Adjust A",
                {
                        { Assembler_AddressingMode::INHERENT,  { {0x19}, 0 } },
                }
        ),
        Instruction::Create(
                "DEC",
                "Decrement Memory Byte",
                {
                        { Assembler_AddressingMode::EXTENDED,   { {0x7A}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0x6A}, 2 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0x6A}, 2 } },
                }
        ),
        Instruction::Create(
                "DECA",
                "Decrement Accumulator A",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x4A}, 0 } },
                }
        ),
        Instruction::Create(
                "DECB",
                "Decrement Accumulator B",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x5A}, 0 } },
                }
        ),
        Instruction::Create(
                "DES",
                "Decrement Stack Pointer",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x34}, 0 } },
                }
        ),
        Instruction::Create(
                "DEX",
                "Decrement Index Register X",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x09}, 0 } },
                }
        ),
        Instruction::Create(
                "DEY",
                "Decrement Index Register Y",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x18, 0x09}, 0 } },
                }
        ),
        Instruction::Create(
                "EORA",
                "Exclusive OR A with Memory",
                {
                        { Assembler_AddressingMode::IMMEDIATE,  { {0x88}, 1 } },
                        { Assembler_AddressingMode::DIRECT,     { {0x98}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0xB8}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0xA8}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xA8}, 1 } }
                }
        ),
        Instruction::Create(
                "EORB",
                "Exclusive OR B with Memory",
                {
                        { Assembler_AddressingMode::IMMEDIATE,  { {0xC8}, 1 } },
                        { Assembler_AddressingMode::DIRECT,     { {0xD8}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0xF8}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0xE8}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xE8}, 1 } }
                }
        ),
        Instruction::Create(
                "FDIV",
                "Fractional Divide 16 by 16 (Unsigned)",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x03}, 0 } },
                }
        ),
        Instruction::Create(
                "IDIV",
                "Integer Divide by 16 by 16 (Unsigned)",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x02}, 0 } },
                }
        ),
        Instruction::Create(
                "INC",
                "Increase Memory Byte",
                {
                        { Assembler_AddressingMode::EXTENDED,   { {0x7C}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0x6C}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0x6C}, 1 } },
                }
        ),
        Instruction::Create(
                "INCA",
                "Increment Accumulator A",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x4C}, 0 } },
                }
        ),
        Instruction::Create(
                "INCB",
                "Increment Accumulator B",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x5C}, 0 } },
                }
        ),
        Instruction::Create(
                "INS",
                "Increment Stack Pointer",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x31}, 0 } },
                }
        ),
        Instruction::Create(
                "INX",
                "Increment Index Register X",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x08}, 0 } },
                }
        ),
        Instruction::Create(
                "INY",
                "Increment Index Register Y",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x18, 0x08}, 0 } },
                }
        ),
        Instruction::Create(
                "JMP",
                "Jump",
                {
                        { Assembler_AddressingMode::EXTENDED,   { {0x7E}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0x6E}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0x6E}, 1 } },
                }
        ),
        Instruction::Create(
                "JSR",
                "Jump to Subroutine",
                {
                        { Assembler_AddressingMode::DIRECT,     { {0x9D}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0x8D}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0xAD}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xAD}, 1 } },
                }
        ),
        Instruction::Create(
                "LDAA",
                "Load Accumulator A",
                {
                        { Assembler_AddressingMode::IMMEDIATE,  { {0x86}, 1 } },
                        { Assembler_AddressingMode::DIRECT,     { {0x96}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0xB6}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0xA6}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xA6}, 1 } },
                }
        ),
        Instruction::Create(
                "LDAB",
                "Load Accumulator B",
                {
                        { Assembler_AddressingMode::IMMEDIATE,  { {0xC6}, 1 } },
                        { Assembler_AddressingMode::DIRECT,     { {0xD6}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0xF6}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0xE6}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xE6}, 1 } },
                }
        ),
        Instruction::Create(
                "LDD",
                "Load Accumulator D",
                {
                        { Assembler_AddressingMode::IMMEDIATE,  { {0xCC}, 2 } },
                        { Assembler_AddressingMode::DIRECT,     { {0xDC}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0xFC}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0xEC}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xEC}, 1 } },
                }
        ),
        Instruction::Create(
                "LDS",
                "Load Stack Pointer",
                {
                        { Assembler_AddressingMode::IMMEDIATE,  { {0x8E}, 2 } },
                        { Assembler_AddressingMode::DIRECT,     { {0x9E}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0xBE}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0xAE}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xAE}, 1 } },
                }
        ),
        Instruction::Create(
                "LDX",
                "Load Index Register X",
                {
                        { Assembler_AddressingMode::IMMEDIATE,  { {0xCE}, 2 } },
                        { Assembler_AddressingMode::DIRECT,     { {0xDE}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0xFE}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0xEE}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xEE}, 1 } },
                }
        ),
        Instruction::Create(
                "LDY",
                "Load Index Register Y",
                {
                        { Assembler_AddressingMode::IMMEDIATE,  { {0x18, 0xCE}, 2 } },
                        { Assembler_AddressingMode::DIRECT,     { {0x18, 0xDE}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0x18, 0xFE}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0x1A, 0xEE}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xEE}, 1 } },
                }
        ),
        Instruction::Create(
                "LSL",
                "Logical Shift Left",
                {
                        { Assembler_AddressingMode::EXTENDED,   { {0x78}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0x68}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0x68}, 1 } },
                }
        ),
        Instruction::Create(
                "LSLA",
                "Logical Shift Left A",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x48}, 0 } },
                }
        ),
        Instruction::Create(
                "LSLB",
                "Logical Shift Left B",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x58}, 0 } },
                }
        ),
        Instruction::Create(
                "LSLD",
                "Logical Shift Left Double",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x05}, 0 } },
                }
        ),
        Instruction::Create(
                "LSR",
                "Logical Shift Right",
                {
                        { Assembler_AddressingMode::EXTENDED,   { {0x74}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0x64}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0x64}, 1 } },
                }
        ),
        Instruction::Create(
                "LSRA",
                "Logical Shift Right A",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x44}, 0 } },
                }
        ),
        Instruction::Create(
                "LSRB",
                "Logical Shift Right B",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x54}, 0 } },
                }
        ),
        Instruction::Create(
                "LSRD",
                "Logical Shift Right Double",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x04}, 0 } },
                }
        ),
        Instruction::Create(
                "MUL",
                "Multiply 8 by 8",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x3D}, 0 } },
                }
        ),
        Instruction::Create(
                "NEG",
                "2's Complement Memory Byte",
                {
                        { Assembler_AddressingMode::EXTENDED,   { {0x70}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0x60}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0x60}, 1 } },
                }
        ),
        Instruction::Create(
                "NEGA",
                "2's Complement A",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x40}, 0 } },
                }
        ),
        Instruction::Create(
                "NEGB",
                "2's Complement B",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x50}, 0 } },
                }
        ),
        Instruction::Create(
                "NOP",
                "No Operation",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x01}, 0 } },
                }
        ),
        Instruction::Create(
                "ORAA",
                "OR Accumulator A (Inclusive)",
                {
                        { Assembler_AddressingMode::IMMEDIATE,  { {0x8A}, 1 } },
                        { Assembler_AddressingMode::DIRECT,     { {0x9A}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0xBA}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0xAA}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xAA}, 1 } },
                }
        ),
        Instruction::Create(
                "ORAB",
                "OR Accumulator B (Inclusive)",
                {
                        { Assembler_AddressingMode::IMMEDIATE,  { {0xCA}, 1 } },
                        { Assembler_AddressingMode::DIRECT,     { {0xDA}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0xFA}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0xEA}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xEA}, 1 } },
                }
        ),
        Instruction::Create(
                "PSHA",
                "Push A onto Stack",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x36}, 0 } },
                }
        ),
        Instruction::Create(
                "PSHB",
                "Push B onto Stack",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x37}, 0 } },
                }
        ),
        Instruction::Create(
                "PSHX",
                "Push X onto Stack",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x3C}, 0 } },
                }
        ),
        Instruction::Create(
                "PSHY",
                "Push Y onto Stack",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x18, 0x3C}, 0 } },
                }
        ),
        Instruction::Create(
                "PULA",
                "Pull A onto Stack",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x32}, 0 } },
                }
        ),
        Instruction::Create(
                "PULB",
                "Pull B onto Stack",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x33}, 0 } },
                }
        ),
        Instruction::Create(
                "PULX",
                "Pull X onto Stack",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x38}, 0 } },
                }
        ),
        Instruction::Create(
                "PULY",
                "Pull Y onto Stack",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x18, 0x38}, 0 } },
                }
        ),
        Instruction::Create(
                "ROL",
                "Rotate Left",
                {
                        { Assembler_AddressingMode::EXTENDED,   { {0x79}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0x69}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0x69}, 1 } },
                }
        ),
        Instruction::Create(
                "ROLA",
                "Rotate Left A",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x49}, 0 } },
                }
        ),
        Instruction::Create(
                "ROLB",
                "Rotate Left B",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x59}, 0 } },
                }
        ),
        Instruction::Create(
                "ROR",
                "Rotate Right",
                {
                        { Assembler_AddressingMode::EXTENDED,   { {0x76}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0x66}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0x66}, 1 } }
                }
        ),
        Instruction::Create(
                "RORA",
                "Rotate Right A",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x46}, 0 } },
                }
        ),
        Instruction::Create(
                "RORB",
                "Rotate Right B",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x56}, 0 } },
                }
        ),
        Instruction::Create(
                "RTI",
                "Return from Interrupt",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x3B}, 0 } },
                }
        ),
        Instruction::Create(
                "RTS",
                "Return from Subroutine",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x39}, 0 } },
                }
        ),
        Instruction::Create(
                "SBA",
                "Subtract B from A",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x10}, 0 } },
                }
        ),
        Instruction::Create(
                "SBCA",
                "Subtract with Carry from A",
                {
                        { Assembler_AddressingMode::IMMEDIATE,  { {0x82}, 1 } },
                        { Assembler_AddressingMode::DIRECT,     { {0x92}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0xB2}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0xA2}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xA2}, 1 } },
                }
        ),
        Instruction::Create(
                "SBCB",
                "Subtract with Carry from B",
                {
                        { Assembler_AddressingMode::IMMEDIATE,  { {0xC2}, 1 } },
                        { Assembler_AddressingMode::DIRECT,     { {0xD2}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0xF2}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0xE2}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xE2}, 1 } },
                }
        ),
        Instruction::Create(
                "SEC",
                "Set Carry",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x0D}, 0 } },
                }
        ),
        Instruction::Create(
                "SEI",
                "Set Interrupt Mask",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x0F}, 0 } },
                }
        ),
        Instruction::Create(
                "SEV",
                "Set Overflow Flag",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x0B}, 0 } },
                }
        ),
        Instruction::Create(
                "STAA",
                "Store Accumulator A",
                {
                        { Assembler_AddressingMode::DIRECT,     { {0x97}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0xB7}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0xA7}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xA7}, 1 } },
                }
        ),
        Instruction::Create(
                "STAB",
                "Store Accumulator B",
                {
                        { Assembler_AddressingMode::DIRECT,     { {0xD7}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0xF7}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0xE7}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xE7}, 1 } },
                }
        ),
        Instruction::Create(
                "STD",
                "Store Accumulator D",
                {
                        { Assembler_AddressingMode::DIRECT,     { {0xDD}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0xFD}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0xED}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xED}, 1 } },
                }
        ),
        Instruction::Create(
                "STOP",
                "Stop Internal Clocks",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0xCF}, 0 } },
                }
        ),
        Instruction::Create(
                "STS",
                "Store Stack Pointer",
                {
                        { Assembler_AddressingMode::DIRECT,     { {0x9F}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0xBF}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0xAF}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xAF}, 1 } },
                }
        ),
        Instruction::Create(
                "STX",
                "Store Index Register X",
                {
                        { Assembler_AddressingMode::DIRECT,     { {0xDF}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0xFF}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0xEF}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xEF}, 1 } },
                }
        ),
        Instruction::Create(
                "STY",
                "Store Index Register Y",
                {
                        { Assembler_AddressingMode::DIRECT,     { {0x18, 0x9F}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0x18, 0xBF}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0x1A, 0xAF}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xAF}, 1 } },
                }
        ),
        Instruction::Create(
                "SUBA",
                "Subtract Memory from A",
                {
                        { Assembler_AddressingMode::IMMEDIATE,  { {0x80}, 1 } },
                        { Assembler_AddressingMode::DIRECT,     { {0x90}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0xB0}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0xA0}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xA0}, 1 } },
                }
        ),
        Instruction::Create(
                "SUBB",
                "Subtract Memory from B",
                {
                        { Assembler_AddressingMode::IMMEDIATE,  { {0xC0}, 1 } },
                        { Assembler_AddressingMode::DIRECT,     { {0xD0}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0xF0}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0xE0}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xE0}, 1 } },
                }
        ),
        Instruction::Create(
                "SUBD",
                "Subtract Memory from D",
                {
                        { Assembler_AddressingMode::IMMEDIATE,  { {0x83}, 2 } },
                        { Assembler_AddressingMode::DIRECT,     { {0x93}, 1 } },
                        { Assembler_AddressingMode::EXTENDED,   { {0xB3}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0xA3}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0xA3}, 1 } },
                }
        ),
        Instruction::Create(
                "SWI",
                "Software Interrupt",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x3F}, 0 } },
                }
        ),
        Instruction::Create(
                "TAB",
                "Transfer A to B",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x16}, 0 } },
                }
        ),
        Instruction::Create(
                "TAP",
                "Transfer A to CC Register",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x06}, 0 } },
                }
        ),
        Instruction::Create(
                "TBA",
                "Transfer B to A",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x17}, 0 } },
                }
        ),
        Instruction::Create(
                "TEST",
                "TEST (Only in Test Modes)",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x00}, 0 } },
                }
        ),
        Instruction::Create(
                "TPA",
                "Transfer CC Register to A",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x07}, 0 } },
                }
        ),
        Instruction::Create(
                "TST",
                "Test Memory",
                {
                        { Assembler_AddressingMode::EXTENDED,   { {0x7D}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0x6D}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0x6D}, 1 } },
                }
        ),
        Instruction::Create(
                "TSTA",
                "Test Accumulator A",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x4D}, 0 } },
                }
        ),
        Instruction::Create(
                "TSTB",
                "Test Accumulator B",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x5D}, 0 } },
                }
        ),
        Instruction::Create(
                "TSX",
                "Transfer Stack Pointer to X",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x30}, 0 } },
                }
        ),
        Instruction::Create(
                "TSY",
                "Transfer Stack Pointer to Y",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x18, 0x30}, 0 } },
                }
        ),
        Instruction::Create(
                "TXS",
                "Transfer X to Stack Pointer",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x35}, 0 } },
                }
        ),
        Instruction::Create(
                "TYS",
                "Transfer Y to Stack Pointer",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x18, 0x35}, 0 } },
                }
        ),
        Instruction::Create(
                "WAI",
                "Wait for Interrupt",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x3E}, 0 } },
                }
        ),
        Instruction::Create(
                "XGDX",
                "Exchange D with X",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x8F}, 0 } },
                }
        ),
        Instruction::Create(
                "XGDY",
                "Exchange D with Y",
                {
                        { Assembler_AddressingMode::INHERENT,   { {0x18, 0x8F}, 0 } },
                }
        ),
};

inline InstructionRef OrgInst = AllInstructions.front();

inline InstructionRef GetInstructionByMnemonic(const std::string &mnemonic) {
    // TODO(alex): replace with hash map for faster lookup
    auto inst = std::find_if(AllInstructions.begin(), AllInstructions.end(), [&mnemonic](const std::shared_ptr<Instruction>& instruction) {
        return instruction->mnemonic == mnemonic;
    });

    if (inst == AllInstructions.end()) {
        return nullptr;
    }

    return *inst;
}

class Row {
public:
    std::string label;
    std::string raw;
    InstructionRef instruction;
    std::vector<std::string> tokens;

    Assembler_AddressingMode mode;
    std::string referencedLabel;
    std::vector<u8> operand;
    std::vector<u8> assembled;
    u16 offset;

    [[nodiscard]] std::string str(const size_t minWidth = 8) const {
        std::stringstream out;
        std::string bytes;

        bytes.append(std::format("{:04x}: ", offset - assembled.size()));

        for (const u8 opcode : assembled)
            bytes.append(std::format("{:02x} ", opcode));

        out << std::left << std::setw(minWidth*2) << bytes << std::setw(minWidth);

        for (const auto& token : tokens)
            out << token;

        return out.str();
    }
};

inline bool IsStringNumber(const std::string &s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

class Assembler {
public:
    void Assemble(std::stringstream& str) {
        for (std::string line; std::getline(str, line, '\n');) {
            lines.emplace_back(AssembleSingleLine(line));
        }

        ResolveBranches();
    }

    void Reset() {
        lines.clear();
        labelAddresses.clear();
    }

    void ResolveBranches() {
        for (auto& row : lines) {
            if (row.mode != Assembler_AddressingMode::RELATIVE)
                continue;

            u16 branchOffset = 0;

            try {
                branchOffset = labelAddresses.at(row.referencedLabel);
            } catch (std::out_of_range& e) {
                throw std::runtime_error("Invalid label");
            }

            i16 offset = branchOffset - row.offset;

            if (offset < -127 || offset > 127) {
                throw std::runtime_error("Branch out of range");
            }

            row.assembled.back() = offset;
        }
    }

    Row AssembleSingleLine(const std::string& str) {
        Row row = {};
        row.raw = str;

        std::stringstream ss(str);
        std::string token;
        while (ss >> token) {
            if (token.empty())
                continue;

            row.tokens.emplace_back(token);
        }

        if (!lines.empty()) {
            row.offset = lines.back().offset;
        }

        if (row.tokens.empty())
            return row;

        std::string instruction = row.tokens[0];
        row.instruction = GetInstructionByMnemonic(row.tokens[0]);

        // NOTE(alex): columns beginning with '*' are comments
        if (instruction.front() == '*')
            return row;

        // NOTE(alex): if the first column does not contain an instruction, it is a label
        if (!row.instruction) {
            if (instruction.front() == '#' || instruction.front() == '$' || isdigit(instruction.front())) {
                throw std::runtime_error("Invalid label name");
            }

            row.label = instruction;
            row.instruction = GetInstructionByMnemonic(row.tokens[1]);
        }

        if (!row.instruction) {
            throw std::runtime_error("Invalid instruction mnemonic");
        }

        row.mode = Assembler_AddressingMode::INHERENT;

        // NOTE(alex): if the column has a label, the operand is the third token rather than the second
        if (row.tokens.size() > (row.label.empty() ? 1 : 2)) {
            std::string &operand = row.tokens[row.label.empty() ? 1 : 2];

            if (!IsStringNumber(operand)
                && !isdigit(operand.front())
                && operand.front() != '#'
                && operand.front() != '$'
                && row.instruction->IsAddressingModeSupported(Assembler_AddressingMode::RELATIVE)) {
                row.mode = Assembler_AddressingMode::RELATIVE;
            } else {
                switch (operand.front()) {
                    case '#':
                        row.mode = Assembler_AddressingMode::IMMEDIATE;
                        operand.erase(0, 1);
                        break;
                    case '$':
                        row.mode = Assembler_AddressingMode::EXTENDED;
                        break;
                    default:
                        row.mode = Assembler_AddressingMode::DIRECT;
                        break;
                }

                switch (operand.back()) {
                    case 'X':
                        row.mode = Assembler_AddressingMode::INDEXED_X;
                        break;
                    case 'Y':
                        row.mode = Assembler_AddressingMode::INDEXED_Y;
                        break;
                }
            }
        }

        try {
            const Operation &operation = row.instruction->opcodes.at(row.mode);
            row.assembled.insert(row.assembled.begin(), operation.opcodes.begin(), operation.opcodes.end());

            if (row.mode != Assembler_AddressingMode::INHERENT && row.mode != Assembler_AddressingMode::RELATIVE) {
                // NOTE(alex): if the column has a label, the operand is the third token rather than the second
                std::string operand = row.tokens[row.label.empty() ? 1 : 2];

                u16 value = 0;
                // NOTE(alex): hex values begin with $
                if (operand.front() == '$') {
                    operand.erase(0, 1);
                    value = std::stoul(operand, nullptr, 16);
                } else {
                    value = std::stoul(operand, nullptr, 10);
                }

                if (row.instruction == OrgInst) {
                    row.offset = value;
                } else {
                    switch (operation.byteCount) {
                        case 1:
                            row.assembled.emplace_back(value);
                            break;
                        case 2:
                            row.assembled.emplace_back(value >> 8);
                            row.assembled.emplace_back(value);
                            break;
                    }
                }
            } else if (row.mode == Assembler_AddressingMode::RELATIVE) {
                // NOTE(alex): dummy byte to be replaced at the branch pass
                row.assembled.emplace_back();

                std::string referencedLabel = row.tokens[row.label.empty() ? 1 : 2];
                row.referencedLabel = referencedLabel;
            }

            if (!row.label.empty())
                labelAddresses.insert_or_assign(row.label, row.offset);
        } catch (std::out_of_range& e) {
            throw std::runtime_error("Invalid addressing mode");
        }

        if (!lines.empty())
            row.offset += row.assembled.size();

        return row;
    }

    std::unordered_map<std::string, u16> labelAddresses;
    std::stringstream stream;
    std::vector<Row> lines;
};

#endif //M68HC11_ASSEMBLER_H
