#pragma once

#include <stdlib.h>
#include "util.h"

// variable length string
struct str {
  int capacity;
	int len;
	char *buf;
};

static inline struct str str_create(int init_capa) {
	init_capa = max(init_capa, 16); // provide at least 16 capacity
	struct str str;
	str.capacity = init_capa;
	str.len = 0;
	str.buf = (char*) malloc(str.capacity);
	return str;
}

static inline void str_append(struct str* pstr, char ch) {
	assert(pstr->len <= pstr->capacity);
	if (pstr->len == pstr->capacity) {
		pstr->capacity <<= 1;
		pstr->buf = (char*) realloc(pstr->buf, pstr->capacity);
	}
	pstr->buf[pstr->len++] = ch;
}

static inline void str_nappend(struct str* pstr, int n, char ch) {
  for (int i = 0; i < n; ++i) {
    str_append(pstr, ch);
  }
}

static inline void str_hexdump(struct str* pstr) {
	printf("str dump:\n");
	for (int i = 0; i < pstr->len; ++i) {
		printf(" %02x", (unsigned char) pstr->buf[i]);
		if ((i + 1) % 16 == 0) {
			printf("\n");
		}
	}
	printf("\n");
}

static inline void str_free(struct str* pstr) {
  if (pstr->buf) {
	  free(pstr->buf);
    pstr->buf = NULL;
  }
}

// Relocate the dword relative displacement at off to point to symbol.
static inline void str_relocate_off_to_sym(struct str* pstr, int off, int symbol) {
  uint32_t next_instr_addr = (uint32_t) (pstr->buf + off + 4);
  *(uint32_t*) (pstr->buf + off)
    = symbol - next_instr_addr;
}

static struct str str_move(struct str* pstr) {
  struct str ret = *pstr;
  pstr->capacity = 0;
  pstr->len = 0;
  pstr->buf = NULL;
  return ret;
}
