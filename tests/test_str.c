#include <assert.h>
#include "str.h"

void test_align() {
  struct str cont = str_create(0);
  str_append(&cont, 0);
  assert(cont.len == 1);
  str_align(&cont, 4);
  assert(cont.len == 4);
  str_align(&cont, 8);
  assert(cont.len == 8);
  str_align(&cont, 4);
  assert(cont.len == 8);
}

int main(void) {
  test_align();
  return 0;
}
