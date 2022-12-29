#include <stdio.h>
#include "sas.h"
#include "sym.h"

int main(void) {
  const char *text_code = R"(
    push %ebp
    # 55

    mov %esp, %ebp
    # 89 e5

    sub $0x7c, %esp
    # 83 ec 7c

    # i
    movl $0x0, -0x4(%ebp)
    # c7 45 fc 00 00 00 00

    jmp l_loop1_check
    # eb 0f

  l_loop1_body:
    # is_prime
    lea -0x7c(%ebp), %edx
    # 8d 55 84 

    mov -0x4(%ebp), %eax
    # 8b 45 fc

    add %edx, %eax
    # 01 d0

    movb $0x1, (%eax)
    # c6 00 01

    addl $0x1, -0x4(%ebp)
    # 83 45 fc 01
   
  l_loop1_check:
    cmpl $0x63, -0x4(%ebp)
    # 83 7d fc 63

    jle l_loop1_body
    # 7e eb

    movb $0x0, -0x7c(%ebp)
    # c6 45 84 00

    movb $0x0, -0x7b(%ebp)
    # c6 45 85 00

    # i
    movl $0x2, -0x8(%ebp)
    # c7 45 f8 02 00 00 00

    jmp l_loop2_check
    # eb 35
  l_loop2_body:
    lea -0x7c(%ebp), %edx
    # 8d 55 84

    mov -0x8(%ebp), %eax
    # 8b 45 f8

    add %edx, %eax
    # 01 d0

    movzbl (%eax), %eax
    # 0f b6 00
  
    test %al, %al
    # 84 c0

    je l_cond1_next
    # 74 22

    mov -0x8(%ebp), %eax
    # 8b 45 f8

    imul %eax, %eax
    # 0f af c0

    # j
    mov %eax, -0xc(%ebp)
    # 89 45 f4

    jmp l_loop3_check
    # eb 11

  l_loop3_body:
    lea -0x7c(%ebp), %edx
    # 8d 55 84

    mov -0xc(%ebp), %eax
    # 8b 45 f4

    add %edx, %eax
    # 01 d0

    movb $0x0, (%eax)
    # c6 00 00

    mov -0x8(%ebp), %eax
    # 8b 45 f8

    add %eax, -0xc(%ebp)
    # 01 45 f4

  l_loop3_check:
    cmpl $0x63, -0xc(%ebp)
    # 83 7d f4 63

    jle l_loop3_body
    # 7e e9

  l_cond1_next:
    addl $0x1, -0x8(%ebp)
    # 83 45 f8 01

  l_loop2_check:
    cmpl $0xa, -0x8(%ebp)
    # 83 7d f8 0a

    jle l_loop2_body
    # 7e c5

    # cnt
    movl $0x0, -0x10(%ebp)
    # c7 45 f0 00 00 00 00

    # i
    movl $0x0, -0x14(%ebp)
    # c7 45 ec 00 00 00 00

    jmp l_loop4_check
    # eb 15

  l_loop4_body:
    lea -0x7c(%ebp), %edx
    # 8d 55 84

    mov -0x14(%ebp), %eax
    # 8b 45 ec

    add %edx, %eax
    # 01 d0

    movzbl (%eax), %eax
    # 0f b6 00

    movzbl %al, %eax
    # 0f b6 c0

    add %eax, -0x10(%ebp)
    # 01 45 f0

    addl $0x1, -0x14(%ebp)
    # 83 45 ec 01

  l_loop4_check:
    cmpl $0x63, -0x14(%ebp)
    # 83 7d ec 63

    jle l_loop4_body
    # 7e e5

    push $0x64
    # 6a 64

    push -0x10(%ebp)
    # ff 75 f0

    push $CNT_FMT_STR
    # 68 <REL R_386_32 CNT_FMT_STR 0>

    call printf
    # e8 <REL R_386_PC32 printf -4>

    add $0xc, %esp
    # 83 c4 0c

    # i
    movl $0x0, -0x18(%ebp)
    # c7 45 e8 00 00 00 00

    jmp l_loop5_check
    # eb 23

  l_loop5_body:
    lea -0x7c(%ebp), %edx
    # 8d 55 84

    mov -0x18(%ebp), %eax
    # 8b 45 e8

    add %edx, %eax
    # 01 d0

    movzbl (%eax), %eax
    # 0f b6 00

    test %al, %al
    # 84 c0

    je l_cond2_next
    # 74 10

    push -0x18(%ebp)
    # ff 75 e8

    push $VAL_FMT_STR
    # 68 <REL R_386_32 VAL_FMT_STR 0>

    call printf
    # e8 <REL R_386_PC32 printf -4>

    add $0x8, %esp
    # 83 c4 08

  l_cond2_next:
    addl $0x1, -0x18(%ebp)
    # 83 45 e8 01

  l_loop5_check:
    cmpl $0x63, -0x18(%ebp)
    # 83 7d e8 63

    jle l_loop5_body
    # 7e d7

    push $0xa
    # 6a 0a

    call putchar
    # e8 <REL R_386_PC32 putchar -4>

    add $0x4, %esp
    # 83 c4 04

    mov $0x0, %eax
    # b8 00 00 00 00

    leave
    # c9

    ret
    # c3
  )";

  sym_register("CNT_FMT_STR", "Found %d primes less than %d\n");
  sym_register("VAL_FMT_STR", " %d");

  struct str bin_code = parse_text_code(NULL, text_code);
  jit_run(&bin_code);
  str_free(&bin_code);
  printf("bye\n");
  return 0;
}
