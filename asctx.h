#pragma once

#include "reloc.h"
#include "vec.h"
#include "dict.h"
#include "str.h"

struct asctx {
  struct vec rel_list; // list of relocation entries
  struct dict label2off;
  struct str bin_code;
};

static struct asctx asctx_create() {
  struct asctx ctx;
  ctx.rel_list = vec_create(sizeof(struct as_rel_s));
  ctx.label2off = dict_create();
  ctx.bin_code = str_create(256);
  return ctx;
}

static void asctx_define_label(struct asctx* ctx, const char* label, int off) {
  int rc = dict_put(&ctx->label2off, label, off);
  assert(rc == 1);
}

static void asctx_free(struct asctx* ctx) {
  vec_free(&ctx->rel_list);
  dict_free(&ctx->label2off);
  str_free(&ctx->bin_code);
}
