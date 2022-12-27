A x86 assembler.

I decide to use C rather than C++ so it's easy to exclude the C++ lib dependencies and run the assembler on some environment like my SOS operating system.

# Reference
- [Linux System Call Table - ChromeOS Doc](https://chromium.googlesource.com/chromiumos/docs/+/HEAD/constants/syscalls.md)
- [A good reference of various relocation entries from Oracle](https://docs.oracle.com/cd/E19120-01/open.solaris/819-0690/6n33n7fcv/index.html)

# Scratch
- TODO: support all the assembly instructions in `test_jit_*`

- principle: one should feel very smooth to translate a assembly instruction to machine code by only referring to inst.def.

- TODO: refactor `jit_runner.c`
- TODO: think about how to do codegen to handle instructions.

- TODO: gradually use mnemonic for opcode in text code
- TODO: be able to do relocation for circular calls with cycle size > 1
  - f -> f -> f .. has cycle size 1
  - f -> g -> f .. has cycle size 2

- use jit runner to try various programs written in C/asm
  - Try other single-file C programs
    - fib matrix
    - find all a/b/c (`<L`) integer trilples for a right triangle 
    - calc sqrt(x) with binary search / newton iteration
    - calculate md5sum
    - generate random number and use that to estimate PI
    - simple curl (with or without SSL?)
    - simple http server (with or without SSL)
    - MAIL client/server
    - simple ssh client.
    - floating point examples
      - calc pi with some interesting formula
      - (1 + 1/n)^n
      - (1 / 1! + 1 / 2! +...)
      - calc sin with taylor series
      - cos(cos(cos(cos(....)))) -> converge to some number (0.73908..)

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
