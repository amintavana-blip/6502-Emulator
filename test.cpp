#include <gtest/gtest.h>
#include "cpu.h"


void DumpMemory(const Mem &memory, Word startAddress, Word endAddress)
{
    std::cout << "\n--- MEMORY HEX DUMP ---" << std::endl;

    // Align start address to the beginning of a 16-byte row
    startAddress &= 0xFFF0;

    for (s32 addr = startAddress; addr <= endAddress; addr++)
    {
        // At the start of every 16-byte row, print the current memory address
        if (addr % 16 == 0)
        {
            std::cout << std::hex << std::setw(4) << std::setfill('0') << addr << ": ";
        }

        // Print the 8-bit byte value in hex format
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)memory[addr] << " ";

        // At the end of the 16-byte row, print a newline
        if (addr % 16 == 15)
        {
            std::cout << std::endl;
        }
    }
    std::cout << "-----------------------\n"<< std::endl;
}


class m6502Test : public testing::Test
{
protected:
    Mem mem;
    CPU cpu;
    CPU cpuCopy;

    void SetUp() override
    {
        cpu.Reset(mem);
    }

    void TearDown() override {}
};

static void BasicVerification(const CPU &cpu, const CPU &cpuCopy)
{
    EXPECT_EQ(cpu.PS.C, cpuCopy.PS.C);
    EXPECT_EQ(cpu.PS.I, cpuCopy.PS.I);
    EXPECT_EQ(cpu.PS.D, cpuCopy.PS.D);
    EXPECT_EQ(cpu.PS.B, cpuCopy.PS.B);
    EXPECT_EQ(cpu.PS.V, cpuCopy.PS.V);
}

TEST_F(m6502Test, LDAImmediateCanLoadValueIntoA)
{   
    // 1. GIVEN: Access the opcode via CPU:: namespace
    mem[0xFFFC] = CPU::INS_LDA_IM;
    mem[0xFFFD] = 0x24;

    // 2. WHEN: Create a real u32 variable to pass into your Execute reference
    s32 cycles = 2;
    s32 CyclesConsumed = cpu.Execute(cycles, mem);

    // 3. THEN: Access your flags via the 'PS' struct instance (cpu.PS.Z and cpu.PS.N)
    EXPECT_EQ(cpu.A, 0x24);
    EXPECT_FALSE(cpu.PS.Z);
    EXPECT_FALSE(cpu.PS.N);
    EXPECT_EQ(CyclesConsumed, 2);
    BasicVerification(cpu, cpuCopy);
    
}

TEST_F(m6502Test, LDAZeroPageCanLoadValueIntoA)
{

    mem[0xFFFC] = CPU::INS_LDA_ZP;
    mem[0xFFFD] = 0x84;
    mem[0x0084] = 0x00;

    s32 cycles = 3;
    s32 CyclesConsumed = cpu.Execute(cycles, mem);

    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_TRUE(cpu.PS.Z);
    EXPECT_FALSE(cpu.PS.N);
    EXPECT_EQ(CyclesConsumed, 3);
    BasicVerification(cpu, cpuCopy);
}

TEST_F(m6502Test, LDAZeroPageXModeCanLoadValueIntoA)
{
    mem[0xFFFC] = CPU::INS_LDA_ZPX;
    mem[0xFFFD] = 0x80;
    cpu.X = 0xFF;
    mem[0x007F] = 0x84;

    s32 cycles = 4;
    s32 CyclesConsumed = cpu.Execute(cycles, mem);

    EXPECT_EQ(cpu.A, 0x84);
    EXPECT_FALSE(cpu.PS.Z);
    EXPECT_TRUE(cpu.PS.N);
    EXPECT_EQ(CyclesConsumed, 4);
    BasicVerification(cpu, cpuCopy);
}

TEST_F(m6502Test, LDAAbsModeCanLoadValueIntoA)
{
    mem[0xFFFC] = CPU::INS_LDA_ABS;
    mem[0xFFFD] = 0x80;
    mem[0xFFFE] = 0x80;
    mem[0x8080] = 0x84;

    s32 cycles = 4;
    s32 CyclesConsumed = cpu.Execute(cycles, mem);

    EXPECT_EQ(cpu.A, 0x84);
    EXPECT_FALSE(cpu.PS.Z);
    EXPECT_TRUE(cpu.PS.N);
    EXPECT_EQ(CyclesConsumed, 4);
    BasicVerification(cpu, cpuCopy);
}

TEST_F(m6502Test, LDAAbsXModeCanLoadValueIntoA)
{
    mem[0xFFFC] = CPU::INS_LDA_ABSX;
    mem[0xFFFD] = 0x70; // Fetches firstly the low byte of the address from the memory
    mem[0xFFFE] = 0x20; // Fetches then the high byte of the address from the memory
    cpu.X = 0x92;

    // Now we have the 16 bit address by putting together the high byte and the low byte
    // 0x20 is the page number and 0x70 is the offset which results in -> 0x2070
    // Per defenition the value in the X register should be added to this address which
    // -gives us the target address
    // In the instruction we have defined if we cross the base address page we need one more cycle
    // -since in the 6502 emulator every page has 256 Byte, 1 increment in the third bit (bit 2)
    // -causes a page crossing. So in this case by adding X to the base address, we are in a new page
    // -which is 0x2102. Attention to the difference of the third bit from 0x2070 -> 0x2102
    mem[0x2102] = 0x42;

    s32 cycles = 5;
    s32 cyclesUsed = cpu.Execute(cycles, mem);

    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_FALSE(cpu.PS.Z);
    EXPECT_FALSE(cpu.PS.N);
    EXPECT_EQ(cyclesUsed, 5);
    BasicVerification(cpu, cpuCopy);
}

TEST_F(m6502Test, LDAAbsYModeCanLoadValueIntoA)
{
    mem[0xFFFC] = CPU::INS_LDA_ABSY;
    mem[0xFFFD] = 0x70;
    mem[0xFFFE] = 0x20;
    cpu.Y = 0x42;

    mem[0x20B2] = 0x42;

    s32 cycles = 4;
    s32 cyclesUsed = cpu.Execute(cycles, mem);

    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_FALSE(cpu.PS.Z);
    EXPECT_FALSE(cpu.PS.N);
    EXPECT_EQ(cyclesUsed, 4);
    BasicVerification(cpu, cpuCopy);
}

TEST_F(m6502Test, LDAIndirectXCanLoadValueIntoA)
{
    mem[0xFFFC] = CPU::INS_LDA_INDX;
    mem[0xFFFD] = 0x42;
    cpu.X = 0x42;
    mem[0x84] = 0x42;
    mem[0x85] = 0x42;

    mem[0x4242] = 0x84;

    s32 cycles = 6;
    s32 cyclesUsed = cpu.Execute(cycles, mem);

    EXPECT_EQ(cpu.A, 0x84);
    EXPECT_FALSE(cpu.PS.Z);
    EXPECT_TRUE(cpu.PS.N);
    EXPECT_EQ(cyclesUsed, 6);
    BasicVerification(cpu, cpuCopy);
}

TEST_F(m6502Test, LDAIndirectYCanLoadValueIntoA)
{
    mem[0xFFFC] = CPU::INS_LDA_INDY;
    mem[0xFFFD] = 0x20;
    mem[0x0020] = 0x00;
    mem[0x0021] = 0x80;
    cpu.Y = 0x04;
    mem[0x8004] = 0x37;

    s32 cycles = 6;
    s32 cyclesUsed = cpu.Execute(cycles, mem);

    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_FALSE(cpu.PS.Z);
    EXPECT_FALSE(cpu.PS.N);
    EXPECT_EQ(cyclesUsed, 6);
    BasicVerification(cpu, cpuCopy);
}

TEST_F(m6502Test, LDXImmediateCanLoadValueIntoX)
{
    mem[0xFFFC] = CPU::INS_LDX_IM;
    mem[0xFFFD] = 0x42;

    s32 cycles = 2;
    s32 CyclesConsumed = cpu.Execute(cycles, mem);

    EXPECT_EQ(cpu.X, 0x42);
    EXPECT_FALSE(cpu.PS.Z);
    EXPECT_FALSE(cpu.PS.N);
    EXPECT_EQ(CyclesConsumed,2);
    BasicVerification(cpu, cpuCopy);
}

TEST_F(m6502Test, LDXZeroPageCanLoadValueIntoX)
{
    mem[0xFFFC] = CPU::INS_LDX_ZP;
    mem[0xFFFD] = 0x24;
    mem[0x0024] = 0x42;

    s32 cycles = 3;
    s32 CyclesConsumed = cpu.Execute(cycles, mem);

    EXPECT_EQ(cpu.X, 0x42);
    EXPECT_FALSE(cpu.PS.Z);
    EXPECT_FALSE(cpu.PS.N);
    EXPECT_EQ(CyclesConsumed,3);
    BasicVerification(cpu, cpuCopy);
}

TEST_F(m6502Test, LDXZeroPageYModeCanLoadValueIntoX)
{
    mem[0xFFFC] = CPU::INS_LDX_ZPY;
    mem[0xFFFD] = 0x42;
    cpu.Y = 0xFF;
    mem[0x0041] = 0x84;

    s32 cycles = 4;
    s32 CyclesConsumed = cpu.Execute(cycles, mem);

    EXPECT_EQ(cpu.X, 0x84);
    EXPECT_FALSE(cpu.PS.Z);
    EXPECT_TRUE(cpu.PS.N);
    EXPECT_EQ(CyclesConsumed,4);
    BasicVerification(cpu, cpuCopy);
}

TEST_F(m6502Test, LDXAbsModeCanLoadValueIntoX)
{
    mem[0xFFFC] = CPU::INS_LDX_ABS;
    mem[0xFFFD] = 0x70;
    mem[0xFFFE] = 0x20;

    mem[0x2070] = 0x84;

    s32 cycles = 4;
    s32 CyclesConsumed = cpu.Execute(cycles, mem);

    EXPECT_EQ(cpu.X, 0x84);
    EXPECT_FALSE(cpu.PS.Z);
    EXPECT_TRUE(cpu.PS.N);
    EXPECT_EQ(CyclesConsumed,4);
    BasicVerification(cpu, cpuCopy);
}

TEST_F(m6502Test, LDXAbsModeYCanLoadValueIntoX)
{
    mem[0xFFFC] = CPU::INS_LDX_ABSY;
    mem[0xFFFD] = 0x70;
    mem[0xFFFE] = 0x20;
    cpu.Y = 0x91;

    mem[0x2101] = 0x84;

    s32 cycles = 5;
    s32 CyclesConsumed = cpu.Execute(cycles, mem);

    EXPECT_EQ(cpu.X, 0x84);
    EXPECT_FALSE(cpu.PS.Z);
    EXPECT_TRUE(cpu.PS.N);
    EXPECT_EQ(CyclesConsumed,5);
    BasicVerification(cpu, cpuCopy);
}

TEST_F(m6502Test, LDYImmediateModeCanLoadValueIntoY)
{
    mem[0xFFFC] = CPU::INS_LDY_IM;
    mem[0xFFFD] = 0x84; // Bit 7 set (Negative)

    s32 cycles = 2;
    s32 CyclesConsumed = cpu.Execute(cycles, mem);

    EXPECT_EQ(cpu.Y, 0x84);
    EXPECT_FALSE(cpu.PS.Z);
    EXPECT_TRUE(cpu.PS.N);
    EXPECT_EQ(CyclesConsumed, 2);
    BasicVerification(cpu, cpuCopy);
}

TEST_F(m6502Test, LDYZeroPageModeCanLoadValueIntoY)
{
    mem[0xFFFC] = CPU::INS_LDY_ZP;
    mem[0xFFFD] = 0x42;
    mem[0x0042] = 0x84;

    s32 cycles = 3;
    s32 CyclesConsumed = cpu.Execute(cycles, mem);

    EXPECT_EQ(cpu.Y, 0x84);
    EXPECT_FALSE(cpu.PS.Z);
    EXPECT_TRUE(cpu.PS.N);
    EXPECT_EQ(CyclesConsumed, 3);
    BasicVerification(cpu, cpuCopy);
}

TEST_F(m6502Test, LDYZeroPageXModeCanLoadValueIntoY)
{
    mem[0xFFFC] = CPU::INS_LDY_ZPX;
    mem[0xFFFD] = 0x42;
    cpu.X = 0xFF; // 0x42 + 0xFF wraps around to 0x41 inside Zero Page
    mem[0x0041] = 0x84;

    s32 cycles = 4;
    s32 CyclesConsumed = cpu.Execute(cycles, mem);

    EXPECT_EQ(cpu.Y, 0x84);
    EXPECT_FALSE(cpu.PS.Z);
    EXPECT_TRUE(cpu.PS.N);
    EXPECT_EQ(CyclesConsumed, 4);
    BasicVerification(cpu, cpuCopy);
}

TEST_F(m6502Test, LDYAbsModeCanLoadValueIntoY)
{
    mem[0xFFFC] = CPU::INS_LDY_ABS;
    mem[0xFFFD] = 0x70; // Low Byte
    mem[0xFFFE] = 0x20; // High Byte
    mem[0x2070] = 0x84;

    s32 cycles = 4;
    s32 CyclesConsumed = cpu.Execute(cycles, mem);

    EXPECT_EQ(cpu.Y, 0x84);
    EXPECT_FALSE(cpu.PS.Z);
    EXPECT_TRUE(cpu.PS.N);
    EXPECT_EQ(CyclesConsumed, 4);
    BasicVerification(cpu, cpuCopy);
}

TEST_F(m6502Test, LDYAbsModeXCanLoadValueIntoY)
{
    mem[0xFFFC] = CPU::INS_LDY_ABSX;
    mem[0xFFFD] = 0x70; // Low Byte
    mem[0xFFFE] = 0x20; // High Byte
    cpu.X = 0x91;       // 0x2070 + 0x91 = 0x2101 (Page cross!)
    mem[0x2101] = 0x84;

    s32 cycles = 5; // 4 base + 1 penalty cycle
    s32 CyclesConsumed = cpu.Execute(cycles, mem);

    EXPECT_EQ(cpu.Y, 0x84);
    EXPECT_FALSE(cpu.PS.Z);
    EXPECT_TRUE(cpu.PS.N);
    EXPECT_EQ(CyclesConsumed, 5);
    BasicVerification(cpu, cpuCopy);
}

TEST_F(m6502Test, STAZeroPageModeCanLoadValueIntoMemoey)
{
    mem[0xFFFC] = CPU::INS_STA_ZP;
    mem[0xFFFD] = 0x21;
    cpu.A = 0x84;

    s32 cycles = 3; 
    s32 CyclesConsumed = cpu.Execute(cycles, mem);

    EXPECT_EQ(mem[0x21], 0x84);
    EXPECT_EQ(CyclesConsumed, 3);
    BasicVerification(cpu, cpuCopy);
}
TEST_F(m6502Test, STAZeroPageXModeCanLoadValueIntoMemoey)
{
    mem[0xFFFC] = CPU::INS_STA_ZPX;
    mem[0xFFFD] = 0x21;
    cpu.X = 0xE8;
    cpu.A = 0x84;

    s32 cycles = 4;
    s32 CyclesConsumed = cpu.Execute(cycles, mem);

    EXPECT_EQ(mem[0x09], 0x84);
    EXPECT_EQ(CyclesConsumed, 4);
    BasicVerification(cpu, cpuCopy);
}

TEST_F(m6502Test, STAAbsModeCanLoadValueIntoMemoey)
{
    mem[0xFFFC] = CPU::INS_STA_ABS;
    mem[0xFFFD] = 0x70;
    mem[0xFFFE] = 0x21;
    cpu.A = 0x84;

    s32 cycles = 4;
    s32 CyclesConsumed = cpu.Execute(cycles, mem);

    EXPECT_EQ(mem[0x2170], 0x84);
    EXPECT_EQ(CyclesConsumed, 4);
    BasicVerification(cpu, cpuCopy);
}

TEST_F(m6502Test, STAAbsXModeCanLoadValueIntoMemoey)
{
    mem[0xFFFC] = CPU::INS_STA_ABSX;
    mem[0xFFFD] = 0x70;
    mem[0xFFFE] = 0x21;
    cpu.X = 0x99;
    cpu.A = 0x84;

    s32 cycles = 5;
    s32 CyclesConsumed = cpu.Execute(cycles, mem);

    EXPECT_EQ(mem[0x2209], 0x84);
    EXPECT_EQ(CyclesConsumed, 5);
    BasicVerification(cpu, cpuCopy);
}

TEST_F(m6502Test, STAAbsYModeCanLoadValueIntoMemoey)
{
    mem[0xFFFC] = CPU::INS_STA_ABSY;
    mem[0xFFFD] = 0x70;
    mem[0xFFFE] = 0x21;
    cpu.Y = 0x22;
    cpu.A = 0x84;

    s32 cycles = 5;
    s32 CyclesConsumed = cpu.Execute(cycles, mem);

    EXPECT_EQ(mem[0x2192], 0x84);
    EXPECT_EQ(CyclesConsumed, 5);
    BasicVerification(cpu, cpuCopy);
}

TEST_F(m6502Test, CPUDoesNothingWhenWeExcuteZeroCycles)
{
    // given:
    s32 Mem_Cycles = 0;

    // when:
    s32 CyclesUsed = cpu.Execute(Mem_Cycles, mem);

    // then:
    EXPECT_EQ(CyclesUsed, 0);
}

TEST_F(m6502Test, CPUCanExecuteMoreCyclesThanRequestedIfRequiredByIns)
{
    mem[0xFFFC] = CPU::INS_LDA_IM;
    mem[0xFFFD] = 0x84;

    s32 cycles = 1;
    s32 CyclesUsed = cpu.Execute(cycles, mem);

    EXPECT_EQ(CyclesUsed, 2);
}

TEST_F(m6502Test, ShowMeTheMemory)
{
    DumpMemory(mem, 0x0080, 0x0090); // Look at the Zero Page pointers
    DumpMemory(mem, 0xFFF0, 0xFFFF); // Look at the Reset Vector area
}