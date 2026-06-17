#include <iostream>
#include <iomanip>
#include "cpu.h"

void CPU::Reset(Mem &memory)
{
    A = X = Y = 0; // Reset registers to their default startup state
    PC = 0xFFFC;   // Set Program Counter to the Reset Vector
    SP = 0x00FF;   // Stack pointer typically starts at the top of page 1
    PS.B = PS.C = PS.D = PS.I = PS.N = PS.V = PS.Z = 0;
}

void CPU::DumpRegisters() const
{
    std::cout << "\n=== CPU REGISTER DUMP ===" << std::endl;
    // std::setw(4) forces the output to pad to 4 characters (e.g., 0xFFFC)
    std::cout << "PC: 0x" << std::hex << std::setfill('0') << std::setw(4) << PC << std::endl;
    std::cout << "SP: 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)SP << std::endl;
    std::cout << "A:  0x" << std::hex << std::setfill('0') << std::setw(2) << (int)A  << std::endl;
    std::cout << "X:  0x" << std::hex << std::setfill('0') << std::setw(2) << (int)X  << std::endl;
    std::cout << "Y:  0x" << std::hex << std::setfill('0') << std::setw(2) << (int)Y  << std::endl;
    std::cout << "=========================\n" << std::endl;
}

Byte CPU::FetchByte(u32 &cycles, Mem &memory)
{
    // 1. Grab data from where PC is currently ponting
    Byte data = memory[PC];
    // 2. Increament the PC by 1 after fetching the the instruction
    PC++;
    // 3. Consuming one cycle from the clock cycle
    cycles--;

    return data;
}

Byte CPU::ReadByte(u32 &cycles, Word address, Mem &memory)
{
    // 1. Grab data from a specific target address in RAM
    Byte data = memory[address];
    // 2. Reading data over the bus still takes 1 clock cycle
    cycles--;

    return data;
}

Word CPU::FetchWord(u32 &cycles, Mem &memory)
{
    // 1. Grab data from where PC is currently ponting
    Word data = memory[PC];
    // 2. Increament the PC by 1 after fetching the the instruction
    PC++;
    // 3. Consuming one cycle from the clock cycle
    cycles--;

    Word highByte = static_cast<Word>(memory[PC]);
    data |= (highByte << 8);
    PC++;
    cycles--;

    return data;
}

void CPU::WriteWord(u32 &cycles, Word value, Word address, Mem &memory)
{
    Byte lowByte = value & 0xFF; // because the size of the data bus is only limited to 8 bytes we have to get the
                                 // lowByte of the value first, we do that with an and operator.
    memory[address] = lowByte;
    cycles--;

    Byte highByte = (value >> 8) & 0xFF; // after getting the lowByte we shift the rest of the bytes to the right and extract them too.
    memory[address + 1] = highByte;
    cycles--;
}

void CPU::LDASetStatus()
{
    PS.Z = (A == 0);
    PS.N = (A & 0x80) != 0;
}

void CPU::Excute(u32 &cycles, Mem &memory)
{

    while (cycles > 0)
    {
        Byte opcode = FetchByte(cycles, memory);
        std::cout << "Remaining Cycles: " << cycles << " | PC: " << std::hex << PC << std::endl;

        switch (opcode)
        {
        case INS_LDA_IM:
        {

            Byte Value = FetchByte(cycles, memory);
            A = Value;
            LDASetStatus();

            break;
        }
        case INS_LDA_ZP:
        {
            Byte ZeroPageAddress = FetchByte(cycles, memory);
            A = ReadByte(cycles, ZeroPageAddress, memory);
            LDASetStatus();
        }
        case INS_LDA_ZPX:

        {
            Byte ZeroPageAddress = FetchByte(cycles, memory);
            ZeroPageAddress += X;
            cycles--;
            A = ReadByte(cycles, ZeroPageAddress, memory);
            LDASetStatus();
        }
        case INS_JSR:
        {
            // 1. Fetch the 16-bit destination address where we want to jump
            Word SubAddress = FetchWord(cycles, memory);

            // 2. Calculate the return point minus 1
            // At this exact moment, PC has already marched forward to the byte
            Word ReturnPointMinusOne = PC - 1;

            // 3. Write this 16-bit address onto the stack
            // We pass our current Stack Pointer to target the right RAM slot.
            WriteWord(cycles, ReturnPointMinusOne, 0x0100 + SP, memory);

            // 4. Update our Stack Pointer register
            // Because we just shoved a 16-bit Word (2 bytes) onto the stack,
            // the stack pointer moves down by 2 slots.
            SP -= 2;
            PC = SubAddress;

            cycles--; // JSR requires 1 final internal cycle to finish changing the hardware registers

            break;
        }
        default:
            std::cout << "Unknown opcode hit: " << std::hex << (int)opcode << std::endl;
            cycles = 0; // Force-stop the execution loop immediately!
            break;
        }
    }
}
