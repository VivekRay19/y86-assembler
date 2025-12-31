# Y86 Assembler

A robust two-pass assembler written in C for the **Y86-64 Architecture**. It translates assembly code (`.ys`) into machine code (`.bin` or hex) and manages symbol tables, error handling, and instruction parsing.

## 📂 Files Included
* `Y86.c` - Main source code containing the two-pass logic.
* `Makefile` - Build script to compile the project easily.
* `input.ys` - Sample Y86 assembly input file for testing.

## 🛠️ How to Compile
If you have `gcc` and `make` installed (Linux/WSL/Mac), simply run:
```bash
make
