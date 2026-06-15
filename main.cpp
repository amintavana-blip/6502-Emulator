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

    memory[0xFFFC] = 0xA5; // LDA Immediate
    memory[0xFFFD] = 0x42;
    memory[0x0042] = 0x84;

    u32 cycles = 3;
    cpu.Excute(cycles,memory);

    return 0;
}