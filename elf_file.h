#pragma once

#include "elf.h"
#include "util.h"
#include "vec.h"
#include "str.h"
#include <assert.h>
#include <stdio.h>

/* data structures for reading and writing elf files */
struct elf_file {
  Elf32_Ehdr ehdr;
  struct vec shtab;
  struct str shstrtab;

  struct str code;
  struct str strtab;
  struct vec symtab;

  uint16_t shn_text;
  uint16_t shn_strtab;
  uint16_t shn_symtab;
  uint16_t shn_shstrtab;
};

/*
 * Add a section header before all it's fields can be decided (e.g. size/offset) so
 * that the section number can be available early enough.
 *
 * The section header need to be revised before writing the file.
 *
 * Return the index of the section header just added.
 */
int ef_add_section(struct elf_file* ef, const char* name, uint32_t type, uint32_t flags, uint32_t link, uint32_t info, uint32_t addralign, uint32_t entsize) {
  Elf32_Shdr newsh;
  memset(&newsh, 0, sizeof(Elf32_Shdr));

  int name_idx = 0;
  if (name) {
    name_idx = str_concat(&ef->shstrtab, name);
  }
  newsh.sh_name = name_idx;
  newsh.sh_type = type;
  newsh.sh_flags = flags;
  newsh.sh_link = link;
  newsh.sh_info = info;
  newsh.sh_addralign = addralign;
  newsh.sh_entsize = entsize;

  vec_append(&ef->shtab, &newsh);
  return ef->shtab.len - 1;
}

struct elf_file ef_create() {
  struct elf_file ef;
  Elf32_Ehdr* ehdr = &ef.ehdr;
  memset(ehdr, 0, sizeof(*ehdr));
  ehdr->e_ident[0] = 0x7f;
  ehdr->e_ident[1] = 'E';
  ehdr->e_ident[2] = 'L';
  ehdr->e_ident[3] = 'F';
  ehdr->e_ident[4] = 1; // ELFCLASS32
  ehdr->e_ident[5] = 1; // little endian
  ehdr->e_ident[6] = 1; // file version == EV_CURRENT
  ehdr->e_ident[7] = 0; // OSABI
  ehdr->e_ident[8] = 0; // ABI version

  ehdr->e_type = ET_REL;
  ehdr->e_machine = EM_386;
  ehdr->e_version = 1;
  ehdr->e_entry = 0;
  ehdr->e_phoff = 0;
  ehdr->e_shoff = 0; // will be set to the correct value in ef_freeze
  ehdr->e_flags = 0;
  ehdr->e_ehsize = sizeof(Elf32_Ehdr);
  assert(sizeof(Elf32_Ehdr) == 52); // this should be a static assert

  ehdr->e_phentsize = 0; // no need to provide the real size since there are not program headers in relocatable file
  ehdr->e_phnum = 0;
  ehdr->e_shentsize = sizeof(Elf32_Shdr);
  assert(sizeof(Elf32_Shdr) == 40);
  ehdr->e_shnum = 0; // will be setup in ef_freeze
  ehdr->e_shstrndx = 0; // Setup later in this function

  ef.shtab = vec_create(sizeof(Elf32_Shdr));
  // add a null section
  ef_add_section(&ef, NULL, 0, 0, 0, 0, 0, 0);

  ef.shstrtab = str_create(0);
  // add a 0 at the beginning so index 0 represents an empty string
  // (this happen for the null section 0)
  str_append(&ef.shstrtab, 0);
  ef.code = str_create(0);
  ef.strtab = str_create(0);
  str_append(&ef.strtab, 0);
  ef.symtab = vec_create(sizeof(Elf32_Sym));
  Elf32_Sym null_sym;
  memset(&null_sym, 0, sizeof(Elf32_Sym));
  vec_append(&ef.symtab, &null_sym);

  ef.shn_text = ef_add_section(&ef, ".text", SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR, 0, 0, 1, 0);
  ef.shn_shstrtab = ef_add_section(&ef, ".shstrtab", SHT_STRTAB, 0, 0, 0, 1, 0);
  ehdr->e_shstrndx = ef.shn_shstrtab;
  ef.shn_strtab = ef_add_section(&ef, ".strtab", SHT_STRTAB, 0, 0, 0, 1, 0);
  // the sh_info field will be set in ef_freeze once we know all the Elf32_Sym entries
  ef.shn_symtab = ef_add_section(&ef, ".symtab", SHT_SYMTAB, 0, ef.shn_strtab, 0, 4, sizeof(Elf32_Sym));
  return ef;
}

/*
 * Value represents offset to the section for function symbols.
 */
void ef_add_symbol(struct elf_file* ef, const char *name, int shn, int value, int size, int bind, int type) {
  int name_idx = str_concat(&ef->strtab, name);

  Elf32_Sym sym;
  sym.st_name = name_idx;
  sym.st_value = value;
  sym.st_size = size;
  sym.st_info = (((bind) << 4) | type);
  sym.st_other = 0; // default visibility
  sym.st_shndx = shn;
  vec_append(&ef->symtab, &sym);
}

/*
 * Try to place the section at specified offset and return the new offset
 * after the placement. Alignment is handled properly.
 *
 * The sh_size field should be already setup before calling this function.
 */
static int ef_place_section(struct elf_file* ef, Elf32_Shdr* sh, int off) {
  int align = sh->sh_addralign;
  assert(align > 0);
  off = make_align(off, align);
  sh->sh_offset = off;
  assert(sh->sh_size > 0);
  return off + sh->sh_size;
}

/*
 * Finalize the content of section header and then elf header (e.g. shoff)
 * by 'virtually' write the elf file.
 */
static void ef_freeze(struct elf_file* ef) {
  // virtually place the Ehdr
  int next_off = sizeof(Elf32_Ehdr);

  // text section
  Elf32_Shdr* text_shdr = vec_get_item(&ef->shtab, ef->shn_text);
  text_shdr->sh_size = ef->code.len;
  next_off = ef_place_section(ef, text_shdr, next_off);

  // strtab
  Elf32_Shdr* strtab_shdr = vec_get_item(&ef->shtab, ef->shn_strtab);
  strtab_shdr->sh_size = ef->strtab.len;
  next_off = ef_place_section(ef, strtab_shdr, next_off);

  // symtab
  Elf32_Shdr* symtab_shdr = vec_get_item(&ef->shtab, ef->shn_symtab);
  symtab_shdr->sh_size = ef->symtab.len * ef->symtab.itemsize;
  next_off = ef_place_section(ef, symtab_shdr, next_off);
  // TODO: sort the symbols to make sure local symbols precede global/weak symbols
  {
    int i;
    for (i = 0; i < ef->symtab.len; ++i) {
      Elf32_Sym *sym = vec_get_item(&ef->symtab, i);
      if (ELF32_ST_BIND(sym->st_info) != STB_LOCAL) {
        break;
      }
    }
    symtab_shdr->sh_info = i;
  }

  // shstrtab
  Elf32_Shdr* shstrtab_shdr = vec_get_item(&ef->shtab, ef->shn_shstrtab);
  shstrtab_shdr->sh_size = ef->shstrtab.len;
  next_off = ef_place_section(ef, shstrtab_shdr, next_off);

  // now we know where the section header table should go
  next_off = make_align(next_off, 16); // force 16 bytes alignment for the section header table
  ef->ehdr.e_shoff = next_off;
  ef->ehdr.e_shnum = ef->shtab.len;
}

static void ef_write_section(FILE* fp, Elf32_Shdr* sh, void *data) {
  fseek(fp, sh->sh_offset, SEEK_SET);
  fwrite(data, 1, sh->sh_size, fp);
}

static void ef_write_shtab(struct elf_file* ef, FILE* fp) {
  fseek(fp, ef->ehdr.e_shoff, SEEK_SET);
  fwrite(ef->shtab.data, sizeof(Elf32_Shdr), ef->shtab.len, fp);
}

/*
 * Write the elf_file to path.
 *
 * Take 2 passes:
 * 1. decide each section's offset. (call freeze)
 * 2. write to file.
 *
 * An alternative solution is to do in one pass but do necessary back patching
 * in the end.
 */
static void ef_write(struct elf_file* ef, const char* path) {
  ef_freeze(ef);
  assert(ef->ehdr.e_ehsize == sizeof(Elf32_Ehdr));
  assert(ef->ehdr.e_shoff > 0);
  assert(ef->ehdr.e_shnum > 0);
  assert(ef->ehdr.e_shstrndx > 0);

  FILE* fp = fopen(path, "wb");
  fwrite(&(ef->ehdr), sizeof(Elf32_Ehdr), 1, fp);

  // write sections
  Elf32_Shdr* text_shdr = vec_get_item(&ef->shtab, ef->shn_text);
  ef_write_section(fp, text_shdr, ef->code.buf);

  // strtab
  Elf32_Shdr* strtab_shdr = vec_get_item(&ef->shtab, ef->shn_strtab);
  ef_write_section(fp, strtab_shdr, ef->strtab.buf);

  // symtab
  Elf32_Shdr* symtab_shdr = vec_get_item(&ef->shtab, ef->shn_symtab);
  ef_write_section(fp, symtab_shdr, ef->symtab.data);

  // shstrtab
  Elf32_Shdr* shstrtab_shdr = vec_get_item(&ef->shtab, ef->shn_shstrtab);
  ef_write_section(fp, shstrtab_shdr, ef->shstrtab.buf);

  ef_write_shtab(ef, fp);
  fclose(fp);
}

void ef_free(struct elf_file* ef) {
  vec_free(&ef->shtab);
  str_free(&ef->shstrtab);
  str_free(&ef->code);
  str_free(&ef->strtab);
  vec_free(&ef->symtab);
}
