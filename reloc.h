#pragma once

#include "str.h"
#include "sym.h"

// define a struct for relocation rather than reusing Elf32_Rel to make it easier
// to handle symbol name.

struct as_rel_s {
  int32_t offset;
  uint8_t rel_type;
  const char* sym;
  int symlen;
  int32_t addend;
};

enum rel_parse_str_stage {
  PARSE_PREFIX = 0,
  PARSE_RELOC_TYPE = 1,
  PARSE_SYM = 2,
  PARSE_ADDEND = 3,
  PARSE_DONE = 4,
};

// refer to glibc elf/elf.h
#define R_386_32 1
#define R_386_PC32 2

uint8_t rel_parse_type(const char* typestr, int len) {
  const char* supported_typestr[] = {
    "R_386_32",
    "R_386_PC32",
  };
  int supported_type[] = {
    R_386_32,
    R_386_PC32,
  };
  assert(sizeof(supported_typestr) / sizeof(*supported_typestr) == sizeof(supported_type) / sizeof(*supported_type));
  for (int i = 0; i < sizeof(supported_typestr) / sizeof(*supported_typestr); ++i) {
    const char* expected_typestr = supported_typestr[i];
    if (strlen(expected_typestr) == len && memcmp(expected_typestr, typestr, len) == 0) {
      return supported_type[i];
    }
  }
  assert(false && "unrecognized relocation type");
}

int32_t rel_parse_addend(const char* text, int len) {
  const char *end = text + len;
  assert(len > 0);
  int32_t sign = 1;
  if (*text == '+') {
    ++text;
  } else if (*text == '-') {
    sign = -1;
    ++text;
  }
  int32_t val = 0;
  
  assert(text != end);
  while (text != end) {
    // allow extra leading 0 and still treat 012 as decimal
    assert(isdigit(*text));
    val = val * 10 + (*text - '0');
    ++text;
  }
  return val * sign;
}

/*
 * The format of a relocation string is:
 * "REL <RELOC_TYPE> <SYM> <ADDEND>"
 */
struct as_rel_s rel_parse_str(uint32_t offset, const char* text_begin, const char* text_end) {
  struct as_rel_s rel_entry;
  rel_entry.offset = offset;

  const char* text_ptr = text_begin;
  int state = PARSE_PREFIX;
  while (text_ptr != text_end) {
    assert(state != PARSE_DONE);
    assert(*text_ptr != ' ');

    const char* text_nxt = text_ptr;
    while (text_nxt != text_end && !isspace(*text_nxt)) {
      ++text_nxt;
    }

    // handle the word
    switch (state) {
    case PARSE_PREFIX:
      assert(text_nxt - text_ptr == 3 && memcmp(text_ptr, "REL", 3) == 0);
      state = PARSE_RELOC_TYPE;
      break;
    case PARSE_RELOC_TYPE:
      rel_entry.rel_type = rel_parse_type(text_ptr, text_nxt - text_ptr);
      state = PARSE_SYM;
      break;
    case PARSE_SYM:
      rel_entry.sym = text_ptr;
      rel_entry.symlen = text_nxt - text_ptr;
      assert(rel_entry.symlen > 0);
      state = PARSE_ADDEND;
      break;
    case PARSE_ADDEND:
      rel_entry.addend = rel_parse_addend(text_ptr, text_nxt - text_ptr);
      state = PARSE_DONE;
      break;
    default:
      assert(false && "invalid parsing state");
    }

    if (text_nxt != text_end && isspace(*text_nxt)) {
      ++text_nxt; // consume the space
    }
    text_ptr = text_nxt;
  }
  assert(state == PARSE_DONE);
  return rel_entry;
}

void rel_entry_dump(struct as_rel_s ent) {
  printf("as_rel_s entry:\n");
  printf("  offset %d\n", ent.offset);
  printf("  type %d\n", ent.rel_type);
  printf("  sym %.*s\n", ent.symlen, ent.sym);
  printf("  addend %d\n", ent.addend);
}

void reloc_apply(struct as_rel_s* pent, struct str* pbin_code) {
  assert(pent->offset >= 0 && pent->offset + 4 <= pbin_code->len);
  uint32_t *patch_loc = (uint32_t*) (pbin_code->buf + pent->offset);
  // don't care about *patch_loc since we have addend field in as_rel_s

  char *name = lenstrdup(pent->sym, pent->symlen);
  void *symaddr = sym_lookup(name);
  if (!symaddr) {
    /*
     * It's weird that RTLD_DEFAULT is not defined in my environment.
     * Since glibc define RTLD_DEFAULT as nullptr, I'll just pass in nullptr.
     *  void *symaddr = dlsym(RTLD_DEFAULT, name);
     */
    symaddr = dlsym(NULL, name);
  }
  free(name);
  if (!symaddr) {
    printf("Symbol not found %.*s\n", pent->symlen, pent->sym);
    assert(false);
  }

  switch (pent->rel_type) {
  case R_386_32:
    *patch_loc = (uint32_t) symaddr + pent->addend;
    break;
  case R_386_PC32:
    *patch_loc = symaddr - (void*) patch_loc + pent->addend;
    break;
  default:
    printf("Unsupported relocation type %d\n", pent->rel_type);
    assert(false);
  }
}
