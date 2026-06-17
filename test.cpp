#include <gtest/gtest.h>
#include "cpu.h"

class m6502Test : public testing::Test {
protected:
    Mem mem;
    CPU cpu;
    CPU cpuCopy;

    void SetUp() override {
        cpu.Reset(mem); 
    }

    void TearDown() override {}
};

TEST_F(m6502Test, LDAImmediateCanLoadValueIntoA) {
    // 1. GIVEN: Access the opcode via CPU:: namespace
    mem[0xFFFC] = CPU::INS_LDA_IM; 
    mem[0xFFFD] = 0x84;       

    // 2. WHEN: Create a real u32 variable to pass into your Execute reference
    u32 cycles = 2; 
    cpu.Execute(cycles, mem);

    // 3. THEN: Access your flags via the 'PS' struct instance (cpu.PS.Z and cpu.PS.N)
    EXPECT_EQ(cpu.A, 0x84);               
    EXPECT_FALSE(cpu.PS.Z); 
    EXPECT_TRUE(cpu.PS.N);                   
}
TEST_F(m6502Test, LDAZeroPageCanLoadValueIntoA)
{
    mem[0xFFFC] = CPU::INS_LDA_ZP;
    mem[0xFFFD] = 0x42;
    mem[0x42] = 0x84;

    u32 cycles = 3;
    cpu.Execute(cycles, mem);

    EXPECT_EQ(cpu.A, 0x84);
}

TEST_F(m6502Test, LDAZeroPageXModeCanLoadValueIntoA)
{
    mem[0xFFFC] = CPU::INS_LDA_ZPX;
    mem[0xFFFD] = 0x80;
    cpu.X = 0xFF;
    mem[0x007F] = 0x84;

    u32 cycles = 4;
    cpu.Execute(cycles, mem);

    EXPECT_EQ(cpu.A, 0x84);
    EXPECT_FALSE (cpu.PS.Z);
    EXPECT_TRUE (cpu.PS.N);
    EXPECT_EQ(cycles, 0);
    EXPECT_EQ(cpu.PS.C, cpuCopy.PS.C);
    EXPECT_EQ(cpu.PS.I, cpuCopy.PS.I);
    EXPECT_EQ(cpu.PS.D, cpuCopy.PS.D);
    EXPECT_EQ(cpu.PS.B, cpuCopy.PS.B);
    EXPECT_EQ(cpu.PS.V, cpuCopy.PS.V);
}