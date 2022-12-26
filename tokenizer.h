#pragma

static const char* skip_spaces(const char* cur, const char* end) {
  while (cur != end && isspace(*cur)) {
    ++cur;
  }
  return cur;
}
