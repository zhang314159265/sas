#pragma once

#include "section.h"

/*
 * The assembly need to maintain metadatas for each label:
 * - offset
 * - bind: local, global, weak
 * - type: function, object
 * - size: number of bytes for a function
 * etc.
 */
struct label_metadata {
  int off;
  int bind;
  int type;
  int size;
  struct section* section;
};

static struct label_metadata labelmd_create() {
  struct label_metadata md;
  memset(&md, 0, sizeof(struct label_metadata));
  md.off = -1;
  return md;
}

static void labelmd_dump(struct label_metadata* md, const char* name) {
  printf(" - %s: off(%d), bind(%d), type(%d), size(%d)\n", name, md->off, md->bind, md->type, md->size);
}
