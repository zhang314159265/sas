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
	str.buf = malloc(str.capacity);
	return str;
}

static inline void str_append(struct str* pstr, char ch) {
	assert(pstr->len <= pstr->capacity);
	if (pstr->len == pstr->capacity) {
		pstr->capacity <<= 1;
		pstr->buf = realloc(pstr->buf, pstr->capacity);
	}
	pstr->buf[pstr->len++] = ch;
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
	free(pstr->buf);
	memset(pstr, 0x1F, sizeof(*pstr));
}
