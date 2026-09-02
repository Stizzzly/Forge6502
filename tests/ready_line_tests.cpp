#include <doctest/doctest.h>

#include <array>
#include <cstdint>

#include <forge6502/cpu6502.hpp>

namespace
{

class RecordingBus final : public forge6502::CpuBus
{
public:
    std::uint8_t CpuRead(std::uint16_t address) override
    {
        ++reads;
        lastRead = address;
        return memory[address];
    }

    void CpuWrite(std::uint16_t address, std::uint8_t value) override
    {
        ++writes;
        lastWrite = address;
        memory[address] = value;
    }

    std::array<std::uint8_t, 65536> memory{};
    std::size_t reads{0};
    std::size_t writes{0};
    std::uint16_t lastRead{0};
    std::uint16_t lastWrite{0};
};

void CompleteReset(forge6502::CPU6502& cpu)
{
    while (!cpu.IsInstructionBoundary())
    {
        cpu.Clock();
    }
}

} // namespace

TEST_CASE("RDY stretches an opcode read without advancing CPU state")
{
    RecordingBus bus;
    bus.memory[0xFFFC] = 0x00;
    bus.memory[0xFFFD] = 0x80;
    bus.memory[0x8000] = 0xEA;

    forge6502::CPU6502 cpu;
    cpu.ConnectBus(&bus);
    cpu.Reset();
    CompleteReset(cpu);

    cpu.ReadyLine(false);
    const auto before = cpu.Snapshot();
    cpu.Clock();
    const auto stalled = cpu.Snapshot();

    CHECK(bus.lastRead == 0x8000);
    CHECK(bus.reads > 0);
    CHECK(stalled.programCounter == before.programCounter);
    CHECK(stalled.cyclesRemaining == before.cyclesRemaining);

    cpu.ReadyLine(true);
    cpu.Clock();
    CHECK(cpu.ProgramCounter() == 0x8001);
    CHECK_FALSE(cpu.IsInstructionBoundary());
}

TEST_CASE("RDY low does not suppress the final write cycle")
{
    RecordingBus bus;
    bus.memory[0xFFFC] = 0x00;
    bus.memory[0xFFFD] = 0x80;
    bus.memory[0x8000] = 0xA9; // LDA #$5A
    bus.memory[0x8001] = 0x5A;
    bus.memory[0x8002] = 0x85; // STA $80
    bus.memory[0x8003] = 0x80;

    forge6502::CPU6502 cpu;
    cpu.ConnectBus(&bus);
    cpu.Reset();
    CompleteReset(cpu);
    while (cpu.ProgramCounter() != 0x8002 || !cpu.IsInstructionBoundary())
    {
        cpu.Clock();
    }

    cpu.Clock(); // STA opcode
    cpu.Clock(); // zero-page operand
    cpu.ReadyLine(false);
    cpu.Clock(); // write must complete even while RDY is low

    CHECK(bus.memory[0x0080] == 0x5A);
    CHECK(bus.lastWrite == 0x0080);
    CHECK(cpu.IsInstructionBoundary());
}

TEST_CASE("RDY stall removes the H mask from unstable high-byte stores")
{
    RecordingBus bus;
    bus.memory[0xFFFC] = 0x00;
    bus.memory[0xFFFD] = 0x80;
    bus.memory[0x8000] = 0xA2; // LDX #$A5
    bus.memory[0x8001] = 0xA5;
    bus.memory[0x8002] = 0xA0; // LDY #$00
    bus.memory[0x8003] = 0x00;
    bus.memory[0x8004] = 0x9E; // SHX $0500,Y
    bus.memory[0x8005] = 0x00;
    bus.memory[0x8006] = 0x05;

    forge6502::CPU6502 cpu;
    cpu.ConnectBus(&bus);
    cpu.Reset();
    CompleteReset(cpu);
    while (cpu.ProgramCounter() != 0x8004 || !cpu.IsInstructionBoundary())
    {
        cpu.Clock();
    }

    cpu.Clock(); // opcode
    cpu.Clock(); // low operand
    cpu.Clock(); // high operand
    cpu.ReadyLine(false);
    cpu.Clock(); // stretched indexed dummy read
    cpu.ReadyLine(true);
    cpu.Clock(); // repeated indexed dummy read
    cpu.Clock(); // write

    CHECK(bus.memory[0x0500] == 0xA5);
    CHECK(cpu.IsInstructionBoundary());
}
