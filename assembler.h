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

    bool IsAddressingModeSupported(Assembler_AddressingMode mode) {
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
                        { Assembler_AddressingMode::IMMEDIATE, { {0x85}, 1 } },
                        { Assembler_AddressingMode::DIRECT, { {0x95}, 1 } },
                        { Assembler_AddressingMode::EXTENDED, { {0xB5}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X, { {0xA5}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y, { {0x18, 0xA5}, 1 } },
                }
        ),
        Instruction::Create(
                "BITB",
                "Bit(s) Test B with Memory",
                {
                        { Assembler_AddressingMode::IMMEDIATE, { {0xC5}, 1 } },
                        { Assembler_AddressingMode::DIRECT, { {0xD5}, 1 } },
                        { Assembler_AddressingMode::EXTENDED, { {0xF5}, 2 } },
                        { Assembler_AddressingMode::INDEXED_X, { {0xE5}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y, { {0x18, 0xE5}, 1 } },
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
                        { Assembler_AddressingMode::RELATIVE, { {0x2A}, 1 } }
                }
        ),
        Instruction::Create(
                "BRCLR",
                "Branch if Bit(s) Clear",
                {
                        { Assembler_AddressingMode::DIRECT,     { {0x13}, 1 } },
                        { Assembler_AddressingMode::INDEXED_X,  { {0x1F}, 1 } },
                        { Assembler_AddressingMode::INDEXED_Y,  { {0x18, 0x1F}, 1 } }
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
                "BRN",
                "Branch Never", // NOTE(alex): Isn't this the exact same as NOP?
                {
                        { Assembler_AddressingMode::RELATIVE, { {0x21}, 1 } }
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

class Column {
public:
    std::string label;
    std::string raw;
    InstructionRef instruction;

    Assembler_AddressingMode mode;
    std::vector<u8> operand;
    std::vector<u8> assembled;
    u32 offset;
};

inline std::unordered_map<std::string, u16> labelAddresses;

inline bool IsStringNumber(const std::string &s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

class Assembler {
public:
    void Assemble(std::stringstream& str) {
        for (std::string line; std::getline(str, line, '\n');) {
            lines.emplace_back(AssembleSingleLine(line));
        }
    }

    Column* GetColumnByLabel(const std::string& label) {
        auto column = std::find_if(lines.begin(), lines.end(), [&label](const Column& col) {
            return label == col.label;
        });

        if (column == lines.end())
            return nullptr;

        return &*column;
    }

    void Reset() {
        lines.clear();
    }

    Column AssembleSingleLine(const std::string& str) {
        Column col = {};
        col.raw = str;

        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string token;
        while (ss >> token) {
            tokens.emplace_back(token);
        }

        if (!lines.empty()) {
            col.offset = lines.back().offset;
        }

        if (tokens.empty())
            return col;

        std::string instruction = tokens[0];
        col.instruction = GetInstructionByMnemonic(tokens[0]);

        // NOTE(alex): if the first column does not contain an instruction, it is a label
        if (!col.instruction) {
            if (instruction.front() == '#' || instruction.front() == '$' || isdigit(instruction.front())) {
                throw std::runtime_error("Invalid label name");
            }

            col.label = instruction;
            col.instruction = GetInstructionByMnemonic(tokens[1]);
        }

        if (!col.instruction) {
            throw std::runtime_error("Invalid instruction mnemonic");
        }

        col.mode = Assembler_AddressingMode::INHERENT;

        // NOTE(alex): if the column has a label, the operand is third token rather than the second
        if (tokens.size() > (col.label.empty() ? 1 : 2)) {
            std::string &operand = tokens[col.label.empty() ? 1 : 2];

            if (!IsStringNumber(operand)
                && !isdigit(operand.front())
                && operand.front() != '#'
                && operand.front() != '$') {
                col.mode = Assembler_AddressingMode::RELATIVE;
            } else {
                switch (operand.front()) {
                    case '#':
                        col.mode = Assembler_AddressingMode::IMMEDIATE;
                        operand.erase(0, 1);
                        break;
                    case '$':
                        col.mode = Assembler_AddressingMode::EXTENDED;
                        break;
                    default:
                        col.mode = Assembler_AddressingMode::DIRECT;
                        break;
                }

                switch (operand.back()) {
                    case 'X':
                        col.mode = Assembler_AddressingMode::INDEXED_X;
                        break;
                    case 'Y':
                        col.mode = Assembler_AddressingMode::INDEXED_Y;
                        break;
                }
            }
        }

        try {
            const Operation &operation = col.instruction->opcodes.at(col.mode);
            col.assembled.insert(col.assembled.begin(), operation.opcodes.begin(), operation.opcodes.end());

            if (col.mode != Assembler_AddressingMode::INHERENT && col.mode != Assembler_AddressingMode::RELATIVE) {
                // NOTE(alex): if the column has a label, the operand is third token rather than the second
                std::string operand = tokens[col.label.empty() ? 1 : 2];

                u16 value = 0;
                // NOTE(alex): hex values begin with $
                if (operand.front() == '$') {
                    operand.erase(0, 1);
                    value = std::stoul(operand, nullptr, 16);
                } else {
                    value = std::stoul(operand, nullptr, 10);
                }

                if (col.instruction == OrgInst) {
                    col.offset = value;
                } else {
                    switch (operation.byteCount) {
                        case 1:
                            col.assembled.emplace_back(value);
                            break;
                        case 2:
                            col.assembled.emplace_back(value >> 8);
                            col.assembled.emplace_back(value);
                            break;
                    }
                }
            } else if (col.mode == Assembler_AddressingMode::RELATIVE) {
                // NOTE(alex): dummy byte to be replaced at the branch pass
                col.assembled.emplace_back(0xFF);

                labelAddresses.insert_or_assign(col.label, col.offset);
            }
        } catch (std::out_of_range& e) {
            throw std::runtime_error("Invalid addressing mode");
        }

        if (!lines.empty())
            col.offset += col.assembled.size();

        return col;
    }

    std::stringstream stream;
    std::vector<Column> lines;
};

#endif //M68HC11_ASSEMBLER_H
