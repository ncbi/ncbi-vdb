/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/

#include <kdb/extern.h>
#include "idxblk-priv.h"
#include <klib/rc.h>
#include <sysalloc.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <byteswap.h>


/*--------------------------------------------------------------------------
 * KColConstBlockMap
 *  maps an index or page section of block index
 */
static
void KColConstBlockMapSwap ( KColConstBlockMap *self, uint32_t count, int type )
{
    uint32_t i;

    switch ( type )
    {
    case btypeUniform:
        ( ( uint32_t* ) self -> h . span ) [ 0 ] = bswap_32 ( self -> h . span [ 0 ] );
        break;
    case btypeMagnitude:
        ( ( uint64_t* ) self -> h . first ) [ 0 ] = bswap_64 ( self -> h . first [ 0 ] );
        break;
    case btypePredictable:
        * ( uint64_t* ) & ( self -> h . pred -> pg ) = bswap_64 ( self -> h . pred -> pg );
        * ( uint32_t* ) & ( self -> h . pred -> sz ) = bswap_32 ( self -> h . pred -> sz );
        break;
    }

    switch ( type )
    {
    case btypeRandom:
    case btypeUniform:
        for ( i = 0; i < count; ++ i )
            ( ( uint64_t* ) self -> d ) [ i ] = bswap_64 ( self -> d [ i ] );
        break;
    }

    switch ( type )
    {
    case btypeRandom:
    case btypeMagnitude:
        for ( i = 0; i < count; ++ i )
            ( ( uint32_t* ) self -> s ) [ i ] = bswap_32 ( self -> s [ i ] );
        break;
    }
}


/*--------------------------------------------------------------------------
 * KColIdxBlock
 *  level 2 index reading block
 *
 *  each index block has two sections
 */

/* Find
 */
static
int KColIdxBlockFind0 ( const KColIdxBlock *self,
    const KColBlockLoc *bloc, uint32_t count,
    int64_t id, int64_t *first, uint32_t *span )
{
    unsigned int left = 0;
    unsigned int right = count;

    do
    {
        unsigned int i = ( left + right ) >> 1;
        int64_t start_id = ( int64_t ) self -> id . d [ i ];
        if ( start_id == id )
        {
            * first = start_id;
            * span = self -> id . s [ i ];
            return ( int ) i;
        }
        if ( start_id > id )
            right = i;
        else
        {
            if ( start_id + self -> id . s [ i ] > id )
            {
                * span = self -> id . s [ i ];
                * first = start_id;
                return ( int ) i;
            }

            left = i + 1;
        }
    }
    while ( left < right );

    return -1;
}

static
int KColIdxBlockFind1 ( const KColIdxBlock *self,
    const KColBlockLoc *bloc, uint32_t count,
    int64_t id, int64_t *first, uint32_t *span )
{
    unsigned int left = 0;
    unsigned int right = count;
    unsigned int range = self -> id . h . span [ 0 ];

    do
    {
        unsigned int i = ( left + right ) >> 1;
        int64_t start_id = ( int64_t ) self -> id . d [ i ];
        if ( start_id == id )
        {
            * first = start_id;
            * span = range;
            return ( int ) i;
        }
        if ( start_id > id )
            right = i;
        else
        {
            if ( start_id + range > id )
            {
                * first = start_id;
                * span = range;
                return ( int ) i;
            }

            left = i + 1;
        }
    }
    while ( left < right );

    return -1;
}

static
int KColIdxBlockFind2 ( const KColIdxBlock *self,
    const KColBlockLoc *bloc, uint32_t count,
    int64_t id, int64_t *first, uint32_t *span )
{
    /* special case for in-core image
       the length of an entry is determined
       by looking at the start of next entry
       with the last entry having its length
       stored in header */

    unsigned int left, right;

    /* examine last entry first, since it's the special case */
    int64_t start_id = ( int64_t ) self -> id . d [ count - 1 ];
    if ( start_id <= id )
    {
        if ( start_id == id ||
             start_id + self -> id . h . span [ 0 ] > id )
        {
            * span = self -> id . h . span [ 0 ];
            * first = start_id;
            return ( int ) ( count - 1 );
        }
        return -1;
    }

    /* now perform normal binary search */
    left = 0;
    right = count - 1;
    while ( left < right )
    {
        unsigned int i = ( left + right ) >> 1;
        start_id = ( int64_t ) self -> id . d [ i ];
        if ( start_id == id )
        {
            * first = start_id;
            * span = ( uint32_t ) ( self -> id . d [ i + 1 ] - ( uint64_t ) start_id );
            return ( int ) i;
        }
        if ( start_id > id )
            right = i;
        else
        {
            if ( self -> id . d [ i + 1 ] > (uint64_t)id )
            {
                * span = ( uint32_t ) ( self -> id . d [ i + 1 ] - ( uint64_t ) start_id );
                * first = start_id;
                return ( int ) i;
            }

            left = i + 1;
        }
    }

    return -1;
}

static
int KColIdxBlockFind3 ( const KColIdxBlock *self,
    const KColBlockLoc *bloc, uint32_t count,
    int64_t id, int64_t *first, uint32_t *span )
{
    int i;
    unsigned int ids_per;

    assert ( id >= bloc -> start_id );
    assert ( id < bloc -> start_id + bloc -> id_range );
    assert ( bloc -> id_range != 0 );

    if ( bloc -> id_range == ( uint32_t ) count )
    {
        * first = id;
        * span = 1;
        return ( int ) ( id - bloc -> start_id );
    }

    assert ( count != 0 );
    ids_per = bloc -> id_range / count;
    assert ( ids_per * count == bloc -> id_range );

    i = ( int ) ( ( id - bloc -> start_id ) / ids_per );
    * first = bloc -> start_id + ( i * ids_per );
    * span = ids_per;
    return i;
}

static
void KColIdxBlockGet0 ( const KColIdxBlock *self,
    const KColBlockLoc *bloc, uint32_t count,
    unsigned int idx, uint64_t *pg, uint32_t *sz )
{
    * pg = self -> pg . d [ idx ];
    * sz = self -> pg . s [ idx ];
}

static
void KColIdxBlockGet1 ( const KColIdxBlock *self,
    const KColBlockLoc *bloc, uint32_t count,
    unsigned int idx, uint64_t *pg, uint32_t *sz )
{
    * pg = self -> pg . d [ idx ];
    * sz = self -> pg . h . span [ 0 ];
}

static
void KColIdxBlockGet2 ( const KColIdxBlock *self,
    const KColBlockLoc *bloc, uint32_t count,
    unsigned int idx, uint64_t *pg, uint32_t *sz )
{
    if ( idx + 1 == count )
    {
        * pg = self -> pg . d [ idx ];
        * sz = self -> pg . h . span [ 0 ];
    }
    else
    {
        * pg = self -> pg . d [ idx ];
        * sz = ( uint32_t )
            ( self -> pg . d [ idx + 1 ] - self -> pg . d [ idx ] );
    }
}

static
void KColIdxBlockGet3 ( const KColIdxBlock *self,
    const KColBlockLoc *bloc, uint32_t count,
    unsigned int idx, uint64_t *pg, uint32_t *sz )
{
    * sz = self -> pg . h . pred [ 0 ] . sz;
    * pg = self -> pg . h . pred [ 0 ] . pg +
        ( (uint64_t) self -> pg . h . pred [ 0 ] . sz * idx );
}

/* RewriteColumns
 *  convert btypeMagnitude columns to id/pg
 *  for best in-core representation
 */
static
void KColIdxBlockRewriteColumns ( KColIdxBlock *self, size_t size,
    char *block, size_t block_size, int count,
    int id_ssz, int id_adj, int pg_ssz, int pg_adj )
{
    int i;
    uint64_t *d;
    uint32_t last;

    /* the blob size array is last */
    if ( pg_adj != 0 )
    {
        assert ( pg_ssz != 0 );

        d = ( uint64_t* ) ( block + size - pg_ssz - pg_adj );

        /* save the last entry in size array */
        last = self -> pg . s [ count - 1 ];

        /* transfer sizes, shifted down by 1 slot */
        for ( i = count; -- i > 0; )
            d [ i ] = self -> pg . s [ i - 1 ];

        /* set slot zero to be first pg */
        d [ 0 ] = self -> pg . h . first [ 0 ];

        /* integrate sizes */
        for ( i = 1; i < count; ++ i )
            d [ i ] += d [ i - 1 ];

        /* store last size in header */
        self -> pg . h . last [ 0 ] = last;

        /* update columns */
        self -> pg . d = d;
        self -> pg . s = ( const uint32_t* ) d;

        /* now convert to id/pg values */
    }
    else if ( pg_ssz != 0 )
    {
        /* pg size array trails column needing expansion */
        uint32_t *s = ( uint32_t* ) ( block + size - pg_ssz );
        for ( i = count; -- i >= 0; )
            s [ i ] = self -> pg . s [ i ];
        
        self -> pg . d = ( const uint64_t* ) s;
        self -> pg . s = s;
    }

    /* the index range array is either last
       or next to last before size array
       which, if there, was already moved */
    if ( id_adj != 0 )
    {
        assert ( id_ssz != 0 );

        d = ( uint64_t* ) ( block + size - pg_ssz - pg_adj - id_ssz - id_adj );

        /* save last as above */
        last = self -> id . s [ count - 1 ];

        /* transfer shifted */
        for ( i = count; -- i > 0; )
            d [ i ] = self -> id . s [ i - 1 ];

        /* set slot zero to first id */
        d [ 0 ] = self -> id . h . first [ 0 ];

        /* integrate spans */
        for ( i = 1; i < count; ++ i )
            d [ i ] += d [ i - 1 ];

        /* store last span in header */
        self -> id . h . last [ 0 ] = last;

        /* update columns */
        self -> id . d = d;
        self -> id . s = ( const uint32_t* ) d;
    }
}

/* Init
 */
rc_t KColIdxBlockInit ( KColIdxBlock *self,
    const KColBlockLoc *bloc, size_t orig,
    const void *block, size_t block_size, bool bswap )
{
    int id_hsz, id_dsz, id_ssz, id_adj = 0;
    int pg_hsz, pg_dsz, pg_ssz, pg_adj = 0;

    int count = ( int ) orig;

    switch ( bloc -> u . blk . id_type )
    {
    case btypeRandom:
        self -> find = KColIdxBlockFind0;
        id_hsz = 0;
        id_dsz = 8;
        id_ssz = 4;
        break;
    case btypeUniform:
        self -> find = KColIdxBlockFind1;
        id_hsz = 4;
        id_dsz = 8;
        id_ssz = 0;
        break;
    case btypeMagnitude:
        self -> find = KColIdxBlockFind2;
        id_hsz = 8;
        id_dsz = 0;
        id_ssz = 4;
        id_adj = 4;
        break;
    case btypePredictable:
    default: /* to quiet unused variable warnings */
        self -> find = KColIdxBlockFind3;
        id_hsz = 0;
        id_dsz = 0;
        id_ssz = 0;
        break;
    }

    switch ( bloc -> u . blk . pg_type )
    {
    case btypeRandom:
        self -> get = KColIdxBlockGet0;
        pg_hsz = 0;
        pg_dsz = 8;
        pg_ssz = 4;

        count -= id_hsz;
        count /= id_dsz + id_ssz + 12;
        break;

    case btypeUniform:
        self -> get = KColIdxBlockGet1;
        pg_hsz = 4;
        pg_dsz = 8;
        pg_ssz = 0;

        count -= id_hsz + 4;
        count /= id_dsz + id_ssz + 8;
        break;

    case btypeMagnitude:
        self -> get = KColIdxBlockGet2;
        pg_hsz = 8;
        pg_dsz = 0;
        pg_ssz = 4;
        pg_adj = 4;

        count -= id_hsz + 8;
        count /= id_dsz + id_ssz + 4;
        break;

    case btypePredictable:
    default: /* to quiet unused variable warnings */
        self -> get = KColIdxBlockGet3;
        pg_hsz = 12;

        if ( bloc -> u . blk . id_type == btypePredictable )
        {
            assert ( bloc -> u . blk . compressed == 0 );
            count = ( int ) bloc -> u . blk . size;
        }
        else
        {
            assert ( id_dsz + id_ssz != 0 );

            count -= id_hsz + pg_hsz;
            count /= id_dsz + id_ssz;
        }

        pg_dsz = 0;
        pg_ssz = 0;
        break;
    }

    id_dsz *= count;
    id_ssz *= count;
    id_adj *= count;
    pg_dsz *= count;
    pg_ssz *= count;
    pg_adj *= count;

    /* TBD - check the optimized output on this...
       some compilers aren't smart enough to recognize
       that union members are not independent objects */
    self -> id . h . p = block;
    self -> pg . h . c = ( const char* ) block + id_hsz;
/*  self -> pg . h . c = self -> id . h . c + id_hsz; */
    self -> id . d = ( const uint64_t* )
        ( self -> pg . h . c + pg_hsz );
    self -> pg . d = ( const uint64_t* )
        ( ( const char* ) self -> id . d + id_dsz );
    self -> id . s = ( const uint32_t * )
        ( ( const char* ) self -> pg . d + pg_dsz );
    self -> pg . s = ( const uint32_t * )
        ( ( const char* ) self -> id . s + id_ssz );

    /* sanity check against block size */
    if ( ( const char* ) block + orig != ( const char* ) self -> pg . s + pg_ssz )
        return RC ( rcDB, rcIndex, rcReading, rcData, rcCorrupt );

    /* byte-swap contents if needed */
    if ( bswap )
    {
        KColConstBlockMapSwap ( & self -> id, count, bloc -> u . blk . id_type );
        if ( self -> pg . h . p != self -> id . h . p )
            KColConstBlockMapSwap ( & self -> pg, count, bloc -> u . blk . pg_type );
    }

    /* check for need to rewrite columns */
    if ( id_adj + pg_adj != 0 )
    {
        size_t size = id_hsz + pg_hsz +
            id_dsz + id_ssz + id_adj +
            pg_dsz + pg_ssz + pg_adj;
        
        if ( size > block_size )
            return RC ( rcDB, rcIndex, rcReading, rcBuffer, rcInsufficient );

        assert ( id_adj == 0 || id_dsz == 0 );
        assert ( pg_adj == 0 || pg_dsz == 0 );

        KColIdxBlockRewriteColumns ( self, size,
            ( char* ) block, block_size, count,
            id_ssz, id_adj, pg_ssz, pg_adj );
    }

    return 0;
}

/* Find
 *  find zero-based index of entry into block
 *  that satisfies request
 *  returns -1 if not found
 */
#ifndef KColIdxBlockFind
LIB_EXPORT int CC KColIdxBlockFind ( const KColIdxBlock *self,
    const KColBlockLoc *bloc, uint32_t count,
    int64_t id, int64_t *first, uint32_t *span )
{
    ( * self -> find ) ( self, bloc, count, id, first, span );
}
#endif

/* Get
 *  get blob location information
 */
#ifndef KColIdxBlockGet
LIB_EXPORT void CC KColIdxBlockGet ( const KColIdxBlock *self,
    const KColBlockLoc *bloc, uint32_t count,
    unsigned int idx, uint64_t *pg, uint32_t *sz )
{
    ( * self -> get ) ( self, bloc, count, idx, pg, sz );
}
#endif


/*--------------------------------------------------------------------------
 * KColBlockLoc
 */

/* EntryCount
 *  returns the number of entries within a block
 *  uses the original size of the uncompressed block
 */
uint32_t KColBlockLocEntryCount ( const KColBlockLoc *self, size_t *orig )
{
    uint32_t count;

    /* when everything is predictable, the
       block size is 12 while the entry count
       is contained in self -> size */
    if ( self -> u . blk . id_type == btypePredictable &&
         self -> u . blk . pg_type == btypePredictable )
    {
        count = self -> u . blk . size;
        * orig = 12;
    }
    else
    {
        int id_hsz, id_dsz, id_ssz;
        int pg_hsz, pg_dsz, pg_ssz;

        count = ( uint32_t ) * orig;

        switch ( self -> u . blk . id_type )
        {
        case btypeRandom:
            id_hsz = 0;
            id_dsz = 8;
            id_ssz = 4;
            break;
        case btypeUniform:
            id_hsz = 4;
            id_dsz = 8;
            id_ssz = 0;
            break;
        case btypeMagnitude:
            id_hsz = 8;
            id_dsz = 0;
            id_ssz = 4;
            break;
        case btypePredictable:
        default: /* to quiet unused variable warnings */
            id_hsz = 0;
            id_dsz = 0;
            id_ssz = 0;
            break;
        }

        switch ( self -> u . blk . pg_type )
        {
        case btypeRandom:
            pg_hsz = 0;
            pg_dsz = 8;
            pg_ssz = 4;
            break;
        case btypeUniform:
            pg_hsz = 4;
            pg_dsz = 8;
            pg_ssz = 0;
            break;
        case btypeMagnitude:
            pg_hsz = 8;
            pg_dsz = 0;
            pg_ssz = 4;
            break;
        case btypePredictable:
        default: /* to quiet unused variable warnings */
            pg_hsz = 12;
            pg_dsz = 0;
            pg_ssz = 0;
            break;
        }

        assert ( id_dsz + id_ssz + pg_dsz + pg_ssz != 0 );

        /* decompose size into number of entries */
        count -= id_hsz + pg_hsz;
        count /= id_dsz + id_ssz + pg_dsz + pg_ssz;
    }

    return count;
}

/* AllocSize
 *  returns a size for the block
 *  since a type 3 index ( range/size only ) is nice for storage
 *  but needs to be converted to id/pos for in-memory lookup
 */
size_t KColBlockLocAllocSize ( const KColBlockLoc *self,
    size_t orig, uint32_t count )
{
    size_t size;

    /* when everything is predictable, the
       pg information is stored within BlocLoc */
    if ( self -> u . blk . id_type == btypePredictable &&
         self -> u . blk . pg_type == btypePredictable )
        size = 12;
    else
    {
        /* there is something within the idx2 block */
        size = 0;

        /* if either index or page loc are magnitude only,
           they'll need to be converted to id/pg when resident */
        if ( self -> u . blk . id_type == btypeMagnitude )
            size += 4;
        if ( self -> u . blk . pg_type == btypeMagnitude )
            size += 4;

        /* if neither are magnitude only,
           keep the original size */
        if ( size == 0 )
            size = orig;
        else
        {
            /* otherwise, convert the single entry
               size adjustment to whole block adjustment */
            size *= count;

            /* add this to the original size */
            size += orig;
        }
    }

    return size;
}
