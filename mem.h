#pragma once
#include <cstdint>

using Byte = unsigned char;
using Word = unsigned short;
using u32  = unsigned int;
using s32  = signed int;

class Mem
{
    public:

    static constexpr u32 MAX_MEM = 1024 * 64;
    Byte Data[MAX_MEM];

    void Initialise();
    Byte& operator[] (u32 address){return Data[address];} 
    //Byte& operator= (u32 address){return Data[address];}
    const Byte& operator[] (u32 address) const{return Data[address];} // read 1 byte from the memory

};