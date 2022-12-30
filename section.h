#pragma once

/*
 * Represents a section in asctx.
 */
struct section {
  const char* name;
  int align; // we need use this to patch the secion header in ef_freeze
  struct str cont; // content
  int shn; // store the shn here to make it easy to find it for a label_metadata
};

struct section section_create(const char* name) {
  struct section sec;
  sec.name = strdup(name);
  sec.align = 1;
  sec.cont = str_create(0);
  sec.shn = -1;
  return sec;
}

static void section_align(struct section* sec, int align) {
  if (align <= 0) {
    return;
  }
  // use lowest common denominator
  sec->align = lcd(sec->align, align);
}

void section_free(struct section* sec) {
  free((void*) sec->name);
  str_free(&sec->cont);
}
