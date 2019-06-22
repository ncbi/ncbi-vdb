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
 */

#include <kfs/extern.h>

struct KCacheTee2File;
#define KFILE_IMPL struct KCacheTee2File
#include <kfs/impl.h>

#include <klib/rc.h>
#include <klib/log.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/time.h>

#include <kfs/recorder.h>
#include "poolpages.h"
#include <kproc/queue.h>
#include <kproc/timeout.h>
#include <kfs/cachetee2file.h>

#include <kfs/defs.h>

#include <sysalloc.h>
#include <stdlib.h>

/*--------------------------------------------------------------------------
 layout of local file:
 
 CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC MMMMMMMMMMMMMM SS BB

 C ... file content in valid/invalid blocks
 M ... bitmap of blocks bytes = ( ( ( content-size / block-size ) + 1 ) / 8 ) + 1 )
 S ... size of content ( uint64_t ) 8 bytes
 B ... used blocksize  ( uint32_t ) 4 bytes
 */

/*--------------------------------------------------------------------------
 * KCacheTeeFile2
 */

#define CACHE_TEE_DEFAULT_BLOCKSIZE ( 128 * 1024 )
#define CACHE_TEE_MIN_BLOCKSIZE ( 16 * 1024 )
#define READ_BLOCK_SIZE_TRYS 3
#define READ_CONTENT_SIZE_TRYS 3

typedef struct KCacheTee2File
{
    KFile dad;
    const KFile * wrapped;                  /* the file we are wrapping */
    KFile * cache;                          /* the cache-file */
    KDirectory * dir;                       /* we have to store a KDirectory because we need it at closing the file,
                                               where we test for promoting the cache */

    uint64_t wrapped_size;                  /* the size of the wrapped file */
    uint64_t cache_size;                    /* the size of the local cache file ( remote_size + bitmap + tail ) */
    uint64_t block_count;                   /* how many blocks do we need to cache the remote file ( last block may be shorter ) */
    uint64_t bitmap_bytes;                  /* how many bytes do we need to store the bitmap */
    
    atomic32_t * bitmap;                    /* the bitmap: each bit represents one block */
    
    KQueue * scratch_pool;                  /* this is necessary to make KCacheTeeFile thread-safe! */
    struct ThePool * pool;                  /* have a cache to answer from RAM! */
    
    uint32_t block_size;                    /* how big is a block ( aka 1 bit in the bitmap )*/

    bool read_only;
    char cache_path [ 1 ];                  /* stores the path to the local cache, for eventual promoting at close */
} KCacheTee2File;


#if __BYTE_ORDER == __LITTLE_ENDIAN
#define SWAP_FN(val) (val)
#else
#define SWAP_FN(val) \
    (((val)>>24)&0xff) | /* move byte 3 to byte 0 */ \
    (((val)<<8)&0xff0000) | /* move byte 1 to byte 2 */ \
    (((val)>>8)&0xff00) | /* move byte 2 to byte 1 */ \
    (((val)<<24)&0xff000000) /* byte 0 to byte 3 */
#endif
#define GEN_BIT_NR_MASK_ROW(i) SWAP_FN( 1 << ( (i) * 4 ) ), SWAP_FN( 1 << ( (i) * 4 + 1 ) ), SWAP_FN( 1 << ( (i) * 4 + 2 ) ), SWAP_FN( 1 << ( (i) * 4 + 3 ) )

const uint32_t BitNr2Mask_2[ 32 ] =
{
    GEN_BIT_NR_MASK_ROW(0),
    GEN_BIT_NR_MASK_ROW(1),
    GEN_BIT_NR_MASK_ROW(2),
    GEN_BIT_NR_MASK_ROW(3),
    GEN_BIT_NR_MASK_ROW(4),
    GEN_BIT_NR_MASK_ROW(5),
    GEN_BIT_NR_MASK_ROW(6),
    GEN_BIT_NR_MASK_ROW(7)
};
#undef SWAP_FN
#undef GEN_BIT_NR_MASK_ROW

#define IS_CACHE_BIT( CacheFile, Block_Nr ) \
    ( ( atomic32_read ( & ( CacheFile )->bitmap[ (Block_Nr) >> 5 ] ) & BitNr2Mask_2[ (Block_Nr) & 31 ] ) > 0 )

#define IS_BITMAP_BIT( BitMap, Block_Nr ) \
    ( ( atomic32_read ( & ( BitMap )[ (Block_Nr) >> 5 ] ) & BitNr2Mask_2[ (Block_Nr) & 31 ] ) > 0 )

#define BITS_2_BYTES( BitCount ) ( ( ( BitCount ) + 7 ) >> 3 )
#define SIZE_2_BLOCK_COUNT( Number_Of_Bytes, Block_Size ) ( ( ( Number_Of_Bytes ) + ( Block_Size ) - 1 ) / ( Block_Size ) )

/* called from: initialize_existing_cache_tee(), initialize_new_cache_tee(), GetCacheTee2FileCompleteness() */
static rc_t create_bitmap_buffer( atomic32_t ** bitmap, uint64_t bitmap_bytes )
{
    rc_t rc = 0;
    *bitmap = calloc ( sizeof **bitmap, ( bitmap_bytes + sizeof ** bitmap - 1 ) / sizeof ** bitmap );
    if ( *bitmap == NULL )
    {
        rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
        LOGERR( klogErr, rc, "init local bitmap-area" );
    }
    return rc;
}

/* called from: IsCacheFileComplete() and IsCacheTeeComplete() */
static bool is_bitmap_full( const atomic32_t * bitmap, uint64_t bitmap_bytes, uint64_t block_count )
{
    uint64_t bitmap_word;
    const uint64_t bitmap_words_minus_one = ( ( bitmap_bytes + sizeof * bitmap - 1 ) >> 2 ) - 1;
    for( bitmap_word = 0; bitmap_word < bitmap_words_minus_one; ++ bitmap_word )
    {
        if ( ~ atomic32_read ( & bitmap [ bitmap_word ] ) != 0 )
            return false;
    }

    {
        uint64_t block_id = ( bitmap_word << 5 );
        while ( block_id < block_count )
        {
            bool block_cached = IS_BITMAP_BIT( bitmap, block_id );
            if ( !block_cached )
                return false;
            ++block_id;
        }
    }
    return true;
}

/* called from: initialize_new_cache_tee() */
static rc_t write_bitmap_and_tail( struct KFile * f,
                                   const atomic32_t * bitmap,
                                   uint64_t to_wrap_size,
                                   uint64_t bitmap_bytes,
                                   uint32_t blocksize )
{
    size_t written;
    uint64_t pos = to_wrap_size;
    
    /* write the bitmap out */
    rc_t rc = KFileWriteAll ( f, pos, ( const void * ) bitmap, bitmap_bytes, &written );
    if ( rc == 0 && written != bitmap_bytes )
    {
        rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );
    }
    if ( rc != 0 )
    {
        LOGERR( klogErr, rc, "cannot write bitmap" );
    }
    
    /* write the size of the to be wrapped file out */
    if ( rc == 0 )
    {
        pos += written;
        rc = KFileWriteAll ( f, pos, &to_wrap_size, sizeof ( to_wrap_size ), &written );
        if ( rc == 0 && written != sizeof ( to_wrap_size ) )
        {
            rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );
        }
        if ( rc != 0  )
        {
            LOGERR( klogErr, rc, "cannot write size of to be wrapped file" );
        }
    }
    
    /* write the blocksize at the end of the cache-file */
    if ( rc == 0 )
    {
        pos += written;
        rc = KFileWriteAll ( f, pos, &blocksize, sizeof ( blocksize ), &written );
        if ( rc == 0 && written != sizeof ( blocksize ) )
        {
            rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );
        }
        if ( rc != 0  )
        {
            LOGERR( klogErr, rc, "cannot write blocksize" );
        }
    }
    return rc;
}

/* called from:  initialize_existing_cache_tee(), TruncateCacheTee2File(),
    GetCacheTee2FileCompleteness(), GetCacheTee2FileTruncatedSize() */
static rc_t read_block_size ( const struct KFile * self, uint64_t cache_size, uint32_t *block_size )
{
    if ( cache_size >= sizeof *block_size )
    {
        uint64_t pos = cache_size - ( sizeof *block_size );
        int num_try = READ_BLOCK_SIZE_TRYS;
        rc_t rc;

        while ( true )
        {
            size_t num_read;
            rc =  KFileRead ( self, pos, block_size, sizeof *block_size, &num_read );
            if ( rc == 0 )
            {
                if ( num_read != sizeof *block_size )
                    rc = SILENT_RC ( rcFS, rcFile, rcValidating, rcParam, rcInvalid );
            }

            if ( rc == 0 && *block_size != 0 )
                // we are done
                return 0;
            
            if ( --num_try == 0 )
                break;
            
            KSleep( 1 );
        }
        
        if ( rc != 0 )
            return rc;
    }
    return RC ( rcFS, rcFile, rcValidating, rcParam, rcInvalid );
}

/* called from: initialize_existing_cache_tee(), TruncateCacheTee2File(),
    GetCacheTee2FileCompleteness(), GetCacheTee2FileTruncatedSize() */
static rc_t read_content_size ( const struct KFile * self, uint64_t cache_size, uint64_t *content_size )
{
    if ( cache_size >= sizeof( *content_size ) + 4 )
    {
        uint64_t pos = ( cache_size - 4 ) - sizeof( *content_size );
        int num_try = READ_CONTENT_SIZE_TRYS;
        rc_t rc;

        while ( true )
        {
            size_t num_read;
            rc =  KFileRead ( self, pos, content_size, sizeof *content_size, &num_read );
            if ( rc == 0 )
            {
                if ( num_read != sizeof *content_size )
                    rc = SILENT_RC ( rcFS, rcFile, rcValidating, rcParam, rcInvalid );
            }
        
            if ( rc == 0 && *content_size != 0 )
            {
                if ( *content_size < cache_size )
                    return 0;
                else
                    return RC ( rcFS, rcFile, rcValidating, rcParam, rcInvalid );
            }
            
            if ( --num_try == 0 )
                break;
            
            KSleep( 1 );
        }
        
        if ( rc != 0 )
            return rc;
    }
    return RC ( rcFS, rcFile, rcValidating, rcParam, rcInvalid );
}

/* called from: TruncateCacheTee2File(), GetCacheTee2FileCompleteness(), GetCacheTee2FileTruncatedSize() */
static rc_t verify_file_structure ( const uint64_t cache_size, const uint32_t block_size, const uint64_t content_size,
                                   uint64_t * block_count, uint64_t * bitmap_bytes )
{
    rc_t rc = 0;
    uint64_t expected_size;

    *block_count = SIZE_2_BLOCK_COUNT( content_size, block_size );
    *bitmap_bytes = BITS_2_BYTES( *block_count );

    /* check if the values 'content-size' and 'block_size' result in the currect real file size */
    expected_size = content_size + *bitmap_bytes + sizeof ( cache_size ) + sizeof ( block_size );
    if ( expected_size != cache_size )
        rc = RC ( rcFS, rcFile, rcValidating, rcParam, rcInvalid );
    return rc;
}

static rc_t switch_to_read_only ( const KCacheTee2File *cself, rc_t rc, int tag );

/* called from: KCacheTee2FileDestroy() */
static rc_t promote_cache ( KCacheTee2File * self )
{
    char cache_file_name [ 4096 ];
    char temp_file_name [ 4096 ];
    size_t num_writ;
    rc_t rc = string_printf ( cache_file_name, sizeof cache_file_name, &num_writ, "%s.cache", self -> cache_path );
    if ( rc == 0 )
        rc = string_printf ( temp_file_name, sizeof temp_file_name, &num_writ, "%s.cache.temp", self -> cache_path );

    /* (1) releaes open cache file ( windows cannot rename open files ) */
    if ( rc == 0 )
        rc = KFileRelease( self -> cache );

    /* (2) rename to temporary name */
    if ( rc == 0 )
    {
        self -> cache = NULL;
        rc = KDirectoryRename ( self -> dir, true, cache_file_name, temp_file_name );
    }

    /* (3) re-open from temporary name */
    if ( rc == 0 )
        rc = KDirectoryOpenFileWrite( self -> dir, &self -> cache, true, "%s", temp_file_name );

    /* (4) perform truncation */
    if ( rc == 0 )
        rc = TruncateCacheTee2File( self -> cache );
    
    /* (5) release open temp. cache file ( windows cannot rename open files ) */
    if ( rc == 0 )
        rc = KFileRelease( self -> cache );

    /* (6) rename to final filename ( windows cannot rename open files ) */
    if ( rc == 0 )
    {
        self -> cache = NULL;
        rc = KDirectoryRename ( self -> dir, true, temp_file_name, self -> cache_path );
    }

    /* (6) re-open from final filename */
    if ( rc == 0 )
        rc = KDirectoryOpenFileWrite( self -> dir, &self -> cache, true, "%s", self -> cache_path );

    return rc;
}

/* called from: KCacheTee2FileDestroy() */
static bool file_exist( KDirectory * dir, const char * filename )
{
    uint32_t pt = KDirectoryPathType ( dir, "%s", filename );
    return ( ( pt & ~kptAlias ) == kptFile );
}

static void * pop_page( KQueue * buffer_pool, uint32_t timeout_millisec )
{
    rc_t rc;
    void * page;
    struct timeout_t tm;
    TimeoutInit ( & tm, timeout_millisec );
    rc = KQueuePop( buffer_pool, &page, &tm );
    if ( rc != 0 )
        page = NULL;
    return page;
}

/* helper to clean up the buffer_pool */
static void clean_up_pool( KQueue * buffer_pool )
{
    void * pool_page;
    while ( ( pool_page = pop_page( buffer_pool, 100 ) ) != NULL )
    {
        free( pool_page );
    }
    KQueueRelease( buffer_pool );
}

/* Destroy ( entered into vtKCacheTee2File-struct )
 */
static rc_t CC KCacheTee2FileDestroy( KCacheTee2File * self )
{
    bool already_promoted_by_other_instance = file_exist( self -> dir, self -> cache_path );
    
    if ( !self -> read_only && !already_promoted_by_other_instance )
    {
        bool fully_in_cache;
        rc_t rc = IsCacheTee2FileComplete ( self -> cache, &fully_in_cache );
        if ( rc == 0 && fully_in_cache )
        {
            promote_cache( self );
        }
    }

    if ( self->bitmap != NULL )
        free( ( void * ) self->bitmap );

    if ( self -> scratch_pool != NULL )
        clean_up_pool( self -> scratch_pool );

    if ( self -> pool != NULL )
        pool_release ( self -> pool );
    
    KFileRelease ( self -> wrapped );
    KFileRelease ( self -> cache );

    if ( already_promoted_by_other_instance )
        KDirectoryRemove ( self -> dir, true, "%s.cache", self -> cache_path );

    KDirectoryRelease ( self -> dir );
    
    free ( self );
    return 0;
}

/* called from: KCacheTee2FileRead() */
static void set_bitmap ( atomic32_t * bitmap, uint64_t start_block, uint64_t block_count )
{
    uint64_t count;
    uint64_t a_block = start_block;
    for ( count = 0; count < block_count; count++ )
    {
        uint32_t old, bits;
        const uint32_t block_bit = BitNr2Mask_2 [ a_block & 31 ];

        old = atomic32_read ( &( bitmap [ a_block >> 5 ] ) );
        do
        {
            bits = old;
            old = atomic32_test_and_set ( &( bitmap [ a_block >> 5 ] ), ( int ) ( bits | block_bit ), ( int ) bits );
        }
        while ( old != bits );
        a_block++;
    }
}

/* called from: KCacheTee2FileRead()*/
static rc_t write_bitmap ( const KCacheTee2File *cself, uint64_t start_block, uint32_t count )
{
    rc_t rc;
    size_t written;
    uint64_t pos;
    size_t to_write;

    uint32_t block_word_1 = ( uint32_t ) ( start_block >> 5 );
    uint32_t block_word_n = ( uint32_t ) ( ( start_block + count ) >> 5 );
    uint64_t bitmap_pos = ( block_word_1 << 2 );
    pos = cself -> wrapped_size + bitmap_pos;
    to_write = ( block_word_n - block_word_1 + 1 ) * 4;

    // last word may go outside bitmap ranges in the file, since bitmap has 1-byte alignment in the file, but 4-byte alignment in memory
    if ( bitmap_pos + to_write > cself->bitmap_bytes )
        to_write = cself->bitmap_bytes - bitmap_pos;

    rc = KFileWriteAll ( cself -> cache, pos, ( const void * ) &cself->bitmap[ block_word_1 ], to_write, &written );
    if ( rc != 0 )
    {
        /* it can happen that we are not able to write to the bitmap because we run out of space
           on the local filesystem. */
        rc = switch_to_read_only( cself, rc, 1 );
    }
    return rc;
}


typedef struct block_span
{
    uint64_t first;
    uint64_t last;
    uint64_t count;
} block_span;

typedef struct read_info
{
    /* what is consecutively available at the beginning of the caller request */
    block_span available;
    
    /* where the available span starts */
    uint64_t first_block_pos;
    
    /* how many bytes are in the available span */
    uint64_t bytes_to_read;

    /* is the available block_span in cache ? */
    bool in_cache;

    struct PoolPage * pp;
} read_info;


/* cself ... the KCacheTee2File ( used: block_size, block_count, wrapped_size, bitmap )
   pos ..... zero-based offset into the file ( unmodified from caller )
   len ..... length of slice ( adjusted to not hang over the end, caller does not allow zero )
   info .... the structure above, telling the caller what is available in cache
*/
static void get_read_info ( const KCacheTee2File *cself, uint64_t pos, size_t len, read_info * info ) 
{
    block_span request;
    bool consecutiv = true;
    bool last_block_incomplete;
    
    request . first = ( pos / cself -> block_size );
    request . last  = ( ( pos + len - 1 ) / cself -> block_size );

    last_block_incomplete = ( request . last >= cself -> block_count );
    if ( last_block_incomplete )
        request . last = ( cself -> block_count - 1 );
    
    request . count = ( request . last - request . first ) + 1;

    info -> available . first = request . first;
    info -> available . last  = request . first;
    info -> available . count = 1;
    
    info -> in_cache = IS_CACHE_BIT( cself, info -> available . first );
    while ( consecutiv && ( info -> available . count < request . count ) )
    {
        bool b = IS_CACHE_BIT( cself, info -> available . last + 1 );
        consecutiv = ( info -> in_cache == b );
        if ( consecutiv )
        {
            info -> available . last += 1;
            info -> available . count += 1;
        }
    }
    
    info -> first_block_pos = ( info -> available . first * cself -> block_size );
    
    if ( last_block_incomplete )
        info -> bytes_to_read = ( ( cself -> wrapped_size + 1 ) - info -> first_block_pos );
    else
        info -> bytes_to_read = ( info -> available . count * cself -> block_size );

    info -> pp = NULL;
    if ( info -> in_cache )
    {
        info -> bytes_to_read -= ( pos - info -> first_block_pos );
        if ( info -> bytes_to_read > len )
            info -> bytes_to_read = len;
            
        pool_page_find ( cself -> pool, &( info -> pp ), pos );
    }
}

static rc_t KCacheTee2FileRead_from_wrapped_using_page ( const KCacheTee2File *cself, struct PoolPage * pp,
        uint64_t pos, void * buffer, size_t bsize, size_t *num_read, read_info * info )
{
    /* we have control of the page and can make as much buffer as we need to */
    rc_t rc = pool_page_prepare( pp, info -> available . count, info -> first_block_pos );
    if ( rc == 0 )
    {
        size_t from_wrapped;
        
        /* read from the wrapped file into the page-buffer */
        rc = pool_page_read_from_file( pp, cself -> wrapped, &from_wrapped );
        if ( rc == 0 )
        {
            size_t num_written_to_cache;
            /* write the buffer into the local cache -file */
            rc = pool_page_write_to_file( pp, cself -> cache, from_wrapped, &num_written_to_cache );
            if ( rc != 0 || num_written_to_cache != from_wrapped )
            {
                /* switch to read-only, because for some reason we cannot write any more...
                   it can happen that we are not able to write to the bitmap because we run out of space
                   on the local filesystem. */
                rc = switch_to_read_only( cself, rc, 2 );
                PLOGERR( klogInt,
                         ( klogInt,
                           rc,
                          "read: $(read), written:$(written)",
                          "read=%lu,written=%lu",
                           from_wrapped, num_written_to_cache ) );
            }
            else
            {
                /* set the block-bits in the bitmap... */
                uint32_t pp_blocks = pool_page_blocks( pp );
                set_bitmap ( cself -> bitmap, info -> available . first, pp_blocks );
                rc = write_bitmap ( cself, info -> available . first, pp_blocks );
                if ( rc != 0 )
                {
                    /* switch to read-only, because for some reason we cannot write any more... */
                    rc = switch_to_read_only( cself, rc, 3 );
                }
            }
        }
        if ( rc == 0 )
            rc = pool_page_get ( pp, pos, buffer, bsize, num_read );

    }
    return rc;
}

static rc_t KCacheTee2FileRead_from_cache_using_page ( const KCacheTee2File *cself, struct PoolPage * pp,
        uint64_t pos, void * buffer, size_t bsize, size_t *num_read, read_info * info )
{
    rc_t rc = pool_page_prepare( pp, info -> available . count, info -> first_block_pos );
    if ( rc == 0 )
    {
        size_t from_cache;
        rc = pool_page_read_from_file( pp, cself -> cache, &from_cache );
        if ( rc == 0 )
            rc = pool_page_get ( pp, pos, buffer, bsize, num_read );
    }
    return rc;
}

/* */
static rc_t KCacheTee2FileRead_rw_using_caller_buffer ( const KCacheTee2File *cself,
        uint64_t pos, void *buffer, size_t *num_read, read_info * info )
{
    rc_t rc;
    
    /* read whole blocks from the wrapped file */
    rc = KFileReadAll ( cself -> wrapped, info -> first_block_pos, buffer, info -> bytes_to_read, num_read );
    if ( rc == 0 )
    {
        /* store them in the cache-file... */
        size_t num_written_to_cache;
        rc = KFileWriteAll ( cself -> cache, info -> first_block_pos, buffer, *num_read, &num_written_to_cache );
        if ( rc != 0 || num_written_to_cache != *num_read )
        {
            /* switch to read-only, because for some reason we cannot write any more...
               it can happen that we are not able to write to the bitmap because we run out of space
               on the local filesystem. */
            rc = switch_to_read_only( cself, rc, 4 );
        }
        else
        {
            /* set the block-bits in the bitmap... */
            set_bitmap ( cself -> bitmap, info -> available . first, info -> available . count );
            rc = write_bitmap ( cself, info -> available . first, info -> available . count );
            if ( rc != 0 )
            {
                /* switch to read-only, because for some reason we cannot write any more... */
                rc = switch_to_read_only( cself, rc, 5 );
            }
        }
    }
    if ( rc == 0 )
    {
        /* now we have to shift the given buffer into place... */
        uint64_t shift_by = ( pos - info -> first_block_pos );
        if ( shift_by > 0 )
        {
            uint8_t * src = buffer;
            src += shift_by;
            *num_read = ( info -> bytes_to_read - shift_by );
            memmove( buffer, src, *num_read );
        }
    }
    return rc;
}

static rc_t KCacheTee2FileRead_rw_using_scratch_buffer ( const KCacheTee2File *cself,
        uint64_t pos, void *buffer, size_t bsize, size_t *num_read, read_info * info )
{
    rc_t rc = 0;
    void * page = pop_page( cself -> scratch_pool, 100 );
    
    if ( page == NULL )
        page = malloc ( cself -> block_size );
    if ( page == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
    else
    {
        /* read one block from the wrapped file */
        rc = KFileReadAll ( cself -> wrapped, info -> first_block_pos, page, cself -> block_size, num_read );
        if ( rc == 0 )
        {
            /* store it in the cache-file... */
            size_t num_written;
            rc = KFileWriteAll ( cself -> cache, info -> first_block_pos, page, *num_read, &num_written );
            if ( rc != 0 || num_written != *num_read )
            {
                /* switch to read-only, because for some reason we cannot write any more...
                   it can happen that we are not able to write to the bitmap because we run out of space
                   on the local filesystem. */
                rc = switch_to_read_only( cself, rc, 6 );
            }
            else
            {
                /* set the block-bit in the bitmap... */
                set_bitmap ( cself -> bitmap, info -> available . first, 1 );
                rc = write_bitmap ( cself, info -> available . first, 1 );
                if ( rc != 0 )
                {
                    /* switch to read-only, because for some reason we cannot write any more... */
                    rc = switch_to_read_only( cself, rc, 7 );
                }
            }
        }
        
        if ( rc == 0 )
        {
            /* now we have to shift the given buffer into place... */
            uint8_t * src = page;
            uint64_t shift_by = ( pos - info -> first_block_pos );
            if ( shift_by > 0 )
            {
                src += shift_by;
                *num_read = ( cself -> block_size - shift_by );
                memmove( buffer, src, *num_read );
            }
            else
            {
                *num_read = bsize;
                memmove( buffer, src, bsize );
            }
        }
        
        if ( KQueuePush ( cself -> scratch_pool, page, NULL ) != 0 )
            free ( page );
    }
    return rc;
}

                                    
/* new strategy: deliver either from cache or wrapped file, maybe less than requested,
   do not use any kind of scratch-buffer to piece blocks together,
   use the caller supplied buffer if possible, only use a scratch-buffer if
   let the caller come back for more... */
static rc_t KCacheTee2FileRead_rw ( const KCacheTee2File *cself, uint64_t pos,
                                    void *buffer, size_t bsize, size_t *num_read )
{
    rc_t rc = 0;
    read_info info;

    *num_read = 0;
    if ( pos > cself -> wrapped_size )
    {
        /* the caller asked for data beyond the end of the file */
        return rc;
    }
    else if ( ( pos + bsize ) > cself -> wrapped_size )
    {
        /* the caller asked for a slice of data reaching beyond the end of the file */
        bsize = cself -> wrapped_size - pos;
    }
    
    /* if the caller asked for nothing... */
    if ( bsize == 0 )
    {
        return rc;
    }
    
    get_read_info ( cself, pos, bsize, &info );

    if ( info . in_cache )
    {
        /* deliver from cache, maybe less than requested, let the caller come back for more... 
        
        request:          |-------------------------
        blocks:         XXXXXX======XXXXXX
        file:     ..........................................
        
        or
        
        request:      |-----------
        blocks:    XXXXXX======XXXXXX
        file:     ..........................................

        or

        request:      |-----------
        blocks:    XXXXXX======XXXXXX
        file:     .................

        */
        if ( info . pp != NULL )
        {
            /* we found it in the memory-cache! ---> GREAT! let's get it from there */
            rc = pool_page_get ( info . pp, pos, buffer, bsize, num_read );
        }
        else
        {
            /* we did not find it in the memory-cache! ---> but we have it in the cache-file...
               let us read it from the cache-file, but store it in the memory-cache */
            struct PoolPage * pp;
            if ( pool_page_find_new ( cself -> pool, &pp ) == 0 && pp != NULL )
            {
                rc = KCacheTee2FileRead_from_cache_using_page ( cself, pp, pos, buffer, bsize, num_read, &info );
                pool_page_release ( pp );
            }
            else
            {
                rc = KFileReadAll( cself -> cache, pos, buffer, info . bytes_to_read, num_read );
            }
        }
    }
    else
    {
        /* the first block is not cached ...
           deliver from the wrapped file, maybe less than requested, store into cache, update bitmap,
           write bitmap, let the caller come back for more...

        request:      |-------------------------
        blocks:    XXXXXX======XXXXXX

        request:      |-----------
        blocks:    XXXXXX======XXXXXX
       
        */
        
        struct PoolPage * pp;
        /* first we try to allocate a pool-page ... */
        if ( pool_page_find_new ( cself -> pool, &pp ) == 0 && pp != NULL )
        {
            rc = KCacheTee2FileRead_from_wrapped_using_page ( cself, pp, pos, buffer, bsize, num_read, &info );
            pool_page_release ( pp );
        }
        else if ( info . bytes_to_read <= bsize )
        {
            /* we do have enough caller buffer to do the whole thing... */
            rc = KCacheTee2FileRead_rw_using_caller_buffer ( cself, pos, buffer, num_read, &info );
        }
        else
        {
            /* we do NOT have enough caller buffer to do the whole thing...
               reduce it to make it fit into the bsize
            */
            info . available . count = ( bsize / cself -> block_size );
            if ( info . available . count > 0 )
            {
                /* we have been given enough buffer from the caller to handle all blocks... */
                info . bytes_to_read = ( info . available . count * cself -> block_size );
                rc = KCacheTee2FileRead_rw_using_caller_buffer ( cself, pos, buffer, num_read, &info );
            }
            else
            {
                /* we do not event have enough buffer from the caller to handle 1 block... */
                rc = KCacheTee2FileRead_rw_using_scratch_buffer ( cself, pos, buffer, bsize, num_read, &info );
            }
        }
    }
    
    if ( info . pp != NULL )
        pool_page_release ( info . pp );

    return rc;
}


/* this is the read-only-version of the strategy: if the first block is in cache - it is the
   same as the r/w-version, if it is not we deliver directly from the wrapped file */
static rc_t KCacheTee2FileRead_ro ( const KCacheTee2File *cself, uint64_t pos,
                                    void *buffer, size_t bsize, size_t *num_read )
{
    rc_t rc = 0;
    read_info info;

    *num_read = 0;
    
    if ( pos > cself -> wrapped_size )
        return rc;
    else if ( ( pos + bsize ) > cself -> wrapped_size )
        bsize = cself -> wrapped_size - pos;

    if ( bsize == 0 )
        return 0;
        
    get_read_info ( cself, pos, bsize, &info );
    
    if ( info . in_cache )
    {
        if ( info . pp != NULL )
        {
            /* we found it in the memory-cache! ---> GREAT! let's get it from there */
            rc = pool_page_get ( info . pp, pos, buffer, bsize, num_read );
        }
        else
        {
            /* we did not find it in the memory-cache! ---> but we have it in the cache-file...
               ( let's see if we can put it into memory )
            */
            struct PoolPage * pp;
            if ( pool_page_find_new ( cself -> pool, &pp ) == 0 )
            {
                rc = KCacheTee2FileRead_from_cache_using_page ( cself, pp, pos, buffer, bsize, num_read, &info );
                pool_page_release ( pp );
            }
            else
                rc = KFileReadAll( cself -> cache, pos, buffer, info . bytes_to_read, num_read );
        }
    }
    else
        rc = KFileReadAll ( cself -> wrapped, pos, buffer, info . bytes_to_read, num_read );

    if ( info . pp != NULL )
        pool_page_release ( info . pp );
    return rc;
}

/**********************************************************************************************
    START vt-functions
**********************************************************************************************/
static struct KSysFile* KCacheTee2FileGetSysFile( const KCacheTee2File *self, uint64_t *offset )
{
    * offset = 0;
    return NULL;
}


static rc_t KCacheTee2FileRandomAccess( const KCacheTee2File *self )
{
    return 0;
}


static rc_t KCacheTee2FileSize( const KCacheTee2File *self, uint64_t *size )
{
    *size = self->wrapped_size;
    return 0;
}


static rc_t KCacheTee2FileSetSize( KCacheTee2File *self, uint64_t size )
{
    return RC ( rcFS, rcFile, rcUpdating, rcFile, rcReadonly );
}


static rc_t KCacheTee2FileWrite( KCacheTee2File *self, uint64_t pos,
                                const void *buffer, size_t size, size_t *num_writ )
{
    return RC ( rcFS, rcFile, rcUpdating, rcInterface, rcUnsupported );
}

/**********************************************************************************************
    END vt-functions
**********************************************************************************************/

static KFile_vt_v1 vtKCacheTee2File_rw =
{
    /* version 1.0 */
    1, 0,

    /* start minor version 0 methods */
    KCacheTee2FileDestroy,
    KCacheTee2FileGetSysFile,
    KCacheTee2FileRandomAccess,
    KCacheTee2FileSize,
    KCacheTee2FileSetSize,
    KCacheTee2FileRead_rw,
    KCacheTee2FileWrite
    /* end minor version 0 methods */
};

static KFile_vt_v1 vtKCacheTee2File_ro =
{
    /* version 1.0 */
    1, 0,

    /* start minor version 0 methods */
    KCacheTee2FileDestroy,
    KCacheTee2FileGetSysFile,
    KCacheTee2FileRandomAccess,
    KCacheTee2FileSize,
    KCacheTee2FileSetSize,
    KCacheTee2FileRead_ro,
    KCacheTee2FileWrite
    /* end minor version 0 methods */
};


static rc_t switch_to_read_only ( const KCacheTee2File *cself, rc_t rc, int tag )
{
    KFile_v1 * p1 = ( KFile_v1 * )cself;
    KFile_vt * p2 = ( KFile_vt * )p1 -> vt;
    p2 -> v1 = vtKCacheTee2File_ro;
    ( ( KCacheTee2File * )cself ) -> read_only = true;
    PLOGERR( klogInt,
             ( klogInt,
               rc,
              "switch_to_read_only( tag:$(tag) )",
              "tag=%d",
               tag ) );
/*    LOGERR( klogInt, rc, "switching cache-tee-file to read-only" ); */
    return 0;
}

static rc_t hand_out_to_wrap_file_as_tee_file( struct KFile const ** tee, struct KFile const * to_wrap )
{
    rc_t rc = KFileAddRef( to_wrap );
    if ( rc != 0 )
    {
        LOGERR( klogErr, rc, "KFileAddRef( file to be wrapped ) failed" );
    }
    else
    {
        *tee = to_wrap;
    }
    return rc;
}

typedef struct cache_tee_params
{
    struct KDirectory * dir;
    struct KFile const * to_wrap;
    struct KFile * cache;
    uint64_t to_wrap_size;
    uint64_t cache_size;
    uint32_t block_size;
    size_t resolved_path_size;
    bool read_only;
    char resolved_path [ 4096 ];    
} cache_tee_params;


static rc_t finish_tee( struct KFile const **tee,
                        const cache_tee_params * ctp,
                        uint64_t block_count,
                        uint64_t bitmap_bytes,
                        atomic32_t * bitmap )
{
    rc_t rc = KDirectoryAddRef ( ctp -> dir );
    if ( rc == 0 )
    {
        rc = KFileAddRef ( ctp -> to_wrap );
        if ( rc == 0 )
        {
            KQueue * q;
            rc = KQueueMake( &q, 32 );
            if ( rc == 0 )
            {
                struct ThePool * pool;
                rc = make_pool ( &pool, ctp -> block_size, 8 );
                if ( rc == 0 )
                {
                    KCacheTee2File * cf = malloc ( sizeof * cf + ctp -> resolved_path_size + 1 );
                    if ( cf == NULL )
                        rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
                    else
                    {
                        /* now we can enter everything into the cf - struct */
                        cf -> wrapped = ctp -> to_wrap;
                        cf -> cache = ctp -> cache;
                        cf -> dir = ctp -> dir;
                        cf -> wrapped_size = ctp -> to_wrap_size;
                        cf -> cache_size = ctp -> cache_size;
                        cf -> block_count = block_count;
                        cf -> bitmap = bitmap;
                        cf -> bitmap_bytes = bitmap_bytes;
                        cf -> scratch_pool = q;
                        cf -> pool = pool;
                        cf -> block_size = ctp -> block_size;
                        cf -> read_only = ctp -> read_only;

                        string_copy ( cf -> cache_path,
                                     ctp -> resolved_path_size + 1,
                                     ctp -> resolved_path,
                                     ctp -> resolved_path_size );

                        if ( ctp -> read_only )
                        {
                            rc = KFileInit ( &cf -> dad,
                                             ( const union KFile_vt * ) &vtKCacheTee2File_ro,
                                             "KCacheTee2File",
                                             ctp -> resolved_path,
                                             true,
                                             false );
                        }
                        else
                        {
                            rc = KFileInit ( &cf -> dad,
                                             ( const union KFile_vt * ) &vtKCacheTee2File_rw,
                                             "KCacheTee2File",
                                             ctp -> resolved_path,
                                             true,
                                             false );
                        }
                        
                        if ( rc != 0 )
                            free( ( void * ) cf );
                        else
                        {
                            /* the wrapper is ready to use now! */
                            *tee = ( const KFile * ) &cf -> dad;
                        }
                    }
                    if ( rc != 0 )
                        pool_release ( pool );
                }
                if ( rc != 0 )
                    KQueueRelease( q );
            }
            if ( rc != 0 )
                KFileRelease ( ctp -> to_wrap );
        }
        if ( rc != 0 )
            KDirectoryRelease ( ctp -> dir );
    }
    return rc;
}

static rc_t initialize_existing_cache_tee ( struct KFile const **tee,
                                            cache_tee_params * ctp,
                                            bool * re_init_cache )
{
    /* first we need to know the size of the cache-file */
    rc_t rc = KFileSize ( ctp -> cache, &( ctp -> cache_size ) );
    if ( rc != 0 )
    {
        LOGERR( klogErr, rc, "cannot detect size of cache-file" );
        *re_init_cache = true;
    }
    else
    {
        /* now let us see what is in the cache-file: what content-size was stored */
        uint64_t content_size;
        rc = read_content_size ( ctp -> cache, ctp -> cache_size, &content_size );
        if ( rc != 0 )
        {
            LOGERR( klogErr, rc, "cannot read content_size" );
            *re_init_cache = true;
        }
        else if ( content_size != ctp -> to_wrap_size )
        {
            /* if it does not match with the size of the wrapped file:
               we should discard the cache file and start a new one! */
            rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );
            PLOGERR( klogErr, ( klogErr, rc, "content-size in local file $(ls) does not match size of remote file $(rs)", 
                               "ls=%lu,rs=%lu", content_size, ctp -> to_wrap_size ) );
            *re_init_cache = true;
        }
        else
        {
            uint32_t block_size;
            rc = read_block_size ( ctp -> cache, ctp -> cache_size, &block_size );
            if ( rc != 0 )
            {
                LOGERR( klogErr, rc, "cannot read block_size" );
                *re_init_cache = true;
            }
            else if ( block_size != ctp -> block_size )
            {
                /* if it does not match with the blocksize the caller requested:
                   we should discard the cache file and start a new one! */
                rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );
                PLOGERR( klogErr, ( klogErr, rc, "block- in local file $(ls) does not match requested value $(rs)", 
                                   "ls=%u,rs=%u", block_size, ctp -> block_size ) );
                *re_init_cache = true;
            }
            else
            {
                /* first we need to know how manny blocks the to-be-wrapped file needs */
                uint64_t block_count  = SIZE_2_BLOCK_COUNT( content_size, block_size );

                /* then we have to calculate how many bytes our bitmap will have */
                uint64_t bitmap_bytes = BITS_2_BYTES( block_count );

                /* new we can calculate the total size of our cache-file */
                uint64_t calculated_cache_size = content_size +
                                    bitmap_bytes +
                                    sizeof( uint64_t ) +
                                    sizeof ( uint32_t );

                if ( calculated_cache_size != ctp -> cache_size )
                {
                    /* if the calculated and the actual cache-size do not match:
                       we should discard the cache file and start a new one! */
                    rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );
                    PLOGERR( klogErr, ( klogErr, rc, "calculated cache-size $(ls) does not match real cache-size $(rs)", 
                                       "ls=%lu,rs=%lu", calculated_cache_size, ctp -> cache_size ) );
                    *re_init_cache = true;
                }
                else
                {
                    atomic32_t * bitmap;
                    /* now make a bitmap-buffer using the size we calculated above */
                    rc = create_bitmap_buffer ( &bitmap, bitmap_bytes );
                    if ( rc != 0 )
                    {
                        LOGERR( klogErr, rc, "cannot create bitmap" );
                    }
                    else
                    {
                        /* now read the bitmap from the cache-file into our buffer */
                        size_t num_read;
                        rc_t rc = KFileReadAll ( ctp -> cache, content_size, ( void * ) bitmap, bitmap_bytes, &num_read );
                        if ( rc != 0 )
                        {
                            LOGERR( klogErr, rc, "cannot read bitmap from cache file" );
                            *re_init_cache = true;
                        }
                        else if ( num_read != bitmap_bytes )
                        {
                            rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );
                            PLOGERR( klogErr, ( klogErr, rc, "cannot read $(ls) bitmap-bytes from cache file, read $(rs) instead", 
                                           "ls=%lu,rs=%lu", bitmap_bytes, num_read ));
                            *re_init_cache = true;
                        }
                        else
                        {
                            /* everything is verified now, finish it by AddRef to everything
                               we hold on */
                            rc = finish_tee( tee,
                                             ctp,
                                             block_count,
                                             bitmap_bytes,
                                             bitmap );
                        }
                        if ( rc != 0 )
                            free( ( void * ) bitmap );
                    }
                }
            }
        }
    }
    return rc;
}

static rc_t initialize_new_cache_tee ( struct KFile const **tee,
                                       cache_tee_params * ctp )
{
    rc_t rc = 0;
    
    /* first we need to know how manny blocks the to-be-wrapped file needs */
    uint64_t block_count  = SIZE_2_BLOCK_COUNT( ctp -> to_wrap_size, ctp -> block_size );

    /* then we have to calculate how many bytes our bitmap will have */
    uint64_t bitmap_bytes = BITS_2_BYTES( block_count );

    /* new we can calculate the total size of our cache-file */
    ctp -> cache_size = ctp -> to_wrap_size +
                        bitmap_bytes +
                        sizeof( uint64_t ) +
                        sizeof ( uint32_t );

    /* now let us set the newly created cache - file to its required size */
    rc = KFileSetSize ( ctp -> cache, ctp -> cache_size );
    if ( rc != 0 )
    {
        LOGERR( klogErr, rc, "cannot set size of new cache-file" );
    }
    else
    {
        atomic32_t * bitmap;
        /* now make a bitmap-buffer using the size we calculated above */
        rc = create_bitmap_buffer ( &bitmap, bitmap_bytes );
        if ( rc != 0 )
        {
            LOGERR( klogErr, rc, "cannot create bitmap" );
        }
        else
        {
            /* write the bitmap and the tail into the cache-file */
            rc = write_bitmap_and_tail ( ctp -> cache,
                                         bitmap,
                                         ctp -> to_wrap_size,
                                         bitmap_bytes,
                                         ctp -> block_size );
            if ( rc == 0 )
            {
                /* finish it by AddRef to everything we hold on
                   and */
                rc = finish_tee( tee,
                                 ctp,
                                 block_count,
                                 bitmap_bytes,
                                 bitmap );
            }
            if ( rc != 0 )
                free( ( void * ) bitmap );
        }
    }
    return rc;
}

/* called if no cache-file existed, or we had a broken one - and removed it */
static rc_t create_new_cachetee( struct KFile const ** tee, cache_tee_params * ctp )
{
    rc_t rc = KDirectoryCreateFile ( ctp -> dir,
                                     & ( ctp -> cache ),
                                     true,
                                     0664,
                                     kcmOpen | kcmParents,
                                     "%s.cache",
                                     ctp -> resolved_path );
    if ( rc != 0 )
    {
        LOGERR( klogErr, rc, "cannot create cache-file" );
    }
    else
    {
        /* we have the exclusive rd/wr access to the cache file !*/
        rc = initialize_new_cache_tee ( tee, ctp ); /* <========== */
        if ( rc != 0 )
        {
            KFileRelease ( ctp -> cache );
            /* try to remove the cache-file, something went wrong... */
            KDirectoryRemove ( ctp -> dir, true, "%s.cache", ctp -> resolved_path );
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KDirectoryVMakeCacheTee2 ( struct KDirectory * self,
                                             struct KFile const ** tee,
                                             struct KFile const * to_wrap,
                                             uint32_t block_size,
                                             const char * path,
                                             va_list args )
{
    rc_t rc = 0;
    
    if ( tee == NULL )
        rc = RC ( rcFS, rcFile, rcAllocating, rcParam, rcNull );
    else
    {
        *tee = NULL;
        if ( to_wrap == NULL )
            rc = RC ( rcFS, rcFile, rcAllocating, rcParam, rcNull );
        else if ( self == NULL )
            rc = RC ( rcFS, rcFile, rcAllocating, rcSelf, rcNull );
        else if ( path == NULL )
            rc = RC ( rcFS, rcFile, rcAllocating, rcPath, rcNull );
        else if ( path [ 0 ] == 0 )
            rc = RC ( rcFS, rcFile, rcAllocating, rcPath, rcEmpty );
    }
    
    if ( rc == 0 )
    {
        cache_tee_params ctp;
        rc = KFileSize ( to_wrap, &( ctp . to_wrap_size ) );
        if ( rc != 0 )
        {
            LOGERR( klogErr, rc, "cannot detect size of file to be wrapped" );
        }
        else if ( ctp . to_wrap_size == 0 )
        {
            rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );
            LOGERR( klogErr, rc, "file to be wrapped is empty" );
        }
        else
        {
            rc = KDirectoryVResolvePath ( self,
                                          false, /* absolute */
                                          ctp . resolved_path,
                                          sizeof ctp . resolved_path,
                                          path,
                                          args );
            if ( rc != 0 )
            {
                PLOGERR( klogErr, ( klogErr, rc, "cannot resolve path of cache file '$(path)'", 
                        "path=%s", path ) );
            }
            else
            {
                ctp . dir = self;
                ctp . to_wrap = to_wrap;
                ctp . block_size = ( block_size > 0 ) ? block_size : CACHE_TEE_DEFAULT_BLOCKSIZE;
                ctp . resolved_path_size = string_size ( ctp . resolved_path );
                ctp . read_only = false;
                
                /* let the blocksize be a multiple of 16 */
                ctp . block_size &= 0xFFFFF0;
                if ( ctp . block_size < CACHE_TEE_MIN_BLOCKSIZE )
                {
                    rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInsufficient );
                    LOGERR( klogErr, rc, "blocksize too small" );
                }
                else
                {
                    /* lets see if we can open the cache-file in read/write - mode */
                    rc = KDirectoryOpenFileSharedWrite ( self,
                                                   &( ctp . cache ),
                                                   true,
                                                   "%s.cache",
                                                   ctp . resolved_path );
                    if ( rc == 0 )
                    {
                        bool re_init_cache = false;
                        /* we have the exclusive rd/wr access to the cache file !*/
                        rc = initialize_existing_cache_tee ( tee, &ctp, &re_init_cache );   /* <========== */
                        if ( rc != 0 )
                        {
                            KFileRelease ( ctp . cache );
                            if ( re_init_cache )
                            {
                                /* let's try to get rid of the existing-cache-file! */
                                rc = KDirectoryRemove ( self, false, "%s.cache", ctp . resolved_path );
                                if ( rc == 0 )
                                {
                                    /* we have removed the broken cache-file, let's try to create a new one */
                                    rc = create_new_cachetee ( tee, &ctp ); /* <========== */
                                }
                            }
                        }
                    }
                    else if ( GetRCState( rc ) == rcNotFound )
                    {
                        /* cache-file does not exist, let's try to create it */
                        rc = create_new_cachetee ( tee, &ctp ); /* <========== */
                    }
                    else
                    {
                        /* let us try to open the cache in read-only mode !*/
                        rc = KDirectoryOpenFileRead ( self,
                                                      ( const struct KFile ** )&( ctp . cache ),
                                                      "%s.cache",
                                                      ctp . resolved_path );
                        if ( rc != 0 )
                        {
                            /* we cannot open the cache-file in read-only-mode */
                            LOGERR( klogErr, rc, "cannot open cache-file" );
                        }
                        else
                        {
                            bool re_init_cache = false;
                            ctp . read_only = true;
                            /* see if we can detect the size of the cache - file */
                            rc = initialize_existing_cache_tee ( tee, &ctp, &re_init_cache ); /* <========== */
                            if ( rc != 0 )
                            {
                                KFileRelease ( ctp . cache );
                                /* because we have only read-only-access,
                                   we cannot use the cache at all if re_init_cache is true! */
                            }
                        }
                    }
                }
            }
        }
        if ( rc != 0 )
            rc = hand_out_to_wrap_file_as_tee_file ( tee, to_wrap );
        
    }
    return rc;
}


LIB_EXPORT rc_t CC KDirectoryMakeCacheTee2 ( struct KDirectory *self,
                                             struct KFile const **tee,
                                             struct KFile const *to_wrap,
                                             uint32_t block_size,
                                             const char *path, ... )
{
    rc_t rc;
    va_list args;
    va_start ( args, path );
    rc = KDirectoryVMakeCacheTee2 ( self, tee, to_wrap, block_size, path, args );
    va_end ( args );
    return rc;
}


static uint64_t count_bits_in_bitmap( const uint64_t block_count, const uint64_t bitmap_bytes, const atomic32_t * bitmap )
{
    uint64_t res = 0;
    uint64_t idx;
    for ( idx = 0; idx < block_count; ++idx )
    {
        if ( IS_BITMAP_BIT( bitmap, idx ) )
            res++;
    }
    return res;
}

LIB_EXPORT rc_t CC TruncateCacheTee2File( struct KFile * self )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC( rcFS, rcFile, rcResizing, rcSelf, rcNull );
    else
    {
        uint64_t file_size;
        rc = KFileSize( self, &file_size );
        if ( rc != 0 )
        {
            LOGERR( klogErr, rc, "cannot detect filesize when truncating cached file" );
        }
        else
        {
            uint32_t block_size;
            rc = read_block_size( self, file_size, &block_size );
            if ( rc == 0 )
            {
                uint64_t content_size;
                rc = read_content_size( self, file_size, &content_size );
                if ( rc == 0 )
                {
                    uint64_t block_count, bitmap_bytes;
                    rc = verify_file_structure( file_size, block_size, content_size, &block_count, &bitmap_bytes );

                    /* truncate the file by setting the new (shorter) filesize */
                    if ( rc == 0 )
                        rc = KFileSetSize( self, content_size );
                }
            }
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC GetCacheTee2FileCompleteness( const struct KFile * self, float * percent, uint64_t * bytes_in_cache )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC( rcFS, rcFile, rcValidating, rcSelf, rcNull );
    else
    {
        uint64_t cache_size;
        if ( percent != NULL ) *percent = 0;
        if ( bytes_in_cache != NULL ) *bytes_in_cache = 0;
        rc = KFileSize( self, &cache_size );
        if ( rc != 0 )
        {
            LOGERR( klogErr, rc, "cannot detect filesize when checking if cached file is complete" );
        }
        else
        {
            uint32_t block_size;
            rc = read_block_size( self, cache_size, &block_size );
            if ( rc == 0 )
            {
                uint64_t content_size;
                rc = read_content_size( self, cache_size, &content_size );
                /* create bitmap buffer */
                if ( rc == 0 )
                {
                    uint64_t block_count;
                    uint64_t bitmap_bytes;
                    rc = verify_file_structure( cache_size, block_size, content_size, &block_count, &bitmap_bytes );
                    if ( rc == 0 )
                    {
                        atomic32_t * bitmap = NULL;
                        rc = create_bitmap_buffer( &bitmap, bitmap_bytes );
                        if ( rc == 0 )
                        {
                            size_t num_read;
                            /* read the bitmap into the memory ... */
                            rc = KFileReadAll ( self, content_size, ( void * ) bitmap, bitmap_bytes, &num_read );
                            if ( rc != 0 )
                            {
                                LOGERR( klogErr, rc, "cannot read bitmap from local file" );
                            }
                            else if ( num_read != bitmap_bytes )
                            {
                                rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );
                                PLOGERR( klogErr, ( klogErr, rc, "cannot read $(ls) bitmap-bytes from local file, read $(rs) instead", 
                                               "ls=%lu,rs=%lu", bitmap_bytes, num_read ));
                            }
                        }
                        if ( rc == 0 )
                        {
                            uint64_t in_cache = count_bits_in_bitmap( block_count, bitmap_bytes, bitmap );
                            if ( in_cache > 0 && block_count > 0 )
                            {
                                float res = ( float ) in_cache;
                                res *= 100;
                                res /= block_count;
                                if ( percent != NULL ) ( *percent ) = res;
                                if ( bytes_in_cache != NULL ) ( *bytes_in_cache ) = ( in_cache * block_size );
                            }
                        }
                        if ( bitmap != NULL )
                            free( ( void * ) bitmap );
                    }
                }
            }
        }
    }
    return rc;
}


/* -----
 * examens the file, and reports the size of the original file ( without the cachefile-footer )
 *
 */
LIB_EXPORT rc_t CC GetCacheTee2FileTruncatedSize( const struct KFile * self, uint64_t * truncated_size )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC( rcFS, rcFile, rcValidating, rcSelf, rcNull );
    else if ( truncated_size == NULL )
        rc = RC( rcFS, rcFile, rcValidating, rcParam, rcNull );
    else
    {
        uint64_t cache_size;
        *truncated_size = 0;
        rc = KFileSize( self, &cache_size );
        if ( rc != 0 )
        {
            LOGERR( klogErr, rc, "cannot detect filesize when checking if cached file is complete" );
        }
        /* TODO: need to do a better constant tha a magic number */
        else if ( cache_size < 13 )
        {
            rc = RC ( rcFS, rcFile, rcValidating, rcSize, rcInsufficient );
            LOGERR( klogErr, rc, "cannot use filesize of zero when checking if cached file is complete" );
        }
        else
        {
            uint32_t block_size;
            rc = read_block_size( self, cache_size, &block_size );
            if ( rc == 0 )
            {
                uint64_t content_size;
                rc = read_content_size( self, cache_size, &content_size );
                if ( rc == 0 )
                {
                    uint64_t block_count, bitmap_bytes;
                    rc = verify_file_structure( cache_size, block_size, content_size, &block_count, &bitmap_bytes );
                    if ( rc == 0 )
                    {
                        *truncated_size = content_size;
                    }
                }
            }
        }
    }
    return rc;
}

static rc_t CC IsThisCacheFileComplete( const struct KFile * self, bool * is_complete )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC( rcFS, rcFile, rcValidating, rcSelf, rcNull );
    else if ( is_complete == NULL )
        rc = RC( rcFS, rcFile, rcValidating, rcParam, rcNull );
    else
    {
        uint64_t file_size;

        *is_complete = false;
        rc = KFileSize( self, &file_size );
        if ( rc != 0 )
        {
            LOGERR( klogErr, rc, "cannot detect filesize when checking if cached file is complete" );
        }
        /* TODO: need to do a better constant tha a magic number */
        else if ( file_size < 13 )
        {
            rc = RC ( rcFS, rcFile, rcValidating, rcSize, rcInsufficient );
            LOGERR( klogErr, rc, "cannot use filesize of zero when checking if cached file is complete" );
        }
        else
        {
            uint32_t block_size;
            rc = read_block_size( self, file_size, &block_size );
            if ( rc == 0 )
            {
                uint64_t content_size;

                rc = read_content_size( self, file_size, &content_size );
                /* create bitmap buffer */
                if ( rc == 0 )
                {
                    uint64_t block_count;
                    uint64_t bitmap_bytes;
                    rc = verify_file_structure( file_size, block_size, content_size, &block_count, &bitmap_bytes );
                    if ( rc == 0 )
                    {
                        atomic32_t * bitmap = NULL;
                        rc = create_bitmap_buffer( &bitmap, bitmap_bytes );
                        if ( rc == 0 )
                        {
                            size_t num_read;
                            /* read the bitmap into the memory ... */
                            rc = KFileReadAll ( self, content_size, ( void * ) bitmap, bitmap_bytes, &num_read );
                            if ( rc != 0 )
                            {
                                LOGERR( klogErr, rc, "cannot read bitmap from local file" );
                            }
                            else if ( num_read != bitmap_bytes )
                            {
                                rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );
                                PLOGERR( klogErr, ( klogErr, rc, "cannot read $(ls) bitmap-bytes from local file, read $(rs) instead", 
                                                    "ls=%lu,rs=%lu", bitmap_bytes, num_read ));
                            }
                        }
                        if ( rc == 0 )
                            *is_complete = is_bitmap_full( bitmap, bitmap_bytes, block_count );

                        if ( bitmap != NULL )
                            free( ( void * ) bitmap );
                    }
                }
            }
        }
    }
    return rc;
}


/* -----
 * reports in the boolean if the file is a cachetoofile and it is complete...
 *
 * KFile is the struct returned by KDirectoryMakeCacheTee()..
 */
LIB_EXPORT rc_t CC IsCacheTee2FileComplete( const struct KFile * self, bool * is_complete )
{
    rc_t rc = 0;
    if ( self == NULL || is_complete == NULL )
        rc = RC ( rcFS, rcFile, rcValidating, rcParam, rcNull );
    else
    {
        if ( &self->vt->v1 != &vtKCacheTee2File_rw ||
             &self->vt->v1 != &vtKCacheTee2File_ro )
        {
            /* the given file is NOT a cache-tee-file */
            rc = IsThisCacheFileComplete( self, is_complete );
        }
        else
        {
            struct KCacheTee2File * ctf = ( struct KCacheTee2File * )self;
            *is_complete = is_bitmap_full( ctf->bitmap, ctf->bitmap_bytes, ctf->block_count );
        }
    }
    return rc;
}

LIB_EXPORT bool CC KFileIsKCacheTee2File( const struct KFile * self )
{
    bool res = false;
    if ( self != NULL )
    {
        res = &self->vt->v1 == &vtKCacheTee2File_rw;
        if ( !res ) res = &self->vt->v1 == &vtKCacheTee2File_ro;
    }
    return res;
}
