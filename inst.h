#pragma once

#include "asctx.h"
#include "dict.h"
#include "operand.h"
#include "util.h"

static void emit_opcode(struct asctx* ctx, uint8_t opc);
static void emit_modrm_sib_disp(struct asctx* ctx, int reg_opext, struct operand* rm_opd);
struct dict valid_instr_stem;

enum {
  OPC_mov,
  OPC_push,
  OPC_pop,
  OPC_sub,
  OPC_add,
  OPC_cmp,
  OPC_leave,
  OPC_ret,
  OPC_int,
  OPC_div,
  OPC_test,
  OPC_neg,
  OPC_lea,
  OPC_imul,
  OPC_nop,
  OPC_call,
  OPC_cmovo,
  OPC_jo,
  OPC_jmp,

  // have separate entries for movzbl, movzwl so we don't need
  // handle two character size bytes elsewhere.
  OPC_movzbl,
};

__attribute__((constructor)) static void init_valid_instr_stem() {
  valid_instr_stem = dict_create();
  dict_put(&valid_instr_stem, "mov", OPC_mov);
  dict_put(&valid_instr_stem, "push", OPC_push);
  dict_put(&valid_instr_stem, "pop", OPC_pop);
  dict_put(&valid_instr_stem, "sub", OPC_sub);
  dict_put(&valid_instr_stem, "add", OPC_add);
  dict_put(&valid_instr_stem, "cmp", OPC_cmp);
  dict_put(&valid_instr_stem, "leave", OPC_leave);
  dict_put(&valid_instr_stem, "ret", OPC_ret);
  dict_put(&valid_instr_stem, "int", OPC_int);
  dict_put(&valid_instr_stem, "div", OPC_div);
  dict_put(&valid_instr_stem, "test", OPC_test);
  dict_put(&valid_instr_stem, "neg", OPC_neg);
  dict_put(&valid_instr_stem, "lea", OPC_lea);
  dict_put(&valid_instr_stem, "imul", OPC_imul);
  dict_put(&valid_instr_stem, "nop", OPC_nop);
  dict_put(&valid_instr_stem, "movzbl", OPC_movzbl);
  dict_put(&valid_instr_stem, "call", OPC_call);
  dict_put(&valid_instr_stem, "cmovo", OPC_cmovo);
  dict_put(&valid_instr_stem, "jmp", OPC_jmp);
  dict_put(&valid_instr_stem, "jo", OPC_jo);
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

#if 0
static void handle_TEMP1(struct asctx* ctx, struct operand* opd, char sizesuf) {
  assert(false && "handle_TEMP1");
}

static void handle_TEMP2(struct asctx* ctx, struct operand *o1, struct operand *o2, char sizesuf) {
  assert(false && "handle_TEMP2");
}
#endif

/*
 * Handle both jmp and jcc.
 * For jmp, cc_opcode_off should be -1;
 * For jcc, cc_opcode_off will reside [0, 0xF]
 */
static void handle_jmp(struct asctx* ctx, struct operand* o1, int sizesuf, int cc_opcode_off) {
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

  // TODO: avoid duplicate the following 3 line with handle_call
  assert(is_mem(o1));
  assert(o1->disp_sym);
  assert(o1->base_regidx < 0 && o1->index_regidx < 0 && o1->log2scale < 0);
  const char* label = o1->disp_sym;
  struct dict_entry* label_entry = dict_lookup(&ctx->label2off, label);

  // TODO: use relocation to handle label
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
 * Return -1 if not a cc;
 * Return cc opcode offset if it's a cc. cc opcode offset resides in [0, 0xF]
 */
static int is_cc(const char* s) {
  struct dict_entry* entry = dict_lookup(&cc2opcodeoff, s);
  if (entry->key) {
    return entry->val;
  } else {
    return -1;
  }
}

/*
 * Return -1 if not a jcc op;
 * Return cc opcode offset if it's a jcc op. cc opcode offset resides in [0, 0xF]
 */
static int is_jcc(const char* opstr) {
  if (strlen(opstr) < 2 || opstr[0] != 'j') {
    return -1;
  }
  return is_cc(opstr + 1);
}

static void handle_call(struct asctx* ctx, struct operand* opd, char sizesuf) {
  assert(is_mem(opd));
  assert(opd->disp_sym);
  assert(opd->base_regidx < 0 && opd->index_regidx < 0 && opd->log2scale < 0);
  const char* func_name = opd->disp_sym;
  str_append(&ctx->bin_code, 0xe8);
  int offset = ctx->bin_code.len;
  // addend is stored in the as_rel_s rather than inplace
  str_append_i32(&ctx->bin_code, 0);

  asctx_add_relocation(ctx, func_name, offset, R_386_PC32);
}

// emit disp as 32bit number and handle relocation if needed
static void emit_disp32(struct asctx* ctx, struct operand* opd) {
  assert(opd->type == MEM);
  if (opd->disp_sym) {
    asctx_add_relocation(ctx, opd->disp_sym, ctx->bin_code.len, R_386_32);
  }
  str_append_i32(&ctx->bin_code, opd->disp);
}

enum DISP_STATE {
  DISP_NONE,
  DISP_8bits,
  DISP_32bits,
};

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
  int disp_state;
  if (mem_opd.disp == 0) {
    disp_state = DISP_NONE;
  } else if (is_int8(mem_opd.disp)) {
    disp_state = DISP_8bits;
  } else {
    disp_state = DISP_32bits;
  }

  bool has_sib = (mem_opd.index_regidx >= 0 || mem_opd.log2scale >= 0);
  bool has_base = mem_opd.base_regidx >= 0;
  if (!has_base && !has_sib) {
    // only displacement
    str_append(&ctx->bin_code, 0x00 | (reg_opext << 3) | 5);
    emit_disp32(ctx, &mem_opd);
    return;
  }

  if (has_sib) {
    assert(mem_opd.index_regidx >= 0 && mem_opd.log2scale >= 0);
  }

  int rmbits = has_sib ? 4 : mem_opd.base_regidx;
  if (!has_sib) {
    assert(mem_opd.base_regidx != 4);
    if (disp_state == DISP_NONE) {
      assert(mem_opd.base_regidx != 5);
    }
  }

  // some validation
  if (disp_state == DISP_NONE) {
    assert(mem_opd.base_regidx != 5); // %ebp slot is special in this case
  }

  if (has_sib) {
    assert(mem_opd.index_regidx != 4); // %esp slot is special in this case
  }

  if (mem_opd.disp == 0 || mem_opd.base_regidx < 0) {
    // mod 00
    str_append(&ctx->bin_code, 0x00 | (reg_opext << 3) | rmbits);
  } else if (is_int8(mem_opd.disp)) {
    // mod 01
    str_append(&ctx->bin_code, 0x40 | (reg_opext << 3) | rmbits);
  } else {
    // mod 10
    str_append(&ctx->bin_code, 0x80 | (reg_opext << 3) | rmbits);
  }

  if (has_sib) {
    int base_regidx = mem_opd.base_regidx >= 0 ? mem_opd.base_regidx : 5;
    // log2scale -- index_reg -- base_reg
    assert(mem_opd.index_regidx != 4);
    assert(mem_opd.base_regidx != 5);
    str_append(&ctx->bin_code, (mem_opd.log2scale << 6) | (mem_opd.index_regidx << 3) | base_regidx);
  }

  // displacement
  if (mem_opd.disp == 0 && has_base) {
  } else if (is_int8(mem_opd.disp) && has_base) {
    // emit the disp8
    str_append(&ctx->bin_code, (int8_t) mem_opd.disp);
  } else {
    // emit the disp32
    emit_disp32(ctx, &mem_opd);
  }
}

/*
 * No matter if opd->imm is in int8 range, emit it as imm32.
 */
static void emit_imm32(struct asctx* ctx, struct operand* opd) {
  assert(opd->type == IMM);
  str_append_i32(&ctx->bin_code, opd->imm);
  if (opd->imm_sym) {
    // need relocation
    asctx_add_relocation(ctx, opd->imm_sym, ctx->bin_code.len - 4, R_386_32);
  }
}

static void emit_opcode(struct asctx* ctx, uint8_t opc) {
  str_append(&ctx->bin_code, opc);
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
    emit_imm32(ctx, o1);
  } else if (is_mem(o1) && is_gpr32(o2)) {
    handle_load_i32(ctx, o1, o2);
  } else if (is_gpr32(o1) && is_mem(o2)) {
    handle_store_i32(ctx, o1, o2);
  } else if (is_imm(o1) && is_mem(o2) && sizesuf == 'l') {
    // mov imm to r/m32
    str_append(&ctx->bin_code, 0xc7);
    emit_modrm_sib_disp(ctx, 0, o2); // emit modrm, sib, displacement
    str_append_i32(&ctx->bin_code, o1->imm);
  } else if (is_imm(o1) && is_mem(o2) && sizesuf == 'b') {
    // mov imm to r/m8
    emit_opcode(ctx, 0xc6);
    emit_modrm_sib_disp(ctx, 0, o2);
    str_append(&ctx->bin_code, o1->imm);
  } else {
    printf("handle_mov %s %s\n", o1->repr, o2->repr);
    assert(false && "handle mov");
  }
}

static void handle_push(struct asctx* ctx, struct operand* opd, char sizesuf) {
  if (is_gpr32(opd)) {
    str_append(&ctx->bin_code, 0x50 + opd->regidx);
  } else if (is_mem(opd)) { // imply 32 bit
    str_append(&ctx->bin_code, 0xff);
    emit_modrm_sib_disp(ctx, 6, opd);
  } else if (is_imm(opd)) {
    // TODO: we could use less bytes for imm8
    str_append(&ctx->bin_code, 0x68);
    emit_imm32(ctx, opd);
  } else {
    assert(false && "handle_push");
  }
}

static void handle_pop(struct asctx* ctx, struct operand* opd, char sizesuf) {
  if (is_gpr32(opd)) {
    str_append(&ctx->bin_code, 0x58 + opd->regidx);
  } else {
    assert(false && "handle_pop");
  }
}

static void handle_div(struct asctx* ctx, struct operand* opd, char sizesuf) {
  if (is_rm32_check(opd, sizesuf)) {
    emit_opcode(ctx, 0xf7);
    emit_modrm_sib_disp(ctx, 6, opd);
  } else {
    assert(false && "handle_div");
  }
}

static void handle_int(struct asctx* ctx, struct operand* opd, char sizesuf) {
  // the immediate value for int instruction is special since it's interpreted
  // as an unsigend 8 bit integer. E.g. $0x80 is interpreted as 128 rather than
  // -128 if interpreted as int8_t.
  assert(is_imm(opd));
  assert(!opd->imm_sym);
  assert(opd->imm >= 0 && opd->imm <= 255);
  str_append(&ctx->bin_code, 0xcd); 
  str_append(&ctx->bin_code, (uint8_t) opd->imm);
}

static void handle_sub(struct asctx* ctx, struct operand *o1, struct operand *o2, char sizesuf) {
  if (is_rm32(o1) && is_gpr32(o2)) {
    emit_opcode(ctx, 0x2b);
    emit_modrm_sib_disp(ctx, o2->regidx, o1);
  } else if (is_imm8(o1) && is_gpr32(o2)) {
    str_append(&ctx->bin_code, 0x83);
    emit_modrm_sib_disp(ctx, 5, o2);
    str_append(&ctx->bin_code, (int8_t) o1->imm);
  } else {
    assert(false && "handle_sub");
  }
}

static void handle_add(struct asctx* ctx, struct operand *o1, struct operand *o2, char sizesuf) {
  if (is_gpr32(o1) && is_rm32(o2)) {
    str_append(&ctx->bin_code, 0x01);
    emit_modrm_sib_disp(ctx, o1->regidx, o2);
  } else if (is_imm8(o1) && is_rm32_check(o2, sizesuf)) {
    str_append(&ctx->bin_code, 0x83);
    emit_modrm_sib_disp(ctx, 0, o2);
    str_append(&ctx->bin_code, (int8_t) o1->imm);
  } else {
    assert(false && "handle_add");
  }
}

static void handle_cmp(struct asctx* ctx, struct operand *o1, struct operand *o2, char sizesuf) {
  if (is_rm32(o1) && is_gpr32(o2)) {
    // r32 - r/m32
    emit_opcode(ctx, 0x3b);
    emit_modrm_sib_disp(ctx, o2->regidx, o1);
  } else if (is_imm8(o1) && is_rm32_check(o2, sizesuf)) {
    str_append(&ctx->bin_code, 0x83);
    emit_modrm_sib_disp(ctx, 7, o2);
    str_append(&ctx->bin_code, (int8_t) o1->imm);
  } else {
    assert(false && "handle_cmp");
  }
}

static void handle_test(struct asctx* ctx, struct operand *o1, struct operand *o2, char sizesuf) {
  if (is_gpr32(o1) && is_rm32(o2)) {
    emit_opcode(ctx, 0x85);
    emit_modrm_sib_disp(ctx, o1->regidx, o2);
  } else if (is_gpr8(o1) && is_rm8(o2)) {
    emit_opcode(ctx, 0x84);
    emit_modrm_sib_disp(ctx, o1->regidx, o2);
  } else {
    assert(false && "handle_test");
  }
}

static void handle_neg(struct asctx* ctx, struct operand* opd, char sizesuf) {
  if (is_rm32_check(opd, sizesuf)) {
    emit_opcode(ctx, 0xf7);
    emit_modrm_sib_disp(ctx, 3, opd);
  } else {
    assert(false && "handle_neg");
  }
}

static void handle_lea(struct asctx* ctx, struct operand *o1, struct operand *o2, char sizesuf) {
  assert(is_rm32(o1) && is_gpr32(o2));
  emit_opcode(ctx, 0x8d);
  emit_modrm_sib_disp(ctx, o2->regidx, o1);
}

static void handle_movzbl(struct asctx* ctx, struct operand *o1, struct operand *o2, char sizesuf) {
  assert(is_rm8(o1) && is_gpr32(o2));
  emit_opcode(ctx, 0x0f);
  emit_opcode(ctx, 0xb6);
  emit_modrm_sib_disp(ctx, o2->regidx, o1);
}

static void handle_imul(struct asctx* ctx, struct operand *o1, struct operand *o2, char sizesuf) {
  if (is_rm32(o1) && is_gpr32(o2)) {
    emit_opcode(ctx, 0x0f);
    emit_opcode(ctx, 0xaf);
    emit_modrm_sib_disp(ctx, o2->regidx, o1);
  } else {
    assert(false && "handle_imul");
  }
}

static void handle_cmovo(struct asctx* ctx, struct operand *o1, struct operand *o2, char sizesuf, int cc_opcode_off) {
  printf("cc_opcode_off is %d\n", cc_opcode_off);
  assert(cc_opcode_off >= 0 && cc_opcode_off <= 0xf);
  assert(is_rm32(o1) && is_gpr32(o2));
  emit_opcode(ctx, 0x0f);
  emit_opcode(ctx, 0x40 + cc_opcode_off);
  emit_modrm_sib_disp(ctx, o2->regidx, o1);
}

/*
 * cc_opcode_off is only valid for setcc, jcc, cmovcc instructions.
 * For others instructions, it should be -1.
 */
static void handle_instr(struct asctx* ctx, const char* opstem, struct operand *o1, struct operand *o2, char sizesuf, int cc_opcode_off) {
  int opc = dict_lookup_nomiss(&valid_instr_stem, opstem);
  switch (opc) {
    case OPC_push:
      handle_push(ctx, o1, sizesuf);
      break;
    case OPC_pop:
      handle_pop(ctx, o1, sizesuf);
      break;
    case OPC_mov:
      handle_mov(ctx, o1, o2, sizesuf);
      break;
    case OPC_sub:
      handle_sub(ctx, o1, o2, sizesuf);
      break;
    case OPC_add:
      handle_add(ctx, o1, o2, sizesuf);
      break;
    case OPC_cmp:
      handle_cmp(ctx, o1, o2, sizesuf);
      break;
    case OPC_leave:
      str_append(&ctx->bin_code, 0xc9);
      break;
    case OPC_ret:
      str_append(&ctx->bin_code, 0xc3);
      break;
    case OPC_nop:
      str_append(&ctx->bin_code, 0x90);
      break;
    case OPC_int:
      handle_int(ctx, o1, sizesuf);
      break;
    case OPC_div:
      handle_div(ctx, o1, sizesuf);
      break;
    case OPC_test:
      handle_test(ctx, o1, o2, sizesuf);
      break;
    case OPC_neg:
      handle_neg(ctx, o1, sizesuf);
      break;
    case OPC_lea:
      handle_lea(ctx, o1, o2, sizesuf);
      break;
    case OPC_movzbl:
      handle_movzbl(ctx, o1, o2, sizesuf);
      break;
    case OPC_imul:
      handle_imul(ctx, o1, o2, sizesuf);
      break;
    case OPC_call:
      handle_call(ctx, o1, sizesuf);
      break;
    case OPC_cmovo:
      handle_cmovo(ctx, o1, o2, sizesuf, cc_opcode_off);
      break;
    case OPC_jo: case OPC_jmp:
      handle_jmp(ctx, o1, sizesuf, cc_opcode_off);
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
