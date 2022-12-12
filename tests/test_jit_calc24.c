#include <assert.h>
#include "jit_runner.h"
#include "sym.h"
#include <readline/readline.h>
#include <math.h>

struct str solve() { // int nums[4]
  char *text_code = R"(
    # lea 0x4(%esp), %ecx
    8d 4c 24 04

    # and $0xfffffff8, %esp
    83 e4 f8

    # push -0x4(%ecx) # push the ret addr
    ff 71 fc

    # push %ebp
    55

    # mov %esp, %ebp
    89 e5

    # push %ebx
    53

    # push %ecx
    51

    # sub $0x30, %esp
    83 ec 30

    # mov %ecx, %ebx
    89 cb

    # movl $0x0, -0x10(%ebp) # evalstacksize
    c7 45 f0 00 00 00 00

    # movl $0x0, solstksize
    c7 05 <REL R_386_32 solstksize 0> 00 00 00 00

    # push $0x0
    6a 00

    # push (%ebx)
    ff 33

    # push $0x0
    6a 00

    # lea -0x38(%ebp), %eax # evalstk
    8d 45 c8

    # push %eax
    50

    # call bt
    e8 <REL R_386_PC32 bt -4>

    # add $0x10, %esp
    83 c4 10

    # mov $eax, -0x14(%ebp) # possible
    89 45 ec

    # cmpl $0x0, -0x14(%ebp)
    83 7d ec 00

    # je l_else
    74 07

    # call print_solution
    e8 <REL R_386_PC32 print_solution -4>

    # jmp l_next
    eb 48

    # push STR_IMPOSSIBLE
    68 <REL R_386_32 STR_IMPOSSIBLE 0>

    # call printf
    e8 <REL R_386_PC32 printf -4>

    # add $0x4, %esp
    83 c4 04

    # movl $0x0, -0xc(%ebp) # i
    c7 45 f4 00 00 00 00

    # jmp l_cond
    eb 22

    # mov -0xc(%ebp), %eax
    8b 45 f4

    # lea 0x0(,%eax, 4), %edx
    8d 14 85 00 00 00 00

    # mov (%ebx), %eax
    8b 03

    # add %edx, %eax
    01 d0

    # mov (%eax), %eax
    8b 00

    # push %eax
    50

    # push STR_SINGLE_NUM
    68 <REL R_386_32 STR_SINGLE_NUM 0>

    # call printf
    e8 <REL R_386_PC32 printf -4>

    # add $0x8, %esp
    83 c4 08

    # addl $0x1, -0xc(%ebp)
    83 45 f4 01

    # cmpl $0x3, -0xc(%ebp)
    83 7d f4 03

    # jle l_body
    7e d8

    # push $0xa
    6a 0a

    # call putchar
    e8 <REL R_386_PC32 putchar -4>

    # add $0x4, %esp
    83 c4 04

    # nop
    90

    # lea -0x8(%ebp), %esp
    8d 65 f8

    # pop %ecx
    59

    # pop %ebx
    5b

    # pop %ebp
    5d

    # lea -0x4(%ecx), %esp
    8d 61 fc

    # ret
    c3
  )";
  struct str bin_code = parse_text_code_simple("solve", text_code);
  jit_make_exec(&bin_code);
  return bin_code;
}

/*
 * isspace function is implemented using __ctype_b_loc.
 * Check: https://stackoverflow.com/questions/37702434/ctype-b-loc-what-is-its-purpose
 * for explanation. It's seems to be a macro rather than a function. So
 * we can not avoid the expansion by disabling inlining.
 */
struct str empty_line() {
  char *text_code = R"(
    # push %ebp
    55

    # mov %esp, %ebp
    89 e5

    # sub $0x4, %esp
    83 ec 04

    # movl $0x0, -0x4(%ebp)
    c7 45 fc 00 00 00 00

    # jmp l_loop_cond
    eb 33

    # call __ctype_b_loc
    e8 <REL R_386_PC32 __ctype_b_loc -4>

    # mov (%eax), %edx
    8b 10

    # mov -0x4(%ebp), %ecx
    8b 4d fc

    # mov 0x8(%ebp), %eax
    8b 45 08

    # add %ecx, %eax
    01 c8

    # movzbl (%eax), %eax
    0f b6 00

    # movsbl %al, %eax
    0f be c0

    # add %eax, %eax
    01 c0

    # add %edx, %eax
    01 d0

    # movzwl (%eax), %eax
    0f b7 00

    # movzwl %ax, %eax
    0f b7 c0

    # and $0x2000, %eax
    25 00 20 00 00

    # test %eax, %eax
    85 c0

    # jne l_is_space
    75 07

    # mov $0x0, %eax
    b8 00 00 00 00

    # jmp l_exit
    eb 18

    # addl $0x1, -0x4(%ebp)
    83 45 fc 01

    # mov -0x4(%ebp), %edx
    8b 55 fc

    # mov 0x8(%ebp), %eax
    8b 45 08

    # add %edx, %eax
    01 d0

    # movzbl (%eax), %eax
    0f b6 00

    # test %al, %al
    84 c0

    # jne l_loop_body
    75 be

    # mov $0x1, %eax
    b8 01 00 00 00

    # leave
    c9

    # ret
    c3
  )";
  struct str bin_code = parse_text_code_simple("empty_line", text_code);
  jit_make_exec(&bin_code);
  return bin_code;
}

struct str usage() {
  char* text_code = R"(
    # push %ebp
    55

    # mov %esp, %ebp
    89 e5

    # push STR_USAGE
    68 <REL R_386_32 STR_USAGE 0>

    # call puts
    e8 <REL R_386_PC32 puts -4>

    # add $0x4, %esp
    83 c4 04

    # nop
    90

    # leave
    c9

    # ret
    c3
  )";
  struct str bin_code = parse_text_code_simple("usage", text_code);
  jit_make_exec(&bin_code);
  return bin_code;
}

struct str parse_input() { // char *line, int nums[4]
  char *text_code = R"(
    # push %ebp
    55

    # mov %esp, %ebp
    89 e5

    # push %ebx
    53

    # sub $0x10, %esp
    83 ec 10

    # movl $0x0, -0x8(%ebp) # cnt
    c7 45 f8 00 00 00 00

    # mov 0x8(%ebp), %eax
    8b 45 08

    # mov %eax, -0xc(%ebp) # cur
    89 45 f4

    # jmp l_inloop_cond
    eb 04

    # addl $0x1, -0xc(%ebp)
    83 45 f4 01

    # call __ctype_b_loc
    e8 <REL R_386_PC32 __ctype_b_loc -4>

    # mov (%eax), %edx
    8b 10

    # mov -0xc(%ebp), %eax
    8b 45 f4

    # movzbl (%eax), %eax
    0f b6 00

    # movsbl %al, %eax
    0f be c0

    # add %eax, %eax
    01 c0

    # add %edx, %eax
    01 d0

    # movzwl (%eax), %eax
    0f b7 00

    # movzwl %ax, %eax
    0f b7 c0

    # and $0x2000, %eax
    25 00 20 00 00

    # test %eax, %eax
    85 c0

    # jne l_inloop_body
    75 d9

    # mov -0xc(%ebp), %eax
    8b 45 f4

    # movzbl (%eax), %eax
    0f b6 00

    # test %al, %al
    84 c0

    # je l_outloop_next
    0f 84 ab 00 00 00

    # call __ctype_b_loc
    e8 <REL R_386_PC32 __ctype_b_loc -4>

    # mov (%eax), %edx
    8b 10

    # mov -0xc(%ebp), %eax
    8b 45 f4

    # movzbl (%eax), %eax
    0f b6 00

    # movsbl %al, %eax
    0f be c0

    # add %eax, %eax
    01 c0

    # add %edx, %eax
    01 d0

    # movzwl (%eax), %eax
    0f b7 00

    # movzwl %ax, %eax
    0f b7 c0

    # and $0x800, %eax
    25 00 08 00 00

    # test %eax, %eax
    85 c0

    # je l_early_fail
    74 06

    # cmpl $0x3, -0x8(%ebp)
    83 7d f8 03

    # jle l_skip_early_fail
    7e 0a

    # mov $0x0, %eax
    b8 00 00 00 00

    # jmp l_exit
    e9 83 00 00 00

    # mov -0xc(%ebp), %eax
    8b 45 f4

    # add $0x1, %eax
    83 c0 01

    # mov %eax, -0x10(%ebp) # nxt
    89 45 f0

    # jmp l_inloop2_cond
    eb 04

    # addl $0x1, -0x10(%ebp)
    83 45 f0 01

    # call __ctype_b_loc
    e8 <REL R_386_PC32 __ctype_b_loc -4>

    # mov (%eax), %edx
    8b 10

    # mov -0x10(%ebp), %eax
    8b 45 f0

    # movzbl (%eax), %eax
    0f b6 00

    # movsbl %al, %eax
    0f be c0

    # add %eax, %eax
    01 c0

    # add %edx. %eax
    01 d0

    # movzwl (%eax), %eax
    0f b7 00

    # movzwl %ax, %eax
    0f b7 c0

    # and $0x800, %eax
    25 00 08 00 00

    # test %eax, %eax
    85 c0

    # jne l_inloop2_body
    75 d9

    # mov -0x10(%ebp), %eax
    8b 45 f0

    # movzbl (%eax), %eax
    0f b6 00

    # mov %al, -0x11(%ebp) # oldch
    88 45 ef

    # mov -0x10(%ebp), %eax
    8b 45 f0

    # movb $0x0, (%eax)
    c6 00 00

    # mov -0x8(%ebp), %eax
    8b 45 f8

    # lea 0x1(%eax), %edx
    8d 50 01

    # mov %edx, -0x8(%ebp)
    89 55 f8

    # lea 0x0(,%eax, 4), %edx
    8d 14 85 00 00 00 00

    # mov 0xc(%ebp), %eax # nums
    8b 45 0c

    # lea (%edx, %eax, 1), %ebx
    8d 1c 02

    # push -0xc(%ebp)
    ff 75 f4

    # call atoi
    e8 <REL R_386_PC32 atoi -4>

    # add $0x4, %esp
    83 c4 04

    # mov %eax, (%ebx)
    89 03

    # mov -0x10(%ebp), %eax
    8b 45 f0

    # movzbl -0x11(%ebp), %edx
    0f b6 55 ef

    # mov %dl, (%eax)
    88 10

    # mov -0x10(%ebp), %eax
    8b 45 f0

    # mov %eax, -0xc(%ebp)
    89 45 f4

    # jmp l_outloop_body
    e9 1e ff ff ff 

    # nop
    90

    # cmpl $0x4, -0x8(%ebp)
    83 7d f8 04

    # sete %al
    0f 94 c0

    # movzbl %al, %eax
    0f b6 c0

    # mov -0x4(%ebp), %ebx # restore
    8b 5d fc

    # leave
    c9

    # ret
    c3
  )";
  struct str bin_code = parse_text_code_simple("parse_input", text_code);
  jit_make_exec(&bin_code);
  return bin_code;
}

struct str bt() { // double* evalstk, int evalstksize, int nums[4], int nums_pos
  char *text_code = R"(
    # lea 0x4(%esp), %ecx
    8d 4c 24 04

    # and $0xfffffff8, %esp
    83 e4 f8

    # push -0x4(%ecx) # push ret addr (useless?)
    ff 71 fc

    # push %ebp
    55

    # mov %esp, %ebp
    89 e5

    # push %ebx
    53

    # push %ecx
    51

    # sub $0x38, %esp
    83 ec 38

    # mov %ecx, %ebx
    89 cb

    # cmpl $0x1, 0x4(%ebx) # evalstksize
    83 7b 04 01

    # jne l_cond1_next
    75 29

    # cmpl $0x4, 0xc(%ebx)
    83 7b 0c 04

    # jne l_cond1_next
    75 23

    # mov (%ebx), %eax
    8b 03

    # fldl (%eax) # FP(evalstk[0])
    dd 00

    # fldl DOUBLE_24 # FP(evalstk[0], 24)
    dd 05 <REL R_386_32 DOUBLE_24 0>

    # fsubrp %st, %st(1) # FP(evalstk[0]-24). fsubrp/fsubp is opposite to the instruction definition in intel manual
    de e9

    # fabs # FP(abs(evalstk[0]-24))
    d9 e1

    # fldl DOUBLE_EPSILON # FP(abs, epsilon)
    dd 05 <REL R_386_32 DOUBLE_EPSILON 0>

    # fcomip %st(1), %st # FP(dif)
    df f1

    # fstp %st(0) # FP()
    dd d8

    # setae %al
    0f 93 c0

    # movzbl %al, %eax
    0f b6 c0

    # jmp l_exit
    e9 25 02 00 00

    # cmpl $0x1, 0x4(%ebx) # evalstksize
    83 7b 04 01

    # jle l_cond2_next
    0f 8e 2c 01 00 00

    # mov 0x4(%ebx), %eax
    8b 43 04

    # add $-2, %eax # there is some trick to avoid overflow?
    05 fe ff ff 1f

    # lea $0x0(,%eax, 8), %edx
    8d 14 c5 00 00 00 00

    # mov (%ebx), %eax
    8b 03

    # add %edx, %eax
    01 d0

    # fldl (%eax)
    dd 00

    # fstpl -0x20(%ebp) # a
    dd 5d e0

    # mov 0x4(%ebx), %eax
    8b 43 04

    # add $-1, %eax
    05 ff ff ff 1f

    # lea 0x0(,%eax, 8), %edx
    8d 14 c5 00 00 00 00

    # mov (%ebx), %eax
    8b 03

    # add %edx, %eax
    01 d0

    # fldl (%eax)
    dd 00

    # fstpl -0x28(%ebp) # b
    dd 5d d8

    # movl $0x4 -0xc(%ebp) # nop
    c7 45 f4 04 00 00 00

    # fldl -0x28(%ebp) # FP(b)
    dd 45 d8

    # fabs # FP(abs_b)
    d9 e1

    # fldl DOUBLE_EPSILON # FP(abs_b, epsilon)
    dd 05 <REL R_386_32 DOUBLE_EPSILON 0>

    # fcomip %st(1), %st # FP(abs_b)
    df f1

    # fstp %st(0) # FP()
    dd d8

    # jb l_cond3_next
    72 07

    # movl $0x3, -0xc(%ebp)
    c7 45 f4 03 00 00 00

    # movl $0x0, -0x10(%ebp) # i
    c7 45 f0 00 00 00 00

    # jmp l_loop_cond
    e9 95 00 00 00

    # mov -0x10(%ebp), %eax
    8b 45 f0

    # add $STR_OPLIST, %eax
    05 <REL R_386_32 STR_OPLIST 0>

    # movzbl (%eax), %eax
    0f b6 00

    # mov %al, -0x29(%ebp) # op
    88 45 d7

    # movsbl -0x29(%ebp), %eax
    0f be 45 d7

    # push -0x24(%ebp)
    ff 75 dc

    # push -0x28(%ebp)
    ff 75 d8

    # push -0x1c(%ebp)
    ff 75 e4

    # push -0x20(%ebp)
    ff 75 e0

    # push %eax
    50

    # call apply_op
    e8 <REL R_386_PC32 apply_op -4>

    # add $0x14, %esp # FP(ans)
    83 c4 14

    # fstpl -0x38(%ebp) # c
    dd 5d c8

    # mov 0x4(%ebx), %eax
    8b 43 04

    # add $-2, %eax
    05 fe ff ff 1f

    # lea 0x0(,%eax, 8), %edx
    8d 14 c5 00 00 00 00

    # mov (%ebx), %eax
    8b 03

    # add %edx, %eax
    01 d0

    # fldl -0x38(%ebp)
    dd 45 c8

    # fstpl (%eax)
    dd 18

    # movsbl -0x29(%ebp), %ecx
    0f be 4d d7

    # mov solstksize, %eax
    a1 <REL R_386_32 solstksize 0>

    # lea 0x1(%eax), %edx
    8d 50 01

    # mov %edx, solstksize
    89 15 <REL R_386_32 solstksize 0>

    # neg %ecx
    f7 d9

    # mov %ecx, %edx
    89 ca

    # mov %edx, solstk(,%eax,4)
    89 14 85 <REL R_386_32 solstk 0>

    # mov 0x4(%ebx), %eax
    8b 43 04

    # sub $0x1, %eax
    83 e8 01

    # push 0xc(%ebx)
    ff 73 0c

    # push 0x8(%ebx)
    ff 73 08

    # push %eax
    50

    # push (%ebx)
    ff 33

    # call bt
    e8 <REL R_386_PC32 bt -4>

    # add $0x10, %esp
    83 c4 10

    # test %eax, %eax
    85 c0

    # je l_cond4_next
    74 0a

    # mov $0x1, %eax
    b8 01 00 00 00

    # jmp l_exit
    e9 3c 01 00 00

    # mov solstksize, %eax
    a1 <REL R_386_32 solstksize 0>

    # sub $0x1, %eax
    83 e8 01

    # mov %eax, solstksize
    a3 <REL R_386_32 solstksize 0>

    # addl $0x1, -0x10(%ebp) # ++i
    83 45 f0 01

    # mov -0x10(%ebp), %eax
    8b 45 f0

    # cmp -0xc(%ebp), %eax
    3b 45 f4

    # jl l_loop_body
    0f 8c 5f ff ff ff

    # mov 0x4(%ebx), %eax
    8b 43 04

    # add $-2, %eax
    05 fe ff ff 1f

    # lea $0x0(,%eax, 8), %edx
    8d 14 c5 00 00 00 00

    # mov (%ebx), %eax
    8b 03

    # add %edx, %eax
    01 d0

    # fldl -0x20(%ebp) # a
    dd 45 e0

    # fstpl (%eax)
    dd 18

    # mov 0x4(%ebx), %eax
    8b 43 04

    # add $-1, %eax
    05 ff ff ff 1f

    # lea 0x0(,%eax, 8), %edx
    8d 14 c5 00 00 00 00

    # mov (%ebx), %eax
    8b 03

    # add %edx, %eax
    01 d0

    # fldl -0x28(%ebp) # b
    dd 45 d8

    # fstpl (%eax)
    dd 18

    # mov 0xc(%ebx), %eax
    8b 43 0c

    # mov %eax, -0x14(%ebp) # i
    89 45 ec

    # jmp l_loop2_cond
    e9 d5 00 00 00

    # mov -0x14(%ebp), %eax
    8b 45 ec

    # lea 0x0(, %eax, 4), %edx
    8d 14 85 00 00 00 00

    # mov 0x8(%ebx), %eax
    8b 43 08

    # add %eax, %edx
    01 c2

    # mov 0xc(%ebx), %eax
    8b 43 0c

    # lea 0x0(, %eax, 4), %ecx
    8d 0c 85 00 00 00 00

    # mov 0x8(%ebx), %eax
    8b 43 08

    # add %ecx, %eax
    01 c8

    # push %edx
    52

    # push %eax
    50

    # call swap
    e8 <REL R_386_PC32 swap -4>

    # add $8, %esp
    83 c4 08

    # mov 0xc(%ebx), %eax
    8b 43 0c

    # lea 0x0(, %eax, 4), %edx
    8d 14 85 00 00 00 00

    # mov 0x8(%ebx), %eax
    8b 43 08

    # add %edx, %eax
    01 d0

    # mov (%eax), %ecx
    8b 08

    # mov 0x4(%ebx), %eax
    8b 43 04

    # lea 0x0(, %eax, 8), %edx
    8d 14 c5 00 00 00 00

    # mov (%ebx), %eax
    8b 03

    # add %edx, %eax
    01 d0

    # mov %ecx, -0x3c(%ebp) # temp
    89 4d c4

    # fildl -0x3c(%ebp) # FP(nums[nums_pos])
    db 45 c4

    # fstpl (%eax)
    dd 18

    # mov 0xc(%ebx), %eax
    8b 43 0c

    # lea 0x0(, %eax, 4), %edx
    8d 14 85 00 00 00 00

    # mov 0x8(%ebx), %eax
    8b 43 08

    # lea (%edx, %eax, 1), %ecx
    8d 0c 02

    # mov solstksize, %eax
    a1 <REL R_386_32 solstksize 0>

    # lea 0x1(%eax), %edx
    8d 50 01

    # mov %edx, solstksize
    89 15 <REL R_386_32 solstksize 0>

    # mov (%ecx), %edx
    8b 11

    # mov %edx, solstk(, %eax, 4)
    89 14 85 <REL R_386_32 solstk 0>

    # mov 0xc(%ebx), %eax
    8b 43 0c

    # lea 0x1(%eax), %edx
    8d 50 01

    # mov 0x4(%ebx), %eax
    8b 43 04

    # add $0x1, %eax
    83 c0 01

    # push %edx
    52

    # push 0x8(%ebx)
    ff 73 08

    # push %eax
    50

    # push (%ebx)
    ff 33

    # call bt
    e8 <REL R_386_PC32 bt -4>

    # add $0x10, %esp
    83 c4 10

    # test %eax, %eax
    85 c0

    # je l_cond5_next
    74 07

    # mov $0x1, %eax
    b8 01 00 00 00

    # jmp l_exit
    eb 48

    # mov solstksize, %eax
    a1 <REL R_386_32 solstksize 0>

    # sub $0x1, %eax
    83 e8 01

    # mov %eax, solstksize
    a3 <REL R_386_32 solstksize 0>

    # mov -0x14(%ebp), %eax # i
    8b 45 ec

    # lea 0x0(, %eax, 4), %edx
    8d 14 85 00 00 00 00

    # mov 0x8(%ebx), %eax
    8b 43 08

    # add %eax, %edx
    01 c2

    # mov 0xc(%ebx), %eax
    8b 43 0c

    # lea 0x0(, %eax, 4), %ecx
    8d 0c 85 00 00 00 00

    # mov 0x8(%ebx), %eax
    8b 43 08

    # add %ecx, %eax
    01 c8

    # push %edx
    52

    # push %eax
    50

    # call swap
    e8 <REL R_386_PC32 swap -4>

    # add $0x8 %esp
    83 c4 08

    # addl $0x1, -0x14(%ebp)
    83 45 ec 01

    # cmpl $0x3, -0x14(%ebp)
    83 7d ec 03

    # jle l_loop2_body
    0f 8e 21 ff ff ff

    # mov $0x0, %eax
    b8 00 00 00 00

    # lea -0x8(%ebp), %esp
    8d 65 f8

    # pop %ecx
    59

    # pop %ebx
    5b

    # pop %ebp
    5d

    # lea -0x4(%ecx), %esp
    8d 61 fc

    # ret
    c3
  )";
  struct str bin_code = parse_text_code_simple("bt", text_code);
  jit_make_exec(&bin_code);
  return bin_code;
}

struct str print_solution() {
  char* text_code = R"(
    # push %ebp
    55

    # mov %esp, %ebp
    89 e5

    # push %edi
    57

    # push %esi
    56

    # push %ebx
    53

    # sub $0x44, %esp
    83 ec 44

    # movl $0x0, -0x10(%ebp) # strstksize
    c7 45 f0 00 00 00 00

    # movl $0x0, -0x14(%ebp) # i
    c7 45 ec 00 00 00 00

    # jmp l_loop_cond
    e9 5d 01 00 00

    # mov -0x14(%ebp), %eax
    8b 45 ec

    # mov solstk(,%eax, 4), %eax
    8b 04 85 <REL R_386_32 solstk 0>

    # mov %al, -0x15(%ebp) # item
    88 45 eb

    # push $0x400
    68 00 04 00 00

    # call malloc
    e8 <REL R_386_PC32 malloc -4>

    # add $0x4, %esp
    83 c4 04

    # mov %eax, -0x1c(%ebp) # nextstr
    89 45 e4

    # cmpb $0x0, -0x15(%ebp)
    80 7d eb 00

    # js l_else
    78 25

    # movsbl -0x15(%ebp), %eax
    0f be 45 eb

    # push %eax
    50

    # push STR_PERCENT_D
    68 <REL R_386_32 STR_PERCENT_D 0>

    # push -0x1c(%ebp)
    ff 75 e4

    # call sprintf
    e8 <REL R_386_PC32 sprintf -4>

    # add $0xc, %esp
    83 c4 0c

    # mov -0x10(%ebp), %eax
    8b 45 f0

    # movl $0x2, -0x4c(%ebp, %eax, 4) # precstk
    c7 44 85 b4 02 00 00 00

    # jmp l_cond_next
    e9 01 01 00 00

    # movzbl -0x15(%ebp), %eax
    0f b6 45 eb

    # neg %eax
    f7 d8

    # mov %al, -0x1d(%ebp) # op
    88 45 e3

    # cmpb $0x2b, -0x1d(%ebp)
    80 7d e3 2b

    # je l_add_sub
    74 0d

    # cmpb $0x2d, -0x1d(%ebp)
    80 7d e3 2d

    # je l_add_sub
    74 07

    # mov $0x1, %eax
    b8 01 00 00 00

    # jmp l_decide_prec_next
    eb 05

    # mov $0x0, %eax
    b8 00 00 00 00

    # mov %eax, -0x24(%ebp) # prec
    89 45 dc

    # mov -0x10(%ebp), %eax
    8b 45 f0

    # sub $0x2, %eax
    83 e8 02

    # mov -0x4c(%ebp, %eax, 4), %eax
    8b 44 85 b4

    # cmp %eax, -0x24(%ebp)
    39 45 dc

    # setg %al
    0f 9f c0

    # movzbl %al, %eax
    0f b6 c0

    # mov %eax, -0x28(%ebp) # lhs_paren
    89 45 d8

    # mov -0x10(%ebp), %eax
    8b 45 f0

    # sub $0x1, %eax
    83 e8 01

    # mov -0x4c(%ebp, %eax, 4), %eax
    8b 44 85 b4

    # cmp %eax, -0x24(%ebp)
    39 45 dc

    # setge %al
    0f 9d c0

    # movzbl %al, %eax
    0f b6 c0

    # mov %eax, -0x2c(%ebp) # rhs_paren
    89 45 d4

    # cmpl $0x0, -0x2c(%ebp)
    83 7d d4 00

    # je l_no_rhs_paren_1
    74 09

    # movl STR_RPAREN, -0x50(%ebp)
    c7 45 b0 <REL R_386_32 STR_RPAREN 0>

    # jmp l_next
    eb 07

    # movl STR_EMPTY, -0x50(%ebp) # arg
    c7 45 b0 <REL R_386_32 STR_EMPTY 0>

    # mov -0x10(%ebp), %eax
    8b 45 f0

    # sub $0x01, %eax
    83 e8 01

    # mov -0x3c(%ebp, %eax, 4), %ecx # strstk
    8b 4c 85 c4

    # cmpl $0x0, -0x2c(%ebp)
    83 7d d4 00

    # je l_no_rhs_paren_2
    74 07

    # mov STR_LPAREN, %edi
    bf <REL R_386_32 STR_LPAREN 0>

    # jmp l_next
    eb 05

    # mov STR_EMPTY, %edi
    bf <REL R_386_32 STR_EMPTY 0>

    # movsbl -0x1d(%ebp), %edx
    0f be 55 e3

    # cmpl $0x0, -0x28(%ebp)
    83 7d d8 00

    # je l_no_lhs_paren_1
    74 07

    # mov STR_RPAREN, %esi
    be <REL R_386_32 STR_RPAREN 0>

    # jmp l_next
    eb 05

    # mov STR_EMPTY, %esi
    be <REL R_386_32 STR_EMPTY 0>

    # mov -0x10(%ebp), %eax
    8b 45 f0

    # sub $0x2, %eax
    83 e8 02

    # mov -0x3c(%ebp, %eax, 4), %eax
    8b 44 85 c4

    # cmpl $0x0, -0x28(%ebp)
    83 7d d8 00

    # je l_no_lhs_paren_2
    74 07

    # mov STR_LPAREN, %ebx
    bb <REL R_386_32 STR_LPAREN 0>

    # jmp l_next
    eb 05

    # mov STR_EMPTY, %ebx
    bb <REL R_386_32 STR_EMPTY 0>

    # push -0x50(%ebp)
    ff 75 b0

    # push %ecx
    51

    # push %edi
    57

    # push %edx
    52

    # push %esi
    56

    # push %eax
    50

    # push %ebx
    53

    # push $STR_CALC_STR
    68 <REL R_386_32 STR_CALC_STR 0>

    # push -0x1c(%ebp)
    ff 75 e4

    # call sprintf
    e8 <REL R_386_PC32 sprintf -4>

    # add $0x24, %esp
    83 c4 24

    # mov -0x10(%ebp), %eax
    8b 45 f0

    # sub $0x2, %eax
    83 e8 02

    # mov -0x3c(%ebp, %eax, 4), %eax
    8b 44 85 c4

    # push %eax
    50

    # call free
    e8 <REL R_386_PC32 free -4>

    # add $0x4, %esp
    83 c4 04

    # mov -0x10(%ebp), %eax
    8b 45 f0

    # sub $0x1, %eax
    83 e8 01

    # mov -0x3c(%ebp, %eax, 4), %eax
    8b 44 85 c4

    # push %eax
    50

    # call free
    e8 <REL R_386_PC32 free -4>

    # add $0x4, %esp
    83 c4 04

    # subl $0x2, -0x10(%ebp)
    83 6d f0 02

    # mov -0x10(%ebp), %eax # strstksize
    8b 45 f0

    # mov -0x24(%ebp), %edx # prec
    8b 55 dc

    # mov %edx, -0x4c(%ebp, %eax, 4)
    89 54 85 b4

    # mov -0x10(%ebp), %eax
    8b 45 f0

    # lea 0x1(%eax), %edx
    8d 50 01

    # mov %edx, -0x10(%ebp)
    89 55 f0

    # mov -0x1c(%ebp), %edx
    8b 55 e4

    # mov %edx, -0x3c(%ebp, %eax, 4)
    89 54 85 c4

    # addl $0x1, -0x14(%ebp)
    83 45 ec 01

    # mov solstksize, %eax
    a1 <REL R_386_32 solstksize 0>

    # cmp %eax, -0x14(%ebp)
    39 45 ec

    # jl l_loop_body
    0f 8c 95 fe ff ff

    # mov -0x3c(%ebp), %eax
    8b 45 c4

    # push %eax
    50

    # call puts
    e8 <REL R_386_PC32 puts -4>

    # add $0x4, %esp
    83 c4 04

    # mov -0x3c(%ebp), %eax
    8b 45 c4

    # push %eax
    50

    # call free
    e8 <REL R_386_PC32 free -4>

    # add $0x4, %esp
    83 c4 04

    # nop
    90

    # lea -0xc(%ebp), %esp
    8d 65 f4

    # pop %ebx
    5b

    # pop %esi
    5e

    # pop %edi
    5f

    # pop %ebp
    5d

    # ret
    c3
  )";
  struct str bin_code = parse_text_code_simple("print_solution", text_code);
  jit_make_exec(&bin_code);
  return bin_code;
}

struct str swap() {
  char *text_code = R"(
    # push %ebp
    55

    # mov %esp, %ebp
    89 e5

    # sub $0x4, %esp
    83 ec 04

    # mov 0x8(%ebp), %eax, # lhs
    8b 45 08

    # mov (%eax), %eax
    8b 00

    # mov %eax, -0x4(%ebp) # save
    89 45 fc

    # mov 0xc(%ebp), %eax # rhs
    8b 45 0c

    # mov (%eax), %edx
    8b 10

    # mov 0x8(%ebp), %eax
    8b 45 08

    # mov %edx, (%eax)
    89 10

    # mov 0xc(%ebp), %eax
    8b 45 0c

    # mov -0x4(%ebp), %edx
    8b 55 fc

    # mov %edx, (%eax)
    89 10

    # nop
    90

    # leave
    c9

    # ret
    c3
  )";
  struct str bin_code = parse_text_code_simple("swap", text_code);
  jit_make_exec(&bin_code);
  return bin_code;
}

struct str apply_op() {
  char *text_code = R"(
    # push %ebp
    55

    # mov %esp, %ebp
    89 e5

    # and $0xfffffff8, %esp
    83 e4 f8

    # sub $0x8, %esp
    83 ec 08

    # mov 0x8(%ebp), %eax # op
    8b 45 08

    # mov %al, 0x4(%esp) # op
    88 44 24 04

    # movsbl 0x4(%esp), %eax
    0f be 44 24 04

    # cmp $0x2f, %eax # '/'
    83 f8 2f

    # je l_div
    74 31

    # cmp $0x2f, %eax
    83 f8 2f

    # jg l_default
    7f 34

    # cmp $0x2d, %eax
    83 f8 2d

    # je l_add
    74 17

    # cmp $0x2d, %eax
    83 f8 2d

    # jg l_default
    7f 2a

    # cmp $0x2a, %eax
    83 f8 2a

    # je l_mul
    74 15

    # cmp $0x2b, %eax
    83 f8 2b

    # jne l_default
    75 20

    # fldl 0xc(%ebp)
    dd 45 0c

    # faddl 0x14(%ebp)
    dc 45 14

    # jmp l_exit
    eb 1c

    # fldl 0xc(%ebp)
    dd 45 0c

    # fsubl 0x14(%ebp)
    dc 65 14

    # jmp l_exit
    eb 14

    # fldl 0xc(%ebp)
    dd 45 0c

    # fmull 0x14(%ebp)
    dc 4d 14

    # jmp l_exit
    eb 0c

    # fldl 0xc(%ebp)
    dd 45 0c

    # fdivl 0x14(%ebp)
    dc 75 14

    # jmp l_exit
    eb 04

    # fld1
    d9 e8

    # fchs
    d9 e0

    # leave
    c9

    # ret
    c3
  )";
  struct str bin_code = parse_text_code_simple("apply_op", text_code);
  jit_make_exec(&bin_code);
  return bin_code;
}

int main(void) {
  char* text_code = R"(
    # push %ebp
    55

    # mov %esp, %ebp
    89 e5

    # sub $0x14, %esp
    83 ec 14

    # push STR_PROMPT
    68 <REL R_386_32 STR_PROMPT 0>

    # call readline
    e8 <REL R_386_PC32 readline -4>

    # add $0x4, %esp
    83 c4 04

    # mov $eax, -0x4(%ebp) # line
    89 45 fc

    # cmpl $0x0, -0x4(%ebp)
    83 7d fc 00

    # je l_done
    74 4d

    # push -0x4(%ebp)
    ff 75 fc

    # call empty_line
    e8 <REL R_386_PC32 empty_line -4>

    # add $0x4, %esp
    83 c4 04

    # test %eax, %eax
    85 c0

    # jne l_add_history_next
    75 0b

    # push -0x4(%ebp)
    ff 75 fc

    # call add_history
    e8 <REL R_386_PC32 add_history -4>

    # add $0x4, %esp
    83 c4 04

    # lea -0x14(%ebp), %eax # nums
    8d 45 ec

    # push %eax
    50

    # push -0x4(%ebp)
    ff 75 fc

    # call parse_input
    e8 <REL R_386_PC32 parse_input -4>

    # add $0x8, %esp
    83 c4 08

    # test %eax, %eax
    85 c0

    # jne l_parse_ok
    75 07

    # call usage
    e8 <REL R_386_PC32 usage -4>

    # jmp l_continue
    eb 17

    # lea -0x14(%ebp), %eax
    8d 45 ec

    # push %eax
    50

    # call solve
    e8 <REL R_386_PC32 solve -4>

    # add $0x4, %esp
    83 c4 04

    # push -0x4(%ebp)
    ff 75 fc

    # call free
    e8 <REL R_386_PC32 free -4>

    # add $0x4, %esp
    83 c4 04

    # jmp l_body
    eb 9d

    # nop
    90

    # push STR_BYE
    68 <REL R_386_32 STR_BYE 0>

    # call puts
    e8 <REL R_386_PC32 puts -4>

    # add $0x4, %esp
    83 c4 04

    # mov $0x0, %eax
    b8 00 00 00 00

    # leave
    c9

    # ret
    c3
  )";
  sym_register("STR_PROMPT", "calc24> ");
  sym_register("STR_BYE", "\nbye");
  sym_register("STR_USAGE", "A valid input is a line containing exactly 4 non-ngative integers.\nE.g., '4 4 10 10'");
  sym_register("readline", readline);

  // solstk and solstksize are global variables in the C program
  int solstk[7];
  int solstksize;
  sym_register("solstksize", &solstksize);
  sym_register("solstk", &solstk);
  sym_register("STR_IMPOSSIBLE", "IMPOSSIBLE:");
  sym_register("STR_SINGLE_NUM", " %d");
  sym_register("STR_PERCENT_D", "%d");
  sym_register("STR_LPAREN", "(");
  sym_register("STR_RPAREN", ")");
  sym_register("STR_EMPTY", "");
  sym_register("STR_CALC_STR", "%s%s%s %c %s%s%s");
  double const_double_24 = 24.0;
  sym_register("DOUBLE_24", &const_double_24);
  double const_double_epsilon = 1e-8;
  sym_register("DOUBLE_EPSILON", &const_double_epsilon);
  sym_register("STR_OPLIST", "+-*/");
  struct str apply_op_bin_code = apply_op();
  struct str swap_bin_code = swap();
  struct str print_solution_bin_code = print_solution();
  struct str bt_bin_code = bt();
  struct str solve_bin_code = solve();
  struct str empty_line_bin_code = empty_line();
  struct str usage_bin_code = usage();
  struct str parse_input_bin_code = parse_input();
  struct str bin_code = parse_text_code_simple(NULL, text_code);
  jit_run(&bin_code);
  str_free(&bin_code);
  str_free(&parse_input_bin_code);
  str_free(&usage_bin_code);
  str_free(&empty_line_bin_code);
  str_free(&solve_bin_code);
  str_free(&bt_bin_code);
  str_free(&print_solution_bin_code);
  str_free(&swap_bin_code);
  str_free(&apply_op_bin_code);
  return 0;
}
