# Forge6502 invariants

- Keep the library platform-neutral. It may depend only on the C++ standard library and `CpuBus`.
- Preserve one externally visible bus transaction per modeled CPU cycle.
- Decimal mode is configurable because NES disables decimal arithmetic while Atari 2600 uses it.
- RDY stretches read cycles but never suppresses a write cycle.
- Every timing or opcode change requires a focused regression test.
