#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/mman.h>
#include <stdint.h>
#include "str.h"
#include "vec.h"
#include "search.h"
#include "reloc.h"
#include "asctx.h"
#include "util.h"
#include "inst.h"
#include "tokenizer.h"

void jit_make_exec(struct str* bin_code) {
	void *fnaddr = bin_code->buf;

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
}

void jit_run(struct str* bin_code) {
  jit_make_exec(bin_code);
	int retval = ((int(*)()) bin_code->buf)();
	printf("jit_run ret %d\n", retval);
}

// token can be preceed by at most one space.
// return the end of the token if found or NULL
const char *gettoken(const char *cur, const char *end) {
	if (cur == end || isspace(*cur)) {
		return NULL;
	}

  // handle '<' specially so: '<REL R_386_PC32 printf -4>' will be treated
  // as a single token
  int special_tok = (*cur == '<');
	while (cur != end && ((special_tok && *cur != '>') || (!special_tok && !isspace(*cur)))) {
		++cur;
	}
  if (cur != end && *cur == '>') {
    ++cur; // go past the trailing '>'
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
 *
 */
void parse_text_code_line(struct asctx* ctx, const char* line, int linelen, const char *argnames[], int argvals[]) {
	#if 0
	printf("%.*s\n", linelen, line);
	#endif
	const char *curptr = line;
	const char *end = line + linelen;
  curptr = skip_spaces(curptr, end);
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
    // [curptr, first_colon) defines the label
    char *label = lenstrdup(curptr, first_colon - curptr);
    asctx_define_label(ctx, label, ctx->bin_code.len);
    free(label);
		curptr = first_colon + 1;
	}
	while (curptr != end && isspace(*curptr)) {
		++curptr;
	}

  const char *tokenend;
	while (true) {
    curptr = skip_spaces(curptr, end);
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
			str_append(&ctx->bin_code, newch);
		} else if (*curptr == '<') {
			assert(*(tokenend - 1) == '>');
      int len = tokenend - curptr;
      if (!memchr(curptr, ' ', tokenend - curptr)) {
        // identify as a symbol if no space found
  			int idx = linear_search(argnames, curptr + 1, tokenend - curptr - 2);
  			assert(idx >= 0 && "symbol not found");
  			int val = argvals[idx];
  			// only support 32 bit values so far and assumes little endian
  			for (int i = 0; i < 4; ++i) {
  				str_append(&ctx->bin_code, val & 0xff);
  				val >>= 8;
  			}
      } else if (len >= 5 && memcmp(curptr, "<REL ", 5) == 0) {
        struct as_rel_s rel_entry = rel_parse_str(ctx->bin_code.len, curptr + 1, tokenend - 1);
        vec_append(&ctx->rel_list, &rel_entry);
        // rel_entry_dump(rel_entry);
        str_nappend(&ctx->bin_code, 4, 0);
      } else {
        printf("unhandled sym: %.*s\n", tokenend - curptr, curptr);
        assert(false);
      }
		} else {
      char* opstr = lenstrdup(curptr, tokenend - curptr);
      int cc_opcode_off = -1;
      if (strcmp("jmp", opstr) == 0 || (cc_opcode_off = is_jcc(opstr)) >= 0) {
        curptr = skip_spaces(tokenend, end);
        tokenend = gettoken(curptr, end);
        char *label = lenstrdup(curptr, tokenend - curptr);
        assert(tokenend == end);
        handle_jmp(ctx, label, cc_opcode_off);
        free(label);
      } else if (strcmp("call", opstr) == 0) {
        curptr = skip_spaces(tokenend, end); 
        tokenend = gettoken(curptr, end);
        assert(tokenend == end);
        char *func_name = lenstrdup(curptr, tokenend - curptr);
        handle_call(ctx, func_name);
        free(func_name);
      } else if (strcmp("mov", opstr) == 0) {
        curptr = tokenend;
        struct operand o1, o2;
        int status;
        status = parse_operand(&curptr, end, &o1);
        assert(status == 0);
        status = parse_operand(&curptr, end, &o2);
        assert(status == 0);
        assert(curptr == end);
        handle_mov(ctx, o1, o2);
        operand_free(&o1);
        operand_free(&o2);
        break;
      } else {
  		  printf("token is %s\n", opstr);
        assert(false && "Unsupported token");
      }
      free(opstr);
    }
		curptr = tokenend;
	}
}

void _parse_text_code(struct asctx* ctx, const char* func_name, const char* text_code, const char* argnames[], int argvals[]) {
	const char *cur = text_code;
	const char *next;
	while (*cur) {
		next = cur;
		while (*next && *next != '\n') {
			++next;
		}
    parse_text_code_line(ctx, cur, next - cur, argnames, argvals);
		if (*next == '\n') {
			++next;
		}
		cur = next;
	}

  const char *debug_func_name = (func_name ? func_name : "<FUNC>");
  printf("=== FUNC %s ===\n", debug_func_name);
	printf("  addr %p\n", ctx->bin_code.buf);
	printf("  len %d\n", ctx->bin_code.len);

  if (func_name) {
    sym_register(func_name, ctx->bin_code.buf);
  }

  // resolve the collected relocation entries.
  // Must be 2 pass because of potential realloc
  for (int i = 0; i < ctx->rel_list.len; ++i) {
    struct as_rel_s* pent = (struct as_rel_s*) vec_get_item(&ctx->rel_list, i);
    reloc_apply(pent, &ctx->bin_code);
  }

  asctx_resolve_label_patch(ctx);
}

/*
 * Each text code represents a function.
 */
struct str parse_text_code(const char* func_name, const char *text_code, const char* argnames[], int argvals[]) {
  struct asctx ctx = asctx_create();
  _parse_text_code(&ctx, func_name, text_code, argnames, argvals);

  struct str bin_code = str_move(&ctx.bin_code);
  asctx_free(&ctx);
	return bin_code;
}

struct str parse_text_code_simple(const char* func_name, const char *text_code) {
  const char* argnames[] = { NULL};
  int argvals[] = {};
  return parse_text_code(func_name, text_code, argnames, argvals);
}
