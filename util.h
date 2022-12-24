#pragma once

#include <ctype.h>

#define true 1
#define false 0

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

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

char* lenstr_dup(const char* src, int len) {
  char* dst = (char*) malloc(len + 1);
  assert(dst);
  memcpy(dst, src, len);
  dst[len] = '\0';
  return dst;
}
