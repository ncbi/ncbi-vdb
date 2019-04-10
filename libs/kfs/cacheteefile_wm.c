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

/*--------------------------------------------------------------------------
KCacheTeeFileWM stands for CacheTeeFile-WaterMark.

This file is process local.
The file has one writer but can server multiple readers.
The location of the file is in /tmp

The strategy is to read the whole file up to the requested position in
large ( x MB ? ) blocks, blocking the caller, then return the requested data.

-------------------------------------------------------------------------- */

#include <kfs/extern.h>

struct KCacheTeeFileWM;
#define KFILE_IMPL struct KCacheTeeFileWM
#include <kfs/impl.h>

#include <klib/rc.h>
#include <klib/log.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/time.h>

#include <kfs/cacheteefile_wm.h>

#include <kfs/defs.h>

#include <sysalloc.h>
#include <stdlib.h>

#define DFLT_BLOCK_SIZE ( 1024 * 1024 * 32 )
#define MIN_BLOCK_SIZE ( 64 * 1024 )

typedef struct KCacheTeeFileWM
{
    KFile dad;
    const KFile * wrapped;                  /* the file we are wrapping */
    KFile * cache;                          /* the cache-file */

    uint8_t * block;
    uint64_t wrapped_size;                  /* the size of the wrapped file */
    uint32_t block_size;                    /* how big is a block ( aka 1 bit in the bitmap )*/
    
    bool read_only;
} KCacheTeeFileWM;


/**********************************************************************************************
    START vt-functions
**********************************************************************************************/
static rc_t CC KCacheTeeFileWMDestroy( KCacheTeeFileWM * self )
{
    rc_t rc;
    
    if ( self -> block != NULL )
    {
        free( ( void * ) self -> block );
        self -> block = NULL;
    }

    rc = KFileRelease ( self -> wrapped );
    if ( rc != 0 )
    {
        LOGERR( klogInt, rc, "Error releasing wrapped file" );
    }
    
    rc = KFileRelease ( self -> cache );
    {
        LOGERR( klogInt, rc, "Error releasing cache file" );
    }
    
    free ( ( void * ) self );
    return 0;
}

static struct KSysFile* KCacheTeeFileWMGetSysFile( const KCacheTeeFileWM * self,
    uint64_t  *offset )
{
    * offset = 0;
    return NULL;
}


static rc_t KCacheTeeFileWMRandomAccess( const KCacheTeeFileWM * self )
{
    return 0;
}


static rc_t KCacheTeeFileWMSize( const KCacheTeeFileWM  *self, uint64_t * size )
{
    *size = self -> wrapped_size;
    return 0;
}


static rc_t KCacheTeeFileWMSetSize( KCacheTeeFileWM * self, uint64_t size )
{
    return RC ( rcFS, rcFile, rcUpdating, rcFile, rcReadonly );
}

static rc_t KCacheTeeFileWMRead_rw ( const KCacheTeeFileWM *cself,
    uint64_t pos, void * buffer, size_t bsize, size_t * num_read )
{
    uint64_t r_pos;
    /* first we try to detect how big the cache-file is */
    rc_t rc = KFileSize ( cself -> cache, &r_pos );
    if ( rc != 0 )
    {
        LOGERR( klogErr, rc, "cannot detect size of cache-file" );
        /* if that is not possible, we use the wrapped file instead - no caching */
        rc = KFileReadAll ( cself -> wrapped, pos, buffer, bsize, num_read );
    }
    else
    {
        if ( pos < r_pos )
        {
            /* we can at least use some part of the cache to read from,
               this might return less bytes than requested, let the caller
               ask for more ... */
            rc = KFileReadAll ( cself -> cache, pos, buffer, bsize, num_read );
        }
        else
        {
            /* requested range is not in the cache, read from wrapped instead,
               but store in cache, use large reads... */
            bool done = false;
            while ( !done )
            {
                size_t r_num_read;
                rc = KFileReadAll ( cself -> wrapped, r_pos, cself -> block,
                                    cself -> block_size, &r_num_read );
                if ( rc == 0 )
                {
                    size_t r_num_written;
                    rc = KFileWriteAll ( cself -> cache, r_pos, ( const void * ) cself -> block, r_num_read, &r_num_written );
                    if ( pos < ( r_pos + r_num_read ) )
                    {
                        /* we have at least a part of the requested data in memory.. */
                        done = true;
                        uint64_t offset = r_pos - pos;                            
                        uint8_t * src = cself -> block;
                        size_t count = r_num_read - offset;
                        src += offset;
                        memmove( buffer, src, count );
                        *num_read = count;
                    }
                    else
                    {
                        /* continue reading towards the watermark ... */
                        if ( rc == 0 && r_num_read == r_num_written )
                        {
                            r_pos += r_num_read;
                        }
                        else
                        {
                            /* we are not able to write to the cache-file, and the requested
                               range is not in the block-buffer: fall back to reading
                               from the wrapped file... */
                            rc = KFileReadAll ( cself -> wrapped, pos, buffer, bsize, num_read );
                            done = true;
                        }
                    }
                }
                else
                {
                    /* there is nothing we can do if we cannot read from the wrapped
                       file, we are done, return the error code to the caller */
                    done = true;
                }
            }
        }
    }
    return rc;
}

static rc_t KCacheTeeFileWMRead_ro ( const KCacheTeeFileWM *cself,
    uint64_t pos, void * buffer, size_t bsize, size_t * num_read )
{
    uint64_t cache_size;
    rc_t rc = KFileSize ( cself -> cache, &cache_size );
    if ( rc != 0 )
    {
        LOGERR( klogErr, rc, "cannot detect size of cache-file" );
        rc = KFileReadAll ( cself -> wrapped, pos, buffer, bsize, num_read );
    }
    else
    {
        if ( pos < cache_size )
        {
            /* we can at least use some part of the cache to read from */
            rc = KFileReadAll ( cself -> cache, pos, buffer, bsize, num_read );
        }
        else
        {
            /* requested range is not in the cache, read from wrapped instead */
            rc = KFileReadAll ( cself -> wrapped, pos, buffer, bsize, num_read );
        }
    }
    return rc;
}
        
static rc_t KCacheTeeFileWMWrite( KCacheTeeFileWM *self, uint64_t pos,
    const void * buffer, size_t size, size_t * num_writ )
{
    return RC ( rcFS, rcFile, rcUpdating, rcInterface, rcUnsupported );
}

/**********************************************************************************************
    END vt-functions
**********************************************************************************************/

static KFile_vt_v1 vtKCacheTeeFile_WM_rw =
{
    /* version 1.0 */
    1, 0,

    /* start minor version 0 methods */
    KCacheTeeFileWMDestroy,
    KCacheTeeFileWMGetSysFile,
    KCacheTeeFileWMRandomAccess,
    KCacheTeeFileWMSize,
    KCacheTeeFileWMSetSize,
    KCacheTeeFileWMRead_rw,
    KCacheTeeFileWMWrite
    /* end minor version 0 methods */
};

static KFile_vt_v1 vtKCacheTeeFile_WM_ro =
{
    /* version 1.0 */
    1, 0,

    /* start minor version 0 methods */
    KCacheTeeFileWMDestroy,
    KCacheTeeFileWMGetSysFile,
    KCacheTeeFileWMRandomAccess,
    KCacheTeeFileWMSize,
    KCacheTeeFileWMSetSize,
    KCacheTeeFileWMRead_ro,
    KCacheTeeFileWMWrite
    /* end minor version 0 methods */
};

static uint32_t normalize_block_size( uint32_t block_size )
{
    uint32_t res = block_size;
    if ( res == 0 )
        res = DFLT_BLOCK_SIZE;
    else
        res &= 0x0FFFFFF00;
    if ( res < MIN_BLOCK_SIZE )
        res = MIN_BLOCK_SIZE;
    return res;
}

static rc_t finish_rw( struct KFile const **tee,
                       struct KFile const * to_wrap,
                       struct KFile * cache,
                       const char * resolved_path,
                       uint64_t wrapped_size,
                       uint32_t block_size )
{
    rc_t rc = KFileAddRef ( to_wrap );
    if ( rc != 0 )
    {
        LOGERR( klogErr, rc, "cannot add-ref the wrapped file" );
    }
    else
    {
        KCacheTeeFileWM * obj = malloc ( sizeof * obj );
        if ( obj == NULL )
        {
            rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
            LOGERR( klogErr, rc, "cannot allocate cache-tee-struct" );
        }
        else
        {
            obj -> wrapped = to_wrap;
            obj -> cache = cache;
            obj -> wrapped_size = wrapped_size;
            obj -> block_size = normalize_block_size( block_size );
            obj -> read_only = false;

            obj -> block = malloc( obj -> block_size );
            if ( obj -> block == NULL )
            {
                rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
                LOGERR( klogErr, rc, "cannot allocate cache-block" );
                free( ( void * ) obj );
            }
            else
            {
                rc = KFileInit ( &obj -> dad,
                                 ( const union KFile_vt * ) &vtKCacheTeeFile_WM_rw,
                                 "KCacheTeeFileWM",
                                 resolved_path,
                                 true,
                                 false );
                if ( rc != 0 )
                {
                    LOGERR( klogErr, rc, "cannot initialize KFile" );
                    free( ( void * ) obj -> block );
                    free( ( void * ) obj );
                }
                else
                {
                    *tee = ( const KFile * ) &obj -> dad;
                }
            }
        }
    }
    return rc;

}

static rc_t finish_ro( struct KFile const **tee,
                       struct KFile const * to_wrap,
                       struct KFile * cache,
                       const char * resolved_path,
                       uint64_t wrapped_size,
                       uint32_t block_size )
{
    rc_t rc = KFileAddRef ( to_wrap );
    if ( rc != 0 )
    {
        LOGERR( klogErr, rc, "cannot add-ref the wrapped file" );
    }
    else
    {
        KCacheTeeFileWM * obj = malloc ( sizeof * obj );
        if ( obj == NULL )
        {
            rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
            LOGERR( klogErr, rc, "cannot allocate cache-tee-struct" );
        }
        else
        {
            obj -> wrapped = to_wrap;
            obj -> cache = cache;
            obj -> wrapped_size = wrapped_size;
            obj -> block_size = normalize_block_size( block_size );
            obj -> read_only = true;
            obj -> block = NULL;

            rc = KFileInit ( &obj -> dad,
                             ( const union KFile_vt * ) &vtKCacheTeeFile_WM_ro,
                             "KCacheTeeFileWM",
                             resolved_path,
                             true,
                             false );
            if ( rc != 0 )
            {
                LOGERR( klogErr, rc, "cannot initialize KFile" );
                free( ( void * ) obj );
            }
            else
            {
                *tee = ( const KFile * ) &obj -> dad;
            }
        }
    }
    return rc;
}

static rc_t create_new_tee( struct KDirectory * self,
                            struct KFile const **tee,
                            struct KFile const * to_wrap,
                            const char * resolved_path,
                            uint64_t wrapped_size,
                            uint32_t block_size )
{
    KFile * cache;
    rc_t rc = KDirectoryCreateFile ( self,
                                     &cache,
                                     true,
                                     0664,
                                     kcmOpen | kcmParents,
                                     "%s.cache",
                                     resolved_path );
    if ( rc != 0 )
    {
        LOGERR( klogErr, rc, "cannot create cache-file" );
    }
    else
    {
        rc = finish_rw( tee, to_wrap, cache, resolved_path,
                        wrapped_size, block_size );
    }
    return 0;
}


LIB_EXPORT rc_t CC KDirectoryVMakeCacheTeeWM ( struct KDirectory * self,
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
        uint64_t wrapped_size;
        rc = KFileSize ( to_wrap, &wrapped_size );
        if ( rc != 0 )
        {
            LOGERR( klogErr, rc, "cannot detect size of file to be wrapped" );
        }
        else if ( wrapped_size == 0 )
        {
            rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );
            LOGERR( klogErr, rc, "file to be wrapped is empty" );
        }
        else
        {
            char resolved_path [ 4096 ];
            rc = KDirectoryVResolvePath ( self,
                                          false, /* absolute */
                                          resolved_path,
                                          sizeof resolved_path,
                                          path,
                                          args );
            if ( rc != 0 )
            {
                PLOGERR( klogErr, ( klogErr, rc, "cannot resolve path of cache file '$(path)'", 
                        "path=%s", path ) );
            }
            else
            {
                KFile * cache;
                /* lets see if we can open the cache-file in read/write - mode */
                rc = KDirectoryOpenFileSharedWrite ( self, &cache, true, "%s.cache", resolved_path );
                if ( rc == 0 )
                {
                    /* cache-file exists and we have the exclusive rd/wr access to it !*/
                    rc = finish_rw( tee, to_wrap, cache, resolved_path,
                                    wrapped_size, block_size );

                }
                else if ( GetRCState( rc ) == rcNotFound )
                {
                    /* cache-file does not exist, let's try to create it */
                    rc = create_new_tee ( self, tee, to_wrap, resolved_path,
                                          wrapped_size, block_size );                    
                }
                else
                {
                    /* let us try to open the cache in read-only mode !*/
                    rc = KDirectoryOpenFileRead ( self,
                                                  ( const struct KFile ** )&( cache ),
                                                  "%s.cache",
                                                  resolved_path );
                    if ( rc != 0 )
                    {
                        /* we cannot open the cache-file in read-only-mode */
                        LOGERR( klogErr, rc, "cannot open cache-file in read-only-mode" );
                    }
                    else
                    {
                        /* finish in read-only mode... */
                        rc = finish_ro( tee, to_wrap, cache, resolved_path,
                                        wrapped_size, block_size );
                    }
                }
            }
        }
        
        if ( rc != 0 )
        {
            /* hand out to-wrap-file as tee-file in case we cannot create a tee-file,
               as kind of a last resort */
            rc = KFileAddRef( to_wrap );
            if ( rc != 0 )
            {
                LOGERR( klogErr, rc, "KFileAddRef( file to be wrapped ) failed" );
            }
            else
            {
                *tee = to_wrap;
            }
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC KDirectoryMakeCacheTeeWM ( struct KDirectory *self,
                                              struct KFile const **tee,
                                              struct KFile const *to_wrap,
                                              uint32_t block_size,
                                              const char *path, ... )
{
    rc_t rc;
    va_list args;
    va_start ( args, path );
    rc = KDirectoryVMakeCacheTeeWM ( self, tee, to_wrap, block_size, path, args );
    va_end ( args );
    return rc;
}

