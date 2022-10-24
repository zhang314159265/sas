#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/mman.h>
#include <stdint.h>
#include "str.h"
#include "search.h"

void jit_run(struct str* bin_code) {
	void *fnaddr = bin_code->buf;
	printf("fnaddr %p\n", fnaddr);
	printf("len %d\n", bin_code->len);

	// mprotect requries the address to be aligned on page boundary
	uint32_t mem_addr = (uint32_t) fnaddr;
	int mem_len = bin_code->len;
	if (mem_addr & 0xFFF) {
		mem_len += (mem_addr & 0xFFF);
		mem_addr = (mem_addr & ~0xFFF);
	}
	int status = mprotect((void*) mem_addr, mem_len, PROT_READ | PROT_WRITE | PROT_EXEC);
	if (status != 0) {
		perror("mprotect fail");
		assert(false && "mprotect fail");
	}
	int retval = ((int(*)()) fnaddr)();
	printf("jit_run ret %d\n", retval);
}

// token can be preceed by at most one space.
// return the end of the token if found or NULL
const char *gettoken(const char *cur, const char *end) {
	if (cur == end || isspace(*cur)) {
		return NULL;
	}
	while (cur != end && !isspace(*cur)) {
		++cur;
	}
	return cur;
}

/*
 * The format of each line of the text code are defined as follows:
 * - there can be arbitrary many preceeding spaces
 * - if the first non space character is '#', ignore the line. This works like a comment
 * - there can be an optional colon in the line. Anything before the first colon
 *   and any space after it are ignored
 * - tokens are congituous non-space segment separated by a single space. Any pattern violating that terminate the token list.
 * - a token can either be two hexdecimal digits or a argument name enclosed by '<' and '>'
 *
 * The format is define this way so
 * - it's easy to use the output of objdump directly.
 * - we can patch the code with runtime symbols. This simulates relocation in some sense.
 */
void parse_text_code_line(const char* line, int linelen, struct str* bin_code, const char *argnames[], int argvals[]) {
	#if 0
	printf("%.*s\n", linelen, line);
	#endif
	const char *curptr = line;
	const char *end = line + linelen;
	while (curptr != end && isspace(*curptr)) {
		++curptr;
	}
	if (*curptr == '#') {
		return; // comment line
	}

	const char *first_colon = NULL;
	// skip until first colon
	for (const char *p = curptr; p != end; ++p) {
		if (*p == ':') {
			first_colon = p;
			break;
		}
	}
	if (first_colon) {
		curptr = first_colon + 1;
	}
	while (curptr != end && isspace(*curptr)) {
		++curptr;
	}

  const char *tokenend;
	// printf("=========split\n");
	while (true) {
		if (curptr != end && *curptr == ' ') {
			++curptr;
		}
		if (!(tokenend = gettoken(curptr, end))) {
			break;
		}
		// printf("token is %.*s\n", tokenend - curptr, curptr);
		assert(tokenend - curptr >= 2);
		if (tokenend - curptr == 2) {
			int a = hex2int(*curptr);
			int b = hex2int(*(curptr + 1));
			assert(a >= 0 && b >= 0);
			char newch = (a << 4) | b;
			str_append(bin_code, newch);
		} else {
			assert(*curptr == '<');
			assert(*(tokenend - 1) == '>');
			int idx = linear_search(argnames, curptr + 1, tokenend - curptr - 2);
			assert(idx >= 0);
			int val = argvals[idx];
			// only support 32 bit values so far and assumes little endian
			for (int i = 0; i < 4; ++i) {
				str_append(bin_code, val & 0xff);
				val >>= 8;
			}
		}
		curptr = tokenend;
	}
}

struct str parse_text_code(const char *text_code, const char* argnames[], int argvals[]) {
	const char *cur = text_code;
	const char *next;
	int capacity = 0;
	struct str bin_code = str_create(256);
	while (*cur) {
		next = cur;
		while (*next && *next != '\n') {
			++next;
		}
		parse_text_code_line(cur, next - cur, &bin_code, argnames, argvals);
		if (*next == '\n') {
			++next;
		}
		cur = next;
	}
	return bin_code;
}
