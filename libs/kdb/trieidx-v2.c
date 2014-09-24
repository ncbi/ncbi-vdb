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
#include "index-priv.h"
#include "trieidx-priv.h"
#include <klib/ptrie.h>
#include <klib/text.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/mmap.h>
#include <klib/pack.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <byteswap.h>
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
    memcpy ( & id, n -> data . addr, sizeof id );

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

        /* assume preservation of persisted projection index */
        self -> byteswap = byteswap;

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

        /* now decide whether to use 1-1 or sparse projection
           if the number of slots is less than twice the number of nodes,
           it is more efficient to store the nodes as a linear array,
           and represent missing ids as nulls.
         */
        if ( ( self -> last - self -> first ) < ( ( int64_t ) self -> count << 1 ) )
        {
            /* take the old projection array as-is,
               treating NULL node ids as holes */
            self -> ord2node = v1 . id2node;
            return 0;
        }

        /* need to create a new projection index */
        self -> byteswap = false;

        /* convert to sparse
           calculate id bits - notice that
           test_id gets left shifted so that
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
                        ord2node [ i ] = byteswap ? bswap_32 ( v1 . id2node [ id ] ) : v1 . id2node [ id ];
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
                        ord2node [ i ] = byteswap ? bswap_32 ( v1 . id2node [ id ] ) : v1 . id2node [ id ];
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
                        ord2node [ i ] = byteswap ? bswap_32 ( v1 . id2node [ id ] ) : v1 . id2node [ id ];
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
            uint16_t id_bits, span_bits;
            /* recheck header size */
            if ( size < sizeof * hdr )
                return RC ( rcDB, rcIndex, rcConstructing, rcTrie, rcCorrupt );

            if ( self -> byteswap )
            {
                self -> first = bswap_64(hdr -> first);
                self -> last = self -> maxid = bswap_64(hdr -> last);
                id_bits = bswap_16(hdr -> id_bits);
                span_bits = bswap_16(hdr -> span_bits);
            }
            else
            {
                self -> first = hdr -> first;
                self -> last = self -> maxid = hdr -> last;
                id_bits = hdr -> id_bits;
                span_bits = hdr -> span_bits;
            }
            self -> id_bits = ( uint8_t ) id_bits;
            self -> span_bits = ( uint8_t ) span_bits;
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
                            if ( byteswap )
                                self -> count = bswap_32 ( self -> count );
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
                            if ( id_bits <= 8 )
                                rc = KPTrieIndexInitID2Ord ( self, size, 1, span_bits, 8 );
                            else if ( id_bits <= 16 )
                                rc = KPTrieIndexInitID2Ord ( self, size, 2, span_bits, 16 );
                            else if ( id_bits <= 32 )
                                rc = KPTrieIndexInitID2Ord ( self, size, 3, span_bits, 32 );
                            else
                                rc = KPTrieIndexInitID2Ord ( self, size, 4, span_bits, 64 );

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
            uint16_t id_bits, span_bits;
            /* recheck header size */
            if ( size < sizeof * hdr )
                return RC ( rcDB, rcIndex, rcConstructing, rcTrie, rcCorrupt );

            if ( self -> byteswap )
            {
                self -> first = bswap_64(hdr -> first);
                self -> last = self -> maxid = bswap_64(hdr -> last);
                id_bits = bswap_16(hdr -> id_bits);
                span_bits = bswap_16(hdr -> span_bits);
            }
            else
            {
                self -> first = hdr -> first;
                self -> last = self -> maxid = hdr -> last;
                id_bits = hdr -> id_bits;
                span_bits = hdr -> span_bits;
            }
            self -> id_bits = ( uint8_t ) id_bits;
            self -> span_bits = ( uint8_t ) span_bits;
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
                            if ( byteswap )
                                self -> count = bswap_32 ( self -> count );
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
                            if ( id_bits <= 8 )
                                rc = KPTrieIndexInitID2Ord ( self, size, 1, span_bits, 8 );
                            else if ( id_bits <= 16 )
                                rc = KPTrieIndexInitID2Ord ( self, size, 2, span_bits, 16 );
                            else if ( id_bits <= 32 )
                                rc = KPTrieIndexInitID2Ord ( self, size, 3, span_bits, 32 );
                            else
                                rc = KPTrieIndexInitID2Ord ( self, size, 4, span_bits, 64 );

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
            * span = ( uint32_t ) ( self -> first + ord - * start_id );
            break;
        case 1:
            * start_id = self -> id2ord . v8 [ ord - 1 ];
            * span = ( uint32_t ) ( ( ( ord == self -> count ) ?
                ( self -> maxid - self -> first + 1 ) : self -> id2ord . v8 [ ord ] ) - * start_id );
            *start_id += self -> first;
            break;
        case 2:
            * start_id = self -> id2ord . v16 [ ord - 1 ];
            * span = ( uint32_t ) ( ( ( ord == self -> count ) ?
                ( self -> maxid - self -> first + 1 ) : self -> id2ord . v16 [ ord ] ) - * start_id );
            *start_id += self -> first; 
            break;
        case 3:
            * start_id = self -> id2ord . v32 [ ord - 1 ];
            * span = ( uint32_t ) ( ( ( ord == self -> count ) ?
                ( self -> maxid - self -> first + 1 ) : self -> id2ord . v32 [ ord ] ) - * start_id );
            *start_id += self -> first; 
            break;
        case 4:
            * start_id = self -> id2ord . v64 [ ord - 1 ];
            * span = ( uint32_t ) ( ( ( ord == self -> count ) ?
                ( self -> maxid - self -> first + 1 ) : self -> id2ord . v64 [ ord ] ) - * start_id );
            *start_id += self -> first; 
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
                    if ( actsize != NULL )
                        * actsize = key -> size;
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

    /* search within in-core index */
    if ( self -> count == 0 )
        rc = RC ( rcDB, rcIndex, rcSelecting, rcString, rcNotFound );
    else
    {
        uint32_t nid;
        PTNode pnode;

        String key;
        StringInitCString ( & key, str );

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
                memcpy ( & id, pnode . data . addr, sizeof id );
                * start_id = id;
                rc = 0;
            }
            else
            {
                /* should be native v2 */
                if ( self -> id_bits > 0 )
                {
                    rc = Unpack ( self -> id_bits, sizeof * start_id * 8,
                        pnode . data . addr, 0, self -> id_bits, NULL,
                        start_id, sizeof * start_id, & usize );
                }
                else
                {
                    rc = 0;
                }
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
                        * span = ( uint32_t ) ( self -> maxid - * start_id + 1 );
                    else switch ( self -> variant )
                    {
                    case 0:
                        for ( ; ord < self -> count; ++ ord )
                        {
                            if ( nid != self -> ord2node [ ord ] )
                                break;
                        }
                        * span = ( uint32_t ) ( self -> first + ord - * start_id );
                        break;
                    case 1:
                        * span = ( uint32_t ) ( self -> first + self -> id2ord . v8 [ ord ] - * start_id );
                        break;
                    case 2:
                        * span = ( uint32_t ) ( self -> first + self -> id2ord . v16 [ ord ] - * start_id );
                        break;
                    case 3:
                        * span = ( uint32_t ) ( self -> first + self -> id2ord . v32 [ ord ] - * start_id );
                        break;
                    case 4:
                        * span = ( uint32_t ) ( self -> first + self -> id2ord . v64 [ ord ] - * start_id );
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
 * KTrieIndex_v2
 */


/* whack whack */
void KTrieIndexWhack_v2 ( KTrieIndex_v2 *self )
{
    KPTrieIndexWhack_v2 ( & self -> pt );
}

/* initialize an index from file */
rc_t KTrieIndexOpen_v2 ( KTrieIndex_v2 *self, const KMMap *mm, bool byteswap )
{
    rc_t rc;
    uint32_t version;
    bool ptorig = false;
    const KDBHdr *hdr = NULL;

#if ! KTRIE_ZEROS_KPTRIE
#error "KTrie is supposed to zero KPTrie"
#endif
    memset ( self, 0, sizeof * self );

    /* open the prior index in persisted mode, but
       don't load it into the core-based Trie */
    rc = KMMapAddrRead ( mm, ( const void** ) & hdr );
    if ( rc != 0 )
        return rc;

    self -> pt . byteswap = byteswap;
    version = byteswap ? bswap_32 ( hdr -> version ) : hdr -> version;

    switch ( version )
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
        break;
    }
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


/* map key to id range */
rc_t KTrieIndexFind_v2 ( const KTrieIndex_v2 *self,
    const char *str, int64_t *start_id,
#if V2FIND_RETURNS_SPAN
    uint32_t *span,
#endif
    int ( CC * custom_cmp ) ( const void *item, const PBSTNode *n, void *data ), void *data, bool convertFromV1  )
{
    /* search within persisted index */
    if ( self -> pt . key2id != NULL )
    {
        return KPTrieIndexFind_v2 ( & self -> pt, str, start_id,
#if V2FIND_RETURNS_SPAN
                                    span,
#endif
                                    custom_cmp, data, convertFromV1 );
    }

    return RC ( rcDB, rcIndex, rcSelecting, rcString, rcNotFound );
}

rc_t KTrieIndexProject_v2 ( const KTrieIndex_v2 *self,
    int64_t id,
#if V2FIND_RETURNS_SPAN
     int64_t *start_id, uint32_t *span,
#endif
    char *key_buff, size_t buff_size, size_t *actsize )
{
    if ( self -> pt . ord2node == NULL )
        return RC ( rcDB, rcIndex, rcProjecting, rcId, rcNotFound );

    return KPTrieIndexProject_v2 ( & self -> pt, id,
#if V2FIND_RETURNS_SPAN
       start_id, span,
#endif
        key_buff, buff_size, actsize );
}
