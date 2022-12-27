#pragma once

#include "asctx.h"
#include "dict.h"
#include "operand.h"
#include "util.h"

struct dict valid_instr_stem;

enum {
  OPC_mov,
  OPC_push,
  OPC_sub,
};

__attribute__((constructor)) static void init_valid_instr_stem() {
  valid_instr_stem = dict_create();
  dict_put(&valid_instr_stem, "mov", OPC_mov);
  dict_put(&valid_instr_stem, "push", OPC_push);
  dict_put(&valid_instr_stem, "sub", OPC_sub);
}

bool is_valid_instr_stem(const char* _s, int len) {
  const char* s = lenstrdup(_s, len);
  struct dict_entry *entry = dict_lookup(&valid_instr_stem, s);
  free((void*) s);
  return entry->key != NULL;
}

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

static void handle_call(struct asctx* ctx, const char* func_name) {
  str_append(&ctx->bin_code, 0xe8);
  int offset = ctx->bin_code.len;
  str_append_i32(&ctx->bin_code, 0);

  struct as_rel_s item;
  item.own_buf = 1;
  item.offset = offset;
  item.rel_type = R_386_PC32;
  item.sym = strdup(func_name);
  item.symlen = strlen(func_name);;
  item.addend = -4;
  vec_append(&ctx->rel_list, &item);
}

/*
 * rm_opd should either be a reigsrer or an memory operand
 */
static void emit_modrm_sib_disp(struct asctx* ctx, int reg_opext, struct operand* rm_opd) {
  assert(is_gpr(rm_opd) || is_mem(rm_opd));

  if (is_gpr(rm_opd)) {
    // ModR/M byte:
    uint8_t mod = 0xc0 | (reg_opext << 3) | (rm_opd->regidx);
    str_append(&ctx->bin_code, mod);
    return;
  }

  struct operand mem_opd = *rm_opd;

  // TODO: only handle simple case of disp(%base) right now
  assert(mem_opd.base_regidx >= 0);
  assert(mem_opd.index_regidx < 0);
  assert(mem_opd.log2scale < 0);

  if (mem_opd.disp == 0) {
    // mod 00
    assert(mem_opd.base_regidx != 4 && mem_opd.base_regidx != 5);
    str_append(&ctx->bin_code, 0x00 | (reg_opext << 3) | mem_opd.base_regidx);
  } else if (is_int8(mem_opd.disp)) {
    // mod 01
    // ModRM: 01 reg_opd.regidx mem_opd.base_regidx
    assert(mem_opd.base_regidx != 4); // TODO: handle SIB case
    str_append(&ctx->bin_code, 0x40 | (reg_opext << 3) | mem_opd.base_regidx);

    // emit the disp8
    str_append(&ctx->bin_code, (int8_t) mem_opd.disp);
  } else {
    // mod 10
    assert(mem_opd.base_regidx != 4); // TODO: handle SIB case
    str_append(&ctx->bin_code, 0x80 | (reg_opext << 3) | mem_opd.base_regidx);

    // emit the disp32
    str_append_i32(&ctx->bin_code, mem_opd.disp);
  }
}

static void handle_loadstore_i32(struct asctx* ctx, struct operand* o1, struct operand* o2) {
  struct operand reg_opd, mem_opd;
  if (is_mem(o1)) {
    // load
    str_append(&ctx->bin_code, 0x8b);
    reg_opd = *o2;
    mem_opd = *o1;
  } else {
    // store
    str_append(&ctx->bin_code, 0x89);
    reg_opd = *o1;
    mem_opd = *o2;
  }
  assert(is_mem(&mem_opd));
  assert(is_gpr32(&reg_opd));

  emit_modrm_sib_disp(ctx, reg_opd.regidx, &mem_opd);
}

static void handle_load_i32(struct asctx* ctx, struct operand* o1, struct operand* o2) {
  assert(is_mem(o1));
  assert(is_gpr32(o2));
  handle_loadstore_i32(ctx, o1, o2);
}

static void handle_store_i32(struct asctx* ctx, struct operand* o1, struct operand* o2) {
  assert(is_gpr32(o1));
  assert(is_mem(o2));
  handle_loadstore_i32(ctx, o1, o2);
}

static void handle_mov(struct asctx* ctx, struct operand* o1, struct operand* o2, char sizesuf) {
  if (is_gpr32(o1) && is_gpr32(o2)) {
    // mov gpr32_0, gpr32_1
    // there are 2 alternative ways to encode this instruction.
    // We can encode this as a load (opcode 0x8b) or store (opcode 0x89).
    //
    // sas uniformly encode this as a store.
    // When interpreted as a store, gpr32_0 will be encoded in reg bits in ModR/M byte,
    // while gpr32_1 will be encoded in r/m bits in ModR/M byte.
    str_append(&ctx->bin_code, 0x89);
    emit_modrm_sib_disp(ctx, o1->regidx, o2);
  } else if (is_imm(o1) && is_gpr32(o2)) {
    // move imm to gpr32
    // always encode imm as imm32 for simplicity
    str_append(&ctx->bin_code, 0xb8 + o2->regidx);
    str_append_i32(&ctx->bin_code, o1->imm);
  } else if (is_mem(o1) && is_gpr32(o2)) {
    handle_load_i32(ctx, o1, o2);
  } else if (is_gpr32(o1) && is_mem(o2)) {
    handle_store_i32(ctx, o1, o2);
  } else if (is_imm(o1) && is_mem(o2) && sizesuf == 'l') {
    // mov imm to r/m32
    str_append(&ctx->bin_code, 0xc7);
    emit_modrm_sib_disp(ctx, 0, o2); // emit modrm, sib, displacement
    str_append_i32(&ctx->bin_code, o1->imm);
  } else {
    printf("handle_mov %s %s\n", o1->repr, o2->repr);
    assert(false && "handle mov");
  }
}

static void handle_push(struct asctx* ctx, struct operand* opd, char sizebuf) {
  if (is_gpr32(opd)) {
    str_append(&ctx->bin_code, 0x50 + opd->regidx);
  } else {
    assert(false && "handle_push");
  }
}

static void handle_sub(struct asctx* ctx, struct operand *o1, struct operand *o2, char sizesuf) {
  if (is_imm8(o1) && is_gpr32(o2)) {
    str_append(&ctx->bin_code, 0x83);
    emit_modrm_sib_disp(ctx, 5, o2);
    str_append(&ctx->bin_code, (int8_t) o1->imm);
  } else {
    assert(false && "handle_sub");
  }
}

static void handle_instr(struct asctx* ctx, const char* opstem, struct operand *o1, struct operand *o2, char sizesuf) {
  int opc = dict_lookup_nomiss(&valid_instr_stem, opstem);
  switch (opc) {
    case OPC_push:
      handle_push(ctx, o1, sizesuf);
      break;
    case OPC_mov:
      handle_mov(ctx, o1, o2, sizesuf);
      break;
    case OPC_sub:
      handle_sub(ctx, o1, o2, sizesuf);
      break;
    default:
      printf("handle instruction %s", opstem);
      if (sizesuf) {
        printf("(%c)", sizesuf);
      }
      printf(":\n");
      operand_debug(o1, 2);
      operand_debug(o2, 2);
      assert(false && "handle_instr ni");
      break;
  }
}
