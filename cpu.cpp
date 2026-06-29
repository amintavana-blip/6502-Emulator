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
    std::cout << "A:  0x" << std::hex << std::setfill('0') << std::setw(2) << (int)A << std::endl;
    std::cout << "X:  0x" << std::hex << std::setfill('0') << std::setw(2) << (int)X << std::endl;
    std::cout << "Y:  0x" << std::hex << std::setfill('0') << std::setw(2) << (int)Y << std::endl;
    std::cout << "=========================\n"
              << std::endl;
}

// We use FetchByte exclusively to read the Instruction Stream (the opcodes and operands)
// because your program code is arranged in a continuous, back-to-back line.
Byte CPU::FetchByte(s32 &cycles, Mem &memory)
{
    // 1. Grab data from where PC is currently ponting
    Byte data = memory[PC];
    // 2. Increament the PC by 1 after fetching the the instruction
    PC++;
    // 3. Consuming one cycle from the clock cycle
    cycles--;

    return data;
}

Byte CPU::ReadByte(s32 &cycles, Word address, Mem &memory)
{
    // 1. Grab data from a specific target address in RAM
    Byte data = memory[address];
    // 2. Reading data over the bus still takes 1 clock cycle
    cycles--;

    return data;
}

Word CPU::ReadWord(s32 &cycles, Word address, Mem &memory)
{
    Byte lowByte = ReadByte(cycles, address, memory);

    Byte highByte = ReadByte(cycles, address + 1, memory);

    return lowByte | (highByte << 8);
}

Word CPU::FetchWord(s32 &cycles, Mem &memory)
{
    // 1. Grab low byte from PC target (limited to 8 bits by the hardware data bus)
    Word data = memory[PC];
    // 2. Advance PC to point to the next byte in memory
    PC++;
    // 3. Consume 1 clock cycle for the memory read operation
    cycles--;
    // 4. Read high byte from RAM and safely expand it into a 16-bit container
    Word highByte = static_cast<Word>(memory[PC]);
    // 5. Shift high byte to upper 8 bits and merge it with low byte via bitwise OR
    data |= (highByte << 8);
    // 6. After this we increment the PC one more time because a data is fetched and PC needs to points to the next address
    PC++;
    // 7. Consume 1 clock cycle for the second memory read operation
    cycles--;

    return data;
}

void CPU::WriteWord(s32 &cycles, Word value, Word address, Mem &memory)
{
    Byte lowByte = value & 0xFF; // because the size of the data bus is only limited to 8 bytes we have to get the
                                 // lowByte of the value first, we do that with an and operator.
    memory[address] = lowByte;
    cycles--;

    Byte highByte = (value >> 8) & 0xFF; // after getting the lowByte we shift the rest of the bytes to the right and extract them too.
    memory[address + 1] = highByte;
    cycles--;
}

void CPU::WriteByte(s32 &cycles, Byte value, Word address, Mem &memory)
{
    memory[address] = value;
    cycles--;
}

void CPU::LDASetStatus()
{
    PS.Z = (A == 0);
    PS.N = (A & 0x80) != 0;
}

void CPU::LDXSetStatus()
{
    PS.Z = (X == 0);
    PS.N = (X & 0x80) != 0;
}

void CPU::LDYSetStatus()
{
    PS.Z = (Y == 0);
    PS.N = (Y & 0x80) != 0;
}

s32 CPU::Execute(s32 &cycles, Mem &memory)
{

    s32 CyclesRequested = cycles;

    while (cycles > 0)
    {
        Byte opcode = FetchByte(cycles, memory);
        std::cout << "Remaining Cycles: " << cycles << " | PC: " << std::hex << PC << std::endl;

        switch (opcode)
        {

            // ===== LDA - Load Accumulator ===== //

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
            Byte Value = ReadByte(cycles, ZeroPageAddress, memory);
            A = Value;
            LDASetStatus();
            break;
        }

        case INS_LDA_ZPX:
        {
            Byte ZeroPageAddress = FetchByte(cycles, memory);
            ZeroPageAddress += X;
            cycles--;
            A = ReadByte(cycles, ZeroPageAddress, memory);
            LDASetStatus();
            break;
        }

        case INS_LDA_ABS:
        {
            Word AbsoluteAddress = FetchWord(cycles, memory);
            A = ReadByte(cycles, AbsoluteAddress, memory);
            LDASetStatus();
            break;
        }

        case INS_LDA_ABSX:
        {

            Word BaseAddress = FetchWord(cycles, memory);
            Word TargetAddress = BaseAddress + X;

            if ((BaseAddress & 0xFF00) != (TargetAddress & 0xFF00))
            {
                cycles--;
            }

            Byte Value = ReadByte(cycles, TargetAddress, memory);
            A = Value;
            LDASetStatus();
            break;
        }

        case INS_LDA_ABSY:
        {
            Word BaseAddress = FetchWord(cycles, memory);
            Word TargetAddress = BaseAddress + Y;

            if ((BaseAddress & 0xFF00) != (TargetAddress & 0xFF00))
            {
                cycles--;
            }

            Byte Value = ReadByte(cycles, TargetAddress, memory);
            A = Value;
            LDASetStatus();
            break;
        }

        case INS_LDA_INDX:
        {
            // The reason that we donot use a Word here to store the address is because this instruction happens in zero page (Byte 0 to 255)
            Byte ZeroPageAddress = FetchByte(cycles, memory);
            Byte IndexAddress = ZeroPageAddress + X;

            cycles--;
            // As discused the address should not cross the zero page, so when make sure that does not happen.
            // With an AND operation we dismantle a potential page crossing
            Byte lowByte = ReadByte(cycles, IndexAddress, memory);
            Byte highByte = ReadByte(cycles, (IndexAddress + 1) & 0xFF, memory);

            // Statying in the zero page is irrelevant for the effective address and it points anywhere in the memory.
            Word EffectiveAddress = lowByte | (highByte << 8);

            A = ReadByte(cycles, EffectiveAddress, memory);
            LDASetStatus();

            break;
        }

        case INS_LDA_INDY:
        {

            Byte ZeroPageAddress = FetchByte(cycles, memory);
            Byte lowByte = ReadByte(cycles, ZeroPageAddress, memory);
            Byte highByte = ReadByte(cycles, (ZeroPageAddress + 1) & 0xFF, memory);

            Word baseAddress = lowByte | (highByte << 8);

            Word EffectiveAddressY = baseAddress + Y;

            if ((ZeroPageAddress & 0xFF00) != (EffectiveAddressY & 0xFF00))
            {
                cycles--;
            }

            A = ReadByte(cycles, EffectiveAddressY, memory);
            LDASetStatus();

            break;
        }

            // ===== LDX - Load X Register ===== //

        case INS_LDX_IM:
        {
            Byte Value = FetchByte(cycles, memory);
            X = Value;
            LDXSetStatus();
            break;
        }

        case INS_LDX_ZP:
        {
            Byte ZeroPageAddress = FetchByte(cycles, memory);
            Byte Value = ReadByte(cycles, ZeroPageAddress, memory);
            X = Value;
            LDXSetStatus();
            break;
        }

        case INS_LDX_ZPY:
        {
            Byte ZeroPageAddress = FetchByte(cycles, memory);
            Byte TargetAddress = ZeroPageAddress + Y;
            cycles--;

            Byte Value = ReadByte(cycles, TargetAddress, memory);
            X = Value;
            LDXSetStatus();
            break;
        }

        case INS_LDX_ABS:
        {
            Byte lowByte = FetchByte(cycles, memory);
            Byte highByte = FetchByte(cycles, memory);

            Word TargetAddress = lowByte | (highByte << 8);

            Byte Value = ReadByte(cycles, TargetAddress, memory);
            X = Value;
            LDXSetStatus();
            break;
        }

        case INS_LDX_ABSY:
        {
            Word TargetAddress = FetchWord(cycles, memory);
            Word FinalAddress = TargetAddress + Y;

            if ((TargetAddress & 0xFF00) != (FinalAddress & 0xFF00))
            {
                cycles--;
            }

            Byte Value = ReadByte(cycles, FinalAddress, memory);
            X = Value;
            LDXSetStatus();
            break;
        }

            // ===== LDY - Load Y Register ===== //

        case INS_LDY_IM:
        {
            Byte Value = FetchByte(cycles, memory);
            Y = Value;
            LDYSetStatus();
            break;
        }

        case INS_LDY_ZP:
        {
            Byte ZeroPageAddress = FetchByte(cycles, memory);
            Byte Value = ReadByte(cycles, ZeroPageAddress, memory);
            Y = Value;
            LDYSetStatus();
            break;
        }

        case INS_LDY_ZPX:
        {
            Byte ZeroPageAddress = FetchByte(cycles, memory);
            Byte TargetAddress = ZeroPageAddress + X;
            cycles--;

            Byte Value = ReadByte(cycles, TargetAddress, memory);
            Y = Value;
            LDYSetStatus();
            break;
        }

        case INS_LDY_ABS:
        {
            Byte lowByte = FetchByte(cycles, memory);
            Byte highByte = FetchByte(cycles, memory);

            Word TargetAddress = lowByte | (highByte << 8);

            Byte Value = ReadByte(cycles, TargetAddress, memory);
            Y = Value;
            LDYSetStatus();
            break;
        }

        case INS_LDY_ABSX:
        {
            Word TargetAddress = FetchWord(cycles, memory);
            Word FinalAddress = TargetAddress + X;

            if ((TargetAddress & 0xFF00) != (FinalAddress & 0xFF00))
            {
                cycles--;
            }

            Byte Value = ReadByte(cycles, FinalAddress, memory);
            Y = Value;
            LDYSetStatus();
            break;
        }

            // ===== STA - Store Accumulator =====

        case INS_STA_ZP:
        {
            Byte ZeroPageAddress = FetchByte(cycles, memory);
            
            WriteByte(cycles, A, ZeroPageAddress, memory);
            break;
        }

        case INS_STA_ZPX:
        {
            Byte ZeroPageAddress = FetchByte(cycles, memory);
            Byte TargetAddress = ZeroPageAddress + X;
            cycles--;

            WriteByte(cycles, A, TargetAddress, memory);
            break;
        }

        case INS_STA_ABS:
        {
            Byte lowByte = FetchByte(cycles, memory);
            Byte highByte = FetchByte(cycles, memory);

            Word TargetAddress = lowByte | (highByte << 8);
            WriteByte(cycles, A, TargetAddress, memory);
            break;
        }

        case INS_STA_ABSX:
        {
            Byte lowByte = FetchByte(cycles, memory);
            Byte highByte = FetchByte(cycles, memory);

            Word TargetAddress = lowByte | (highByte << 8);

            Word FinalAddress = TargetAddress + X;
            WriteByte(cycles, A, FinalAddress, memory);
            break;
        }

        case INS_STA_ABSY:
        {
            Byte lowByte = FetchByte(cycles, memory);
            Byte highByte = FetchByte(cycles, memory);

            Word TargetAddress = lowByte | (highByte << 8);

            Word FinalAddress = TargetAddress + Y;
            WriteByte(cycles, A, FinalAddress, memory);
            break;
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
            WriteWord(cycles, ReturnPointMinusOne, STACK_PAGE_BASE + SP, memory);

            // 4. Update our Stack Pointer register
            // Because we just shoved a 16-bit Word (2 bytes) onto the stack,
            // the stack pointer moves down by 2 slots.
            SP -= 2;
            PC = SubAddress;

            cycles--; // JSR requires 1 final internal cycle to finish changing the hardware registers
            break;
        }

        default:
        {
            std::cout << "Unknown opcode hit: " << std::hex << (int)opcode << std::endl;
            cycles = 0; // Force-stop the execution loop immediately!
            break;
        }
        } // End of switch

    } // End of the while loop

    s32 CyclesActuallyUsed = CyclesRequested - cycles;

    return CyclesActuallyUsed;
}