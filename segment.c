/* -------------------- target memory segments ------------------------ */

#include <stdlib.h>
#include <stdint.h>
#include <err.h>

typedef struct segment segment;

struct segment {
  uint32_t   addr;
  uint32_t   size;
  uint8_t    *buf;
  segment    *next;
};

static segment *segments = NULL;

segment *segment_new(uint32_t addr, uint32_t size)
{
  segment    *seg;

  if ((seg = malloc(sizeof(segment))) == NULL)
    errx(1, "not enough memory");
  seg->addr = addr;
  seg->size = size;
  if ((seg->buf = malloc(size)) == NULL)
    errx(1, "not enough memory");
  seg->next = segments;
  segments = seg;
  return seg;
}

void segment_free()
{
  segment  *seg;

  while (segments != NULL) {
    free(segments->buf);
    seg = segments->next;
    free(segments);
    segments = seg;
  }
}

uint32_t segment_addr(segment *seg){return seg->addr;}
uint32_t segment_size(segment *seg){return seg->size;}
uint8_t *segment_buf(segment  *seg){return seg->buf;}
segment *segment_next(segment *seg){return seg->next;}
segment *segment_start()           {return segments;}

/* EOF */
