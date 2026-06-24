#include <iostream>
#include <cassert>
#include "mem.h"

void Mem::Initialise()
{
    for (u32 i = 0; i < MAX_MEM; i++)
    {
        Data[i] = 0;
    }
}