#include "jit_runner.h"

int main(void) {
	const char* msg = "sum is %d\n";
	const char* argnames[] = {"STR_ADDR", NULL};
	int argvals[] = {(uint32_t) msg};
	const char *text_code = R"(
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
		eb 0a

		# mov -0xc(%ebp), %eax
		8b 45 f4

		# add %eax, -0x10(%ebp)
		01 45 f0

		# addl $0x1, -0xc(%ebp)
		83 45 f4 01

		# cmpl $0x64, -0xc(%ebp)
		83 7d f4 64

		# jle to loop body
		7e f0

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
	struct str bin_code = parse_text_code(text_code, argnames, argvals);
	jit_run(&bin_code);
	str_free(&bin_code);

  return 0;
}
