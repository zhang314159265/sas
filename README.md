A x86 assembler.

I decide to use C rather than C++ so it's easy to exclude the C++ lib dependencies and run the assembler on some environment like my SOS operating system.

# Reference
- [Linux System Call Table - ChromeOS Doc](https://chromium.googlesource.com/chromiumos/docs/+/HEAD/constants/syscalls.md)
- [A good reference of various relocation entris from Oracle](https://docs.oracle.com/cd/E19120-01/open.solaris/819-0690/6n33n7fcv/index.html)

# Scratch
- principle: one should feel very smooth to translate a assembly instruction to machine code by only referring to inst.def.

- TODO: run all tests rather than only the one specified by the default target
- TODO: gradually use mnemonic for opcode in text code
- TODO: be able to do relocation for circular calls with cycle size > 1
  - f -> f -> f .. has cycle size 1
  - f -> g -> f .. has cycle size 2
- TODO: support label in text code and use that in nqueen... // TODO HERE

- use jit runner to try various programs written in C/asm
  - next: prime sieve // TODO HERE 
    - // be able to quickly translate assembly to text code using inst.def

  - next: mersenne number (find 11..)
  - next: fib matrix
  - next: find a floating point example
  - Try other single-file C programs
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
