#include <stdio.h>
#include "sas.h"
#include "elf_file.h"

int main(int argc, char **argv) {
  CHECK(argc == 3, "Usage: sas <asm_file> <object_file>");
  const char* asm_path = argv[1];
  FILE* fp = fopen(asm_path, "r");
  assert(fp && "Fail to open file");

  struct asctx ctx = asctx_create();

  char buf[4096];
  while (fgets(buf, sizeof(buf), fp)) {
    int len = strlen(buf);
    while (len > 0 && isspace(buf[len - 1])) {
      --len;
    }
    buf[len] = '\0';
    parse_text_code_line(&ctx, buf, len);
  }

  str_hexdump(&ctx.bin_code);

  struct elf_file ef = ef_create();
  asctx_resolve_label_patch(&ctx); // TODO handle label patch with relocation
  ef.code = str_move(&ctx.bin_code);

  ef_add_symbols_from_labels(&ef, &ctx);
  ef_dump_syms(&ef);

  // handle relocation
  ef_handle_relocs(&ef, &ctx);

  ef_write(&ef, argv[2]);

  asctx_free(&ctx);
  fclose(fp);
  printf("bye\n");
  return 0;
}
