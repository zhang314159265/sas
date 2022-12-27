#include "jit_runner.h"

int main(void) {
	const char* argnames[] = {NULL};
	int argvals[] = {};
	const char *text_code = R"(
    # b8 03 00 00 00 
    mov $3, %eax
    # c3
    ret
  )";
	struct str bin_code = parse_text_code(NULL, text_code, argnames, argvals);
	jit_run(&bin_code);
	str_free(&bin_code);

  return 0;
}
