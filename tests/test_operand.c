#include <assert.h>
#include "operand.h"

int main(void) {
  int imm;
  int status;

  status = parse_imm("$2", &imm);
  assert(status == 0);
  assert(imm == 2);

  status = parse_imm("$-2", &imm);
  assert(status == 0);
  assert(imm == -2);

  status = parse_imm("$0x12", &imm);
  assert(status == 0);
  assert(imm == 18);

  status = parse_imm("$-0x12", &imm);
  assert(status == 0);
  assert(imm == -18);

  status = parse_imm("$-0x", &imm);
  assert(status < 0);
  return 0;
}
