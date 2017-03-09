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

struct KSparseFile_v1;
#define KFILE_IMPL struct KSparseFile_v1

#include <klib/log.h>
#include <klib/rc.h>
#include <kfs/extern.h>
#include <kfs/file.h>
#include <kfs/sparsefile.h>
#include <kproc/timeout.h>

#include <kfs/file-impl.h>

#include <sysalloc.h>

#include <assert.h>


/*--------------------------------------------------------------------------
 * KSparseFileHeader
 *  stored in the first sector of sparse file
 */
typedef struct KSparseFileHeader_v1 KSparseFileHeader_v1;
struct KSparseFileHeader_v1
{
    char ncbi [ 4 ];                /* "ncbi"                      */
    char sprs [ 4 ];                /* "sprs"                      */
    uint32_t byte_order;            /* 0x08122011 when good,       */
                                    /* 0x11021208 when reversed    */
    uint32_t version;               /* current file format version */
    union
    {
        struct
        {
            uint64_t logical_size;  /* logical file size in bytes  */
            uint64_t dense_len;     /* number of dense data blocks */
            uint64_t dense_start;   /* offset to first data block  */
            uint64_t map_start;     /* offset to block map         */
            uint64_t map_size;      /* number of stored map bytes  */
            uint64_t block_size;    /* block size in bytes         */
            uint8_t fixed;          /* file size is fixed or not   */
        } v1;

        uint8_t filler [ 512 - 16 ];
    } u;
};

#define KSPARSE_FILE_VERSION 1

static
uint64_t KSparseFileHeaderLogicalSize ( const KSparseFileHeader_v1 * self )
{
    switch ( self -> version )
    {
    case 1:
        return self -> u . v1 . logical_size;
    }

    return 0;
}

static
void KSparseFileHeaderSetLogicalSize ( KSparseFileHeader_v1 * self, uint64_t size )
{
    switch ( self -> version )
    {
    case 1:
        self -> u . v1 . logical_size = size;
    }
}

static
uint64_t KSparseFileHeaderDenseLength ( const KSparseFileHeader_v1 * self )
{
    switch ( self -> version )
    {
    case 1:
        return self -> u . v1 . dense_len;
    }

    return 0;
}

static
void KSparseFileHeaderSetDenseLength ( KSparseFileHeader_v1 * self, uint64_t length )
{
    switch ( self -> version )
    {
    case 1:
        self -> u . v1 . dense_len = length;
    }
}

static
size_t KSparseFileHeaderBlockSize ( const KSparseFileHeader_v1 * self )
{
    switch ( self -> version )
    {
    case 1:
        return ( size ) self -> u . v1 . block_size;
    }

    return 0;
}

static
bool KSparseFileHeaderFixed ( const KSparseFileHeader_v1 * self )
{
    switch ( self -> version )
    {
    case 1:
        return ( bool ) self -> u . v1 . fixed;
    }

    return 0;
}


/*--------------------------------------------------------------------------
 * KSparseFileFreelist
 *  a simple dual-level bitmap free-list
 *  the first level is an index onto the second level
 *  where each bit in the first level corresponds to 1024 bits in the second
 *  and each bit in the second corresponds to a block in the dense file
 */
typedef struct KSparseFileFreelist_v1 KSparseFileFreelist_v1;
struct KSparseFileFreelist_v1
{
    uint64_t * bm1;
    uint64_t * bm2;
    uint64_t len;
};

#define KSparseFileFreelistTest_v1( bm, idx, off ) \
    ( ( bm ) [ idx ] & ( ( uint64_t ) 1 << ( off ) ) )

#define KSparseFileFreelistSet_v1( bm, idx, off ) \
    ( void ) ( ( bm ) [ idx ] |= ( ( uint64_t ) 1 << ( off ) ) )

static
void KSparseFileFreelistWhack_v1 ( KSparseFileFreelist_v1 * self )
{
    free ( self -> bm2 );
    free ( self -> bm1 );
    self -> bm1 = self -> bm2 = NULL;
}

static
rc_t KSparseFileFreelistMark_v1 ( KSparseFileFreelist_v1 * self, uint64_t dense_block_idz, uint64_t dense_len )
{
    uint64_t div_1024 = dense_block_idz / 1024;
    uint64_t bm2_idx = dense_block_idz / 64;
    uint64_t bm2_off = dense_block_idz % 64;
    uint64_t bm1_idx = div_1024 / 64;
    uint64_t bm1_off = div_1024 % 64;

    assert ( dense_block_idz < dense_len );

    if ( self -> len != dense_len )
    {
        size_t size2 = ( size_t ) ( ( dense_len + 63 ) / 64 );
        size_t size1 = ( size2 + 15 ) / 16;
        void * mem = realloc ( self -> bm1, size1 * 8 );
        if ( mem == NULL )
            return RC ( rcFS, rcFile, rcResizing, rcMemory, rcExhausted );
        self -> bm1 = mem;
        mem = realloc ( self -> bm2, size2 * 8 );
        if ( mem == NULL )
            return RC ( rcFS, rcFile, rcResizing, rcMemory, rcExhausted );
        self -> bm2 = mem;

        while ( self -> len < dense_len )
        {
#pragma error "here I have to clear the bits up toward the top"
        }
    }

    /* should not already be set */
    assert ( KSparseFileFreelistTest_v1 ( self -> bm2, bm2_idx, bm2_off ) == 0 );
    KSparseFileFreelistSet_v1 ( self -> bm2, bm2_idx, bm2_off );

    /* mark the level 1 */
    KSparseFileFreelistSet_v1 ( self -> bm1, bm1_idx, bm1_off );

    return 0;
}


/*--------------------------------------------------------------------------
 * KSparseFileBlockIdMap
 *  a map from zero-based logical block-id to one-based dense block-id
 *
 *  the size of each id is determined by the number of dense blocks,
 *  as contained in the header "dense_len".
 *
 *  the length of the map array is determined by the "logical_size"
 *  stored in header, divided by the "block_size".
 */
typedef struct KSparseFileBlockIdMap_v1 KSparseFileBlockIdMap_v1;
struct KSparseFileBlockIdMap_v1
{
    union
    {
        uint8_t * u8;
        uint16_t * u16;
        uint32_t * u32;
        uint64_t * u64;

    } map;

    uint8_t var;
};

enum
{
    smapU8,     /* use map . u8  [] */
    smapU16,    /* use map . u16 [] */
    smapU32,    /* use map . u32 [] */
    smapU64,    /* use map . u64 [] */

    smapCount
};

static
void KSparseFileBlockIdMapWhack_v1 ( KSparseFileBlockIdMap_v1 * self )
{
    switch ( self -> var )
    {
    case smapU8:
        free ( self -> map . u8 );
        self -> map . u8 = NULL;
        break;
    case smapU16:
        free ( self -> map . u16 );
        self -> map . u16 = NULL;
        break;
    case smapU32:
        free ( self -> map . u32 );
        self -> map . u32 = NULL;
        break;
    case smapU64:
        free ( self -> map . u64 );
        self -> map . u64 = NULL;
        break;
    }
    self -> var = smapU8;
}

static
rc_t KSparseFileBlockIdMapResize ( KSparseFileBlockIdMap_v1 * self, uint64_t * map_len,
    uint64_t new_log_block_idz, uint32_t new_var )
{
    void * map_mem;
    size_t new_elem_size, new_size;
    uint64_t i, old_log_block_id = * map_len;

    uint64_t new_map_len = ( new_log_block_idz + 64 ) & ~ ( uint64_t ) 63;

    switch ( self -> var )
    {
    case smapU8:
        map_mem = self -> map . u8;
        new_elem_size = sizeof self -> map . u8 [ 0 ];
        break;
    case smapU16:
        map_mem = self -> map . u16;
        new_elem_size = sizeof self -> map . u16 [ 0 ];
        break;
    case smapU32:
        map_mem = self -> map . u32;
        new_elem_size = sizeof self -> map . u32 [ 0 ];
        break;
    case smapU64:
        map_mem = self -> map . u64;
        new_elem_size = sizeof self -> map . u64 [ 0 ];
        break;
    default:
        return RC ( rcFS, rcFile, rcAccessing, rcData, rcCorrupt );
    }

    if ( new_var != self -> var )
    {
        switch ( new_var )
        {
        case smapU8:
            new_elem_size = sizeof self -> map . u8 [ 0 ];
            break;
        case smapU16:
            new_elem_size = sizeof self -> map . u16 [ 0 ];
            break;
        case smapU32:
            new_elem_size = sizeof self -> map . u32 [ 0 ];
            break;
        case smapU64:
            new_elem_size = sizeof self -> map . u64 [ 0 ];
            break;
        default:
            return RC ( rcFS, rcFile, rcAccessing, rcParam, rcInvalid );
        }
    }

    /* detect a case where there's nothing to do */
    else if ( new_map_len == * map_len )
    {
        return 0;
    }

    new_size = ( size_t ) new_map_len * new_elem_size;
#pragma message "this needs to have checking for overflow"
    map_mem = realloc ( map_mem, new_size );
    if ( map_mem == NULL )
        return RC ( rcFS, rcFile, rcResizing, rcMemory, rcExhausted );

#undef V2
#define V2( v1, v2 ) \
    ( ( v1 ) * smapCount + ( v2 ) )

    switch ( V2 ( self -> var, new_var ) )
    {

    case V2 ( smapU8, smapU8 ):
    {
        uint8_t * dst = map_mem;
        for ( i = * map_len; i < new_map_len; ++ i )
            dst [ i ] = 0;
        self -> map . u8 = map_mem;
        break;
    }

    case V2 ( smapU8, smapU16 ):
    {
        const uint8_t * src = map_mem;
        uint16_t * dst = map_mem;
        for ( i = * map_len; i != 0; )
        {
            -- i;
            dst [ i ] = src [ i ];
        }

        for ( i = * map_len; i < new_map_len; ++ i )
            dst [ i ] = 0;

        self -> map . u16 = map_mem;
        break;
    }

    case V2 ( smapU16, smapU16 ):
    {
        uint16_t * dst = map_mem;
        for ( i = * map_len; i < new_map_len; ++ i )
            dst [ i ] = 0;
        self -> map . u16 = map_mem;
        break;
    }

    case V2 ( smapU16, smapU32 ):
    {
        const uint16_t * src = map_mem;
        uint32_t * dst = map_mem;
        for ( i = * map_len; i != 0; )
        {
            -- i;
            dst [ i ] = src [ i ];
        }

        for ( i = * map_len; i < new_map_len; ++ i )
            dst [ i ] = 0;

        self -> map . u32 = map_mem;
        break;
    }

    case V2 ( smapU32, smapU32 ):
    {
        uint32_t * dst = map_mem;
        for ( i = * map_len; i < new_map_len; ++ i )
            dst [ i ] = 0;
        self -> map . u32 = map_mem;
        break;
    }

    case V2 ( smapU32, smapU64 ):
    {
        const uint32_t * src = map_mem;
        uint64_t * dst = map_mem;
        for ( i = * map_len; i != 0; )
        {
            -- i;
            dst [ i ] = src [ i ];
        }

        for ( i = * map_len; i < new_map_len; ++ i )
            dst [ i ] = 0;

        self -> map . u64 = map_mem;
        break;
    }

    case V2 ( smapU64, smapU64 ):
    {
        uint64_t * dst = map_mem;
        for ( i = * map_len; i < new_map_len; ++ i )
            dst [ i ] = 0;
        self -> map . u64 = map_mem;
        break;
    }

    default:
        switch ( self -> var )
        {
        case smapU8:
            self -> map . u8 = map_mem;
            break;
        case smapU16:
            self -> map . u16 = map_mem;
            break;
        case smapU32:
            self -> map . u32 = map_mem;
            break;
        case smapU64:
            self -> map . u64 = map_mem;
            break;
        }
        return RC ( rcFS, rcFile, rcResizing, rcParam, rcInvalid );
    }

#undef V2

    self -> var = new_var;
    * map_len = new_map_len;

    return 0;
}

static
uint64_t KSparseFileBlockIdMapLookup ( const KSparseFileBlockIdMap * self, uint64_t map_len, uint64_t log_block_idz )
{
    assert ( log_block_idz < map_len );

    switch ( self -> var )
    {
    case smapU8:
        return self -> map . u8 [ log_block_idz ];
    case smapU16:
        return self -> map . u16 [ log_block_idz ];
    case smapU32:
        return self -> map . u32 [ log_block_idz ];
    case smapU64:
        return self -> map . u64 [ log_block_idz ];
    default:
        assert ( 0 );
    }

    return 0;
}


/*--------------------------------------------------------------------------
 * KSparseFile
 *  a wrapper file that looks onto a "dense" file
 */
struct KSparseFile_v1
{
    KFile_v1 dad;
    KSparseFileHeader_v1 hdr;
    KSparseFileFreelist_v1 free;
    uint64_t map_len;
    KFile_v1 * dense;
    KSparseFileBlockIdMap_v1 map;
};

static
rc_t CC KSparseFileWhack_v1 ( KSparseFile_v1 * self )
{
    /* TBD - perform any writes to dense file */

    /* tear down free list */
    KSparseFileFreelistWhack_v1 ( & self -> free );

    /* tear down block-id map */
    KSparseFileBlockIdMapWhack_v1 ( & self -> map );
    self -> map_len = 0;

    /* close down the file */
    KFileRelease_v1 ( self -> dense );
    self -> dense = NULL;

    /* free the object */
    free ( self );
    return 0;
}

static
struct KSysFile_v1 * CC KSparseFileGetSysFile_v1 ( const KSparseFile_v1 * self, uint64_t * offset )
{
    assert ( offset != NULL );
    assert ( * offset == 0 );

    return NULL;
}

static
rc_t CC KSparseFileRandomAccess_v1 ( const KSparseFile_v1 * self )
{
    /* by definition, we are random access */
    return 0;
}

static
rc_t CC KSparseFileSize_v1 ( const KSparseFile_v1 * self, uint64_t * size )
{
    assert ( self != NULL );
    assert ( size != NULL );

    * size = KSparseFileHeaderLogicalSize ( & self -> hdr );

    return 0;
}

static
rc_t KSparseFileDeleteDenseBlock_v1 ( KSparseFile_v1 *self, uint64_t dense_block_id, timeout_t *tm )
{
    uint64_t dense_block_idz = dense_block_id - 1;
    size_t block_size = KSparseFileHeaderBlockSize ( & self -> hdr );
    uint64_t dense_start = KSparseFileHeaderDenseStart ( & self -> hdr );
    uint64_t dense_len = KSparseFileHeaderDenseLength ( & self -> hdr );
    uint64_t dense_pos = dense_block_idz * block_size + dense_start;

    if ( dense_block_id == dense_len )
    {
#pragma error "update the free list, see how many blocks can be released"
        KFileSetSize_v1 ( self -> dense, dense_pos );
        KSparseFileHeaderSetDenseLength ( & self -> hdr, dense_block_idz );
    }
    else
    {
#pragma error "zero out the trapped block"
        rc = KSparseFileFreelistMark_v1 ( & self -> free, dense_block_idx, dense_len );
    }

    return rc;
}

static
rc_t CC KSparseFileSetSize_v1 ( KSparseFile_v1 * self, uint64_t new_logical_size )
{
    uint64_t logical_size;

    assert ( self != NULL );
    assert ( self -> dad . write_enabled );

    logical_size = KSparseFileHeaderLogicalSize ( & self -> hdr );
    if ( new_logical_size != logical_size )
    {
        size_t block_size;
        uint64_t old_log_block_idz;
        uint64_t new_log_block_idz;

        /* if the file is fixed size, this must fail with a non-zero rc */
        if ( KSparseFileHeaderFixed ( & self -> hdr ) )
            return RC ( rcFS, rcFile, rcResizing, rcConstraint, rcViolated );

        /* calculate the new logical size in blocks */
        block_size = KSparseFileHeaderBlockSize ( & self -> hdr );
        assert ( block_size != 0 );
        old_log_block_idz = logical_size / block_size;
        new_log_block_idz = new_logical_size / block_size;

        /* see if resize affects map */
        if ( new_log_block_idz != old_log_block_idz )
        {
            rc_t rc = 0;

            /* truncate dense file */
            while ( old_log_block_idz > new_log_block_idz )
            {
                uint64_t dense_block_id = KSparseFileBlockIdMapLookup ( & self -> map,
                    self -> map_len, old_log_block_idz );
                if ( dense_block_id != 0 )
                {
                    rc = KSparseFileDeleteDenseBlock_v1 ( self, dense_block_id, NULL );
                    if ( rc != 0 )
                        break;
                }
                
                -- old_log_block_idz;
            }

            if ( rc == 0 )
            {
                /* need to resize the existing map */
                rc = KSparseFileBlockIdMapResize ( & self -> map, & self -> map_len,
                    new_log_block_idz, self -> map -> var );
            }

            /* TBD - rewrite map and/or footer to dense file */
        }

        /* record the new logical size */
        KSparseFileHeaderSetLogicalSize ( & self -> hdr, new_logical_size );
    }

    return 0;
}

static
rc_t KSparseFileReadBlock_v1 ( const KSparseFile_v1 *self, uint64_t log_block_idz,
    size_t offset, uint8_t * buffer, size_t bsize, timeout_t *tm )
{
    rc_t rc = 0;
    uint64_t dense_block_id = KSparseFileBlockIdMapLookup ( & self -> map, self -> map_len, log_block_idz );
    if ( dense_block_id == 0 )
    {
        /* sparse read */
        memset ( buffer, 0, bsize );
    }
    else
    {
        size_t block_size = KSparseFileHeaderBlockSize ( & self -> hdr );
        uint64_t dense_start = KSparseFileHeaderDenseStart ( & self -> hdr );
        uint64_t dense_pos = ( dense_block_id - 1 ) * block_size + dense_start;
        assert ( offset + bsize <= block_size );
        rc = KFileTimedReadExactly ( self -> dense, dense_pos + offset, buffer, bsize, tm );
    }

    return rc;
}

static
rc_t CC KSparseFileTimedRead_v1 ( const KSparseFile_v1 *self, uint64_t pos,
    void *buffer, size_t bsize, size_t *num_read, struct timeout_t *tm )
{
    rc_t rc = 0;

    uint8_t * b;
    size_t total;
    uint64_t log_block_id, offset, start, end;

    /* if "pos" is >= logical_size, read NOTHING */
    if ( pos >= self -> logical_size )
    {
        assert ( num_read != NULL );
        * num_read = 0;
        return 0;
    }

    /* will need to loop over buffer */
    b = buffer;

    /* convert "pos" into a block_id and offset */
    assert ( self -> block_size != 0 );
    log_block_id = pos / self -> block_size;
    offset = pos % self -> block_size;

    /* the end of the requested logical read region */
    end = pos + bsize;

    /* limit to end of file */
    if ( end > self -> logical_size )
        end = self -> logical_size;

    /* loop over each affected block */
    for ( total = 0, start = pos - offset; start < end;
           ++ log_block_id, offset = 0, start += self -> block_size )
    {
        /* the number to read in this block.
           can be < block_size for first or last,
           must be exactly block_size in middle */
        uint64_t to_read = end - start - offset;
        if ( to_read > ( uint64_t ) self -> block_size )
            to_read = self -> block_size;

        /* read a number of bytes from this block */
        rc = KSparseFileReadBlock_v1 ( self, log_block_id, ( size_t ) offset,
            b + total, ( size_t ) to_read, tm );

        /* break if not all bytes were read */
        if ( rc == 0 )
            break;

        /* incorporate all bytes into count */
        total += ( size_t ) to_read;
    }

    /* record total number read */
    assert ( num_read != NULL );
    * num_read = total;

    /* if any were read, there can be no error */
    if ( total != 0 )
        return 0;

    return rc;
}

static
rc_t CC KSparseFileRead_v1 ( const KSparseFile_v1 *self, uint64_t pos, 
    void *buffer, size_t bsize, size_t *num_read )
{
    return KSparseFileTimedRead_v1 ( self, pos, buffer, bsize, num_read, NULL );
}

static
bool KSparseFileScanBlock_v1 ( const uint8_t * buffer, size_t bsize )
{
    /* TBD - this needs to examine the left edge byte-by-byte,
       then run the interior using SSE instructions,
       then examine any right edge byte-by-byte. */
    size_t i;
    for ( i = 0; i < bsize; ++ i )
    {
        if ( buffer [ i ] != 0 )
            return false;
    }
    return true;
}

static
rc_t KSparseFileExtendMap_v1 ( KSparseFile_v1 * self, uint64_t log_block_id )
{
    void * map_mem;
    uint64_t map_len;
    size_t elem_size, zero_size;

    /* can't extend map of a fixed sparse file */
    if ( self -> fixed )
        return RC ( rcFS, rcFile, rcResizing, rcSize, rcLocked );

    /* figure out which variant */
    switch ( self -> var )
    {
    case smapU8:
        map_mem = self -> map . u8;
        elem_size = sizeof self -> map . u8 [ 0 ];
        break;
    case smapU16:
        map_mem = self -> map . u16;
        elem_size = sizeof self -> map . u16 [ 0 ];
        break;
    case smapU32:
        map_mem = self -> map . u32;
        elem_size = sizeof self -> map . u32 [ 0 ];
        break;
    case smapU64:
        map_mem = self -> map . u64;
        elem_size = sizeof self -> map . u64 [ 0 ];
        break;
    default:
        return RC ( rcFS, rcFile, rcResizing, rcData, rcCorrupt );
    }

    /* resize it */
    assert ( log_block_id >= self -> map_len );
    map_len = ( log_block_id + 64 ) & ~ ( uint64_t ) 63;
    if ( ( uint64_t ) ( size_t ) map_len != map_len )
        return RC ( rcFS, rcFile, rcResizing, rcMemory, rcExhausted );
    /* TBD - put in checks for overflow */
    map_mem = realloc ( map_mem, elem_size * ( size_t ) map_len );
    if ( map_mem == NULL )
        return RC ( rcFS, rcFile, rcResizing, rcMemory, rcExhausted );

    /* zero it out and set it */
    zero_size = ( size_t ) ( map_len - self -> map_len ) * elem_size;
    switch ( self -> var )
    {
    case smapU8:
        self -> map . u8 = map_mem;
        memset ( & self -> map . u8 [ self -> map_len ], 0, zero_size );
        break;
    case smapU16:
        self -> map . u16 = map_mem;
        memset ( & self -> map . u16 [ self -> map_len ], 0, zero_size );
        break;
    case smapU32:
        self -> map . u32 = map_mem;
        memset ( & self -> map . u32 [ self -> map_len ], 0, zero_size );
        break;
    case smapU64:
        self -> map . u64 = map_mem;
        memset ( & self -> map . u64 [ self -> map_len ], 0, zero_size );
        break;
    }

    self -> map_len = map_len;

    return 0;
}

static
rc_t KSparseFileAllocDenseBlock_v1 ( KSparseFile_v1 *self, uint64_t * dense_block_id )
{
    /* TBD - add in a deleted block list */
    uint64_t idz = self -> dense_len;

    void * map_mem;
    size_t new_elem;
    bool resize = false;

    switch ( self -> var )
    {
    case smapU8:
        assert ( idz < 0x100 );
        if ( idz == 0xFF )
        {
            map_mem = self -> map . u8;
            new_elem = sizeof self -> map . u16 [ 0 ];
            resize = true;
        }
        break;
    case smapU16:
        assert ( idz < 0x10000 );
        if ( idz == 0xFFFF )
        {
            map_mem = self -> map . u16;
            new_elem = sizeof self -> map . u32 [ 0 ];
            resize = true;
        }
        break;
    case smapU32:
        assert ( idz < 0x100000000 );
        if ( idz == 0xFFFFFFFF )
        {
            map_mem = self -> map . u32;
            new_elem = sizeof self -> map . u64 [ 0 ];
            resize = true;
        }
        break;
    }

    if ( resize )
    {
        uint64_t i;
        size_t new_size = ( size_t ) self -> map_len * new_elem;
        map_mem = realloc ( map_mem, new_size );
        if ( map_mem == NULL )
            return RC ( rcFS, rcFile, rcResizing, rcMemory, rcExhausted );

        switch ( self -> var )
        {
        case smapU8:
        {
            const uint8_t * src = map_mem;
            uint16_t * dst = map_mem;
            for ( i = self -> map_len; i != 0; )
            {
                -- i;
                dst [ i ] = src [ i ];
            }
            self -> map . u16 = map_mem;
            self -> var = smapU16;
            break;
        }

        case smapU16:
        {
            const uint16_t * src = map_mem;
            uint32_t * dst = map_mem;
            for ( i = self -> map_len; i != 0; )
            {
                -- i;
                dst [ i ] = src [ i ];
            }
            self -> map . u32 = map_mem;
            self -> var = smapU32;
            break;
        }

        case smapU32:
        {
            const uint32_t * src = map_mem;
            uint64_t * dst = map_mem;
            for ( i = self -> map_len; i != 0; )
            {
                -- i;
                dst [ i ] = src [ i ];
            }
            self -> map . u64 = map_mem;
            self -> var = smapU64;
            break;
        }

        }
    }

    * dense_block_id = self -> dense_len = idz + 1;

    return 0;
}

static
rc_t KSparseFileZeroDense_v1 ( KSparseFile_v1 *self, uint64_t pos, size_t bytes, timeout_t * tm )
{
    rc_t rc;
    size_t total, num_writ;

    /* TBD - it's doubtful that this function is useful, at least for left/right fills. */
    size_t zero_size = 1 * 1024 * 1024;
    void * zero_bytes = calloc ( 1, zero_size );
    if ( zero_bytes == NULL )
        return RC ( rcFS, rcFile, rcWriting, rcMemory, rcExhausted );

    for ( rc = 0, total = 0; total < bytes; total += num_writ )
    {
        size_t to_write = bytes - total;
        if ( to_write > zero_size )
            to_write = zero_size;

        rc = KFileTimedWrite ( self -> dense, pos + total, zero_bytes, to_write, & num_writ, tm );
        if ( rc != 0 )
            break;
        if ( num_writ == 0 )
        {
            rc = RC ( rcFS, rcFile, rcWriting, rcTransfer, rcIncomplete );
            break;
        }
    }

    free ( zero_bytes );

    return rc;
}

static
rc_t KSparseFileWriteBlock_v1 ( KSparseFile_v1 *self, uint64_t log_block_id,
    size_t offset, const uint8_t * buffer, size_t bsize, timeout_t *tm )
{
    rc_t rc;
    uint32_t case_code;
    uint64_t dense_block_id, dense_pos;
    bool all_zeros, whole_block, initial_block, last_block, block_exists;
    bool alloc_block, fill_left, write_data, fill_right, delete_block;

    /* extend map if needed */
    if ( log_block_id >= self -> map_len )
    {
        rc = KSparseFileExtendMap_v1 ( self, log_block_id );
        if ( rc != 0 )
            return rc;
    }

    /* state variables:
       a. all_zeros
       b. whole block
       c. initial block
       d. block_exists
    */

    /* examine the buffer for all zeros */
    assert ( buffer != NULL );
    all_zeros = KSparseFileScanBlock_v1 ( buffer, bsize );

    /* check for writing a whole block */
    whole_block = bsize == self -> block_size;

    /* initial block differs from other blocks by offset.
       if the initial block has no offset, then it does
       not differ in any significant way. */
    initial_block = offset != 0;

    /* the last block may differ from other blocks by
       not extending to the right edge. otherwise, it
       is not distinguished. */
    last_block = offset + bsize != self -> block_size;

    /* check to see if the block already exists */
    assert ( log_block_id < self -> map_len );
    switch ( self -> var )
    {
    case smapU8:
        assert ( self -> map . u8 != NULL );
        dense_block_id = self -> map . u8 [ log_block_id ];
        break;
    case smapU16:
        assert ( self -> map . u16 != NULL );
        dense_block_id = self -> map . u16 [ log_block_id ];
        break;
    case smapU32:
        assert ( self -> map . u32 != NULL );
        dense_block_id = self -> map . u32 [ log_block_id ];
        break;
    case smapU64:
        assert ( self -> map . u64 != NULL );
        dense_block_id = self -> map . u64 [ log_block_id ];
        break;
    default:
        return RC ( rcFS, rcFile, rcWriting, rcData, rcCorrupt );
    }

    block_exists = dense_block_id != 0;

    /* logic to determine sequence of actions */
    alloc_block = fill_left = write_data = fill_right = delete_block = false;

    case_code =
        ( ( uint32_t ) all_zeros     << 3 ) |
        ( ( uint32_t ) block_exists  << 2 ) |
        ( ( uint32_t ) whole_block   << 1 ) |
        ( ( uint32_t ) initial_block << 0 ) ;
    switch ( case_code )
    {
    case 0: /* !all_zeros, !block_exists, !whole_block, !initial_block */
        alloc_block = write_data = fill_right = true;
        break;
    case 1: /* !all_zeros, !block_exists, !whole_block, initial_block */
        alloc_block = fill_left = write_data = true;
        fill_right = last_block;
        break;
    case 2: /* !all_zeros, !block_exists, whole_block, !initial_block */
        alloc_block = write_data = true;
        break;
    case 3: /* !all_zeros, !block_exists, whole_block, initial_block */
        return RC ( rcFS, rcFile, rcWriting, rcConstraint, rcViolated );
    case 4: /* !all_zeros, block_exists, !whole_block, !initial_block */
    case 5: /* !all_zeros, block_exists, !whole_block, initial_block */
    case 6: /* !all_zeros, block_exists, whole_block, !initial_block */
        write_data = true;
        break;
    case 7: /* !all_zeros, block_exists, whole_block, initial_block */
        return RC ( rcFS, rcFile, rcWriting, rcConstraint, rcViolated );
    case 8: /* all_zeros, !block_exists, !whole_block, !initial_block */
    case 9: /* all_zeros, !block_exists, !whole_block, initial_block */
    case 10: /* all_zeros, !block_exists, whole_block, !initial_block */
        break;
    case 11: /* all_zeros, !block_exists, whole_block, initial_block */
        return RC ( rcFS, rcFile, rcWriting, rcConstraint, rcViolated );
    case 12: /* all_zeros, block_exists, !whole_block, !initial_block */
    case 13: /* all_zeros, block_exists, !whole_block, initial_block */
        write_data = true;
        break;
    case 14: /* all_zeros, block_exists, whole_block, !initial_block */
        delete_block = true;
        break;
    case 15: /* all_zeros, block_exists, whole_block, initial_block */
        /* no break */
    default:
        return RC ( rcFS, rcFile, rcWriting, rcConstraint, rcViolated );
    }

    /* follow steps */
    if ( alloc_block )
    {
        uint64_t new_dense_block_id;

        assert ( ! block_exists );
        rc = KSparseFileAllocDenseBlock_v1 ( self, & new_dense_block_id );
        if ( rc != 0 )
            return rc;

        dense_block_id = new_dense_block_id;
    }

    /* determine the dense position of block */
    dense_pos = ( dense_block_id - 1 ) * self -> block_size + self -> dense_start;

    /* fill left part of dense block with zeros */
    if ( fill_left )
    {
        /* TBD - do any file systems exist that would leave
           unwritten portions of a file as anything but zero?
           if not, then this is completely unnecessary */
        rc = KSparseFileZeroDense_v1 ( self, dense_pos, offset, tm );
    }
    if ( rc == 0 )
    {
        /* write data to the block */
        if ( write_data )
            rc = KFileTimedWriteExactly_v1 ( self -> dense, dense_pos + offset, buffer, bsize, tm );
        else if ( delete_block )
        {
            KSparseFileDeleteDenseBlock_v1 ( self, dense_block_id, tm );
            dense_block_id = 0;
        }

        if ( rc == 0 )
        {
            if ( fill_right )
            {
                size_t fill_bytes;

                /* TBD - do any file systems exist that would leave
                   unwritten portions of a file as anything but zero?
                   if not, then this is completely unnecessary */
                fill_bytes = self -> block_size - bsize - offset;
                rc = KSparseFileZeroDense_v1 ( self, dense_pos + offset + bsize, fill_bytes, tm );
            }
            if ( rc == 0 )
            {
                uint64_t logical_size;

                if ( alloc_block || delete_block )
                {
                    switch ( self -> var )
                    {
                    case smapU8:
                        self -> map . u8 [ log_block_id ] = ( uint8_t )  dense_block_id;
                        break;
                    case smapU16:
                        self -> map . u16 [ log_block_id ] = ( uint16_t ) dense_block_id;
                        break;
                    case smapU32:
                        self -> map . u32 [ log_block_id ] = ( uint32_t ) dense_block_id;
                        break;
                    case smapU64:
                        self -> map . u64 [ log_block_id ] = ( uint64_t ) dense_block_id;
                        break;
                    }
                }

                logical_size = log_block_id * self -> block_size + offset + bsize;
                if ( logical_size > self -> logical_size )
                    self -> logical_size = logical_size;

                /* done */
                return 0;
            }
        }
    }

    /* having arrived here, need to undo what has been done */
    if ( alloc_block )
        KSparseFileDeleteDenseBlock_v1 ( self, dense_block_id, NULL );

    return rc;
}

static
rc_t CC KSparseFileTimedWrite_v1 ( KSparseFile_v1 *self, uint64_t pos,
    const void *buffer, size_t bsize, size_t *num_writ, timeout_t *tm )
{
    rc_t rc = 0;

    size_t total;
    const uint8_t * b;
    uint64_t log_block_id, offset, start, end;

    /* will need to loop over buffer */
    b = buffer;

    /* convert "pos" into a block_id and offset */
    assert ( self -> block_size != 0 );
    log_block_id = pos / self -> block_size;
    offset = pos % self -> block_size;

    /* the end of the logical write region */
    end = pos + bsize;

    /* loop over each affected block */
    for ( total = 0, start = pos - offset; start < end;
           ++ log_block_id, offset = 0, start += self -> block_size )
    {
        /* the number to write in this block.
           can be < block_size for first or last,
           must be exactly block_size in middle */
        uint64_t to_write = end - start - offset;
        if ( to_write > ( uint64_t ) self -> block_size )
            to_write = self -> block_size;

        /* write a number of bytes to this block */
        rc = KSparseFileWriteBlock_v1 ( self, log_block_id, ( size_t ) offset,
            b + total, ( size_t ) to_write, tm );

        /* break if not all bytes were written */
        if ( rc == 0 )
            break;

        /* incorporate all bytes into count */
        total += ( size_t ) to_write;
    }

    /* record total number written */
    assert ( num_writ != NULL );
    * num_writ = total;

    /* if any were written, there can be no error */
    if ( total != 0 )
        return 0;

    return rc;
}

static
rc_t CC KSparseFileWrite_v1 ( KSparseFile_v1 *self, uint64_t pos, 
    const void *buffer, size_t bsize, size_t *num_writ )
{
    return KSparseFileTimedWrite_v1 ( self, pos, buffer, bsize, num_writ, NULL );
}

static
uint32_t CC KSparseFileType_v1 ( const KSparseFile_v1 * self )
{
    return kfdFile;
}

static const KFile_vt_v1 vtKSparseFile_v1 =
{
    /* version */
    1, 2,

    /* 1.0 */
    KSparseFileWhack_v1,
    KSparseFileGetSysFile_v1,
    KSparseFileRandomAccess_v1,
    KSparseFileSize_v1,
    KSparseFileSetSize_v1,
    KSparseFileRead_v1,
    KSparseFileWrite_v1,

    /* 1.1 */
    KSparseFileType_v1,

    /* 1.2 */
    KSparseFileTimedRead_v1,
    KSparseFileTimedWrite_v1

};



/* AddRef
 * Release
 *  for KFile_v1
 *  resistant to NULL self references
 */
LIB_EXPORT rc_t CC KSparseFileAddRef_v1 ( const KSparseFile_v1 * self )
{
    return KFileAddRef_v1 ( & self -> dad );
}

LIB_EXPORT rc_t CC KSparseFileRelease_v1 ( const KSparseFile_v1 * self )
{
    return KFileRelease_v1 ( & self -> dad );
}


/* ToKFile
 *  cast the sparse file to a normal KFile
 *  returns a new reference to the KFile
 */
LIB_EXPORT rc_t CC KSparseFileToKFile_v1 ( const KSparseFile_v1 * self, KFile_v1 ** file )
{
    rc_t rc = 0;

    if ( file == NULL )
        rc = RC ( rcFS, rcFile, rcCasting, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcFS, rcFile, rcCasting, rcSelf, rcNull );
        else
        {
            rc = KFileAddRef_v1 ( & self -> dad );
            if ( rc == 0 )
            {
                * file = ( KFile_v1 * ) & self -> dad;
                return 0;
            }
        }

        * file = NULL;
    }

    return rc;
}

/* InitFromDenseFile
 *  if the supplied dense file is not empty,
 *  finish initialization from the dense file
 */
static
rc_t KSparseFileInitFromDenseFile ( KSparseFile_v1 * self )
{
    uint64_t deof;
    rc_t rc = KFileSize_v1 ( self -> dense, & deof );
    if ( rc == 0 && deof != 0 )
    {
        /* TBD */
        rc = -1;
    }

    return rc;
}


/* Make
 *  make a sparse file from an existing KFile
 *  the existing file must be either empty or already a sparse file
 *
 *  "sparse" [ OUT ] - return parameter for sparse file
 *
 *  "dense" [ IN ] - backing file for dense data. must either be empty
 *   or a variable-size sparse file with the same block size
 *
 *  "block_size" [ IN ] - size of file blocks, e.g. 128 * 1024.
 *   must be an even power of 2, e.g. 0001 0010 0100 1000
 */
LIB_EXPORT rc_t CC KSparseFileMake_v1 ( KSparseFile_v1 ** sparse, struct KFile_v1 * dense, size_t block_size )
{
    rc_t rc;

    if ( sparse == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcSelf, rcNull );
    else
    {
        if ( dense == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );

        /* must have read capability */
        else if ( ! dense -> read_enabled )
        {
            if ( dense -> write_enabled )
                rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcWriteonly );
            else
                rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNoPerm );
        }

        /* require block size to be non-zero, even power of two */
        else if ( block_size == 0 || ( block_size & ( block_size - 1 ) ) != 0 )
            rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );

        else
        {
            /* ALL OF THE PARAMETERS APPEAR OKAY. NOW EXAMINE THE DENSE FILE */

            /* the dense file MUST be random-access */
            rc = KFileRandomAccess_v1 ( dense );
            if ( rc == 0 )
            {
                /* we expect the dense file to be a normal file.
                   in particular, can't deal with FIFO or socket,
                   and in the case of a block device, we'd have to
                   assume the block size of the device, which will
                   be for later. */
                uint32_t file_type = KFileType_v1 ( dense );
                switch ( file_type )
                {
                case kfdNull:
                case kfdInvalid:
                    rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcIncorrect );
                    break;
                case kfdFile:
                case kfdCharDev:
                    break;
                case kfdBlockDev:
                case kfdFIFO:
                case kfdSocket:
                    rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcIncorrect );
                    break;
                default:
                    rc = RC ( rcFS, rcFile, rcConstructing, rcType, rcUnrecognized );
                }

                if ( rc == 0 )
                {
                    KSparseFile *spF = calloc ( 1, sizeof * spF );
                    if ( spF == NULL )
                        rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
                    else
                    {
                        rc = KFileInit_v1 ( &spF -> dad, ( const KFile_vt * ) &vtKSparseFile_v1, 
                            "KSparseFile", "no-name", true, dense -> write_enabled );
                        if ( rc == 0 )
                        {
                            rc = KFileAddRef ( dense );
                            if ( rc == 0 )
                            {
                                spF -> dense = dense;
                                spF -> block_size = block_size;

                                rc = KSparseFileInitFromDenseFile ( spF );
                                if ( rc == 0 )
                                {
                                    *sparse = spF;
                                    return 0;
                                }

                                KFileRelease ( spF -> dense );
                                spF -> dense = NULL;
                            }
                        }
                    }

                    free ( spF );
                }
            }
        }

        * sparse = NULL;
    }

    return rc;
}

/* MakeFixed
 *  make a fixed-size sparse file from an existing KFile
 *  the existing file must be either empty or already a fixed-size sparse file of same size
 *
 *  "sparse" [ OUT ] - return parameter for sparse file
 *
 *  "dense" [ IN ] - backing file for dense data. must either be empty
 *   or a fixed-size sparse file with the same logical and block sizes
 *
 *  "block_size" [ IN ] - size of file blocks, e.g. 128 * 1024.
 *   must be an even power of 2.
 *
 *  "logical_size" [ IN ] - end of file in bytes.
 *   not required to be power of 2.
 */
LIB_EXPORT rc_t CC KSparseFileMakeFixed_v1 ( KSparseFile_v1 ** sparse, struct KFile_v1 * dense,
    size_t block_size, uint64_t logical_size )
{
    rc_t rc;

    if ( sparse == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcSelf, rcNull );
    else
    {
        if ( dense == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );

        /* must have read capability */
        else if ( ! dense -> read_enabled )
        {
            if ( dense -> write_enabled )
                rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcWriteonly );
            else
                rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNoPerm );
        }

        /* require block size to be non-zero, even power of two */
        else if ( block_size == 0 || ( block_size & ( block_size - 1 ) ) != 0 )
            rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );

        /* require a non-zero logical size */
        else if ( logical_size == 0 )
            rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );

        else
        {
            /* ALL OF THE PARAMETERS APPEAR OKAY. NOW EXAMINE THE DENSE FILE */

            /* the dense file MUST be random-access */
            rc = KFileRandomAccess_v1 ( dense );
            if ( rc == 0 )
            {
                /* we expect the dense file to be a normal file.
                   in particular, can't deal with FIFO or socket,
                   and in the case of a block device, we'd have to
                   assume the block size of the device, which will
                   be for later. */
                uint32_t file_type = KFileType_v1 ( dense );
                switch ( file_type )
                {
                case kfdNull:
                case kfdInvalid:
                    rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcIncorrect );
                    break;
                case kfdFile:
                case kfdCharDev:
                    break;
                case kfdBlockDev:
                case kfdFIFO:
                case kfdSocket:
                    rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcIncorrect );
                    break;
                default:
                    rc = RC ( rcFS, rcFile, rcConstructing, rcType, rcUnrecognized );
                }

                if ( rc == 0 )
                {
                    KSparseFile *spF = calloc ( 1, sizeof * spF );
                    if ( spF == NULL )
                        rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
                    else
                    {
                        rc = KFileInit_v1 ( &spF -> dad, ( const KFile_vt * ) &vtKSparseFile_v1, 
                            "KSparseFile", "no-name", true, dense -> write_enabled );
                        if ( rc == 0 )
                        {
                            rc = KFileAddRef ( dense );
                            if ( rc == 0 )
                            {
                                spF -> dense = dense;
                                spF -> block_size = block_size;

                                rc = KSparseFileInitFromDenseFile ( spF );
                                if ( rc == 0 )
                                {
                                    *sparse = spF;
                                    return 0;
                                }

                                KFileRelease ( spF -> dense );
                                spF -> dense = NULL;
                            }
                        }
                    }

                    free ( spF );
                }
            }
        }

        * sparse = NULL;
    }

    return rc;
}
