#pragma once

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define bool int
#define true 1
#define false 0

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

static int make_align(int val, int align) {
  assert(align > 0);
  return (val + align - 1) / align * align;
}

int is_int8(int32_t val) {
  return val >= -128 && val <= 127; 
}

int hex2int(char ch) {
	ch = tolower(ch);
	if (ch >= '0' && ch <= '9') {
		return ch - '0';
	} else if (ch >= 'a' && ch <= 'f') {
		return ch - 'a' + 10;
	} else {
		return -1;
	}
}

char* lenstrdup(const char* src, int len) {
  char* dst = (char*) malloc(len + 1);
  assert(dst != NULL);
  memcpy(dst, src, len);
  dst[len] = '\0';
  return dst;
}
