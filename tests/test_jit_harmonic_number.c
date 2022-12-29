#include <stdio.h>
#include "sas.h"
#include <math.h>

struct str harmonic() { // int n
  const char* text_code = R"(
    # push %ebp
    55

    # mov %esp, %ebp
    89 e5

    # and $0xfffffff8, %esp
    83 e4 f8

    # sub $0x10, %esp
    83 ec 10

    # fldz # fpu[0]
    d9 ee

    # fstpl 0x8(%esp) # fpu[], sum
    dd 5c 24 08

    # movl $0x1, 0x4(%esp) # i
    c7 44 24 04 01 00 00 00

    # jmp l_cond
    eb 17

    # fildl 0x4(%esp) # fpu[i]
    db 44 24 04

    # fld1 # fpu[i, 1]
    d9 e8

    # ST(i) = ST(0) / ST(i). a bit confusing since it's inconsistent to intel doc. According to intel doc, it should be fdivrp
    # This page https://bugs.freebsd.org/bugzilla/show_bug.cgi?id=264590 says
    # the assembler exchanges fdivp <-> fdivrp, fsubp <-> fsubrp
    # fdivp %st, %st(1) # fpu[1/i] 
    de f1

    # fldl 0x8(%esp) # fpu[1/i, sum]
    dd 44 24 08

    # faddp %st, %st(1) # fpu[1/i + sum]
    de c1

    # fstpl 0x8(%esp) # fpu[]
    dd 5c 24 08

    # addl $0x1, 0x4(%esp)
    83 44 24 04 01

    # mov 0x4(%esp), %eax
    8b 44 24 04

    # cmp 0x8(%ebp), %eax
    3b 45 08

    # jle l_body
    7e e0

    # fldl 0x8(%esp) # fpu[sum]
    dd 44 24 08

    # leave
    c9

    # ret
    c3
  )";
  struct str bin_code = parse_text_code("harmonic", text_code);
  jit_make_exec(&bin_code);
  return bin_code;
}

int main(void) {
  const char* text_code = R"(
    # lea 0x4(%esp), %ecx
    8d 4c 24 04

    # and $0xfffffff8, %esp
    83 e4 f8

    # push -0x4(%ecx)
    ff 71 fc

    # push %ebp
    55

    # mov %esp, %ebp
    89 e5

    # push %ecx
    51

    # sub $0x1c, %esp
    83 ec 1c

    # movl $0x0, -0xc(%ebp) # nfail
    c7 45 f4 00 00 00 00

    # movl $0x1, -0x10(%ebp) # i
    c7 45 f0 01 00 00 00

    # jmp l_loop_cond
    eb 5b

    # push -0x10(%ebp)
    ff 75 f0

    # call harmonic # return value is on the fpu register stack
    e8 <REL R_386_PC32 harmonic -4>

    # add $0x4, %esp # fpu[hval]
    83 c4 04

    # fstpl -0x18(%ebp) # h, fpu[]
    dd 5d e8

    # mov -0x10(%ebp), %eax
    8b 45 f0

    # add $0x1, %eax
    83 c0 01

    # mov %eax, -0x1c(%ebp)
    89 45 e4

    # fildl -0x1c(%ebp) # fpu[ival]
    db 45 e4

    # lea -0x8(%esp), %esp
    8d 64 24 f8

    # fstpl (%esp) # fpu[]
    dd 1c 24

    # call log # fpu[ln(i+1)]
    e8 <REL R_386_PC32 log -4>

    # add $0x8, %esp
    83 c4 08

    # fldl -0x18(%ebp) # fpu[ln(i+1), h]
    dd 45 e8

    # fxch %st(1) # fpu[h, ln(i+1)]
    d9 c9

    # fcomip %st(1), %st # fpu[h]
    df f1

    # fstp %st(0) # fpu[]
    dd d8

    # ja fail_check
    77 1f

    # fildl -0x10(%ebp)
    db 45 f0

    # lea -0x8(%esp), %esp
    8d 64 24 f8

    # fstpl (%esp)
    dd 1c 24

    # call log # fpu[log(i)]
    e8 <REL R_386_PC32 log -4>

    # add $0x8, %esp
    83 c4 08

    # fld1 # fpu[log(i), 1]
    d9 e8

    # faddp %st, %st(1) # fpu[log(i) + 1]
    de c1

    # fldl -0x18(%ebp) # fpu[log(i) + 1, h]
    dd 45 e8

    # fcomip %st(1), %st # fpu[log(i) + 1]
    df f1

    # fstp %st(0) # fpu[]
    dd d8

    # jbe pass_check
    76 04

    # addl $0x1, -0xc(%ebp)
    83 45 f4 01

    # addl $0x1, -0x10(%ebp)
    83 45 f0 01

    # cmpl %0x64, -0x10(%ebp)
    83 7d f0 64

    # jle l_loop_body
    7e 9f

    # cmpl $0x0, -0xc(%ebp)
    83 7d f4 00

    # je l_pass_msg
    74 07

    # mov FAIL_MSG, %eax
    b8 <REL R_386_32 FAIL_MSG 0>

    # jmp l_next
    eb 05

    # mov PASS_MSG, %eax
    b8 <REL R_386_32 PASS_MSG 0>

    # push %eax
    50

    # call puts
    e8 <REL R_386_PC32 puts -4>

    # add $0x4, %esp
    83 c4 04

    # mov $0x0, %eax
    b8 00 00 00 00

    # mov -0x4(%ebp), %ecx
    8b 4d fc

    # leave
    c9

    # lea -0x4(%ecx), %esp
    8d 61 fc

    # ret
    c3
  )";
  sym_register("log", log);
  sym_register("FAIL_MSG", "FAIL");
  sym_register("PASS_MSG", "PASS");
  struct str harmonic_bin_code = harmonic();
  struct str bin_code = parse_text_code(NULL, text_code);
  jit_run(&bin_code);
  str_free(&bin_code);
  str_free(&harmonic_bin_code);
  printf("bye\n");
  return 0;
}
