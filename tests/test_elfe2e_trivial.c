#include <stdio.h>
#include "elf_file.h"
#include "jit_runner.h"

int main(void) {
  struct elf_file ef = ef_create();
  ef.code = parse_text_code_simple(NULL, R"(
    mov $88, %eax
    ret
  )");
  ef_add_symbol(&ef, "main", ef.shn_text, 0, ef.code.len, STB_GLOBAL, STT_FUNC);
  ef_write(&ef, "/tmp/hello.o");
  printf("bye\n");
  return 0;
}
