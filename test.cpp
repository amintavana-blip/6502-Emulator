#include <gtest/gtest.h>
#include "cpu.h"

class m6502Test : public testing::Test {
protected:
    Mem mem;
    CPU cpu;

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