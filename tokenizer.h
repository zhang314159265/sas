#pragma once

/*
 * Only general parsing APIs goes to this file. 'parse_operand' which is used
 * to parse an operand for assembly instructions goes to 'operand.h'
 */

static const char* skip_spaces(const char* cur, const char* end) {
  while (cur != end && isspace(*cur)) {
    ++cur;
  }
  return cur;
}

/*
 * A very general function to get the next token.
 * A token can be either a longest string of characters without spaces.
 * or something enclosed by '<' and '>' (syntax for manual relocation entries)
 *
 * token can be preceed by a while space
 * return the end of the token if found or NULL
 */
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
