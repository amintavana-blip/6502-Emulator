#pragma once
#include <cstdint>
#include "mem.h"

using Byte = unsigned char;
using Word = unsigned short;
using u32 = unsigned int;
using s32 = signed int;

class CPU
{

private:
    Byte FetchByte(s32 &cycles, Mem &memory);
    Byte ReadByte(s32 &cycles, Word address, Mem &memory);
    Word FetchWord(s32 &cycles, Mem &memory);
    Word ReadWord(s32 &cycles, Word address, Mem &memory);
    void WriteWord(s32 &cycles, Word value, Word address, Mem &memory);
    void WriteByte(s32 &cycles, Byte value, Word address, Mem &memory);


public:
    Word PC; // program counter
    Byte SP; // stack pointer

    Byte A, X, Y; // registers

    struct StatusFlags
    {
        Byte C : 1; // carry flag
        Byte Z : 1; // zero flag
        Byte V : 1; // overflow flag
        Byte I : 1; // interrupt disable
        Byte D : 1; // decimal mode
        Byte B : 1; // break command
        Byte N : 1; // negative flag

    } PS; // processor status

    void Reset(Mem &memory);
    s32 Execute(s32 &cycles, Mem &memory);
    void DumpRegisters() const;
    static constexpr Word STACK_PAGE_BASE = 0x0100;

    //==== OPCODES ====//

    static constexpr Byte INS_LDA_IM = 0xA9;
    static constexpr Byte INS_LDA_ZP = 0xA5;
    static constexpr Byte INS_LDA_ZPX = 0xB5;
    static constexpr Word INS_LDA_ABS = 0xAD;
    static constexpr Word INS_LDA_ABSX = 0xBD;
    static constexpr Word INS_LDA_ABSY = 0xB9;
    static constexpr Byte INS_LDA_INDX = 0xA1;
    static constexpr Byte INS_LDA_INDY = 0xB1;
    static constexpr Word INS_JSR = 0x20;
    void LDASetStatus();
};