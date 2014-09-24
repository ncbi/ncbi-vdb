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

#include <kfs/extern.h>
#include <kfs/pmem.h>
#include <kfs/pagefile.h>
#include <klib/refcount.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * KMemBlock
 * KExtMemBlock
 *  bad for random access, but reasonable for the application
 *  blocks are stored as linked lists
 */
typedef struct KMemBlock KMemBlock;
struct KMemBlock
{
    uint64_t next;
    uint64_t size;
    uint8_t data [ 48 ];
};

typedef struct KExtMemBlock KExtMemBlock;
struct KExtMemBlock
{
    uint64_t next;
    uint8_t data [ 56 ];
};

#define MEMBLOCK_DATASIZE( block, block_size ) \
    ( ( block_size ) - sizeof * ( block ) + sizeof ( block ) -> data )

#define MEMBLOCK_RDATA( block, block_size, offset ) \
    & ( block ) -> data [ MEMBLOCK_DATASIZE ( block, block_size ) - ( offset ) ]

#define eByteOrderTag 0x05031988
#define eByteOrderReverse 0x88190305

typedef struct KHdrMemBlock KHdrMemBlock;
struct KHdrMemBlock
{
    /* standard header */
    uint32_t endian;
    uint32_t version;
    union
    {
        /* v1 layout */
        struct
        {
            /* number of bytes allocated */
            uint64_t used;

            /* head of free list */
            uint64_t free_list;

            /* size of each block: <= page size */
            uint32_t block_size;

            /* free-list fragentation information */
            uint16_t free_count;
            uint16_t frag_count;

        } v1;

    } u;
};


/*--------------------------------------------------------------------------
 * KMemPage
 *  local cache for page during access
 *  remembers KPage and the pgid to avoid having to call to get it
 *  allows for accessing by block
 */
typedef struct KMemPage KMemPage;
struct KMemPage
{
    KPage *pg;
    uint32_t pgid;
};

static
rc_t KMemPageWhack ( KMemPage *self )
{
    rc_t rc;

    if ( self == NULL )
        return 0;

    rc = KPageRelease ( self -> pg );
    if ( rc == 0 )
        memset ( self, 0, sizeof * self );

    return rc;
}

static
rc_t KMemPageInit ( KMemPage *mp, KPage *pg0 )
{
    rc_t rc = KPageAddRef ( pg0 );
    if ( rc != 0 )
    {
        memset ( mp, 0, sizeof * mp );
        return rc;
    }

    mp -> pg = pg0;
    mp -> pgid = 1;
    return 0;
}

static
rc_t KMemPageAlloc ( KMemPage *self, KPageFile *pf )
{
    rc_t rc = KPageRelease ( self -> pg );
    if ( rc == 0 )
    {
        rc = KPageFileAlloc ( pf, & self -> pg, & self -> pgid );
        if ( rc != 0 )
            memset ( self, 0, sizeof * self );
    }
    return rc;
}

static
rc_t KMemPageGet ( KMemPage *self, KPageFile *pf, uint32_t pgid )
{
    rc_t rc = KPageRelease ( self -> pg );
    if ( rc == 0 )
    {
        rc = KPageFileGet ( pf, & self -> pg, self -> pgid = pgid );
        if ( rc != 0 )
            memset ( self, 0, sizeof * self );
    }
    return rc;
}


/*--------------------------------------------------------------------------
 * KMemBank
 *  manages simple blocks of memory
 *  optionally backed by a persistent page file
 *
 *  stores accounting information in block 0
 *
 *  accounting information includes block size and linkage
 *
 *  block links are maintained in a table
 */
struct KMemBank
{
    uint64_t limit;

    KPageFile *pf;

    KPage *pg0;
    KHdrMemBlock *hdr;

    /* the difference between page bits and block bits
       used to go between page id and block id */
    uint32_t pb_bits;

    /* for converting a block id to byte offset */
    uint32_t blk_bits;

    /* for masking off block index within page */
    uint32_t blk_mask;

    KRefcount refcount;
    bool read_only;
};


/* Whack
 */
static
rc_t KMemBankWhack ( KMemBank *self )
{
    rc_t rc = KPageRelease ( self -> pg0 );
    if ( rc != 0 )
        return rc;

    rc = KPageFileDropBacking ( self -> pf );
    /* TBD - can report errors */

    rc  = KPageFileRelease ( self -> pf );
    /* TBD - can report errors */

    free ( self );

    return 0;
}


/* Make
 *  make a memory bank with a fixed block size
 *  the total amount of memory may be limited
 *  the memory may be drawn from an externally supplied page file
 *
 *  "block_size" [ IN ] - must be an even multiple of 2, minimum 64
 *
 *  "limit" [ IN, DFLT ZERO ] - the maximum number of bytes
 *  to be allocated zero for unlimited
 *
 *  "backing" [ IN, NULL OKAY ] - a page source for allocator.
 *  a new reference will be added if not null.
 */
LIB_EXPORT rc_t CC KMemBankMake ( KMemBank **bankp,
    size_t block_size, uint64_t limit, KPageFile *backing )
{
    rc_t rc;

    if ( bankp == NULL )
        rc = RC ( rcFS, rcMemory, rcConstructing, rcParam, rcNull );
    else
    {
        size_t page_size = KPageConstSize ();

        if ( page_size < sizeof ( KHdrMemBlock ) )
            rc = RC ( rcFS, rcMemory, rcConstructing, rcBlob, rcInsufficient );
        else if ( ( block_size & ( block_size - 1 ) ) != 0 )
            rc = RC ( rcFS, rcMemory, rcConstructing, rcParam, rcInvalid );
        else if ( block_size < 64 )
            rc = RC ( rcFS, rcMemory, rcConstructing, rcParam, rcInsufficient );
        else if ( block_size > page_size )
            rc = RC ( rcFS, rcMemory, rcConstructing, rcParam, rcExcessive );
        else
        {
            if ( backing != NULL )
                rc = KPageFileAddRef ( backing );
            else
            {
                size_t climit = ( size_t ) limit;
                if ( ( uint64_t ) climit != limit )
                    rc = RC ( rcFS, rcMemory, rcConstructing, rcParam, rcExcessive );
                else
                {
                    if ( climit == 0 )
                        climit = 256 * 1024 * 1024;
                    rc = KPageFileMakeUpdate ( & backing, NULL, climit, false );
                }
            }

            if ( rc == 0 )
            {
                KMemBank *bank = malloc ( sizeof * bank );
                if ( bank == NULL )
                    rc = RC ( rcFS, rcMemory, rcConstructing, rcMemory, rcExhausted );
                else
                {
                    bank -> pf = backing;
                    rc = KPageFilePosGet ( backing, & bank -> pg0, 0 );
                    if ( rc == 0 )
                    {
                        rc = KPageAccessUpdate ( bank -> pg0, ( void** ) & bank -> hdr, & page_size );
                        if ( rc == 0 )
                            bank -> read_only = false;
                        else
                        {
                            rc_t rc2 = KPageAccessRead ( bank -> pg0, ( const void** ) & bank -> hdr, & page_size );
                            if ( rc2 == 0 )
                            {
                                bank -> read_only = true;
                                rc = 0;
                            }
                        }
                        if ( rc == 0 )
                        {
                            KHdrMemBlock *hdr = bank -> hdr;

                            /* turn block size into shift bits */
                            for ( bank -> blk_bits = 6; block_size > ( ( size_t ) 1U << bank -> blk_bits ); ++ bank -> blk_bits )
                                ( void ) 0;

                            /* turn page size into shift bits */
                            for ( bank -> pb_bits = 6; page_size > ( ( size_t ) 1U << bank -> pb_bits ); ++ bank -> pb_bits )
                                ( void ) 0;

                            /* conversion bits between page and block */
                            assert ( bank -> pb_bits >= bank -> blk_bits );
                            bank -> pb_bits -= bank -> blk_bits;
                            bank -> blk_mask = ( 1U << bank -> pb_bits ) - 1;

                            /* initialize the header if new */
                            if ( hdr -> endian == 0 )
                            {
                                if ( bank -> read_only )
                                    rc = RC ( rcFS, rcMemory, rcConstructing, rcFile, rcCorrupt );
                                else
                                {
                                    size_t offset;
                                    uint8_t *pgmem = ( uint8_t* ) hdr;

                                    hdr -> endian = eByteOrderTag;
                                    hdr -> version = 1;
                                    hdr -> u . v1 . used = 0;
                                    hdr -> u . v1 . free_list = 0;
                                    hdr -> u . v1 . block_size = ( uint32_t ) block_size;
                                    hdr -> u . v1 . free_count = 0;
                                    hdr -> u . v1 . frag_count = 0;

                                    /* place remainder of page 0 on free list */
                                    for ( offset = block_size; offset < sizeof * hdr; offset += block_size )
                                        ( void ) 0;
                                    if ( offset < page_size )
                                    {
                                        KExtMemBlock *blk;
                                        uint64_t next = hdr -> u . v1 . free_list = offset / block_size + 1;
                                        do
                                        {
                                            blk = ( KExtMemBlock* ) & pgmem [ offset ];
                                            blk -> next = ++ next;
                                            ++ hdr -> u . v1 . free_count;
                                        }
                                        while ( ( offset += block_size ) < page_size );
                                        assert ( offset == page_size );
                                        blk -> next = 0;
                                    }
                                }
                            }
                            else if ( hdr -> endian != eByteOrderTag )
                            {
                                if ( hdr -> endian == eByteOrderReverse )
                                    rc = RC ( rcFS, rcMemory, rcConstructing, rcByteOrder, rcIncorrect );
                                else
                                    rc = RC ( rcFS, rcMemory, rcConstructing, rcFile, rcCorrupt );
                            }
                            else if ( hdr -> version != 1 )
                            {
                                rc = RC ( rcFS, rcMemory, rcConstructing, rcFile, rcBadVersion );
                            }

                            if ( rc == 0 )
                            {
                                /* record our happy limit */
                                bank -> limit = limit;

                                /* initialize the reference counter */
                                KRefcountInit ( & bank -> refcount, 1, "KMemBank", "make", "bank" );

                                * bankp = bank;
                                return 0;
                            }
                        }

                        KPageRelease ( bank -> pg0 );
                    }

                    free ( bank );
                }

                KPageFileRelease ( backing );
            }
        }

        * bankp = NULL;
    }

    return rc;
}


/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC KMemBankAddRef ( const KMemBank *self )
{
    if ( self != NULL ) switch ( KRefcountAdd ( & self -> refcount, "KMemBank" ) )
    {
    case krefOkay:
        break;
    default:
        return RC ( rcFS, rcMemory, rcAttaching, rcConstraint, rcViolated );
    }

    return 0;
}

LIB_EXPORT rc_t CC KMemBankRelease ( const KMemBank *self )
{
    if ( self != NULL ) switch ( KRefcountDrop ( & self -> refcount, "KMemBank" ) )
    {
    case krefOkay:
        break;
    case krefWhack:
        return KMemBankWhack ( ( KMemBank* ) self );
    default:
        return RC ( rcFS, rcMemory, rcReleasing, rcConstraint, rcViolated );
    }

    return 0;
}


/* AccessBlock
 */
static
rc_t KMemBankAccessBlockRead ( const KMemBank *self,
    uint64_t id, KMemPage *mp, const KExtMemBlock **blkp )
{
    rc_t rc;
    size_t bytes;
    const uint8_t *pgmem;

    /* get the proper page */
    uint32_t pgid = ( uint32_t ) ( ( id - 1 ) >> self -> pb_bits ) + 1;
    if ( pgid != mp -> pgid )
    {
        rc = KMemPageGet ( mp, self -> pf, pgid );
        if ( rc != 0 )
            return rc;
    }

    /* get pointer to page */
    rc = KPageAccessRead ( mp -> pg, ( const void** ) & pgmem, & bytes );
    if ( rc == 0 )
        * blkp = ( const KExtMemBlock* ) & pgmem [ ( ( id - 1 ) & self -> blk_mask ) << self -> blk_bits ];

    return rc;
}

static
rc_t KMemBankAccessBlockUpdate ( const KMemBank *self,
    uint64_t id, KMemPage *mp, KExtMemBlock **blkp )
{
    rc_t rc;
    size_t bytes;
    uint8_t *pgmem;

    /* get the proper page */
    uint32_t pgid = ( uint32_t ) ( ( id - 1 ) >> self -> pb_bits ) + 1;
    if ( pgid != mp -> pgid )
    {
        rc = KMemPageGet ( mp, self -> pf, pgid );
        if ( rc != 0 )
            return rc;
    }

    /* get pointer to page */
    rc = KPageAccessUpdate ( mp -> pg, ( void** ) & pgmem, & bytes );
    if ( rc == 0 )
        * blkp = ( KExtMemBlock* ) & pgmem [ ( ( id - 1 ) & self -> blk_mask ) << self -> blk_bits ];

    return rc;
}


/* AllocBlock
 */
static
rc_t KMemBankAllocBlock ( KMemBank *self, uint64_t *id,
    KMemPage *mp, KExtMemBlock **blkp, bool clear )
{
    rc_t rc;
    uint8_t *pgmem;
    KExtMemBlock *blk = NULL;

    KHdrMemBlock *hdr = self -> hdr;

    /* look for free block */
    if ( hdr -> u . v1 . free_list == 0 )
    {
        uint64_t next;
        size_t bsz, total, bytes;

        /* allocate a page-worth of blocks */
        rc = KMemPageAlloc ( mp, self -> pf );
        if ( rc != 0 )
            return rc;

        /* access the page in update mode */
        rc = KPageAccessUpdate ( mp -> pg, ( void** ) & pgmem, & bytes );
        if ( rc != 0 )
            return rc;

        /* stitch the blocks together into linked list of free blocks */
        bsz = hdr -> u . v1 . block_size;
        next = ( uint64_t ) ( mp -> pgid - 1 ) << self -> pb_bits;
        hdr -> u . v1 . free_list = ++ next;

        for ( total = 0; total < bytes; total += bsz )
        {
            blk = ( KExtMemBlock* ) & pgmem [ total ];
            blk -> next = ++ next;
            ++ hdr -> u . v1 . free_count;
        }
        blk -> next = 0;
    }

    rc = KMemBankAccessBlockUpdate ( self, * id = hdr -> u . v1 . free_list, mp, blkp );
    if ( rc == 0 )
    {
        blk = * blkp;
        hdr -> u . v1 . free_list = blk -> next;
        -- hdr -> u . v1 . free_count;
        blk -> next = 0;

        if ( clear )
            memset ( blk, 0, hdr -> u . v1 . block_size );
    }

    return rc;
}


/* Alloc
 *  allocate memory
 *
 *  "id" [ OUT ] - return parameter for memory allocation id
 *
 *  "bytes" [ IN ] - the number of bytes to allocate
 *
 *  "clear" [ IN ] - if true, initialize the memory to zero
 */
LIB_EXPORT rc_t CC KMemBankAlloc ( KMemBank *self,
    uint64_t *id, uint64_t bytes, bool clear )
{
    rc_t rc;

    if ( id == NULL )
        rc = RC ( rcFS, rcMemory, rcAllocating, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcFS, rcMemory, rcAllocating, rcSelf, rcNull );
        else if ( bytes == 0 )
            rc = 0;
        else
        {
            KExtMemBlock *block;

            KMemPage mp;
            KMemPageInit ( & mp, self -> pg0 );

            rc = KMemBankAllocBlock ( self, id, & mp, & block, clear );
            if ( rc == 0 )
            {
                KMemBlock *head = ( KMemBlock* ) block;
                uint64_t total, block_size = self -> hdr -> u . v1 . block_size;

                /* record size in first block */
                head -> size = bytes;

                for ( total = MEMBLOCK_DATASIZE ( head, block_size );
                      total < bytes;
                      total += MEMBLOCK_DATASIZE ( block, block_size ) )
                {
                    rc = KMemBankAllocBlock ( self, & block -> next, & mp, & block, clear );
                    if ( rc != 0 )
                        break;
                }

                if ( rc == 0 )
                {
                    KMemPageWhack ( & mp );
                    return 0;
                }

                KMemBankFree ( self, * id );
            }

            KMemPageWhack ( & mp );
        }

        * id = 0;
    }

    return rc;
}


/* Free
 *  free memory
 *
 *  "id" [ IN, ZERO OKAY ] - id of the allocation to be freed
 */
LIB_EXPORT rc_t CC KMemBankFree ( KMemBank *self, uint64_t id )
{
    rc_t rc;

    if ( id == 0 )
        return 0;

    if ( self == NULL )
        rc = RC ( rcFS, rcMemory, rcReleasing, rcSelf, rcNull );
    else
    {
        uint32_t count;
        uint64_t free_list;
        const KExtMemBlock *cblock;

        KMemPage mp;
        KMemPageInit ( & mp, self -> pg0 );

        /* find tail of allocation and count blocks to be freed */
        rc = KMemBankAccessBlockRead ( self, free_list = id, & mp, & cblock );
        for ( count = 1; rc == 0 && cblock -> next != 0; ++ count )
            rc = KMemBankAccessBlockRead ( self, free_list = cblock -> next, & mp, & cblock );
        if ( rc == 0 )
        {
            KExtMemBlock *block;
            rc = KMemBankAccessBlockUpdate ( self, free_list, & mp, & block );
            if ( rc == 0 )
            {
                block -> next = self -> hdr -> u . v1 . free_list;
                self -> hdr -> u . v1 . free_list = id;
                self -> hdr -> u . v1 . free_count += ( uint16_t ) count;
            }
        }

        KMemPageWhack ( & mp );
    }

    return rc;
}


/* Size
 *  returns the allocated capacity of the memory object
 *
 *  "id" [ IN ] - id of the allocation
 *
 *  "size" [ OUT ] - return parameter
 */
LIB_EXPORT rc_t CC KMemBankSize ( const KMemBank *self, uint64_t id, uint64_t *size )
{
    rc_t rc;

    if ( size == NULL )
        rc = RC ( rcFS, rcMemory, rcAccessing, rcParam, rcNull );
    else
    {
        * size = 0;

        if ( id == 0 )
            rc = 0;
        else if ( self == NULL )
            rc = RC ( rcFS, rcMemory, rcAccessing, rcSelf, rcNull );
        else
        {
            const KMemBlock *block;
            
            KMemPage mp;
            KMemPageInit ( & mp, self -> pg0 );

            rc = KMemBankAccessBlockRead ( self, id, & mp, ( const KExtMemBlock** ) & block );
            if ( rc == 0 )
                * size = block -> size;

            KMemPageWhack ( & mp );
        }
    }

    return rc;
}


/* SetSize
 *  performs a reallocation of memory object
 *
 *  "id" [ IN ] - id of the allocation being resized
 *
 *  "bytes" [ IN ] - new capacity
 */
LIB_EXPORT rc_t CC KMemBankSetSize ( KMemBank *self, uint64_t id, uint64_t bytes )
{
    rc_t rc;

    if ( id == 0 )
        rc = RC ( rcFS, rcMemory, rcResizing, rcId, rcNull );
    else if ( self == NULL )
        rc = RC ( rcFS, rcMemory, rcResizing, rcSelf, rcNull );
    else
    {
        const KMemBlock *chead;
            
        KMemPage mp;
        KMemPageInit ( & mp, self -> pg0 );

        /* get head block */
        rc = KMemBankAccessBlockRead ( self, id, & mp, ( const KExtMemBlock** ) & chead );
        if ( rc == 0 && chead -> size != bytes )
        {
            uint64_t cur = id;
            uint64_t total, size = chead -> size;
            uint64_t block_size = self -> hdr -> u . v1 . block_size;
            const KExtMemBlock *cblock = ( const KExtMemBlock* ) chead;

            /* while total < new size && more blocks, walk forward */
            for ( total = MEMBLOCK_DATASIZE ( chead, block_size );
                  total < size && total < bytes;
                  total += MEMBLOCK_DATASIZE ( cblock, block_size ) )
            {
                rc = KMemBankAccessBlockRead ( self, cur = cblock -> next, & mp, & cblock );
                if ( rc != 0 )
                    break;
            }
            if ( rc == 0 )
            {
                KExtMemBlock *block;
                rc = KMemBankAccessBlockUpdate ( self, cur, & mp, & block );
                if ( rc == 0 )
                {
                    KMemBlock *head = ( KMemBlock* ) block;

                    /* truncation */
                    if ( bytes < size )
                    {
                        /* free any remaining blocks */
                        rc = KMemBankFree ( self, block -> next );
                        if ( rc == 0 )
                        {
                            block -> next = 0;
                            if ( bytes < total )
                            {
                                if ( size > total )
                                    size = total;
                                if ( cur == id )
                                    memset ( MEMBLOCK_RDATA ( head, block_size, total - bytes ), 0, size - bytes );
                                else
                                    memset ( MEMBLOCK_RDATA ( block, block_size, total - bytes ), 0, size - bytes );
                            }
                        }
                    }
                    /* extension */
                    else
                    {
                        /* zero out to either new size or end of block */
                        if ( size < total )
                        {
                            uint64_t end = bytes;
                            if ( bytes > total )
                                end = total;
                            if ( cur == id )
                                memset ( MEMBLOCK_RDATA ( head, block_size, total - size ), 0, end - size );
                            else
                                memset ( MEMBLOCK_RDATA ( block, block_size, total - size ), 0, end - size );
                        }

                        /* allocate more blocks as necessary */
                        for ( ; total < bytes; total += MEMBLOCK_DATASIZE ( block, block_size ) )
                        {
                            rc = KMemBankAllocBlock ( self, & block -> next, & mp, & block, true );
                            if ( rc != 0 )
                                break;
                        }
                    }
                    if ( rc == 0 )
                    {
                        /* set the new size */
                        rc = KMemBankAccessBlockUpdate ( self, id, & mp, ( KExtMemBlock** ) & head );
                        if ( rc == 0 )
                            head -> size = bytes;
                    }
                }
            }
        }

        KMemPageWhack ( & mp );
    }

    return rc;
}


/* Read
 *  linearly access memory
 *
 *  "id" [ IN ] - id of the allocation being read
 *
 *  "pos" [ IN ] - starting offset into allocation
 *
 *  "buffer" [ IN ] and "bsize" [ IN ] - buffer for returned data
 *
 *  "num_read" [ OUT ] - return parameter for the number of bytes read
 */
LIB_EXPORT rc_t CC KMemBankRead ( const KMemBank *self, uint64_t id,
   uint64_t pos, void *buffer, size_t bsize, size_t *num_read )
{
    rc_t rc;

    if ( num_read == NULL )
        rc = RC ( rcFS, rcMemory, rcReading, rcParam, rcNull );
    else
    {
        if ( id == 0 )
            rc = RC ( rcFS, rcMemory, rcReading, rcId, rcNull );
        else if ( self == NULL )
            rc = RC ( rcFS, rcMemory, rcReading, rcSelf, rcNull );
        else
        {
            const KMemBlock *head;
            
            KMemPage mp;
            KMemPageInit ( & mp, self -> pg0 );

            /* get head block */
            rc = KMemBankAccessBlockRead ( self, id, & mp, ( const KExtMemBlock** ) & head );
            if ( rc == 0 && bsize != 0 && pos < head -> size )
            {
                if ( buffer == NULL )
                    rc = RC ( rcFS, rcMemory, rcReading, rcBuffer, rcNull );
                else
                {
                    const KExtMemBlock *block = ( const KExtMemBlock* ) head;
                    uint64_t end, block_size = self -> hdr -> u . v1 . block_size;

                    size_t to_read = bsize;
                    if ( pos + bsize > head -> size )
                        to_read = ( size_t ) ( head -> size - pos );

                    /* scan ahead to pos */
                    for ( end = MEMBLOCK_DATASIZE ( head, block_size );
                          end < pos;
                          end += MEMBLOCK_DATASIZE ( block, block_size ) )
                    {
                        rc = KMemBankAccessBlockRead ( self, block -> next, & mp, & block );
                        if ( rc != 0 )
                            break;
                    }
                    if ( rc == 0 )
                    {
                        uint8_t *dst = buffer;

                        /* the number of bytes from pos to end of block */
                        size_t remaining = ( size_t ) ( end - pos );

                        /* the number to copy will be <= the remaining bytes */
                        size_t to_copy, total = ( remaining > to_read ) ? to_read : remaining;

                        /* initial copy may be partial or complete */
                        memcpy ( dst, MEMBLOCK_RDATA ( block, block_size, remaining ), total );

                        for ( ; total < to_read; total += to_copy )
                        {
                            rc = KMemBankAccessBlockRead ( self, block -> next, & mp, & block );
                            if ( rc != 0 )
                                break;

                            to_copy = MEMBLOCK_DATASIZE ( block, block_size );
                            if ( total + to_copy > to_read )
                                to_copy = to_read - total;

                            memcpy ( & dst [ total ], block -> data, to_copy );
                        }

                        if ( total != 0 )
                        {
                            * num_read = total;
                            KMemPageWhack ( & mp );
                            return 0;
                        }
                    }
                }
            }

            KMemPageWhack ( & mp );
        }

        * num_read = 0;
    }

    return rc;
}


/* Write
 *  linearly update memory
 *  will resize as required
 *
 *  "id" [ IN ] - id of the allocation being writ
 *
 *  "pos" [ IN ] - starting offset into allocation
 *
 *  "buffer" [ IN ] and "size" [ IN ] - data to write
 *
 *  "num_writ" [ OUT ] - return parameter for the number of bytes written
 */
LIB_EXPORT rc_t CC KMemBankWrite ( KMemBank *self, uint64_t id,
    uint64_t pos, const void *buffer, size_t size, size_t *num_writ )
{
    rc_t rc;

    if ( num_writ == NULL )
        rc = RC ( rcFS, rcMemory, rcWriting, rcParam, rcNull );
    else
    {
        if ( id == 0 )
            rc = RC ( rcFS, rcMemory, rcWriting, rcId, rcNull );
        else if ( self == NULL )
            rc = RC ( rcFS, rcMemory, rcWriting, rcSelf, rcNull );
        else
        {
            const KMemBlock *chead;
            
            KMemPage mp;
            KMemPageInit ( & mp, self -> pg0 );

            /* get head block */
            rc = KMemBankAccessBlockRead ( self, id, & mp, ( const KExtMemBlock** ) & chead );
            if ( rc == 0 && size != 0 )
            {
                if ( buffer == NULL )
                    rc = RC ( rcFS, rcMemory, rcWriting, rcBuffer, rcNull );
                else
                {
                    uint64_t cur = id;
                    uint64_t eof = chead -> size;
                    const KExtMemBlock *cblock = ( const KExtMemBlock* ) chead;
                    uint64_t end, block_size = self -> hdr -> u . v1 . block_size;

                    /* scan ahead to pos */
                    for ( end = MEMBLOCK_DATASIZE ( chead, block_size );
                          end < eof && end < pos;
                          end += MEMBLOCK_DATASIZE ( cblock, block_size ) )
                    {
                        rc = KMemBankAccessBlockRead ( self, cur = cblock -> next, & mp, & cblock );
                        if ( rc != 0 )
                            break;
                    }
                    if ( rc == 0 )
                    {
                        KExtMemBlock *block;
                        rc = KMemBankAccessBlockUpdate ( self, cur, & mp, & block );
                        if ( rc == 0 )
                        {
                            size_t to_copy;
                            KMemBlock *head = ( KMemBlock* ) block;

                            /* detect write position beyond eof */
                            if ( eof < pos )
                            {
                                /* need to zero ahead to either pos or end of block */
                                uint64_t stop = pos;
                                if ( pos > end )
                                    stop = end;

                                /* bytes to zero */
                                to_copy = ( size_t ) ( stop - eof );

                                /* zero from starting offset */
                                if ( cur == id )
                                    memset ( MEMBLOCK_RDATA ( head, block_size, end - eof ), 0, to_copy );
                                else
                                    memset ( MEMBLOCK_RDATA ( block, block_size, end - eof ), 0, to_copy );

                                /* allocate more blocks as necessary */
                                for ( ; end < pos; end += MEMBLOCK_DATASIZE ( block, block_size ) )
                                {
                                    rc = KMemBankAllocBlock ( self, & block -> next, & mp, & block, true );
                                    if ( rc != 0 )
                                        break;
                                    cur = block -> next;
                                }
                            }
                            if ( rc == 0 )
                            {
                                size_t total;
                                const uint8_t *src = buffer;

                                /* copy in bytes - alloc as needed */
                                for ( total = 0; ; end += MEMBLOCK_DATASIZE ( block, block_size ) )
                                {
                                    to_copy = ( size_t ) ( end - pos );
                                    if ( total + to_copy > size )
                                        to_copy = size - total;

                                    if ( cur == id )
                                        memcpy ( MEMBLOCK_RDATA ( head, block_size, end - pos ), & src [ total ], to_copy );
                                    else
                                        memcpy ( MEMBLOCK_RDATA ( block, block_size, end - pos ), & src [ total ], to_copy );

                                    pos += to_copy;
                                    if ( ( total += to_copy ) == size )
                                        break;

                                    if ( block -> next == 0 )
                                        rc = KMemBankAllocBlock ( self, & block -> next, & mp, & block, false );
                                    else
                                        rc = KMemBankAccessBlockUpdate ( self, block -> next, & mp, & block );
                                    if ( rc != 0 )
                                        break;

                                    cur = block -> next;
                                }

                                /* update head -> size */
                                if ( total != 0 )
                                {
                                    rc = 0;
                                    if ( pos > eof )
                                    {
                                        rc = KMemBankAccessBlockUpdate ( self, block -> next, & mp, ( KExtMemBlock** ) & head );
                                        if ( rc == 0 )
                                            head -> size = pos;
                                    }
                                    if ( rc == 0 )
                                    {
                                        KMemPageWhack ( & mp );
                                        * num_writ = total;
                                        return 0;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            KMemPageWhack ( & mp );
        }

        * num_writ = 0;
    }

    return rc;
}
