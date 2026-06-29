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

    static constexpr Byte INS_LDX_IM = 0xA2;
    static constexpr Byte INS_LDX_ZP = 0xA6;
    static constexpr Byte INS_LDX_ZPY = 0xB6;
    static constexpr Word INS_LDX_ABS = 0xAE;
    static constexpr Word INS_LDX_ABSY = 0xBE;

    static constexpr Byte INS_LDY_IM = 0xA0;
    static constexpr Byte INS_LDY_ZP = 0xA4;
    static constexpr Byte INS_LDY_ZPX = 0xB4;
    static constexpr Word INS_LDY_ABS = 0xAC;
    static constexpr Word INS_LDY_ABSX = 0xBC;

    static constexpr Byte INS_STA_ZP = 0x85;
    static constexpr Byte INS_STA_ZPX = 0x95;
    static constexpr Word INS_STA_ABS = 0x8D;
    static constexpr Word INS_STA_ABSX = 0x9D;
    static constexpr Word INS_STA_ABSY = 0x99;
    static constexpr Byte INS_STA_INDX = 0x81;
    static constexpr Byte INS_STA_INDY = 0x91;

    static constexpr Word INS_JSR = 0x20;
    void LDASetStatus();
    void LDXSetStatus();
    void LDYSetStatus();
};