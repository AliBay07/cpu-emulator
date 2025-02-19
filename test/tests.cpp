#include "gtest/gtest.h"
#include "../src/cpu.h"
#include <stdexcept> 

class MainCpuTest : public ::testing::Test
{

public:
    Memory mem;
    CPU cpu;

protected:
    void SetUp() override
    {
        cpu.reset(mem);
    }

    void TearDown() override
    {
    }
};

static void verifyUnmodifiedFlagsFromLDA (const CPU& cpu, const CPU& cpuCopy)
{
    EXPECT_EQ(cpu.C, cpuCopy.C);
    EXPECT_EQ(cpu.I, cpuCopy.I);
    EXPECT_EQ(cpu.D, cpuCopy.D);
    EXPECT_EQ(cpu.B, cpuCopy.B);
    EXPECT_EQ(cpu.V, cpuCopy.V);
}

// Test to check that we do nothing if we pass 0 cycles to the CPU
TEST_F(MainCpuTest, TheCPUDoesNothingWhenWeExecuteZeroCycles)
{
    constexpr s32 NUM_CYCLES = 0;

    s32 cyclesUsed = cpu.execute(NUM_CYCLES, mem);

    EXPECT_EQ(cyclesUsed, 0);
}

// Test to check if the CPU uses more cycles than requested if the amount requested is not enough for the instruction
TEST_F(MainCpuTest, CPUCanExecuteMoreCyclesThanRequestedIfRequiredByTheInstruction)
{

    mem[0xFFFC] = CPU::INS_LDA_IM;
    mem[0XFFFD] = 0x84;
    constexpr s32 NUM_CYCLES = 1;

    s32 cyclesUsed = cpu.execute(NUM_CYCLES, mem);

    EXPECT_EQ(cyclesUsed, 2);
}

// Test to check that the CPU will throw an exception if given a bad instruction
TEST_F(MainCpuTest, ExecutingABadInstructionWillThrowException)
{
    CPU cpuCopy = cpu;

    mem[0xFFFC] = 0x0; // Invalid instruction
    mem[0XFFFD] = 0x0;
    constexpr s32 NUM_CYCLES = 2;

    EXPECT_THROW({
        s32 cyclesUsed = cpu.execute(NUM_CYCLES, mem);
    }, s32);
}

// Test to check if the LDA instruction in the immediate mode works as expected
TEST_F(MainCpuTest, LDAImmediateCanLoadAValueIntoTheARegister)
{

    CPU cpuCopy = cpu;

    mem[0xFFFC] = CPU::INS_LDA_IM;
    mem[0XFFFD] = 0x84;
    constexpr s32 NUM_CYCLES = 2;
    s32 cyclesUsed = cpu.execute(NUM_CYCLES, mem);

    EXPECT_EQ(cpu.A, 0x84);
    EXPECT_EQ(cyclesUsed, NUM_CYCLES);
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);
    verifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

// Test to check if the LDA instruction will add a zero value in the A register, while handling the flags correctly
TEST_F(MainCpuTest, LDAImmediateCanAffectTheZeroFlag)
{

    cpu.A = 0x44;
    CPU cpuCopy = cpu;

    mem[0xFFFC] = CPU::INS_LDA_IM;
    mem[0XFFFD] = 0x0;
    constexpr s32 NUM_CYCLES = 2;
    s32 cyclesUsed = cpu.execute(NUM_CYCLES, mem);

    EXPECT_EQ(cpu.A, 0x0);
    EXPECT_EQ(cyclesUsed, NUM_CYCLES);
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    verifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

// Test to check if the LDA instruction in the zero-page mode works as expected
TEST_F(MainCpuTest, LDAZeroPageCanLoadAValueIntoTheARegister)
{

    CPU cpuCopy = cpu;

    mem[0xFFFC] = CPU::INS_LDA_ZP;
    mem[0XFFFD] = 0x42;
    mem[0X0042] = 0x37;
    constexpr s32 NUM_CYCLES = 3;
    s32 cyclesUsed = cpu.execute(NUM_CYCLES, mem);

    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_EQ(cyclesUsed, NUM_CYCLES);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    verifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

// Test to check if the LDA instruction in the zero page x mode works as expected
TEST_F(MainCpuTest, LDAZeroPageXCanLoadAValueIntoTheARegister)
{

    CPU cpuCopy = cpu;

    cpu.X = 5;

    mem[0xFFFC] = CPU::INS_LDA_ZPX;
    mem[0XFFFD] = 0x42;
    mem[0X0047] = 0x37;
    constexpr s32 NUM_CYCLES = 4;
    s32 cyclesUsed = cpu.execute(NUM_CYCLES, mem);

    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_EQ(cyclesUsed, NUM_CYCLES);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    verifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

// Test to check if the LDA instruction in the absolute mode works as expected
TEST_F(MainCpuTest, LDAAbsoluteCanLoadAValueIntoTheARegister)
{

    CPU cpuCopy = cpu;

    mem[0xFFFC] = CPU::INS_LDA_ABS;
    mem[0XFFFD] = 0x80;
    mem[0XFFFE] = 0x44; //0x4480 (my device is little endian, so the least significant bytes are stored first, i won't handle the cases for big endian devices)
    mem[0x4480] = 0x37;
    constexpr s32 NUM_CYCLES = 4;

    s32 cyclesUsed = cpu.execute(NUM_CYCLES, mem);

    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_EQ(cyclesUsed, NUM_CYCLES);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    verifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

// Test to check if the LDA instruction in the absolute x mode works as expected
TEST_F(MainCpuTest, LDAAbsoluteXCanLoadAValueIntoTheARegister)
{

    cpu.X = 1;
    CPU cpuCopy = cpu;

    mem[0xFFFC] = CPU::INS_LDA_ABSX;
    mem[0XFFFD] = 0x80;
    mem[0XFFFE] = 0x44;
    mem[0x4481] = 0x37;
    constexpr s32 NUM_CYCLES = 4;

    s32 cyclesUsed = cpu.execute(NUM_CYCLES, mem);

    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_EQ(cyclesUsed, NUM_CYCLES);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    verifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

// Test to check if the LDA instruction in the absolute x mode works as expected with page crossed
TEST_F(MainCpuTest, LDAAbsoluteXCanLoadAValueIntoTheARegisterWithPageCrossed)
{

    cpu.X = 0xFF;
    CPU cpuCopy = cpu;

    mem[0xFFFC] = CPU::INS_LDA_ABSX;
    mem[0XFFFD] = 0x02;
    mem[0XFFFE] = 0x44; // 0x4402
    mem[0x4501] = 0x37; // 0x4402 + 0xFF crosses page boundary!
    constexpr s32 NUM_CYCLES = 5;

    s32 cyclesUsed = cpu.execute(NUM_CYCLES, mem);

    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_EQ(cyclesUsed, NUM_CYCLES);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    verifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

// Test to check if the LDA instruction in the absolute y mode works as expected
TEST_F(MainCpuTest, LDAAbsoluteYCanLoadAValueIntoTheARegister)
{

    cpu.Y = 1;
    CPU cpuCopy = cpu;

    mem[0xFFFC] = CPU::INS_LDA_ABSY;
    mem[0XFFFD] = 0x80;
    mem[0XFFFE] = 0x44;
    mem[0x4481] = 0x37;
    constexpr s32 NUM_CYCLES = 4;

    s32 cyclesUsed = cpu.execute(NUM_CYCLES, mem);

    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_EQ(cyclesUsed, NUM_CYCLES);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    verifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

// Test to check if the LDA instruction in the absolute y mode works as expected with page crossed
TEST_F(MainCpuTest, LDAAbsoluteYCanLoadAValueIntoTheARegisterWithPageCrossed)
{

    cpu.Y = 0xFF;
    CPU cpuCopy = cpu;

    mem[0xFFFC] = CPU::INS_LDA_ABSY;
    mem[0XFFFD] = 0x02;
    mem[0XFFFE] = 0x44; // 0x4402
    mem[0x4501] = 0x37; // 0x4402 + 0xFF crosses page boundary!
    constexpr s32 NUM_CYCLES = 5;

    s32 cyclesUsed = cpu.execute(NUM_CYCLES, mem);

    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_EQ(cyclesUsed, NUM_CYCLES);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    verifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

// Test to check if the LDA instruction in the indirect x mode works as expected
TEST_F(MainCpuTest, LDAIndirectXCanLoadAValueIntoTheARegister)
{

    cpu.X = 0x04;
    CPU cpuCopy = cpu;

    mem[0xFFFC] = CPU::INS_LDA_INDX;
    mem[0XFFFD] = 0x02;
    mem[0X0006] = 0x00; // 0x2 + 0x4
    mem[0x0007] = 0x80; // 0x8000
    mem[0x8000] = 0x37;
    constexpr s32 NUM_CYCLES = 6;

    s32 cyclesUsed = cpu.execute(NUM_CYCLES, mem);

    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_EQ(cyclesUsed, NUM_CYCLES);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    verifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

// Test to check if the LDA instruction in the indirect y mode works as expected
TEST_F(MainCpuTest, LDAIndirectYCanLoadAValueIntoTheARegister)
{

    cpu.Y = 0x04;
    CPU cpuCopy = cpu;

    mem[0xFFFC] = CPU::INS_LDA_INDY;
    mem[0XFFFD] = 0x02;
    mem[0X0002] = 0x00;
    mem[0x0003] = 0x80; // 0x8000 + 0x4
    mem[0x8004] = 0x37;
    constexpr s32 NUM_CYCLES = 5;

    s32 cyclesUsed = cpu.execute(NUM_CYCLES, mem);

    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_EQ(cyclesUsed, NUM_CYCLES);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    verifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

// Test to check if the LDA instruction in the indirect y mode works as expected with page crossed
TEST_F(MainCpuTest, LDAIndirectYCanLoadAValueIntoTheARegisterWithPageCrossed)
{

    cpu.Y = 0xFF;
    CPU cpuCopy = cpu;

    mem[0xFFFC] = CPU::INS_LDA_INDY;
    mem[0XFFFD] = 0x02;
    mem[0X0002] = 0x02;
    mem[0x0003] = 0x80; // 0x8002 + 0xFF
    mem[0x8101] = 0x37;
    constexpr s32 NUM_CYCLES = 6;

    s32 cyclesUsed = cpu.execute(NUM_CYCLES, mem);

    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_EQ(cyclesUsed, NUM_CYCLES);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    verifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
