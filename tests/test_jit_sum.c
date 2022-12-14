#include "sas.h"
#include "text_code_collection.h"

int main(void) {
  const char* text_code = sum_text_code;
  sym_register("STR_ADDR", "sum is %d\n");
	struct str bin_code = parse_text_code(NULL, text_code);
	jit_run(&bin_code);
	str_free(&bin_code);

  return 0;
}
