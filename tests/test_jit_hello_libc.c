#include "jit_runner.h"

int main(void) {
  /*
   * run examples/hello-libc.asm
   */
  const char* msg = "call libc printf\n";
	const char* argnames[] = {"MSG", NULL};
	int argvals[] = {(int) msg};
	const char *text_code = R"(
    68 <MSG>
    # the rel address need to be adjusted later
    e8 00 00 00 00
    # add $0x4, %esp
    83 c4 04
    c3 
  )";
	struct str bin_code = parse_text_code(NULL, text_code, argnames, argvals);
  // fill in printf address offset
  *(uint32_t*) (bin_code.buf + 6) = (int) printf - ((int) bin_code.buf + 10);
	jit_run(&bin_code);
	str_free(&bin_code);

	return 0;
}
