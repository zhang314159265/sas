#include "jit_runner.h"

int main(void) {
	/*
   * run examples/hello.s
	 */
	const char *msg = "hello from jit_runner\n";
	int msglen = strlen(msg);
	const char* argnames[] = {"MSG", "MSGLEN", NULL};
	int argvals[] = {(int) msg, (int) msglen};
	printf("msg address %p\n", msg);

	const char *text_code = R"(
    b8 04 00 00 00
    bb 01 00 00 00
    b9 <MSG>
    ba <MSGLEN>
    cd 80

    b8 01 00 00 00
    bb 00 00 00 00
    cd 80
  )";

	struct str bin_code = parse_text_code(text_code, argnames, argvals);
	jit_run(&bin_code);
	str_free(&bin_code);

  return 0;
}
