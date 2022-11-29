#include <stdio.h>
#include "jit_runner.h"
#include "sym.h"

struct str bt() { // step, N
  const char* text_code = R"(
    # push %ebp
    55

    # mov %esp, %ebp
    89 e5

    # push %ebx
    53

    # sub $0x14, %esp
    83 ec 14

    # mov 8(%ebp), %eax
    8b 45 08

    # cmp 0xc(%ebp), %eax
    3b 45 0c

    # jne <start>
    75 12

    # mov <ANS>, %eax
    a1 <REL R_386_32 ANS 0>

    # add $1, %eax
    83 c0 01

    # mov %eax, <ANS>
    a3 <REL R_386_32 ANS 0>

    # jmp l_exit
    e9 df 00 00 00

    # mov 0x8(%ebp), %eax
    8b 45 08

    # mov %eax, -0x18(%ebp) # i
    89 45 e8

    # jmp l_out_loop_cond
    e9 c8 00 00 00

    # first swap
    # mov 8(%ebp), %eax
    8b 45 08

    # mov STATE(,%eax, 4), %eax
    8b 04 85 <REL R_386_32 STATE 0>

    # mov %eax, -0xc(%ebp) # store to temp
    89 45 f4

    # mov -0x18(%ebp), %eax
    8b 45 e8

    # mov STATE(, %eax, 4), %edx
    8b 14 85 <REL R_386_32 STATE 0>

    # mov 8(%ebp), %eax
    8b 45 08

    # mov %edx, STATE(,%eax,4)
    89 14 85 <REL R_386_32 STATE 0>

    # mov -0x18(%ebp), %eax
    8b 45 e8

    # mov -0xc(%ebp), %edx
    8b 55 f4

    # mov %edx, STATE(, %eax, 4)
    89 14 85 <REL R_386_32 STATE 0>

    # movl $1, -0x14(%ebp) # valid
    c7 45 ec 01 00 00 00

    # movl $0, -0x10(%ebp) # j
    c7 45 f0 00 00 00 00

    # jmp l_in_loop_cond
    eb 38

    # mov 0x8(%ebp), %eax
    8b 45 08

    # sub -0x10(%ebp), %eax
    2b 45 f0

    # mov %eax, %edx
    89 c2
  
    # mov 0x8(%ebp), %eax
    8b 45 08

    # mov STATE(, %eax, 4), %ecx
    8b 0c 85 <REL R_386_32 STATE 0>

    # mov -0x10(%ebp), %eax
    8b 45 f0

    # mov STATE(, %eax, 4), %ebx
    8b 1c 85 <REL R_386_32 STATE 0>

    # mov %ecx, %eax
    89 c8

    # sub %ebx, %eax
    29 d8

    # mov %eax, %ecx
    89 c1

    # neg %ecx
    f7 d9

    # cmovns %ecx, %eax
    0f 49 c1

    # cmp %eax, %edx
    39 c2

    # jne l_if_next
    75 09

    # movl $0, -0x14(%ebp)
    c7 45 ec 00 00 00 00

    # jmp l_in_loop_next
    eb 0c

    # addl $0x1, -0x10(%ebp)
    83 45 f0 01

    # mov -0x10(%ebp), %eax
    8b 45 f0

    # cmp 0x8(%ebp), %eax
    3b 45 08

    # jl l_in_loop_body
    7c c0

    # cmpl $0x0, -0x14(%ebp)
    83 7d ec 00

    # je l_if2_next
    74 12

    # mov $0x8(%ebp), %eax
    8b 45 08

    # add $0x1, %eax
    83 c0 01

    # push 0xc(%ebp)
    ff 75 0c

    # push %eax
    50

    # call bt # Note, need special handling for relocating this recursive call
    e8 <REL R_386_PC32 bt -4>

    # add $0x8, %esp
    83 c4 08

    # second swap
    # mov 0x8(%ebp), %eax
    8b 45 08

    # mov STATE(,%eax,4) %eax
    8b 04 85 <REL R_386_32 STATE 0>

    # mov %eax, -0x8(%ebp) # temp
    89 45 f8

    # mov -0x18(%ebp), %eax
    8b 45 e8

    # mov STATE(, %eax, 4), %edx
    8b 14 85 <REL R_386_32 STATE 0>

    # mov 0x8(%ebp), %eax
    8b 45 08

    # mov %edx, STATE(,%eax, 4)
    89 14 85 <REL R_386_32 STATE 0>

    # mov -0x18(%ebp), %eax
    8b 45 e8

    # mov -0x8(%ebp), %edx
    8b 55 f8

    # mov %edx, STATE(,%eax, 4)
    89 14 85 <REL R_386_32 STATE 0>

    # add $1, -0x18(%ebp)
    83 45 e8 01

    # mov -0x18(%ebp), %eax
    8b 45 e8

    # cmp 0xc(%ebp), %eax
    3b 45 0c

    # jl l_out_loop_body
    0f 8c 2c ff ff ff

    # mov -0x4(%ebp), %ebx
    8b 5d fc
 
    #leave
    c9

    # ret
    c3
  )";
  struct str bin_code = parse_text_code_simple("bt", text_code);
  jit_make_exec(&bin_code);
  return bin_code;
}

struct str nqueen() {
  const char* text_code = R"(
    # push %ebp
    55

    # mov %esp, %ebp
    89 e5

    # sub $0x4, %esp
    83 ec 04

    # movl $0x0, -0x4(%ebp)
    c7 45 fc 00 00 00 00

    # jmp cond
    eb 11

    # mov -4(%ebp), %eax
    8b 45 fc

    # mov -4(%ebp), %edx
    8b 55 fc

    # mov %edx, 0x0(, %eax, 4)
    89 14 85 <REL R_386_32 STATE 0>

    # addl $1, -4(%ebp)
    83 45 fc 01

    # mov -4(%ebp), %eax
    8b 45 fc

    # cmp 8(%ebp), %eax
    3b 45 08

    # jl body
    7c e7

    # push 0x8(%ebp)
    ff 75 08

    # push 0
    6a 00

    # call bt
    e8 <REL R_386_PC32 bt -4>

    # add $8, %esp
    83 c4 08

    # mov ANS, %eax
    a1 <REL R_386_32 ANS 0>

    # leave
    c9

    # ret
    c3
  )";
  struct str bin_code = parse_text_code_simple("nqueen", text_code);
  jit_make_exec(&bin_code);
  return bin_code;
}

int main(void) {
  int state[64]; // assume it's enough
  int ans = 0;

  sym_register("ANS", (void*) &ans);
  sym_register("STATE", (void*) state);
  sym_register("SOL_MSG", (void*) "%d queen solution: %d\n");

  struct str bt_bin_code = bt();
  struct str nqueen_bin_code = nqueen();

  const char* text_code = R"(
    # push %ebp
    55
    # mov %esp, %ebp
    89 e5

    # sub $4, %esp
    83 ec 04

    # movl $0x8, -0x4(%ebp)
    c7 45 fc 08 00 00 00

    # push -0x4(%ebp)
    ff 75 fc

    # TODO: call nqueen
    # b8 05 00 00 00
    e8 <REL R_386_PC32 nqueen -4>

    # add $0x4, %esp
    83 c4 04

    # push %eax
    50

    # push -0x4(%ebp)
    ff 75 fc

    # push <SOL_MSG>
    68 <REL R_386_32 SOL_MSG 0>

    # call printf
    e8 <REL R_386_PC32 printf -4>

    # add $0xc, %esp
    83 c4 0c

    # mov $0, %eax
    b8 00 00 00 00

    # leave
    c9
    # ret
    c3
  )";
  struct str bin_code = parse_text_code_simple(NULL, text_code);
  jit_run(&bin_code);
  str_free(&bin_code);
  str_free(&nqueen_bin_code);
  str_free(&bt_bin_code);
  printf("bye\n");
  return 0;
}