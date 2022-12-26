#pragma once

#include <stdio.h>
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

  // IMM specific fields
  // {
  int32_t imm;
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

static int is_gpr32(struct operand* op) {
  return op->type == REG && op->nbit == 32;
}

static int is_imm(struct operand* op) {
  return op->type == IMM;
}

// return 0 if the input string is an immediate number (prefix by '$')
// return a negative value otherwise.
//
// TODO: may need generalize this function to handle immediate numbers
// not prefixed by '$'
static int parse_imm(const char*s, int32_t* pimm) {
  if (!*s || s[0] != '$') {
    return -1;
  }
  ++s;

  // only handle decimal or hexadecimal. (i.e. 012 will be treatd as 12 in decimal rather than 10)
  int sign = 1; // default
  if (*s == '+' || *s == '-') {
    if (*s == '-') {
      sign = -1;
    }
    ++s;
  }
  int base = 10;
  if (strlen(s) > 2 && s[0] == '0' && tolower(s[1]) == 'x') {
    base = 16;
    s += 2;
  }
  if (!*s) {
    return -1;
  }
  // TODO: handle 32bit overflow
  int significant = 0;
  char ch;
  int dig;
  for (; (ch = *s); ++s) {
    ch = tolower(ch);
    if (ch >= '0' && ch <= '9') {
      dig = (ch - '0');
    } else if (base == 16 && ch >= 'a' && ch <= 'f') {
      dig = ch - 'a' + 10;
    } else {
      return -1;
    }
    significant = significant * base + dig;
  }
  *pimm = sign * significant;
  return 0;
}

static void operand_init(struct operand* op) {
  const char* repr = op->repr;
  int regidx = -1;
  int32_t imm;
  int status;
  assert(repr);

  if ((regidx = parse_gpr32(repr)) >= 0) {
    op->type = REG;
    op->regidx = regidx;
    op->nbit = 32;
  } else if ((status = parse_imm(repr, &imm)) == 0) {
    op->type = IMM;
    op->imm = imm;

    if (imm >= -128 && imm <= 127) {
      op->nbit = 8;
    } else {
      op->nbit = 32;
    }
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
