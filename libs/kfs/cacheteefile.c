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

struct KCacheTeeFile;
#define KFILE_IMPL struct KCacheTeeFile
#include <kfs/impl.h>

#include <klib/rc.h>
#include <klib/log.h>
#include <klib/out.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/checksum.h>
#include <klib/time.h>

#include <kfs/cacheteefile.h>
#include <kfs/defs.h>
#include <kproc/queue.h>
#include <kproc/timeout.h>
#include <atomic32.h>

#include <sysalloc.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <endian.h>

#include <stdio.h>

#define NO_SCRATCH_BUFFER 1
#define USE_BUFFER_POOL NO_SCRATCH_BUFFER && 1

/* byte-order is an issue for treating these as words */
#define USE_32BIT_BITMAP_WORDS 1


/*--------------------------------------------------------------------------
 layout of local file:
 
 CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC MMMMMMMMMMMMMM SS BB

 C ... file content in valid/invalid blocks
 M ... bitmap of blocks bytes = ( ( ( content-size / block-size ) + 1 ) / 8 ) + 1 )
 S ... size of content ( uint64_t ) 8 bytes
 B ... used blocksize  ( uint32_t ) 4 bytes
 */


/*--------------------------------------------------------------------------
 * KCacheTeeFile
 */

#define CACHE_TEE_DEFAULT_BLOCKSIZE ( 32 * 1024 * 4 )

#define CACHE_STAT 0

#if( CACHE_STAT > 0 )
typedef struct CacheStatistic
{
    uint64_t requests;
    uint64_t requested_bytes;
    uint64_t delivered_bytes;    
    uint64_t requests_below_32k;
    uint64_t requests_consecutive;
    uint64_t requests_in_first32k;
    uint64_t prev_pos;
    uint64_t requests_same_pos;
    uint64_t requests_same_pos_and_len;
    size_t prev_len;
    size_t min_len;
    size_t max_len;
    
} CacheStatistic;


static void init_cache_stat( CacheStatistic * stat )
{
    stat -> requests = 0;
    stat -> requested_bytes = 0;
    stat -> delivered_bytes = 0;    
    stat -> requests_below_32k = 0;
    stat -> requests_consecutive = 0;
    stat -> requests_in_first32k = 0;
    stat -> prev_pos = 0xFFFFFFFFFFFFFFFF;
    stat -> requests_same_pos = 0;
    stat -> requests_same_pos_and_len = 0;
    stat -> prev_len = 0;
    stat -> min_len = -1;    
    stat -> max_len = 0;
}

static void write_cache_stat( CacheStatistic * stat, uint64_t pos, size_t requested, size_t delivered )
{
    ( stat -> requests )++;
    ( stat -> requested_bytes )+= requested;
    ( stat -> delivered_bytes )+= delivered;    

    if ( requested < 0x08000 )
        ( stat -> requests_below_32k )++;

    if ( delivered < stat -> min_len )
        stat -> min_len    = delivered;

    if ( delivered > stat -> max_len )
        stat -> max_len    = delivered;

    if ( pos + requested < 0x08000 )
        ( stat -> requests_in_first32k )++;
        
    if ( stat -> prev_pos != pos )
    {
        if ( stat -> prev_pos + stat -> prev_len == pos )
        {
            ( stat -> requests_consecutive )++;
        }
        
        stat -> prev_pos = pos;
        stat -> prev_len = requested;
    }
    else
    {
        if ( stat -> prev_len != requested )
        {
            ( stat -> requests_same_pos_and_len )++;
            stat -> prev_len = requested;
        }
        ( stat -> requests_same_pos )++;
    }
}

static void report_cache_stat( CacheStatistic * stat )
{
    fprintf( stderr, "\n" );
    fprintf( stderr, "cache-stat.requests ................... %lu\n", stat -> requests );
    fprintf( stderr, "cache-stat.requested_bytes ............ %lu\n", stat -> requested_bytes );
    fprintf( stderr, "cache-stat.delivered_bytes ............ %lu\n", stat -> delivered_bytes );    
    fprintf( stderr, "cache-stat.requests_below_32k ......... %lu\n", stat -> requests_below_32k );
    fprintf( stderr, "cache-stat.requests_consecutive........ %lu\n", stat -> requests_consecutive );
    fprintf( stderr, "cache-stat.requests_in_first32k........ %lu\n", stat -> requests_in_first32k );
    
    fprintf( stderr, "cache-stat.requests_same_pos .......... %lu\n", stat -> requests_same_pos );
    fprintf( stderr, "cache-stat.requests_same_pos_and_len .. %lu\n", stat -> requests_same_pos );
    fprintf( stderr, "cache-stat.min_len .................... %u\n", stat -> min_len );
    fprintf( stderr, "cache-stat.max_len .................... %u\n", stat -> max_len );
    
    fprintf( stderr, "\n" );
}

#endif


typedef struct KCacheTeeFile
{
    KFile dad;
    const KFile * remote;                    /* the remote file we are wrapping (can be a local one too, we make no assumptions about that) */
    KFile * local;                            /* the local cache of the remote one */
    KDirectory * dir;                        /* we have to store a KDirectory because we need it at closing the file, where we test for promoting the cache */

    uint64_t remote_size;                    /* the size of the wrapped file */
    uint64_t local_size;                    /* the size of the local cache file ( remote_size + bitmap + tail ) */
    uint64_t block_count;                    /* how many blocks do we need to cache the remote file ( last block may be shorter ) */

#if USE_32BIT_BITMAP_WORDS
    atomic32_t * bitmap;
#else
    uint8_t volatile * bitmap;                /* the bitmap of cached blocks */
#endif

    uint64_t bitmap_bytes;                    /* how many bytes do we need to store the bitmap */

#if USE_BUFFER_POOL
    KQueue * buffer_pool;
#endif
#if ! NO_SCRATCH_BUFFER
    uint8_t * scratch_buffer;
    uint64_t first_block_in_scratch;        /* what is the block-id of the first block in the scratch-buffer */
    uint64_t scratch_size;                    /* how many bytes are allocated for the scratch-buffer */
    uint64_t valid_scratch_bytes;            /* how many bytes store valid data in the scratch-buffer */
#endif

    uint32_t block_size;                    /* how big is a block ( aka 1 bit in the bitmap )*/

#if( CACHE_STAT > 0 )
    CacheStatistic stat;                    /* optional cache statistic */
#endif

    bool local_read_only;
    bool promote;
    char local_path [ 1 ];                    /* stores the path to the local cache, for eventual promoting at close */
} KCacheTeeFile;


#if USE_32BIT_BITMAP_WORDS

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define SWAP_FN(val) (val)
#else
#define SWAP_FN(val) \
    (((val)>>24)&0xff) | /* move byte 3 to byte 0 */ \
    (((val)<<8)&0xff0000) | /* move byte 1 to byte 2 */ \
    (((val)>>8)&0xff00) | /* move byte 2 to byte 1 */ \
    (((val)<<24)&0xff000000) /* byte 0 to byte 3 */
#endif
#define GEN_BIT_NR_MASK_ROW(i) SWAP_FN( 1U << ( (i) * 4 ) ), SWAP_FN( 1U << ( (i) * 4 + 1 ) ), SWAP_FN( 1U << ( (i) * 4 + 2 ) ), SWAP_FN( 1U << ( (i) * 4 + 3 ) )

const uint32_t BitNr2Mask[ 32 ] =
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
    ( ( atomic32_read ( & ( CacheFile )->bitmap[ (Block_Nr) >> 5 ] ) & BitNr2Mask[ (Block_Nr) & 31 ] ) > 0 )

#define IS_BITMAP_BIT( BitMap, Block_Nr ) \
    ( ( atomic32_read ( & ( BitMap )[ (Block_Nr) >> 5 ] ) & BitNr2Mask[ (Block_Nr) & 31 ] ) > 0 )

#else

const uint8_t BitNr2Mask[ 8 ] =
{
         /* 0  1  2  3   4   5   6    7  */
            1, 2, 4, 8, 16, 32, 64, 128
};

#define IS_CACHE_BIT( CacheFile, Block_Nr ) ( ( CacheFile->bitmap[ (Block_Nr) >> 3 ] & BitNr2Mask[ (Block_Nr) & 7 ] ) > 0 )
#define IS_BITMAP_BIT( BitMap, Block_Nr ) ( ( BitMap[ (Block_Nr) >> 3 ] & BitNr2Mask[ (Block_Nr) & 7 ] ) > 0 )


#endif

#define BITS_2_BYTES( BitCount ) ( ( ( BitCount ) + 7 ) >> 3 )
#define SIZE_2_BLOCK_COUNT( Number_Of_Bytes, Block_Size ) ( ( ( Number_Of_Bytes ) + ( Block_Size ) - 1 ) / ( Block_Size ) )

static rc_t calculate_local_size_from_remote_size( KCacheTeeFile *self )
{
    rc_t rc = 0;
    if ( self->block_size > 0 )
    {
        self->block_count = SIZE_2_BLOCK_COUNT( self->remote_size, self->block_size );
        self->bitmap_bytes = BITS_2_BYTES( self->block_count );
        self->local_size = ( self->remote_size +
                             self->bitmap_bytes + 
                             sizeof self->remote_size + 
                             sizeof self->block_size );
    }
    else
    {
        rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );
        LOGERR( klogErr, rc, "div by zero attempt in calculating local size" );
    }
    return rc;
}

/*    factored out because it is used in:
    create_bitmap(), IsCacheFileComplete(), GetCacheCompleteness() and Has_Cache_Zero_Blocks()
*/
#if USE_32BIT_BITMAP_WORDS
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
#else
static rc_t create_bitmap_buffer( uint8_t volatile ** bitmap, uint64_t bitmap_bytes )
{
    rc_t rc = 0;
    *bitmap = calloc ( sizeof **bitmap, bitmap_bytes );
    if ( *bitmap == NULL )
    {
        rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
        LOGERR( klogErr, rc, "init local bitmap-area" );
    }
    return rc;
}
#endif

/*    factored out because it is used in:
    init_new_local_file() and verify_existing_local_file()
*/
static rc_t create_bitmap( KCacheTeeFile *self )
{
    return create_bitmap_buffer( &self->bitmap, self->bitmap_bytes );
}


#if USE_32BIT_BITMAP_WORDS
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
#else
static bool is_bitmap_full( const uint8_t volatile * bitmap, uint64_t bitmap_bytes, uint64_t block_count )
{
    uint64_t bitmap_byte = 0;
    while( bitmap_byte < ( bitmap_bytes - 1 ) )
    {
        if ( bitmap[ bitmap_byte++ ] != 0xFF )
            return false;
    }

    {
        uint64_t block_id = ( bitmap_byte << 3 );
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
#endif


static rc_t init_new_local_file( KCacheTeeFile * cf )
{
    rc_t rc = create_bitmap( cf );
    if ( rc == 0 )
    {
        size_t written;
        uint64_t pos = cf -> remote_size;

        /* write the bitmap ( zero'd out ) into the local file */
        rc = KFileWriteAll ( cf -> local, pos, 
            ( const void * ) cf -> bitmap, cf -> bitmap_bytes, &written );
        if ( rc == 0 && written != cf -> bitmap_bytes )
        {
            rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );
            LOGERR( klogErr, rc, "no full initialization of local file bitmap" );
        }

        /* write the remote-file-size into the local file ( uint64_t = 8 bytes )*/
        if ( rc == 0 )
        {
            pos += cf -> bitmap_bytes;
            rc = KFileWriteAll ( cf -> local, pos,
                                 &cf -> remote_size, sizeof( cf -> remote_size ), &written );
            if ( rc == 0 && written != sizeof( cf -> remote_size ) )
            {
                rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );
                LOGERR( klogErr, rc, "cannot write remote-filesize into local file" );
            }
        }

        /* write the block-size into the local file ( size_t = 4 bytes )*/
        if ( rc == 0 )
        {
            pos += sizeof( cf -> remote_size );
            rc = KFileWriteAll ( cf -> local, pos,
                                 &cf -> block_size, sizeof( cf -> block_size ), &written );
            if ( rc == 0 && written != sizeof( cf -> block_size ) )
            {
                rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );
                LOGERR( klogErr, rc, "cannot write block-size into local file" );
            }
        }

    }
    return rc;
}


static rc_t try_read_uint32_t( const struct KFile * self, uint64_t pos, uint32_t * value )
{
    size_t num_read;
    rc_t rc =  KFileRead( self, pos, value, sizeof *value, &num_read );
    if ( rc == 0 )
    {
        if ( num_read != sizeof *value )
            rc = SILENT_RC ( rcFS, rcFile, rcValidating, rcParam, rcInvalid );
    }
    return rc;
}

static rc_t try_read_uint64_t( const struct KFile * self, uint64_t pos, uint64_t * value )
{
    size_t num_read;
    rc_t rc =  KFileRead( self, pos, value, sizeof *value, &num_read );
    if ( rc == 0 )
    {
        if ( num_read != sizeof *value )
            rc = SILENT_RC ( rcFS, rcFile, rcValidating, rcParam, rcInvalid );
    }
    return rc;
}


static rc_t read_block_size( const struct KFile * self, uint64_t local_size, uint32_t *block_size )
{
    if ( local_size >= sizeof *block_size )
    {
        uint64_t pos = local_size - ( sizeof *block_size );
        int num_try = 3;
        rc_t rc;

        while ( true )
        {
            rc = try_read_uint32_t( self, pos, block_size );
            
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


static rc_t read_content_size( const struct KFile * self, uint64_t local_size, uint64_t *content_size )
{
    if ( local_size >= sizeof( *content_size ) + 4 )
    {
        uint64_t pos = ( local_size - 4 ) - sizeof( *content_size );
        int num_try = 3;
        rc_t rc;

        while ( true )
        {
            rc = try_read_uint64_t( self, pos, content_size );
            if ( rc == 0 && *content_size != 0 )
            {
                if ( *content_size < local_size )
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


static rc_t verify_file_structure( const uint64_t local_size, const uint32_t block_size, const uint64_t content_size,
                                   uint64_t * block_count, /* size_t */ uint64_t * bitmap_bytes )
{
    rc_t rc = 0;
    uint64_t expected_size;

    *block_count = SIZE_2_BLOCK_COUNT( content_size, block_size );
    *bitmap_bytes = BITS_2_BYTES( *block_count );

    /* check if the values 'content-size' and 'block_size' result in the currect real file size */
    expected_size = content_size + *bitmap_bytes + sizeof ( local_size ) + sizeof ( block_size );
    if ( expected_size != local_size )
        rc = RC ( rcFS, rcFile, rcValidating, rcParam, rcInvalid );
    return rc;
}


static rc_t read_bitmap( KCacheTeeFile * cf )
{
    /* read the bitmap from the local file */
    size_t num_read;
    rc_t rc = KFileReadAll ( cf -> local, cf -> remote_size, ( void * ) cf -> bitmap, cf -> bitmap_bytes, &num_read );
    if ( rc != 0 )
    {
        LOGERR( klogErr, rc, "cannot read bitmap from local file" );
    }
    else if ( num_read != cf -> bitmap_bytes )
    {
        rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );
        PLOGERR( klogErr, ( klogErr, rc, "cannot read $(ls) bitmap-bytes from local file, read $(rs) instead", 
                       "ls=%lu,rs=%lu", cf -> bitmap_bytes, num_read ));
    }
    return rc;
}


static rc_t verify_existing_local_file( KCacheTeeFile * cf, bool * fully_in_cache )
{
    uint64_t bitmap_bytes, content_size, block_count;
    uint32_t block_size;
    bool cached = false;
    
    rc_t rc = read_block_size ( cf -> local, cf -> local_size, &block_size );
    /* read content-size, compare to the content-size of the remote file */
    if ( rc == 0 )
        rc = read_content_size ( cf -> local, cf -> local_size, &content_size );

    if ( rc == 0 && content_size != cf -> remote_size )
    {
        rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );
        PLOGERR( klogErr, ( klogErr, rc, "content-size in local file $(ls) does not match size of remote file $(rs)", 
                           "ls=%lu,rs=%lu", content_size, cf -> remote_size ) );
    }

    /* check if the local file has the right size ( content + bitmap + content_size + block_size )*/
    if ( rc == 0 )
        rc = verify_file_structure ( cf -> local_size, block_size, content_size, &block_count, &bitmap_bytes );

    /* check if the requested block-size equals the stored block-size */
    if ( rc == 0 )
    {
        if ( cf -> block_size == 0 )
            cf -> block_size = CACHE_TEE_DEFAULT_BLOCKSIZE;
        if ( cf -> block_size != block_size )
        {
            PLOGMSG( klogWarn, ( klogWarn, "block-size in local file $(ls) does not match requested value $(rv)",
                                "ls=%u,rv=%u", block_size, cf -> block_size ) );
        }
        /* use the block-size as stored in the file */
        cf -> block_size = block_size;
        cf -> block_count = block_count;
        cf -> bitmap_bytes = bitmap_bytes;
        rc = create_bitmap( cf );
    }

    /* read the bitmap into the memory ... */
    if ( rc == 0 )
        rc = read_bitmap( cf );

    if ( rc == 0 )
        cached = is_bitmap_full( cf -> bitmap, cf -> bitmap_bytes, cf -> block_count ); 
        
    if ( rc == 0 && fully_in_cache != NULL )
        *fully_in_cache = cached;
        
    return rc;
}


LIB_EXPORT rc_t CC IsCacheFileComplete( const struct KFile * self, bool * is_complete )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC( rcFS, rcFile, rcValidating, rcSelf, rcNull );
    else if ( is_complete == NULL )
        rc = RC( rcFS, rcFile, rcValidating, rcParam, rcNull );
    else
    {
        uint64_t local_size;
        *is_complete = false;
        rc = KFileSize( self, &local_size );
        if ( rc != 0 )
        {
            LOGERR( klogErr, rc, "cannot detect filesize when checking if cached file is complete" );
        }
        /* TODO: need to do a better constant tha a magic number */
        else if (local_size < 13)
        {
            rc = RC ( rcFS, rcFile, rcValidating, rcSize, rcInsufficient );
            LOGERR( klogErr, rc, "cannot use filesize of zero when checking if cached file is complete" );
        }
        else
        {
            uint32_t block_size;
            rc = read_block_size( self, local_size, &block_size );
            if ( rc == 0 )
            {
                uint64_t content_size;

                rc = read_content_size( self, local_size, &content_size );
                /* create bitmap buffer */
                if ( rc == 0 )
                {
                    uint64_t block_count;
                    /* size_t */ uint64_t bitmap_bytes;
                    rc = verify_file_structure( local_size, block_size, content_size, &block_count, &bitmap_bytes );
                    if ( rc == 0 )
                    {
#if USE_32BIT_BITMAP_WORDS
                        atomic32_t * bitmap = NULL;
#else
                        uint8_t volatile * bitmap = NULL;
#endif
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


LIB_EXPORT rc_t CC TruncateCacheFile( struct KFile * self )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC( rcFS, rcFile, rcResizing, rcSelf, rcNull );
    else
    {
        uint64_t local_size;
        rc = KFileSize( self, &local_size );
        if ( rc != 0 )
        {
            LOGERR( klogErr, rc, "cannot detect filesize when truncating cached file" );
        }
        else
        {
            uint32_t block_size;
            rc = read_block_size( self, local_size, &block_size );
            if ( rc == 0 )
            {
                uint64_t content_size;
                rc = read_content_size( self, local_size, &content_size );
                if ( rc == 0 )
                {
                    uint64_t block_count, bitmap_bytes;
                    rc = verify_file_structure( local_size, block_size, content_size, &block_count, &bitmap_bytes );

                    /* truncate the file by setting the new (shorter) filesize */
                    if ( rc == 0 )
                        rc = KFileSetSize( self, content_size );
                }
            }
        }
    }
    return rc;
}


static bool file_exist( KDirectory * dir, const char * filename )
{
    uint32_t pt = KDirectoryPathType ( dir, "%s", filename );
    return ( ( pt & ~kptAlias ) == kptFile );
}


static rc_t promote_cache( KCacheTeeFile * self )
{
    char cache_file_name [ 4096 ];
    char temp_file_name [ 4096 ];
    size_t num_writ;
    rc_t rc = string_printf ( cache_file_name, sizeof cache_file_name, &num_writ, "%s.cache", self -> local_path );
    if ( rc == 0 )
        rc = string_printf ( temp_file_name, sizeof temp_file_name, &num_writ, "%s.cache.temp", self -> local_path );

    /* (1) releaes open cache file ( windows cannot rename open files ) */
    if ( rc == 0 )
        rc = KFileRelease( self -> local );

    /* (2) rename to temporary name */
    if ( rc == 0 )
    {
        self -> local = NULL;
        rc = KDirectoryRename ( self -> dir, true, cache_file_name, temp_file_name );
    }

    /* (3) open from temporary name */
    if ( rc == 0 )
        rc = KDirectoryOpenFileWrite( self -> dir, &self -> local, true, "%s", temp_file_name );

    /* (4) perform truncation */
    if ( rc == 0 )
        rc = TruncateCacheFile( self -> local );
    
    /* (5) releaes open temp. cache file ( windows cannot rename open files ) */
    if ( rc == 0 )
        rc = KFileRelease( self -> local );

    /* (6) rename to final filename ( windows cannot rename open files ) */
    if ( rc == 0 )
    {
        self -> local = NULL;
        rc = KDirectoryRename ( self -> dir, true, temp_file_name, self -> local_path );
    }

    /* (6) open from final filename */
    if ( rc == 0 )
        rc = KDirectoryOpenFileWrite( self -> dir, &self -> local, true, "%s", self -> local_path );

    return rc;
}

#if USE_BUFFER_POOL
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
static void clean_up_buffer_pool( KQueue * buffer_pool )
{
    void * pool_page;
    while ( ( pool_page = pop_page( buffer_pool, 100 ) ) != NULL )
    {
        free( pool_page );
    }
    KQueueRelease( buffer_pool );
}
#endif

/* Destroy
 */
static rc_t CC KCacheTeeFileDestroy( KCacheTeeFile * self )
{
    rc_t rc;
    bool already_promoted_by_other_instance = file_exist( self -> dir, self -> local_path );
    
#if( CACHE_STAT > 0 )
    report_cache_stat( & self -> stat );
#endif
    
    if ( !self -> local_read_only && !already_promoted_by_other_instance )
    {
        bool fully_in_cache;
        rc = IsCacheFileComplete ( self -> local, &fully_in_cache );
        if ( rc == 0 && fully_in_cache && self -> promote )
        {
            promote_cache( self );
        }
    }

    if ( self->bitmap != NULL )
        free( ( void * ) self->bitmap );
#if ! NO_SCRATCH_BUFFER
    if ( self->scratch_buffer != NULL )
        free( self->scratch_buffer );
#endif

#if USE_BUFFER_POOL
    clean_up_buffer_pool( self -> buffer_pool );
#endif

    KFileRelease ( self -> remote );
    KFileRelease ( self -> local );

    if ( already_promoted_by_other_instance )
        KDirectoryRemove ( self -> dir, true, "%s.cache", self -> local_path );
        
    KDirectoryRelease ( self -> dir );
    
    free ( self );
    return 0;
}


static void set_bitmap( const KCacheTeeFile *cself, uint64_t start_block, uint64_t block_count )
{
#if USE_32BIT_BITMAP_WORDS
    uint32_t old, bits;
    const uint32_t block_bit = BitNr2Mask [ start_block & 31 ];

    /* we should get rid of block count */
    assert ( block_count == 1 );

    old = atomic32_read ( & cself -> bitmap [ start_block >> 5 ] );
    do
    {
        bits = old;
        old = atomic32_test_and_set ( & cself -> bitmap [ start_block >> 5 ], ( int ) ( bits | block_bit ), ( int ) bits );
    }
    while ( old != bits );

#else
    uint64_t block_idx, block_nr;
    for ( block_idx = 0, block_nr = start_block; 
          block_idx < block_count; 
          ++block_idx, ++block_nr )
    {
        cself->bitmap[ block_nr >> 3 ] |= BitNr2Mask[ block_nr & 0x07 ];
    }
#endif
}


static rc_t switch_to_read_only( const KCacheTeeFile *cself, rc_t rc )
{
    KCacheTeeFile *self = ( KCacheTeeFile * )cself;
    self->local_read_only = true;
    LOGERR( klogInt, rc, "switching cache-tee-file to read-only" );
    return 0;
}

static rc_t write_bitmap( const KCacheTeeFile *cself, uint64_t block )
{
    rc_t rc;
    size_t written;
    uint64_t pos;
    size_t to_write;
#if USE_32BIT_BITMAP_WORDS
    uint32_t block_word = ( uint32_t ) ( block >> 5 );
    uint64_t bitmap_pos = ( block_word << 2 );
    pos = cself->remote_size + bitmap_pos;
    to_write = 4;

    // last word may go outside bitmap ranges in the file, since bitmap has 1-byte alignment in the file, but 4-byte alignment in memory
    if (bitmap_pos + to_write > cself->bitmap_bytes)
        to_write = cself->bitmap_bytes - bitmap_pos;

    rc = KFileWriteAll( cself->local, pos, ( const void * ) &cself->bitmap[ block_word ], to_write, &written );
#else
    uint32_t block_byte = ( uint32_t ) ( block >> 3 );
    pos = cself->remote_size + block_byte;
    to_write = 1;
    rc = KFileWriteAll( cself->local, pos, ( const void * ) &cself->bitmap[ block_byte ], to_write, &written );
#endif
    if ( rc != 0 )
    {
        /* it can happen that we are not able to write to the bitmap because we run out of space
           on the local filesystem. */
        rc = switch_to_read_only( cself, rc );
        /*
        PLOGERR( klogErr, ( klogErr, rc, "cannot write local-file-bitmap block $(block) at $(pos) $(to_write) bytes",
                           "block=%lu,pos=%lu,to_write=%zu", block, pos, to_write ) );
        */
    }
    return rc;
}


#if ! NO_SCRATCH_BUFFER
static rc_t resize_scratch_buffer( const KCacheTeeFile *cself, uint64_t new_size )
{
    rc_t rc = 0;
    KCacheTeeFile *self = ( KCacheTeeFile * )cself;

    /* create scratch-buffer or resize it if needed */
    if ( self->scratch_size == 0 )
    {
        self->scratch_buffer = malloc( new_size );
        if ( self->scratch_buffer == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
        else
            self->scratch_size = new_size;
    }
    else if ( self->scratch_size < new_size )
    {
        uint8_t * tmp = realloc( self->scratch_buffer, new_size );
        if ( tmp == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
        else
        {
            self->scratch_buffer = tmp;
            self->scratch_size = new_size;
        }
    }
    return rc;
}
#endif


size_t check_rd_len( const KCacheTeeFile *cself, uint64_t pos, size_t bsize )
{
    size_t res = bsize;
    uint64_t last_pos = pos;
    last_pos += bsize;
    if ( last_pos >= cself->remote_size )
    {
        if ( pos >= cself->remote_size )
            res = 0;
        else
            res = ( cself->remote_size - pos );
    }
    return res;
}

static rc_t rd_remote_wr_local( const KCacheTeeFile *cself, uint64_t pos,
                                void *buffer, size_t bsize, size_t *num_read )
{
    rc_t rc = 0;
    if ( bsize > 0 )
    {
        size_t bytes_read;
        *num_read = 0;
        rc = KFileReadAll( cself->remote, pos, buffer, bsize, &bytes_read );
        if ( rc != 0 || bytes_read == 0 ) /** try again **/
        {
            rc = KFileReadAll( cself->remote, pos, buffer, bsize, &bytes_read );
            if ( rc == 0 && bytes_read == 0 )
            { /*old behavior */
                rc = RC ( rcFS, rcFile, rcReading, rcBuffer, rcEmpty );
            }
        }
        if ( rc == 0 )
        {
            if ( cself->local_read_only )
                *num_read = bytes_read;
            else
            {
                /* it can happen that we are running out of space in the local filesystem,
                   that means we cannot write ( any more ) */
                rc = KFileWriteAll( cself->local, pos, buffer, bytes_read, num_read );
                if ( rc != 0 )
                    rc = switch_to_read_only( cself, rc );
            }
        }
    }
    return rc;
}


static rc_t KCacheTeeFileRead_simple2( const KCacheTeeFile *cself, uint64_t pos,
                                       void *buffer, size_t bsize, size_t *num_read )
{
    uint64_t block = pos / cself->block_size;
    size_t   offset = pos % cself->block_size;
    size_t   to_read_total = bsize;
    int64_t salvage_block = -1;

#if NO_SCRATCH_BUFFER
    rc_t rc = 0;
    uint64_t first_block_in_scratch = -1;
    uint64_t valid_scratch_bytes = 0;
    uint8_t * scratch_buffer = NULL;
    
#if USE_BUFFER_POOL
    scratch_buffer = pop_page( cself -> buffer_pool, 200 );
#endif    

    if ( scratch_buffer == NULL )
        scratch_buffer = malloc ( cself -> block_size );

    if ( scratch_buffer == NULL )
        return RC ( rcFS, rcFile, rcReading, rcMemory, rcExhausted );
#else
    uint64_t first_block_in_scratch = cself -> first_block_in_scratch;
    uint64_t valid_scratch_bytes = cself -> valid_scratch_bytes;
    rc_t rc = resize_scratch_buffer( cself, cself->block_size );
    uint8_t * scratch_buffer = cself -> scratch_buffer;
#endif
        
    *num_read = 0;

    while ( rc == 0 && to_read_total > 0 )
    {
        size_t to_read = cself->block_size - offset;
        
        if ( to_read > to_read_total )
            to_read = to_read_total;

        if ( first_block_in_scratch == block )
        {
            if ( valid_scratch_bytes <= offset )
            { /** EOF in remote file and nothing to read **/
                to_read_total = to_read = 0; 
            }
            else
            { 
                if ( to_read > valid_scratch_bytes - offset )
                { /** EOF in remote file something left**/
                   to_read_total = to_read = valid_scratch_bytes - offset;
                }
                memmove( buffer, scratch_buffer + offset, to_read );
            }

            /*** move source counters **/
            offset += to_read;
            block  += offset / cself->block_size;
            offset %= cself->block_size;

            /*** move output counters **/       
            to_read_total -= to_read;
            *num_read += to_read;
            buffer = ((char*)buffer) + to_read;
        }
        else if ( IS_CACHE_BIT( cself, block ) )
        {
            uint64_t fpos = block * cself->block_size;
            if ( fpos < cself -> remote_size )
            {
                int64_t fbsize = cself -> remote_size - fpos;
                size_t nread = 0;

                if( fbsize > cself->block_size )
                    fbsize = cself -> block_size;

                rc = KFileReadAll( cself->local, fpos, scratch_buffer, fbsize, &nread );
                if ( rc == 0 )
                {
                    int i;
                    uint64_t *b = ( uint64_t* )scratch_buffer;
                    first_block_in_scratch = block;
                    valid_scratch_bytes = nread;
                    
                    if ( block != salvage_block )
                    { /** check for fully space page, but don't do it in infinite loop **/
                        for ( i = 0; i < ( nread/ sizeof( *b ) ) && b [ i]==0; i++ ) { } 
                        if ( i == ( nread / sizeof( *b ) ) )
                        {
                            rc = rd_remote_wr_local( cself, block*cself->block_size, scratch_buffer, fbsize, &nread );
                            if ( rc == 0 )
                                salvage_block = block;
                        }
                        else
                        {
                            salvage_block = -1;
                        }
                    }
                }
            }
            else
            {
                to_read_total = 0;
            }
        }
        else
        {
            uint64_t fpos = block * cself->block_size;
            if ( fpos < cself -> remote_size )
            {
                int64_t fbsize = cself->remote_size - fpos;
                size_t  nread = 0;

                if ( fbsize > cself->block_size )
                    fbsize = cself->block_size;

                rc = rd_remote_wr_local( cself, fpos, scratch_buffer, fbsize, &nread );
                if ( rc == 0 )
                {
                    first_block_in_scratch = block;
                    valid_scratch_bytes = nread;
                    if ( !cself->local_read_only )
                    {
                        set_bitmap( cself, block, 1 );
                        rc = write_bitmap( cself, block );
                    }
                }
            }
            else
            {
                to_read_total = 0;
            }
        }

    }

#if NO_SCRATCH_BUFFER
#if USE_BUFFER_POOL
    if ( KQueuePush( cself -> buffer_pool, scratch_buffer, NULL ) != 0 )
        free ( scratch_buffer );
#else
    free ( scratch_buffer );
#endif
#else
    ( ( KCacheTeeFile * )cself ) -> first_block_in_scratch = first_block_in_scratch;
    ( ( KCacheTeeFile * )cself ) -> valid_scratch_bytes = valid_scratch_bytes;
#endif

    return rc;
}

#if 0
/**********************************************************************************************
    try #3
**********************************************************************************************/
static rc_t KCacheTeeFileRead_3( const KCacheTeeFile *cself, uint64_t pos,
                                 void * buffer, size_t bsize, size_t *num_read )
{
    rc_t rc = 0;
    uint64_t i_pos = pos;
    uint8_t * i_buffer = buffer;
    size_t still_to_read = bsize;
    size_t total_read = 0;
    size_t bytes_read;

    /* we have to exclude that we are requesting beyond EOF, we can do that because we know the size
       of the file, the 'constructor' of the KCacheTeeFile has requested it and rejects construction
       if it cannot get this size */
    if ( ( i_pos + still_to_read ) > cself -> remote_size )
    {
        still_to_read = ( cself -> remote_size - i_pos );
    }
    
    while ( rc == 0 && still_to_read > 0 )
    {
        uint64_t block = ( i_pos / cself->block_size );
        if ( IS_CACHE_BIT( cself, block ) )
        {
            /* i_pos is in a cached block... */
            size_t to_read = ( ( ( block + 1 ) * cself->block_size ) - i_pos );
            if ( to_read > still_to_read ) to_read = still_to_read;
            rc = KFileReadAll( cself->local, i_pos, i_buffer, to_read, &bytes_read );
            if ( rc == 0 )
            {
                /* check what we read from the local file is completely empty */
                size_t i = 0;
                while ( i_buffer[ i ] == 0 && i < bytes_read ) { i++; }
                if ( i == ( bytes_read + 1 ) )
                {
                    /* we have to read from remote because this segment is zero!
                       this is a fix for broken cache-files */
                    cself->bitmap[ block >> 3 ] &= ~( BitNr2Mask[ block & 0x07 ] );
                    rc = write_bitmap( cself, block );
                    /* do not advance the buffer, because in the loop this will be read remotely now */
                }
                else
                {
                    i_buffer += bytes_read;
                    total_read += bytes_read;
                    i_pos += bytes_read;
                    still_to_read -= bytes_read;
                }
            }
        }
        else
        {
            /* i_pos is not in a cached block... */
            uint64_t block_pos = ( block * cself->block_size );
            uint64_t ofs = ( i_pos - block_pos );
            if ( ofs == 0 && still_to_read >= cself->block_size )
            {
                /* read the whole block remotely, this can return less than requested if last block...
                   and it can return 0 bytes if the http-connection times out... */
                rc = KFileReadAll( cself->remote, i_pos, i_buffer, cself->block_size, &bytes_read );
                if ( rc == 0 )
                {
                    if ( bytes_read == 0 )
                    {
                        still_to_read = 0;    /* terminate the loop, return as much as read so far, return rc ==0 */
                    }
                    else
                    {
                        /* write it to the local file */
                        size_t bytes_written;
                        rc = KFileWriteAll( cself->local, i_pos, i_buffer, bytes_read, &bytes_written );
                        if ( rc == 0 )
                        {
                            if ( bytes_written != bytes_read )
                            {
                                rc = RC ( rcFS, rcFile, rcWriting, rcTransfer, rcTooShort );
                            }
                            else
                            {
                                /* write the bitmap */
                                set_bitmap( cself, block, 1 );
                                rc = write_bitmap( cself, block );
                                if ( rc == 0 )
                                {
                                    i_buffer += bytes_read;
                                    total_read += bytes_read;
                                    i_pos += bytes_read;
                                    still_to_read -= bytes_read;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                /* use the scratch-buffer to read the whole block, copy the smaller part that was requested */
                if ( cself->scratch_size < cself->block_size )
                {
                    rc = resize_scratch_buffer( cself, cself->block_size );
                }
                if ( rc == 0 )
                {
                    /* read the whole block remotely, this can return less than requested if last block...
                       and it can return 0 bytes if the http-connection times out... */
                    rc = KFileReadAll( cself->remote, block_pos, cself->scratch_buffer, cself->block_size, &bytes_read );
                    if ( rc == 0 )
                    {
                        if ( bytes_read == 0 )
                        {
                            still_to_read = 0;    /* terminate the loop, return as much as read so far, return rc ==0 */
                        }
                        else
                        {
                            /* write it to the local file */
                            size_t bytes_written;
                            rc = KFileWriteAll( cself->local, block_pos, cself -> scratch_buffer, bytes_read, &bytes_written );
                            if ( rc == 0 )
                            {
                                if ( bytes_written != bytes_read )
                                {
                                    rc = RC ( rcFS, rcFile, rcWriting, rcTransfer, rcTooShort );
                                }
                                else
                                {
                                    /* write the bitmap */
                                    set_bitmap( cself, block, 1 );
                                    rc = write_bitmap( cself, block );
                                    if ( rc == 0 )
                                    {
                                        /* here comes the difference: copy the bytes from the offset */
                                        size_t to_copy = ( cself->block_size - ofs );
                                        if ( to_copy > still_to_read ) to_copy = still_to_read;
                                        memmove ( i_buffer, &( cself->scratch_buffer[ ofs ] ), to_copy );
                                        
                                        i_buffer += to_copy;
                                        total_read += to_copy;
                                        i_pos += to_copy;
                                        still_to_read -= to_copy;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if ( rc == 0 )
        *num_read = total_read;

    return rc;
}
#endif

/**********************************************************************************************
    START vt-functions
**********************************************************************************************/

static struct KSysFile* KCacheTeeFileGetSysFile( const KCacheTeeFile *self, uint64_t *offset )
{
    * offset = 0;
    return NULL;
}


static rc_t KCacheTeeFileRandomAccess( const KCacheTeeFile *self )
{
    return 0;
}


static rc_t KCacheTeeFileSize( const KCacheTeeFile *self, uint64_t *size )
{
    *size = self->remote_size;
    return 0;
}


static rc_t KCacheTeeFileSetSize( KCacheTeeFile *self, uint64_t size )
{
    return RC ( rcFS, rcFile, rcUpdating, rcFile, rcReadonly );
}

static rc_t KCacheTeeFileRead( const KCacheTeeFile *cself, uint64_t pos,
                               void *buffer, size_t bsize, size_t *num_read )
{

    /* rc_t rc = KCacheTeeFileRead_3( cself, pos, buffer, bsize, num_read ); */
    rc_t rc = KCacheTeeFileRead_simple2( cself, pos, buffer, bsize, num_read );

#if( CACHE_STAT > 0 )
    write_cache_stat( & ( ( ( KCacheTeeFile * )cself ) -> stat ), pos, bsize, *num_read );
#endif
    
    return rc;
}


static rc_t KCacheTeeFileWrite( KCacheTeeFile *self, uint64_t pos,
                                const void *buffer, size_t size, size_t *num_writ )
{
    return RC ( rcFS, rcFile, rcUpdating, rcInterface, rcUnsupported );
}

/**********************************************************************************************
    END vt-functions
**********************************************************************************************/


static KFile_vt_v1 vtKCacheTeeFile =
{
    /* version 1.0 */
    1, 0,

    /* start minor version 0 methods */
    KCacheTeeFileDestroy,
    KCacheTeeFileGetSysFile,
    KCacheTeeFileRandomAccess,
    KCacheTeeFileSize,
    KCacheTeeFileSetSize,
    KCacheTeeFileRead,
    KCacheTeeFileWrite
    /* end minor version 0 methods */
};

static rc_t hand_out_remote_file_as_tee_file( struct KFile const **tee, struct KFile const *remote )
{
    rc_t rc = KFileAddRef( remote );
    if ( rc == 0 )
        *tee = remote;
    return rc;
}

static rc_t make_cache_tee( struct KDirectory *self, struct KFile const **tee,
    struct KFile const *remote, struct KFile *local, uint32_t blocksize, bool read_only, bool promote, const char *path )
{
    rc_t rc;
    size_t path_size = string_size ( path );
    KCacheTeeFile * cf = malloc ( sizeof * cf + path_size + 1 );
    if ( cf == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
    else
    {
        cf -> dir = self;
        string_copy( cf -> local_path, path_size + 1, path, path_size );
        cf -> remote = remote;     /* store the file-objects but no AddRef()'s yet! */
        cf -> local  = local;
        cf -> block_size = ( blocksize > 0 ) ? blocksize : CACHE_TEE_DEFAULT_BLOCKSIZE;
        cf -> bitmap = NULL;
#if ! NO_SCRATCH_BUFFER
        cf -> scratch_buffer = NULL;
        cf -> scratch_size = 0;
        cf -> first_block_in_scratch = -1;
        cf -> valid_scratch_bytes = 0;
#endif
        cf -> local_read_only = read_only;
        cf -> promote = promote;

#if( CACHE_STAT > 0 )
        init_cache_stat( & cf -> stat );
#endif
        
        rc = KFileSize( local, &cf -> local_size );
        if ( rc != 0 )
        {
            LOGERR( klogErr, rc, "cannot detect size of local file" );
        }
        else
        {
            bool fully_in_cache = false;
            
            rc = KFileSize( cf -> remote, &cf -> remote_size );
            if ( rc != 0 )
            {
                LOGERR( klogErr, rc, "cannot detect size of remote file" );
            }
            else
            {
                if ( cf -> local_size == 0 && ! cf -> local_read_only )
                {
                    /* the local file was just created... */
                    if ( cf -> remote_size == 0 )
                    {
                        rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );
                        LOGERR( klogErr, rc, "size of remote file is zero" );
                    }
                    else
                    {
                        rc = calculate_local_size_from_remote_size( cf );
                        if ( rc == 0 )
                        {
                            rc = KFileSetSize ( cf -> local, cf -> local_size );
                            if ( rc == 0 )
                            {
                                rc = init_new_local_file( cf );
                            }
                            else
                            {
                                PLOGERR( klogInt, ( klogInt, rc, "cannot size local file to $(l) bytes", "l=%lu", cf->local_size ) );
                            }
                        }
                    }
                }
                else
                    rc = verify_existing_local_file( cf, &fully_in_cache );
            }

            if ( rc == 0 && fully_in_cache && ! cf -> local_read_only && cf -> promote )
            {
                /* here is the up-front-test: the cache is complete and we have write access! */
                rc = promote_cache( cf );
                if ( rc == 0 )
                {
                    *tee = cf -> local;
                    free ( cf );
                    return rc;  /* here we return the truncated, promoted cache as tee-file ! */
                }
            }

            if ( rc == 0 )
            {
                if ( cf -> remote_size == 0 )
                {
                    cf -> remote_size = cf -> local_size;
                }
                
                /* now we have to AddRef() everything we hang on until the final release! */
                rc = KDirectoryAddRef ( cf -> dir );
                if ( rc == 0 )
                {
                    rc = KFileAddRef( cf -> remote );
                    if ( rc == 0 )
                    {
#if USE_BUFFER_POOL
                        rc = KQueueMake( &cf -> buffer_pool, 32 );
#endif
                        if ( rc == 0 )
                        {
                            rc = KFileInit( &cf -> dad, (const union KFile_vt *)&vtKCacheTeeFile, "KCacheTeeFile", path, true, false );
                            if ( rc == 0 )
                            {
                                /* the wrapper is ready to use now! */
                                *tee = ( const KFile * ) &cf -> dad;
                                return 0;
                            }
                            else
                            {
                                LOGERR( klogErr, rc, "cannot initialize KFile-structure" );
#if USE_BUFFER_POOL
                                KQueueRelease( cf -> buffer_pool );
#endif
                                /* TODO: check if we actually need to release cf->local here, since we never attached to it */
                                KFileRelease( cf -> local );
                                KFileRelease( cf -> remote );
                                KDirectoryRelease ( cf -> dir );
                            }
                        }
                        else
                        {
                            KFileRelease( cf -> remote );
                            KDirectoryRelease ( cf -> dir );
                        }
                    }
                    else
                        KDirectoryRelease ( cf -> dir );
                }
            }
        }
        free ( cf );
        /* if we arrived here, we do not have enough space on the local filesystem */
        rc = hand_out_remote_file_as_tee_file( tee, remote );
        LOGERR( klogInt, rc, "skipping the cache-tee completely" );
    }
    return rc;
}

static rc_t make_read_only_cache_tee( struct KDirectory *self,
    struct KFile const **tee, struct KFile const *remote, uint32_t blocksize, const char *path )
{
    const struct KFile * local;
    rc_t rc = KDirectoryOpenFileRead( self, &local, "%s.cache", path );
    if ( rc == 0 )
        rc = make_cache_tee( self, tee, remote, ( struct KFile * )local, blocksize, true, false, path );
    return rc;
}


static
rc_t KDirectoryVMakeCacheTeeInt ( struct KDirectory *self,
    struct KFile const **tee, struct KFile const *remote,
    uint32_t blocksize, const char *path, va_list args, bool promote )
{
    rc_t rc;
    if ( tee == NULL || remote == NULL )
        rc = RC ( rcFS, rcFile, rcAllocating, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcFS, rcFile, rcAllocating, rcSelf, rcNull );
        else if ( path == NULL )
            rc = RC ( rcFS, rcFile, rcAllocating, rcPath, rcNull );
        else if ( path [ 0 ] == 0 )
            rc = RC ( rcFS, rcFile, rcAllocating, rcPath, rcEmpty );
        else
        {
            char full [ 4096 ];

            rc = KDirectoryVResolvePath ( self, false, full, sizeof full, path, args );
            if ( rc != 0 )
            {
                PLOGERR( klogErr, ( klogErr, rc, "cannot resolve path of cache file '$(path)'", 
                        "path=%s", full ) );
            }
            else
            {
                if ( rc == 0 )
                {
                    /* it was possible to aquire the lock on the cache-file */
                    struct KFile * local;
                    rc = KDirectoryOpenFileSharedWrite( self, &local, true, "%s.cache", full );
                    if ( rc == 0 )
                    {
                        /* we have the exclusive rd/wr access to the cache file !*/
                        rc = make_cache_tee( self, tee, remote, local, blocksize, false, promote, full );
                    }
                    else if ( GetRCState( rc ) == rcNotFound )
                    {
                        rc = KDirectoryCreateFile( self, &local, true,
                            0664, kcmOpen | kcmParents, "%s.cache", full );
                        if ( rc == 0 )
                        {
                            /* we have the exclusive rd/wr access to the cache file !*/
                            rc = make_cache_tee( self, tee, remote, local, blocksize, false, promote, full );
                        }
                    }
                    else
                    {
                        /* we do not have the exclusive rd/wr access to the cache file !*/
                        rc = make_read_only_cache_tee( self, tee, remote, blocksize, full );
                    }
                }
                else if ( GetRCState ( rc ) == rcBusy )
                {
                    /* it was NOT possible to aquire the lock on the cache-file */
                    rc = make_read_only_cache_tee( self, tee, remote, blocksize, full );
                }
                else
                {
                    PLOGERR( klogErr, ( klogErr, rc, "cannot create lock-file '$(path).cache.lock'", 
                            "path=%s", full ) );
                }
            }
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC KDirectoryVMakeCacheTee ( struct KDirectory *self,
    struct KFile const **tee, struct KFile const *remote,
    uint32_t blocksize, const char *path, va_list args )
{
    return KDirectoryVMakeCacheTeeInt ( self, tee, remote, blocksize, path, args, false );
}


LIB_EXPORT rc_t CC KDirectoryMakeCacheTee ( struct KDirectory *self,
    struct KFile const **tee, struct KFile const *remote,
    uint32_t blocksize, const char *path, ... )
{
    rc_t rc;
    va_list args;
    va_start ( args, path );

    rc = KDirectoryVMakeCacheTee ( self, tee, remote, blocksize, path, args );

    va_end ( args );

    return rc;
}


LIB_EXPORT rc_t CC KDirectoryVMakeCacheTeePromote ( struct KDirectory *self,
    struct KFile const **tee, struct KFile const *remote,
    uint32_t blocksize, const char *path, va_list args )
{
    return KDirectoryVMakeCacheTeeInt ( self, tee, remote, blocksize, path, args, true );
}


LIB_EXPORT rc_t CC KDirectoryMakeCacheTeePromote ( struct KDirectory *self,
    struct KFile const **tee, struct KFile const *remote,
    uint32_t blocksize, const char *path, ... )
{
    rc_t rc;
    va_list args;
    va_start ( args, path );

    rc = KDirectoryVMakeCacheTeePromote ( self, tee, remote, blocksize, path, args );

    va_end ( args );

    return rc;
}


#if USE_32BIT_BITMAP_WORDS
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
#else
static uint64_t count_bits_in_bitmap( const uint64_t block_count, const uint64_t bitmap_bytes, const uint8_t volatile * bitmap )
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
#endif

LIB_EXPORT rc_t CC GetCacheCompleteness( const struct KFile * self, float * percent, uint64_t * bytes_in_cache )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC( rcFS, rcFile, rcValidating, rcSelf, rcNull );
    else
    {
        uint64_t local_size;
        if ( percent != NULL ) *percent = 0;
        if ( bytes_in_cache != NULL ) *bytes_in_cache = 0;
        rc = KFileSize( self, &local_size );
        if ( rc != 0 )
        {
            LOGERR( klogErr, rc, "cannot detect filesize when checking if cached file is complete" );
        }
        else
        {
            uint32_t block_size;
            rc = read_block_size( self, local_size, &block_size );
            if ( rc == 0 )
            {
                uint64_t content_size;
                rc = read_content_size( self, local_size, &content_size );
                /* create bitmap buffer */
                if ( rc == 0 )
                {
                    uint64_t block_count;
                    uint64_t bitmap_bytes;
                    rc = verify_file_structure( local_size, block_size, content_size, &block_count, &bitmap_bytes );
                    if ( rc == 0 )
                    {
#if USE_32BIT_BITMAP_WORDS
                        atomic32_t * bitmap = NULL;
#else
                        uint8_t volatile * bitmap = NULL;
#endif
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
LIB_EXPORT rc_t CC GetCacheTruncatedSize( const struct KFile * self, uint64_t * truncated_size )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC( rcFS, rcFile, rcValidating, rcSelf, rcNull );
    else if ( truncated_size == NULL )
        rc = RC( rcFS, rcFile, rcValidating, rcParam, rcNull );
    else
    {
        uint64_t local_size;
        *truncated_size = 0;
        rc = KFileSize( self, &local_size );
        if ( rc != 0 )
        {
            LOGERR( klogErr, rc, "cannot detect filesize when checking if cached file is complete" );
        }
        /* TODO: need to do a better constant tha a magic number */
        else if (local_size < 13)
        {
            rc = RC ( rcFS, rcFile, rcValidating, rcSize, rcInsufficient );
            LOGERR( klogErr, rc, "cannot use filesize of zero when checking if cached file is complete" );
        }
        else
        {
            uint32_t block_size;
            rc = read_block_size( self, local_size, &block_size );
            if ( rc == 0 )
            {
                uint64_t content_size;
                rc = read_content_size( self, local_size, &content_size );
                if ( rc == 0 )
                {
                    uint64_t block_count, bitmap_bytes;
                    rc = verify_file_structure( local_size, block_size, content_size, &block_count, &bitmap_bytes );
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

LIB_EXPORT rc_t CC Has_Cache_Zero_Blocks( const struct KFile * self, uint64_t * checked_blocks, uint64_t * empty_blocks )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC( rcFS, rcFile, rcValidating, rcSelf, rcNull );
    else if ( checked_blocks == NULL || empty_blocks == NULL )
        rc = RC( rcFS, rcFile, rcValidating, rcParam, rcNull );
    else
    {
        uint64_t local_size;
        *checked_blocks = 0;
        *empty_blocks = 0;
        rc = KFileSize( self, &local_size );
        if ( rc != 0 )
        {
            LOGERR( klogErr, rc, "cannot detect filesize when checking if cached file is complete" );
        }
        else
        {
            uint32_t block_size;
            rc = read_block_size( self, local_size, &block_size );
            if ( rc == 0 )
            {
                uint64_t content_size;
                rc = read_content_size( self, local_size, &content_size );
                /* create bitmap buffer */
                if ( rc == 0 )
                {
                    uint64_t block_count;
                    uint64_t bitmap_bytes;
                    rc = verify_file_structure( local_size, block_size, content_size, &block_count, &bitmap_bytes );
                    if ( rc == 0 )
                    {
#if USE_32BIT_BITMAP_WORDS
                        atomic32_t * bitmap = NULL;
#else
                        uint8_t volatile * bitmap = NULL;
#endif
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
                            /* loop through the bitmap for each bit set verify that the corresponding block is not zero */
                            uint8_t * buffer = malloc( block_size );
                            if ( buffer != NULL )
                            {
                                uint64_t idx;
                                for ( idx = 0; idx < block_count && rc == 0; ++idx )
                                {
                                    if ( IS_BITMAP_BIT( bitmap, idx ) )
                                    {
                                        size_t num_read;
                                        rc = KFileReadAll ( self, idx * block_size, buffer, block_size, &num_read );
                                        if ( rc == 0 )
                                        {
                                            ( *checked_blocks )++;
                                            if ( num_read > 0 )
                                            {
                                                size_t i, n;
                                                for ( i = 0, n = 0; i < num_read; ++i )
                                                    if ( buffer[ i ] != 0 ) n++;
                                                if ( n == num_read ) ( * empty_blocks )++;
                                            }
                                        }
                                    }
                                }
                                free( buffer );
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
 * reports in the boolean if the file is a cachetoofile and it is complete...
 *
 * KFile is the struct returned by KDirectoryMakeCacheTee()..
 */
LIB_EXPORT rc_t CC IsCacheTeeComplete( const struct KFile * self, bool * complete )
{
    rc_t rc = 0;
    if ( self == NULL || complete == NULL )
        rc = RC ( rcFS, rcFile, rcValidating, rcParam, rcNull );
    else
    {
        if ( &self->vt->v1 != &vtKCacheTeeFile )
            rc = RC ( rcFS, rcFile, rcValidating, rcParam, rcInvalid );
        else
        {
            struct KCacheTeeFile * ctf = ( struct KCacheTeeFile * )self;
            *complete = is_bitmap_full( ctf->bitmap, ctf->bitmap_bytes, ctf->block_count );
        }
    }
    return rc;
}

LIB_EXPORT bool CC KFileIsKCacheTeeFile( const struct KFile * self )
{
    return self != NULL && &self->vt->v1 == &vtKCacheTeeFile;
}
