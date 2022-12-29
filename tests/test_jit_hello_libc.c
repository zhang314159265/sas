#include "jit_runner.h"

int main(void) {
  /*
   * run examples/hello-libc.asm
   */
  sym_register("MSG", "call libc printf\n");
	const char *text_code = R"(
    push $MSG
    # 68 <MSG>

    call printf
    # e8 00 00 00 00

    add $0x4, %esp
    # 83 c4 04

    ret
    # c3 
  )";
	struct str bin_code = parse_text_code(NULL, text_code);
  #if 0 // manual relocating
  // fill in printf address offset
  *(uint32_t*) (bin_code.buf + 6) = (int) printf - ((int) bin_code.buf + 10);
  #endif
	jit_run(&bin_code);
	str_free(&bin_code);

	return 0;
}
