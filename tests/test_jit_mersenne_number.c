#include <stdio.h>
#include "sas.h"
#include "sym.h"

struct str is_prime() { // int n
  const char* text_code = R"(
    # push %ebp
    55

    # mov %esp, %ebp
    89 e5

    # sub $0x4, %esp
    83 ec 04

    # movl $0x2, -0x4(%ebp) # i
    c7 45 fc 02 00 00 00

    # jmp l_cond
    eb 18

    # mov 0x8(%ebp), %eax
    8b 45 08

    # cltd
    99

    # idivl -0x4(%ebp)
    f7 7d fc

    # mov %edx, %eax
    89 d0

    # test %eax, %eax
    85 c0

    # jne l_if_next
    75 07

    # mov $0x0, %eax
    b8 00 00 00 00

    # jmp l_exit
    eb 11

    # addl $0x1, -0x4(%ebp)
    83 45 fc 01

    # mov -0x4(%ebp), %eax
    8b 45 fc

    # cmp 0x8(%ebp), %eax
    3b 45 08

    # jl l_body
    7c e0

    # mov $0x1, %eax
    b8 01 00 00 00

    # leave
    c9

    # ret
    c3
  )";
  struct str bin_code = parse_text_code("is_prime", text_code);
  jit_make_exec(&bin_code);
  return bin_code;
}

int main(void) {
  const char *text_code = R"(
    # push %ebp
    55

    # mov %esp, %ebp
    89 e5

    # sub $0x8, %esp
    83 ec 08

    # movl $0xffffffff, -0x4(%ebp) # found
    c7 45 fc ff ff ff ff

    # movl $0x2, -0x8(%ebp) # i
    c7 45 f8 02 00 00 00

    # jmp l_cond
    eb 39

    # push -0x8(%ebp)
    ff 75 f8

    # call is_prime
    e8 <REL R_386_PC32 is_prime -4>

    # add $0x4, %esp
    83 c4 04

    # test %eax, %eax
    85 c0

    # je l_if_next
    74 26

    # mov -0x8(%ebp), %eax
    8b 45 f8

    # mov $0x1, %edx
    ba 01 00 00 00

    # mov %eax, %ecx
    89 c1

    # shl %cl, %edx
    d3 e2

    # mov %edx, %eax
    89 d0

    # sub $0x1, %eax
    83 e8 01

    # push %eax
    50

    # call is_prime
    e8 <REL R_386_PC32 is_prime -4>

    # add $0x4, %esp
    83 c4 04

    # test %eax, %eax
    85 c0

    # jne l_if_next
    75 08

    # mov -0x8(%ebp), %eax
    8b 45 f8

    # mov %eax, -0x4(%ebp)
    89 45 fc

    # jmp l_next
    eb 0a

    # addl $0x1, -0x8(%ebp)
    83 45 f8 01

    # cmpl $0x1e, -0x8(%ebp)
    83 7d f8 1e

    # jle l_body
    7e c1

    # push -0x4(%ebp)
    ff 75 fc

    # push <SOL_MSG>
    68 <REL R_386_32 SOL_MSG 0>

    # call printf
    e8 <REL R_386_PC32 printf -4>

    # add $0x8, %esp
    83 c4 08

    # mov $0x0, %eax
    b8 00 00 00 00

    # leave
    c9

    # ret
    c3
  )";
  sym_register("SOL_MSG", "The first non prime mersene number with prime power is: 2**%d - 1\n");
  struct str is_prime_bin_code = is_prime();
  struct str bin_code = parse_text_code(NULL, text_code);
  jit_run(&bin_code);
  str_free(&is_prime_bin_code);
  str_free(&bin_code);
  printf("bye\n");
  return 0;
}
