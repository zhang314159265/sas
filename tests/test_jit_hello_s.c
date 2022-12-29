#include "jit_runner.h"

int main(void) {
	/*
   * run examples/hello.s
	 */
	const char *msg = "hello from jit_runner\n";
	int msglen = strlen(msg);
  sym_register("MSG", (void*) msg);
  sym_register("MSGLEN", (void*) msglen);
	printf("msg address %p\n", msg);

	const char *text_code = R"(
    # b8 04 00 00 00
    mov $4, %eax

    # bb 01 00 00 00
    mov $1, %ebx

    # b9 <MSG>
    # b9 <REL R_386_32 MSG 0>
    mov $MSG, %ecx

    # ba <MSGLEN>
    # ba <REL R_386_32 MSGLEN 0>
    mov $MSGLEN, %edx

    # cd 80
    int $0x80

    # b8 01 00 00 00
    mov $1, %eax
    # bb 00 00 00 00
    mov $0, %ebx
    # cd 80
    int $0x80
  )";

	struct str bin_code = parse_text_code(NULL, text_code);
  str_hexdump(&bin_code);
	jit_run(&bin_code);
	str_free(&bin_code);

  return 0;
}
