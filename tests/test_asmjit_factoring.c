#include "jit_runner.h"
#include <stdio.h>

int main(void) {
  sym_register("MAIN_MSG", "Factoring %d into:");
  sym_register("FACTOR_MSG", " %d");
	const char* text_code = R"(
	  push %ebp
		# 55

		mov %esp, %ebp
		# 89 e5

		push %ecx
		# 51

		sub $0x14, %esp
		# 83 ec 14

		movl $0x7ff, -0xc(%ebp)
		# c7 45 f4 ff 07 00 00
 
		sub $0x8, %esp
		# 83 ec 08

		push -0xc(%ebp)
		# ff 75 f4

		push $MAIN_MSG
		# 68 <MAIN_MSG>

		# call printf
		# e8 00 00 00 00
    # e8 <REL R_386_PC32 printf -4>
    call printf

		add $0x10, %esp
		# 83 c4 10

		mov -0xc(%ebp), %eax
		# 8b 45 f4

    # left
		mov %eax, -0x14(%ebp)
		# 89 45 ec

    # cur
		movl $0x2, -0x10(%ebp)
		# c7 45 f0 02 00 00 00

		jmp l_loop_check
		# eb 38

l_loop_body:
		mov -0x14(%ebp), %eax
		# 8b 45 ec

		mov $0, %edx
		# ba 00 00 00 00

		divl -0x10(%ebp)
		# f7 75 f0

		mov %edx, %eax
		# 89 d0

		test %eax, %eax
		# 85 c0

		jne l_cond_else
		# 75 23

		sub $0x8, %esp
		# 83 ec 08

		push -0x10(%ebp)
		# ff 75 f0

		push $FACTOR_MSG
		# 68 <FACTOR_MSG>

		call printf
		# e8 00 00 00 00
    # e8 <REL R_386_PC32 printf -4>

		add $0x10, %esp
		# 83 c4 10

		mov -0x14(%ebp), %eax
		# 8b 45 ec

		mov $0x0, %edx
		# ba 00 00 00 00

		divl -0x10(%ebp)
		# f7 75 f0

		mov %eax, -0x14(%ebp)
		# 89 45 ec

		jmp l_cond_next
		# eb 04

l_cond_else:
		addl $0x1, -0x10(%ebp)
		# 83 45 f0 01

l_cond_next:
l_loop_check:
		cmpl $0x1, -0x14(%ebp)
		# 83 7d ec 01

		jne l_loop_body 
		# 75 c2

		sub $0xc, %esp
		# 83 ec 0c

		push $0xa
		# 6a 0a

		call putchar
		# e8 00 00 00 00
    # e8 <REL R_386_PC32 putchar -4>

		mov -0x4(%ebp), %ecx
		# 8b 4d fc

		leave
		# c9

		mov $5, %eax
	  # b8 07 00 00 00

		ret
		# c3
	)";
	struct str bin_code = parse_text_code(NULL, text_code);
	jit_run(&bin_code);
	str_free(&bin_code);
	printf("bye\n");
	return 0;
}
