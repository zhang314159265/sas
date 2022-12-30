#include <stdio.h>
#include "elf_file.h"
#include "sas.h"

int main(void) {
  struct elf_file ef = ef_create();
  struct asctx ctx = asctx_create();
  _parse_text_code(&ctx, NULL, R"(
    mov $88, %eax
    ret
  )");
  ef_add_symbol(&ef, "main", ef.shn_text, 0, ctx.textsec->cont.len, STB_GLOBAL, STT_FUNC);
  ef_set_shn_in_ctx(&ef, &ctx);
  ef_write(&ef, &ctx, "/tmp/hello.o");
  printf("bye\n");
  return 0;
}
