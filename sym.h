/* 
 * Maintain global registry to symbols. When doing relocation, symbols are
 * resolved by this registry before calling dlsym.
 */

#pragma once
#include <dlfcn.h>
#include "vec.h"

static void *sym_lookup(const char* name);

struct sym_entry {
  const char* name;  // memory owned by this entry
  void *addr;
};

// TODO: use a hash map rather than doing linear scan
struct vec global_entries;

void sym_module_init() __attribute__((constructor));
void sym_module_init() {
  global_entries = vec_create(sizeof(struct sym_entry));
}

static void sym_register(const char* name, void* addr) {
  assert(sym_lookup(name) == NULL);
  assert(addr);
  struct sym_entry entry;
  entry.name = strdup(name);
  entry.addr = addr;
  vec_append(&global_entries, &entry);
}

// return NULL if symbol not found
static void *sym_lookup(const char* name) {
  for (int i = 0; i < global_entries.len; ++i) {
    struct sym_entry* pent = (struct sym_entry*) vec_get_item(&global_entries, i);
    if (strcmp(pent->name, name) == 0) {
      return pent->addr;
    }
  }
  return NULL;
}
