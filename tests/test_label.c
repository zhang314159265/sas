// check the comment in Makefile for why this test is not using gtest
#include "sas.h"
#include "text_code_collection.h"

int main(void) {
  const char* text_code = sum_text_code;
  sym_register("STR_ADDR", "sum is %d\n");
  struct asctx ctx = asctx_create();
  _parse_text_code(&ctx, NULL, text_code);

  struct dict* label2off = &ctx.label2off;
  assert(label2off->size == 2);
  assert(dict_lookup_nomiss(label2off, "l_loop_body") == 26);
  assert(dict_lookup_nomiss(label2off, "l_loop_cond") == 36);
  asctx_free(&ctx);
  return 0;
}
