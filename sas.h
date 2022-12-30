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
#include "check.h"
#include "elf.h"

// do unescape inplace
char* unescape(char* s) {
  // TODO: only handle \n for now
  // TODO: don't handle the case that '\' is itself escaped
  char *pos = s, *cur = s;
  while (*cur) {
    if (*cur == '\\' && cur[1] == 'n') {
      *pos++ = '\n';
      cur += 2;
    } else {
      *pos++ = *cur++;
    }
  }
  *pos = '\0';
  return s;
}

void parse_directive(struct asctx* ctx, const char* directive, const char* cur, const char* end) {
  cur = skip_spaces(cur, end);

  // TODO avoid strcmp one by one
  if (strcmp(directive, ".globl") == 0) {
    const char* tokenend = gettoken(cur, end);
    assert(tokenend == end);
    const char* label = lenstrdup(cur, tokenend - cur);
    struct label_metadata* md = asctx_register_label(ctx, label);
    md->bind = STB_GLOBAL;
    free((void*) label);
  } else if (strcmp(directive, ".string") == 0) {
    assert(end - cur >= 2);
    assert(*cur == '"');
    assert(end[-1] == '"');

    // NOTE: looks like the assembler need to handle escape sequence!
    // otherwise printf will printf '\n' as 2 characters literally.
    // In theory we could also let printf to handle the escaping.
    // The most important thing is, exactly one of compiler/assembler/
    // library should do the escape.
    char *s = lenstrdup(cur + 1, end - cur - 2);
    unescape(s);
    // TODO: putting everything including the string to .text for now
    str_concat(&ctx->bin_code, s);
  } else {
    // ignore all others directives
    printf("WARNING: ignore directive '%s'\n", directive);
  }
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
 * (TODO: the comment need to be updated)
 */
void parse_text_code_line(struct asctx* ctx, const char* line, int linelen) {
	#if 0
	printf("Line: %.*s\n", linelen, line);
	#endif
	const char *cur = line;
	const char *end = line + linelen;
  cur = skip_spaces(cur, end);
	if (*cur == '#') {
		return; // comment line
	}

  // handle label
	const char *first_colon = NULL;
  // dont look at ':' beyond the next space so
  //   .ident  "GCC: (Ubuntu 11.3.0-1ubuntu1~22.04) 11.3.0"
  // is not mistakenly treated as a label.
	for (const char *p = cur; p != end && !isspace(*p); ++p) {
		if (*p == ':') {
			first_colon = p;
			break;
		}
	}
	if (first_colon) {
    // [cur, first_colon) defines the label
    char *label = lenstrdup(cur, first_colon - cur);
    asctx_define_label(ctx, label, ctx->bin_code.len);
    free(label);
		cur = first_colon + 1;
	}

  const char *tokenend;
	while (true) {
    cur = skip_spaces(cur, end);
		if (!(tokenend = gettoken(cur, end))) {
			break;
		}
		// printf("token is %.*s\n", tokenend - cur, cur);
		assert(tokenend - cur >= 2);
    int a, b;
		if (tokenend - cur == 2 && (a = hex2int(*cur)) >= 0 && (b = hex2int(cur[1])) >= 0) {
      // hex code
			char newch = (a << 4) | b;
			str_append(&ctx->bin_code, newch);
		} else if (*cur == '<') {
      // enclosed directive
			assert(*(tokenend - 1) == '>');
      int len = tokenend - cur;
      if (len >= 5 && memcmp(cur, "<REL ", 5) == 0) {
        struct as_rel_s rel_entry = rel_parse_str(ctx->bin_code.len, cur + 1, tokenend - 1);
        vec_append(&ctx->rel_list, &rel_entry);
        // rel_entry_dump(rel_entry);
        str_nappend(&ctx->bin_code, 4, 0);
      } else {
        printf("unhandled enclosed directive: %.*s\n", tokenend - cur, cur);
        assert(false);
      }
		} else {
      break;
    }
		cur = tokenend;
	}

  if (!tokenend) {
    return;
  }

  // [cur, tokenend) represents the opcode for an assembly instruction
  char* opstr = lenstrdup(cur, tokenend - cur);
  if (*opstr == '.') { // an assembler directive
    parse_directive(ctx, opstr, tokenend, end); 
    free(opstr);
    return;
  }
  assert(tokenend - cur > 0);
  int cc_opcode_off = -1;
  char sizesuf = '\0';

  // separate the potential size suffix
  if (is_valid_instr_stem(opstr, tokenend - cur - 1)) {
    sizesuf = opstr[tokenend - cur - 1];
    opstr[tokenend - cur - 1] = '\0';
  }

  // handle ops with cc
  int status;
  if (strncmp("cmov", opstr, 4) == 0 && (status = is_cc(opstr + 4)) >= 0) {
    free(opstr);
    opstr = strdup("cmovo"); // cmovo represents all cmovcc. cc_opcode_off specific which cc code to use
    cc_opcode_off = status;
  }
  if ((status = is_jcc(opstr)) >= 0) {
    free(opstr);
    opstr = strdup("jo");
    cc_opcode_off = status;
  }

  if (is_valid_instr_stem(opstr, strlen(opstr))) {
    struct operand lhs, rhs;
    lhs.repr = NULL;
    lhs.type = NUL;
    rhs.repr = NULL;
    rhs.type = NUL;

    cur = tokenend;
    int status;
    if (cur != end) {
      status = parse_operand(&cur, end, &lhs); 
      assert(status == 0);
    }
    if (cur != end) {
      status = parse_operand(&cur, end, &rhs);
      assert(status == 0);
    }
    assert(cur == end);
    handle_instr(ctx, opstr, &lhs, &rhs, sizesuf, cc_opcode_off);
    operand_free(&lhs);
    operand_free(&rhs);
  } else {
    printf("opstr is %s\n", opstr);
    assert(false && "Unsupported op");
  }
  free(opstr);
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
