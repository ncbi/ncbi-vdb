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

#define BTREE_KEY2ID 1

#include <kdb/extern.h>
#include <kdb/btree.h>
#include <kfs/file.h>
#include <kfs/pagefile.h>
#include <klib/refcount.h>
#include <klib/sort.h>
#include <klib/rc.h>
#include <klib/text.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* must be kept in sync with kfs/pagefile.c */
#define PGBITS 15
#define PGSIZE ( 1U << PGBITS )

#if _DEBUGGING
static
rc_t page_access_read ( const KPage *page, const void **mem )
{
    size_t bytes;
    rc_t rc = KPageAccessRead ( page, mem, & bytes );
    if ( rc == 0 && bytes != PGSIZE )
        rc = RC ( rcDB, rcBlob, rcAccessing, rcFormat, rcIncorrect );
    return rc;
}

static
rc_t page_access_update ( KPage *page, void **mem )
{
    size_t bytes;
    rc_t rc = KPageAccessUpdate ( page, mem, & bytes );
    if ( rc == 0 && bytes != PGSIZE )
        rc = RC ( rcDB, rcBlob, rcAccessing, rcFormat, rcIncorrect );
    return rc;
}
#else

#define page_access_read( page, mem ) \
    KPageAccessRead ( page, mem, NULL )
#define page_access_update( page, mem ) \
    KPageAccessUpdate ( page, mem, NULL )

#endif


#if ! BTREE_KEY2ID

/*--------------------------------------------------------------------------
 * KBTreeValue
 *  gives access to a value within a data page
 */
typedef struct KBTreeValueImpl KBTreeValueImpl;
struct KBTreeValueImpl
{
    KPage *page;
    size_t offset;
    size_t size;
};

/* Whack
 *  destroys object for further use
 *  may write modifications to disk ( see "write_through" below )
 */
LIB_EXPORT rc_t CC KBTreeValueWhack ( KBTreeValue *xself )
{
    KBTreeValueImpl *self = ( KBTreeValueImpl* ) xself;
    assert ( sizeof * self <= sizeof * xself );
    if ( self != NULL )
    {
        rc_t rc = KPageRelease ( self -> page );
        memset ( self, 0, sizeof * xself );
        return rc;
    }
    return 0;
}


/* AccessRead
 * AccessUpdate
 *  gain access to value address
 *  update access marks data page as modified
 *
 *  "mem" [ OUT ] - pointer to value
 *
 *  "bytes" [ OUT, NULL OKAY ] - size of value memory
 */
LIB_EXPORT rc_t CC KBTreeValueAccessRead ( const KBTreeValue *xself,
    const void **mem, size_t *bytes )
{
    rc_t rc;
    size_t dummy;

    const KBTreeValueImpl *self = ( const KBTreeValueImpl* ) xself;
    assert ( sizeof * self <= sizeof * xself );

    if ( bytes == NULL )
        bytes = & dummy;

    if ( mem == NULL )
        rc = RC ( rcDB, rcBlob, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcBlob, rcAccessing, rcSelf, rcNull );
        else
        {
            rc = KPageAccessRead ( self -> page, mem, bytes );
            if ( rc == 0 )
            {
                const uint8_t *byte_ptr = * mem;
                assert ( self -> offset + self -> size <= * bytes );
                * bytes = self -> size;
                * mem = byte_ptr + self -> offset;
                return 0;
            }
        }

        * mem = NULL;
    }

    * bytes = 0;
    return rc;
}

LIB_EXPORT rc_t CC KBTreeValueAccessUpdate ( KBTreeValue *xself, void **mem, size_t *bytes )
{
    rc_t rc;
    size_t dummy;

    KBTreeValueImpl *self = ( KBTreeValueImpl* ) xself;
    assert ( sizeof * self <= sizeof * xself );

    if ( bytes == NULL )
        bytes = & dummy;

    if ( mem == NULL )
        rc = RC ( rcDB, rcBlob, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcBlob, rcAccessing, rcSelf, rcNull );
        else
        {
            rc = KPageAccessUpdate ( self -> page, mem, bytes );
            if ( rc == 0 )
            {
                uint8_t *byte_ptr = * mem;
                assert ( self -> offset + self -> size <= * bytes );
                * bytes = self -> size;
                * mem = byte_ptr + self -> offset;
                return 0;
            }
        }

        * mem = NULL;
    }

    * bytes = 0;
    return rc;
}


/*--------------------------------------------------------------------------
 * KPageFileStream
 *  represents a data stream made of pages and nodes
 *
 *  empty representation
 *    eof == 0, root == 0
 *
 *  simplest representation:
 *    0 < eof <= PGSIZE, root != 0 :: root page is data
 *
 *  common representation:
 *    PGSIZE < eof, root != 0 :: root page is a node
 */
typedef struct KPageFileStream KPageFileStream;
struct KPageFileStream
{
    uint64_t eof;                         /* logical end of file */
    uint32_t root;                        /* starting page id    */
    uint32_t min_size;                    /* minimum entry size  */
    uint32_t max_size;                    /* maximum entry size  */
    uint32_t chunk_bytes;                 /* alignment factor    */
    uint32_t page_adj;                    /* bytes to adjust pg  */
    uint32_t size_bytes;                  /* leading size bytes  */
};

#define SNBITS ( PGBITS - 2 )
#define SNSIZE ( 1U << SNBITS )

typedef struct KStreamNode KStreamNode;
struct KStreamNode
{
    uint32_t page [ SNSIZE ];             /* data or node pages  */
};


/* Init
 */
static
rc_t KPageFileStreamInit ( KPageFileStream *s, size_t min_size, size_t max_size, size_t chunk_size )
{
    /* zero is a good default for most things */
    memset ( s, 0, sizeof * s );

    /* min and max size must be reasonable */
    if ( min_size > max_size || max_size == 0 )
        return RC ( rcDB, rcFile, rcConstructing, rcRange, rcInvalid );
    if ( max_size > PGSIZE )
        return RC ( rcDB, rcFile, rcConstructing, rcRange, rcExcessive );

    /* chunk size cannot be too large, either */
    if ( chunk_size > PGSIZE )
        return RC ( rcDB, rcFile, rcConstructing, rcParam, rcExcessive );

    /* record limits */
    s -> min_size = ( uint32_t ) min_size;
    s -> max_size = ( uint32_t ) max_size;

    /* detect size bytes */
    if ( min_size == max_size )
        s -> chunk_bytes = ( uint32_t ) max_size;
    else
    {
        if ( max_size <= UINT8_MAX )
            s -> size_bytes = 1;
        else if ( max_size <= UINT16_MAX )
            s -> size_bytes = 2;
        else
            s -> size_bytes = 4;

        s -> chunk_bytes = ( uint32_t ) chunk_size;
    }

    /* produce page adjustment for chunking */
    s -> page_adj = PGSIZE % s -> chunk_bytes;

    return 0;
}


/* Check
 */
static
rc_t KPageFileStreamCheck ( KPageFileStream *self, size_t min_size, size_t max_size, size_t chunk_size )
{
    if ( self -> eof == 0 )
    {
        assert ( self -> root == 0 );
        return KPageFileStreamInit ( self, min_size, max_size, chunk_size );
    }

    assert ( self -> root != 0 );
    if ( self -> min_size > min_size || self -> max_size < max_size )
        return RC ( rcDB, rcFile, rcConstructing, rcParam, rcInconsistent );

    return 0;
}


/* Alloc
 *  allocate space in stream
 *
 *  there are 7 cases to handle within this function
 *
 *  1. allocation within first page of data, including initial allocation
 *  2. initial allocation from second data page
 *  3. no space in root node for allocation
 *  4. allocation crosses slot boundaries
 *  5. end slot is null
 *  6. end slot has data page
 *  7. end slot has node page
 */
static
rc_t KPageFileStreamAlloc ( KPageFileStream *self, KPageFile *pgfile, KBTreeValue *valp, size_t size, uint32_t *chunk )
{
    /* give return parameter a type */
    KBTreeValueImpl *val = ( KBTreeValueImpl* ) valp;

    rc_t rc;
    KPage *page;
    uint32_t new_root;
    KStreamNode *node;
    uint64_t start, end, chunk_mask;
    uint64_t num_pages, virt_eof;

    /* chunk parameter can be NULL */
    uint32_t dummy;
    if ( chunk == NULL )
        chunk = & dummy;

    /* we're static - these should be okay */
    assert ( self != NULL );
    assert ( pgfile != NULL );
    assert ( val != NULL );

    /* check against size constraint */
    if ( size < self -> min_size || size > self -> max_size )
        return RC ( rcDB, rcFile, rcAllocating, rcConstraint, rcViolated );

    /* calcuate starting offset */
    chunk_mask = self -> chunk_bytes - 1;
    if ( self -> page_adj != 0 )
    {
        /* this is a case where the chunking is not a power of two
           which is generally used only for fixed sized elements.
           the case is that a single data page will not completely
           fill with an integral number of elements, leaving an amount
           in "page_adj" for each page used.

           for simplicity, the actual eof is converted to a virtual
           eof by imagining contiguous storage, subtracting off the
           page adjust for each page used.

           the chunk alignment is then performed against the virtual
           eof, giving a new start offset which is finally converted
           back to actual space by adding in an adjustment for each
           page used.
        */
        
        num_pages = self -> eof >> PGBITS;
        virt_eof = self -> eof - num_pages * self -> page_adj; 
        start = virt_eof + chunk_mask;
        start -= start % ( chunk_mask + 1 );
        if ( ( ( start / self -> chunk_bytes ) >> 32 ) != 0 )
            return RC ( rcDB, rcFile, rcAllocating, rcId, rcExhausted );
        * chunk = start / self -> chunk_bytes;
        start += ( start / ( PGSIZE - self -> page_adj ) ) * self -> page_adj;
    }
    else
    {
        /* this is the general case where chunk size is a nice power of 2 */
        assert ( ( self -> chunk_bytes & ( self -> chunk_bytes - 1 ) ) == 0 );
        start = ( self -> eof + chunk_mask ) & ~ chunk_mask;
        if ( ( ( start / self -> chunk_bytes ) >> 32 ) != 0 )
            return RC ( rcDB, rcFile, rcAllocating, rcId, rcExhausted );
        * chunk = start / self -> chunk_bytes;
    }

    end = start + size + self -> size_bytes;

    /* detect need to go to next page */
    if ( ( start >> PGBITS ) != ( ( end - 1 ) >> PGBITS ) )
    {
        start = ( start + PGSIZE - 1 ) & ~ ( uint64_t ) ( PGSIZE - 1 );
        end = start + size;
        assert ( ( start >> PGBITS ) == ( ( end - 1 ) >> PGBITS ) );

        /* reassign chunk */
        if ( self -> page_adj != 0 )
        {
            num_pages = start >> PGBITS;
            virt_eof = start - num_pages * self -> page_adj; 
            virt_eof += chunk_mask;
            virt_eof -= virt_eof % ( chunk_mask + 1 );
            if ( ( ( virt_eof / self -> chunk_bytes ) >> 32 ) != 0 )
                return RC ( rcDB, rcFile, rcAllocating, rcId, rcExhausted );
            * chunk = virt_eof / self -> chunk_bytes;
        }
        else
        {
            * chunk = start / self -> chunk_bytes;
        }
    }

    /* CASE 1 - stream is empty or has single data page */
    if ( end <= PGSIZE )
    {
        /* return data page reference in "val" */
        if ( self -> root == 0 )
            /* create initial root data page */
            rc = KPageFileAlloc ( pgfile, & val -> page, & self -> root );
        else
            /* access existing page */
            rc = KPageFileGet ( pgfile, & val -> page, self -> root );
    }
    /* CASE 2 - conversion from data to node page */
    else if ( self -> eof <= PGSIZE )
    {
        /* create new root */
        rc = KPageFileAlloc ( pgfile, & page, & new_root );
        if ( rc == 0 )
        {
            rc = page_access_update ( page, ( void** ) & node );
            if ( rc == 0 )
            {
                /* put old root in slot 0 */
                node -> page [ 0 ] = self -> root;

                /* create new data page, returning reference in "val" */
                rc = KPageFileAlloc ( pgfile, & val -> page, & node -> page [ 1 ] );
                if ( rc == 0 )

                    /* update root */
                    self -> root = new_root;
            }

            /* release new root page */
            KPageRelease ( page );
        }
    }
    /* root is a node page and eof > PGSIZE, node_size */
    else
    {
        /* bytes currently represented by root node */
        uint64_t slot_size = PGSIZE;
        while ( self -> eof > slot_size )
            slot_size <<= SNBITS;

        /* CASE 3 - create a new root page due to overflow at this level */
        rc = 0;
        if ( end > slot_size )
        {
            /* create a new root as before */
            rc = KPageFileAlloc ( pgfile, & page, & new_root );
            if ( rc == 0 )
            {
                rc = page_access_update ( page, ( void** ) & node );
                if ( rc == 0 )
                {
                    /* put old root in slot 0 */
                    node -> page [ 0 ] = self -> root;

                    /* update root */
                    self -> root = new_root;

                    /* root size is now much bigger */
                    slot_size <<= SNBITS;
                }
                
                KPageRelease ( page );
            }
        }
        if ( rc == 0 )
        {
            bool done;
            uint32_t page_id, slot_id;

            /* convert to size represented by each slot */
            slot_size >>= SNBITS;

            /* slot number
               NB - "end" has been modified to point AT last byte */
            slot_id = ( uint32_t ) ( -- end / slot_size );

            for ( done = false, page_id = self -> root; rc == 0 && ! done; )
            {
                rc = KPageFileGet ( pgfile, & page, page_id );
                if ( rc == 0 )
                {
                    const KStreamNode *cnode;
                    rc = page_access_read ( page, ( const void** ) & cnode );
                    if ( rc == 0 )
                    {
                        /* slots hold data pages */
                        if ( slot_size == PGSIZE )
                        {
                            /* return reference to page in "val" */
                            if ( cnode -> page [ slot_id ] != 0 )
                                rc = KPageFileGet ( pgfile, & val -> page, cnode -> page [ slot_id ] );
                            else
                            {
                                rc = page_access_update ( page, ( void** ) & node );
                                if ( rc == 0 )
                                    rc = KPageFileAlloc ( pgfile, & val -> page, & node -> page [ slot_id ] );
                            }
                            done = true;
                        }

                        /* slots hold node pages */
                        else
                        {
                            /* look for empty node slot */
                            if ( cnode -> page [ slot_id ] == 0 )
                            {
                                /* make current node writable */
                                rc = page_access_update ( page, ( void** ) & node );
                                if ( rc == 0 )
                                {
                                    /* allocate a new child */
                                    KPage *newpage;
                                    rc = KPageFileAlloc ( pgfile, & newpage, & node -> page [ slot_id ] );
                                    if ( rc == 0 )
                                        KPageRelease ( newpage );
                                }
                            }

                            /* retrieve child node page id */
                            page_id = cnode -> page [ slot_id ];

                            /* convert end to offset within child */
                            end %= slot_size;

                            /* convert slot_size to child slot_size */
                            slot_size >>= SNBITS;

                            /* new slot_id within child */
                            slot_id = ( uint32_t ) ( end / slot_size );
                        }
                    }

                    KPageRelease ( page );
                }
            }
        }
    }

    /* finish the value */
    if ( rc == 0 )
    {
        self -> eof = start + size + self -> size_bytes;

        if ( self -> size_bytes != 0 )
        {
            uint8_t *mem;

            /* access the node data for update */
            assert ( val -> page != NULL );
            rc = page_access_update ( val -> page, ( void** ) & mem );

            /* store the size if variable */
            mem += start & ( PGSIZE - 1 );
            switch ( self -> size_bytes )
            {
            case 1:
                mem [ 0 ] = ( uint8_t ) size;
                break;
            case 2:
                ( ( uint16_t* ) mem ) [ 0 ] = ( uint16_t ) size;
                break;
            case 4:
                ( ( uint32_t* ) mem ) [ 0 ] = ( uint32_t ) size;
                break;
            }
        }

        /* record the page offset to DATA and data size */
        val -> offset = ( ( uint32_t ) start + self -> size_bytes ) & ( PGSIZE - 1 );
        val -> size = size;

        return 0;
    }

    * chunk = 0;
    memset ( val, 0, sizeof * val );
    return rc;
}


/* Get
 *  retrieve data from stream
 *
 *  there are N cases to handle within this function
 *
 *  1. chunk not contained within stream
 *  2. root page is data
 *  3. root page is node
 */
static
rc_t KPageFileStreamGet ( const KPageFileStream *self, KPageFile *pgfile, KBTreeValue *valp, uint32_t chunk )
{
    KBTreeValueImpl *val = ( KBTreeValueImpl* ) valp;

    rc_t rc;
    uint64_t start, num_pages;

    assert ( self != NULL );
    assert ( val != NULL );
    assert ( self -> eof == 0 || self -> root != 0 );

    /* convert chunk into byte offset */
    start = ( uint64_t ) chunk * self -> chunk_bytes;
    num_pages = start / ( PGSIZE - self -> page_adj );
    start += num_pages * self -> page_adj;

    /* CASE 1 - chunk is not in stream */
    if ( start >= self -> eof )
        rc = RC ( rcDB, rcFile, rcReading, rcData, rcNotFound );

    /* CASE 2 - root page is data */
    else if ( self -> eof <= PGSIZE )
        rc = KPageFileGet ( pgfile, & val -> page, self -> root );

    /* CASE 3 - root page is node */
    else
    {
        bool done;
        uint32_t pgid;
        uint32_t shift_bits;
        uint64_t slot_id = ( self -> eof - 1 ) >> PGBITS;

        /* determine CURRENT root node status */
        uint64_t slot_size;
        for ( shift_bits = PGBITS, slot_size = PGSIZE; slot_id >= SNSIZE; slot_id >>= SNBITS, shift_bits += SNBITS )
            slot_size <<= SNBITS;

        /* now generate slot_id */
        slot_id = start >> shift_bits;

        for ( rc = 0, pgid = self -> root, done = false; rc == 0 && ! done; )
        {
            KPage *page;
            rc = KPageFileGet ( pgfile, & page, pgid );
            if ( rc == 0 )
            {
                const KStreamNode *cnode;
                rc = page_access_read ( page, ( const void** ) & cnode );
                if ( rc == 0 )
                {
                    assert ( cnode -> page [ slot_id ] != 0 );
                    if ( slot_size <= PGSIZE )
                    {
                        rc = KPageFileGet ( pgfile, & val -> page, cnode -> page [ slot_id ] );
                        done = true;
                    }
                    else
                    {
                        pgid = cnode -> page [ slot_id ];
                        slot_size >>= SNBITS;
                        slot_id = ( start / slot_size ) & ( SNSIZE - 1 );
                    }
                }

                KPageRelease ( page );
            }
        }
    }

    if ( rc == 0 )
    {
        const uint8_t *mem;

        /* access the node data for update */
        assert ( val -> page != NULL );
        rc = page_access_read ( val -> page, ( const void** ) & mem );

        /* store the size if variable */
        mem += start & ( PGSIZE - 1 );
        switch ( self -> size_bytes )
        {
        case 0:
            val -> size = self -> max_size;
            break;
        case 1:
            val -> size = mem [ 0 ];
            break;
        case 2:
            val -> size = ( ( const uint16_t* ) mem ) [ 0 ];
            break;
        case 4:
            val -> size = ( ( const uint32_t* ) mem ) [ 0 ];
            break;
        }

        /* record the page offset to DATA */
        val -> offset = ( ( uint32_t ) start + self -> size_bytes ) & ( PGSIZE - 1 );

        return 0;
    }

    memset ( val, 0, sizeof * val );
    return rc;
}

#endif /* BTREE_KEY2ID */


/*--------------------------------------------------------------------------
 * KBTree
 *  this implementation is an extremely simplified structure
 *  meant to provide the ability to create an index for temporary use
 */
#define eByteOrderTag 0x05031988
#define eByteOrderReverse 0x88190305

#if ! BTREE_KEY2ID

/* v1 stores keys in a stream */
typedef struct KBTreeHdr_v1 KBTreeHdr_v1;
struct KBTreeHdr_v1
{
    /* streams of data */
    KPageFileStream key_stream;
    KPageFileStream val_stream;

    /* only type [ 0 ] is used - rest are for alignment */
    KBTreeKeyType type [ 4 ];

    /* tree root */
    uint32_t root;

    /* next to last */
    uint32_t version;

    /* last */
    uint32_t endian;
};

/* v2 does not have a key stream, but keeps the size */
typedef struct KBTreeHdr_v2 KBTreeHdr_v2;
struct KBTreeHdr_v2
{
    /* data stream */
    KPageFileStream val_stream;

    /* key min/max */
    uint16_t key_min, key_max;

    /* type [ 0 ] is type
       type [ 1 ] is non-zero if comparison function was used
       rest are for alignment */
    KBTreeKeyType type [ 4 ];

    /* tree root */
    uint32_t root;

    /* next to last */
    uint32_t version;

    /* last */
    uint32_t endian;
};

#endif /* ! BTREE_KEY2ID */

/* v3 does not store values, but stores keys in node pages */
typedef struct KBTreeHdr_v3 KBTreeHdr_v3;
struct KBTreeHdr_v3
{
    /* last entry id */
    uint32_t id_seq;

    /* key min/max */
    uint16_t key_min, key_max;

    /* type [ 0 ] is type
       type [ 1 ] is non-zero if comparison function was used
       rest are for alignment */
    KBTreeKeyType type [ 4 ];

    /* tree root */
    uint32_t root;

    /* next to last */
    uint32_t version;

    /* last */
    uint32_t endian;
};

#if BTREE_KEY2ID
typedef struct KBTreeHdr_v3 KBTreeHdr;
#else
typedef struct KBTreeHdr_v2 KBTreeHdr;
#endif


static
rc_t KBTreeReadHeader ( KBTreeHdr *hdr, const KFile *f )
{
    uint64_t eof;
    rc_t rc = KFileSize ( f, & eof );
    if ( rc == 0 )
    {
        size_t num_read;

        /* this would be an empty file */
        if ( eof == 0 )
        {
            memset ( hdr, 0, sizeof * hdr );
            return RC ( rcDB, rcTree, rcConstructing, rcData, rcNotFound );
        }

        if ( eof < sizeof * hdr )
            return RC ( rcDB, rcTree, rcConstructing, rcData, rcCorrupt );

        rc = KFileReadAll ( f, eof - sizeof * hdr, hdr, sizeof * hdr, & num_read );
        if ( rc == 0 && num_read != sizeof * hdr )
            rc = RC ( rcDB, rcTree, rcConstructing, rcData, rcInsufficient );
        if ( rc == 0 )
        {
            if ( hdr -> endian != eByteOrderTag )
            {
                if ( hdr -> endian == eByteOrderReverse )
                    return RC ( rcDB, rcTree, rcConstructing, rcByteOrder, rcIncorrect );
                return RC ( rcDB, rcTree, rcConstructing, rcData, rcCorrupt );
            }
            if ( hdr -> version != 2 )
                return RC ( rcDB, rcTree, rcConstructing, rcHeader, rcBadVersion );
        }
    }
    return rc;
}

struct KBTree
{
    /* file itself */
    KFile *file;

    /* page cache layered on top */
    KPageFile *pgfile;

    /* comparison function */
    KBTreeCompareFunc cmp;

    /* "header" is stored at end */
    KBTreeHdr hdr;

    KRefcount refcount;

    bool read_only;
};

typedef struct KBTreeSrchWindow{
   uint16_t	lower;
   uint16_t	upper;
} KBTreeSrchWindow;


/* the leaf node works out naturally to be an odd count
   which is needed for splitting at the median. so we
   let a leaf fill, then split before further insert */
#define LNSIZE ( ( PGSIZE - 2 ) / 8 )
#define LNMEDIAN ( LNSIZE / 2 )
#define LNHALF ( LNSIZE / 2 )

typedef struct KBTreeLeafNode_v1 KBTreeLeafNode_v1;
struct KBTreeLeafNode_v1
{
    uint32_t key [ LNSIZE ];
    uint32_t val [ LNSIZE ];
    uint16_t count;
};

typedef struct KBTreeLeafEntry_v2 KBTreeLeafEntry_v2;
struct KBTreeLeafEntry_v2
{
    uint16_t key;
    uint16_t ksize;
};

typedef struct KBTreeLeafNode_v2 KBTreeLeafNode_v2;
struct KBTreeLeafNode_v2
{
    uint16_t key_prefix;       /*** prefix offset into the blob ***/
    uint16_t key_prefix_len;   /*** length of the prefix ***/
    KBTreeSrchWindow   win[256]; /*** search windows for first letter of the key ***/
    uint16_t count;	       /*** number of elements **/
    uint16_t key_bytes;        /*** bytes used for storing keys ***/
    KBTreeLeafEntry_v2 ord [ ( PGSIZE - 8 - 256 * sizeof(KBTreeSrchWindow)) / sizeof ( KBTreeLeafEntry_v2 ) ];
};

typedef struct KBTreeLeafEntry_v2 KBTreeLeafEntry;


/* the branch node works out to be an even key count
   which means that we also split before insert when
   full, but the split leaves the target insert side
   light, corrected immediately with an insert */
#define BRSIZE ( ( PGSIZE - 2 - 4 ) / 12 )

typedef struct KBTreeBranchNode_v1 KBTreeBranchNode_v1;
struct KBTreeBranchNode_v1
{
    uint32_t key [ BRSIZE ];
    uint32_t val [ BRSIZE ];
    uint32_t trans [ BRSIZE + 1 ];
    uint16_t count;
};

typedef struct KBTreeBranchEntry_v2 KBTreeBranchEntry_v2;
struct KBTreeBranchEntry_v2
{
    uint16_t key;
    uint16_t ksize;
    uint32_t trans;
};

typedef struct KBTreeBranchNode_v2 KBTreeBranchNode_v2;
struct KBTreeBranchNode_v2
{
    uint16_t key_prefix;       /*** offset into the blob ***/
    uint16_t key_prefix_len;   /*** length of the prefix ***/
    KBTreeSrchWindow   win[256]; /*** search windows for first letter of the key ***/
    /* the exact structure here is important:
       "ltrans" will be accessed as node -> ord [ -1 ] . trans */
    uint16_t count;
    uint16_t key_bytes;
    
    uint32_t ltrans;
    KBTreeBranchEntry_v2 ord [ ( PGSIZE - 12 - 256 * sizeof(KBTreeSrchWindow) ) / sizeof ( KBTreeBranchEntry_v2 ) ];
};

typedef struct KBTreeBranchEntry_v2 KBTreeBranchEntry;

typedef struct KBTreeLeafNode_v2 KBTreeLeafNode;
typedef struct KBTreeBranchNode_v2 KBTreeBranchNode;

/* when keys are stored in pages, the max key size
   will be such that some number of keys are guaranteed to fit */
#define MIN_KEY_COUNT 2
#define MAX_KEY_SIZE \
    (( PGSIZE - 12 - 256 * sizeof(KBTreeSrchWindow)   - \
        MIN_KEY_COUNT * ( sizeof ( KBTreeBranchEntry_v2 ) + sizeof ( uint32_t ) ) \
        ) / MIN_KEY_COUNT )



#if _DEBUGGING 
#define VALIDATE_SEARCH_WINDOW 0
#endif

#if VALIDATE_SEARCH_WINDOW
static
bool validate_search_window(KBTreeSrchWindow *win)
{
	int i;
	if(win[0].lower != 0) return false;
	for(i=0;i<255;i++){
		if(win[i+1].lower != win[i].upper) return false;
	}
	return true;
}
#else
#define validate_search_window(A) true
#endif

/* Whack
 */
static
rc_t KBTreeWhack ( KBTree *self )
{
    if ( self -> read_only || self -> file == NULL )
        KPageFileRelease ( self -> pgfile );
    else
    {
        size_t num_writ;

        /* request page file size */
        uint64_t eof;
        rc_t rc = KPageFileSize ( self -> pgfile, & eof, NULL, NULL );
        if ( rc != 0 )
            return rc;

        /* drop the page file and its cache */
        KPageFileRelease ( self -> pgfile );

        /* write header to tail */        
        rc = KFileWrite ( self -> file, eof, & self -> hdr, sizeof self -> hdr, & num_writ );
        if ( rc == 0 && num_writ != sizeof self -> hdr )
            rc = RC ( rcDB, rcTree, rcPersisting, rcTransfer, rcIncomplete );
        if ( rc == 0 )
            rc = KFileSetSize ( self -> file, eof + sizeof self -> hdr );
        if ( rc != 0 )
        {
            /* TBD - can issue a warning here */
        }
    }

    KFileRelease ( self -> file );
    free ( self );
    return 0;
}


/* MakeRead
 * MakeUpdate
 *  make a b-tree object backed by supplied KFile
 *
 *  "backing" [ IN ] - open file with read permissions.
 *   NB - a reference will be attached to this file.
 *
 *  "climit" [ IN ] - cache limit in bytes. the internal cache will
 *   retain UP TO ( but not exceeding ) the limit specified. a value
 *   of 0 ( zero ) will disable caching.
 *
 *  "cmp" [ IN, NULL OKAY ] - optional comparison callback function for opaque keys.
 *   specific key types will use internal comparison functions. for opaque keys, a
 *   NULL function pointer will cause ordering by size and binary comparison.
 */
LIB_EXPORT rc_t CC KBTreeMakeRead ( const KBTree **btp,
    const KFile *backing, size_t climit, KBTreeCompareFunc cmp )
{
    rc_t rc;

    if ( btp == NULL )
        rc = RC ( rcDB, rcTree, rcConstructing, rcParam, rcNull );
    else
    {
        if ( backing == NULL )
            rc = RC ( rcDB, rcTree, rcConstructing, rcFile, rcNull );
        else
        {
            KBTree *bt = malloc ( sizeof * bt );
            if ( bt == NULL )
                rc = RC ( rcDB, rcTree, rcConstructing, rcMemory, rcExhausted );
            else
            {
                rc = KBTreeReadHeader ( & bt -> hdr, backing );
                if ( rc == 0 )
                {
                    rc = KFileAddRef ( backing );
                    if ( rc == 0 )
                    {
                        /* create page file */
                        rc = KPageFileMakeRead ( ( const KPageFile** ) & bt -> pgfile, backing, climit );
                        if ( rc == 0 )
                        {
                            /* ready to go */
                            bt -> file = ( KFile* ) backing;
                            if ( bt -> hdr . type [ 1 ] )
                                bt -> cmp = cmp;
                            KRefcountInit ( & bt -> refcount, 1, "KBTree", "make-read", "btree" );
                            bt -> read_only = true;

                            * btp = bt;
                            return 0;
                        }

                        KFileRelease ( backing );
                    }
                }

                free ( bt );
            }
        }

        * btp = NULL;
    }

    return rc;
}


/* MakeUpdate
 *  make a b-tree object backed by supplied KFile
 *
 *  "backing" [ IN ] - open file with read & write permissions.
 *   NB - a reference will be attached to this file.
 *
 *  "climit" [ IN ] - cache limit in bytes. the internal cache will
 *   retain UP TO ( but not exceeding ) the limit specified. a value
 *   of 0 ( zero ) will disable caching.
 *
 *  "write_through" [ IN ] - if true, causes flushing of modified page
 *   after its value is released
 *
 *  "type" [ IN ] - describes the key type ( see above )
 *
 *  "key_chunk_size" [ IN ] - the "chunking" ( alignment ) factor for
 *   storing keys, rounded up to the nearest power of 2.
 *
 *  "value_chunk_size" [ IN ] - chunking factor for values
 *   ( see "key_chunk_size" )
 *
 *  "min_key_size" [ IN ] and "max_key_size" [ IN ] - specifies the allowed
 *   opaque key sizes. min == max implies fixed size. ignored for well
 *   known fixed size key types.
 *
 *  "min_value_size" [ IN ] and "max_value_size" [ IN ] - specifies the allowed
 *   value sizes. min == max implies fixed size.
 *
 *  "cmp" [ IN ] - comparison callback function for opaque keys.
 */
#if BTREE_KEY2ID
LIB_EXPORT rc_t CC KBTreeMakeUpdate ( KBTree **btp, KFile *backing,
    size_t climit, bool write_through, KBTreeKeyType type,
    size_t min_key_size, size_t max_key_size, size_t id_size,
    KBTreeCompareFunc cmp )
#else
LIB_EXPORT rc_t CC KBTreeMakeUpdate ( KBTree **btp, KFile *backing,
    size_t climit, bool write_through, KBTreeKeyType type,
    size_t key_chunk_size, size_t value_chunk_size,
    size_t min_key_size, size_t max_key_size,
    size_t min_value_size, size_t max_value_size,
    KBTreeCompareFunc cmp )
#endif
{
    rc_t rc;

    if ( btp == NULL )
        rc = RC ( rcDB, rcTree, rcConstructing, rcParam, rcNull );
    else
    {
        if ( type >= kbtLastDefined )
            rc = RC ( rcDB, rcTree, rcConstructing, rcType, rcUnrecognized );
        else if ( min_key_size == 0 )
            rc = RC ( rcDB, rcTree, rcConstructing, rcParam, rcInvalid );
        else if ( max_key_size > MAX_KEY_SIZE )
            rc = RC ( rcDB, rcTree, rcConstructing, rcParam, rcExcessive );
        else if ( min_key_size > max_key_size )
            rc = RC ( rcDB, rcTree, rcConstructing, rcParam, rcInconsistent );
#if BTREE_KEY2ID && ! BTREE_KEY2ID64
        else if ( id_size != sizeof ( uint32_t ) )
            rc = RC ( rcDB, rcTree, rcConstructing, rcParam, rcInvalid );
#endif
        else
        {
            KBTree *bt = calloc ( 1,sizeof * bt );
            if ( bt == NULL )
                rc = RC ( rcDB, rcTree, rcConstructing, rcMemory, rcExhausted );
            else
            {
                if ( backing == NULL || ( rc = KBTreeReadHeader ( & bt -> hdr, backing )) == 0 || GetRCState ( rc ) == rcNotFound )
                {
                    /* detect empty file */
                    if ( bt -> hdr . version == 0 )
                    {
#if BTREE_KEY2ID
                        assert ( bt -> hdr . id_seq == 0 );
#else
                        KPageFileStreamInit ( & bt -> hdr . val_stream, min_value_size, max_value_size, value_chunk_size );
#endif
                        bt -> hdr . type [ 0 ] = type;
                        bt -> hdr . type [ 1 ] = cmp != NULL;
                        bt -> hdr . key_min = ( uint16_t ) min_key_size;
                        bt -> hdr . key_max = ( uint16_t ) max_key_size;
#if BTREE_KEY2ID
                        bt -> hdr . version = 3;
#else
                        bt -> hdr . version = 2;
#endif
                        bt -> hdr . endian = eByteOrderTag;
                        rc = 0;
                    }
                    else
                    {
                        /* check for parameter equivalence */
#if BTREE_KEY2ID
                        if ( bt -> hdr . version < 3 )
#else
                        if ( bt -> hdr . version < 2 )
#endif
                            rc = RC ( rcDB, rcTree, rcConstructing, rcHeader, rcBadVersion );
                        else if ( bt -> hdr . key_min > min_key_size || bt -> hdr . key_max < max_key_size )
                            rc = RC ( rcDB, rcTree, rcConstructing, rcParam, rcInconsistent );
                        else
#if ! BTREE_KEY2ID
                            rc = KPageFileStreamCheck ( & bt -> hdr . val_stream, min_value_size, max_value_size, value_chunk_size );
#endif
                        if ( rc == 0 && bt -> hdr . type [ 0 ] != type )
                            rc = RC ( rcDB, rcTree, rcConstructing, rcType, rcInconsistent );
                    }

                    if ( rc == 0 )
                    {
                        if(backing) rc = KFileAddRef ( backing );
                        if ( rc == 0 )
                        {
                            /* create page file */
                            rc = KPageFileMakeUpdate ( & bt -> pgfile, backing, climit, write_through );
                            if ( rc == 0 )
                            {
                                /* ready to go */
                                bt -> file = backing;
                                if ( ! bt -> hdr . type [ 1 ] )
                                    bt -> cmp = NULL;
                                else
                                    bt -> cmp = cmp;
                                KRefcountInit ( & bt -> refcount, 1, "KBTree", "make-update", "btree" );
                                bt -> read_only = false;

                                * btp = bt;
                                return 0;
                            }

                            if(backing) KFileRelease ( backing );
                        }
                    }
                }

                free ( bt );
            }
        }

        * btp = NULL;
    }

    return rc;
}


/* AddRef
 * Release
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC KBTreeAddRef ( const KBTree *self )
{
    if ( self != NULL ) switch ( KRefcountAdd ( & self -> refcount, "KBTree" ) )
    {
    case krefOkay:
        break;
    default:
        return RC ( rcDB, rcTree, rcAttaching, rcConstraint, rcViolated );
    }
    return 0;
}

LIB_EXPORT rc_t CC KBTreeRelease ( const KBTree *self )
{
    if ( self != NULL ) switch ( KRefcountDrop ( & self -> refcount, "KBTree" ) )
    {
    case krefOkay:
        break;
    case krefWhack:
        return KBTreeWhack ( ( KBTree* ) self );
    default:
        return RC ( rcDB, rcTree, rcReleasing, rcConstraint, rcViolated );
    }
    return 0;
}


/* DropBacking
 *  used immediately prior to releasing
 *  prevents modified pages from being flushed to disk
 *  renders object nearly useless
 */
LIB_EXPORT rc_t CC KBTreeDropBacking ( KBTree *self )
{
    rc_t rc;

    if ( self == NULL )
        return RC ( rcDB, rcTree, rcDetaching, rcSelf, rcNull );

    rc = KPageFileDropBacking ( self -> pgfile );
    if ( rc == 0 )
    {
        rc = KFileRelease ( self -> file );
        if ( rc == 0 )
            self -> file = NULL;
    }

    return rc;
}


/* Size
 *  returns size in bytes of file and cache
 *
 *  "lsize" [ OUT, NULL OKAY ] - return parameter for logical size
 *
 *  "fsize" [ OUT, NULL OKAY ] - return parameter for file size
 *
 *  "csize" [ OUT, NULL OKAY ] - return parameter for cache size
 */
LIB_EXPORT rc_t CC KBTreeSize ( const KBTree *self,
    uint64_t *lsize, uint64_t *fsize, size_t *csize )
{
    size_t dummysz;
    uint64_t dummy64;

    if ( self != NULL )
        return KPageFileSize ( self -> pgfile, lsize, fsize, csize );

    if ( lsize == NULL )
        lsize = & dummy64;
    if ( fsize == NULL )
        fsize = & dummy64;
    if ( csize == NULL )
        csize = & dummysz;

    * lsize = 0;
    * fsize = 0;
    * csize = 0;

    return RC ( rcDB, rcTree, rcAccessing, rcSelf, rcNull );
}


/* Find
 *  searches for a match
 *
 *  "val" [ OUT ] - return parameter for value found
 *   accessed via KBTreeValueAccess* described above
 *   must be balanced with a call to KBTreeValueWhack.
 *
 *  "key" [ IN ] and "key_size" [ IN ] - describes an
 *   opaque key
 */

static __inline__
int compare_keys ( const KBTree *self, const void *query, size_t qsize, const void *key, size_t key_size )
{
    if ( self -> cmp != NULL )
        return ( * self -> cmp ) ( query, qsize, key, key_size );
    else {
        size_t csize = (qsize < key_size) ? qsize : key_size;
        int diff = memcmp ( query, key, csize);
        if (diff == 0)
            return (int)qsize - (int)key_size;
        return diff;
    }
}

#if BTREE_KEY2ID
static
rc_t leaf_find ( const KBTree *self, const KPage *page,
    uint32_t *id, const void *query, size_t qsize )
#else
static
rc_t leaf_find ( const KBTree *self, const KPage *page,
    KBTreeValue *val, const void *query, size_t qsize )
#endif
{
    int lower, upper;
    const uint8_t *query_8 = query;
    uint16_t	q;

    const KBTreeLeafNode *cnode;
    rc_t rc = page_access_read ( page, ( const void** ) & cnode );
    if ( rc != 0 )
        return rc;
    if(cnode->key_prefix_len > 0){
    	const size_t key_prefix_len=cnode->key_prefix_len;
 	/*** prefix must match ***/	
        assert(compare_keys(self,query,key_prefix_len, ((char *)cnode )+cnode->key_prefix,key_prefix_len) == 0);
	/*************************/
	query_8 += key_prefix_len;
	qsize   -= key_prefix_len;
    }

    /* perform search on branch node */
    q = (qsize > 0)?*query_8:0;
    for ( lower = cnode->win[q].lower, upper = cnode -> win[q].upper; lower < upper; )
    {
        /* determine the slot to examine */
        int slot = ( lower + upper ) >> 1;

        /* perform comparison */
        const uint8_t *key = & ( ( const uint8_t* ) cnode ) [ cnode -> ord [ slot ] . key ];
        int diff = compare_keys ( self, query_8, qsize, key , cnode -> ord [ slot ] . ksize);
        if ( diff == 0 )
        {
            uint32_t val_id = * ( const uint32_t* ) & key [ cnode -> ord [ slot ] . ksize ];
#if BTREE_KEY2ID
            * id = val_id;
            return 0;
#else
            return KPageFileStreamGet ( & self -> hdr . val_stream, self -> pgfile, val, val_id );
#endif
        }
        if ( diff < 0 )
            upper = slot;
        else
            lower = slot + 1;
    }

    return RC ( rcDB, rcTree, rcSelecting, rcItem, rcNotFound );
}

#if BTREE_KEY2ID
static
rc_t branch_find ( const KBTree *self, const KPage *page,
    uint32_t *id, const void *query, size_t qsize )
#else
static
rc_t branch_find ( const KBTree *self, const KPage *page,
    KBTreeValue *val, const void *query, size_t qsize )
#endif
{
    int lower, upper;
    const uint8_t *query_8 = query;
    size_t  qsize_8 = qsize;
    uint16_t q;
    KPage *child;
    uint32_t nid;

    const KBTreeBranchNode *cnode;
    rc_t rc = page_access_read ( page, ( const void** ) & cnode );
    if ( rc != 0 )
        return rc;
    if(cnode->key_prefix_len > 0){
        const size_t key_prefix_len=cnode->key_prefix_len;
 	/*** prefix must match ***/	
        assert(compare_keys(self,query,key_prefix_len, ((char *)cnode )+cnode->key_prefix,key_prefix_len) == 0);
	/*************************/
	query_8 += key_prefix_len;
        qsize_8 -= key_prefix_len;
    }
    /* perform search on branch node */
    q = (qsize_8 > 0)?*query_8:0;
    for ( lower = cnode->win[q].lower, upper = cnode -> win[q].upper; lower < upper; )
    {
        /* determine the slot to examine */
        int slot = ( lower + upper ) >> 1;

        /* perform comparison */
        const uint8_t *key = & ( ( const uint8_t* ) cnode ) [ cnode -> ord [ slot ] . key ];
        int diff = compare_keys ( self, query_8, qsize_8, key , cnode -> ord [ slot ] . ksize );
        if ( diff == 0 )
        {
            uint32_t val_id = * ( const uint32_t* ) & key [ cnode -> ord [ slot ] . ksize ];
#if BTREE_KEY2ID
            * id = val_id;
            return 0;
#else
            return KPageFileStreamGet ( & self -> hdr . val_stream, self -> pgfile, val, val_id );
#endif
        }
        if ( diff < 0 )
            upper = slot;
        else
            lower = slot + 1;
    }
    /* should have the last slot tried ( < 0 ) or next slot to try ( > 0 ) */
    assert ( lower == upper );

    /* the node id is left-shifted by 1 and has the "branch-bit" indicator
       in the LSB. the remaining bits should NOT be zero */
    /* NB - if "upper" is 0 and type is signed,
       this will access entry -1, giving "ltrans" */
    nid = cnode -> ord [ upper - 1 ] . trans;
    assert ( ( nid >> 1 ) != 0 );

    /* access child node */
    rc = KPageFileGet ( self -> pgfile, & child, nid >> 1 );
    if ( rc == 0 )
    {
        rc = ( ( ( nid & 1 ) == 0 ) ? leaf_find : branch_find )
#if BTREE_KEY2ID
            ( self, child, id, query, qsize );
#else
            ( self, child, val, query, qsize );
#endif

        KPageRelease ( child );
    }

    return rc;
}


#if BTREE_KEY2ID
LIB_EXPORT rc_t CC KBTreeFind ( const KBTree *self, uint64_t *id,
    const void *key, size_t key_size )
#else
LIB_EXPORT rc_t CC KBTreeFind ( const KBTree *self, KBTreeValue *val,
    const void *key, size_t key_size )
#endif
{
    rc_t rc;

#if BTREE_KEY2ID
    if ( id == NULL )
#else
    if ( val == NULL )
#endif
        rc = RC ( rcDB, rcTree, rcSelecting, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcTree, rcSelecting, rcSelf, rcNull );
        else if ( key_size == 0 )
            rc = RC ( rcDB, rcTree, rcSelecting, rcParam, rcEmpty );
        else if ( key == NULL )
            rc = RC ( rcDB, rcTree, rcSelecting, rcParam, rcNull );
        else if ( self -> hdr . root == 0 )
            rc = RC ( rcDB, rcTree, rcSelecting, rcItem, rcNotFound );
        else
        {
            /* bring in root page */
            KPage *page;
            rc = KPageFileGet ( self -> pgfile, & page, self -> hdr . root >> 1 );
            if ( rc == 0 )
            {
#if BTREE_KEY2ID && ! BTREE_KEY2ID64
                uint32_t id32;
#endif
                rc = ( ( ( self -> hdr . root & 1 ) == 0 ) ? leaf_find : branch_find )
#if BTREE_KEY2ID64
                    ( self, page, id, key, key_size );
#elif BTREE_KEY2ID
                    ( self, page, & id32, key, key_size );
#else
                    ( self, page, val, key, key_size );
#endif
#if BTREE_KEY2ID && ! BTREE_KEY2ID64
                    * id = id32;
#endif
                
                KPageRelease ( page );
            }
        }

        if ( rc != 0 )
#if BTREE_KEY2ID
            * id = 0;
#else
            memset ( val, 0, sizeof * val );
#endif
    }

    return rc;
}


/* Entry
 *  searches for a match or creates a new entry
 *
 *  "val" [ OUT ] - return parameter for value found
 *   accessed via KBTreeValueAccess* described above
 *   must be balanced with a call to KBTreeValueWhack.
 *
 *  "was_inserted" [ OUT ] - if true, the returned value was the result of an
 *   insertion and can be guaranteed to be all 0 bits. otherwise, the returned
 *   value will be whatever was there previously.
 *
 *  "alloc_size" [ IN ] - the number of value bytes to allocate upon insertion,
 *   i.e. if the key was not found. this value must agree with the limits
 *   specified in Make ( see above ).
 *
 *  "key" [ IN ] and "key_size" [ IN ] - describes an
 *   opaque key
 */
typedef struct KBTreeSplit_v1 KBTreeSplit_v1;
struct KBTreeSplit_v1
{
    uint32_t left;
    uint32_t right;
    uint32_t key;
    uint32_t val;
};

typedef struct KBTreeSplit_v2 KBTreeSplit_v2;
struct KBTreeSplit_v2
{
    void *key;
    uint32_t left;
    uint32_t right;
    uint16_t ksize;
    uint8_t buff [ 256 ];
};

typedef struct KBTreeSplit_v2 KBTreeSplit;
#define KBTreeSplitInit( split ) \
    ( void ) ( ( split ) -> key = NULL )
#define KBTreeSplitWhack( split ) \
    ( ( split ) -> key != NULL && ( split ) -> key != ( void* ) ( split ) -> buff ) ? \
    free ( ( split ) -> key ) : ( void ) 0

typedef struct KBTreeEntryData KBTreeEntryData;
struct KBTreeEntryData
{
    KBTree *self;
#if BTREE_KEY2ID
    uint32_t *id;
#else
    KBTreeValue *val;
    size_t alloc_size;
#endif
    const void *key;
    size_t key_size;
    bool was_inserted;
};


#if _DEBUGGING
#include <stdio.h>

#if 0
static
void dump_leaf_page ( const KBTreeLeafNode *cnode )
{
    int i;
    const char *page;

    if ( cnode == NULL )
    {
        printf ( "NULL\n\n" );
        return;
    }

    printf ( "KBTreeLeafNode\n"
             "  count:     %u\n"
             "  key bytes: %u\n"
             , cnode -> count
             , cnode -> key_bytes
        );

    if ( cnode -> count == 0 )
    {
        putchar ( '\n' );
        return;
    }

    page = ( const void* ) cnode;

    printf ( "  entries:\n" );
    for ( i = 0; i < cnode -> count; ++ i )
    {
        int key_size = cnode -> ord [ i ] . ksize;
        const char *key = & page [ cnode -> ord [ i ] . key ];
        uint32_t val_id = * ( const uint32_t* ) & key [ key_size ];
        printf ( "  % 5d. key = '%.*s', offset = %u, value-id = %u\n"
                 , i
                 , key_size, key
                 , cnode -> ord [ i ] . key
                 , val_id
            );
    }

    putchar ( '\n' );
}
#endif

#if _HUGE_CROWBAR_KLUDGE
static
void validate_key_order ( const KBTree *self, const KBTreeEntryData *pb, const KBTreeLeafNode *cnode )
{
    if ( cnode -> count != 0 )
    {
        KBTreeValue left, right;
        rc_t rc = KPageFileStreamGet ( & self -> hdr . key_stream, self -> pgfile, & left, cnode -> key [ 0 ] );
        if ( rc == 0 )
        {
            uint32_t i;
            size_t sleft, sright;
            const void *mleft, *mright;
            rc = KBTreeValueAccessRead ( & left, & mleft, & sleft );

            for ( i = 1; rc == 0 && i < cnode -> count; ++ i )
            {
                rc = KPageFileStreamGet ( & self -> hdr . key_stream, self -> pgfile, & right, cnode -> key [ i ] );
                if ( rc == 0 )
                {
                    rc = KBTreeValueAccessRead ( & right, & mright, & sright );
                    if ( rc == 0 )
                    {
                        int diff = ( * self -> cmp ) ( mleft, sleft, mright, sright );
                        if ( diff > 0 )
                        {
                            fprintf ( stderr, "AAAAAHHHH!!!!\n" );
                            rc = -1;
                        }
                    }

                    KBTreeValueWhack ( & left );
                    left = right;
                }
            }

            KBTreeValueWhack ( & left );
        }
    }
}
#endif /* _HUGE_CROWBAR_KLUDGE */
#endif /* _DEBUGGING */

static
rc_t make_entry ( KBTree *self, KBTreeEntryData *pb, void *hdrp, void *ordp )
{
#if BTREE_KEY2ID
    rc_t rc=0;
    /* use id provided from outside */
#if 0 /* automatically generate */
    if ( ( * pb -> id = ++ self -> hdr . id_seq ) == 0 )
    {
        -- self -> hdr . id_seq;
        rc = RC ( rcDB, rcTree, rcInserting, rcId, rcExhausted );
    }
    else
#endif
#else
    /* insert value */
    uint32_t val_id;
    rc_t rc = KPageFileStreamAlloc ( & self -> hdr . val_stream, self -> pgfile,
            pb -> val, pb -> alloc_size, & val_id );
    if ( rc == 0 )
#endif
    {
        uint8_t *page = hdrp;
        KBTreeLeafNode *hdr = hdrp;
        KBTreeLeafEntry *ord = ordp;
        const uint8_t *key = pb -> key;
        uint16_t key_size = (uint16_t) ( pb -> key_size - hdr->key_prefix_len );


        assert(memcmp(key,page + hdr->key_prefix, hdr -> key_prefix_len)==0);/*** validate in debug mode **/ 
        key += hdr->key_prefix_len;

        /* grab memory for key and value-id */
        hdr -> key_bytes += key_size + sizeof ( uint32_t );
        assert ( hdr -> key_bytes < PGSIZE );

        /* location of key within page */
        ord -> key = PGSIZE - hdr -> key_bytes;
        ord -> ksize = ( uint16_t ) key_size;

        /* insert key */
        memcpy ( page + ord -> key, key,  key_size );

        /* record value id */
#if BTREE_KEY2ID
        memcpy ( & page [ ord -> key + key_size ], pb -> id, sizeof * pb -> id );
#else
        memcpy ( page  +  ord -> key + key_size, & val_id, sizeof val_id );
#endif

        /* mark inserted */
        pb -> was_inserted = true;
    }

    return rc;
}

static
bool leaf_node_full ( const KBTreeLeafNode *node, size_t key_size )
{
    /*** add existing keys ***/
    size_t size = node -> key_bytes;
    /*** add new key ***/

    assert( key_size >= node -> key_prefix_len);
    size += key_size + sizeof ( uint32_t ) - node -> key_prefix_len;
    /*** add key index **/
    size += (uint8_t*)(node->ord + node -> count+1)  - (uint8_t*)node ;

    if ( size > PGSIZE )
        return true;
    return false;
}

static
rc_t leaf_insert ( KBTree *self, KBTreeEntryData *pb, KBTreeLeafNode *node, uint32_t slot )
{
    rc_t rc;
    uint16_t q;

    /* check that key will fit */
    if ( leaf_node_full ( node, pb->key_size ) ) {
        return RC ( rcDB, rcTree, rcInserting, rcConstraint, rcViolated );
    }

    /* open hole */
    if ( slot != node -> count )
        memmove ( & node -> ord [ slot + 1 ], & node -> ord [ slot ], sizeof node -> ord [ 0 ] * ( node -> count - slot ) );

    /* enter into the leaf */
    rc = make_entry ( self, pb, node, & node -> ord [ slot ] );
    if ( rc == 0 )
        ++ node -> count;

    /* recover from error */
    else if ( slot != node -> count )
        memmove ( & node -> ord [ slot ], & node -> ord [ slot + 1 ], sizeof node -> ord [ 0 ] * ( node -> count - slot ) );

    /* correct search windows */
    q=(pb->key_size > node->key_prefix_len)?((uint8_t*)pb->key)[node->key_prefix_len]:0;
    /*** unused windows should have been maintained properly ****/
    assert((node->win[q].upper==node->win[q].lower)?(node->win[q].lower==slot):true);
    node->win[q].upper ++; 
    for(q=q+1; q < 256; q++){
	node->win[q].lower++;
	node->win[q].upper++;
    }
    assert(validate_search_window(node->win));
    return rc;
}

static
int CC KBTreeLeafEntry_sort_desc_by_offset ( const void *a, const void *b, void *data )
{
    const KBTreeLeafEntry *ord = ( const void* ) data;
    int left = * ( const uint16_t* ) a;
    int right = * ( const uint16_t* ) b;
    return ord [ right ] . key - ord [ left ] . key;
}

static
rc_t split_leaf ( KBTree *self, KBTreeEntryData *pb,
    KBTreeLeafNode *left, KBTreeLeafNode *right, uint32_t slot, KBTreeSplit *split)
{
    size_t off, ksize;
    bool hoist_existing;
    uint16_t ord [ ( sizeof left -> ord / sizeof left -> ord [ 0 ] + 1 ) / 2 ];

    /* calculate median */
    int32_t i, j, median = ( left -> count + 1 ) >> 1;

    /* pointers to pages */
    uint8_t * lpage = ( void* ) left;
    uint8_t * rpage = ( void* ) right;

    /* check to see if key being inserted would be hoisted */
    if ( slot == median && median > ( left -> count >> 1 ) )
    {
        hoist_existing = false;
        ksize = pb -> key_size;
    }
    else
    {
        /* TBD - choose median based upon count and slot */
        /* get median key size */
        hoist_existing = true;
        ksize = left -> ord [ median ] . ksize + left -> key_prefix_len;
    }

    /* allocate key space */
    split -> key = split -> buff;
    if ( ksize > ( sizeof split -> buff - sizeof ( uint32_t ) ) )
    {
        split -> key = malloc ( ksize + sizeof ( uint32_t ) );
        if ( split -> key == NULL )
            return RC ( rcDB, rcTree, rcInserting, rcMemory, rcExhausted );
    }

    /* copy out key plus value id */
    if ( hoist_existing ){
	split -> ksize = 0;
	if(left -> key_prefix_len > 0){
		memcpy ( ((uint8_t*)split -> key) + split -> ksize, lpage + left -> key_prefix, left -> key_prefix_len );
		split -> ksize +=left -> key_prefix_len;
	}
        memcpy(((uint8_t*)split -> key) + split -> ksize, lpage + left -> ord [ median ] . key , left -> ord [ median ] . ksize + sizeof ( uint32_t ) );
    	split -> ksize += left -> ord [ median ] . ksize;
    }

    /* right page is empty */
    right -> key_bytes = 0;

    /* copy all of the keys to the right of median from left to right */
    for ( i = 0, j = median + hoist_existing; j < left -> count; ++ i, ++ j )
    {
        ksize = left -> ord [ j ] . ksize + sizeof ( uint32_t );
        right -> ord [ i ] . ksize = left -> ord [ j ] . ksize;
        right -> key_bytes += (uint16_t) ksize;
        right -> ord [ i ] . key = ( uint16_t ) ( PGSIZE - right -> key_bytes );
        memcpy (rpage + right -> ord [ i ] . key, lpage + left -> ord [ j ] . key, ksize );
	if(i == 0 && left->key_prefix_len > 0){
		right -> key_prefix_len = left -> key_prefix_len;
		right -> key_bytes     += left -> key_prefix_len;
		right -> key_prefix     = PGSIZE - right -> key_bytes;
		memcpy ( rpage + right -> key_prefix, lpage + left->key_prefix, left -> key_prefix_len );
	}
    }

    /* each node has half its former count */
    right -> count = ( uint16_t ) i;
    left -> count = median;

    /* compact the keys */
    for ( i = 0; i < median; ++ i )
        ord [ i ] = i;
    ksort ( ord, median, sizeof ord [ 0 ], KBTreeLeafEntry_sort_desc_by_offset, left -> ord );
    for ( left -> key_bytes = 0, i = 0; i < median; ++ i )
    {
        j = ord [ i ];
        ksize = left -> ord [ j ] . ksize + sizeof ( uint32_t );
        off = PGSIZE - ( left -> key_bytes += (uint16_t) ksize );
        if ( left -> ord [ j ] . key != off )
        {
            memmove ( & lpage [ off ], & lpage [ left -> ord [ j ] . key ], ksize );
            left -> ord [ j ] . key = ( uint16_t ) off;
        }
	if(i == 0 && left->key_prefix_len > 0){ /** need to restore prefix from the right ***/
		left -> key_bytes += left -> key_prefix_len;
		left -> key_prefix = PGSIZE - left -> key_bytes;
		memcpy ( lpage + left -> key_prefix, rpage + right -> key_prefix, left -> key_prefix_len );
	}
    }
    /*** maintain search windows ****/
    /*** find the last window of left ***/
    for(i=0;left->win[i].upper < left->count; i++){
	assert(i<256);
	right->win[i].lower=0;
	right->win[i].upper=0;
    }
    j=right->ord[0].ksize?((uint8_t*)right)[right->ord[0].key]:0;
    assert(i<=j);
    for(;i<j;i++){
	right->win[i].lower=0;
        right->win[i].upper=0;
        left->win[i].upper=left->count;
	left->win[i+1].lower=left->count;
    }
    /*** set first window of right ***/
    right->win[i].lower = 0;
    assert( left->win[i].upper >= left->count + hoist_existing);
    right->win[i].upper = left->win[i].upper - left->count - hoist_existing;
    /*** set last window of left ***/ 
    left->win[i].upper = left->count;
    i++;
    /*** set right windows from left, reset left ***/
    for(;i<256;i++){
	assert( left->win[i].lower >= left->count + hoist_existing );
	assert( left->win[i].upper >= left->count + hoist_existing );
	right->win[i].lower = left->win[i].lower - left->count  - hoist_existing;
	right->win[i].upper = left->win[i].upper - left->count  - hoist_existing;
	left->win[i].lower=left->count;
	left->win[i].upper=left->count;
    }
    assert(right->win[255].upper == right -> count);
    /*** reset right window ***/
    for(;i<256;i++){
	right->win[i].lower=right->count;
        right->win[i].upper=right->count;
    }
    assert(validate_search_window(right->win));
    assert(validate_search_window(left->win));
    /*****************************************/

    /* if insert would be hoisted, do it directly */
    if ( ! hoist_existing )
    {
#if BTREE_KEY2ID
#if 0 /*** use external id ***/
        if ( ( * pb -> id = ++ self -> hdr . id_seq ) == 0 )
        {
            -- self -> hdr . id_seq;
            return RC ( rcDB, rcTree, rcInserting, rcId, rcExhausted );
        }
#endif
        memcpy ( split -> key, pb -> key , pb -> key_size );
        memcpy ( & ( ( uint8_t* ) split -> key ) [ pb -> key_size ], pb -> id, sizeof * pb -> id );
        split -> ksize =  (uint16_t) pb -> key_size;
        return 0;
#else
        /* insert value */
        uint32_t val_id;
        rc_t rc = KPageFileStreamAlloc ( & self -> hdr . val_stream, self -> pgfile,
            pb -> val, pb -> alloc_size, & val_id );
        if ( rc == 0 )
        {
            memcpy ( split -> key, pb->key, pb->key_size);
            * ( uint32_t* ) & ( ( uint8_t* ) split -> key ) [ pb->key_size ] = val_id;
	    split -> ksize =  pb -> key_size;
        }
        return rc;
#endif
    }

    /* TBD - with keys in pages, there is no guarantee that the following insert
       will succeed, unless there would be a provision for overflow. */

    /* decide where to insert entry */
    if ( slot <= (uint32_t) median )
        return leaf_insert ( self, pb, left, slot );
    return leaf_insert ( self, pb, right, slot - median - 1 );
}



static
rc_t leaf_entry ( KBTree *self, KBTreeEntryData *pb, KPage *page, KBTreeSplit *split)
{
    int lower, upper;
    KBTreeLeafNode *node;

    const uint8_t *query = pb -> key;
    size_t qsize = pb -> key_size;
    uint16_t q;

    /* access page for read */
    const KBTreeLeafNode *cnode;
    rc_t rc = page_access_read ( page, ( const void** ) & cnode );
    if ( rc != 0 )
        return rc;

    /* special case for root node initial insert */
    if ( cnode -> count == 0 )
    {
        rc = page_access_update ( page, ( void** ) & node );
        if ( rc == 0 )
        {
            rc = make_entry ( self, pb, node, & node -> ord [ 0 ] );
            if ( rc == 0 ){
		int i;
	    	q=(qsize >0)? *query:0;
                node -> count = 1;
		node -> win[q].lower = 0;
		node -> win[q].upper = 1;
		for(i=0;i<q;i++){
			node -> win[i].lower = 0;
			node -> win[i].upper = 0;
		}
		for(i=q+1;i<256;i++){
                        node -> win[i].lower = 1;
                        node -> win[i].upper = 1;
                }
    		assert(validate_search_window(node->win));

	    }
        }
        return rc;
    }

    /* perform search on branch node */
    /* start with prefix compare */
    if(cnode->key_prefix_len > 0){
        const size_t key_prefix_len=cnode->key_prefix_len;
 	/*** prefix must match ***/	
        assert(compare_keys(self,query,key_prefix_len, ((char *)cnode )+cnode->key_prefix,key_prefix_len) == 0);
	/*************************/
	query += cnode->key_prefix_len;
	qsize -= cnode->key_prefix_len;
	
    }
    q=(qsize >0)? *query:0;
    for ( lower = cnode->win[q].lower, upper = cnode -> win[q].upper; lower < upper; )
    {
        /* determine the slot to examine */
        int slot = ( lower + upper ) >> 1;

        /* perform comparison */
        const uint8_t *key = & ( ( const uint8_t* ) cnode ) [ cnode -> ord [ slot ] . key ];
        int diff = compare_keys ( self, query, qsize, key, cnode -> ord [ slot ] . ksize );
        if ( diff == 0 )
        {
            uint32_t val_id;
            memcpy ( & val_id, & key [ cnode -> ord [ slot ] . ksize ], sizeof val_id );
#if BTREE_KEY2ID
            * pb -> id = val_id;
            return 0;
#else
            return KPageFileStreamGet ( & self -> hdr . val_stream, self -> pgfile, pb -> val, val_id );
#endif
        }
        if ( diff < 0 )
            upper = slot;
        else
            lower = slot + 1;
    }

    /* should have the last slot tried ( < 0 ) or next slot to try ( > 0 ) */
    assert ( lower == upper );

    /* going to need to update the node */
    rc = page_access_update ( page, ( void** ) & node );
    if ( rc != 0 )
        return rc;

    /* unless an error occurs */
    pb -> was_inserted = true;

    /* test for split */
    if ( leaf_node_full ( node, pb->key_size ) )
    {
        KPage *dup;

        /* create another leaf node to right */
        rc = KPageFileAlloc ( self -> pgfile, & dup, & split -> right );
        if ( rc == 0 )
        {
            void *mem;
            rc = page_access_update ( dup, & mem );
            if ( rc == 0 )
            {
                rc = split_leaf ( self, pb, node, mem, upper, split);
                if ( rc == 0 )
                {
                    rc = RC ( rcDB, rcTree, rcInserting, rcId, rcInsufficient );
                    split -> right <<= 1;
                }
            }

            KPageRelease ( dup );
        }
    }
    else
    {
        /* simple insert */
        rc = leaf_insert ( self, pb, node, upper );

#if _DEBUGGING && _HUGE_CROWBAR_KLUDGE
        validate_key_order ( self, pb, node );
#endif
    }

    return rc;
}


#if _DEBUGGING
#if 0
static
void dump_branch_page ( const KBTreeBranchNode *cnode )
{
    int i;
    const char *page;

    if ( cnode == NULL )
    {
        printf ( "NULL\n\n" );
        return;
    }

    printf ( "KBTreeBranchNode\n"
             "  count:     %u\n"
             "  key bytes: %u\n"
             , cnode -> count
             , cnode -> key_bytes
        );

    if ( cnode -> count == 0 )
    {
        putchar ( '\n' );
        return;
    }

    page = ( const void* ) cnode;

    printf ( "  entries:\n" );
    for ( i = 0; i < cnode -> count; ++ i )
    {
        int key_size = cnode -> ord [ i ] . ksize;
        const char *key = & page [ cnode -> ord [ i ] . key ];
        uint32_t val_id = * ( const uint32_t* ) & key [ key_size ];
        printf ( "  % 5d. key = '%.*s', offset = %u, value-id = %u\n"
                 , i
                 , key_size, key
                 , cnode -> ord [ i ] . key
                 , val_id
            );
    }

    printf ( "  transitions:\n" );
    for ( i = 0; i <= cnode -> count; ++ i )
    {
        uint32_t node_id = cnode -> ord [ i - 1 ] . trans;
        printf ( "  % 5d. node-id = %u\n"
                 , i
                 , node_id
            );
    }

    putchar ( '\n' );
}
#endif
#endif

static
bool branch_node_full ( const KBTreeBranchNode *node, size_t key_size )
{
    /*** add existing keys ***/
    size_t size = node -> key_bytes;
    /*** add new key ***/
    assert( key_size >= node -> key_prefix_len);
    size += key_size + sizeof ( uint32_t ) - node -> key_prefix_len;

    /*** add key index **/
    size += (uint8_t*)(node->ord + node -> count+1)  - (uint8_t*)node ;
    if ( size > PGSIZE )
        return true;
    return false;
}

static
rc_t branch_insert ( KBTree *self, KBTreeBranchNode *node, const KBTreeSplit *split, uint32_t slot )
{
    uint16_t ksize = split -> ksize;
    uint8_t *key   = split -> key;
    uint8_t *page = (uint8_t*) node;
    uint16_t q;

    /* remove prefix from the key */
    ksize -= node -> key_prefix_len;

    assert(memcmp(key,page + node->key_prefix, node -> key_prefix_len)==0);/*** validate in debug mode **/ 
    key   += node -> key_prefix_len;

    /* check that key will fit */
    if ( branch_node_full ( node, split -> ksize ) )
        return RC ( rcDB, rcTree, rcInserting, rcConstraint, rcViolated );

    /* open hole */
    if ( slot != node -> count )
        memmove ( & node -> ord [ slot + 1 ], & node -> ord [ slot ], sizeof node -> ord [ 0 ] * ( node -> count - slot ) );

    /* enter key value pair */
    node -> key_bytes += ksize + sizeof ( uint32_t );
    node -> ord [ slot ] . ksize = ksize;
    node -> ord [ slot ] . key = ( uint16_t ) ( PGSIZE - node -> key_bytes );
    memcpy ( & ( ( uint8_t* ) node ) [ PGSIZE - node -> key_bytes ], key, ksize + sizeof ( uint32_t ) );

    /* enter the new transitions */
    assert ( node -> ord [ ( int ) slot - 1 ] . trans == split -> left );
    node -> ord [ ( int ) slot - 1 ] . trans = split -> left;
    node -> ord [ slot ] . trans = split -> right;
#if 0
    printf("branch_insert: count:%d prefix_len:%d slot:%d left:%d right:%d \n",node->count, node->key_prefix_len, slot,split -> left,split -> right);
#endif

    ++ node -> count;
    
    /* correct search windows */
    q=(split->ksize > node->key_prefix_len)?((uint8_t*)split->key)[node->key_prefix_len]:0;
    /*** unused windows should have beed maintained properly ****/
    assert((node->win[q].upper==node->win[q].lower)?(node->win[q].lower==slot):true);
    node->win[q].upper ++;
    for(q=q+1; q < 256; q++){
        node->win[q].lower++;
        node->win[q].upper++;
    }
    assert(validate_search_window(node->win));
    return 0;
}

static
int CC KBTreeBranchEntry_sort_desc_by_offset ( const void *a, const void *b, void *data )
{
    const KBTreeBranchEntry *ord = ( const void* ) data;
    int left = * ( const uint16_t* ) a;
    int right = * ( const uint16_t* ) b;
    return ord [ right ] . key - ord [ left ] . key;
}

static
rc_t split_branch ( KBTree *self,
    KBTreeBranchNode *left, KBTreeBranchNode *right,
    const KBTreeSplit *val, KBTreeSplit *split, uint32_t slot)
{
    size_t off, ksize;
    bool hoist_existing;
    uint16_t ord [ ( sizeof left -> ord / sizeof left -> ord [ 0 ] + 1 ) / 2 ];

    /* calculate median */
    int32_t i, j, median = ( left -> count + 1 ) >> 1;

    /* pointers to pages */
    uint8_t * lpage = ( void* ) left;
    uint8_t * rpage = ( void* ) right;

    /* check to see if key being inserted would be hoisted */
    if ( slot == median && median > ( left -> count >> 1 ) )
    {
        hoist_existing = false;
        ksize = val -> ksize;
    }
    else
    {
        /* get median key size */
        hoist_existing = true;
        ksize = left -> ord [ median ] . ksize + left -> key_prefix_len;
    }

    /* allocate key space */
    split -> key = split -> buff;
    if ( ksize > ( sizeof split -> buff - sizeof ( uint32_t ) ) )
    {
        split -> key = malloc ( ksize + sizeof ( uint32_t ) );
        if ( split -> key == NULL )
            return RC ( rcDB, rcTree, rcInserting, rcMemory, rcExhausted );
    }

    /* copy out key plus value id */
    if ( hoist_existing ){
        split -> ksize = 0;
        if(left -> key_prefix_len > 0){
                memcpy ( ((uint8_t*)split -> key) + split -> ksize, lpage + left -> key_prefix, left -> key_prefix_len );
                split -> ksize +=left -> key_prefix_len;
        }
        memcpy(((uint8_t*)split -> key) + split -> ksize, lpage + left -> ord [ median ] . key , left -> ord [ median ] . ksize + sizeof ( uint32_t ) );
        split -> ksize +=  left -> ord [ median ] . ksize;
    }


    /* right page is empty */
    right -> key_bytes = 0;

    /* copy all of the keys to the right of the median from left to right */
    for ( i = 0, j = median + hoist_existing; j < left -> count; ++ i, ++ j )
    {
        ksize = left -> ord [ j ] . ksize + sizeof ( uint32_t );
        right -> ord [ i ] . ksize = left -> ord [ j ] . ksize;
        right -> key_bytes += (uint16_t) ksize;
        right -> ord [ i ] . key = ( uint16_t ) ( PGSIZE - right -> key_bytes );
        memcpy ( & rpage [ PGSIZE - right -> key_bytes ], & lpage [ left -> ord [ j ] . key ], ksize );
        right -> ord [ i - 1 ] . trans = left -> ord [ j - 1 ] . trans;
        if(i == 0 && left->key_prefix_len > 0){
                off = PGSIZE - right -> key_bytes - left -> key_prefix_len;
                memcpy ( & rpage [ off ], lpage + left -> key_prefix, left -> key_prefix_len );
                right -> key_bytes += left -> key_prefix_len;
                right -> key_prefix_len = left -> key_prefix_len;
                right -> key_prefix = (uint16_t) off;
        }
    }

    /* copy the last trans */
    right -> ord [ i - 1 ] . trans = left -> ord [ j - 1 ] . trans;

    /* each node has half its former count */
    right -> count = ( uint16_t ) i;
    left -> count = median;

    /* compact the keys */
    for ( i = 0; i < median; ++ i )
        ord [ i ] = i;
    ksort ( ord, median, sizeof ord [ 0 ], KBTreeBranchEntry_sort_desc_by_offset, left -> ord );
    for ( left -> key_bytes = 0, i = 0; i < median; ++ i )
    {
        j = ord [ i ];
        ksize = left -> ord [ j ] . ksize + sizeof ( uint32_t );
        off = PGSIZE - ( left -> key_bytes += (uint16_t) ksize );
        if ( left -> ord [ j ] . key != off )
        {
            memmove ( & lpage [ off ], & lpage [ left -> ord [ j ] . key ], ksize );
            left -> ord [ j ] . key = ( uint16_t ) off;
        }
	if(i == 0 && left->key_prefix_len > 0){ /** need to restore prefix from the right ***/
                off = PGSIZE - left -> key_bytes - left -> key_prefix_len;
                memcpy ( & lpage [ off ], rpage + right -> key_prefix, left -> key_prefix_len );
                left -> key_bytes += left -> key_prefix_len;
                left -> key_prefix = (uint16_t) off;
        }
    }
    /*** maintain search windows ****/
    /*** find the last window of left ***/
    for(i=0;left->win[i].upper < left->count; i++){
	assert(i<256);
	right->win[i].lower=0;
	right->win[i].upper=0;
    }
    j=right->ord[0].ksize?((uint8_t*)right)[right->ord[0].key]:0;
    assert(i<=j);
    for(;i<j;i++){
	right->win[i].lower=0;
        right->win[i].upper=0;
        left->win[i].upper=left->count;
	left->win[i+1].lower=left->count;
    }
    /*** set first window of right ***/
    right->win[i].lower = 0;
    assert( left->win[i].upper >= left->count + hoist_existing);
    right->win[i].upper = left->win[i].upper - left->count - hoist_existing;
    /*** set last window of left ***/ 
    left->win[i].upper = left->count;
    i++;
    /*** set right windows from left, reset left ***/
    for(;i<256;i++){
	assert( left->win[i].lower >= left->count + hoist_existing );
	assert( left->win[i].upper >= left->count + hoist_existing );
	right->win[i].lower = left->win[i].lower - left->count  - hoist_existing;
	right->win[i].upper = left->win[i].upper - left->count  - hoist_existing;
	left->win[i].lower=left->count;
	left->win[i].upper=left->count;
    }
    assert(right->win[255].upper == right -> count);
    /*** reset right window ***/
    for(;i<256;i++){
	right->win[i].lower=right->count;
        right->win[i].upper=right->count;
    }
    assert(validate_search_window(right->win));
    assert(validate_search_window(left->win));
    /*****************************************/

    /* if insert would be hoisted, do it directly */
    if ( ! hoist_existing )
    {
        /* copy key and value */
        memcpy ( split -> key, val -> key, val -> ksize + sizeof ( uint32_t ) );
	split ->  ksize = val -> ksize;

        /* set left and right transitions */
        assert ( left -> ord [ median - 1 ] . trans == val -> left );
        left -> ord [ median - 1 ] . trans = val -> left;
        right -> ltrans = val -> right;

        return 0;
    }

    /* decide where to insert entry */
    if ( slot <= (uint32_t) median )
        return branch_insert ( self, left, val, slot );
    return branch_insert ( self, right, val, slot - median - 1 );
}
#define MIN_PREFIX_TO_COMPACT 1 /*** minimum prefix delta to initiate compacting ****/
static
rc_t leaf_compact (KBTree *self, KPage *pg,uint16_t prefix_len)
{
    const KBTreeLeafNode *cnode;
    rc_t rc = page_access_read(pg, ( const void** ) & cnode );
    if(rc == 0  && cnode->key_prefix_len < prefix_len) {
	assert(cnode->count   > 0); 
	if( cnode->count   > 0 && prefix_len > cnode->key_prefix_len ){
	    prefix_len -= cnode->key_prefix_len; /*** need only to compact the delta **/
	    assert(memcmp(((uint8_t*)cnode)+cnode->ord[0].key,
		          ((uint8_t*)cnode)+cnode->ord[cnode->count-1].key,
			  prefix_len )==0);/****** To be sure ****/
            if( prefix_len >= MIN_PREFIX_TO_COMPACT){ /*** good to compact ***/
		KBTreeLeafNode *node;
		rc = page_access_update ( pg, ( void** ) & node );
		if(rc == 0){
			uint8_t  q,last;
			uint16_t i;
			uint16_t ord [ ( sizeof node -> ord / sizeof node -> ord [ 0 ] + 1 ) / 2 ];
			for ( i = 0; i < node->count; ++ i ) ord[i]=i; /*** build and sort indexing array ***/
			ksort ( ord, node->count, sizeof ord [ 0 ], KBTreeLeafEntry_sort_desc_by_offset, node -> ord );
			/*** deal with prefix and index 0 ***/
			/*** prefix is cut from the last stored key on the page **/
			if(node->key_prefix_len == 0){
				node->key_prefix = node->ord[ord[0]].key;
			}
			node->key_prefix_len    += prefix_len;
			node->ord[ord[0]].key   += prefix_len;
			node->ord[ord[0]].ksize -= prefix_len;
			/*** deal with index 1 (shift) ***/
			if(node->count > 1){
				node->ord[ord[1]].key   += prefix_len;
				node->ord[ord[1]].ksize -= prefix_len;
			}
			/*** cut all other values ***/
			for ( i = 2; i < node->count; ++ i ){
				uint16_t j = ord[i];
				uint8_t *src = ((uint8_t*)node) + node->ord[j].key + prefix_len;
				uint8_t *dst = src + (i-1)*prefix_len; /*** proportional offset ***/
				uint16_t ksize = node->ord[j].ksize + sizeof ( uint32_t ) - prefix_len;
				memmove(dst,src,ksize);
				node->ord[j].ksize -= prefix_len;
				node->ord[j].key    = (uint16_t) ( dst -  ((uint8_t*)node) );
			}
			assert( node -> key_bytes > (node->count-1) * prefix_len);
			node -> key_bytes -= (node->count-1) * prefix_len;
			/*** rebuild search windows ***/
			q=node->ord[0].ksize?((uint8_t*)node)[node->ord[0].key]:0;
			for(i=0;i<q;i++){
				node->win[i].lower=0;
				node->win[i].upper=0;
			}
			node->win[q].lower=0;
			for(i=0,last=q;i< node->count; ++ i ){
				q=node->ord[i].ksize?((uint8_t*)node)[node->ord[i].key]:0;
				assert(last<=q);
				if(q!=last){
					node->win[last++].upper = i;
					for(;last<q;last++){/*** need to catch up ***/
						node->win[last].lower=i;
						node->win[last].upper=i;
					}
					node->win[q].lower=i;
					last=q;
				}
			}
			node->win[last].upper=node->count;
			for(i=last+1;i<256;i++){
				node->win[i].lower=node->count;
				node->win[i].upper=node->count;
			}
			assert(validate_search_window(node->win));
		}	
	    }
	}
    }
    return rc;
}
static
rc_t branch_compact (KBTree *self, KPage *pg,uint16_t prefix_len)
{
    const KBTreeBranchNode *cnode;
    rc_t rc = page_access_read(pg, ( const void** ) & cnode );
    if(rc == 0 && cnode->key_prefix_len < prefix_len) {
	assert(cnode->count   > 0); 
	if( cnode->count   > 0 && prefix_len > cnode->key_prefix_len ){
	    prefix_len -= cnode->key_prefix_len; /*** need only to compact the delta **/
	    assert(memcmp(((uint8_t*)cnode)+cnode->ord[0].key,
		          ((uint8_t*)cnode)+cnode->ord[cnode->count-1].key,
			  prefix_len )==0);/****** To be sure ****/
            if( prefix_len >= MIN_PREFIX_TO_COMPACT){ /*** good to compact ***/
		KBTreeBranchNode *node;
		rc = page_access_update ( pg, ( void** ) & node );
		if(rc == 0){
			uint8_t	 q,last;
			uint16_t i;
			uint16_t ord [ ( sizeof node -> ord / sizeof node -> ord [ 0 ] + 1 ) / 2 ];
			for ( i = 0; i < node->count; ++ i ) ord[i]=i; /*** build and sort indexing array ***/
			ksort ( ord, node->count, sizeof ord [ 0 ], KBTreeBranchEntry_sort_desc_by_offset, node -> ord );
			/*** deal with prefix and index 0 ***/
			/*** prefix is cut from the last stored key on the page **/
			if(node->key_prefix_len == 0){
				node->key_prefix = node->ord[ord[0]].key;
			}
			node->key_prefix_len    += prefix_len;
			node->ord[ord[0]].key   += prefix_len;
			node->ord[ord[0]].ksize -= prefix_len;
			/*** deal with index 1 (shift) ***/
			if(node->count > 1){
				node->ord[ord[1]].key   += prefix_len;
				node->ord[ord[1]].ksize -= prefix_len;
			}
			/*** cut all other values ***/
			for ( i = 2; i < node->count; ++ i ){
				uint16_t j = ord[i];
				uint8_t *src = ((uint8_t*)node) + node->ord[j].key + prefix_len;
				uint8_t *dst = src + (i-1)*prefix_len; /*** proportional offset ***/
				uint16_t ksize = node->ord[j].ksize + sizeof ( uint32_t ) - prefix_len;
				memmove(dst,src,ksize);
				node->ord[j].ksize -= prefix_len;
				node->ord[j].key    = (uint16_t) ( dst -  ((uint8_t*)node) );
			}
			assert( node -> key_bytes > (node->count-1) * prefix_len);
			node -> key_bytes -= (node->count-1) * prefix_len;
			/*** rebuild search windows ***/
			 q=node->ord[0].ksize?((uint8_t*)node)[node->ord[0].key]:0;
                        for(i=0;i<q;i++){
                                node->win[i].lower=0;
                                node->win[i].upper=0;
                        }
                        node->win[q].lower=0;
                        for(i=0,last=q;i< node->count; ++ i ){
                                q=node->ord[i].ksize?((uint8_t*)node)[node->ord[i].key]:0;
                                assert(last<=q);
                                if(q!=last){
                                        node->win[last++].upper = i;
                                        for(;last<q;last++){/*** need to catch up ***/
                                                node->win[last].lower=i;
                                                node->win[last].upper=i;
                                        }
                                        node->win[q].lower=i;
                                        last=q;
                                }
                        }
                        node->win[last].upper=node->count;
                        for(i=last+1;i<256;i++){
                                node->win[i].lower=node->count;
                                node->win[i].upper=node->count;
                        }
                        assert(validate_search_window(node->win));
		}	
	    }
	}
    }
    return rc;
}

static
rc_t compact_page ( KBTree *self, uint32_t nid, uint16_t prefix_len)
{
    KPage *pg;
    rc_t rc = KPageFileGet ( self -> pgfile, & pg, nid >> 1 );
    if ( rc == 0 ){
	rc = ( ( nid & 1 ) == 0 ) ?
            leaf_compact ( self, pg, prefix_len):
            branch_compact ( self, pg, prefix_len);
	KPageRelease(pg);
    }
    return rc;
}


static
rc_t branch_entry ( KBTree *self, KBTreeEntryData *pb, KPage *page, KBTreeSplit *rsplit)
{
    KPage *child;
    uint32_t nid;
    int lower, upper;
    const uint8_t *query = pb -> key;
    size_t qsize = pb -> key_size;
    uint16_t q;

    /* look at node in read-only mode */
    const KBTreeBranchNode *cnode;
    rc_t rc = page_access_read ( page, ( const void** ) & cnode );
    if ( rc != 0 )
        return rc;

    /* perform search on branch node */
    /* start with prefix compare */
    if(cnode->key_prefix_len > 0){
        const size_t key_prefix_len=cnode->key_prefix_len;
 	/*** prefix must match ***/	
        assert(compare_keys(self,query,key_prefix_len, ((char *)cnode )+cnode->key_prefix,key_prefix_len) == 0);
	/*************************/
	query += cnode->key_prefix_len;
	qsize -= cnode->key_prefix_len;
    }
    q = (qsize>0)?*query:0;

    for ( lower = cnode->win[q].lower, upper = cnode -> win[q].upper; lower < upper; )
    {
        /* determine the slot to examine */
        int slot = ( lower + upper ) >> 1;

        /* perform comparison */
        const uint8_t *key = & ( ( const uint8_t* ) cnode ) [ cnode -> ord [ slot ] . key ];
        int diff = compare_keys ( self, query, qsize, key, cnode -> ord [ slot ] . ksize );
        if ( diff == 0 )
        {
            uint32_t val_id = * ( const uint32_t* ) & key [ cnode -> ord [ slot ] . ksize ];
#if BTREE_KEY2ID
            * pb -> id = val_id;
            return 0;
#else
            return KPageFileStreamGet ( & self -> hdr . val_stream, self -> pgfile, pb -> val, val_id );
#endif
        }
        if ( diff < 0 )
            upper = slot;
        else
            lower = slot + 1;
    }

    /* should have the last slot tried ( < 0 ) or next slot to try ( > 0 ) */
    assert ( lower == upper );

    /* the node id is left-shifted by 1 and has the "branch-bit" indicator
       in the LSB. the remaining bits should NOT be zero */
    nid = cnode -> ord [ upper - 1 ] . trans;
    assert ( ( nid >> 1 ) != 0 );

    /* access child node */
    rc = KPageFileGet ( self -> pgfile, & child, nid >> 1 );
    if ( rc == 0 )
    {
        KBTreeSplit split;
        KBTreeSplitInit ( & split );
        /* recursively continue search */
        rc = ( ( nid & 1 ) == 0 ) ?
            leaf_entry ( self, pb, child, & split):
            branch_entry ( self, pb, child, & split);
        if ( rc != 0 )
        {
            /* detect split */
            if ( GetRCState ( rc ) == rcInsufficient && GetRCObject ( rc ) == rcId )
            {
		/* splitting may replace value being inserted in to the branch ***/
                /* access current node */
                KBTreeBranchNode *node;
                rc = page_access_update ( page, ( void** ) & node );
                if ( rc == 0 )
                {
                    split . left = nid;

                    /* if we are also full, we have to split */
                    if ( branch_node_full ( node, split.ksize ) )
                    {
                        KPage *dup;
                        /* create another branch node to right */
                        rc = KPageFileAlloc ( self -> pgfile, & dup, & rsplit -> right );
                        if ( rc == 0 )
                        {
                            void *mem;
                            rc = page_access_update ( dup, & mem );
                            if ( rc == 0 )
                            {
                                split_branch( self, node, mem, & split, rsplit, upper);
                                rc = RC ( rcDB, rcTree, rcInserting, rcId, rcInsufficient );
                                rsplit -> right += rsplit -> right + 1;
                            }

                            KPageRelease ( dup );
                        }
                    }
                    else
                    {
			int pl;/**common prefix_len **/
                        /* hoist into location */
			/* it will increase node->count and upper will start pointing into a new key ***/
			if(rc==0 && upper > 0 ){ /*** left side compact is possible  upper is 1 based ***/
				uint8_t   *a = (uint8_t*)split.key;
				uint8_t   *b = (uint8_t*)node;

				a += node->key_prefix_len;
				b += node->ord[upper-1].key;
				assert ( node->ord[upper-1].trans == split.left );
				for(pl=0;pl < split.ksize - node->key_prefix_len && pl < node->ord[upper-1].ksize && a[pl]==b[pl];pl++){}
				pl+=node->key_prefix_len;
                                if(pl >= MIN_PREFIX_TO_COMPACT){
					rc=compact_page(self,split.left,pl);
				}
			}
			if(rc==0 && upper < node->count ){ /*** right side compact is possible ***/
				uint8_t   *a = (uint8_t*)split.key;
				uint8_t   *b = (uint8_t*)node;
				a += node->key_prefix_len;
				b += node->ord[upper].key;
				for(pl=0;pl < split.ksize - node->key_prefix_len && pl < node->ord[upper].ksize && a[pl]==b[pl];pl++){}
				pl+=node->key_prefix_len;
                                if(pl >= MIN_PREFIX_TO_COMPACT){
					rc=compact_page(self,split.right,pl);
                                }
			} 
			if(rc == 0 ){
				rc=branch_insert ( self, node, & split, upper );
				assert(rc==0);
			}
                    }
                }
            }
        }

        KBTreeSplitWhack ( & split );

        KPageRelease ( child );
    }

    return rc;
}


#if BTREE_KEY2ID
LIB_EXPORT rc_t CC KBTreeEntry ( KBTree *self, uint64_t *id,
    bool *was_inserted, const void *key, size_t key_size )
#else
LIB_EXPORT rc_t CC KBTreeEntry ( KBTree *self,
    KBTreeValue *val, bool *was_inserted, size_t alloc_size,
    const void *key, size_t key_size )
#endif
{
    rc_t rc;

    bool dummy;
    if ( was_inserted == NULL )
        was_inserted = & dummy;

#if BTREE_KEY2ID
    if ( id == NULL )
#else
    if ( val == NULL )
#endif
        rc = RC ( rcDB, rcTree, rcUpdating, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcTree, rcUpdating, rcSelf, rcNull );
        else if ( key_size == 0 )
            rc = RC ( rcDB, rcTree, rcUpdating, rcParam, rcEmpty );
        else if ( key == NULL )
            rc = RC ( rcDB, rcTree, rcUpdating, rcParam, rcNull );
        else if ( key_size < self -> hdr . key_min )
            rc = RC ( rcDB, rcTree, rcUpdating, rcParam, rcInsufficient );
        else if ( key_size > self -> hdr . key_max )
            rc = RC ( rcDB, rcTree, rcUpdating, rcParam, rcExcessive );
        else
        {
            bool leaf;
            KPage *page;

#if BTREE_KEY2ID && ! BTREE_KEY2ID64
            /* TEMPORARY
               until full 64-bit ids are supported
            */
            uint32_t id32 = ( uint32_t ) * id;
#endif

            /* package the entry params into a block */
            KBTreeEntryData pb;
            pb . self = self;
#if BTREE_KEY2ID64
            pb . id = id;
#elif BTREE_KEY2ID
            pb . id = & id32;
#else
            pb . val = val;
            pb . alloc_size = alloc_size;
#endif
            pb . key = key;
            pb . key_size = key_size;
            pb . was_inserted = false;

            /* get root node */
            if ( self -> hdr . root == 0 )
            {
                /* allocate initial node */
                leaf = true;
                rc = KPageFileAlloc ( self -> pgfile, & page, & self -> hdr . root );
                self -> hdr . root <<= 1;
            }
            else
            {
                /* fetch existing */
                leaf = ( self -> hdr . root & 1 ) == 0;
                rc = KPageFileGet ( self -> pgfile, & page, self -> hdr . root >> 1 );
            }
            if ( rc == 0 )
            {
                KBTreeSplit split;
                KBTreeSplitInit ( & split );

                /* perform recursive operation */
		/* top page is always unbounded */
                rc = leaf ?
                    leaf_entry ( self, & pb, page, & split ):
                    branch_entry ( self, & pb, page, & split );

#if BTREE_KEY2ID && ! BTREE_KEY2ID64
                * id = id32;
#endif
                if ( rc != 0 )
                {
                    /* detect split */
                    if ( GetRCState ( rc ) == rcInsufficient && GetRCObject ( rc ) == rcId )
                    {
                        KPage *new_root;

                        /* record old root */
                        split . left = self -> hdr . root;
                        rc = KPageFileAlloc ( self -> pgfile, & new_root, & self -> hdr . root );
                        if ( rc == 0 )
                        {
                            KBTreeBranchNode *node;
                            rc = page_access_update ( new_root, ( void** ) & node );
                            if ( rc == 0 )
                            {
				uint16_t	q,i;
                                /* install the new root */
                                node -> key_bytes = split . ksize + sizeof ( uint32_t );
                                node -> key_prefix_len = 0;
                                node -> key_prefix = 0;
                                node -> ord [ 0 ] . ksize = split . ksize;
                                node -> ord [ 0 ] . key = ( uint16_t ) ( PGSIZE - node -> key_bytes );
                                memcpy ( & ( ( uint8_t* ) node ) [ PGSIZE - node -> key_bytes ], split . key, node -> key_bytes );
                                node -> ltrans = split . left;
                                node -> ord [ 0 ] . trans = split . right;
                                node -> count = 1;
/* 				assert ( split . left < 10000 && split . right < 10000); */
                                /*** init search windows ***/
                                q=(split.ksize>0)? *(uint8_t*)split.key:0;
				node -> win[q].lower = 0;
				node -> win[q].upper = 1;
				for(i=0;i<q;i++){
					node -> win[i].lower = 0;
					node -> win[i].upper = 0;
				}
				for(i=q+1;i<256;i++){
					node -> win[i].lower = 1;
					node -> win[i].upper = 1;
				}
				assert(validate_search_window(node->win));

                                /* mark as branch node */
                                self -> hdr . root += self -> hdr . root + 1;
                            }

                            KPageRelease ( new_root );
                        }

                        if ( rc != 0 )
                            self -> hdr . root = split . left;
                    }
                }

                KPageRelease ( page );

                KBTreeSplitWhack ( & split );

                if ( rc == 0 )
                {
                    * was_inserted = pb . was_inserted;
                    return 0;
                }
            }
        }

#if BTREE_KEY2ID
        * id = 0;
#else
        memset ( val, 0, sizeof * val );
#endif
    }

    * was_inserted = false;
    return rc;
}


/* ForEach
 *  executes a function on each tree element
 *
 *  "reverse" [ IN ] - if true, iterate in reverse order
 *
 *  "f" [ IN ] and "data" [ IN, OPAQUE ] - callback function
 */
#if BTREE_KEY2ID
static
rc_t invoke_foreach_func ( const KBTree *self, const void *cnode, const void *ordp,
    void ( CC * f ) ( const void *key, size_t key_size, uint64_t id, void *data ), void *data )
#else
static
rc_t invoke_foreach_func ( const KBTree *self, const void *cnode, const void *ordp,
    void ( CC * f ) ( const void *key, size_t key_size, KBTreeValue *val, void *data ), void *data )
#endif
{
#if ! BTREE_KEY2ID
    rc_t rc;
    KBTreeValue val;
#endif

    const KBTreeLeafEntry *ord = ordp;
    const uint8_t *page = cnode;
    const uint8_t *key = & page [ ord -> key ];
    size_t key_size = ord -> ksize;

    uint32_t val_id;
    memcpy ( & val_id, & key [ key_size ], sizeof val_id );

#if BTREE_KEY2ID
    ( * f ) ( key, key_size, val_id, data );
    return 0;
#else
    rc = KPageFileStreamGet ( & self -> hdr . val_stream, self -> pgfile, & val, val_id );
    if ( rc == 0 )
    {
        ( * f ) ( key, key_size, & val, data );
        KBTreeValueWhack ( & val );
    }
    return rc;
#endif
}

#if BTREE_KEY2ID
static
rc_t leaf_foreach ( const KBTree *self, bool reverse, uint64_t nid,
    void ( CC * f ) ( const void *key, size_t key_size, uint64_t id, void *data ), void *data )
#else
static
rc_t leaf_foreach ( const KBTree *self, bool reverse, uint32_t nid,
    void ( CC * f ) ( const void *key, size_t key_size, KBTreeValue *val, void *data ), void *data )
#endif
{
    rc_t rc;
    KPage *page;

    if ( nid == 0 )
        return 0;

    rc = KPageFileGet ( self -> pgfile, & page, (uint32_t) ( nid >> 1 ) );
    if ( rc == 0 )
    {
        const KBTreeLeafNode *cnode;
        rc = page_access_read ( page, ( const void** ) & cnode );
        if ( rc == 0 )
        {
            uint32_t i;
            if ( reverse ) for ( i = cnode -> count; rc == 0 && i > 0; )
            {
                rc = invoke_foreach_func ( self, cnode, & cnode -> ord [ i ], f, data );
            }
            else for ( i = 0; rc == 0 && i < cnode -> count; ++ i )
            {
                rc = invoke_foreach_func ( self, cnode, & cnode -> ord [ i ], f, data );
            }
        }

        KPageRelease ( page );
    }
    return rc;
}

#if BTREE_KEY2ID
static
rc_t branch_foreach ( const KBTree *self, bool reverse, uint64_t nid,
    void ( CC * f ) ( const void *key, size_t key_size, uint64_t id, void *data ), void *data )
#else
static
rc_t branch_foreach ( const KBTree *self, bool reverse, uint32_t nid,
    void ( CC * f ) ( const void *key, size_t key_size, KBTreeValue *val, void *data ), void *data )
#endif
{
    KPage *page;
    rc_t rc = KPageFileGet ( self -> pgfile, & page, (uint32_t) ( nid >> 1 ) );
    if ( rc == 0 )
    {
        const KBTreeBranchNode *cnode;
        rc = page_access_read ( page, ( const void** ) & cnode );
        if ( rc == 0 )
        {
            uint32_t i, child;
            if ( reverse )
            {
                for ( i = cnode -> count; rc == 0 && i > 0; )
                {
                    child = cnode -> ord [ -- i ] . trans;
                    rc = ( ( child & 1 ) == 0 ) ?
                        leaf_foreach ( self, reverse, child, f, data ):
                        branch_foreach ( self, reverse, child, f, data );

                    if ( rc == 0 )
                        rc = invoke_foreach_func ( self, cnode, & cnode -> ord [ i ], f, data );
                }
                if ( rc == 0 && cnode -> count != 0 )
                {
                    child = cnode -> ltrans;
                    rc = ( ( child & 1 ) == 0 ) ?
                        leaf_foreach ( self, reverse, child, f, data ):
                        branch_foreach ( self, reverse, child, f, data );
                }
            }
            else
            {
                child = cnode -> ltrans;
                rc = ( ( child & 1 ) == 0 ) ?
                    leaf_foreach ( self, reverse, child, f, data ):
                    branch_foreach ( self, reverse, child, f, data );

                for ( i = 0; rc == 0 && i < cnode -> count; ++ i )
                {
                    rc = invoke_foreach_func ( self, cnode, & cnode -> ord [ i ], f, data );
                    if ( rc == 0 )
                    {
                        child = cnode -> ord [ i ] . trans;
                        rc = ( ( child & 1 ) == 0 ) ?
                            leaf_foreach ( self, reverse, child, f, data ):
                            branch_foreach ( self, reverse, child, f, data );
                    }
                }
            }
        }

        KPageRelease ( page );
    }
    return rc;
}

#if BTREE_KEY2ID
LIB_EXPORT rc_t CC KBTreeForEach ( const KBTree *self, bool reverse,
    void ( CC * f ) ( const void *key, size_t key_size, uint64_t id, void *data ), void *data )
#else
LIB_EXPORT rc_t CC KBTreeForEach ( const KBTree *self, bool reverse,
    void ( CC * f ) ( const void *key, size_t key_size, KBTreeValue *val, void *data ), void *data )
#endif
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcDB, rcTree, rcVisiting, rcSelf, rcNull );
    else if ( f == NULL )
        rc = RC ( rcDB, rcTree, rcVisiting, rcFunction, rcNull );
    else
    {
        rc = ( ( self -> hdr . root & 1 ) == 0 ) ?
            leaf_foreach ( self, reverse, self -> hdr . root, f, data ):
            branch_foreach ( self, reverse, self -> hdr . root, f, data );
    }

    return rc;
}
