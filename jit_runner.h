#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include "str.h"
#include "vec.h"
#include "search.h"
#include "reloc.h"
#include "asctx.h"
#include "util.h"
#include "inst.h"
#include "tokenizer.h"
#include "jit.h"

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
void parse_text_code_line(struct asctx* ctx, const char* line, int linelen) {
	#if 0
	printf("Line: %.*s\n", linelen, line);
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
    int a, b;
		if (tokenend - curptr == 2 && (a = hex2int(*curptr)) >= 0 && (b = hex2int(curptr[1])) >= 0) {
			char newch = (a << 4) | b;
			str_append(&ctx->bin_code, newch);
		} else if (*curptr == '<') {
			assert(*(tokenend - 1) == '>');
      int len = tokenend - curptr;
      if (len >= 5 && memcmp(curptr, "<REL ", 5) == 0) {
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
      assert(tokenend - curptr > 0);
      int cc_opcode_off = -1;
      char sizesuf = '\0';
      if (is_valid_instr_stem(opstr, tokenend - curptr - 1)) {
        sizesuf = opstr[tokenend - curptr - 1];
        opstr[tokenend - curptr - 1] = '\0';
      }

      if (strncmp("cmov", opstr, 4) == 0 && (cc_opcode_off = is_cc(opstr + 4)) >= 0) {
        // TODO: avoid dupliate the code with the else case
        struct operand lhs, rhs;
        lhs.repr = NULL;
        lhs.type = NUL;
        rhs.repr = NULL;
        rhs.type = NUL;

        curptr = tokenend;
        int status;
        if (curptr != end) {
          status = parse_operand(&curptr, end, &lhs); 
          assert(status == 0);
        }
        if (curptr != end) {
          status = parse_operand(&curptr, end, &rhs);
          assert(status == 0);
        }
        assert(curptr == end);
        handle_cmovcc(ctx, cc_opcode_off, &lhs, &rhs, sizesuf);
        operand_free(&lhs);
        operand_free(&rhs);
      } else if (strcmp("jmp", opstr) == 0 || (cc_opcode_off = is_jcc(opstr)) >= 0) {
        curptr = skip_spaces(tokenend, end);
        tokenend = gettoken(curptr, end);
        char *label = lenstrdup(curptr, tokenend - curptr);
        tokenend = skip_spaces(tokenend, end);
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
      } else if (strcmp("nop", opstr) == 0) {
        assert(tokenend == end);
        str_append(&ctx->bin_code, 0x90);
      } else if (is_valid_instr_stem(opstr, strlen(opstr))) {
        struct operand lhs, rhs;
        lhs.repr = NULL;
        lhs.type = NUL;
        rhs.repr = NULL;
        rhs.type = NUL;

        curptr = tokenend;
        int status;
        if (curptr != end) {
          status = parse_operand(&curptr, end, &lhs); 
          assert(status == 0);
        }
        if (curptr != end) {
          status = parse_operand(&curptr, end, &rhs);
          assert(status == 0);
        }
        assert(curptr == end);
        handle_instr(ctx, opstr, &lhs, &rhs, sizesuf);
        operand_free(&lhs);
        operand_free(&rhs);
      } else {
  		  printf("token is %s\n", opstr);
        assert(false && "Unsupported token");
      }
      free(opstr);
      break;
    }
		curptr = tokenend;
	}
}

void _parse_text_code(struct asctx* ctx, const char* func_name, const char* text_code) {
	const char *cur = text_code;
	const char *next;
	while (*cur) {
		next = cur;
		while (*next && *next != '\n') {
			++next;
		}
    parse_text_code_line(ctx, cur, next - cur);
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
struct str parse_text_code(const char* func_name, const char *text_code) {
  struct asctx ctx = asctx_create();
  _parse_text_code(&ctx, func_name, text_code);

  struct str bin_code = str_move(&ctx.bin_code);
  asctx_free(&ctx);
	return bin_code;
}
