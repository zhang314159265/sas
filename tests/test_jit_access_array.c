#include "jit_runner.h"

int main() {
  int arr[] = {7, 1, 2, 8, 4, 10, -2, 9};
  sym_register("ARR", arr);
  const char *text_code = R"(
    push %ebx
    # 53

    # use ebx as base register and set it to ARR
    # bb <ARR>
    mov $ARR, %ebx

    push %esi
    # 56

    # use esi as index register and set it up
    # be 06 00 00 00
    mov $6, %esi
    
    # mod b00
    # reg b000
    # r/m b100, SIB
    # SIB
    # S: b10
    # I: b110
    # B: b011
    # 8b 04 b3
    mov (%ebx, %esi, 4), %eax

    pop %esi
    # 5e
    pop %ebx
    # 5b

    ret
    # c3
  )";

	struct str bin_code = parse_text_code_simple(NULL, text_code);
	jit_run(&bin_code);
	str_free(&bin_code);
	return 0;
}
