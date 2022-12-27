#pragma once

#include "reloc.h"
#include "vec.h"
#include "dict.h"
#include "str.h"

struct label_patch_entry {
  const char* label;
  int off;
};

struct asctx {
  struct vec rel_list; // list of relocation entries
  struct dict label2off;
  struct str bin_code;

  struct vec label_patch_list; // similar to relocation but use label definition rather than symbol to resolve
};

static struct asctx asctx_create() {
  struct asctx ctx;
  ctx.rel_list = vec_create(sizeof(struct as_rel_s));
  ctx.label2off = dict_create();
  ctx.bin_code = str_create(256);
  ctx.label_patch_list = vec_create(sizeof(struct label_patch_entry));
  return ctx;
}

static void asctx_define_label(struct asctx* ctx, const char* label, int off) {
  int rc = dict_put(&ctx->label2off, label, off);
  assert(rc == 1);
}

static void asctx_resolve_label_patch(struct asctx* ctx) {
  for (int i = 0; i < ctx->label_patch_list.len; ++i) {
    struct label_patch_entry* patch_entry = vec_get_item(&ctx->label_patch_list, i);
    uint32_t label_off = dict_lookup_nomiss(&ctx->label2off, patch_entry->label);
    uint32_t patch_off = patch_entry->off;
    *(uint32_t*)(ctx->bin_code.buf + patch_off) = label_off - (patch_off + 4);
  }
}

static void asctx_add_relocation(struct asctx* ctx, const char* symname, int offset, int reloc_type) {
  struct as_rel_s item;
  item.own_buf = 1;
  item.offset = offset;
  item.rel_type = reloc_type;
  item.sym = strdup(symname);
  item.symlen = strlen(symname);;
  if (reloc_type == R_386_32) {
    item.addend = 0;
  } else if (reloc_type == R_386_PC32) {
    item.addend = -4;
  }
  vec_append(&ctx->rel_list, &item);
}

static void asctx_free(struct asctx* ctx) {
  for (int i = 0; i < ctx->rel_list.len; ++i) {
    struct as_rel_s *item = vec_get_item(&ctx->rel_list, i);
    if (item->own_buf) {
      free((void*) item->sym);
    }
  }
  vec_free(&ctx->rel_list);
  dict_free(&ctx->label2off);
  str_free(&ctx->bin_code);

  for (int i = 0; i < ctx->label_patch_list.len; ++i) {
    struct label_patch_entry* item = vec_get_item(&ctx->label_patch_list, i);
    free((void*) item->label);
  }
  vec_free(&ctx->label_patch_list);
}
