#include <stdint.h>

typedef struct segment segment_t;

segment_t  *segment_new(uint32_t addr, int size);
void        segment_free(void);
uint32_t    segment_addr(segment_t  *seg);
uint32_t    segment_size(segment_t  *seg);
uint8_t    *segment_buf(segment_t   *seg);
segment_t  *segment_next(segment_t  *seg);
segment_t  *segment_start(void);

