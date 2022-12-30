#pragma once

#include "reloc.h"
#include "vec.h"
#include "dict.h"
#include "str.h"
#include "check.h"
#include "label_metadata.h"
#include "section.h"

struct label_patch_entry {
  const char* label;
  int off; // assume current section to be the .text
};

struct asctx {
  struct vec rel_list; // list of relocation entries

  // TODO: have some datastructure the combine the following 2
  struct dict label2idx;
  struct vec labelmd_list;

  struct vec label_patch_list; // similar to relocation but use label definition rather than symbol to resolve

  // a table of struct section.
  // TODO: should we use a dict to speed up search? Or it's fine since the number
  // of sections is often small.
  struct vec sectab;
  // the current section. Point the .text by default
  struct section* cursec;
  struct section* textsec;
};

static struct str* asctx_expect_cursec_buf(struct asctx* ctx, const char *name) {
  struct section* cursec = ctx->cursec;
  CHECK(strcmp(cursec->name, name) == 0, "Expect current section to be '%s' but got %s", name, cursec->name);
  return &ctx->cursec->cont;
}

static struct str* asctx_cursec_buf(struct asctx* ctx) {
  return &ctx->cursec->cont;
}

static int asctx_cursec_buflen(struct asctx* ctx) {
  return asctx_cursec_buf(ctx)->len;
}

static void asctx_switch_section(struct asctx* ctx, const char* name) {
  bool found = 0;
  VEC_FOREACH(&ctx->sectab, struct section, sptr) {
    if (strcmp(name, sptr->name) == 0) {
      found = 1;
      ctx->cursec = sptr;
      break;
    }
  }
  // TODO: be able to create the section on the fly
  assert(found);
}

static struct asctx asctx_create() {
  struct asctx ctx;
  ctx.rel_list = vec_create(sizeof(struct as_rel_s));
  ctx.label2idx = dict_create();
  ctx.labelmd_list = vec_create(sizeof(struct label_metadata));
  ctx.label_patch_list = vec_create(sizeof(struct label_patch_entry));

  ctx.sectab = vec_create(sizeof(struct section));

  // by default we have a .text section
  struct section text_section = section_create(".text");
  vec_append(&ctx.sectab, &text_section);
  ctx.cursec = vec_get_item(&ctx.sectab, 0);
  ctx.textsec = ctx.cursec;

  // precreate the data section
  struct section data_section = section_create(".data");
  vec_append(&ctx.sectab, &data_section);
  return ctx;
}

/*
 * Return the pointer to the label metadata if label is found; return NULL
 * otherwise.
 */
static struct label_metadata* asctx_get_label_metadata(struct asctx* ctx, const char* label) {
  struct dict_entry *entry = dict_lookup(&ctx->label2idx, label);
  if (entry->key) {
    return vec_get_item(&ctx->labelmd_list, entry->val);
  } else {
    return NULL;
  }
}

/*
 * Register a default constructed label metadata if label is not registered yet;
 * be an nop otherwise.
 *
 * In either case, return the pointer for the label metadata.
 */
static struct label_metadata* asctx_register_label(struct asctx* ctx, const char* label) {
  assert(ctx->label2idx.size == ctx->labelmd_list.len);
  struct dict_entry *entry = dict_lookup(&ctx->label2idx, label);
  int idx = -1;
  if (entry->key) {
    idx = entry->val;
  } else {
    idx = ctx->label2idx.size;
    int rc = dict_put(&ctx->label2idx, label, idx);
    assert(rc == 1);
    // new label
    struct label_metadata md = labelmd_create();
    vec_append(&ctx->labelmd_list, &md);
  }

  assert(ctx->label2idx.size == ctx->labelmd_list.len);
  struct label_metadata* ret = vec_get_item(&ctx->labelmd_list, idx);
  assert(ret);
  return ret;
}

/*
 * It's possible that the label has already been created by .globl
 */
static void asctx_define_label(struct asctx* ctx, const char* label, int off) {
  struct label_metadata *md = asctx_register_label(ctx, label);
  md->off = off;
  md->section = ctx->cursec;
}

static void asctx_resolve_label_patch(struct asctx* ctx) {
  for (int i = 0; i < ctx->label_patch_list.len; ++i) {
    struct label_patch_entry* patch_entry = vec_get_item(&ctx->label_patch_list, i);
    uint32_t md_idx = dict_lookup_nomiss(&ctx->label2idx, patch_entry->label);
    struct label_metadata* md = vec_get_item(&ctx->labelmd_list, md_idx);
    uint32_t label_off = md->off;
    uint32_t patch_off = patch_entry->off;

    // here we assumbe to patch the text section
    *(uint32_t*)(ctx->textsec->cont.buf + patch_off) = label_off - (patch_off + 4);
  }
}

static void asctx_add_relocation(struct asctx* ctx, const char* symname, int offset, int reloc_type) {
  struct as_rel_s item;
  item.own_buf = 1;
  item.offset = offset;
  item.rel_type = reloc_type;
  item.sym = strdup(symname);
  item.symlen = strlen(symname);;
  if (reloc_type == R_386_32) {
    item.addend = 0;
  } else if (reloc_type == R_386_PC32) {
    item.addend = -4;
  }
  vec_append(&ctx->rel_list, &item);
}

static void asctx_dump_relocs(struct asctx* ctx) {
  printf("asctx contains %d relocation entries:\n", ctx->rel_list.len);
  VEC_FOREACH(&ctx->rel_list, struct as_rel_s, relptr) {
    rel_entry_dump(*relptr);
  }
}

static void asctx_dump_labels(struct asctx* ctx) {
  printf("asctx contains %d labels:\n", ctx->label2idx.size);

  DICT_FOREACH(&ctx->label2idx, entry) {
    assert(entry->key);
    struct label_metadata* md = vec_get_item(&ctx->labelmd_list, entry->val);
    labelmd_dump(md, entry->key);
  }
}

static void asctx_free(struct asctx* ctx) {
  for (int i = 0; i < ctx->rel_list.len; ++i) {
    struct as_rel_s *item = vec_get_item(&ctx->rel_list, i);
    if (item->own_buf) {
      free((void*) item->sym);
    }
  }
  vec_free(&ctx->rel_list);
  dict_free(&ctx->label2idx);
  vec_free(&ctx->labelmd_list);

  for (int i = 0; i < ctx->label_patch_list.len; ++i) {
    struct label_patch_entry* item = vec_get_item(&ctx->label_patch_list, i);
    free((void*) item->label);
  }
  vec_free(&ctx->label_patch_list);

  VEC_FOREACH(&ctx->sectab, struct section, sptr) {
    section_free(sptr);
  }
  vec_free(&ctx->sectab);
}
