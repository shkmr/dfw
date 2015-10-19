/* ------------------------- elf using bfd ----------------------------*/

#include <stdlib.h>
#include <err.h>
#include <bfd.h>

#include "segment.h"

#define SEG_MAX 1024

static elf_read_section(bfd *prog, char *name)
{
  asection    *sec;
  segment_t   *seg;
  int         lma, size, ptr;

  sec = bfd_get_section_by_name(prog, name);
  if (sec == NULL) {
    warnx("section %s not found", name);
    return;
  }
  if (sec->flags & SEC_HAS_CONTENTS) {
    size = bfd_section_size(prog, sec);
    ptr = 0;
    while (size > SEG_MAX) {
      seg = segment_new(sec->lma + ptr, SEG_MAX);
      bfd_get_section_contents(prog, sec, segment_buf(seg), ptr, SEG_MAX);
      ptr  += SEG_MAX;
      size -= SEG_MAX;
    }
    seg = segment_new(sec->lma + ptr, size);
    bfd_get_section_contents(prog, sec, segment_buf(seg), ptr, size);
  }
}

void elf_read(char *fname)
{
  bfd       *prog;

  prog = bfd_openr(fname, NULL);
  if (prog == NULL) errx(1, "bfd_open %s", fname);

  if (!bfd_check_format_matches(prog, bfd_object, NULL))
    errx(1, "bfd_check_format_matches");

  elf_read_section(prog, ".text");
  elf_read_section(prog, ".data");
  elf_read_section(prog, ".vectors");

  bfd_close(prog);
}
