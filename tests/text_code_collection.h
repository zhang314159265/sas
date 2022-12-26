#pragma once

// text code for examples/sum.c
const char *sum_text_code = R"(
  # push %ebp
  55
  # mov %esp, %ebp . 2 alternative ways to encode
  # 89 e5
  8b ec

  # push %ecx
  51

  # sub $0x14, %esp
  83 ec 14

  # movl $0x0, -0x10(%ebp)
  c7 45 f0 00 00 00 00

  # movl $0x1, -0xc(%ebp)
  c7 45 f4 01 00 00 00

  # jmp to cond check
  # Instead of using short jmp: eb 0a
  # sas uses near jmp for simplicity: e9 xx yy zz ww
  jmp l_loop_cond

l_loop_body:
  # mov -0xc(%ebp), %eax
  8b 45 f4

  # add %eax, -0x10(%ebp)
  01 45 f0

  # addl $0x1, -0xc(%ebp)
  83 45 f4 01

l_loop_cond:
  # cmpl $0x64, -0xc(%ebp)
  83 7d f4 64

  # jle to loop body
  # 7e f0
  jle l_loop_body

  # sub $0x8, %esp
  83 ec 08

  # push -0x10(%ebp)
  ff 75 f0

  # push format string
  68 <STR_ADDR>

  # call printf, patch later
  # e8 00 00 00 00
  e8 <REL R_386_PC32 printf -4>

  # leave
  c9

  b8 05 00 00 00 
  c3
)";
