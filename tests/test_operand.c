#include <assert.h>
#include "operand.h"

int main(void) {
  int imm;
  int reloc_imm;
  int status;

  status = parse_imm("$2", &imm, &reloc_imm);
  assert(status == 0);
  assert(imm == 2);

  status = parse_imm("$-2", &imm, &reloc_imm);
  assert(status == 0);
  assert(imm == -2);

  status = parse_imm("$0x12", &imm, &reloc_imm);
  assert(status == 0);
  assert(imm == 18);

  status = parse_imm("$-0x12", &imm, &reloc_imm);
  assert(status == 0);
  assert(imm == -18);

  status = parse_imm("$-0x", &imm, &reloc_imm);
  assert(status < 0);
  return 0;
}
