// check the comment in Makefile for why this test is not using gtest
#include "sas.h"
#include "text_code_collection.h"

int main(void) {
  const char* text_code = sum_text_code;
  sym_register("STR_ADDR", "sum is %d\n");
  struct asctx ctx = asctx_create();
  _parse_text_code(&ctx, NULL, text_code);

  struct dict* label2idx = &ctx.label2idx;
  assert(label2idx->size == 2);
  assert(asctx_get_label_metadata(&ctx, "l_loop_body")->off == 26);
  assert(asctx_get_label_metadata(&ctx, "l_loop_cond")->off == 36);
  asctx_free(&ctx);
  return 0;
}
