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
The file has one writer/reader. There is no read-only version of it.
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

#include <kproc/procmgr.h>
#include <kproc/lock.h>

#include <kfs/cacheteefile_wm.h>
#include <kfs/defs.h>

#include <sysalloc.h>
#include <stdlib.h>

#define DFLT_BLOCK_SIZE ( 1024 * 1024 * 32 )
#define MIN_BLOCK_SIZE ( 64 * 1024 )

/* prototypes for functions in cacheteefile_wm_cleanup.c ( avoids another private header-file ) */
struct CacheTeeWMCleanupTask;
rc_t Make_CacheTeeWMCleanup_Task ( struct CacheTeeWMCleanupTask **task, const char * cache_file );
rc_t Execute_and_Release_CacheTeeWMCleanup_Task ( struct CacheTeeWMCleanupTask * self );

typedef struct KCacheTeeFileWM
{
    KFile dad;
    const KFile * wrapped;      /* the file we are wrapping */
    KFile * cache;              /* the cache-file */
    KLock * cache_lock;         /* a lock in case the file is used multithreaded */
    struct CacheTeeWMCleanupTask * cleanup;     /* cleanup-task for windows... */

    uint8_t * block;            /* to buffer to read from wrapped and write to cache */
    uint64_t wrapped_size;      /* the size of the wrapped file */
    uint32_t block_size;        /* how big is the block */
    
} KCacheTeeFileWM;

/* helper to call the platform-independent function to get the process-id */
static rc_t assemble_file_path( const char * location, char * buffer, size_t buffer_size )
{
    struct KProcMgr * proc_mgr;
    rc_t rc = KProcMgrMakeSingleton ( &proc_mgr );
    if ( rc != 0 )
    {
        LOGERR( klogInt, rc, "cannot access process-manager" );
    }
    else
    {
        uint32_t pid;
        rc = KProcMgrGetPID ( proc_mgr, &pid );
        if ( rc != 0 )
        {
            LOGERR( klogInt, rc, "cannot access process-id" );
        }
        else
        {
            char hostname[ 1024 ];
            rc = KProcMgrGetHostName ( proc_mgr, hostname, sizeof hostname );
            if ( rc != 0 )
            {
                LOGERR( klogInt, rc, "cannot access hostname" );    
            }
            else
            {
                size_t num_writ;
                size_t loc_len = string_size ( location );
                char term = location[ loc_len - 1 ];
                KTime_t t = KTimeStamp ();
                
                if ( ( term  == '/' ) || ( term == '\\' ) )
                    rc = string_printf ( buffer, buffer_size, &num_writ, "%s%s_%u_%u",
                                         location, hostname, pid, t );
                else
                {
#if WINDOWS
                    rc = string_printf ( buffer, buffer_size, &num_writ, "%s\\%s_%u_%u",
                                         location, hostname, pid, t );
#else
                    rc = string_printf ( buffer, buffer_size, &num_writ, "%s/%s_%u_%u",
                                         location, hostname, pid, t );    
#endif    
                }
            
                if ( rc != 0 )
                {
                    LOGERR( klogInt, rc, "cannot assemble file path" );
                }
            }
        }
        KProcMgrRelease ( proc_mgr );
    }
    return rc;
}

/**********************************************************************************************
    START vt-functions
**********************************************************************************************/
static rc_t CC KCacheTeeFileWMDestroy( KCacheTeeFileWM * self )
{
    rc_t rc = KFileRelease ( self -> wrapped );
    if ( rc != 0 )
    {
        LOGERR( klogInt, rc, "Error releasing wrapped file" );
    }
    
    rc = KFileRelease ( self -> cache );
    if ( rc != 0 )
    {
        LOGERR( klogInt, rc, "Error releasing cache file" );
    }

    if ( self -> cache_lock != NULL )
    {
        rc = KLockRelease( self -> cache_lock );
        if ( rc != 0 )
        {
            LOGERR( klogInt, rc, "Error releasing cache-lock" );    
        }
        self -> cache_lock = NULL;
    }
    
    if ( self -> cleanup != NULL )
    {
        rc = Execute_and_Release_CacheTeeWMCleanup_Task ( self -> cleanup ); /* in cacheteefile_wm_cleanup.c */
        self -> cleanup = NULL;
    }
    
    /* now remove the cache-file itself if we created it ( self -> block != NULL ) */
    if ( self -> block != NULL )
    {
        free( ( void * ) self -> block );
        self -> block = NULL;
    }

    free ( ( void * ) self );
    return rc;
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

static rc_t KCacheTeeFileWMLoop( const KCacheTeeFileWM *cself,
    uint64_t r_pos, uint64_t pos, void * buffer, size_t bsize, size_t * num_read )
{
    rc_t rc;
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
            if ( r_num_read == 0 )
            {
                done = true;
                *num_read = 0;
            }
            else
            {
                size_t r_num_written;
                rc = KFileWriteAll ( cself -> cache, r_pos, ( const void * ) cself -> block, r_num_read, &r_num_written );
                if ( pos < ( r_pos + r_num_read ) )
                {
                    /* we have at least a part of the requested data in memory.. */
                    uint8_t * src = cself -> block;
                    uint64_t offset = r_pos >= pos ? r_pos - pos : pos - r_pos;
                    size_t count = r_num_read - offset;
                    done = true;
                    if ( count > bsize ) count = bsize;
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
        }
        else
        {
            /* there is nothing we can do if we cannot read from the wrapped
               file, we are done, return the error code to the caller */
            done = true;
        }
    }
    return rc;
}
static rc_t KCacheTeeFileWMRead ( const KCacheTeeFileWM *cself,
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
            rc = KLockAcquire ( cself -> cache_lock );
            if ( rc != 0 )
            {
                LOGERR( klogErr, rc, "cannot acquire cache-lock" );
                /* if that is not possible, we use the wrapped file instead - no caching */
                rc = KFileReadAll ( cself -> wrapped, pos, buffer, bsize, num_read );
            }
            else
            {
                /* requested range is not in the cache, read from wrapped instead,
                   but store in cache, use large reads... */
                rc = KCacheTeeFileWMLoop( cself, r_pos, pos, buffer, bsize, num_read );

                KLockUnlock ( cself -> cache_lock );
            }
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

static KFile_vt_v1 vtKCacheTeeFile_WM =
{
    /* version 1.0 */
    1, 0,

    /* start minor version 0 methods */
    KCacheTeeFileWMDestroy,
    KCacheTeeFileWMGetSysFile,
    KCacheTeeFileWMRandomAccess,
    KCacheTeeFileWMSize,
    KCacheTeeFileWMSetSize,
    KCacheTeeFileWMRead,
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

static rc_t finish( struct KDirectory * self,
                    struct KFile const **tee,
                    struct KFile const * to_wrap,
                    struct KFile * cache,
                    const char * cache_file_name,
                    uint64_t wrapped_size,
                    uint32_t block_size )
{
    KCacheTeeFileWM * obj;
    
    rc_t rc = KFileAddRef ( to_wrap );
    if ( rc != 0 )
    {
        LOGERR( klogErr, rc, "cannot add-ref the wrapped file" );
        return rc;
    }
    
    obj = malloc ( sizeof * obj );
    if ( obj == NULL )
    {
        rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
        LOGERR( klogErr, rc, "cannot allocate cache-tee-struct" );
        KFileRelease( to_wrap );
        return rc;
    }

    obj -> wrapped = to_wrap;
    obj -> cache = cache;
    obj -> wrapped_size = wrapped_size;
    obj -> block_size = normalize_block_size( block_size );
        
    obj -> block = malloc( obj -> block_size );
    if ( obj -> block == NULL )
    {
        rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
        LOGERR( klogErr, rc, "cannot allocate cache-block" );
        KFileRelease( to_wrap );
        free( ( void * ) obj );
        return rc;
    }

    rc = KLockMake ( &obj -> cache_lock );
    if ( rc != 0 )
    {
        LOGERR( klogErr, rc, "cannot create cache-lock" );
        KFileRelease( to_wrap );
        free( ( void * ) obj -> block );
        free( ( void * ) obj );
        return rc;
    }
    
    rc = KFileInit ( &obj -> dad,
                    ( const union KFile_vt * ) &vtKCacheTeeFile_WM,
                     "KCacheTeeFileWM",
                     cache_file_name,
                     true,
                     false );
    if ( rc != 0 )
    {
        LOGERR( klogErr, rc, "cannot initialize KFile" );
        KLockRelease( obj -> cache_lock );
        KFileRelease( to_wrap );
        free( ( void * ) obj -> block );
        free( ( void * ) obj );
        return rc;
    }

#if WINDOWS
    rc = Make_CacheTeeWMCleanup_Task ( &obj -> cleanup, cache_file_name );
    if ( rc != 0 )
    {
        obj -> cleanup = NULL;
        LOGERR( klogErr, rc, "cannot create cleanup task" );
        KLockRelease( obj -> cache_lock );        
        KFileRelease( to_wrap );
        free( ( void * ) obj -> block );
        free( ( void * ) obj );
    }
#else
    obj -> cleanup = NULL;
    rc = KDirectoryRemove ( self, true, "%s", cache_file_name );
    if ( rc != 0 )
    {
        PLOGERR( klogErr, ( klogErr, rc, "cannot remove cache file '$(path)'", 
                "path=%s", cache_file_name ) );
    }
#endif    

    if ( rc != 0 )
    {
        KFileRelease( to_wrap );
        KLockRelease( obj -> cache_lock );        
        free( ( void * ) obj -> block );
        free( ( void * ) obj );
    }
    else
    {
        *tee = ( const KFile * ) &obj -> dad;
    }
    return rc;
}

LIB_EXPORT rc_t CC KDirectoryMakeCacheTeeWM ( struct KDirectory * self,
                                              struct KFile const ** tee,
                                              struct KFile const * to_wrap,
                                              uint32_t block_size,
                                              const char * location )
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
        else if ( location == NULL )
            rc = RC ( rcFS, rcFile, rcAllocating, rcPath, rcNull );
        else if ( location [ 0 ] == 0 )
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
            char buffer[ 4096 ];
            rc = assemble_file_path( location, buffer, sizeof buffer );
            if ( rc == 0 )
            {
                KFile * cache;
                rc = KDirectoryCreateFile ( self, &cache, true, 0664, kcmOpen | kcmParents,
                                            "%s", buffer );
                if ( rc != 0 )
                {
                    PLOGERR( klogErr, ( klogErr, rc, "cannot create cache file '$(path)'", 
                            "path=%s", buffer ) );
                }
                else
                {
                    /* creating the cache-file did succeed */
                    rc = finish( self, tee, to_wrap, cache, buffer, wrapped_size, block_size );
                    if ( rc != 0 )
                    {
                        KFileRelease( cache );
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

rc_t CTWM_get( const void ** ctwm, const char * basename );
rc_t CTWM_register( const void * ctwm, const char * basename );

LIB_EXPORT rc_t CC KDirectoryMakeCacheTeeWMfromURL ( struct KDirectory * self,
                 struct KFile const ** tee,
                 uint32_t block_size,
                 const char * location,
                 const char * token,
                 rc_t ( CC * on_create ) ( struct KFile const ** f, void *data ),
                 void *data )
{
    rc_t rc = 0;
    
    if ( tee == NULL )
        rc = RC ( rcFS, rcFile, rcAllocating, rcParam, rcNull );
    else
    {
        *tee = NULL;
        if ( self == NULL )
            rc = RC ( rcFS, rcFile, rcAllocating, rcSelf, rcNull );
        else if ( location == NULL )
            rc = RC ( rcFS, rcFile, rcAllocating, rcPath, rcNull );
        else if ( location [ 0 ] == 0 )
            rc = RC ( rcFS, rcFile, rcAllocating, rcPath, rcEmpty );
        else if ( token == NULL )
            rc = RC ( rcFS, rcFile, rcAllocating, rcPath, rcNull );
        else if ( token [ 0 ] == 0 )
            rc = RC ( rcFS, rcFile, rcAllocating, rcPath, rcEmpty );
    }

    if ( rc == 0 )
    {
        const void * existing_tee = NULL;
        rc = CTWM_get( &existing_tee, token );
        if ( rc == 0 )
        {
            if ( existing_tee != NULL )
            {
                *tee = existing_tee;
            }
            else
            {
                if ( on_create == NULL )
                {
                    rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );
                    LOGERR( klogErr, rc, "invalid callback" );
                }
                else
                {
                    struct KFile const * remote_file;
                    rc = on_create( &remote_file, data );
                    if ( rc == 0 )
                    {
                        rc = KDirectoryMakeCacheTeeWM ( self, tee, remote_file,
                                              block_size, location );
                        if ( rc == 0 )
                        {
                            rc = CTWM_register( ( void * )*tee, token );
                            if ( rc != 0 )
                            {
                                rc = 0;
                            }
                        }
                        else
                        {
                            *tee = remote_file;
                            rc = 0;
                        }
                    }
                }
            }
        }
    }
    return rc;
}

LIB_EXPORT bool CC KFileIsKCacheTeeWM( const struct KFile * self )
{
    bool res = false;
    if ( self != NULL )
    {
        res = ( &self->vt->v1 == &vtKCacheTeeFile_WM );
    }
    return res;
}
