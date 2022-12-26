#pragma

#include "inst.h"

static const char* skip_spaces(const char* cur, const char* end) {
  while (cur != end && isspace(*cur)) {
    ++cur;
  }
  return cur;
}

/*
 * Return 0 on success and a negative value for failure.
 *
 * On success, set popd->repr the whole string representing the operand.
 * Rely on operand_init to futher parse the operand string to understand if
 * it's a register/immediate number/memory operand and what's its size.
 */
static int parse_operand(const char** pcur, const char* end, struct operand* popd) {
  const char* cur = *pcur;
  cur = skip_spaces(cur, end);
  if (cur == end) {
    return -1;
  }

  // finish parsing when reaching end or comma and balance is 0
  int balance = 0;
  const char* start = cur;
  const char* repr = NULL;
  while (true) {
    if (cur == end) {
      if (balance > 0) {
        return -1;
      } else {
        repr = lenstrdup(start, end - start);
        break;
      }
    }
    if (*cur == ',' && balance == 0) {
      repr = lenstrdup(start, cur - start);
      ++cur;
      break;
    }
    if (*cur == '(') {
      ++balance;
    } else if (*cur == ')') {
      assert(balance > 0);
      --balance;
    }
    ++cur;
  }

  assert(repr);
  popd->repr = repr;
  operand_init(popd);
  *pcur = cur;
  return 0;
}
