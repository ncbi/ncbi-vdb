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
#include <stdio.h>

struct KCacheTeeFile;
#define KFILE_IMPL struct KCacheTeeFile
#include <kfs/impl.h>
#include <kfs/lockfile.h>

#include <klib/rc.h>
#include <klib/log.h>
#include <klib/out.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/checksum.h>

#include <kfs/cacheteefile.h>

#include <sysalloc.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


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
#define CACHE_TEE_REPORT 0
#define CACHE_TEE_LOCKING 0

typedef struct KCacheTeeFile
{
    KFile dad;
    const KFile * remote;
    KFile * local;
    KFile * logger;
    KDirectory * dir;
    KLockFile * lock;

    uint8_t * bitmap;
    uint8_t * scratch_buffer;

    uint64_t remote_size;
    uint64_t local_size;
    uint64_t block_count;
    uint64_t log_file_pos;
    uint64_t first_block_in_scratch;

    uint64_t bitmap_bytes;
    uint64_t scratch_size;
    uint64_t valid_scratch_bytes;
    uint32_t block_size;
    uint32_t cluster_factor;
    bool fully_in_cache;
    bool report;
    bool locking;
    bool local_read_only;
    char local_path [ 1 ];
} KCacheTeeFile;


const uint8_t BitNr2Mask[ 8 ] =
{
         /* 0  1  2  3   4   5   6    7  */
            1, 2, 4, 8, 16, 32, 64, 128
};

#define IS_CACHE_BIT( CacheFile, Block_Nr ) ( ( CacheFile->bitmap[ (Block_Nr) >> 3 ] & BitNr2Mask[ (Block_Nr) & 7 ] ) > 0 )

#define IS_BITMAP_BIT( BitMap, Block_Nr ) ( ( BitMap[ (Block_Nr) >> 3 ] & BitNr2Mask[ (Block_Nr) & 7 ] ) > 0 )

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


static rc_t create_bitmap_buffer( uint8_t ** bitmap, uint64_t bitmap_bytes )
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

static rc_t create_bitmap( KCacheTeeFile *self )
{
    return create_bitmap_buffer( &self->bitmap, self->bitmap_bytes );
}


static bool is_bitmap_full( const uint8_t * bitmap, uint64_t bitmap_bytes, uint64_t block_count )
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


static void report( const KCacheTeeFile * cf )
{
    OUTMSG(( "log  : %s\n", cf -> logger != NULL ? "YES" : "NO" ));
    OUTMSG(( "size : local=%,lu remote=%,lu\n", cf -> local_size, cf -> remote_size ));
    OUTMSG(( "block: size=%,u count=%,u\n", cf -> block_size, cf -> block_count ));
    OUTMSG(( "bitmp: bytes=%,u\n", cf -> bitmap_bytes ));
    OUTMSG(( "fully: %s\n", cf -> fully_in_cache ? "YES" : "NO" ));
}


static rc_t init_new_local_file( KCacheTeeFile * cf )
{
    rc_t rc = create_bitmap( cf );
    cf->fully_in_cache = false;
    if ( rc == 0 )
    {
        size_t written;
        uint64_t pos = cf -> remote_size;
        /* write the bitmap ( zero'd out ) into the local file */
        rc = KFileWriteAll ( cf -> local, pos, 
                             cf -> bitmap, cf -> bitmap_bytes, &written );
        if ( rc == 0 && written != cf -> bitmap_bytes )
        {
            rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );
            LOGERR( klogErr, rc, "no full initialization of local file bitmap" );
        }
        if ( rc == 0 && cf->report )
        {
            OUTMSG(( "init new cachefile: %u bytes of bitmaps written at pos %lu\n", written, pos ));
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
        if ( rc == 0 && cf->report )
        {
            OUTMSG(( "init new cachefile: %u bytes of remote size written at pos %lu ( remote-size = %lu )\n", written, pos, cf->remote_size ));
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
        if ( rc == 0 && cf->report )
        {
            OUTMSG(( "init new cachefile: %u bytes of block_size written at pos %lu ( block-size = %lu )\n", written, pos, cf->block_size ));
        }

    }
    if ( rc == 0 && cf -> report )
        report( cf );
    return rc;
}


static rc_t read_block_size( const struct KFile * self, uint64_t local_size, uint32_t *block_size, bool silent )
{
    size_t num_read;
    uint64_t pos = local_size - ( sizeof *block_size );
    /* read the block-size, compare to the requested block-size */
    rc_t rc =  KFileRead( self, pos, block_size, sizeof *block_size, &num_read );
    if ( rc != 0 )
    {
        if ( !silent )
        {
            LOGERR( klogErr, rc, "cannot read block-size from local file" );
        }
    }
    else if ( num_read != sizeof *block_size )
    {
        rc = RC ( rcFS, rcFile, rcValidating, rcParam, rcInvalid );
        if ( !silent )
        {
            LOGERR( klogErr, rc, "cannot read block-size from local file" );
        }
    }
    if ( rc == 0 && *block_size == 0 )
    {
        rc = RC ( rcFS, rcFile, rcValidating, rcParam, rcInvalid );
        if ( !silent )
        {
            LOGERR( klogErr, rc, "block-size read from local file is zero" );
        }
    }
    return rc;
}


static rc_t read_content_size( const struct KFile * self, uint64_t local_size, uint64_t *content_size, bool silent )
{
    size_t num_read;
    uint64_t pos = ( local_size - 4 ) - sizeof( *content_size );
    rc_t rc =  KFileRead ( self, pos, content_size, sizeof *content_size, &num_read );
    if ( rc != 0 )
    {
        if ( !silent )
        {
            LOGERR( klogErr, rc, "cannot read content-size from local file" );
        }
    }
    else if ( num_read != sizeof *content_size )
    {
        rc = RC ( rcFS, rcFile, rcValidating, rcParam, rcInvalid );
        if ( !silent )
        {
            LOGERR( klogErr, rc, "cannot read content-size from local file" );
        }
    }
    if ( rc == 0 && *content_size == 0 )
    {
        rc = RC ( rcFS, rcFile, rcValidating, rcParam, rcInvalid );
        if ( !silent )
        {
            LOGERR( klogErr, rc, "content-size read from local file is zero" );
        }
    }
    if ( rc == 0 && *content_size >= local_size )
    {
        rc = RC ( rcFS, rcFile, rcValidating, rcParam, rcInvalid );
        if ( !silent )
        {
            LOGERR( klogErr, rc, "content-size read from local file is invalid" );
        }
    }
    return rc;
}


static rc_t verify_file_structure( const uint64_t local_size, const uint32_t block_size, const uint64_t content_size,
                                   uint64_t * block_count, /* size_t */ uint64_t * bitmap_bytes, bool silent )
{
    rc_t rc = 0;
    uint64_t expected_size;

    *block_count = SIZE_2_BLOCK_COUNT( content_size, block_size );
    *bitmap_bytes = BITS_2_BYTES( *block_count );

    /* check if the values 'content-size' and 'block_size' result in the currect real file size */
    expected_size = content_size + *bitmap_bytes + sizeof ( local_size ) + sizeof ( block_size );
    if ( expected_size != local_size )
    {
        rc = RC ( rcFS, rcFile, rcValidating, rcParam, rcInvalid );
        if ( !silent )
        {
            LOGERR( klogErr, rc, "content-size/blocksize do not make sense for this file" );
        }
    }
    return rc;
}


static rc_t read_bitmap( KCacheTeeFile * cf )
{
    size_t num_read;
    /* read the bitmap from the local file */
    rc_t rc = KFileReadAll ( cf -> local, cf -> remote_size, cf -> bitmap, cf -> bitmap_bytes, &num_read );
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


static rc_t read_bitmap_partial( KCacheTeeFile * cf, uint64_t offset, uint64_t count )
{
    rc_t rc = 0;
    uint8_t * temp = malloc( count );
    if ( temp == NULL )
        rc = read_bitmap( cf );
    else
    {
        uint64_t ofs = cf -> remote_size + offset;
        size_t num_read;
        rc = KFileReadAll ( cf -> local, ofs, temp, count, &num_read );
        if ( rc == 0 )
        {
            uint64_t i;
            for ( i = 0; i < num_read; ++i )
                cf->bitmap[ offset + i ] |= temp[ i ];
        }
        free( ( void * ) temp );
    }
    return rc;
}


static rc_t verify_existing_local_file( KCacheTeeFile * cf, bool silent )
{
    /* size_t */ uint64_t bitmap_bytes;
    uint32_t block_size;
    uint64_t content_size, block_count;

    rc_t rc = read_block_size ( cf -> local, cf -> local_size, &block_size, silent );
    /* read content-size, compare to the content-size of the remote file */
    if ( rc == 0 )
        rc = read_content_size ( cf -> local, cf -> local_size, &content_size, silent );

    if ( rc == 0 && content_size != cf -> remote_size )
    {
        rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );
        PLOGERR( klogErr, ( klogErr, rc, "content-size in local file $(ls) does not match size of remote file $(rs)", 
                           "ls=%lu,rs=%lu", content_size, cf -> remote_size ) );
    }

    /* check if the local file has the right size ( content + bitmap + content_size + block_size )*/
    if ( rc == 0 )
        rc = verify_file_structure ( cf -> local_size, block_size, content_size, &block_count, &bitmap_bytes, silent );

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

    /* check if we have the whole remote file in cache */
    if ( rc == 0 )
        cf -> fully_in_cache = is_bitmap_full( cf -> bitmap, cf -> bitmap_bytes, cf -> block_count ); 

    if ( rc == 0 && cf -> report )
        report( cf );

    return rc;
}


LIB_EXPORT rc_t CC IsCacheFileComplete( const struct KFile * self, bool * is_complete, bool silent )
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
            if ( !silent )
            {
                LOGERR( klogErr, rc, "cannot detect filesize when checking if cached file is complete" );
            }
        }
        /* TODO: need to do a better constant tha a magic number */
        else if (local_size < 13)
        {
            rc = RC ( rcFS, rcFile, rcValidating, rcSize, rcInsufficient );
            if ( !silent )
            {
                LOGERR( klogErr, rc, "cannot use filesize of zero when checking if cached file is complete" );
            }
        }
        else
        {
            uint32_t block_size;
            rc = read_block_size( self, local_size, &block_size, silent );
            if ( rc == 0 )
            {
                uint64_t content_size;

                rc = read_content_size( self, local_size, &content_size, silent );
                /* create bitmap buffer */
                if ( rc == 0 )
                {
                    uint64_t block_count;
                    /* size_t */ uint64_t bitmap_bytes;
                    rc = verify_file_structure( local_size, block_size, content_size, &block_count, &bitmap_bytes, silent );
                    if ( rc == 0 )
                    {
                        uint8_t * bitmap = NULL;
                        rc = create_bitmap_buffer( &bitmap, bitmap_bytes );
                        if ( rc == 0 )
                        {
                            size_t num_read;
                            /* read the bitmap into the memory ... */
                            rc = KFileReadAll ( self, content_size, bitmap, bitmap_bytes, &num_read );
                            if ( rc != 0 )
                            {
                                if ( !silent )
                                    LOGERR( klogErr, rc, "cannot read bitmap from local file" );
                            }
                            else if ( num_read != bitmap_bytes )
                            {
                                rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );
                                if ( !silent )
                                {
                                    PLOGERR( klogErr, ( klogErr, rc, "cannot read $(ls) bitmap-bytes from local file, read $(rs) instead", 
                                                        "ls=%lu,rs=%lu", bitmap_bytes, num_read ));
                                }
                            }
                        }
                        if ( rc == 0 )
                            *is_complete = is_bitmap_full( bitmap, bitmap_bytes, block_count );

                        if ( bitmap != NULL )
                            free( bitmap );
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
            rc = read_block_size( self, local_size, &block_size, false );
            if ( rc == 0 )
            {
                uint64_t content_size;
                rc = read_content_size( self, local_size, &content_size, false );
                if ( rc == 0 )
                {
                    uint64_t block_count;
                    /* size_t */ uint64_t bitmap_bytes;
                    rc = verify_file_structure( local_size, block_size, content_size, &block_count, &bitmap_bytes, false );

                    /* truncate the file by setting the new (shorter) filesize */
                    if ( rc == 0 )
                        rc = KFileSetSize( self, content_size );
                }
            }
        }
    }
    return rc;
}


static rc_t promote_cache( KCacheTeeFile * cf )
{
    char cache_file_name [ 4096 ];
    char temp_file_name [ 4096 ];
    size_t num_writ;
    rc_t rc = string_printf ( cache_file_name, sizeof cache_file_name, &num_writ, "%s.cache", cf -> local_path );
    if ( rc == 0 )
        rc = string_printf ( temp_file_name, sizeof temp_file_name, &num_writ, "%s.cache.temp", cf -> local_path );

    /* (1) releaes open cache file ( windows cannot rename open files ) */
    if ( rc == 0 )
        rc = KFileRelease( cf -> local );

    /* (2) rename to temporary name */
    if ( rc == 0 )
    {
        cf -> local = NULL;
        rc = KDirectoryRename ( cf -> dir, true, cache_file_name, temp_file_name );
    }

    /* (3) open from temporary name */
    if ( rc == 0 )
        rc = KDirectoryOpenFileWrite( cf -> dir, &cf -> local, true, "%s", temp_file_name );

    /* (4) perform truncation */
    if ( rc == 0 )
        rc = TruncateCacheFile( cf -> local );
    
    /* (5) releaes open temp. cache file ( windows cannot rename open files ) */
    if ( rc == 0 )
        rc = KFileRelease( cf -> local );

    /* (6) rename to final filename ( windows cannot rename open files ) */
    if ( rc == 0 )
    {
        cf -> local = NULL;
        rc = KDirectoryRename ( cf -> dir, true, temp_file_name, cf -> local_path );
    }

    /* (6) open from final filename */
    if ( rc == 0 )
        rc = KDirectoryOpenFileWrite( cf -> dir, &cf -> local, true, "%s", cf -> local_path );

    return rc;
}


/* Destroy
 */
static rc_t CC KCacheTeeFileDestroy( KCacheTeeFile *self )
{
    bool promote_test;

    if ( self->report )
    {
        OUTMSG(( "\nDESTROY cacheteefile '%s'\n\n", self -> local_path ));
    }


    if ( CACHE_TEE_LOCKING > 0 )
        promote_test = ( !self -> local_read_only && self -> lock != NULL );
    else
        promote_test = ( !self -> local_read_only );

    if ( promote_test )
    {
        rc_t rc = IsCacheFileComplete ( self -> local, &self -> fully_in_cache, false );
        if ( rc == 0 && self -> fully_in_cache )
        {
            if ( self->report )
            {
                OUTMSG(( "\nPROMOTING cacheteefile '%s'\n\n", self -> local_path ));
            }
            promote_cache( self );
        }
    }

    if ( self->bitmap != NULL )
        free( self->bitmap );
    if ( self->scratch_buffer != NULL )
        free( self->scratch_buffer );

    KDirectoryRelease ( self->dir );
    KFileRelease ( self -> remote );
    KFileRelease ( self -> local );

    if ( self->locking )
        KLockFileRelease ( self -> lock );

    KFileRelease ( self -> logger );

    free ( self );
    return 0;
}



static rc_t log_to_file( KFile *log, uint64_t *log_file_pos,
                         uint64_t pos, size_t bsize, size_t num_read )
{
    char buffer[ 1024 ];
    size_t printf_written;
    rc_t rc = string_printf( buffer, sizeof buffer, &printf_written, 
                             "READ at %lu.%u read = %u\n",
                             pos, bsize, num_read );
    if ( rc == 0 )
    {
        size_t log_written;
        rc = KFileWriteAll ( log, *log_file_pos, buffer, printf_written, &log_written );
        if ( rc == 0 )
            *log_file_pos += log_written;
    }
    return rc;
}


static void set_bitmap( const KCacheTeeFile *cself, uint64_t start_block, uint64_t block_count )
{
    uint64_t block_idx, block_nr;
    for ( block_idx = 0, block_nr = start_block; 
          block_idx < block_count; 
          ++block_idx, ++block_nr )
    {
        cself->bitmap[ block_nr >> 3 ] |= BitNr2Mask[ block_nr & 0x07 ];
    }
}

static rc_t write_bitmap( const KCacheTeeFile *cself, uint64_t start_block, uint64_t block_count )
{
    size_t written;
    uint32_t start_block_byte = ( uint32_t ) ( start_block >> 3 );
    uint32_t end_block_byte = ( uint32_t ) ( ( start_block + block_count - 1 ) >> 3 );
    uint64_t pos = cself->remote_size + start_block_byte;
    size_t to_write = ( end_block_byte - start_block_byte ) + 1;
    rc_t rc = KFileWriteAll( cself->local, pos, &cself->bitmap[ start_block_byte ], to_write, &written );
    if ( rc != 0 )
    {
        PLOGERR( klogErr, ( klogErr, rc, "cannot write local-file-bitmap block $(sb).$(cn)", 
                           "sb=%lu,cn=%lu", start_block, block_count ) );
    }
    if ( rc == 0 && cself->report )
    {
        OUTMSG(( "writing bitmap: %u bytes written at pos %lu\n", written, pos ));
    }

    return rc;
}


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
        if ( rc == 0 && bytes_read == 0 )
		{
            if ( cself->report ) OUTMSG(( "0 bytes read from remote source, possible HUP, retrying\n" ));
            rc = KFileReadAll( cself->remote, pos, buffer, bsize, &bytes_read );
            if ( rc == 0 && bytes_read == 0 )
			{
                rc = RC ( rcFS, rcFile, rcReading, rcBuffer, rcEmpty );
            }
        }
        if ( rc == 0 )
        {
            if ( cself->report )
            {
                OUTMSG(( "reading remote data: %u bytes read at pos %lu\n", bytes_read, pos ));
            }

            if ( cself->local_read_only )
            {
                *num_read = bytes_read;
            }
            else
            {
                rc = KFileWriteAll( cself->local, pos, buffer, bytes_read, num_read );
                if ( rc == 0 && cself->report )
                {
                    OUTMSG(( "writing local data: %u bytes written at pos %lu\n", *num_read, pos ));
                }
            }
        }
    }
    return rc;
}


#if 0
static void clr_bitmap( const KCacheTeeFile *cself, uint64_t start_block, uint64_t block_count )
{
    uint64_t block_idx, block_nr;
    for ( block_idx = 0, block_nr = start_block; 
          block_idx < block_count; 
          ++block_idx, ++block_nr )
    {
        cself->bitmap[ block_nr >> 3 ] &= ~( BitNr2Mask[ block_nr & 0x07 ] );
    }
}

static uint64_t calc_req_blocks( uint64_t pos, uint64_t first_req_block, size_t bsize, uint32_t block_size )
{
    uint64_t temp;
    uint64_t res = pos;
    res += bsize;
    temp = first_req_block;
    temp *= block_size;
    res -= temp;
    res += block_size;
    res--;
    res /= block_size;
    return res;
}

static rc_t KCacheTeeFileRead_simple_cached( const KCacheTeeFile *cself, uint64_t pos,
                                             void *buffer, size_t bsize, size_t *num_read, uint64_t first_req_block )
{
    rc_t rc = 0;
    bool found_in_scratch = false;
    size_t to_read;
    size_t block_count = 1;
    uint64_t req_blocks = calc_req_blocks( pos, first_req_block, bsize, cself->block_size );

    /* we read as much as we have from the local cache, forcing the caller
       to eventually make another request ( the non-cached part of it ) afterwards */
    if ( req_blocks > 1 )
    {
        /* detect how many blocks are in cache after the first_req_block */
        uint64_t block = first_req_block;
        block_count = 0;
        do
        {
            block++;
            block_count++;
        }
        while ( ( block_count < req_blocks ) && ( IS_CACHE_BIT( cself, block ) ) );
    }

    /* check if we eventually have the requested bytes in the scratch-buffer */
    if ( cself -> first_block_in_scratch == first_req_block )
    {
        uint64_t first_pos_in_scratch = first_req_block;
        first_pos_in_scratch *= cself->block_size;
        if ( pos >= first_pos_in_scratch )
        {
            uint64_t last_pos_in_scratch = first_pos_in_scratch;
            last_pos_in_scratch += cself -> valid_scratch_bytes;
            found_in_scratch = ( pos < last_pos_in_scratch );
            if ( found_in_scratch )
            {
                uint64_t available_in_scratch = ( last_pos_in_scratch - pos );
                if ( bsize > available_in_scratch )
                    to_read = available_in_scratch;
                else
                    to_read = bsize;

                memmove ( buffer, &( cself->scratch_buffer[ pos - first_pos_in_scratch ] ), to_read );
                *num_read = to_read;
            }
        }
    }

    if ( !found_in_scratch )
    {
        /* now we have to check how much of the request can be satisfied from the local file */
        uint64_t reachable = first_req_block;
        reachable += block_count;
        reachable *= cself->block_size;
        reachable -= pos;

        /* are we requesting beyond the end of file? */
        if ( reachable >= bsize )
            to_read = check_rd_len( cself, pos, bsize );
        else
            to_read = check_rd_len( cself, pos, reachable );
        rc = KFileReadAll( cself->local, pos, buffer, to_read, num_read );
    }

    return rc;
}


static rc_t KCacheTeeFileRead_simple_not_cached( const KCacheTeeFile *cself, uint64_t pos,
                                                 void *buffer, size_t bsize, size_t *num_read, uint64_t first_req_block )
{
    rc_t rc;
    size_t to_read_remote;
    size_t block_count = 1;
    uint64_t req_blocks = calc_req_blocks( pos, first_req_block, bsize, cself->block_size );

    if ( req_blocks >  1 )
    {
        uint64_t block = first_req_block;
        block_count = 0;
        do
        {
            block++;
            block_count++;
        }
        while ( ( block_count < req_blocks ) && ( !( IS_CACHE_BIT( cself, block ) ) ) );
    }

    to_read_remote = ( block_count * cself->block_size );
    rc = resize_scratch_buffer( cself, to_read_remote );
    if ( rc == 0 )
    {
        size_t l_num_read;
        uint64_t block_start = first_req_block;
        block_start *= cself->block_size;
        to_read_remote = check_rd_len( cself, block_start, to_read_remote );
        rc = rd_remote_wr_local( cself, block_start, cself->scratch_buffer, to_read_remote, &l_num_read );
        if ( rc == 0 )
        {
            ( ( KCacheTeeFile * )cself ) -> first_block_in_scratch = first_req_block;
            ( ( KCacheTeeFile * )cself ) -> valid_scratch_bytes = l_num_read;
            if ( !cself->local_read_only )
            {
                set_bitmap( cself, first_req_block, block_count );
                rc = write_bitmap( cself, first_req_block, block_count );
            }
        }
        if ( rc == 0 )
        {
            /* what we have to return to the caller is somewhere in the scratch_buffer */
            uint64_t offset = pos;
            offset -= block_start;

            /* how much we can copy depends on how much was not in cache,
               and how much was requested in the first place... */
            if ( l_num_read >= offset )
            {
                l_num_read -= offset;
                if ( l_num_read > bsize )
                    l_num_read = bsize;
                memmove ( buffer, &( cself->scratch_buffer[ offset ] ), l_num_read );
            }
            else
            {
                l_num_read = 0;
            }
            *num_read = l_num_read;
        }
    }
    return rc;
}

static rc_t KCacheTeeFileRead_simple( const KCacheTeeFile *cself, uint64_t pos,
                                      void *buffer, size_t bsize, size_t *num_read )
{
    rc_t rc;
    uint64_t first_req_block = pos;
    first_req_block /= cself->block_size;

    *num_read = 0;
    if ( cself->report )
        OUTMSG(( "\nREQUEST '%s': %,lu .[%,lu] ( first_req_block=%,lu )\n",
                 cself->local_path, pos, bsize, first_req_block ));

    /* "simple" strategy, read only that much as requested... */
    if ( IS_CACHE_BIT( cself, first_req_block ) )
        rc = KCacheTeeFileRead_simple_cached( cself, pos, buffer, bsize, num_read, first_req_block );
    else
    {
        if ( cself->locking )
        {
            rc = KCacheTeeFileRead_simple_not_cached( cself, pos, buffer, bsize, num_read, first_req_block );
        }
        else
        {
            uint64_t offset = ( first_req_block >> 3 );
            uint64_t count = ( ( bsize /  cself->block_size ) >> 3 );
            if ( count < 1 ) count = 1;
            rc = read_bitmap_partial( ( KCacheTeeFile * )cself, offset, count );
            if ( rc == 0 )
            {
                if ( IS_CACHE_BIT( cself, first_req_block ) )
                    rc = KCacheTeeFileRead_simple_cached( cself, pos, buffer, bsize, num_read, first_req_block );
                else
                    rc = KCacheTeeFileRead_simple_not_cached( cself, pos, buffer, bsize, num_read, first_req_block );
            }
        }
    }

    if ( cself->logger != NULL )
    {
        KCacheTeeFile * self = ( KCacheTeeFile * ) cself;
        log_to_file( self->logger, &self->log_file_pos, pos, bsize, *num_read );
    }

	/* test that the buffer does not contain zero bytes ... */
	{
		uint32_t * wb = buffer;
		if ( wb[ 0 ] == 0 && wb[ 1 ] == 0 )
		{
			rc = KCacheTeeFileRead_simple_not_cached( cself, pos, buffer, bsize, num_read, first_req_block );
		}
	}
	
    return rc;
}

#endif

static rc_t KCacheTeeFileRead_simple2( const KCacheTeeFile *cself, uint64_t pos,
                                      void *buffer, size_t bsize, size_t *num_read )
{
    rc_t rc=0;
    uint64_t block = pos / cself->block_size;
    size_t   offset = pos % cself->block_size;
    size_t   to_read_total = bsize;
    
    if ( cself->report )
        OUTMSG(( "\nREQUEST '%s': %,lu .[%,lu] ( first_req_block=%,lu )\n",
                 cself->local_path, pos, bsize, block ));
    *num_read = 0;
    rc = resize_scratch_buffer( cself, cself->block_size );

    while(rc == 0 && to_read_total > 0){
        int64_t salvage_block=-1;
        size_t to_read = cself->block_size - offset;
        if(to_read > to_read_total) to_read = to_read_total;

        if(cself->valid_scratch_bytes >= offset+to_read && cself -> first_block_in_scratch == block ){
            memcpy(buffer,cself->scratch_buffer+offset,to_read);
            /*** move source counters **/
            offset += to_read;
            block  += offset / cself->block_size;
            offset %= cself->block_size;
            /*** move output counters **/       
            to_read_total -= to_read;
            *num_read += to_read;
            buffer = ((char*)buffer) + to_read;
        } else if (IS_CACHE_BIT(cself,block)){
            uint64_t fpos = block * cself->block_size;
            int64_t  fbsize = cself->remote_size - fpos;
            size_t   nread=0;

            if(fbsize > cself->block_size) fbsize = cself->block_size;

            rc = KFileReadAll( cself->local, fpos, cself->scratch_buffer, fbsize, &nread );
            if(rc == 0){
                int i;
                uint64_t *b = (uint64_t*)cself->scratch_buffer;
                ( ( KCacheTeeFile * )cself ) -> first_block_in_scratch = block;
                ( ( KCacheTeeFile * )cself ) -> valid_scratch_bytes = nread;
                if(block != salvage_block){ /** check for fully space page, but don't do it in infinite loop **/
                    for(i=0;i<nread/sizeof(*b) && b[i]==0;i++){} 
                    if(i==nread/sizeof(*b)){
                        if ( cself->report ) OUTMSG(( "Cached page is filled with zeros, possibly a sparse page. Attempting to reload from remote source\n"));
                        rc = rd_remote_wr_local( cself, block*cself->block_size, cself->scratch_buffer, fbsize, &nread );
                        if(rc == 0) salvage_block = block;
                    } else {
                        salvage_block = -1;
                    }
                }
            }
        } else {
            uint64_t fpos = block * cself->block_size;
            int64_t  fbsize = cself->remote_size - fpos;
            size_t   nread=0;

            if(fbsize > cself->block_size) fbsize = cself->block_size;

            if(!cself->locking){
                rc = read_bitmap_partial( ( KCacheTeeFile * )cself, block >> 3, 1 );
                if (IS_CACHE_BIT(cself,block)) /** bit was recently set by parallel activity **/
                        continue; /** will pick it up next cycle ***/
            }
            rc = rd_remote_wr_local( cself, fpos, cself->scratch_buffer, fbsize, &nread );
            if(rc == 0 ){
                ( ( KCacheTeeFile * )cself ) -> first_block_in_scratch = block;
                ( ( KCacheTeeFile * )cself ) -> valid_scratch_bytes = nread;
                if ( !cself->local_read_only ) {
                    set_bitmap( cself, block, 1 );
                    rc = write_bitmap( cself, block, 1 );
                }
            }
        }

    }

    if ( cself->logger != NULL )
    {
        KCacheTeeFile * self = ( KCacheTeeFile * ) cself;
        log_to_file( self->logger, &self->log_file_pos, pos, bsize, *num_read );
    }

    return rc;
}


/**********************************************************************************************
    try #3
**********************************************************************************************/
#if 0
static rc_t KCacheTeeFileRead_3( const KCacheTeeFile *cself, uint64_t pos,
                                 void * buffer, size_t bsize, size_t *num_read )
{
	rc_t rc = 0;
	uint64_t i_pos = pos;
	uint8_t * i_buffer = buffer;
	size_t still_to_read = bsize;
	size_t total_read = 0;
	size_t bytes_read;

	if ( i_pos + still_to_read > cself -> remote_size )
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
					clr_bitmap( cself, block, 1 );
					rc = write_bitmap( cself, block, 1 );
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
						still_to_read = 0;	/* terminate the loop, return as much as read so far, return rc ==0 */
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
								rc = write_bitmap( cself, block, 1 );
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
							still_to_read = 0;	/* terminate the loop, return as much as read so far, return rc ==0 */
						}
						else
						{
							/* write it to the local file */
							size_t bytes_written;
							rc = KFileWriteAll( cself->local, block_pos, cself->scratch_buffer, bytes_read, &bytes_written );
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
									rc = write_bitmap( cself, block, 1 );
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
	/* return KCacheTeeFileRead_3( cself, pos, buffer, bsize, num_read ); */
    return KCacheTeeFileRead_simple2( cself, pos, buffer, bsize, num_read );
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


static rc_t make_cache_tee( struct KDirectory *self, struct KFile const **tee,
    struct KFile const *remote, struct KFile *local, struct KFile *logger, KLockFile *lock,
    uint32_t blocksize, uint32_t cluster, bool report, bool locking, const char *path )
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
        cf -> logger = logger;
        cf -> lock = lock;
        cf -> report = report;
        cf -> locking = locking;
        cf -> cluster_factor = cluster;
        cf -> local_read_only = !( local -> write_enabled );
        cf -> block_size = ( blocksize > 0 ) ? blocksize : CACHE_TEE_DEFAULT_BLOCKSIZE;
        cf -> bitmap = NULL;
        cf -> scratch_buffer = NULL;
        cf -> scratch_size = 0;
        cf -> first_block_in_scratch = 0;
        cf -> valid_scratch_bytes = 0;

        rc = KFileSize( local, &cf -> local_size );
        if ( rc != 0 )
        {
            LOGERR( klogErr, rc, "cannot detect size of local file" );
        }
        else
        {
            bool promote = false;

            rc = KFileSize( cf -> remote, &cf -> remote_size );
            if ( rc != 0 )
            {
                LOGERR( klogErr, rc, "cannot detect size of remote file" );
            }
            else
            {
                if ( cf -> local_size == 0 )
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
                    rc = verify_existing_local_file( cf, false );
            }

            if ( locking )
                promote = ( rc == 0 && cf -> fully_in_cache && ! cf -> local_read_only && cf -> lock != NULL );
            else
                promote = ( rc == 0 && cf -> fully_in_cache && ! cf -> local_read_only );

            if ( promote )
            {
                /* here is the up-front-test: the cache is complete and we have write access! */
                rc = promote_cache( cf );
                if ( rc == 0 )
                {
                    *tee = cf -> local;

                    if ( locking )
                        KLockFileRelease ( cf -> lock );

                    free ( cf );
                    return rc;  /* here we return the truncated, promoted cache as tee-file ! */
                }
            }

            if ( rc == 0 )
            {
                if ( cf -> remote_size == 0 )
                    cf -> remote_size = cf -> local_size;

                /* now we have to AddRef() everything we hang on until the final release! */
                rc = KDirectoryAddRef ( cf -> dir );
                if ( rc == 0 )
                {
                    rc = KFileAddRef( cf -> remote );
                    if ( rc == 0 )
                    {
                        /* rc = KFileAddRef( cf -> local ); */
                        if ( rc == 0 )
                        {
                            if ( cf -> logger != NULL )
                                rc = KFileAddRef( cf -> logger );
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
                                    KFileRelease( cf -> logger );
                                    KFileRelease( cf -> local );
                                    KFileRelease( cf -> remote );
                                    KDirectoryRelease ( cf -> dir );
                                }
                            }
                            else
                            {
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
    }
    return rc;
}


static rc_t make_read_only_cache_tee( struct KDirectory *self,
    struct KFile const **tee, struct KFile const *remote, struct KFile *logger,
    uint32_t blocksize, uint32_t cluster, bool report, const char *path )
{
    const struct KFile * local;
    rc_t rc = KDirectoryOpenFileRead( self, &local, "%s.cache", path );
    if ( rc == 0 )
    {
        if ( report )
            KOutMsg( "successfuly opened cache file '%s.cache' in read/only-mode\n",
                      path );
        rc = make_cache_tee( self, tee, remote, ( struct KFile * )local, logger, NULL,
                             blocksize, cluster, report, false, path );
    }
    else
    {
        /* we cannot even open the local cache in read-only mode,
           we give up - and return a reference to the remote file... */
        rc = KFileAddRef( remote );
        if ( rc == 0 )
        {
            if ( report )
                KOutMsg( "cannot open cache file '%s.cache' in read/only-mode ( returning remote-file )\n",
                          path );
            *tee = remote;
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC KDirectoryVMakeCacheTee ( struct KDirectory *self,
    struct KFile const **tee, struct KFile const *remote, struct KFile *logger,
    uint32_t blocksize, uint32_t cluster, bool report, const char *path, va_list args )
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
            bool locking = ( CACHE_TEE_LOCKING > 0 );

            rc = KDirectoryVResolvePath ( self, false, full, sizeof full, path, args );
            if ( rc != 0 )
            {
                PLOGERR( klogErr, ( klogErr, rc, "cannot resolve path of cache file '$(path)'", 
                        "path=%s", full ) );
            }
            else
            {
                KLockFile *lock = NULL;

                if ( locking )
                {
                    if ( report )
                        KOutMsg( "CREATE CacheTeeFile for '%s'\n", full );
                    rc = KDirectoryCreateLockFile ( self, &lock, "%s.cache.lock", full );
                }

                if ( rc == 0 )
                {
                    /* it was possible to aquire the lock on the cache-file */
                    struct KFile * local;

                    if ( locking && report )
                        KOutMsg( "successfuly aquired lockfile '%s.cache.lock'\n", full );

                    rc = KDirectoryOpenFileWrite( self, &local, true, "%s.cache", full );
                    if ( rc == 0 )
                    {
                        if ( report )
                            KOutMsg( "successfuly opened '%s.cache'\n", full );

                        /* we have the exclusive rd/wr access to the cache file !*/
                        rc = make_cache_tee( self, tee, remote, local, logger, lock,
                                             blocksize, cluster, report, locking, full );
                    }
                    else if ( GetRCState( rc ) == rcNotFound )
                    {
                        rc = KDirectoryCreateFile( self, &local, true,
                            0664, kcmOpen | kcmParents, "%s.cache", full );
                        if ( rc == 0 )
                        {
                            if ( report )
                                KOutMsg( "cache-file '%s.cache' created with rd/wr-access\n", full );
                            /* we have the exclusive rd/wr access to the cache file !*/
                            rc = make_cache_tee( self, tee, remote, local, logger, lock,
                                                 blocksize, cluster, report, locking,  full );
                        }
                    }
                    else
                    {
                        if ( report )
                            KOutMsg( "no rd/wr access to cache-file '%s.cache' rc=%R\n", full, rc );

                        /* we do not have the exclusive rd/wr access to the cache file !*/

                        if ( locking )
                            rc = KLockFileRelease ( lock );

                        if ( rc == 0 )
                            rc = make_read_only_cache_tee( self, tee, remote, logger,
                                    blocksize, cluster, report, full );
                    }
                }
                else if ( GetRCState ( rc ) == rcBusy )
                {
                    if ( report )
                        KOutMsg( "failed to aquired lockfile '%s.cache.lock'\n", full );

                    /* it was NOT possible to aquire the lock on the cache-file */
                    rc = make_read_only_cache_tee( self, tee, remote, logger,
                            blocksize, cluster, report, full );
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


LIB_EXPORT rc_t CC KDirectoryMakeCacheTee ( struct KDirectory *self,
    struct KFile const **tee, struct KFile const *remote, struct KFile *logger,
    uint32_t blocksize, uint32_t cluster, bool report, const char *path, ... )
{
    rc_t rc;
    va_list args;
    va_start ( args, path );

    rc = KDirectoryVMakeCacheTee ( self, tee, remote, logger, blocksize, cluster, report, path, args );

    va_end ( args );

    return rc;
}


LIB_EXPORT rc_t CC GetCacheCompleteness( const struct KFile * self, float * percent, uint64_t * bytes_in_cache )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC( rcFS, rcFile, rcValidating, rcSelf, rcNull );
    else
    {
        uint64_t local_size;
		if ( percent != NULL ) *percent = 0;
        rc = KFileSize( self, &local_size );
        if ( rc != 0 )
        {
            LOGERR( klogErr, rc, "cannot detect filesize when checking if cached file is complete" );
        }
        else
        {
            uint32_t block_size;
            rc = read_block_size( self, local_size, &block_size, false );
            if ( rc == 0 )
            {
                uint64_t content_size;
                rc = read_content_size( self, local_size, &content_size, false );
                /* create bitmap buffer */
                if ( rc == 0 )
                {
                    uint64_t block_count;
                    /* size_t */ uint64_t bitmap_bytes;
                    rc = verify_file_structure( local_size, block_size, content_size, &block_count, &bitmap_bytes, false );
                    if ( rc == 0 )
                    {
                        uint8_t * bitmap = NULL;
                        rc = create_bitmap_buffer( &bitmap, bitmap_bytes );
                        if ( rc == 0 )
                        {
                            size_t num_read;
                            /* read the bitmap into the memory ... */
                            rc = KFileReadAll ( self, content_size, bitmap, bitmap_bytes, &num_read );
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
                            uint64_t idx, in_cache = 0;
                            for ( idx = 0; idx < block_count; ++idx )
                            {
                                if ( IS_BITMAP_BIT( bitmap, idx ) )
                                    in_cache++;
                            }
							
                            if ( in_cache > 0 && block_count > 0 )
                            {
                                float res = (float) in_cache;
                                res *= 100;
                                res /= block_count;
                                if ( percent != NULL ) ( *percent ) = res;
								if ( bytes_in_cache != NULL ) ( *bytes_in_cache ) = ( in_cache * block_count );
                            }
                        }
                        if ( bitmap != NULL )
                            free( bitmap );
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
LIB_EXPORT rc_t CC GetCacheTruncatedSize( const struct KFile * self, uint64_t * truncated_size, bool silent )
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
            if ( !silent )
            {
                LOGERR( klogErr, rc, "cannot detect filesize when checking if cached file is complete" );
            }
        }
        /* TODO: need to do a better constant tha a magic number */
        else if (local_size < 13)
        {
            rc = RC ( rcFS, rcFile, rcValidating, rcSize, rcInsufficient );
            if ( !silent )
            {
                LOGERR( klogErr, rc, "cannot use filesize of zero when checking if cached file is complete" );
            }
        }
        else
        {
            uint32_t block_size;
            rc = read_block_size( self, local_size, &block_size, true );
            if ( rc == 0 )
            {
                uint64_t content_size;
                rc = read_content_size( self, local_size, &content_size, true );
                if ( rc == 0 )
                {
                    uint64_t block_count;
                    /* size_t */ uint64_t bitmap_bytes;
                    rc = verify_file_structure( local_size, block_size, content_size, &block_count, &bitmap_bytes, true );
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
            rc = read_block_size( self, local_size, &block_size, false );
            if ( rc == 0 )
            {
                uint64_t content_size;
                rc = read_content_size( self, local_size, &content_size, false );
                /* create bitmap buffer */
                if ( rc == 0 )
                {
                    uint64_t block_count;
                    uint64_t bitmap_bytes;
                    rc = verify_file_structure( local_size, block_size, content_size, &block_count, &bitmap_bytes, false );
                    if ( rc == 0 )
                    {
                        uint8_t * bitmap = NULL;
                        rc = create_bitmap_buffer( &bitmap, bitmap_bytes );
                        if ( rc == 0 )
                        {
                            size_t num_read;
                            /* read the bitmap into the memory ... */
                            rc = KFileReadAll ( self, content_size, bitmap, bitmap_bytes, &num_read );
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
                            free( bitmap );
                    }
                }
            }
        }
	
	}
	return rc;
}
