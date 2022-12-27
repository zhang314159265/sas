#include <assert.h>
#include "operand.h"

void test_parse_imm() {
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
}

void test_parse_mem() {
  struct operand opd;
  int status;

  status = parse_mem("3(%eax, %ecx, 4)", &opd);
  assert(status == 0);
  assert(opd.type == MEM);
  assert(opd.disp == 3);
  assert(opd.base_regidx == 0);
  assert(opd.index_regidx = 1);
  assert(opd.log2scale = 2);
}

int main(void) {
  test_parse_imm();
  test_parse_mem();
  return 0;
}
