/* my_utils.h */

#include <stdint.h>

#include <klib/rc.h>

#ifndef countof
#define countof(arr) (sizeof(arr)/sizeof(arr[0]))
#endif

typedef struct Alignment_CacheItem
{
    int64_t row_id;
    int64_t start;
    uint64_t len;
    uint32_t seq_start; /* zero-based! Each alignment's REF_START (start) is relative to corresponding REFERENCE.SEQ_START, so we have to store this relation here */
} Alignment_CacheItem;

typedef struct Alignment_Cache
{
    size_t size;
    Alignment_CacheItem data[1024*1024];
} Alignment_Cache;

void Alignment_Init ( Alignment_Cache* self );
rc_t Alignment_Add ( Alignment_Cache* self, int64_t row_id, int64_t start, uint64_t len, uint32_t seq_start );
Alignment_CacheItem const* Alignment_Get ( Alignment_Cache const* self, size_t i );