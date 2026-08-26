# Forge6502

Forge6502 is a reusable, cycle-sequenced NMOS 6502 core written in C++20.
It contains no console, cartridge, video, audio, or frontend dependencies.

The core supports decimal arithmetic as a configuration option, all official
opcodes, stable unofficial NMOS opcodes, cycle-visible bus accesses, IRQ/NMI,
reset sequencing, and the RDY input used by systems such as the Atari 2600.

Consumers link the CMake target `Forge6502::Core` and implement the two-method
`forge6502::CpuBus` interface.

## Projects using Forge6502

- [DendyForge](https://github.com/Stizzzly/DendyForge) — NES/Famicom/Dendy emulator
- [RamboForge](https://github.com/Stizzzly/RamboForge) — Atari 2600 emulator

```powershell
cmake --preset mingw-clang-debug
cmake --build --preset mingw-clang-debug
ctest --test-dir out/build/mingw-clang-debug --output-on-failure
```

Licensed under the MIT License.
