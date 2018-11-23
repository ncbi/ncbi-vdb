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

struct KCacheTeeFile_v3;
#define KFILE_IMPL struct KCacheTeeFile_v3

struct KCacheTeeChunkReader;
#define KCHUNKREADER_IMPL struct KCacheTeeChunkReader

#include <kfs/impl.h>
#include "sysfile-priv.h"

#include <klib/rc.h>
#include <klib/debug.h>
#include <klib/log.h>
#include <klib/status.h>
#include <klib/vector.h>
#include <kproc/lock.h>
#include <kproc/cond.h>
#include <kproc/thread.h>
#include <kproc/queue.h>
#include <kfs/chunk-reader.h>

#define DEFAULT_PAGE_SIZE ( 32U * 1024U )
#define DEFAULT_CLUSTER_FACT 4U
#define MAX_REQUEST_SIZE ( 256U * 1024U * 1024U )
#define MAX_PAGE_SIZE ( 256U * 1024U * 1024U )
#define MAX_RAM_CACHE_BYTES ( 64UL * 1024UL * 1024UL * 1024UL )
#if _DEBUGGING
#define MIN_PAGE_SIZE ( 256U )
#else
#define MIN_PAGE_SIZE ( 4U * 1024U )
#endif
#define MSGQ_LENGTH 16

typedef struct KCacheTeeFile_v3 KCacheTeeFile_v3;
typedef struct KCacheTeeFileTail KCacheTeeFileTail;
typedef struct KCacheTeeChunkReader KCacheTeeChunkReader;

struct KCacheTeeChunkReader
{
    KChunkReader dad;
    KCacheTeeFile_v3 * ctf;
};

struct KCacheTeeFile_v3
{
    KFile_v1 dad;                   /* constant      */
    uint64_t source_size;           /* constant      */
    const KFile * source;           /* bg thread use */
    KChunkReader * chunks;          /* bg thread use */
    KDirectory * dir;               /* fg thread use */
    KFile * cache_file;             /* shared use    */
    KVector * ram_cache;            /* shared use    */
    atomic32_t * bitmap;            /* shared use    */
    const KCacheTeeFileTail * tail; /* constant      */
    KQueue * msgq;                  /* shared use    */
    KLock * lock;                   /* shared use    */
    KCondition * cond;              /* shared use    */
    KThread * thread;               /* fg thread use */
    size_t bmap_size;               /* constant      */
    uint32_t page_size;             /* constant      */
    uint32_t cluster_fact;          /* constant      */
    uint32_t ram_limit;             /* constant      */
    uint32_t ram_pg_count;          /* bg thread use */
    bool quitting;                  /* shared use    */
    bool whole_file;                /* constant      */
    char path [ 4098 ];             /* constant      */
};

struct KCacheTeeFileTail
{
    uint64_t orig_size;
    uint32_t page_size;
};

typedef struct KCacheTeeFileMsg KCacheTeeFileMsg;
struct KCacheTeeFileMsg
{
    uint64_t pos;
    size_t size;
    struct timeout_t * tm;
};

static
rc_t CC KCacheTeeChunkReaderDestroy ( KCacheTeeChunkReader * self )
{
    /* NB - do not release: this is a borrowed reference */
    self -> ctf = NULL;
    free ( self );
    return 0;
}

static
size_t CC KCacheTeeChunkReaderBufferSize ( const KCacheTeeChunkReader * self )
{
    return self -> ctf -> page_size;
}

static
rc_t CC KCacheTeeChunkReaderNext ( KCacheTeeChunkReader * self, void ** buf, size_t * size )
{
    * buf = malloc ( * size = self -> ctf -> page_size );
    if ( * buf == NULL )
        return RC ( rcFS, rcBuffer, rcAllocating, rcMemory, rcExhausted );
    return 0;
}

static
rc_t CC KCacheTeeChunkReaderConsume ( KCacheTeeChunkReader * self, uint64_t pos, const void * buf, size_t size )
{
    /* AT THIS POINT...

       what we have is a newly read buffer, potentially partial.
       if partial, we zero the remainder. actually if partial, it
       had better be the last page in the file.

       the buffer needs to be inserted into the ram_cache,
       and written to the cache_file,
       and entered into the index.

       finally, we signal the fg thread that something has changed. */

    /* TBD */
    return -1;
}

static
rc_t CC KCacheTeeChunkReaderReturn ( KCacheTeeChunkReader * self, void * buf, size_t size )
{
    /* ignore */
    return 0;
}

static KChunkReader_vt_v1 KCacheTeeChunkReader_vt =
{
    /* version == 1.0 */
    1, 0,

    /* start minor version == 0 */
    KCacheTeeChunkReaderDestroy,
    KCacheTeeChunkReaderBufferSize,
    KCacheTeeChunkReaderNext,
    KCacheTeeChunkReaderConsume,
    KCacheTeeChunkReaderReturn
    /* end minor version == 0 */
};

static
rc_t KCacheTeeFileMakeChunkReader ( KCacheTeeFile_v3 * self )
{
    rc_t rc;
    KCacheTeeChunkReader * obj;

    STATUS ( STAT_PRG, "%s - allocating chunk-reader\n", __func__ );

    obj = malloc ( sizeof * obj );
    if ( obj == NULL )
        rc = RC ( rcFS, rcFile, rcAllocating, rcMemory, rcExhausted );
    else
    {
        STATUS ( STAT_PRG, "%s - binding virtual table\n", __func__ );
        rc = KChunkReaderInit ( & obj -> dad, ( const KChunkReader_vt * ) & KCacheTeeChunkReader_vt );
        if ( rc == 0 )
        {
            /* NB - do NOT attach reference here,
               or this will create an unbreakable cycle */
            obj -> ctf = self;

            /* the cache-tee file owns the reader reference */
            self -> chunks = & obj -> dad;

            /* success */
            return 0;
        }

        STATUS ( STAT_PRG, "%s - freeing chunk-reader\n", __func__ );
        free ( obj );
    }

    return rc;
}

static
rc_t CC KCacheTeeFileDestroy ( KCacheTeeFile_v3 *self )
{
    rc_t rc;
    ( void ) rc;

    /* seal msg queue */
    KQueueSeal ( self -> msgq );

    /* stop background thread */
    self -> quitting = true;
    rc = KThreadWait ( self -> thread, NULL );

    /* release thread */
    KThreadRelease ( self -> thread );

    /* release queue */
    KQueueRelease ( self -> msgq );

    /* release lock */
    KLockRelease ( self -> lock );

    /* release condition */
    KConditionRelease ( self -> cond );

    /* free ram_cache pages */
    /* TBD */

    /* free ram_cache */
    KVectorRelease ( self -> ram_cache );

    /* release source */
    KFileRelease ( self -> source );

    /* release chunked reader */
    KChunkReaderRelease ( self -> chunks );

    /* ? promote cache_file ? */
    /* TBD */

    /* free bitmap */
    free ( self -> bitmap );

    /* release cache_file */
    KFileRelease ( self -> cache_file );

    /* release directory */
    KDirectoryRelease ( self -> dir );

    free ( self );

    return 0;
}

static
struct KSysFile_v1 * CC KCacheTeeFileGetSysFile ( const KCacheTeeFile_v3 *self, uint64_t *offset )
{
    return NULL;
}

static
rc_t CC KCacheTeeFileRandomAccess ( const KCacheTeeFile_v3 *self )
{
    if ( self -> cache_file != NULL )
        return KFileRandomAccess ( self -> cache_file );
    return KFileRandomAccess ( self -> source );
}

static
rc_t CC KCacheTeeFileGetSize ( const KCacheTeeFile_v3 *self, uint64_t *size )
{
    * size = self -> source_size;
    return 0;
}

static
rc_t CC KCacheTeeFileSetSize ( KCacheTeeFile_v3 *self, uint64_t size )
{
    return RC ( rcFS, rcFile, rcUpdating, rcFile, rcReadonly );
}

static
rc_t CC KCacheTeeFileRead ( const KCacheTeeFile_v3 *self, uint64_t pos,
    void *buffer, size_t bsize, size_t *num_read )
{
    /* TBD */
    * num_read = 0;
    return -1;
}

static
rc_t CC KCacheTeeFileWrite ( KCacheTeeFile_v3 *self, uint64_t pos,
    const void *buffer, size_t size, size_t *num_writ )
{
    * num_writ = 0;
    return RC ( rcFS, rcFile, rcUpdating, rcInterface, rcUnsupported );
}

static
uint32_t CC KCacheTeeFileGetType ( const KCacheTeeFile_v3 * self )
{
    if ( self -> cache_file != NULL )
        return KFileType ( self -> cache_file );
    return KFileType ( self -> source );
}

static
rc_t CC KCacheTeeFileTimedRead ( const KCacheTeeFile_v3 *self, uint64_t pos,
    void *buffer, size_t bsize, size_t *num_read, struct timeout_t *tm )
{
    /* TBD */
    * num_read = 0;
    return -1;
}

static
rc_t CC KCacheTeeFileTimedWrite ( KCacheTeeFile_v3 *self, uint64_t pos,
    const void *buffer, size_t size, size_t *num_writ, struct timeout_t *tm )
{
    * num_writ = 0;
    return RC ( rcFS, rcFile, rcUpdating, rcInterface, rcUnsupported );
}

static
rc_t CC KCacheTeeFileReadChunked ( const KCacheTeeFile_v3 *self, uint64_t pos,
    KChunkReader * chunks, size_t bsize, size_t * total_read )
{
    rc_t rc = 0;
    size_t total, num_read;

    assert ( chunks != NULL );

    for ( total = 0; rc == 0 && total < bsize; total += num_read )
    {
        void * chbuf;
        size_t chsize;
        rc = KChunkReaderNextBuffer ( chunks, & chbuf, & chsize );
        if ( rc == 0 )
        {
            rc = KCacheTeeFileRead ( self, pos + total, chbuf, chsize, & num_read );
            if ( rc == 0 && num_read != 0 )
                rc = KChunkReaderConsumeChunk ( chunks, pos + total, chbuf, num_read );
            KChunkReaderReturnBuffer ( chunks, chbuf, chsize );
        }

        if ( num_read == 0 )
            break;
    }

    * total_read = total;

    return ( total == 0 ) ? rc : 0;
}

static
rc_t CC KCacheTeeFileTimedReadChunked ( const KCacheTeeFile_v3 *self, uint64_t pos,
    KChunkReader * chunks, size_t bsize, size_t * total_read, struct timeout_t * tm )
{
    rc_t rc = 0;
    size_t total, num_read;

    assert ( chunks != NULL );

    for ( total = 0; rc == 0 && total < bsize; total += num_read )
    {
        void * chbuf;
        size_t chsize;
        rc = KChunkReaderNextBuffer ( chunks, & chbuf, & chsize );
        if ( rc == 0 )
        {
            rc = KCacheTeeFileTimedRead ( self, pos + total, chbuf, chsize, & num_read, tm );
            if ( rc == 0 && num_read != 0 )
                rc = KChunkReaderConsumeChunk ( chunks, pos + total, chbuf, num_read );
            KChunkReaderReturnBuffer ( chunks, chbuf, chsize );
        }

        if ( num_read == 0 )
            break;
    }

    * total_read = total;

    return ( total == 0 ) ? rc : 0;
}

static KFile_vt_v1 KCacheTeeFile_v3_vt =
{
    /* version == 1.3 */
    1, 3,

    /* start minor version == 0 */
    KCacheTeeFileDestroy,
    KCacheTeeFileGetSysFile,
    KCacheTeeFileRandomAccess,
    KCacheTeeFileGetSize,
    KCacheTeeFileSetSize,
    KCacheTeeFileRead,
    KCacheTeeFileWrite,
    /* end minor version == 0 */

    /* start minor version == 1 */
    KCacheTeeFileGetType,
    /* end minor version == 1 */

    /* start minor version == 2 */
    KCacheTeeFileTimedRead,
    KCacheTeeFileTimedWrite,
    /* end minor version == 2 */

    /* start minor version == 3 */
    KCacheTeeFileReadChunked,
    KCacheTeeFileTimedReadChunked
    /* end minor version == 3 */
};

/* RunThread
 */
static
rc_t KCacheTeeFileBGLoop ( KCacheTeeFile_v3 * self )
{
    rc_t rc = 0;

    STATUS ( STAT_PRG, "BG: %s - entering loop\n", __func__ );
    while ( ! self -> quitting )
    {
        KCacheTeeFileMsg * dmsg;
        STATUS ( STAT_PRG, "BG: %s - waiting on message queue\n", __func__ );
        /* TBD - use a timeout for looking at queue */
        rc = KQueuePop ( self -> msgq, ( void ** ) & dmsg, NULL );
        if ( rc == 0 )
        {
            size_t num_read;

            KCacheTeeFileMsg msg = * dmsg;
            free ( dmsg );

            /* examine request:

               record whether there was any hit to existing pages
               from the starting positions,

               update the request to remove existing pages

               broadcast to foreground if there were any hits */

            /* issue a chunked read to source file
               if the request has unsatisfied starting position */
            if ( msg . tm == NULL )
                rc = KFileReadChunked ( self -> source, msg . pos, self -> chunks, msg . size, & num_read );
            else
                rc = KFileTimedReadChunked ( self -> source, msg . pos, self -> chunks, msg . size, & num_read, msg . tm );
        }
    }

    return rc;
}

static
rc_t CC KCacheTeeFileRunThread ( const KThread * t, void * data )
{
    rc_t rc;
    KCacheTeeFile_v3 * self = data;

    STATUS ( STAT_PRG, "BG: %s - starting thread\n", __func__ );

    STATUS ( STAT_PRG, "BG: %s - acquiring lock\n", __func__ );
    rc = KLockAcquire ( self -> lock );
    if ( rc != 0 )
    {
        PLOGERR ( klogSys, ( klogSys, rc, "BG: $(func) - failed to acquire mutex"
                             , "func=%s"
                             , __func__
                      ) );
        self -> quitting = true;
        return rc;
    }

    STATUS ( STAT_PRG, "BG: %s - signaling FG to sync\n", __func__ );
    rc = KConditionSignal ( self -> cond );
    KLockRelease ( self -> lock );
    if ( rc != 0 )
    {
        PLOGERR ( klogSys, ( klogSys, rc, "BG: $(func) - failed to signal foreground"
                             , "func=%s"
                             , __func__
                      ) );
        self -> quitting = true;
        return rc;
    }

    return KCacheTeeFileBGLoop ( self );
}

/* MakeKCacheTeeFile ( constructor )
 */
static
rc_t KCacheTeeFileStartBgThread ( KCacheTeeFile_v3 * self )
{
    rc_t rc;

    STATUS ( STAT_PRG, "%s - acquiring lock\n", __func__ );
    rc = KLockAcquire ( self -> lock );
    if ( rc != 0 )
    {
        PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to acquire mutex"
                             , "func=%s"
                             , __func__
                      ) );
    }
    else
    {
        STATUS ( STAT_PRG, "%s - starting bg thread\n", __func__ );
        rc = KThreadMake ( & self -> thread, KCacheTeeFileRunThread, self );
        if ( rc != 0 )
        {
            PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to start background thread"
                                 , "func=%s"
                                 , __func__
                          ) );
        }
        else
        {
            STATUS ( STAT_GEEK, "%s - waiting on bg thread\n", __func__ );
            rc = KConditionWait ( self -> cond, self -> lock );
            if ( rc != 0 )
            {
                PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to wait on condition signal"
                                     , "func=%s"
                                     , __func__
                              ) );
            }
        }

        STATUS ( STAT_PRG, "%s - releasing lock\n", __func__ );
        KLockRelease ( self -> lock );
    }

    return rc;
}

static
rc_t KCacheTeeFileInitSync ( KCacheTeeFile_v3 * self )
{
    rc_t rc;

    STATUS ( STAT_PRG, "%s - allocating fg->bg message queue\n", __func__ );
    rc = KQueueMake ( & self -> msgq, MSGQ_LENGTH );
    if ( rc != 0 )
    {
        PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to allocate $(elems) element message queue"
                             , "func=%s,elems=%u"
                             , __func__
                             , MSGQ_LENGTH
                      ) );
    }
    else
    {
        STATUS ( STAT_PRG, "%s - allocating mutex\n", __func__ );
        rc = KLockMake ( & self -> lock );
        if ( rc != 0 )
        {
            PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to create mutex"
                                 , "func=%s"
                                 , __func__
                          ) );
        }
        else
        {
            STATUS ( STAT_PRG, "%s - allocating condition\n", __func__ );
            rc = KConditionMake ( & self -> cond );
            if ( rc != 0 )
            {
                PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to create condition"
                                     , "func=%s"
                                     , __func__
                              ) );
            }
        }
    }

    return rc;
}

static
rc_t KCacheTeeFileInitNew ( KCacheTeeFile_v3 * self )
{
    rc_t rc;
    uint64_t calculated_eof;

    STATUS ( STAT_PRG, "%s - initializing new cache file '%s.cache'\n", __func__, self -> path );

    calculated_eof = self -> source_size + self -> bmap_size + sizeof * self -> tail;
    STATUS ( STAT_PRG, "%s - setting file size to %lu bytes\n", __func__, calculated_eof );
    rc = KFileSetSize ( self -> cache_file, calculated_eof );
    if ( rc != 0 )
    {
        PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to set file size of '$(path).cache' to $(eof) bytes"
                             , "func=%s,path=%s,eof=%lu"
                             , __func__
                             , self -> path
                             , calculated_eof
                      ) );
    }
    else
    {
        uint64_t pos;

        /* write the tail and then we're done */
        pos = self -> source_size + self -> bmap_size;
        STATUS ( STAT_PRG, "%s - writing initial cache file tail at offset %lu\n", __func__, pos );
        rc = KFileWriteExactly ( self -> cache_file, pos, self -> tail, sizeof * self -> tail );
        if ( rc != 0 )
        {
            PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to write file tail of '$(path).cache'"
                                 , "func=%s,path=%s"
                                 , __func__
                                 , self -> path
                      ) );
        }
    }

    return rc;
}

static
rc_t KCacheTeeFileInitExisting ( KCacheTeeFile_v3 * self )
{
    rc_t rc;
    uint64_t pos, actual_eof;

    STATUS ( STAT_PRG, "%s - initializing existing cache file '%s.cache'\n", __func__, self -> path );

    /* first of all, let's get the cache file size */
    rc = KFileSize ( self -> cache_file, & actual_eof );
    STATUS ( STAT_GEEK
             , "%s - file size = %lu, rc = $R\n"
             , __func__
             , actual_eof
             , rc
        );
    if ( rc != 0 )
    {
        PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to obtain file size of '$(path).cache'"
                             , "func=%s,path=%s"
                             , __func__
                             , self -> path
                      ) );
    }
    else
    {
        uint64_t calculated_eof;

        /* next, we can calculate what the new file size should be */
        calculated_eof = self -> source_size + self -> bmap_size + sizeof * self -> tail;
        STATUS ( STAT_GEEK
                 , "%s - calculated size = %lu\n"
                 , __func__
                 , calculated_eof
            );
        if ( calculated_eof != actual_eof )
        {
            STATUS ( STAT_QA
                     , "%s - actual size ( %lu ) != required size ( %lu )\n"
                     , __func__
                     , actual_eof
                     , calculated_eof
                );
            
            rc = RC ();
        }
        else
        {
            KCacheTeeFileTail existing_tail;

            pos = actual_eof - sizeof existing_tail;
            STATUS ( STAT_PRG, "%s - reading tail of shared cache file '%s.cache'\n", __func__, self -> path );
            rc = KFileReadExactly ( self -> cache_file, pos, & existing_tail, sizeof existing_tail );
            STATUS ( STAT_GEEK
                     , "%s - read results: { orig_size = %lu, page_size = %u }, rc = %R\n"
                     , __func__
                     , existing_tail . orig_size
                     , existing_tail . page_size
                     , rc
                );
            if ( rc != 0 )
            {
                PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to read tail of '$(path).cache'"
                                     , "func=%s,path=%s"
                                     , __func__
                                     , self -> path
                              ) );
            }
            else if ( self -> source_size != existing_tail . orig_size ||
                      self -> page_size != existing_tail . page_size )
            {
                PLOGMSG ( klogWarn, ( klogWarn, "$(func) - cache file parameters have changed for '$(path).cache'"
                                     , "func=%s,path=%s"
                                     , __func__
                                     , self -> path
                              ) );

                rc = RC ();
            }
            else
            {
                /* at this point, we can just read in the existing bitmap */
                STATUS ( STAT_PRG, "%s - reading bitmap of shared cache file '%s.cache'\n", __func__, self -> path );
                rc = KFileReadExactly ( self -> cache_file, self -> source_size, self -> bitmap, self -> bmap_size );
                if ( rc != 0 )
                {
                    PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to read bitmap of '$(path).cache'"
                                         , "func=%s,path=%s"
                                         , __func__
                                         , self -> path
                                  ) );
                }
            }
        }
    }

    return rc;
}

static
rc_t KCacheTeeFileInitShared ( KCacheTeeFile_v3 * self )
{
    rc_t rc;
    uint64_t pos, calculated_eof;

    STATUS ( STAT_PRG, "%s - initializing shared cache file '%s.cache'\n", __func__, self -> path );

    rc = KCacheTeeFileInitExisting ( self );
    if ( rc == 0 )
        return rc;

    PLOGMSG ( klogWarn, ( klogWarn, "$(func) - stale cache file '$(path).cache'. Reinitializing."
                         , "func=%s,path=%s"
                         , __func__
                         , self -> path
                  ) );

    calculated_eof = self -> source_size + self -> bmap_size + sizeof * self -> tail;
    
    STATUS ( STAT_PRG, "%s - setting file size to %lu bytes\n", __func__, calculated_eof );
    rc = KFileSetSize ( self -> cache_file, calculated_eof );
    if ( rc != 0 )
    {
        PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to set file size of '$(path).cache' to $(eof) bytes"
                             , "func=%s,path=%s,eof=%lu"
                             , __func__
                             , self -> path
                             , calculated_eof
                      ) );
    }
    else
    {
        /* write the bitmap and tail and then we're done */
        pos = self -> source_size;
        STATUS ( STAT_PRG, "%s - writing initial cache file tail at offset %lu\n", __func__, pos );
        rc = KFileWriteExactly ( self -> cache_file, pos, self -> bitmap, self -> bmap_size + sizeof * self -> tail );
        if ( rc != 0 )
        {
            PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to reinitialize '$(path).cache'"
                                 , "func=%s,path=%s"
                                 , __func__
                                 , self -> path
                      ) );
        }
    }

    return rc;
}

static
rc_t KCacheTeeFileOpen ( KCacheTeeFile_v3 * self, KDirectory * dir,
    const char * fmt, va_list args, const KFile ** promoted )
{
    rc_t rc;
    uint64_t dummy;

    ( void ) dummy;

    /* detect NO FILE CACHE case */
    if ( fmt == NULL || fmt [ 0 ] == 0 )
    {
        STATUS ( STAT_PRG, "%s - no file cache will be used\n", __func__ );
        return 0;
    }

    STATUS ( STAT_PRG, "%s - resolving cache file location\n", __func__ );
    rc = KDirectoryVResolvePath ( dir, true, self -> path, sizeof self -> path, fmt, args );
    if ( rc != 0 )
    {
        PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to resolve cache file path"
                             , "func=%s"
                             , __func__
                      ) );
    }
    else
    {
        STATUS ( STAT_PRG, "%s - duplicating directory reference %#p\n", __func__, dir );
        rc = KDirectoryAddRef ( dir );
        if ( rc == 0 )
        {
            self -> dir = dir;

            STATUS ( STAT_PRG, "%s - attempting to open file '%s' read-only\n", __func__, self -> path );
            rc = KDirectoryOpenFileRead ( dir, promoted, "%s", self -> path );
            if ( rc == 0 )
                STATUS ( STAT_QA, "%s - file '%s' exists\n", __func__, self -> path );
            else
            {
                STATUS ( STAT_PRG
                         , "%s - attempting to open file '%s.cache' shared read/write\n"
                         , __func__
                         , self -> path
                    );
                rc = KDirectoryOpenFileSharedWrite ( dir, & self -> cache_file, true, "%s.cache", self -> path );
                STATUS ( STAT_GEEK
                         , "%s - open shared file attempt: fd = %d, rc = $R\n"
                         , __func__
                         , KFileGetSysFile ( self -> cache_file, & dummy ) -> fd
                         , rc
                    );
                if ( rc == 0 )
                    rc = KCacheTeeFileInitShared ( self );
                else if ( GetRCState ( rc ) == rcNotFound )
                {
                    STATUS ( STAT_PRG
                             , "%s - attempting to create file '%s.cache' read/write\n"
                             , __func__
                             , self -> path
                        );
                    /* TBD - if this fails, go back to open-shared-write */
                    rc = KDirectoryCreateFile ( dir, & self -> cache_file,
                        true, 0666, kcmCreate | kcmParents, "%s.cache", self -> path );
                    STATUS ( STAT_GEEK
                             , "%s - create file attempt: fd = %d, rc = $R\n"
                             , __func__
                             , KFileGetSysFile ( self -> cache_file, & dummy ) -> fd
                             , rc
                        );
                    if ( rc == 0 )
                        rc = KCacheTeeFileInitNew ( self );
                }

                if ( self -> cache_file == NULL && rc != 0 )
                {
                    STATUS ( STAT_PRG
                             , "%s - attempting to open file '%s.cache' read-only\n"
                             , __func__
                             , self -> path
                        );
                    rc = KDirectoryOpenFileRead ( dir,
                        ( const KFile ** ) & self -> cache_file, "%s.cache", self -> path );
                    STATUS ( STAT_GEEK
                             , "%s - open read-only file attempt: fd = %d, rc = $R\n"
                             , __func__
                             , KFileGetSysFile ( self -> cache_file, & dummy ) -> fd
                             , rc
                        );
                    if ( rc == 0 )
                        rc = KCacheTeeFileInitExisting ( self );
                }

                if ( rc != 0 )
                {
                    PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to open cache file '$(path).cache'"
                                         , "func=%s,path=%s"
                                         , __func__
                                         , self -> path
                                  ) );

                    KFileRelease ( self -> cache_file );
                    self -> cache_file = NULL;
                }
            }
        }
    }

    return rc;
}

static
rc_t KCacheTeeFileMakeBitmap ( KCacheTeeFile_v3 * self )
{
    size_t bmsize;
    size_t num_words;
    uint64_t num_pages;
    atomic32_t * bitmap;
    KCacheTeeFileTail * tail;

    STATUS ( STAT_PRG, "%s - allocating bitmap index\n", __func__ );

    num_pages = ( self -> source_size + self -> page_size - 1 ) / self -> page_size;
    num_words = ( size_t ) ( num_pages + 31 ) / 32;
    bmsize = ( size_t ) num_words * sizeof * bitmap;
    bmsize += sizeof * tail;

    assert ( bmsize % sizeof * bitmap == 0 );
    bitmap = calloc ( bmsize / sizeof * bitmap, sizeof * bitmap );
    if ( bitmap == NULL )
    {
        rc_t rc = RC ( rcFS, rcFile, rcAllocating, rcMemory, rcExhausted );
        PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to allocate $(size) bytes"
                             , "func=%s,size=%zu"
                             , __func__
                             , bmsize
                      ) );
        return rc;
    }

    tail = ( KCacheTeeFileTail * ) & bitmap [ num_words ];
    tail -> orig_size = self -> source_size;
    tail -> page_size = self -> page_size;

    self -> bitmap = bitmap;
    self -> tail = tail;
    self -> bmap_size = bmsize;

    return 0;
}

static
rc_t KCacheTeeFileMakeRAMCache ( KCacheTeeFile_v3 * self )
{
    STATUS ( STAT_PRG, "%s - allocating ram cache\n", __func__ );
    return KVectorMake ( & self -> ram_cache );
}

static
rc_t KCacheTeeFileBindSourceFile ( KCacheTeeFile_v3 * self, const KFile * source )
{
    rc_t rc;

    STATUS ( STAT_PRG, "%s - obtaining source file size\n", __func__ );
    rc = KFileSize ( source, & self -> source_size );
    STATUS ( STAT_PRG
             , "%s - source file size returned %lu bytes, rc = %R\n"
             , __func__
             , self -> source_size
             , rc
        );
    if ( rc != 0 )
    {
        PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to obtain source file size"
                             , "func=%s"
                             , __func__
                      ) );
    }
    else
    {
        STATUS ( STAT_PRG, "%s - duplicating file reference %#p\n", __func__, source );
        rc = KFileAddRef ( source );
        if ( rc == 0 )
        {
            self -> source = source;
            STATUS ( STAT_GEEK
                     , "%s - file reference %#p has refcount = %u\n"
                     , __func__
                     , source
                     , atomic32_read ( & source -> refcount )
                );

            rc = KCacheTeeFileMakeChunkReader ( self );
        }
    }

    return rc;
}

static
void KCacheTeeFileBindConstants ( KCacheTeeFile_v3 * self,
    size_t page_size, uint32_t cluster_factor, uint32_t ram_pages )
{
    size_t request_size, ram_cache_size;

    STATUS ( STAT_GEEK, "%s - storing parameters and constants\n", __func__ );

    /* set up page size */
    self -> page_size = DEFAULT_PAGE_SIZE;
    if ( page_size != 0 )
    {
        /* test for even power of 2 */
        if ( ( ( page_size - 1 ) & page_size ) != 0 )
        {
            size_t ps = MIN_PAGE_SIZE;
            while ( ps < page_size )
                ps += ps;
            page_size = ps;
        }

        if ( page_size > MAX_PAGE_SIZE )
            page_size = MAX_PAGE_SIZE;
        else if ( page_size < MIN_PAGE_SIZE )
            page_size = MIN_PAGE_SIZE;

        /* require even power of 2 */
        assert ( ( ( page_size - 1 ) & page_size ) == 0 );
        self -> page_size = ( uint32_t ) page_size;
    }

    /* set up cluster factor */
    if ( cluster_factor == 0 )
        cluster_factor = DEFAULT_CLUSTER_FACT;

    request_size = ( size_t ) self -> page_size * cluster_factor;
    if ( request_size > MAX_REQUEST_SIZE )
        cluster_factor = MAX_REQUEST_SIZE / self -> page_size;

    self -> cluster_fact = cluster_factor;

    /* set up ram cache page limit */
    ram_cache_size = ( size_t ) self -> page_size * ram_pages;

    if ( ram_cache_size > MAX_RAM_CACHE_BYTES )
        ram_pages = MAX_RAM_CACHE_BYTES / self -> page_size;

    self -> ram_limit = ram_pages;
}

LIB_EXPORT rc_t CC KDirectoryVMakeKCacheTeeFile_v3 ( KDirectory * self,
    const KFile ** tee, const KFile * source,
    size_t page_size, uint32_t cluster_factor, uint32_t ram_pages,
    const char * fmt, va_list args )
{
    rc_t rc;

    STATUS ( STAT_QA, "%s - making cache-tee file v3\n", __func__ );

    if ( tee == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );
    else
    {
        * tee = NULL;

        if ( self == NULL )
            rc = RC ( rcFS, rcDirectory, rcConstructing, rcSelf, rcNull );
        else if ( source == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );
        else if ( ! source -> read_enabled )
        {
            if ( source -> write_enabled )
                rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcWriteonly );
            else
                rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNoPerm );
        }
        else
        {
            KCacheTeeFile_v3 * obj;

            /* allocate space for the object */
            STATUS ( STAT_PRG, "%s - allocating %u byte object\n", __func__, sizeof * obj );
            obj = malloc ( sizeof * obj );
            if ( obj == NULL )
            {
                rc = RC ( rcFS, rcFile, rcAllocating, rcMemory, rcExhausted );
                PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to allocate $(bytes) bytes for object"
                                     , "func=%s,bytes=%zu"
                                     , __func__
                                     , sizeof * obj
                              ) );
            }
            else
            {
                /* zero everything out but the path */
                const size_t clear_bytes = sizeof * obj - sizeof obj -> path + 1;
                STATUS ( STAT_GEEK, "%s - zeroing first %zu bytes\n", __func__, clear_bytes );
                memset ( obj, 0, clear_bytes );

                /* bind it to vtable - it becomes a KFile after this */
                STATUS ( STAT_PRG, "%s - binding virtual table\n", __func__ );
                rc = KFileInit_v1 ( & obj -> dad, ( const KFile_vt * ) & KCacheTeeFile_v3_vt,
                    "KCacheTeeFile_v3", "", true, false );
                if ( rc != 0 )
                {
                    free ( obj );
                    PLOGERR ( klogInt, ( klogInt, rc, "$(func) - failed to bind vtable to object"
                                         , "func=%s"
                                         , __func__
                                  ) );
                }
                else
                {
                    /* bind the parameters and constants to object */
                    KCacheTeeFileBindConstants ( obj, page_size, cluster_factor, ram_pages );

                    /* study the source file */
                    rc = KCacheTeeFileBindSourceFile ( obj, source );
                    if ( rc == 0 )
                    {
                        /* create the RAM cache */
                        rc = KCacheTeeFileMakeRAMCache ( obj );
                        if ( rc == 0 )
                        {
                            /* make the bitmap */
                            rc = KCacheTeeFileMakeBitmap ( obj );
                            if ( rc == 0 )
                            {
                                /* open cache file - ignore errors */
                                KCacheTeeFileOpen ( obj, self, fmt, args, tee );

                                /* if the promoted file exists,
                                   then just hand out that */
                                if ( * tee != NULL )
                                {
                                    KFileRelease_v1 ( & obj -> dad );
                                    return 0;
                                }

                                /* make all of the synchronization primitives */
                                rc = KCacheTeeFileInitSync ( obj );
                                if ( rc == 0 )
                                {
                                    /* start the background thread */
                                    rc = KCacheTeeFileStartBgThread ( obj );
                                    if ( rc == 0 )
                                    {
                                        * tee = & obj -> dad;
                                        return 0;
                                    }
                                }
                            }
                        }
                    }
 
                    KFileRelease_v1 ( & obj -> dad );
                }
            }
        }
    }

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryMakeKCacheTeeFile_v3 ( KDirectory * self,
    const KFile ** tee, const KFile * source,
    size_t page_size, uint32_t cluster_factor, uint32_t ram_pages,
    const char * fmt, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, fmt );

    rc = KDirectoryVMakeKCacheTeeFile_v3 ( self, tee, source, page_size, cluster_factor, ram_pages, fmt, args );

    va_end ( args );

    return rc;
}
