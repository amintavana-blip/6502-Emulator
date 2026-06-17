#include "cpu.h"
#include "memory.h"
#include <iostream>
#include <stdio.h>


int main()
{
    CPU cpu;
    Mem memory;

    memory.Initialise();
    cpu.Reset(memory);

    memory[0xFFFC] = CPU::INS_JSR;
    memory[0xFFFD] = 0x42;
    memory[0xFFFE] = 0x42;
    memory[0x4242] = CPU::INS_LDA_IM;
    memory[0x4243] = 0x84;

    u32 cycles = 9;
    cpu.Execute(cycles,memory);
    cpu.DumpRegisters();

    return 0;
}