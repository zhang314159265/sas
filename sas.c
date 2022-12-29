#include <stdio.h>
#include "jit_runner.h"

int main(int argc, char **argv) {
  assert(argc == 2 && "Missing assembly file");
  const char* asm_path = argv[1];
  FILE* fp = fopen(asm_path, "r");
  assert(fp && "Fail to open file");

  fclose(fp);
  printf("bye\n");
  return 0;
}
