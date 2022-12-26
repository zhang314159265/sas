#pragma once

#include "dict.h"

struct dict twoch2reg16;
struct dict twoch2reg8;

__attribute__((constructor)) static void init_twoch2reg() {
  twoch2reg16 = dict_create();
  dict_put(&twoch2reg16, "ax", 0);
  dict_put(&twoch2reg16, "cx", 1);
  dict_put(&twoch2reg16, "dx", 2);
  dict_put(&twoch2reg16, "bx", 3);
  dict_put(&twoch2reg16, "sp", 4);
  dict_put(&twoch2reg16, "bp", 5);
  dict_put(&twoch2reg16, "si", 6);
  dict_put(&twoch2reg16, "di", 7);

  twoch2reg8 = dict_create();
  dict_put(&twoch2reg8, "al", 0);
  dict_put(&twoch2reg8, "cl", 1);
  dict_put(&twoch2reg8, "dl", 2);
  dict_put(&twoch2reg8, "bl", 3);
  dict_put(&twoch2reg8, "ah", 4);
  dict_put(&twoch2reg8, "ch", 5);
  dict_put(&twoch2reg8, "dh", 6);
  dict_put(&twoch2reg8, "bh", 7);
}

enum OPERAND_TYPE {
  REG = 0,
  IMM = 1,
  MEM = 2,
};

// can represent a imm/reg/mem operand
struct operand {
  const char* repr; // optional string representation of the operand

  int type; // OPERAND_TYPE
  int nbit; // -1 if unknown

  // REG specific fields 
  // {
  int regidx;
  // }
};

/*
 * Return [0, 7] if the input reprends a 32 bit general purpose register;
 * return -1 otherwise
 */
static int parse_gpr32(const char* s) {
  if (strlen(s) != 4 || s[0] != '%' || s[1] != 'e') {
    return -1;
  }
  struct dict_entry* entry = dict_lookup(&twoch2reg16, s + 2);
  if (!entry->key) {
    return -1;
  } else {
    return entry->val;
  }
}

static int is_grp32(struct operand* op) {
  return op->type == REG && op->nbit == 32;
}

static void operand_init(struct operand* op) {
  const char* repr = op->repr;
  int regidx = -1;
  assert(repr);

  if ((regidx = parse_gpr32(repr)) >= 0) {
    op->type = REG;
    op->regidx = regidx;
    op->nbit = 32;
  } else {
    printf("not yet suport initializing operand with '%s'\n", op->repr);
    assert(false && "operand_init ni");
  }
}

static void operand_free(struct operand* op) {
  if (op->repr) {
    free((void*) op->repr);
    op->repr = NULL;
  }
}
