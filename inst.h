#pragma once

#include "asctx.h"

static void handle_jmp(struct asctx* ctx, const char* label) {
  struct dict_entry* label_entry = dict_lookup(&ctx->label2off, label);

  // XXX to simplify the code, we always use near jmp (disp32) rather than
  // short jmp (disp8)
  //
  // Must append the opcode before grabbing the offset to patch.
  str_append(&ctx->bin_code, 0xe9);
  uint32_t off = ctx->bin_code.len;
  uint32_t val;
  if (label_entry->key) {
    // label already defined
    val = label_entry->val - (off + 4);
  } else {
    val = 0;
    struct label_patch_entry patch_entry;
    patch_entry.label = strdup(label);
    patch_entry.off = off;
    vec_append(&ctx->label_patch_list, &patch_entry);
  }

  str_append_i32(&ctx->bin_code, val);
}
