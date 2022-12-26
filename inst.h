#pragma once

#include "asctx.h"
#include "dict.h"

struct dict cc2opcodeoff;

 __attribute__((constructor)) static void init_cc2opcodeoff() {
  cc2opcodeoff = dict_create();
  dict_put(&cc2opcodeoff, "o", 0x0);
  dict_put(&cc2opcodeoff, "no", 0x1);
  dict_put(&cc2opcodeoff, "b", 0x2);
  dict_put(&cc2opcodeoff, "c", 0x2);
  dict_put(&cc2opcodeoff, "nae", 0x2);
  dict_put(&cc2opcodeoff, "ae", 0x3);
  dict_put(&cc2opcodeoff, "nb", 0x3);
  dict_put(&cc2opcodeoff, "nc", 0x3);
  dict_put(&cc2opcodeoff, "e", 0x4);
  dict_put(&cc2opcodeoff, "z", 0x4);
  dict_put(&cc2opcodeoff, "ne", 0x5);
  dict_put(&cc2opcodeoff, "nz", 0x5);
  dict_put(&cc2opcodeoff, "be", 0x6);
  dict_put(&cc2opcodeoff, "na", 0x6);
  dict_put(&cc2opcodeoff, "a", 0x7);
  dict_put(&cc2opcodeoff, "nbe", 0x7);
  dict_put(&cc2opcodeoff, "s", 0x8);
  dict_put(&cc2opcodeoff, "ns", 0x9);
  dict_put(&cc2opcodeoff, "p", 0xa);
  dict_put(&cc2opcodeoff, "pe", 0xa);
  dict_put(&cc2opcodeoff, "np", 0xb);
  dict_put(&cc2opcodeoff, "po", 0xb);
  dict_put(&cc2opcodeoff, "l", 0xc);
  dict_put(&cc2opcodeoff, "nge", 0xc);
  dict_put(&cc2opcodeoff, "ge", 0xd);
  dict_put(&cc2opcodeoff, "nl", 0xd);
  dict_put(&cc2opcodeoff, "le", 0xe);
  dict_put(&cc2opcodeoff, "ng", 0xe);
  dict_put(&cc2opcodeoff, "g", 0xf);
  dict_put(&cc2opcodeoff, "nle", 0xf);
}

/*
 * Handle both jmp and jcc.
 * For jmp, cc_opcode_off should be -1;
 * For jcc, cc_opcode_off will reside [0, 0xF]
 */
static void handle_jmp(struct asctx* ctx, const char* label, int cc_opcode_off) {
  struct dict_entry* label_entry = dict_lookup(&ctx->label2off, label);

  // XXX to simplify the code, we always use near jmp (disp32) rather than
  // short jmp (disp8)
  //
  // Must append the opcode before grabbing the offset to patch.
  if (cc_opcode_off >= 0) {
    // append jcc opcode
    str_append(&ctx->bin_code, 0x0f);
    str_append(&ctx->bin_code, 0x80 + cc_opcode_off);
  } else {
    str_append(&ctx->bin_code, 0xe9);
  }
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

/*
 * Return -1 if not a jcc op;
 * Return cc opcode offset if it's a jcc op. cc opcode offset resides in [0, 0xF]
 */
static int is_jcc(const char* opstr) {
  int opoff = -1;
  if (strlen(opstr) < 2 || opstr[0] != 'j') {
    return opoff;
  }
  struct dict_entry* entry = dict_lookup(&cc2opcodeoff, opstr + 1);
  if (entry->key) {
    return entry->val;
  } else {
    return opoff;
  }
}
