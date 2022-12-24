#include "jit_runner.h"
#include "text_code_collection.h"

int main(void) {
	const char* msg = "sum is %d\n";
	const char* argnames[] = {"STR_ADDR", NULL};
	int argvals[] = {(uint32_t) msg};

  const char* text_code = sum_text_code;
	struct str bin_code = parse_text_code(NULL, text_code, argnames, argvals);
	jit_run(&bin_code);
	str_free(&bin_code);

  return 0;
}
