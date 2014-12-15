/* my_utils.c */

#include <stdio.h>

#include "my_utils.h"


void Alignment_Init ( Alignment_Cache* self )
{
    self -> size = 0;
}

rc_t Alignment_Add ( Alignment_Cache* self, int64_t row_id, int64_t start, uint64_t len, uint32_t seq_start )
{
    if ( self -> size == countof ( self->data ) )
        return (rc_t)(-1);

    self->data [ self->size ] . row_id = row_id;
    self->data [ self->size ] . start = start;
    self->data [ self->size ] . len = len;
    self->data [ self->size ] . seq_start = seq_start;

    /* TODO: remove this */
    if ( self->data[ self->size - 1].row_id >=  self->data[ self->size].row_id)
    {
        printf ("WARNING: adding Alignment_CacheItem out of order: prev=%lld, cur=%lld\n",
            self->data[ self->size - 1].row_id, self->data[ self->size].row_id);
        return (rc_t)(-2);
    }

    ++ self->size;

    return 0;
}

Alignment_CacheItem const* Alignment_Get ( Alignment_Cache const* self, size_t i )
{
    return & self->data [ i ];
}
