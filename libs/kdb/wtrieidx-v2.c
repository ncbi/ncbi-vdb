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

#include "windex-priv.h"
#include "trieidx-priv.h"

#include <kdb/index.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/md5.h>
#include <kfs/mmap.h>
#include <klib/ptrie.h>
#include <klib/text.h>
#include <klib/pack.h>
#include <klib/rc.h>
#include <os-native.h>
#include <sysalloc.h>

#include <byteswap.h>

#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#define KTRIE_ZEROS_KPTRIE 1

/*--------------------------------------------------------------------------
 * KPTrieIndex_v2
 *  persisted keymap
 */


/* Init
 *  opens and initializes persisted keymap structure
 */
static
rc_t KPTrieIndexInitID2Ord ( KPTrieIndex_v2 *self, size_t in_size,
    int variant, int span_bits, int elem_bits )
{
    rc_t rc;
    union
    {
        uint8_t *v8;
        uint16_t *v16;
        uint32_t *v32;
        uint64_t *v64;
    } dst;
    size_t elem_bytes = elem_bits >> 3;
    uint32_t scount = self -> count - 1;

    assert ( self -> count != 0 );

    if ( span_bits * scount > in_size * 8 )
        return RC ( rcDB, rcIndex, rcConstructing, rcIndex, rcCorrupt );

    dst . v8 = malloc ( self -> count * elem_bytes );
    if ( dst . v8 == NULL )
        rc = RC ( rcDB, rcIndex, rcConstructing, rcMemory, rcExhausted );
    else
    {
        size_t usize;
        rc = Unpack ( span_bits, elem_bits,
            & self -> ord2node [ self -> count ], 0,
            span_bits * scount, NULL, & dst . v8 [ elem_bytes ],
            scount * elem_bytes, & usize );
        if ( rc == 0 )
        {
            uint32_t i;

            self -> id2ord . v8 = dst . v8;
            self -> variant = variant;

            /* integrate to simple translation */
            switch ( variant )
            {
            case 1:
                dst . v8 [ 0 ] = 0;
                for ( i = 0; i < scount; ++ i )
                    dst . v8 [ i + 1 ] += dst . v8 [ i ];
                break;
            case 2:
                dst . v16 [ 0 ] = 0;
                for ( i = 0; i < scount; ++ i )
                    dst . v16 [ i + 1 ] += dst . v16 [ i ];
                break;
            case 3:
                dst . v32 [ 0 ] = 0;
                for ( i = 0; i < scount; ++ i )
                    dst . v32 [ i + 1 ] += dst . v32 [ i ];
                break;
            case 4:
                dst . v64 [ 0 ] = 0;
                for ( i = 0; i < scount; ++ i )
                    dst . v64 [ i + 1 ] += dst . v64 [ i ];
                break;
            }

            return 0;
        }

        free ( dst . v8 );
    }

    return rc;
}

static
void CC KPTrieIndexExtractV1Range_v2 ( PTNode *n, void *data )
{
    KPTrieIndex_v2 *self = data;

    /* capture node id */
    uint32_t id;
    assert ( n -> data . size == sizeof id );
    memmove ( & id, n -> data . addr, sizeof id );

    /* perform min/max */
    if ( self -> count == 0 )
        self -> first = self -> last = id;
    else if ( id < self -> first )
        self -> first = id;
    else if ( id > self -> last )
        self -> last = id;

    ++ self -> count;
}

static
rc_t KPTrieIndexInitFromV1_v2 ( KPTrieIndex_v2 *self, const KMMap *mm, bool byteswap )
{
    KPTrieIndex_v1 v1;
    rc_t rc = KPTrieIndexInit_v1 ( & v1, mm, byteswap );
    if ( rc == 0 )
    {
        uint32_t *ord2node;
        uint32_t total_id, test_id;
        uint32_t i, id, id_bits, num_ids;

        /* hopefully we got a projection index */
        if ( v1 . id2node == NULL )
        {
#if ! KTRIE_ZEROS_KPTRIE
            self -> count = 0;
#endif
            /* need to derive first and last from trie */
            PTrieForEach ( v1 . key2id, KPTrieIndexExtractV1Range_v2, self );
            if ( self -> count == 0 )
                KPTrieIndexWhack_v1 ( & v1 );
            else
            {
                /* take trie as-is */
                self -> key2id = v1 . key2id;
                self -> maxid = self -> last;
            }

            /* note that this assumes a span of 1 for
               each id. there are no known uses of v1 without
               a projection index, so this is unlikely to be important */

            return 0;
        }

        /* take id range */
        self -> first = v1 . first;
        self -> last = self -> maxid = v1 . last;

        /* count comes from trie as always */
        self -> count = PTrieCount ( v1 . key2id );

        /* detect empty trie */
        if ( self -> count == 0 || self -> first > self -> last )
        {
            self -> first = self -> last = self -> maxid = 0;
            return 0;
        }

        /* take trie as-is */
        self -> key2id = v1 . key2id;

        /* now decide whether to use 1-1 or sparse projection */
        if ( ( self -> last - self -> first ) < ( ( int64_t ) self -> count << 1 ) )
        {
            /* take the old projection array as-is,
               treating NULL node ids as holes */
            self -> ord2node = v1 . id2node;
            return 0;
        }

        /* convert to sparse
           calculate id bits - notice that
           total_id gets right shifted so that
           the loop is guaranteed to exit */
        num_ids = ( uint32_t ) ( self -> last - self -> first + 1 );
        for ( total_id = num_ids >> 1, id_bits = 1, test_id = 1;
            test_id <= total_id;
            ++ id_bits, test_id <<= 1 )
            ( void ) 0;

        /* determine variant */
        if ( id_bits <= 8 )
        {
            /* allocate 4 bytes for new ord2node and 1 for id2ord */
            uint8_t *id2ord = malloc ( self -> count * 5 );
            if ( id2ord == NULL )
                rc = RC ( rcDB, rcIndex, rcConstructing, rcMemory, rcExhausted );
            else
            {
                ord2node = ( uint32_t* ) & id2ord [ self -> count ];
                self -> ord2node = ord2node;
                self -> id2ord . v8 = id2ord;
                self -> variant = 1;

                /* walk across v1 table, looking at each id */
                for ( i = id = 0; id < num_ids; ++ id )
                {
                    /* detect non NULL node ids
                       and pretend they represent a contiguous
                       span with no holes in id space */
                    if ( v1 . id2node [ id ] != 0 )
                    {
                        /* prevent overwriting */
                        if ( i == self -> count )
                        {
                            rc = RC ( rcDB, rcIndex, rcConstructing, rcIndex, rcCorrupt );
                            break;
                        }

                        /* record id and node for slot */
                        id2ord [ i ] = ( uint8_t ) id;
                        ord2node [ i ] = v1 . id2node [ id ];
                        ++ i;
                    }
                }
            }
        }
        else if ( id_bits <= 16 )
        {
            uint16_t *id2ord = malloc ( self -> count * 6 );
            if ( id2ord == NULL )
                rc = RC ( rcDB, rcIndex, rcConstructing, rcMemory, rcExhausted );
            else
            {
                ord2node = ( uint32_t* ) & id2ord [ self -> count ];
                self -> ord2node = ord2node;
                self -> id2ord . v16 = id2ord;
                self -> variant = 2;

                for ( i = id = 0; id < num_ids; ++ id )
                {
                    if ( v1 . id2node [ id ] != 0 )
                    {
                        if ( i == self -> count )
                        {
                            rc = RC ( rcDB, rcIndex, rcConstructing, rcIndex, rcCorrupt );
                            break;
                        }

                        id2ord [ i ] = ( uint16_t ) id;
                        ord2node [ i ] = v1 . id2node [ id ];
                        ++ i;
                    }
                }
            }
        }
        else
        {
            uint32_t *id2ord = malloc ( self -> count * 8 );
            if ( id2ord == NULL )
                rc = RC ( rcDB, rcIndex, rcConstructing, rcMemory, rcExhausted );
            else
            {
                ord2node = & id2ord [ self -> count ];
                self -> ord2node = ord2node;
                self -> id2ord . v32 = id2ord;
                self -> variant = 3;

                for ( i = id = 0; id < num_ids; ++ id )
                {
                    if ( v1 . id2node [ id ] != 0 )
                    {
                        if ( i == self -> count )
                        {
                            rc = RC ( rcDB, rcIndex, rcConstructing, rcIndex, rcCorrupt );
                            break;
                        }

                        id2ord [ i ] = id;
                        ord2node [ i ] = v1 . id2node [ id ];
                        ++ i;
                    }
                }
            }
        }

        if ( rc == 0 )
        {        
            if ( i == self -> count )
                return 0;
            rc = RC ( rcDB, rcIndex, rcConstructing, rcIndex, rcCorrupt );
        }

        KPTrieIndexWhack_v1 ( & v1 );
    }

    return rc;
}

rc_t KPTrieIndexInit_v2 ( KPTrieIndex_v2 *self, const KMMap *mm, bool byteswap )
{
    /* get size of map, assumed to be size of file */
    size_t size;
    rc_t rc = KMMapSize ( mm, & size );
    if ( rc == 0 )
    {
        const KPTrieIndexHdr_v2 *hdr;

#if ! KTRIE_ZEROS_KPTRIE
        self -> mm = NULL;
        self -> ord2node = NULL;
        self -> id2ord . v32 = NULL;
        self -> variant = 0;
#endif

        /* ignore empty file */
        if ( size == 0 )
        {
#if ! KTRIE_ZEROS_KPTRIE
            self -> first = self -> last = self -> maxid = 0;
            self -> key2id = NULL;
            self -> count = 0;
#endif
            return 0;
        }

        /* have to have at least the base header */
        if ( size < sizeof hdr -> dad )
            return RC ( rcDB, rcIndex, rcConstructing, rcTrie, rcCorrupt );

        rc = KMMapAddrRead ( mm, ( const void** ) & hdr );
        if ( rc == 0 )
        {
            /* recheck header size */
            if ( size < sizeof * hdr )
                return RC ( rcDB, rcIndex, rcConstructing, rcTrie, rcCorrupt );

            self -> first = hdr -> first;
            self -> last = self -> maxid = hdr -> last;
            self -> id_bits = ( uint8_t ) hdr -> id_bits;
            self -> span_bits = ( uint8_t ) hdr -> span_bits;
            self -> byteswap = byteswap;

            /* try to create the pttree */
            rc = PTrieMakeOrig ( & self -> key2id,
                hdr + 1, size -= sizeof * hdr, byteswap );
            if ( rc == 0 )
            {
                size_t ptsize = PTrieSize ( self -> key2id );
                if ( ptsize <= size )
                {
                    /* the count covers at least the number of trie nodes */
                    self -> count = PTrieCount ( self -> key2id );

                    /* it could be stored without projection */
                    if ( ptsize == size )
                        return 0;

                    /* calculate remaining bytes */                     
                    size -= ptsize;

                    /* there must be enough for an array of 4-byte node ids */
                    if ( size >= ( ( size_t ) self -> count << 2 ) )
                    {
                        /* take the persisted array as-is */
                        self -> ord2node = ( const uint32_t* )
                            ( ( const char* ) ( hdr + 1 ) + ptsize );

                        /* read the count */
                        if ( size >= 4 )
                        {
                            self -> count = * ( self -> ord2node ) ++;
                            size -= 4;
                        }

                        /* determine strategy from id span and count */
                        if ( ( self -> last - self -> first ) < ( ( int64_t ) self -> count << 1 ) )
                        {
                            /* must be contiguous */
                            self -> count = ( uint32_t ) ( self -> last - self -> first + 1 );

                            /* size should be exactly this number of slots */
                            if ( size == ( ( size_t ) self -> count << 2 ) )
                                return 0;

                            /* fall through to error return */
                        }
                        else if ( ( size == 4 && self -> count == 1 ) || size > ( ( size_t ) self -> count << 2 ) )
                        {
                            /* sparse */
                            size -= ( size_t ) self -> count << 2;

                            /* unpack id map */
                            if ( hdr -> id_bits <= 8 )
                                rc = KPTrieIndexInitID2Ord ( self, size, 1, hdr -> span_bits, 8 );
                            else if ( hdr -> id_bits <= 16 )
                                rc = KPTrieIndexInitID2Ord ( self, size, 2, hdr -> span_bits, 16 );
                            else if ( hdr -> id_bits <= 32 )
                                rc = KPTrieIndexInitID2Ord ( self, size, 3, hdr -> span_bits, 32 );
                            else
                                rc = KPTrieIndexInitID2Ord ( self, size, 4, hdr -> span_bits, 64 );

                            /* done */
                            if ( rc == 0 )
                                return 0;

                            PTrieWhack ( self -> key2id ), self -> key2id = NULL;
                            return rc;
                        }
                    }
                }

                PTrieWhack ( self -> key2id ), self -> key2id = NULL;
                rc = RC ( rcDB, rcIndex, rcConstructing, rcTrie, rcCorrupt );
            }
        }
    }

    return rc;
}

rc_t KPTrieIndexInit_v3_v4 ( KPTrieIndex_v2 *self, const KMMap *mm, bool byteswap, bool ptorig )
{
    /* get size of map, assumed to be size of file */
    size_t size;
    rc_t rc = KMMapSize ( mm, & size );
    if ( rc == 0 )
    {
        const KPTrieIndexHdr_v3 *hdr;

#if ! KTRIE_ZEROS_KPTRIE
        self -> mm = NULL;
        self -> ord2node = NULL;
        self -> id2ord . v32 = NULL;
        self -> variant = 0;
#endif

        /* ignore empty file */
        if ( size == 0 )
        {
#if ! KTRIE_ZEROS_KPTRIE
            self -> first = self -> last = self -> maxid = 0;
            self -> key2id = NULL;
            self -> count = 0;
#endif
            return 0;
        }

        /* have to have at least the base header */
        if ( size < sizeof hdr -> dad )
            return RC ( rcDB, rcIndex, rcConstructing, rcTrie, rcCorrupt );

        rc = KMMapAddrRead ( mm, ( const void** ) & hdr );
        if ( rc == 0 )
        {
            /* recheck header size */
            if ( size < sizeof * hdr )
                return RC ( rcDB, rcIndex, rcConstructing, rcTrie, rcCorrupt );

            self -> first = hdr -> first;
            self -> last = self -> maxid = hdr -> last;
            self -> id_bits = ( uint8_t ) hdr -> id_bits;
            self -> span_bits = ( uint8_t ) hdr -> span_bits;
            self -> byteswap = byteswap;

            /* try to create the pttree */
            rc = ( ptorig ? PTrieMakeOrig : PTrieMake )
                ( & self -> key2id, hdr + 1, size -= sizeof * hdr, byteswap );
            if ( rc == 0 )
            {
                size_t ptsize = PTrieSize ( self -> key2id );
                if ( ptsize <= size )
                {
                    /* the count covers at least the number of trie nodes */
                    self -> count = PTrieCount ( self -> key2id );

                    /* it could be stored without projection */
                    if ( ptsize == size )
                        return 0;

                    /* calculate remaining bytes */                     
                    size -= ptsize;

                    /* there must be enough for an array of 4-byte node ids */
                    if ( size >= ( ( size_t ) self -> count << 2 ) )
                    {
                        /* take the persisted array as-is */
                        self -> ord2node = ( const uint32_t* )
                            ( ( const char* ) ( hdr + 1 ) + ptsize );

                        /* read the count */
                        if ( size >= 4 )
                        {
                            self -> count = * ( self -> ord2node ) ++;
                            size -= 4;
                        }

                        /* determine strategy from id span and count */
                        if ( ( self -> last - self -> first ) < ( ( int64_t ) self -> count << 1 ) )
                        {
                            /* must be contiguous */
                            self -> count = ( uint32_t ) ( self -> last - self -> first + 1 );

                            /* size should be exactly this number of slots */
                            if ( size == ( ( size_t ) self -> count << 2 ) )
                                return 0;

                            /* fall through to error return */
                        }
                        else if ( ( size == 4 && self -> count == 1 ) || size > ( ( size_t ) self -> count << 2 ) )
                        {
                            /* sparse */
                            size -= ( size_t ) self -> count << 2;

                            /* unpack id map */
                            if ( hdr -> id_bits <= 8 )
                                rc = KPTrieIndexInitID2Ord ( self, size, 1, hdr -> span_bits, 8 );
                            else if ( hdr -> id_bits <= 16 )
                                rc = KPTrieIndexInitID2Ord ( self, size, 2, hdr -> span_bits, 16 );
                            else if ( hdr -> id_bits <= 32 )
                                rc = KPTrieIndexInitID2Ord ( self, size, 3, hdr -> span_bits, 32 );
                            else
                                rc = KPTrieIndexInitID2Ord ( self, size, 4, hdr -> span_bits, 64 );

                            /* done */
                            if ( rc == 0 )
                                return 0;

                            PTrieWhack ( self -> key2id ), self -> key2id = NULL;
                            return rc;
                        }
                    }
                }

                PTrieWhack ( self -> key2id ), self -> key2id = NULL;
                rc = RC ( rcDB, rcIndex, rcConstructing, rcTrie, rcCorrupt );
            }
        }
    }

    return rc;
}

/* Whack
 *  closes down keymap
 */
void KPTrieIndexWhack_v2 ( KPTrieIndex_v2 *self )
{
    free ( ( void* ) self -> id2ord . v8 );
    PTrieWhack ( self -> key2id );
    KMMapRelease ( self -> mm );
    memset ( self, 0, sizeof * self );
}

uint32_t KPTrieIndexID2Ord_v2 ( const KPTrieIndex_v2 *self, int64_t id )
{
    if ( id >= self -> first && id <= self -> maxid )
    {
        int64_t nid;
        uint32_t left, right, ord, count = self -> count;

        /* convert id either to a zero-based ord,
           or else the translated id in id2ord */
        id -= self -> first;

        /* handle type of projection */
        switch ( self -> variant )
        {
        case 0:
            /* return one-based ord */
            return ( uint32_t ) ( id + 1 );

        case 1:
            for ( left = 1, right = count; left <= right; )
            {
                ord = ( left + right ) >> 1;
                nid = self -> id2ord . v8 [ ord - 1 ];
                if ( id == nid )
                    return ord;

                if ( id < nid )
                {
                    right = ord - 1;
                    continue;
                }
                if ( ord == count )
                    return ord;

                nid = self -> id2ord . v8 [ ord ];
                if ( id < nid )
                    return ord;

                left = ord + 1;
            }
            break;

        case 2:
            for ( left = 1, right = count; left <= right; )
            {
                ord = ( left + right ) >> 1;
                nid = self -> id2ord . v16 [ ord - 1 ];
                if ( id == nid )
                    return ord;

                if ( id < nid )
                {
                    right = ord - 1;
                    continue;
                }
                if ( ord == count )
                    return ord;

                nid = self -> id2ord . v16 [ ord ];
                if ( id < nid )
                    return ord;

                left = ord + 1;
            }
            break;

        case 3:
            for ( left = 1, right = count; left <= right; )
            {
                ord = ( left + right ) >> 1;
                nid = self -> id2ord . v32 [ ord - 1 ];
                if ( id == nid )
                    return ord;

                if ( id < nid )
                {
                    right = ord - 1;
                    continue;
                }
                if ( ord == count )
                    return ord;

                nid = self -> id2ord . v32 [ ord ];
                if ( id < nid )
                    return ord;

                left = ord + 1;
            }
            break;

        case 4:
            for ( left = 1, right = count; left <= right; )
            {
                ord = ( left + right ) >> 1;
                nid = self -> id2ord . v64 [ ord - 1 ];
                if ( id == nid )
                    return ord;

                if ( id < nid )
                {
                    right = ord - 1;
                    continue;
                }
                if ( ord == count )
                    return ord;

                nid = self -> id2ord . v64 [ ord ];
                if ( id < nid )
                    return ord;

                left = ord + 1;
            }
            break;
        }
    }
    return 0;
}

static
rc_t KPTrieIndexProject_v2 ( const KPTrieIndex_v2 *self,
    int64_t id,
#if V2FIND_RETURNS_SPAN
    int64_t *start_id, uint32_t *span,
#endif
    char *key_buff, size_t buff_size, size_t *actsize )
{
    uint32_t nid, ord = KPTrieIndexID2Ord_v2 ( self, id );
    if ( ord != 0 )
    {
        assert ( start_id != NULL );
        assert ( span != NULL );

        nid = self -> ord2node [ ord - 1 ];

        switch ( self -> variant )
        {
        case 0:
            * start_id = id;
            for ( ; ord < self -> count; ++ ord )
            {
                if ( nid != self -> ord2node [ ord ] )
                    break;
            }
            * span = self -> first + ord - * start_id;
            break;
        case 1:
            * start_id = self -> id2ord . v8 [ ord - 1 ];
            * span = ( uint32_t ) ( ( ( ord == self -> count ) ?
                ( self -> maxid  - self -> first + 1 ) : self -> id2ord . v8 [ ord ] ) - * start_id );
            *start_id += self->first; 
            break;
        case 2:
            * start_id = self -> id2ord . v16 [ ord - 1 ];
            * span = ( uint32_t ) ( ( ( ord == self -> count ) ?
                ( self -> maxid  - self -> first + 1 ) : self -> id2ord . v16 [ ord ] ) - * start_id );
            *start_id += self->first; 
            break;
        case 3:
            * start_id = self -> id2ord . v32 [ ord - 1 ];
            * span = ( uint32_t ) ( ( ( ord == self -> count ) ?
                ( self -> maxid  - self -> first + 1 ) : self -> id2ord . v32 [ ord ] ) - * start_id );
            *start_id += self->first; 
            break;
        case 4:
            * start_id = self -> id2ord . v64 [ ord - 1 ];
            * span = ( uint32_t ) ( ( ( ord == self -> count ) ?
                ( self -> maxid  - self -> first + 1 ) : self -> id2ord . v64 [ ord ] ) - * start_id );
            *start_id += self->first; 
            break;
        }

        if ( nid != 0 )
        {
            rc_t rc;
            PTNode node;

            if ( self -> byteswap )
                nid = bswap_32 ( nid );

            rc = PTrieGetNode ( self -> key2id, & node, nid );
            if ( rc == 0 )
            {
                const String *key;
                rc = PTNodeMakeKey ( & node, & key );
                if ( rc == 0 )
                {
                    if (actsize)
                        *actsize = key -> size;
                    if ( key -> size >= buff_size )
                        rc = RC ( rcDB, rcIndex, rcProjecting, rcBuffer, rcInsufficient );
                    else
                        string_copy ( key_buff, buff_size, key -> addr, key -> size );

                    StringWhack ( ( String* ) key );
                }
            }
            return rc;
        }
    }

    return RC ( rcDB, rcIndex, rcProjecting, rcId, rcNotFound );
}

/* Find
 */
static
rc_t KPTrieIndexFind_v2 ( const KPTrieIndex_v2 *self,
    const char *str, int64_t *start_id,
#if V2FIND_RETURNS_SPAN
    uint32_t *span,
#endif
    int ( CC * custom_cmp ) ( const void *item, const PBSTNode *n, void *data ), void *data, bool convertFromV1 )
{
    rc_t rc;

    /* detect empty index */
    if ( self -> count == 0 )
        rc = RC ( rcDB, rcIndex, rcSelecting, rcString, rcNotFound );
    else
    {
        uint32_t nid;
        PTNode pnode;

        String key;
        StringInitCString ( & key, str );

        /* try to find string */
        nid = PTrieFind ( self -> key2id, & key, & pnode, custom_cmp, data );
        if ( nid == 0 )
            rc = RC ( rcDB, rcIndex, rcSelecting, rcString, rcNotFound );
        else
        {
            size_t usize;

            /* detect conversion from v1 */
            if ( convertFromV1 && self -> id_bits == 0 )
            {
                /* v1 stored tree will have just a 32-bit spot id as data */
                uint32_t id;
                assert ( pnode . data . size == sizeof id );
                memmove ( & id, pnode . data . addr, sizeof id );
                * start_id = id;
                rc = 0;
            }
            else
            {
                /* should be native v2 */
                rc = Unpack ( self -> id_bits, sizeof * start_id * 8,
                    pnode . data . addr, 0, self -> id_bits, NULL,
                    start_id, sizeof * start_id, & usize );
                * start_id += self -> first;
            }

            if ( rc == 0 )
            {
#if V2FIND_RETURNS_SPAN
                if ( self -> ord2node != NULL )
                {
                    uint32_t ord = KPTrieIndexID2Ord_v2 ( self, * start_id );
                    if ( ord == 0 )
                        rc = RC ( rcDB, rcIndex, rcSelecting, rcId, rcNotFound );
                    else if ( ord == self -> count )
                        * span = self -> maxid - * start_id + 1;
                    else switch ( self -> variant )
                    {
                    case 0:
                        for ( ; ord < self -> count; ++ ord )
                        {
                            if ( nid != self -> ord2node [ ord ] )
                                break;
                        }
                        * span = self -> first + ord - * start_id;
                        break;
                    case 1:
                        * span = self -> first + self -> id2ord . v8 [ ord ] - * start_id;
                        break;
                    case 2:
                        * span = self -> first + self -> id2ord . v16 [ ord ] - * start_id;
                        break;
                    case 3:
                        * span = self -> first + self -> id2ord . v32 [ ord ] - * start_id;
                        break;
                    case 4:
                        * span = self -> first + self -> id2ord . v64 [ ord ] - * start_id;
                        break;
                    }
                }
                else if ( self -> span_bits == 0 )
                    * span = 1;
                else
                {
                    rc = Unpack ( self -> span_bits, sizeof * span * 8,
                        pnode . data . addr, 0, self -> id_bits, NULL,
                        span, sizeof * span, & usize );
                }
#endif
            }
        }
    }

    return rc;
}


/*--------------------------------------------------------------------------
 * KTrieIdxNode_v2
 */

static
rc_t KTrieIdxNodeMake_v2_s1 ( KTrieIdxNode_v2_s1 **n, const String *key, int64_t id )
{
    rc_t rc = TNodeMake ( ( TNode** ) n, sizeof ** n + key -> size );
    if ( rc != 0 )
        rc = ResetRCContext ( rc, rcDB, rcIndex, rcInserting );
    else
    {
        KTrieIdxNode_v2_s1 *node = * n;
        string_copy ( node -> key, key -> size + 1, key -> addr, key -> size);
        StringInit ( & node -> n . key, node -> key, key -> size, key -> len );
        node -> start_id = id;
    }

    return rc;
}

static
rc_t KTrieIdxNodeMakeHole_v2_s1 ( KTrieIdxNode_v2_s1 **n, int64_t id )
{
    rc_t rc = TNodeMake ( ( TNode** ) n, sizeof ** n );
    if ( rc != 0 )
        rc = ResetRCContext ( rc, rcDB, rcIndex, rcInserting );
    else
    {
        KTrieIdxNode_v2_s1 *node = * n;
        node -> key [ 0 ] = 0;
        StringInit ( & node -> n . key, node -> key, 0, 0 );
        node -> start_id = id;
    }

    return rc;
}

static
rc_t KTrieIdxNodeMake_v2_s2 ( KTrieIdxNode_v2_s2 **n, const String *key, int64_t id )
{
    rc_t rc = TNodeMake ( ( TNode** ) n, sizeof ** n + key -> size );
    if ( rc != 0 )
        rc = ResetRCContext ( rc, rcDB, rcIndex, rcInserting );
    else
    {
        KTrieIdxNode_v2_s2 *node = * n;
        string_copy ( node -> key, key -> size + 1, key -> addr, key -> size);
        StringInit ( & node -> n . key, node -> key, key -> size, key -> len );
        node -> start_id = id;
        node -> span = 1;
    }
    return rc;
}

static
void CC KTrieIdxNodeWhack_v2 ( TNode *n, void *data )
{
    TNodeWhack ( n );
}

#if 0
static
void CC KTrieIdxNodeUnlink_v2 ( TNode *n, void *data )
{
    if ( TrieUnlink ( data, n ) )
        TNodeWhack ( n );
}
#endif


/*--------------------------------------------------------------------------
 * KTrieIndex_v2
 */

static
uint32_t KTrieIndexID2Ord_v2 ( const KTrieIndex_v2 *self, int64_t id )
{
    if ( id >= self -> first && id <= self -> last )
    {
        uint32_t left, right, count = self -> count;
        for ( left = 1, right = count; left <= right; )
        {
            uint32_t ord = ( left + right ) >> 1;
            const KTrieIdxNode_v2_s1 *node = self -> ord2node [ ord - 1 ];
            if ( id == node -> start_id )
                return ord;

            if ( id < node -> start_id )
            {
                right = ord - 1;
                continue;
            }

            if ( ord == count )
                return ord;

            node = self -> ord2node [ ord ];
            if ( id < node -> start_id )
                return ord;

            left = ord + 1;
        }
    }
    return 0;
}

static
uint32_t KTrieIndexNode2Ord_v2 ( const KTrieIndex_v2 *self, const KTrieIdxNode_v2_s1 *node )
{
    if ( self -> ord2node != NULL )
        return KTrieIndexID2Ord_v2 ( self, node -> start_id );
    return 0;
}

/* KTrieIndexWrite_v2
 */
typedef struct PersistTrieData PersistTrieData;
struct PersistTrieData
{
    uint64_t pos;
    KFile *f;
    KMD5File *fmd5;
    uint8_t *buffer;
    size_t bsize;
    size_t marker;

    int64_t first;
    size_t ptt_size;
    size_t node_data_size;
    uint16_t id_bits;
    uint16_t span_bits;
    rc_t rc;
};

static
rc_t CC KTrieIndexWrite_v2 ( void *param,
    const void *buffer, size_t size, size_t *num_writ )
{
    PersistTrieData *pb = param;
    size_t total, to_write;

    for ( total = 0; total < size; total += to_write )
    {
        to_write = size - total;
        if ( pb -> marker + to_write > pb -> bsize )
            to_write = pb -> bsize - pb -> marker;

        if ( to_write > 0 )
        {
            memmove ( pb -> buffer + pb -> marker,
                ( const uint8_t* ) buffer + total, to_write );
            pb -> marker += to_write;
        }

        if ( pb -> marker == pb -> bsize )
        {
            size_t num_flushed;
            pb -> rc = KFileWrite ( pb -> f, pb -> pos,
                pb -> buffer, pb -> bsize, & num_flushed );
            if ( pb -> rc != 0 )
            {
                * num_writ = 0;
                return pb -> rc;
            }

            if ( num_flushed == 0 )
            {
                * num_writ = total + to_write;
                return pb -> rc = RC ( rcDB, rcIndex, rcPersisting, rcTransfer, rcIncomplete );
            }

            pb -> marker -= num_flushed;
            pb -> pos += num_flushed;

            if ( pb -> marker != 0 )
                memmove ( pb -> buffer, pb -> buffer + num_flushed, pb -> marker );
        }
    }

    * num_writ = total;
    return 0;
}

/* KTrieIndexAux_v2
 */
static
rc_t CC KTrieIndexAux_v2_s1 ( void *param, const void *node, size_t *num_writ,
    PTWriteFunc write, void *write_param )
{
    PersistTrieData *pb = param;

    if ( write != NULL && pb -> node_data_size != 0 )
    {
        char buffer [ 8 ];
        const KTrieIdxNode_v2_s1 *n = node;

        /* pack from 64 possible bits down to total id span */
        if ( pb -> id_bits != 0 )
        {
            /* store name->id mapping as a simple translation
               from first, because we don't have easy access to
               neighboring nodes for storage as 1st derivative. */
            uint64_t idd = n -> start_id - pb -> first;

            bitsz_t psize;
            rc_t rc = Pack ( 64, pb -> id_bits, & idd,
                sizeof idd, NULL, buffer, 0, sizeof buffer * 8, & psize );
            if ( rc != 0 )
                return rc;

            /* the packing should produce a single unit */
            if ( psize != pb -> id_bits )
                return RC ( rcDB, rcIndex, rcPacking, rcData, rcCorrupt );
        }

        /* write out the node */
        return ( * write ) ( write_param, buffer, pb -> node_data_size, num_writ );
    }

    /* will always store an integral number of bytes */
    * num_writ = pb -> node_data_size;
    return 0;
}

static
rc_t CC KTrieIndexAux_v2_s2 ( void *param, const void *node, size_t *num_writ,
    PTWriteFunc write, void *write_param )
{
    PersistTrieData *pb = param;

    if ( write != NULL && pb -> node_data_size != 0 )
    {
        const KTrieIdxNode_v2_s2 *n = node;

        rc_t rc;
        char buffer [ 12 ];
        bitsz_t psize, offset;

        if ( pb -> id_bits == 0 )
            offset = 0;
        else
        {
            /* again store name->id mapping as a simple translation
               from first, but pack bits tightly */
            uint64_t idd = n -> start_id - pb -> first;
            rc = Pack ( 64, pb -> id_bits, & idd,
                sizeof idd, NULL, buffer, 0, sizeof buffer * 8, & offset );
            if ( rc != 0 )
                return rc;
            if ( offset != pb -> id_bits )
                return RC ( rcDB, rcIndex, rcPacking, rcData, rcCorrupt );
        }

        /* now pack id span down to a minimal number of bits
           6/8/09 - this is known to fail because Pack hasn't been
           updated to start on a non-0 bit offset */
        if ( pb -> span_bits != 0 )
        {
            rc = Pack ( 32, pb -> span_bits, & n -> span, sizeof n -> span,
                NULL, buffer, offset, sizeof buffer * 8 - offset, & psize );
            if ( rc != 0 )
                return rc;
            if ( psize != pb -> span_bits )
                return RC ( rcDB, rcIndex, rcPacking, rcData, rcCorrupt );
        }

        /* write out packed combination */
        return ( * write ) ( write_param, buffer, pb -> node_data_size, num_writ );
    }

    * num_writ = pb -> node_data_size;
    return 0;
}

/* KTrieIndexPersist_v*
 *  write keymap to indicated location
 */
#if KDBINDEXVERS == 2

static
void KTrieIndexPersistHdr_v2 ( KTrieIndex_v2 *self, PersistTrieData *pb )
{
    KPTrieIndexHdr_v2 *hdr;

    uint64_t total_id, test_id;
    uint32_t total_span, test_span;

    pb -> pos = 0;

    hdr = ( KPTrieIndexHdr_v2* ) pb -> buffer;
    pb -> marker = sizeof * hdr;

    /* stamp version header */
    KDBHdrInit(&hdr->dad, 2);

    /* store first and last ids */
    pb -> first = self -> first;
    hdr -> first = self -> first;
    hdr -> last = self -> last;

    /* calculate id bits - notice that
       total_id gets right shifted so that
       the loop is guaranteed to exit */
    total_id = self -> last - self -> first;
    if ( total_id == 0 )
        pb -> id_bits = 0;
    else for ( total_id >>= 1, pb -> id_bits = 1, test_id = 1;
          test_id <= total_id;
          ++ pb -> id_bits, test_id <<= 1 )
        ( void ) 0;

    /* if we have maintained a projection index,
       calculate max span now */
    if ( self -> ord2node != NULL )
    {
        uint32_t i, span, max_span;
        int64_t cur, prev = self -> first;
        for ( i = max_span = 1; i < self -> count; prev = cur, ++ i )
        {
            cur = self -> ord2node [ i ] -> start_id;
            span = ( uint32_t ) ( cur - prev );
            if ( span > max_span )
                max_span = span;
        }
        
        span = ( uint32_t ) ( self -> last - prev );
        if ( span > max_span )
            max_span = span;

        self -> max_span = max_span;
    }

    /* calculate span bits */
    total_span = self -> max_span;
    if ( total_span == 0 )
        pb -> span_bits = 0;
    else for ( total_span >>= 1, pb -> span_bits = 1, test_span = 1;
          test_span <= total_span;
          ++ pb -> span_bits, test_span <<= 1 )
        ( void ) 0;

    /* record these as header data */
    hdr -> id_bits = pb -> id_bits;
    hdr -> span_bits = pb -> span_bits;

    /* zero trailing junk */
    hdr -> align [ 0 ] = hdr -> align [ 1 ] = 0;
}

#else

static
void KTrieIndexPersistHdr_v3_v4 ( KTrieIndex_v2 *self, PersistTrieData *pb )
{
    KPTrieIndexHdr_v3 *hdr;

    uint64_t total_id, test_id;
    uint32_t total_span, test_span;

    pb -> pos = 0;

    hdr = ( KPTrieIndexHdr_v3* ) pb -> buffer;
    pb -> marker = sizeof * hdr;

    /* stamp version header */
    KDBHdrInit(&hdr->dad.h, KDBINDEXVERS);
    hdr->dad.index_type = kitText;

    /* store first and last ids */
    pb -> first = self -> first;
    hdr -> first = self -> first;
    hdr -> last = self -> last;

    /* calculate id bits - notice that
       total_id gets right shifted so that
       the loop is guaranteed to exit */
    total_id = self -> last - self -> first;
    if ( total_id == 0 )
        pb -> id_bits = 0;
    else for ( total_id >>= 1, pb -> id_bits = 1, test_id = 1;
          test_id <= total_id;
          ++ pb -> id_bits, test_id <<= 1 )
        ( void ) 0;

    /* if we have maintained a projection index,
       calculate max span now */
    if ( self -> ord2node != NULL )
    {
        uint32_t i, span, max_span;
        int64_t cur, prev = self -> first;
        for ( i = max_span = 1; i < self -> count; prev = cur, ++ i )
        {
            cur = self -> ord2node [ i ] -> start_id;
            span = ( uint32_t ) ( cur - prev );
            if ( span > max_span )
                max_span = span;
        }
        
        span = ( uint32_t ) ( self -> last - prev );
        if ( span > max_span )
            max_span = span;

        self -> max_span = max_span;
    }

    /* calculate span bits */
    total_span = self -> max_span;
    if ( total_span == 0 )
        pb -> span_bits = 0;
    else for ( total_span >>= 1, pb -> span_bits = 1, test_span = 1;
          test_span <= total_span;
          ++ pb -> span_bits, test_span <<= 1 )
        ( void ) 0;

    /* record these as header data */
    hdr -> id_bits = pb -> id_bits;
    hdr -> span_bits = pb -> span_bits;

    /* zero trailing junk */
    hdr -> align [ 0 ] = hdr -> align [ 1 ] = 0;
}

#endif 

static
rc_t KTrieIndexPersistTrie_v2 ( const KTrieIndex_v2 *self, PersistTrieData *pb )
{
    rc_t rc;

    /* persist the trie to file,
       using tree-internal key storage,
       capture the image size in pb */
    if ( self -> ord2node != NULL )
    {
        pb -> node_data_size = ( pb -> id_bits + 7 ) >> 3;
        rc = TriePersist ( & self -> key2id, & pb -> ptt_size,
            false, KTrieIndexWrite_v2, pb, KTrieIndexAux_v2_s1, pb );
    }
    else
    {
        pb -> node_data_size = ( pb -> id_bits + pb -> span_bits + 7 ) >> 3;
        rc = TriePersist ( & self -> key2id, & pb -> ptt_size,
            false, KTrieIndexWrite_v2, pb, KTrieIndexAux_v2_s2, pb );
    }

    if ( rc == 0 && pb -> marker != 0 )
    {
        size_t num_writ;
        rc = KFileWrite ( pb -> f, pb -> pos,
            pb -> buffer, pb -> marker, & num_writ );
        if ( rc == 0 && num_writ != pb -> marker )
            rc = RC ( rcDB, rcIndex, rcPersisting, rcTransfer, rcIncomplete );
    }

    return rc;
}


static
rc_t KTrieIndexPersistProjContig_v2 ( const KTrieIndex_v2 *self,
    PersistTrieData *pb, PTrie *tt, uint32_t *ord2node )
{
    uint32_t i, j, nid;
    int64_t id = self -> first;
    for ( i = j = nid = 0; i < self -> count; ++ id, ++ j, ++ i )
    {
        const KTrieIdxNode_v2_s1 *node = self -> ord2node [ i ];

        /* back fill repeats */
        for ( ; id < node -> start_id; ++ id )
            ord2node [ j ++ ] = nid;

        /* check for a hole in id space */
        if ( node -> n . key . size == 0 )
            nid = 0;
        else
        {
            PTNode pn;
            nid = PTrieFind ( tt, & node -> n . key, & pn, NULL, NULL );
            if ( nid == 0 )
                return RC ( rcDB, rcIndex, rcPersisting, rcTransfer, rcIncomplete );
        }

        /* record nid for i at j */
        ord2node [ j ] = nid;
    }

    /* finish off trailing span */
    for ( ; id <= self -> last; ++ id )
        ord2node [ j ++ ] = nid;

    return 0;
}

static
rc_t KTrieIndexPersistProjSparse_v2 ( const KTrieIndex_v2 *self,
    PersistTrieData *pb, PTrie *tt, uint32_t *ord2node, bitsz_t *psize )
{
    uint32_t i, nid;
    int64_t *id2ord = ( void* ) & ord2node [ self -> count ];
    for ( i = 0; i < self -> count; ++ i )
    {
        const KTrieIdxNode_v2_s1 *node = self -> ord2node [ i ];

        /* record negated id for i - see 1st derivative below */
        id2ord [ i ] = - node -> start_id;

        /* check for a hole in id space */
        if ( node -> n . key . size == 0 )
            nid = 0;
        else
        {
            PTNode pn;
            nid = PTrieFind ( tt, & node -> n . key, & pn, NULL, NULL );
            if ( nid == 0 )
                return RC ( rcDB, rcIndex, rcPersisting, rcTransfer, rcIncomplete );
        }

        /* record nid for i */
        ord2node [ i ] = nid;
    }

    /* produce first derivative of ids
       for any given pair, the 1st derivative is generally
       right - left, which is usually stored right, such that
       we start at the end and move left toward the start, i.e.

       right -= left and move left

       in this case, we want to eliminate the leading 0
       and shift everything down, so we produce the result
       to the left side and move right toward end, but this
       requires more complicated arithmetic in order to preserve
       right - left, i.e.

       left = right - left and move right

       to avoid this arithmetic, the ids were stored negated above
       which converts the operation into

       left -= right and move right
    */
    for ( i = 1; i < self -> count; ++ i )
        id2ord [ i - 1 ] -= id2ord [ i ];

    /* pack from 64 to span-bits */
    return Pack ( 64, pb -> span_bits, id2ord, ( size_t ) ( self -> count - 1 ) << 3,
                NULL, id2ord, 0, ( bitsz_t ) self -> count << 6, psize );
}

#if KDBINDEXVERS == 2

static
rc_t KTrieIndexPersistProj_v2 ( const KTrieIndex_v2 *self, PersistTrieData *pb )
{
    rc_t rc = 0;
    void * addr;
    size_t map_size;
    uint64_t file_size;
    size_t num_to_read;
    uint64_t num_ids;
    bool is_sparse;

    /* there must be something to do */
    if ( self -> count == 0 || self -> ord2node == NULL )
        return 0;

    /* calculate what kind of projection strategy to use:
       when avg ( id span ) <= 2.0, just use a straight array.
       otherwise, use two arrays: first for node ids and last
       being 1st derivative of positional start_ids.

       the calculation of the ratio would be
         num_ids = self -> last - self -> first + 1;
         ratio = num_ids / self -> count;
         if ( ratio <= 2.0 )
             use 1-1
         else
             use sparse

       by reorganizing the comparison, we get
         if ( num_ids <= 2 * self -> count )...
    */
    num_ids = self -> last - self -> first + 1;
    if ( num_ids <= ( ( uint64_t ) self -> count << 1 ) )
    {
        /* store 1-1 projection index */
        is_sparse = false;

        /* map size is 4 bytes per id */
        map_size = pb -> ptt_size + ( ( size_t ) num_ids << 2 );
    }
    else
    {
        /* store sparse projection index */
        is_sparse = true;

        /* map size for node ids is 4 bytes per slot */
        map_size = pb -> ptt_size + ( ( size_t ) self -> count << 2 );

        /* map size for 1st derivative ids is initially 8 bytes per slot
           used initially to store full ids and then reduced 1st deriv. */
        map_size += ( size_t ) self -> count << 3;
    }

    /* add in 4 bytes for count */
    map_size += 4;

    /* create an updatable region spanning from end of header,
       starting from PTrie and extending to end of projection index */
    addr = malloc ( map_size );
    if ( addr == NULL )
        rc = RC ( rcDB, rcIndex, rcPersisting, rcMemory, rcExhausted );
    else
    {
        size_t num_read;

        rc = KFileSize ( pb -> f, & file_size );
        num_to_read = file_size - sizeof ( KPTrieIndexHdr_v2 );
        if ( rc == 0 )
        {
            rc = KFileReadAll ( pb -> f, sizeof ( KPTrieIndexHdr_v2 ), addr,
			     num_to_read, & num_read );
            if ( rc == 0 )
            {
                if ( num_read != num_to_read )
                {
                    rc = RC ( rcDB, rcIndex, rcPersisting, rcHeader, rcInsufficient );
                }
            }
        }
        if ( rc != 0 )
            free ( addr );
    }

    if ( rc == 0 )
    {
        size_t num_writ;
        /* inflate the PTrie */
        PTrie *tt;
        rc = PTrieMakeOrig ( & tt, addr, pb -> ptt_size );
        if ( rc == 0 )
        {
            uint32_t *ord2node;
            assert ( pb -> ptt_size == PTrieSize ( tt ) );
            assert ( self -> count >= PTrieCount ( tt ) );
            ord2node = ( void* ) ( ( char* ) addr + pb -> ptt_size );
            assert ( ( ( size_t ) ord2node & 3 ) == 0 );
            
            /* set count */
            * ord2node ++ = self -> count;

            if ( ! is_sparse )
                rc = KTrieIndexPersistProjContig_v2 ( self, pb, tt, ord2node );
            else
            {
                bitsz_t psize;
                rc = KTrieIndexPersistProjSparse_v2 ( self, pb, tt, ord2node, & psize );
                if ( rc == 0 )
                {
                    map_size -= ( size_t ) self -> count << 3;
                    map_size += ( psize + 7 ) >> 3;
                }
            }

            /* done with pttree */
            PTrieWhack ( tt );
        }
        rc = KFileWrite ( pb -> f, file_size, 
			  (uint8_t*)addr + num_to_read,  map_size - num_to_read, & num_writ );
        if ( rc == 0  &&  num_writ != map_size - num_to_read )
            rc = RC ( rcDB, rcIndex, rcPersisting, rcHeader, rcInsufficient );
        free ( addr );
    }

    return rc;
}

#else

static
rc_t KTrieIndexPersistProj_v3 ( const KTrieIndex_v2 *self, PersistTrieData *pb )
{
    rc_t rc = 0;
    void * addr;
    size_t map_size;
    uint64_t file_size;
    size_t num_to_read;
    uint64_t num_ids;
    bool is_sparse;

    /* there must be something to do */
    if ( self -> count == 0 || self -> ord2node == NULL )
        return 0;

    /* calculate what kind of projection strategy to use:
       when avg ( id span ) <= 2.0, just use a straight array.
       otherwise, use two arrays: first for node ids and last
       being 1st derivative of positional start_ids.

       the calculation of the ratio would be
         num_ids = self -> last - self -> first + 1;
         ratio = num_ids / self -> count;
         if ( ratio <= 2.0 )
             use 1-1
         else
             use sparse

       by reorganizing the comparison, we get
         if ( num_ids <= 2 * self -> count )...
    */
    num_ids = self -> last - self -> first + 1;
    if ( num_ids <= ( ( uint64_t ) self -> count << 1 ) )
    {
        /* store 1-1 projection index */
        is_sparse = false;

        /* map size is 4 bytes per id */
        map_size = pb -> ptt_size + ( ( size_t ) num_ids << 2 );
    }
    else
    {
        /* store sparse projection index */
        is_sparse = true;

        /* map size for node ids is 4 bytes per slot */
        map_size = pb -> ptt_size + ( ( size_t ) self -> count << 2 );

        /* map size for 1st derivative ids is initially 8 bytes per slot
           used initially to store full ids and then reduced 1st deriv. */
        map_size += ( size_t ) self -> count << 3;
    }

    /* add in 4 bytes for count */
    map_size += 4;

    rc = KFileSize ( pb -> f, & file_size );
    if ( rc == 0 )
    {
        /* create an updatable region spanning from end of header,
           starting from PTrie and extending to end of projection index */
        addr = malloc ( map_size );
        if ( addr == NULL )
            rc = RC ( rcDB, rcIndex, rcPersisting, rcMemory, rcExhausted );
        else
        {
            size_t num_read;
            num_to_read = file_size - sizeof ( KPTrieIndexHdr_v3 );
            rc = KFileReadAll ( pb -> f, sizeof ( KPTrieIndexHdr_v3 ),
                addr, num_to_read, & num_read );
            if ( rc != 0 )
                free ( addr );
            else if ( num_read != num_to_read )
            {
                free ( addr );
                rc = RC ( rcDB, rcIndex, rcPersisting, rcHeader, rcInsufficient );
            }
        }
    }

    if ( rc == 0 )
    {
        size_t num_writ;
        /* inflate the PTrie */
        PTrie *tt;
#if KDBINDEXVERS > 3
        rc = PTrieMake ( & tt, addr, pb -> ptt_size, self -> pt . byteswap );
#else
        rc = PTrieMakeOrig ( & tt, addr, pb -> ptt_size, self -> pt . byteswap );
#endif
        if ( rc == 0 )
        {
            uint32_t *ord2node;
            assert ( pb -> ptt_size == PTrieSize ( tt ) );
            assert ( self -> count >= PTrieCount ( tt ) );
            ord2node = ( void* ) ( ( char* ) addr + pb -> ptt_size );
            assert ( ( ( size_t ) ord2node & 3 ) == 0 );
            
            /* set count */
            * ord2node ++ = self -> count;

            if ( ! is_sparse )
                rc = KTrieIndexPersistProjContig_v2 ( self, pb, tt, ord2node );
            else
            {
                bitsz_t psize;
                rc = KTrieIndexPersistProjSparse_v2 ( self, pb, tt, ord2node, & psize );
                if ( rc == 0 )
                {
                    map_size -= ( size_t ) self -> count << 3;
                    map_size += ( psize + 7 ) >> 3;
                }
            }

            /* done with pttree */
            PTrieWhack ( tt );

            if ( rc == 0 )
            {
                rc = KFileWrite ( pb -> f, file_size, 
                     ( uint8_t* ) addr + num_to_read,  map_size - num_to_read, & num_writ );

                if ( rc == 0  &&  num_writ != map_size - num_to_read )
                    rc = RC ( rcDB, rcIndex, rcPersisting, rcHeader, rcInsufficient );
            }
        }

        free ( addr );
    }

    return rc;
}

#endif

static
rc_t KTrieIndexCreateMD5Wrapper ( KDirectory *dir, KFile ** fp, KMD5File ** wrapper,
    char relpath [ 256 ], const char md5_relpath [ 260 ] )
{
    /* create the md5 file for read/write */
    KFile *f;
    rc_t rc = KDirectoryCreateFile ( dir, & f, true,
                                      0664, kcmInit, "%s", md5_relpath );
    if ( rc == 0 )
    {
        /* create an md5sum formatter */
        KMD5SumFmt *fmt;
        rc = KMD5SumFmtMakeUpdate ( & fmt, f );
        if ( rc == 0 )
        {
            int dot_pos;

            /* convert relative path to a leaf */
            char *leaf = strrchr ( relpath, '/' );
            if ( leaf ++ == NULL )
                leaf = relpath;

            /* trim off ".tmp" from "leaf"
               so that the format string reflects final name
               without the need to rename later */
            dot_pos = strlen ( leaf ) - 4;
            assert ( dot_pos > 0 );
            assert ( strcmp ( & leaf [ dot_pos ], ".tmp" ) == 0 );
            leaf [ dot_pos ] = 0;

            /* "fmt" now owns "f" */
            f = NULL;

            /* create a file wrapper that calculates and prints md5 */
            rc = KMD5FileMakeWrite ( wrapper, * fp, fmt, leaf );

            /* "wrapper" attaches a reference to "fmt", so we have to
               dump our reference regardless of "rc" */
            KMD5SumFmtRelease ( fmt );

            /* restore dot */
            leaf [ dot_pos ] = '.';

            /* if we succeeded, swap the "wrapper" for input file */
            if ( rc == 0 )
            {
                * fp = KMD5FileToKFile ( * wrapper );
                return 0;
            }
        }

        /* failed */
        KFileRelease ( f );
    }

    return rc;
}

rc_t KTrieIndexPersist_v2 ( const KTrieIndex_v2 *self,
    bool proj, KDirectory *dir, const char *path, bool use_md5 )
{
    rc_t rc;
    PersistTrieData pb;

    assert ( self != NULL );
    if ( self -> count == 0 )
        return 0;

    pb . fmd5 = NULL;

    /** Trie may have holes in serialization due to memory alignments ***/
    pb . buffer = calloc ( pb . bsize = 32 * 1024, 1 );
    if ( pb . buffer == NULL )
        rc = RC ( rcDB, rcIndex, rcPersisting, rcMemory, rcExhausted );
    else
    {
        /* determine the name of the file:
           it is created under a temporary name
           relative to the directory provided */
        char tmpname [ 256 ];
        rc = KDirectoryResolvePath ( dir, false,
            tmpname, sizeof tmpname, "%s.tmp", path );
        if ( rc == 0 )
        {
            /* create the name of temporary md5 file */
            char tmpmd5name [ 260 ];
            sprintf ( tmpmd5name, "%s.md5", tmpname );

            /* create the output file under temporary name
               ? why does it need read/write capabilities? */
            rc = KDirectoryCreateFile ( dir, & pb . f,
                                         true, 0664, kcmInit, "%s", tmpname );
            if ( rc == 0 )
            {
                /* if using md5, wrap output file */
                if ( use_md5 )
                    rc = KTrieIndexCreateMD5Wrapper ( dir, & pb . f, & pb . fmd5, tmpname, tmpmd5name );
                if ( rc == 0 )
                {
                    /* initial size */
                    pb . ptt_size = 0;
#if KDBINDEXVERS == 2
                    KTrieIndexPersistHdr_v2 ( ( KTrieIndex_v2* ) self, & pb );
#else
                    KTrieIndexPersistHdr_v3_v4 ( ( KTrieIndex_v2* ) self, & pb );
#endif

                    /* persist tree */
                    rc = KTrieIndexPersistTrie_v2 ( self, & pb );
                    if ( rc == 0 )
                    {
                        /* persist projection table */
                        if ( proj )
                        {
#if KDBINDEXVERS == 2
                            rc = KTrieIndexPersistProj_v2 ( self, & pb );
#else
                            rc = KTrieIndexPersistProj_v3 ( self, & pb );
#endif
                        }
                    }
                }
                    
                /* close down the file now, success or not */
                KFileRelease ( pb . f );
                pb . f = NULL;

                /* douse buffer and mark NULL in case of later attempt */
                free ( pb . buffer );
                pb . buffer = NULL;

                /* rename the files on success */
                if ( rc == 0 )
                {
                    /* works even if "path" is absolute */
                    rc = KDirectoryRename ( dir, false, tmpname, path );                        
                    if ( rc == 0 )
                    {
                        int tmplen;

                        /* done if this was the only file to rename */
                        if ( ! use_md5 )
                            return 0;

                        /* use "tmpname" as a real "md5" name */
                        tmplen = strlen ( tmpname );
                        assert ( strcmp ( & tmpname [ tmplen - 4 ], ".tmp" ) == 0 );
                        strcpy ( & tmpname [ tmplen - 3 ], "md5" );

                        /* rename md5 file and be done on success */
                        rc = KDirectoryRename ( dir, false, tmpmd5name, tmpname );
                        if ( rc == 0 )
                            return 0;

                        /* failure here means we have a good index file,
                           but a bad md5 file, so convert "tmpname" to the
                           actual name of the index file */
                        tmpname [ tmplen - 4 ] = 0;
                    }
                }

                /* failed, remove the output files here */
                KDirectoryRemove ( dir, false, "%s", tmpname );
                if ( use_md5 )
                    KDirectoryRemove ( dir, false, "%s", tmpmd5name );
            }
        }

        /* douse buffer */
        free ( pb . buffer );
    }
    
    return rc;
}


/* whack whack */
void KTrieIndexWhack_v2 ( KTrieIndex_v2 *self )
{
    KPTrieIndexWhack_v2 ( & self -> pt );
    TrieWhack ( & self -> key2id, KTrieIdxNodeWhack_v2, NULL );
    free ( self -> ord2node );
}

/* initialize an index from file - can be NULL */
rc_t KTrieIndexOpen_v2 ( KTrieIndex_v2 *self, const KMMap *mm, bool byteswap )
{
    rc_t rc;
    bool ptorig = false;
    const KDBHdr *hdr = NULL;

#if ! KTRIE_ZEROS_KPTRIE
#error "KTrie is supposed to zero KPTrie"
#endif
    memset ( self, 0, sizeof * self );
    self -> pt . byteswap = byteswap;

    /* create an empty Trie index,
       with numeric but auto-expand character set,
       and a bucket size of 512, beyond which the
       tree will branch.
    */
    rc = TrieInit ( & self -> key2id, "0-9", 512, true );
    if ( rc != 0 )
        return rc;

    /* when opened for create, there will be no existing index */
    if ( mm == NULL )
        return 0;

    rc = KMMapAddrRead ( mm, (const void**) & hdr );
    if ( rc != 0 )
        return rc;

    switch ( hdr -> version )
    {
    case 1:
        rc = KPTrieIndexInitFromV1_v2 ( & self -> pt, mm, byteswap );
        break;
    case 2:
        rc = KPTrieIndexInit_v2 ( & self -> pt, mm, byteswap );
        break;
    case 3:
        ptorig = true;
    case 4:
        rc = KPTrieIndexInit_v3_v4 ( & self -> pt, mm, byteswap, ptorig );
        break;
    default:
        rc = RC(rcDB, rcIndex, rcConstructing, rcIndex, rcBadVersion);
    }
    /* open the prior index in persisted mode, but
       don't load it into the core-based Trie */
    if ( rc == 0 )
    {
        /* the file existed but was empty */
        if ( self -> pt . key2id == NULL )
        {
            self -> pt . mm = NULL;
            return 0;
        }

        /* retain a reference to memory map */
        rc = KMMapAddRef ( mm );
        if ( rc == 0 )
        {
            self -> pt . mm = mm;
            return 0;
        }

        /* self -> pt gets whacked below */
    }

    KTrieIndexWhack_v2 ( self );
    return rc;
}

/* KTrieIndexPopulate_v2
 */
typedef struct KTrieIndexPopulateData_v2_s2 KTrieIndexPopulateData_v2_s2;
struct KTrieIndexPopulateData_v2_s2
{
    int64_t first;

    KTrieIndex_v2 *self;
    uint32_t count;
    rc_t rc;

    uint8_t id_bits;
    uint8_t span_bits;
};

static
bool CC KTrieIndexPopulate_v2_s2 ( PTNode *n, void *data )
{
    const String *key;
    KTrieIndexPopulateData_v2_s2 *pb = data;

    int64_t id;
    size_t usize;
    uint32_t span;

    /* capture node data */
    assert ( n -> data . size == sizeof id );
    pb -> rc = Unpack ( pb -> id_bits, sizeof id * 8,
        n -> data . addr, 0, pb -> id_bits, NULL, & id, sizeof id, & usize );
    if ( pb -> rc == 0 )
    {
        pb -> rc = Unpack ( pb -> span_bits, sizeof span * 8,
            n -> data . addr, pb -> id_bits, pb -> span_bits, NULL, & span, sizeof span, & usize );
    }
    if ( pb -> rc != 0 )
        return true;

    pb -> rc = PTNodeMakeKey ( n, & key );
    if ( pb -> rc == 0 )
    {
        KTrieIdxNode_v2_s2 *node;
        pb -> rc = KTrieIdxNodeMake_v2_s2 ( & node, key, id + pb -> first );
        StringWhack ( ( String* ) key );
        if ( pb -> rc == 0 )
        {
            node -> span = span;

            pb -> rc = TrieInsert ( & pb -> self -> key2id, & node -> n );
            if ( pb -> rc == 0 )
            {
                ++ pb -> count;
                return false;
            }

            KTrieIdxNodeWhack_v2 ( & node -> n, NULL );
        }
    }

    return true;
}

/* KTrieIndexAttach_v2
 *  attach a keymap to an existing table
 */
static
rc_t KTrieIndexPopulate_v2_s1 ( KTrieIndex_v2 *self, uint32_t i, int64_t idd )
{
    rc_t rc;
    uint32_t nid = self -> pt . ord2node [ i ];

    if ( i != 0 && self -> pt . ord2node [ i - 1 ] == nid )
        return 0;

    i = self -> count;

    if ( nid == 0 )
    {
        rc = KTrieIdxNodeMakeHole_v2_s1 ( & self -> ord2node [ i ], self -> pt . first + idd );
        if ( rc == 0 )
            self -> count = i + 1;
    }
    else
    {
        PTNode pnode;
        rc = PTrieGetNode ( self -> pt . key2id, & pnode, nid );
        if ( rc == 0 )
        {
            const String *key;
            rc = PTNodeMakeKey ( & pnode, & key );
            if ( rc == 0 )
            {
                rc = KTrieIdxNodeMake_v2_s1 ( & self -> ord2node [ i ],
                    key, self -> pt . first + idd );
                StringWhack ( ( String* ) key );
                if ( rc == 0 )
                {
                    rc = TrieInsert ( & self -> key2id, & self -> ord2node [ i ] -> n );
                    if ( rc != 0 )
                        KTrieIdxNodeWhack_v2 ( & self -> ord2node [ i ] -> n, NULL );
                    else
                        self -> count = i + 1;
                }
            }
        }
    }

    return rc;
}

rc_t KTrieIndexAttach_v2 ( KTrieIndex_v2 *self, bool proj )
{
    rc_t rc = 0;

    /* if persisted index is empty, bail */
    if ( self -> count != 0 || self -> pt . count == 0 )
        return 0;

    /* see if we can use existing projection index */
    if ( proj && self -> pt . ord2node != NULL )
    {
        uint32_t i;

        self -> ord2node =
            malloc ( ( ( self -> pt . count + 4095 ) & ~ 4095 ) * sizeof self -> ord2node [ 0 ] );
        if ( self -> ord2node == NULL )
            return RC ( rcDB, rcIndex, rcUpdating, rcMemory, rcExhausted );

        /* we were called because our count is 0 */
        assert ( self -> count == 0 );

        /* handle variant */
        assert ( self -> pt . variant == 0 || self -> pt . id2ord . v8 != NULL );
        switch ( self -> pt . variant )
        {
        case 0:  /* 1-1 id to name */
            for ( rc = 0, i = 0; i < self -> pt . count && rc == 0; ++ i )
                rc = KTrieIndexPopulate_v2_s1 ( self, i, i );
            break;
        case 1:  /* sparse 8-bit   */
            for ( rc = 0, i = 0; i < self -> pt . count && rc == 0; ++ i )
                rc = KTrieIndexPopulate_v2_s1 ( self, i, self -> pt . id2ord . v8 [ i ] );
            break;
        case 2:  /* sparse 16-bit  */
            for ( rc = 0, i = 0; i < self -> pt . count && rc == 0; ++ i )
                rc = KTrieIndexPopulate_v2_s1 ( self, i, self -> pt . id2ord . v16 [ i ] );
            break;
        case 3:  /* sparse 32-bit  */
            for ( rc = 0, i = 0; i < self -> pt . count && rc == 0; ++ i )
                rc = KTrieIndexPopulate_v2_s1 ( self, i, self -> pt . id2ord . v32 [ i ] );
            break;
        case 4:  /* sparse 64-bit  */
            for ( rc = 0, i = 0; i < self -> pt . count && rc == 0; ++ i )
                rc = KTrieIndexPopulate_v2_s1 ( self, i, self -> pt . id2ord . v64 [ i ] );
            break;
        }

        if ( rc != 0 )
        {
            for ( i = self -> count; i > 0; )
                KTrieIdxNodeWhack_v2 ( & self -> ord2node [ -- i ] -> n, NULL );
            free ( self -> ord2node ), self -> ord2node = NULL;
            return rc;
        }
    }
    else
    {
        KTrieIndexPopulateData_v2_s2 pb;
        pb . first = self -> pt . first;
        pb . self = self;
        pb . count = 0;
        pb . rc = 0;
        PTrieDoUntil ( self -> pt . key2id, KTrieIndexPopulate_v2_s2, & pb );
        if ( pb . rc == 0 && pb . count != self -> pt . count )
            return RC ( rcDB, rcIndex, rcUpdating, rcIndex, rcCorrupt );
        self -> count = pb . count;
    }

    /* record known dimensions */
    self -> first = self -> pt . first;
    self -> last = self -> pt . last;

    /* should be able to drop persisted copy now */
    KPTrieIndexWhack_v2 ( & self -> pt );

    return 0;
}

rc_t KTrieIndexInsert_v2 ( KTrieIndex_v2 *self,
    bool proj, const char *str, int64_t id )
{
    rc_t rc;
    String key;
    void *ord2node;
    uint32_t count;

#if DISABLE_PROJ
    proj = false;
#endif

    /* get the number of nodes in proj index or Trie.
       the persisted tree is only loaded into the in-core
       tree for edits ( insert/delete ), so the counts
       may differ. also, when in projection mode, the
       count refers to the number of array slots, which
       can be > the number of Trie nodes if there are
       holes in the id space. when not projecting, count
       is exactly the number of nodes in the Trie.
    */
    count = self -> count;

    /* detect first modification */
    if ( self -> count == 0 )
    {
        /* detect persisted data */
        if ( self -> pt . key2id != NULL )
        {
            /* load persisted data into core */
            rc = KTrieIndexAttach_v2 ( self, proj );
            if ( rc != 0 )
                return rc;

            /* should have loaded everything */
            assert ( self -> count != 0 );
            count = self -> count;
        }
    }

    /* v2 only allows increasing ids
       they don't have to be contiguous,
       but they cannot repeat and cannot decrease */
    else if ( id <= self -> last )
        return RC ( rcDB, rcIndex, rcInserting, rcConstraint, rcViolated );

    /* convert key to String */
    StringInitCString ( & key, str );

    /* insertion strategy depends upon projection index */
    if ( proj )
    {
        KTrieIdxNode_v2_s1 *node;

        /* check for extension of last node */
        if ( count != 0 )
        {
            /* a non-zero count implies nodes in projection array */
            assert ( self -> ord2node != NULL );

            /* get last node */
            node = self -> ord2node [ count - 1 ];
            assert ( node != NULL );

            /* if the keys match, this is an update to the node */
            if ( StringEqual ( & key, & node -> n . key ) )
            {
                /* this must be an extension of range */
                if ( id != self -> last + 1 )
                    return RC ( rcDB, rcIndex, rcInserting, rcConstraint, rcViolated );

                /* extend and done */
                self -> last = id;
                return 0;
            }

            /* not last node - create a hole if needed */
            if ( id != self -> last + 1 )
            {
                /* extend array if needed
                   should never have to handle initial insert,
                   but would be happy to do so if asked */
                if ( ( count & 4095 ) == 0 )
                {
                    ord2node = realloc ( self -> ord2node,
                        ( count + 4096 ) * sizeof self -> ord2node [ 0 ] );
                    if ( ord2node == NULL )
                        return RC ( rcDB, rcIndex, rcInserting, rcMemory, rcExhausted );
                    self -> ord2node = ord2node;
                }

                /* create NULL mapping */
                rc = KTrieIdxNodeMakeHole_v2_s1 ( & node, self -> last + 1 );
                if ( rc != 0 )
                    return rc;

                /* NB - this will cause count to be > num_nodes in Trie */
                self -> ord2node [ count ++ ] = node;
            }
        }

        /* make a new mapping starting with id */
        rc = KTrieIdxNodeMake_v2_s1 ( & node, & key, id );
        if ( rc == 0 )
        {
            /* attempt insertion */
            rc = TrieInsertUnique ( & self -> key2id, & node -> n, NULL );
            if ( rc == 0 )
            {
                /* create or extend projection array */
                if ( ( count & 4095 ) == 0 )
                {
                    ord2node = realloc ( self -> ord2node,
                        ( count + 4096 ) * sizeof self -> ord2node [ 0 ] );
                    if ( ord2node == NULL )
                        rc = RC ( rcDB, rcIndex, rcInserting, rcMemory, rcExhausted );
                    else
                        self -> ord2node = ord2node;
                }

                if ( rc == 0 )
                {
                    /* set/extend range, detecting first insertion */
                    self -> last = id;
                    if ( count == 0 )
                        self -> first = id;

                    /* project */
                    self -> ord2node [ count ] = node;
                    self -> count = count + 1;

                    /* insertion complete */
                    return 0;
                }

                /* remove failed insertion */
                TrieUnlink ( & self -> key2id, & node -> n );
            }

            /* clean up new node */
            KTrieIdxNodeWhack_v2 ( & node -> n, NULL );
        }

        /* clean up insertion of hole */
        if ( count != self -> count )
        {
            assert ( count - 1 == self -> count );
            KTrieIdxNodeWhack_v2 ( & self -> ord2node [ count - 1 ] -> n, NULL );
        }
    }
    else
    {
        KTrieIdxNode_v2_s2 *node;

        /* make a new mapping starting with id and a span of 1 */
        rc = KTrieIdxNodeMake_v2_s2 ( & node, & key, id );
        if ( rc == 0 )
        {
            /* attempt insertion */
            KTrieIdxNode_v2_s2 *exist;
            rc = TrieInsertUnique ( & self -> key2id, & node -> n, ( TNode** ) & exist );
            if ( rc == 0 )
            {
                /* set/extend range, detecting first insertion */
                if ( count == 0 )
                {
                    self -> max_span = 1;
                    self -> first = id;
                }
                self -> last = id;

                /* insertion complete */
                self -> count = count + 1;
                return 0;
            }

            /* clean up new node */
            KTrieIdxNodeWhack_v2 ( & node -> n, NULL );

            /* check existing for proper extension */
            if ( exist != NULL )
            {
                if ( id == exist -> start_id + exist -> span )
                {
                    assert ( count != 0 );

                    /* we already know id > last
                       and that it boarders the range of "exist"
                       so it must be last + 1 */
                    assert ( id - 1 == self -> last );
                    self -> last = id;

                    /* extend the span of "exist" */
                    ++ exist -> span;
                    if ( exist -> span > self -> max_span )
                        self -> max_span = exist -> span;

                    return 0;
                }
            }
        }
    }

    return rc;
}

/* drop string from trie and all mappings */
rc_t KTrieIndexDelete_v2 ( KTrieIndex_v2 *self, bool proj, const char *str )
{
    rc_t rc;
    String key;
    TNode *tnode;
    /* uint32_t count; */

#if DISABLE_PROJ
    proj = false;
#endif

    /* detect first modification */
    /* count = self -> count; */
    if ( self -> count != 0 )
    {
        /* detect persisted data */
        if ( self -> pt . key2id != NULL )
        {
            rc = KTrieIndexAttach_v2 ( self, proj );
            if ( rc != 0 )
                return rc;
        }
    }

    StringInitCString ( & key, str );

    /* interface states that all objects are dropped.
       however, this implementation only allows unique
       mappings to a contig range, so a simple find is sufficient */
    tnode = TrieFind ( & self -> key2id, & key );
    if ( tnode == NULL )
        return RC ( rcDB, rcIndex, rcRemoving, rcString, rcNotFound );

    /* remove from trie */
    TrieUnlink ( & self -> key2id, tnode );

    /* neutralize node in projection index */
    if ( proj )
    {
        KTrieIdxNode_v2_s1 *node = ( KTrieIdxNode_v2_s1* ) tnode;
        uint32_t ord = KTrieIndexNode2Ord_v2 ( self, node );
        if ( ord != 0 )
        {
            self -> ord2node [ ord - 1 ] -> n . key . size = 0;
            self -> ord2node [ ord - 1 ] -> n . key . len = 0;
            self -> ord2node [ ord - 1 ] -> key [ 0 ] = 0;
            return 0;
        }
    }

    /* whack node */
    KTrieIdxNodeWhack_v2 ( tnode, NULL );

    return 0;
}

/* map key to id range */
rc_t KTrieIndexFind_v2 ( const KTrieIndex_v2 *self,
    const char *str, int64_t *start_id,
#if V2FIND_RETURNS_SPAN
    uint32_t *span,
#endif
    int ( CC * custom_cmp ) ( const void *item, const PBSTNode *n, void *data ), void *data, bool convertFromV1 )
{
    /* search within in-core index */
    if ( self -> count != 0 )
    {
        const TNode *tnode;

        String key;
        StringInitCString ( & key, str );

        tnode = TrieFind ( & self -> key2id, & key );
        if ( tnode != NULL )
        {
            if ( self -> ord2node != NULL )
            {
                const KTrieIdxNode_v2_s1 *node = ( const KTrieIdxNode_v2_s1* ) tnode;
                uint32_t ord = KTrieIndexNode2Ord_v2 ( self, node );
                if ( ord == 0 )
                    return RC ( rcDB, rcIndex, rcSelecting, rcIndex, rcCorrupt );

                * start_id = node -> start_id;
#if V2FIND_RETURNS_SPAN
                if ( ord == self -> count )
                    * span = ( uint32_t ) ( self -> last - node -> start_id + 1 );
                else
                    * span = ( uint32_t ) ( self -> ord2node [ ord ] -> start_id - node -> start_id );
#endif
            }
            else
            {
                const KTrieIdxNode_v2_s2 *node = ( const KTrieIdxNode_v2_s2* ) tnode;
                * start_id = node -> start_id;
#if V2FIND_RETURNS_SPAN
                * span = node -> span;
#endif
            }

            return 0;
        }
    }

    /* search within persisted index */
    else if ( self -> pt . key2id != NULL )
    {
        return KPTrieIndexFind_v2 ( & self -> pt, str, start_id,
#if V2FIND_RETURNS_SPAN
                                    span,
#endif
                                    custom_cmp, data, convertFromV1 );
    }

    return RC ( rcDB, rcIndex, rcSelecting, rcString, rcNotFound );
}

/* projection index id to key-string */
typedef struct KTrieIndexProjectData_v2 KTrieIndexProjectData_v2;
struct KTrieIndexProjectData_v2
{
    int64_t id;
    const KTrieIdxNode_v2_s2 *node;
};

static
bool CC KTrieIndexProjectScan_v2 ( TNode *n, void *data )
{
    KTrieIndexProjectData_v2 *pb = (KTrieIndexProjectData_v2 *)data;
    const KTrieIdxNode_v2_s2 *node = ( const KTrieIdxNode_v2_s2* ) n;

    if ( pb -> id >= node -> start_id &&
         pb -> id < node -> start_id + node -> span )
    {
        pb -> node = node;
        return true;
    }

    return false;
}

rc_t KTrieIndexProject_v2 ( const KTrieIndex_v2 *self,
    int64_t id,
#if V2FIND_RETURNS_SPAN
     int64_t *start_id, uint32_t *span,
#endif
    char *key_buff, size_t buff_size, size_t *actsize )
{
    if ( self -> count != 0 )
    {
        if ( self -> ord2node != NULL )
        {
            uint32_t ord = KTrieIndexID2Ord_v2 ( self, id );
            if ( ord != 0 )
            {
                const KTrieIdxNode_v2_s1 *node = self -> ord2node [ ord - 1 ];
                
                if (actsize)
                    *actsize = node -> n . key . size;
                if ( node -> n . key . size >= buff_size )
                    return RC ( rcDB, rcIndex, rcProjecting, rcBuffer, rcInsufficient );
                string_copy ( key_buff, buff_size,
                    node -> n . key . addr, node -> n . key . size );
                * start_id = node -> start_id;
                * span = ( ( ord == self -> count ) ?
                    ( self -> last + 1 ) : ( self -> ord2node [ ord ] -> start_id ) ) - node -> start_id;
                return 0;
            }
        }
        else
        {
            KTrieIndexProjectData_v2 pb;
            pb . id = id;
            if ( TrieDoUntil ( & self -> key2id, KTrieIndexProjectScan_v2, & pb ) )
            {
                const KTrieIdxNode_v2_s2 *node = pb . node;
                
                if (actsize)
                    *actsize = node -> n . key . size;
                if ( node -> n . key . size >= buff_size )
                    return RC ( rcDB, rcIndex, rcProjecting, rcBuffer, rcInsufficient );
                string_copy ( key_buff, buff_size,
                    node -> n . key . addr, node -> n . key . size );
                * start_id = node -> start_id;
                * span = node -> span;
                return 0;
            }
        }
    }

    else if ( self -> pt . ord2node != NULL )
    {
        return KPTrieIndexProject_v2 ( & self -> pt, id,
#if V2FIND_RETURNS_SPAN
            start_id, span,
#endif
            key_buff, buff_size, actsize );
    }

    return RC ( rcDB, rcIndex, rcProjecting, rcId, rcNotFound );
}
