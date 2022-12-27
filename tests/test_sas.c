#include "jit_runner.h"

void test_jmp() {
  struct str bin_code;
  bin_code = parse_text_code_simple(NULL, R"(
    jmp label
    nop
    nop
    nop
  label:
  )");
  assert(str_check(&bin_code, "e903000000909090"));
}

void test_mov() {
  struct str bin_code;
  
  // test load nodisp
  bin_code = parse_text_code_simple(NULL, "mov (%ebx), %eax");
  assert(str_check(&bin_code, "8b03"));

  // test store nodisp
  bin_code = parse_text_code_simple(NULL, "mov %eax, (%ebx)");
  assert(str_check(&bin_code, "8903"));

  // test load disp8
  bin_code = parse_text_code_simple(NULL, "mov -0x10(%ebp), %eax");
  assert(str_check(&bin_code, "8b45f0"));

  // test load disp32
  bin_code = parse_text_code_simple(NULL, "mov 0x1234(%ebp), %eax");
  assert(str_check(&bin_code, "8b8534120000"));

  // test store disp8
  bin_code = parse_text_code_simple(NULL, "mov %eax, -0x10(%ebp)");
  assert(str_check(&bin_code, "8945f0"));

  // test store disp32
  bin_code = parse_text_code_simple(NULL, "mov %eax, 0x1234(%ebp)");
  assert(str_check(&bin_code, "898534120000"));
}

int main(void) {
  test_jmp();
  test_mov();
  return 0;
}
