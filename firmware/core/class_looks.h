#ifndef PD_CLASS_LOOKS_H
#define PD_CLASS_LOOKS_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define PD_LOOK_SIDE 64
#define PD_LOOK_BYTES (64u*64u*3u)
#define PD_LOOK_LARGE_BYTES (128u*128u*3u)
/* Explicit refresh only. Read-only catalog, one verified native frame cached.
 * NULL disables SD assets. Failure never changes the caller's output. */
void pd_looks_set_file(const char *path);
bool pd_look_image(unsigned lineage,unsigned body,unsigned outfit,unsigned scale,uint8_t *out,size_t capacity);
const char *pd_looks_status(void);
unsigned pd_looks_reads(void);
#endif
