A x86 assembler.

I decide to use C rather than C++ so it's easy to exclude the C++ lib dependencies and run the assembler on some environment like my SOS operating system.

# Reference
- [Linux System Call Table - ChromeOS Doc](https://chromium.googlesource.com/chromiumos/docs/+/HEAD/constants/syscalls.md)

# Scratch
- use jit runner to try various programs written in C/asm
  - next: mersenne number
- read volume 2 of intel SDM
  - roughly done for chapter 2. content for IA-32e mode and AVX are skipped
	- check mov instruction.
	- next chapter 6.
- TODO: use my assembler to assemble hello.s

## Instruction Format
Group 1
LOCK 0xF0
REPNE/REPNZ 0xF2
REP/REPE/REPZ 0xF3

Group 2
segment override
CS 0x2E
SS 0x36
DS 0x3E
ES 0x26
FS 0x64
GS 0x65
branch hint
branch not taken 0x2E
branch taken 0x3E

Group 3
operand-size override prefix 0x66
address-size override prefix 0x67
