#pragma once

#include <stdio.h>
#include "dict.h"
#include "tokenizer.h"

enum OPERAND_TYPE {
  NUL = 0,
  REG = 1,
  IMM = 2,
  MEM = 3,
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
  const char* imm_sym; // symbol for immediate number. Used for relocation
  // }

  // MEM specific fields
  // {
  int32_t disp;
  const char* disp_sym; // symbol for displacement. Used for relocation
  // assume base and index registers to be 32 bit
  int base_regidx; // -1 if not exist
  int index_regidx; // -1 if not exist
  int log2scale; // -1 if not applicable
  // }
};

struct dict twoch2reg16;
struct dict twoch2reg8;

void operand_debug(struct operand* opd, int indent) {
  printf("%*s", indent, "");
  switch (opd->type) {
  case REG:
    printf("r%d\n", opd->nbit);
    break;
  case NUL:
    printf("-NO OPERAND-\n");
    break;
  case IMM:
    printf("imm%d\n", is_int8(opd->imm) ? 8 : 32);
    break;
  case MEM:
    printf("mem\n");
    break;
  default:
    assert(false && "operand_debug: can not reach here");
  }
}

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

/*
 * Return [0, 7] if the input reprends a 8 bit general purpose register;
 * return -1 otherwise
 */
static int parse_gpr8(const char* s) {
  if (strlen(s) != 3 || s[0] != '%') {
    return -1;
  }
  struct dict_entry* entry = dict_lookup(&twoch2reg8, s + 1);
  if (!entry->key) {
    return -1;
  } else {
    return entry->val;
  }
}

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

static int is_gpr8(struct operand* op) {
  return op->type == REG && op->nbit == 8;
}

static int is_gpr32(struct operand* op) {
  return op->type == REG && op->nbit == 32;
}

static int is_gpr(struct operand* op) {
  return op->type == REG;
}

static int is_imm(struct operand* op) {
  return op->type == IMM;
}

static int is_imm8(struct operand* op) {
  return op->type == IMM && !op->imm_sym && is_int8(op->imm);
}

static int is_mem(struct operand* op) {
  return op->type == MEM;
}

static bool is_rm8(struct operand* op) {
  return is_gpr8(op) || is_mem(op);
}

static bool is_rm32(struct operand* op) {
  return is_gpr32(op) || is_mem(op);
}

// check size suffix for mem operand
static bool is_rm32_check(struct operand* op, char opsuf) {
  return is_gpr32(op) || (is_mem(op) && opsuf == 'l');
}

// return 0 if the input string is an immediate number (without the '$' prefix)
// return a negative value otherwise.
static int parse_imm_noprefix(const char*s, int32_t* pimm) {
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

// return 0 if the input string is an immediate number (prefix by '$')
// return a negative value otherwise.
static int parse_imm(const char*s, int32_t* pimm, int* reloc_imm) {
  if (!*s || s[0] != '$') {
    return -1;
  }
  ++s;
  if (isalpha(*s) || *s == '_') {
    *reloc_imm = 1; 
    // TODO: validate that it's a vaild identifier?
    *pimm = 0;

    // NOTE: we can not add relocation entry here since we don't know
    // the offset for relocation yet.
    return 0;
  } else {
    *reloc_imm = 0;
    return parse_imm_noprefix(s, pimm);
  }
}

/*
 * Returns 0 if the input represents a memory operand; return -1 otherwise
 */
static int parse_mem(const char* s, struct operand* op) {
  // it's fine that we mark op as MEM too early.
  // It will be overriden later if this operand is not a memory operand.
  op->type = MEM;
  op->disp = 0;
  op->disp_sym = NULL;
  op->base_regidx = -1;
  op->index_regidx = -1;
  op->log2scale = -1;
  const char* end = s + strlen(s);
  const char* cur = s;
  int status;

  cur = skip_spaces(cur, end);
  if (cur == end) {
    return -1;
  }
  if (*cur != '(') {
    // displacement
    const char *disp_start = cur;
    while (*cur != '(' && *cur && !isspace(*cur)) {
      ++cur; 
    }
    char *dispstr = lenstrdup(disp_start, cur - disp_start);
    cur = skip_spaces(cur, end);
    assert(*cur == '(' || !*cur);

    if (isalpha(*dispstr) || *dispstr == '_') {
      // symbol as displacement
      op->disp_sym = dispstr;
      status = 0;
    } else {
      status = parse_imm_noprefix(dispstr, &op->disp);
      free(dispstr);
    }

    if (status != 0) {
      return -1;
    }
  }

  // only a displacement
  if (!*cur) {
    return 0;
  }

  /*
   * Only handles 2 cases ATM:
   * 1. (%gpr)
   * 2. (%grp1, %gpr2, scale)
   *  Need handle more complex case like a SIB missing base regiser but having index register.
   */
  assert(*cur == '(');
  ++cur;

  const char *base_reg_start = cur;
  while (*cur != ',' && *cur != ')') {
    ++cur;
  }
  if (cur - base_reg_start > 0) {
    char *base_reg_str = lenstrdup(base_reg_start, cur - base_reg_start);
    op->base_regidx = parse_gpr32(base_reg_str);
    free(base_reg_str);
    if (op->base_regidx < 0) {
      return -1;
    }
  }

  if (*cur == ')') {
    ++cur;
    cur = skip_spaces(cur, end);
    assert(*cur == '\0');
    // () is treated as invalid
    return op->base_regidx >= 0 ? 0 : -1;
  }
  assert(*cur == ',');

  // TODO: Assume both base, index ans scale exists
  ++cur;
  cur = skip_spaces(cur, end);
  const char *index_reg_start = cur;
  while (*cur != ',' && *cur != ')') {
    ++cur;
  }
  char *index_reg_str = lenstrdup(index_reg_start, cur - index_reg_start);
  op->index_regidx = parse_gpr32(index_reg_str);
  free(index_reg_str);
  if (op->index_regidx < 0 || *cur != ',') {
    return -1;
  }
  ++cur;

  // parse the scale
  cur = skip_spaces(cur, end);
  switch (*cur) {
  case '1':
    op->log2scale = 0;
    break;
  case '2':
    op->log2scale = 1;
    break;
  case '4':
    op->log2scale = 2;
    break;
  case '8':
    op->log2scale = 3;
    break;
  default:
    return -1;
  }
  ++cur;
  if (*cur != ')' || cur[1] != '\0') {
    return -1;
  }
  return 0;
}

static void operand_init(struct operand* op) {
  const char* repr = op->repr;
  int regidx = -1;
  int32_t imm;
  int reloc_imm;
  int status;
  assert(repr);

  if ((regidx = parse_gpr8(repr)) >= 0) {
    op->type = REG;
    op->regidx = regidx;
    op->nbit = 8;
  } else if ((regidx = parse_gpr32(repr)) >= 0) {
    op->type = REG;
    op->regidx = regidx;
    op->nbit = 32;
  } else if ((status = parse_imm(repr, &imm, &reloc_imm)) == 0) {
    op->type = IMM;
    op->imm = imm;
    op->imm_sym = NULL;
    if (reloc_imm) {
      op->imm_sym = strdup(repr + 1); 
    }

    // relocation implies 32 bits
    if ((imm >= -128 && imm <= 127) && !reloc_imm) {
      op->nbit = 8;
    } else {
      op->nbit = 32;
    }
  } else if ((status = parse_mem(repr, op)) == 0) {
    // nothing else to do here
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
  if (op->type == IMM) {
    if (op->imm_sym) {
      free((void*) op->imm_sym);
      op->imm_sym = NULL;
    }
  }
  if (op->type == MEM) {
    if (op->disp_sym) {
      free((void*) op->disp_sym);
      op->disp_sym = NULL;
    }
  }
}

/*
 * Return 0 on success and a negative value for failure.
 *
 * On success, set popd->repr the whole string representing the operand.
 * Rely on operand_init to futher parse the operand string to understand if
 * it's a register/immediate number/memory operand and what's its size.
 *
 * A success call will consume the separating comma if there is any.
 */
static int parse_operand(const char** pcur, const char* end, struct operand* popd) {
  const char* cur = *pcur;
  cur = skip_spaces(cur, end);
  if (cur == end) {
    return -1;
  }

  // finish parsing when reaching end or comma and balance is 0
  int balance = 0;
  const char* start = cur;
  const char* repr = NULL;
  while (true) {
    if (cur == end) {
      if (balance > 0) {
        return -1;
      } else {
        repr = lenstrdup(start, end - start);
        break;
      }
    }
    if (*cur == ',' && balance == 0) {
      repr = lenstrdup(start, cur - start);
      ++cur;
      break;
    }
    if (*cur == '(') {
      ++balance;
    } else if (*cur == ')') {
      assert(balance > 0);
      --balance;
    }
    ++cur;
  }

  assert(repr);
  popd->repr = repr;
  operand_init(popd);
  *pcur = cur;
  return 0;
}
