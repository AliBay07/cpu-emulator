#ifndef CPU_H
#define CPU_H

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iomanip>

using Byte = unsigned char;  // 8 bits
using Word = unsigned short; // 16 bits

using u32 = unsigned int; // 4 Bytes (unsigned)
using s32 = signed int; // 4 Bytes (signed)

// Structure representing memory
struct Memory {
    static constexpr u32 MAX_MEM = 1024 * 64;
    Byte Data[MAX_MEM];

    // Function which will intialise the memory
    void init() {
        for (unsigned char & i : Data) {
            i = 0;
        }
    }

    // Function which will read one byte from the memory
    Byte operator[](u32 address) const {
        // assert here address is < MAX_MEM
        return Data[address];
    }

    // Function which will write one byte in the memory
    Byte &operator[](u32 address) {
        // assert here address is < MAX_MEM
        return Data[address];
    }

    // Function which will write one word in the memory
    void writeWord(Word value, u32 address, s32 &cycles) {
        Data[address] = value & 0xFF;
        Data[address + 1] = (value >> 8);
        cycles -= 2;
    }
};

// Structure representing the 6502 CPU
struct CPU {
    // Registers

    Word PC; // Program Counter
    Byte SP; // Stack Pointer

    Byte A; // Accumulator register
    Byte X; // Index X register
    Byte Y; // Index Y register

    // Processor Status (1 byte containing 8 bits representing 8 different flags)

    Byte C: 1; // Carry Flag
    Byte Z: 1; // Zero Flag
    Byte I: 1; // Interrupt Disable
    Byte D: 1; // Decimal Mode
    Byte B: 1; // Break Command
    Byte V: 1; // Overflow Flag
    Byte N: 1; // Negative Flag

    // Instructions
    static constexpr Byte INS_LDA_IM = 0XA9;   // Load Accumulator - immediate
    static constexpr Byte INS_LDA_ZP = 0XA5;   // Load Accumulator - zero page
    static constexpr Byte INS_LDA_ZPX = 0XB5;  // Load Accumulator - zero page x
    static constexpr Byte INS_LDA_ABS = 0xAD;  // Load Accumulator - absolute
    static constexpr Byte INS_LDA_ABSX = 0xBD; // Load Accumulator - absolute x
    static constexpr Byte INS_LDA_ABSY = 0xB9; // Load Accumulator - absolute y
    static constexpr Byte INS_LDA_INDX = 0xA1; // Load Accumulator - indirect x
    static constexpr Byte INS_LDA_INDY = 0xB1; // Load Accumulator - indirect y

    static constexpr Byte INS_JSR = 0x20; // Jump to Subroutine - Absolute

    // Function which will reset the CPU registers
    void reset(Memory &memory) {
        PC = 0xFFFC;
        SP = 0xFF;
        C = Z = I = D = B = V = N = 0;
        A = X = Y = 0;
        memory.init();
    }

    // Function which will fetch one byte from where the program counter is pointing to
    Byte fetchByte(s32 &cycles, Memory &memory) {
        Byte data = memory[PC];
        PC++;
        cycles--;
        return data;
    }

    // Function which will fetch one word from where the program counter is pointing to
    Word fetchWord(s32 &cycles, Memory &memory) {
        Word data = memory[PC];
        PC++;
        data |= (memory[PC] << 8);
        PC++;
        cycles -= 2;

        return data;
    }

    // Function which will read one byte from an address in the memory
    static Byte readByte(s32 &cycles, Word address, Memory &memory) {
        Byte data = memory[address];
        cycles--;
        return data;
    }

    // Function which will read one word from an address in the memory
    static Word readWord(s32 &cycles, Word address, Memory &memory) {
        Byte loByte = readByte(cycles, address, memory);
        Byte hiByte = readByte(cycles, address + 1, memory);
        return loByte | (hiByte << 8);
    }

    // Function which will set the flag values after an LDA instruction
    void LDASetStatus() {
        Z = (A == 0);
        N = (A & 0b10000000) > 0;
    }

    // Function which will execute instructions, it will also return the number of cycles used
    s32 execute(s32 cycles, Memory &memory) {

        const s32 cyclesRequested = cycles;

        while (cycles > 0) {
            Byte instruction = fetchByte(cycles, memory);

            switch (instruction) {
                case INS_LDA_IM: {
                    Byte value = fetchByte(cycles, memory);
                    A = value;
                    LDASetStatus();
                    break;
                }
                case INS_LDA_ZP: {
                    Byte zeroPageAddress = fetchByte(cycles, memory);
                    A = readByte(cycles, zeroPageAddress, memory);
                    LDASetStatus();
                    break;
                }
                case INS_LDA_ZPX: {
                    Byte zeroPageAddress = fetchByte(cycles, memory);
                    zeroPageAddress += X;
                    cycles--;
                    A = readByte(cycles, zeroPageAddress, memory);
                    LDASetStatus();
                    break;
                }
                case INS_LDA_ABS: {
                    Word absAddress = fetchWord(cycles, memory);
                    A = readByte(cycles, absAddress, memory);
                    break;
                }
                case INS_LDA_ABSX: {
                    Word absAddress = fetchWord(cycles, memory);
                    Word absAddressX = absAddress + X;
                    A = readByte(cycles, absAddressX, memory);
                    if (absAddressX - absAddress >= 0xFF) // we crossed a page boundary
                    {
                        cycles--;
                    }
                    break;
                }
                case INS_LDA_ABSY: {
                    Word absAddress = fetchWord(cycles, memory);
                    Word absAddressY = absAddress + Y;
                    A = readByte(cycles, absAddressY, memory);
                    if (absAddressY - absAddress >= 0xFF) // we crossed a page boundary
                    {
                        cycles--;
                    }
                    break;
                }
                case INS_LDA_INDX: {
                    Byte ZPAddress = fetchByte(cycles, memory);
                    ZPAddress += X;
                    cycles--;
                    Word effectiveAddress = readWord(cycles, ZPAddress, memory);
                    A = readByte(cycles, effectiveAddress, memory);
                    break;
                }
                case INS_LDA_INDY: {
                    Byte ZPAddress = fetchByte(cycles, memory);
                    Word effectiveAddress = readWord(cycles, ZPAddress, memory);
                    Word effectiveAddressY = effectiveAddress + Y;
                    A = readByte(cycles, effectiveAddressY, memory);
                    if (effectiveAddressY - effectiveAddress >= 0xFF) // we crossed a page boundary
                    {
                        cycles--;
                    }
                    break;
                }
                case INS_JSR: {
                    Word subAddress = fetchWord(cycles, memory);
                    memory.writeWord(PC - 1, SP, cycles);
                    SP -= 2;
                    PC = subAddress;
                    cycles--;
                    break;
                }
                default: {
                    printf("Instruction '%d' not handled\n", instruction);
                    throw -1;
                }
            }
        }

        const s32 numCyclesUsed = cyclesRequested - cycles;
        return numCyclesUsed;
    }
};

// Function to print the entire memory
void printMemory(const Memory &mem) {
    const u32 columns = 32;

    for (u32 i = 0; i < Memory::MAX_MEM; i++) {
        if (i % columns == 0) {
            std::cout << "\n"
                      << std::setw(4);
        }

        std::cout << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(mem[i]) << " ";

        if ((i + 1) % columns == 0) {
            std::cout << std::dec;
        }
    }

    std::cout << std::endl;
}

#endif