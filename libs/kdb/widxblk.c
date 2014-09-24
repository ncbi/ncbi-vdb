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
#include "widxblk-priv.h"
#include "werror-priv.h"
#include <sysalloc.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <byteswap.h>


/*--------------------------------------------------------------------------
 * KColBlockMap
 *  maps an index or page section of block index
 */

/* KColBlockMapSetEntry
 *  sets an entry
 */
#ifndef KColBlockMapSetEntry
LIB_EXPORT void CC KColBlockMapSetEntry ( KColBlockMap *self,
    uint32_t slot, uint64_t first, uint32_t span )
{
    ( * self -> trans ) ( self, slot, first, span );
}
#endif

static
void KColBlockMapTrans0 ( KColBlockMap *self, uint32_t idx,
    uint64_t first, uint32_t span )
{
    self -> d [ idx ] = first;
    self -> s [ idx ] = span;
}

static
void KColBlockMapTrans1 ( KColBlockMap *self, uint32_t idx,
    uint64_t first, uint32_t span )
{
    self -> d [ idx ] = first;
    assert ( idx == 0 || self -> h . span [ 0 ] == span );
    if ( idx == 0 )
        self -> h . span [ 0 ] = span;
}

static
void KColBlockMapTrans2 ( KColBlockMap *self, uint32_t idx,
    uint64_t first, uint32_t span )
{
    self -> s [ idx ] = span;
    if ( idx == 0 )
        self -> h . first [ 0 ] = first;
}

static
void KColBlockMapTrans3 ( KColBlockMap *self, uint32_t idx,
    uint64_t first, uint32_t span )
{
    assert ( idx == 0 || self -> h . pred -> sz == span );
    if ( idx == 0 )
    {
        self -> h . pred -> pg = first;
        self -> h . pred -> sz = span;
    }
}

static
void KColBlockMapTransNull ( KColBlockMap *self, uint32_t idx,
    uint64_t first, uint32_t span )
{
}

static
void KColBlockMapSwap ( KColBlockMap *self, uint32_t count, int type )
{
    uint32_t i;

    switch ( type )
    {
    case btypeUniform:
        self -> h . span [ 0 ] = bswap_32 ( self -> h . span [ 0 ] );
        break;
    case btypeMagnitude:
        self -> h . first [ 0 ] = bswap_64 ( self -> h . first [ 0 ] );
        break;
    case btypePredictable:
        self -> h . pred -> pg = bswap_64 ( self -> h . pred -> pg );
        self -> h . pred -> sz = bswap_32 ( self -> h . pred -> sz );
        break;
    }

    switch ( type )
    {
    case btypeRandom:
    case btypeUniform:
        for ( i = 0; i < count; ++ i )
            self -> d [ i ] = bswap_64 ( self -> d [ i ] );
        break;
    }

    switch ( type )
    {
    case btypeRandom:
    case btypeMagnitude:
        for ( i = 0; i < count; ++ i )
            self -> s [ i ] = bswap_32 ( self -> s [ i ] );
        break;
    }
}


/*--------------------------------------------------------------------------
 * KColBlockLocInfo
 *  extensions for defining prospective idx2 blocks
 */

/* MergeIDTest
 *  tests viability of merging two blocks with same id type
 *  returns the required type for merge
 */
static
uint32_t KColBlockLocInfoMergeIDTest ( uint32_t id_type,
    const KColBlockLocInfo *left, const KColBlockLocInfo *right )
{
    switch ( id_type )
    {
    case btypeRandom:
        break;

    case btypeUniform:
        assert ( left -> count != 0 );
        assert ( right -> count != 0 );
        if ( ( left -> end_id - left -> start_id ) / left -> count ==
             ( right -> end_id - right -> start_id ) / right -> count )
            return btypeUniform;
        break;

    case btypeMagnitude:
        if ( left -> end_id == right -> start_id )
            return btypeMagnitude;
        break;

    case btypePredictable:
        assert ( left -> count != 0 );
        assert ( right -> count != 0 );

        if ( ( left -> end_id - left -> start_id ) / left -> count ==
             ( right -> end_id - right -> start_id ) / right -> count )
        {
            if ( left -> end_id == right -> start_id )
                return btypePredictable;
            return btypeUniform;
        }

        if ( left -> end_id == right -> start_id )
            return btypeMagnitude;
        break;
    }

    return btypeRandom;
}

/* MergePGTest
 *  tests viability of merging two blocks with same id type
 */
static
uint32_t KColBlockLocInfoMergePGTest ( uint32_t pg_type,
    const KColBlockLocInfo *left, const KColBlockLocInfo *right )
{
    switch ( pg_type )
    {
    case btypeRandom:
        break;

    case btypeUniform:
        if ( left -> size == right -> size )
            return btypeUniform;
        break;

    case btypeMagnitude:
        if ( left -> end_pg == right -> start_pg )
            return btypeMagnitude;
        break;

    case btypePredictable:
        if ( left -> size == right -> size )
        {
            if ( left -> end_pg == right -> start_pg )
                return btypePredictable;
            return btypeUniform;
        }
        if ( left -> end_pg == right -> start_pg )
            return btypeMagnitude;
        break;
    }

    return btypeRandom;
}

/* ConvertIDCost
 *  measures cost of converting an id type
 */
static
int64_t KColBlockLocInfoConvertIDCost ( const KColBlockLocInfo *bloc, uint32_t id_type )
{
    size_t cost;

    if ( bloc -> id_type == ( uint16_t ) id_type )
        return 0;

    switch ( ( id_type << 2 ) | bloc -> id_type )
    {
        /* conversions to most expensive type */
    case ( ( btypeRandom << 2 ) | btypeUniform ):
        cost = ( ( size_t ) bloc -> count << 2 ) - 4;
        break;
    case ( ( btypeRandom << 2 ) | btypeMagnitude ):
        cost = ( ( size_t ) bloc -> count << 3 ) - 8;
        break;
    case ( ( btypeRandom << 2 ) | btypePredictable ):
        cost = ( size_t ) bloc -> count * 12;
        break;

        /* other supported conversions */
    case ( ( btypeUniform << 2 ) | btypePredictable ):
        cost = ( size_t ) bloc -> count << 3;
        break;
    case ( ( btypeMagnitude << 2 ) | btypePredictable ):
        cost = ( size_t ) bloc -> count << 2;
        break;

        /* no other conversions are supported */
    default:
        return 0xFFFFFFFF;
    }

    /* consider the conversion done */
    return cost;
}

/* ConvertPGCost
 *  measures cost of converting a pg type
 */
static
int64_t KColBlockLocInfoConvertPGCost ( const KColBlockLocInfo *bloc, uint32_t pg_type )
{
    int64_t cost;

    if ( bloc -> pg_type == ( uint16_t ) pg_type )
        return 0;

    switch ( ( pg_type << 2 ) | bloc -> pg_type )
    {
        /* conversion to most expensive type */
    case ( ( btypeRandom << 2 ) | btypeUniform ):
        cost = ( ( uint32_t ) bloc -> count << 2 ) - 4;
        break;
    case ( ( btypeRandom << 2 ) | btypeMagnitude ):
        cost = ( ( uint32_t ) bloc -> count << 3 ) - 8;
        break;
    case ( ( btypeRandom << 2 ) | btypePredictable ):
        cost = ( uint32_t ) ( bloc -> count - 1 ) * 12;
        break;

        /* other supported conversions */
    case ( ( btypeUniform << 2 ) | btypePredictable ):
        cost = ( ( int64_t ) bloc -> count << 3 ) - 12;
        break;
    case ( ( btypeMagnitude << 2 ) | btypePredictable ):
        cost = ( ( int64_t ) bloc -> count << 2 ) - 12;
        break;

        /* no other conversions are supported */
    default:
        return 0xFFFFFFFF;
    }

    /* consider the conversion done */
    return cost;
}

/* Merge
 *  calculates the cost of merging two blocks
 *  returns the cost in bytes where a benefit is negative
 *  merges right into left
 */
int64_t KColBlockLocInfoMerge ( KColBlockLocInfo *a, const KColBlockLocInfo *b )
{
    int64_t cost;
    uint32_t id_type, pg_type;

    /* calculate optimistic lowest-common types */
    id_type = ( uint32_t ) a -> id_type & ( uint32_t ) b -> id_type;
    pg_type = ( uint32_t ) a -> pg_type & ( uint32_t ) b -> pg_type;

    /* test merge of b into a */
    id_type = KColBlockLocInfoMergeIDTest ( id_type, a, b );
    pg_type = KColBlockLocInfoMergePGTest ( pg_type, a, b );

    /* calculate cost of type conversion */
    cost = KColBlockLocInfoConvertIDCost ( a, id_type );
    cost += KColBlockLocInfoConvertPGCost ( a, pg_type );
    cost += KColBlockLocInfoConvertIDCost ( b, id_type );
    cost += KColBlockLocInfoConvertPGCost ( b, pg_type );

    /* perform the merge */
    a -> end_id = b -> end_id;
    a -> end_pg = b -> end_pg;
    a -> count += b -> count;
    a -> id_type = id_type;
    a -> pg_type = pg_type;

    /* the resultant block cannot be too large */
    if ( ( ( a -> end_id - a -> start_id ) >> 32 ) == 0 )
    {
        size_t hdr_size_div4 = pg_type;
        size_t entry_size_div4 = ( id_type ^ 3 ) + ( pg_type ^ 3 );
        if ( id_type != 3 )
            hdr_size_div4 += id_type;

        if ( ( hdr_size_div4 + entry_size_div4 * a -> count ) <= ( 0x10000 / 4 ) )
            return cost - sizeof ( KColBlockLoc );
    }

    /* too big */
    return 0xFFFFFFFF;
}


/*--------------------------------------------------------------------------
 * KColWIdxBlock
 *  level 2 index building block
 *
 *  each index block has two sections
 */

/* Init
 */
rc_t KColWIdxBlockInit ( KColWIdxBlock *iblk, const KColBlockLocInfo *info )
{
    /* start out with entry count */
    int block_size = ( int ) info -> count;

    /* determine block size and write funcs */
    int id_hsz, id_dsz, id_ssz;
    int pg_hsz, pg_dsz, pg_ssz;
    switch ( info -> id_type )
    {
    case btypeRandom:
        iblk -> id . trans = KColBlockMapTrans0;
        id_hsz = 0;
        id_dsz = 8;
        id_ssz = 4;
        break;
    case btypeUniform:
        iblk -> id . trans = KColBlockMapTrans1;
        id_hsz = 4;
        id_dsz = 8;
        id_ssz = 0;
        break;
    case btypeMagnitude:
        iblk -> id . trans = KColBlockMapTrans2;
        id_hsz = 8;
        id_dsz = 0;
        id_ssz = 4;
        break;
    case btypePredictable:
    default: /* to quiet compiler warnings */
        iblk -> id . trans = KColBlockMapTransNull;
        id_hsz = 0;
        id_dsz = 0;
        id_ssz = 0;
        break;
    }

    switch ( info -> pg_type )
    {
    case btypeRandom:
        iblk -> pg . trans = KColBlockMapTrans0;
        pg_hsz = 0;
        pg_dsz = 8;
        pg_ssz = 4;
        break;
    case btypeUniform:
        iblk -> pg . trans = KColBlockMapTrans1;
        pg_hsz = 4;
        pg_dsz = 8;
        pg_ssz = 0;
        break;
    case btypeMagnitude:
        iblk -> pg . trans = KColBlockMapTrans2;
        pg_hsz = 8;
        pg_dsz = 0;
        pg_ssz = 4;
        break;
    case btypePredictable:
    default: /* to quiet compiler warnings */
        iblk -> pg . trans = KColBlockMapTrans3;
        pg_hsz = 12;
        pg_dsz = 0;
        pg_ssz = 0;
        break;
    }

    /* record count */
    iblk -> count = block_size;

    /* account for the number of entries */
    id_dsz *= block_size;
    id_ssz *= block_size;
    pg_dsz *= block_size;
    pg_ssz *= block_size;

    /* allocate a block with compression buffer */
    block_size = id_hsz + id_dsz + id_ssz +
        pg_hsz + pg_dsz + pg_ssz;
    iblk -> id . h . p = malloc ( block_size + block_size );
    if ( iblk -> id . h . p == NULL )
        return RC ( rcDB, rcIndex, rcConstructing, rcMemory, rcExhausted );

    /* record uncompressed block size */
    iblk -> size = block_size;

    /* map the block */
    iblk -> pg . h . c = iblk -> id . h . c + id_hsz;
    iblk -> id . d = ( uint64_t* ) ( iblk -> pg . h . c + pg_hsz );
    iblk -> pg . d = ( uint64_t* ) ( ( char* ) iblk -> id . d + id_dsz );
    iblk -> id . s = ( uint32_t * ) ( ( char* ) iblk -> pg . d + pg_dsz );
    iblk -> pg . s = ( uint32_t * ) ( ( char* ) iblk -> id . s + id_ssz );

    /* prepare index */
    iblk -> idx = 0;

    return 0;
}

/* Whack
 */
void KColWIdxBlockWhack ( KColWIdxBlock *self )
{
    if ( self != NULL )
        free ( self -> id . h . p );
}

/* SetEntry
 *  sets index and page location information
 */
#ifndef KColWIdxBlockSetEntry
void KColWIdxBlockSetEntry ( KColWIdxBlock *self,
    int64_t start_id, uint32_t span,
    uint64_t pg, uint32_t sz )
{
    KColBlockMapSetEntry ( & self -> id, self -> idx, start_id, span );
    KColBlockMapSetEntry ( & self -> pg, self -> idx, pg, sz );
    ++ self -> idx;
}
#endif

/* Compress
 *  compress the block
 *  update the KColBlockLoc
 */
rc_t KColWIdxBlockCompress ( KColWIdxBlock *self, bool bswap,
    const KColBlockLocInfo *info, KColBlockLoc *bloc, size_t *to_write )
{
    assert ( info -> id_type < 4 );
    bloc -> u . blk . id_type = ( uint32_t ) info -> id_type;

    assert ( info -> pg_type < 4 );
    bloc -> u . blk . pg_type = ( uint32_t ) info -> pg_type;

    assert ( ( ( info -> end_id - info -> start_id ) >> 32 ) == 0 );
    bloc -> id_range = ( uint32_t ) ( info -> end_id - info -> start_id );

    bloc -> start_id = info -> start_id;

    bloc -> pg = 0;

    if ( info -> id_type == btypePredictable &&
         info -> pg_type == btypePredictable )
    {
        assert ( ( self -> count >> 27 ) == 0 );
        bloc -> u . blk . size = self -> count;
        bloc -> u . blk . compressed = 0;
        * to_write = 12;

        if ( bswap )
        {
            KColBlockMapSwap ( & self -> id, self -> count, info -> id_type );
            KColBlockMapSwap ( & self -> pg, self -> count, info -> pg_type );
        }

        return 0;
    }

    assert ( ( self -> size >> 27 ) == 0 );
    bloc -> u . blk . size = self -> size;
    * to_write = self -> size;

    if ( bswap )
    {
        KColBlockMapSwap ( & self -> id, self -> count, info -> id_type );
        KColBlockMapSwap ( & self -> pg, self -> count, info -> pg_type );
    }

    /* not compressing today */
    bloc -> u . blk . compressed = 0;
    return 0;
}

/* PersistPtr
 *  access the pointer for persisting to disk
 */
const void *KColWIdxBlockPersistPtr ( const KColWIdxBlock *self,
    const KColBlockLoc *bloc )
{
    assert ( bloc -> u . blk . compressed == 0 );
    return self -> id . h . p;
}
