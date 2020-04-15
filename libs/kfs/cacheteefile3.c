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
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/time.h>
#include <klib/status.h>
#include <klib/vector.h>
#include <klib/container.h>
#include <kproc/lock.h>
#include <kproc/cond.h>
#include <kproc/thread.h>
#include <kproc/queue.h>
#include <kproc/timeout.h>
#include <kfs/chunk-reader.h>
#include <kfs/lockfile.h>

/* for Windows */
#include <kproc/task.h>
#include <kproc/procmgr.h>
#include <kfs/remove-file-task.h>

#include <arch-impl.h>

#define DEFAULT_PAGE_SIZE ( 32U * 1024U )
#define DEFAULT_CLUSTER_FACT 4U
#define MAX_REQUEST_SIZE ( 256U * 1024U * 1024U )
#define MAX_PAGE_SIZE ( 256U * 1024U * 1024U )
#define MAX_RAM_CACHE_BYTES ( 64ULL * 1024ULL * 1024ULL * 1024ULL )
#if _DEBUGGING
#define MIN_PAGE_SIZE ( 256U )
#else
#define MIN_PAGE_SIZE ( 4U * 1024U )
#endif
#define MSGQ_LENGTH ( 16 * 1024 )
#define BMWORDBITS 5
#define BMWORDSIZE ( 1U << BMWORDBITS )
#define BMWORDMASK ( BMWORDSIZE - 1 )

#if BMWORDSIZE == 32
typedef uint32_t bmap_t;
typedef int32_t sbmap_t;
#define bmap_lsbit( self ) uint32_lsbit( self )
#elif BMWORDSIZE == 64
typedef uint64_t bmap_t;
typedef int64_t sbmap_t;
#define bmap_lsbit( self ) uint64_lsbit( self )
#else
#error "bitmap word size neither 32 nor 64 bits"
#endif

typedef struct KCacheTeeFile_v3 KCacheTeeFile_v3;
typedef struct KCacheTeeFileTail KCacheTeeFileTail;
typedef struct KCacheTeeChunkReader KCacheTeeChunkReader;

struct KCacheTeeChunkReader
{
    KChunkReader dad;
    KCacheTeeFile_v3 * ctf;
};

typedef struct KCacheTeeFileLMRUPage KCacheTeeFileLMRUPage;
struct KCacheTeeFileLMRUPage
{
    DLNode dad;
    size_t pg_idx;
};


/*

  in this configuration, we're using a mutex to protect
  shared data structures, so there's no benefit to using
  atomic operations in the bitmap.

  it also means that we COULD go to using 64-bit operations,
  except that it would violate backward compatibility.

 */

struct KCacheTeeFile_v3
{
    KFile_v1 dad;                   /* constant      */
    KTaskTicket rm_file_tkt;        /* constant      */
    uint64_t source_size;           /* constant      */
    const KFile * source;           /* bg thread use */
    KChunkReader * chunks;          /* bg thread use */
    KDirectory * dir;               /* fg thread use */
    KProcMgr * procmgr;             /* fg thread use */
    KFile * cache_file;             /* shared use    */
    KVector * ram_cache;            /* shared use    */
    KVector * ram_cache_mru_idx;    /* shared use    */
    DLList ram_cache_mru;           /* shared use    */
    volatile bmap_t * bitmap;       /* shared use    */
    const KCacheTeeFileTail * tail; /* constant      */
    DLList msgq;                    /* shared use    */
    KLock * qlock;                  /* shared use    */
    KLock * cache_lock;             /* shared use    */
    KCondition * bgcond;            /* shared use    */
    KCondition * fgcond;            /* shared use    */
    KThread * thread;               /* fg thread use */
    size_t bmap_size;               /* constant      */
    uint32_t page_size;             /* constant      */
    uint32_t cluster_fact;          /* constant      */
    uint32_t ram_limit;             /* constant      */
    uint32_t ram_pg_count;          /* bg thread use */
    volatile bool quitting;         /* shared use    */
    bool buffer_was_cached;         /* bg thread use */
    bool try_promote_on_close;      /* fg thread use */
    bool remove_on_close;           /* fg thread use */    
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
    DLNode dad;
    uint64_t pos;
    uint64_t size;
    struct timeout_t tm;
    size_t initial_page_idx;
    bool have_tm;
};

typedef struct KCacheTeeFileTreeNode KCacheTeeFileTreeNode;
struct KCacheTeeFileTreeNode
{
    BSTNode dad;
    const KFile * file;
    char path [ 4096 ];
};

static
int64_t CC KCacheTeeFileTreeNodeFind ( const void *item, const BSTNode *n )
{
    const char * path = ( const char * ) item;
    const KCacheTeeFileTreeNode * node = ( const KCacheTeeFileTreeNode * ) n;

    return strcmp ( path, node -> path );
}

static
int64_t CC KCacheTeeFileTreeNodeSort ( const BSTNode *item, const BSTNode *n )
{
    const KCacheTeeFileTreeNode * a = ( const KCacheTeeFileTreeNode * ) item;
    const KCacheTeeFileTreeNode * b = ( const KCacheTeeFileTreeNode * ) n;

    return strcmp ( a -> path, b -> path );
}

#if WINDOWS
//#include <synchapi.h>
//#include <processthreadsapi.h>
#include <windows.h>
static CRITICAL_SECTION crit;
static
int enter_crit_section ( void )
{
    static bool crit_init;
    if ( ! crit_init )
    {
        InitializeCriticalSection ( & crit );
        crit_init = true;
    }
    EnterCriticalSection ( & crit );
    return 0;
}
#define ENTER_CRIT_SECTION() \
    enter_crit_section ()
#define EXIT_CRIT_SECTION() \
    ( LeaveCriticalSection ( & crit ), 0 )
#define CUR_THREAD_ID() \
    ( uint64_t ) GetCurrentThreadId ()
#else
#include <pthread.h>
static pthread_mutex_t crit = PTHREAD_MUTEX_INITIALIZER;
#define ENTER_CRIT_SECTION() \
    pthread_mutex_lock ( & crit )
#define EXIT_CRIT_SECTION() \
    pthread_mutex_unlock ( & crit )
#define CUR_THREAD_ID() \
    ( uint64_t ) pthread_self ()
#endif
static BSTree open_cache_tee_files;

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
    if ( self -> ctf -> quitting )
    {
        STATUS ( STAT_PRG, "BG: %s - refusing request due to quitting\n", __func__ );
        * buf = NULL;
        * size = 0;
        return RC ( rcFS, rcBuffer, rcAllocating, rcTransfer, rcCanceled );
    }

    STATUS ( STAT_PRG, "BG: %s - allocating page buffer of %zu bytes\n", __func__, self -> ctf -> page_size );
    * buf = malloc ( * size = self -> ctf -> page_size );
    if ( * buf == NULL )
        return RC ( rcFS, rcBuffer, rcAllocating, rcMemory, rcExhausted );

    return 0;
}

static
rc_t KCacheTeeFileRAMCacheInsert ( KCacheTeeFile_v3 * self,
    const void * buf, size_t pg_idx )
{
    rc_t rc;

    void * existing = NULL;
    KCacheTeeFileLMRUPage * lmru = NULL;

    if ( self -> ram_limit == 0 )
    {
        STATUS ( STAT_PRG, "BG: %s - RAM cache not in use\n", __func__ );
        return SILENT_RC ( rcFS, rcNode, rcInserting, rcFunction, rcNotAvailable );
    }

    /* 1. retrieve any existing buffer - ignore error */
    STATUS ( STAT_PRG, "BG: %s - checking for existing buffer @ %zu\n", __func__, pg_idx );
    KVectorGetPtr ( self -> ram_cache, pg_idx, & existing );

    /* 2. store buffer within RAM cache */
    STATUS ( STAT_PRG, "BG: %s - storing buffer @ %zu\n", __func__, pg_idx );
    rc = KVectorSetPtr ( self -> ram_cache, pg_idx, buf );
    STATUS ( STAT_GEEK, "BG: %s - store result: %R\n", __func__, rc );
    if ( rc == 0 )
    {
        /* record this bit of information for later */
        self -> buffer_was_cached = true;

        /* 3. delete any buffer that was there before */
        if ( existing != NULL )
        {
            STATUS ( STAT_PRG, "BG: %s - freeing previous buffer\n", __func__ );
            free ( existing );

            /* 4. retrieve existing MRU node */
            STATUS ( STAT_PRG, "BG: %s - retriving existing MRU node\n", __func__ );
            rc = KVectorGetPtr ( self -> ram_cache_mru_idx, pg_idx, ( void ** ) & lmru );
            if ( rc == 0 )
            {
                /* 5. reuse to mark as MRU */
                STATUS ( STAT_PRG, "BG: %s - relinking MRU node to head of list\n", __func__ );
                DLListUnlink ( & self -> ram_cache_mru, & lmru -> dad );
                DLListPushHead ( & self -> ram_cache_mru, & lmru -> dad );
            }
        }

        /* if cache is already full ... */
        else if ( self -> ram_pg_count == self -> ram_limit )
        {
            /* 6. pop LRU page from cache */
            STATUS ( STAT_PRG, "BG: %s - popping LRU node\n", __func__ );
            lmru = ( KCacheTeeFileLMRUPage * ) DLListPopTail ( & self -> ram_cache_mru );

            /* 7. free old page buffer */
            STATUS ( STAT_PRG, "BG: %s - retrieving LRU buffer @ %zu\n", __func__, lmru -> pg_idx );
            rc = KVectorGetPtr ( self -> ram_cache, lmru -> pg_idx, & existing );
            if ( rc == 0 && existing != NULL )
            {
                STATUS ( STAT_PRG, "BG: %s - freeing LRU buffer\n", __func__ );
                free ( existing );
            }

            /* 8. forget pointers in vectors */
            STATUS ( STAT_PRG, "BG: %s - clearing cache and mru-index vector entries\n", __func__ );
            KVectorUnset ( self -> ram_cache, lmru -> pg_idx );
            KVectorUnset ( self -> ram_cache_mru_idx, lmru -> pg_idx );

            /* 9. clear bit in bitmap if cache file not in use */
            if ( self -> cache_file == NULL )
            {
                STATUS ( STAT_PRG, "BG: %s - clear page %zu present in bitmap\n", __func__, lmru -> pg_idx );
                self -> bitmap [ lmru -> pg_idx >> BMWORDBITS ] &= ~ ( 1U << ( lmru -> pg_idx & BMWORDMASK ) );
            }

            /* 10. reuse node and insert new guy as MRU */
            STATUS ( STAT_PRG, "BG: %s - reusing MRU node and placing at head of list\n", __func__ );
            lmru -> pg_idx = pg_idx;
            DLListPushHead ( & self -> ram_cache_mru, & lmru -> dad );
            KVectorSetPtr ( self -> ram_cache_mru_idx, pg_idx, lmru );
        }

        /* cache is not full */
        else
        {
            /* 11. allocate node */
            STATUS ( STAT_PRG, "BG: %s - allocating MRU node\n", __func__ );
            lmru = malloc ( sizeof * lmru );
            if ( lmru == NULL )
                rc = RC ( rcFS, rcFile, rcReading, rcMemory, rcExhausted );
            else
            {
                /* 12. insert into index */
                lmru -> pg_idx = pg_idx;
                STATUS ( STAT_PRG, "BG: %s - inserting MRU node into index @ %zu\n", __func__, pg_idx );
                rc = KVectorSetPtr ( self -> ram_cache_mru_idx, pg_idx, lmru );
                if ( rc == 0 )
                {
                    /* 13. insert as MRU */
                    STATUS ( STAT_PRG, "BG: %s - placing MRU node into head of list\n", __func__ );
                    DLListPushHead ( & self -> ram_cache_mru, & lmru -> dad );

                    /* 14. update cache count */
                    ++ self -> ram_pg_count;
                    STATUS ( STAT_PRG, "BG: %s - new RAM cache page count is %u\n"
                             , __func__
                             , self -> ram_pg_count
                        );
                }
            }
        }
    }

    return rc;
}

static
rc_t KCacheTeeFileCacheInsert ( KCacheTeeFile_v3 * self,
    uint64_t pos, const void * buf, size_t size )
{
    rc_t rc = SILENT_RC ( rcFS, rcFile, rcWriting, rcFunction, rcNotAvailable );

    if ( self -> cache_file == NULL )
        STATUS ( STAT_PRG, "BG: %s - cache file not in use\n", __func__ );
    else
    {
        STATUS ( STAT_PRG, "BG: %s - writing %zu bytes to cache file @ %lu\n"
                 , __func__
                 , size
                 , pos
            );
        rc = KFileWriteExactly_v1 ( self -> cache_file, pos, buf, size );
    }

    return rc;
}

static
rc_t KCacheTeeFileSaveBitmap ( KCacheTeeFile_v3 * self )
{
    rc_t rc = 0;
    
    if ( self -> cache_file != NULL )
    {
        STATUS ( STAT_PRG, "BG: %s - saving cache bitmap\n", __func__ );
    
        rc = KFileWriteExactly_v1 ( self -> cache_file, self -> source_size,
            ( const void * ) self -> bitmap, self -> bmap_size );
    
        STATUS ( STAT_GEEK, "BG: %s - saved bm result code %R\n", __func__, rc );

    }
    
    return rc;
}

static
rc_t CC KCacheTeeChunkReaderConsume ( KCacheTeeChunkReader * chunk,
    uint64_t pos, const void * buf, size_t size )
{
    rc_t rc;

    /* switch to operating within KCacheTeeFile */
    KCacheTeeFile_v3 * self = chunk -> ctf;

    /* detect last buffer */
    if ( size < ( size_t ) self -> page_size )
    {
        STATUS ( STAT_PRG, "BG: %s - detected short buffer\n", __func__ );
        if ( pos + size != self -> source_size )
            return RC ( rcFS, rcFile, rcReading, rcConstraint, rcViolated );
        STATUS ( STAT_PRG, "BG: %s - short buffer is last in source file\n", __func__ );
    }

    /* detect last chunk beyond EOF */
    if ( pos + size > self -> source_size )
    {
        STATUS ( STAT_PRG, "BG: %s - buffer extends past EOF\n", __func__ );
        if ( pos >= self -> source_size )
            return RC ( rcFS, rcFile, rcReading, rcConstraint, rcViolated );
        size = ( size_t ) ( self -> source_size - pos );
        STATUS ( STAT_PRG, "BG: %s - considering only first %zu of buffer\n", __func__, size );
    }

    /* mutex around shared structures */
    STATUS ( STAT_PRG, "BG: %s - acquiring cache lock\n", __func__ );
    rc = KLockAcquire ( self -> cache_lock );
    if ( rc == 0 )
    {
        size_t pg_idx;
        rc_t rc2, rc3;

        pg_idx = ( size_t ) ( pos / self -> page_size );

        STATUS ( STAT_PRG, "BG: %s - insert buffer into RAM cache\n", __func__ );
        rc2 = KCacheTeeFileRAMCacheInsert ( self, buf, pg_idx );
        STATUS ( STAT_PRG, "BG: %s - write buffer to cache file\n", __func__ );
        rc3 = KCacheTeeFileCacheInsert ( self, pos, buf, size );

        if ( rc2 == 0 || rc3 == 0 )
        {
            /* set the "present" bit in bitmap */
            STATUS ( STAT_PRG, "BG: %s - set page %zu present in bitmap\n", __func__, pg_idx );
            self -> bitmap [ pg_idx >> BMWORDBITS ] |= 1U << ( pg_idx & BMWORDMASK );
            
            /* save the change immediately */
            KCacheTeeFileSaveBitmap ( self );

            /* notify any listeners */
            STATUS ( STAT_PRG, "BG: %s - broadcasting event to all waiting readers\n", __func__ );
            KConditionBroadcast ( self -> fgcond );
        }
        else
        {
            rc = ( self -> ram_limit != 0 ) ? rc2 : rc3;
        }

        STATUS ( STAT_PRG, "BG: %s - releasing cache lock\n", __func__ );
        KLockUnlock ( self -> cache_lock );
    }

    return rc;
}

static
rc_t CC KCacheTeeChunkReaderReturn ( KCacheTeeChunkReader * self, void * buf, size_t size )
{
    if ( self -> ctf -> buffer_was_cached )
    {
        STATUS ( STAT_PRG, "BG: %s - ignoring buffer return message\n", __func__ );
        self -> ctf -> buffer_was_cached = false;
    }
    else
    {
        STATUS ( STAT_PRG, "BG: %s - deleting buffer that was not cached\n", __func__ );
        free ( buf );
    }

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

/* decl for functions used for promotion */
static bool is_bitmap_complete_v3( const KCacheTeeFile_v3 * self );
static rc_t finalize_v3 ( KCacheTeeFile_v3 * self );

static
rc_t CC KCacheTeeFileDestroy ( KCacheTeeFile_v3 *self )
{
    rc_t rc;

    /* remove cache file entry from open-cache-file cache */
    if ( self -> cache_file != NULL )
    {
        int status = ENTER_CRIT_SECTION ();
        if ( status == 0 )
        {
            BSTNode * node = BSTreeFind ( & open_cache_tee_files, self -> path, KCacheTeeFileTreeNodeFind );
            if ( node != NULL )
            {
                BSTreeUnlink ( & open_cache_tee_files, node );
                free ( node );
            }
            
            EXIT_CRIT_SECTION ();
        }
    }

    ( void ) rc;

    /* signal all threads that this cache-tee-file is shutting down

       by definition of the referencing counting system, the executing
       thread maintains the last reference to this object - no other
       thread can access it. this signal is therefore only for bg thread.

       this must be done before sealing the queue */
    STATUS ( STAT_PRG, "%s - setting 'quitting' flag\n", __func__ );
    self -> quitting = true;

    /* wake the background thread */
    STATUS ( STAT_PRG, "%s - signaling background thread to exit\n", __func__ );
    KConditionSignal ( self -> bgcond );

    /* stop background thread */
    STATUS ( STAT_PRG, "%s - waiting for bg thread to quit\n", __func__ );
    rc = KThreadWait ( self -> thread, NULL );

    /* release thread */
    STATUS ( STAT_PRG, "%s - releasing bg thread\n", __func__ );
    KThreadRelease ( self -> thread );

    /* message queue must be empty by design */
    assert ( DLListPopHead ( & self -> msgq ) == NULL );

    /* release locks */
    STATUS ( STAT_PRG, "%s - releasing queue lock object\n", __func__ );
    KLockRelease ( self -> qlock );
    STATUS ( STAT_PRG, "%s - releasing cache lock object\n", __func__ );
    KLockRelease ( self -> cache_lock );

    /* release condition */
    STATUS ( STAT_PRG, "%s - releasing fg condition\n", __func__ );
    KConditionRelease ( self -> fgcond );
    STATUS ( STAT_PRG, "%s - releasing bg condition\n", __func__ );
    KConditionRelease ( self -> bgcond );

    /* free ram_cache pages */
    STATUS ( STAT_PRG, "%s - freeing any pages from RAM cache\n", __func__ );
    while ( 1 )
    {
        void * buffer = NULL;
        KCacheTeeFileLMRUPage * lmru;

        lmru = ( KCacheTeeFileLMRUPage * ) DLListPopTail ( & self -> ram_cache_mru );
        if ( lmru == NULL )
            break;
        STATUS ( STAT_PRG, "%s - popped LRU page %zu\n", __func__, lmru -> pg_idx );

        STATUS ( STAT_PRG, "%s - retrieving page from cache\n", __func__ );
        KVectorGetPtr ( self -> ram_cache, lmru -> pg_idx, & buffer );
        if ( buffer != NULL )
        {
            STATUS ( STAT_PRG, "%s - freeing page buffer\n", __func__ );
            free ( buffer );
        }

        STATUS ( STAT_GEEK, "%s - freeing mru node\n", __func__ );
        free ( lmru );
    }

    /* free ram_cache_mru_idx */
    STATUS ( STAT_PRG, "%s - releasing ram cache MRU index\n", __func__ );
    KVectorRelease ( self -> ram_cache_mru_idx );

    /* free ram_cache */
    STATUS ( STAT_PRG, "%s - releasing ram cache\n", __func__ );
    KVectorRelease ( self -> ram_cache );

    /* release source */
    STATUS ( STAT_PRG, "%s - releasing source file\n", __func__ );
    KFileRelease ( self -> source );

    /* release chunked reader */
    STATUS ( STAT_PRG, "%s - releasing chunked reader\n", __func__ );
    KChunkReaderRelease ( self -> chunks );

    /* promote cache_file */
    if ( self -> try_promote_on_close )
    {
        STATUS ( STAT_PRG, "%s - try to promote on release\n", __func__ );
        if ( is_bitmap_complete_v3( self ) )
        {
            STATUS ( STAT_PRG, "%s - cache is complete, promotion possible\n", __func__ );
            finalize_v3( self );
        }
    }

    /* free bitmap */
    STATUS ( STAT_PRG, "%s - freeing bitmap\n", __func__ );
    free ( ( void * ) self -> bitmap );

    /* release cache_file */
    STATUS ( STAT_PRG, "%s - releasing cache file\n", __func__ );
    KFileRelease ( self -> cache_file );

    /* delete cache file */
    if ( self -> remove_on_close )
    {
        STATUS ( STAT_PRG, "%s - removing cache-file on exit\n", __func__ );        
        KDirectoryRemove ( self -> dir, false, "%s.cache", self -> path );
    }

    /* remove any file removal task */
    if ( self -> procmgr != NULL )
    {
        KProcMgrRemoveCleanupTask ( self -> procmgr, & self -> rm_file_tkt );
        KProcMgrRelease ( self -> procmgr );
    }
    
    /* release directory */
    STATUS ( STAT_PRG, "%s - releasing cache file directory\n", __func__ );
    KDirectoryRelease ( self -> dir );

    STATUS ( STAT_PRG, "%s - freeing self\n", __func__ );
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
bool KCacheTeeFilePageInCache ( const KCacheTeeFile_v3 * self, const size_t pg_idx )
{
    /* NB - assumes bitmap is locked against modification */
    assert ( ( pg_idx / BMWORDSIZE ) * sizeof self -> bitmap [ 0 ] < self -> bmap_size );
    return ( self -> bitmap [ pg_idx >> BMWORDBITS ] & ( 1 << ( pg_idx & BMWORDMASK ) ) ) != 0;
}

static
size_t KCacheTeeFileReadFromRAM ( const KCacheTeeFile_v3 *self, uint64_t pos,
    uint8_t *buffer, size_t bsize, size_t *num_read, size_t initial_page_idx )
{
    uint32_t offset = ( uint32_t ) pos & ( self -> page_size - 1 );
    uint32_t num_copied, to_copy = self -> page_size - offset;

    size_t i, total;

    STATUS ( STAT_PRG, "%s - reading contiguous pages from RAM cache\n", __func__ );
    for ( i = total = 0; total < bsize; total += num_copied, ++ i )
    {
        rc_t rc;
        uint8_t * pg = NULL;

        ( void ) rc;

        STATUS ( STAT_PRG, "%s - retrieving page %zu from RAM cache\n"
                 , __func__
                 , initial_page_idx + i
            );
        rc = KVectorGetPtr ( self -> ram_cache, initial_page_idx + i, ( void ** ) & pg );
        if ( rc != 0 )
        {
            STATUS ( STAT_QA, "%s - RAM cache read error: %R\n", __func__, rc );
            break;
        }
        if ( pg == NULL )
        {
            STATUS ( STAT_QA, "%s - page %zu not present in RAM cache\n"
                     , __func__
                     , initial_page_idx + i
                );
            break;
        }

        if ( total + to_copy > bsize )
        {
            to_copy = bsize - total;
            STATUS ( STAT_GEEK, "%s - limiting bytes to copy to %zu\n", __func__, to_copy );
        }

        STATUS ( STAT_PRG, "%s - copying %zu bytes to read buffer\n", __func__, to_copy );
        memmove ( & buffer [ total ], & pg [ offset ], num_copied = to_copy );

        to_copy = self -> page_size;
        offset = 0;
    }

    STATUS ( STAT_PRG, "%s - copied %zu bytes total\n", __func__, total );
    * num_read = total;
    return total;
}

static
uint32_t bmword_contig_bits ( const bmap_t word, const uint32_t initial_bit_pos, bool * found_zero )
{
    /* NB - assumes a masked word, so counts all bits set */

    /* EXPLANATION

      assuming 32-bit bmap word and given a word such as:

        0bxxxxxxxxxxxxxxxxxxxx100000000000

      we can find the position of the right-most bit
      with the function uint32_lsbit(), returning 0..31
      unless the input is 0, in which case it returns
      an invalid ( negative ) index.

      we can also isolate that bit as a value by either
      shifting the constant 1U to the starting position:

        ( 1U << starting_position )

      or by some whole word trickery:

        ( word & - word )

      if we assume that the bits to the left look like

        0bxxxxxxxxxx0111111111100000000000

      we can find the position of the right-most ZERO bit
      to the left of the right-most ONE bit by adding the
      value at the right-most ONE bit to the word, causing
      a carry into the ZERO bit:

        0bxxxxxxxxxx0111111111100000000000
                        +
        0b00000000000000000000100000000000
                        =
        0bxxxxxxxxxx1000000000000000000000

      we can then find the position of the right-most bit of
      the sum which will be exactly the position of the zero
      bit we were looking for. The difference between the two
      positions will give the number of contiguous bits that
      were set from the initial bit.

      there are edge cases to consider:
        a. word is zero
        b. word is all ones
        c. left-most contiguous bit is MSB

    */

    bmap_t first_one_value;
    int first_one_pos, first_zero_pos;

    /* edge case "a" */
    if ( word == 0 )
    {
        * found_zero = true;
        return 0;
    }

    /* edge case "b" */
    if ( ~ word == 0 )
        return BMWORDSIZE;

    /* find position of right-most bit */
    first_one_pos = bmap_lsbit ( word );
    assert ( first_one_pos >= 0 );

    /* detect case where the initial bit is NOT set */
    assert ( initial_bit_pos < BMWORDSIZE );
    if ( ( uint32_t ) first_one_pos > initial_bit_pos )
    {
        * found_zero = true;
        return 0;
    }

    /* in truth, the initial_bit_pos should be identical */
    assert ( ( uint32_t ) first_one_pos == initial_bit_pos );

    /* generate the value at right-most bit */
    first_one_value = word & - ( sbmap_t ) word;

    /* find the position of the right-most zero beyond right-most one */
    first_zero_pos = bmap_lsbit ( word + first_one_value );

    /* handle edge case "c" when "word + first_one_value" will roll over to 0 */
    if ( first_zero_pos < 0 )
        return BMWORDSIZE - first_one_pos;

    * found_zero = true;
    assert ( first_one_pos < first_zero_pos );
    return first_zero_pos - first_one_pos;
}

static
uint32_t KCacheTeeFileContigPagesInFileCache ( const KCacheTeeFile_v3 * self,
    size_t initial_page_idx, size_t end_page_idx )
{
    size_t i, last;
    bool found_zero;
    uint32_t initial_bit_pos;
    uint32_t count, max_page_count;
    bmap_t word, initial_mask;

    found_zero = false;

    i = initial_page_idx >> BMWORDBITS;
    STATUS ( STAT_GEEK, "%s - initial page idx=%zu, end page idx=%zu, initial word idx=%zu\n"
             , __func__
             , initial_page_idx
             , end_page_idx
             , i
        );

    /* generate mask against left-most word */
    initial_bit_pos = ( uint32_t ) ( initial_page_idx & BMWORDMASK );
    /* TBD - this will break in 64-bit */
    initial_mask = ( uint32_t ) ( 0xFFFFFFFFU << ( initial_page_idx & BMWORDMASK ) );
    STATUS ( STAT_GEEK, "%s - initial word bitpos=%u, initial mask=0b%0*lb\n"
             , __func__
             , initial_bit_pos
             , BMWORDSIZE
             , ( uint64_t ) initial_mask
        );

    /* NB - assumes bitmap is locked against modification */

    /* get the initial word from bitmap */
    word = self -> bitmap [ i ] & initial_mask;
    STATUS ( STAT_GEEK, "%s - initial masked word=0b%0*lb\n"
             , __func__
             , BMWORDSIZE
             , ( uint64_t ) word
        );
    
    /* limit the count after seeing max pages */
    assert ( initial_page_idx < end_page_idx );
    max_page_count = ( uint32_t ) ( end_page_idx - initial_page_idx );
    STATUS ( STAT_GEEK, "%s - max page count=%u\n"
             , __func__
             , max_page_count
        );

    /* count the bits in the initial word */
    count = bmword_contig_bits ( word, initial_bit_pos, & found_zero );
    STATUS ( STAT_GEEK, "%s - initial contiguous page count=%u, found zero=%s\n"
             , __func__
             , count
             , found_zero ? "true" : "false"
        );

    /* have we seen enough already? */
    if ( count > max_page_count )
    {
        STATUS ( STAT_PRG, "%s - early bailout: found > %u pages\n", __func__, max_page_count );
        return max_page_count;
    }

    /* did we break contiguity? */
    if ( found_zero )
        return count;

    /* calculate the last word index */
    last = ( end_page_idx - 1 ) >> BMWORDBITS;

    /* if the last page was within the same word,
       we'd have returned already. */
    assert ( last >= i );

    /* walk across the bitmap */
    STATUS ( STAT_PRG, "%s - walking from word index %zu to %zu, inclusive\n"
             , __func__
             , i + 1
             , last
        );
    for ( ++ i ; i <= last; ++ i )
    {
        /* retrieve bitmap word */
        word = self -> bitmap [ i ];
        STATUS ( STAT_GEEK, "%s - word[%zu]=0b%0*lb\n"
                 , __func__
                 , i
                 , BMWORDSIZE
                 , ( uint64_t ) word
            );

        /* count the number of bits from leading */
        count += bmword_contig_bits ( word, 0, & found_zero );
        STATUS ( STAT_GEEK, "%s - contiguous page count=%u, found zero=%s\n"
                 , __func__
                 , count
                 , found_zero ? "true" : "false"
            );

        /* have we seen enough already? */
        if ( count > max_page_count )
        {
            STATUS ( STAT_PRG, "%s - early bailout: found > %u pages\n", __func__, max_page_count );
            return max_page_count;
        }

        /* did we break contiguity? */
        if ( found_zero )
            break;
    }

    return count;
}

static
rc_t KCacheTeeFileReadFromFile ( const KCacheTeeFile_v3 *self, uint64_t pos,
    void *buffer, size_t bsize, size_t *num_read, size_t initial_page_idx )
{
    uint64_t end;
    size_t end_page_idx;
    uint32_t num_contig_pages;

    /* ending index - non-inclusive */
    assert ( self -> page_size != 0 );
    assert ( pos / self -> page_size == ( uint64_t ) initial_page_idx );
    end_page_idx = ( size_t ) ( ( pos + bsize + self -> page_size - 1 ) / self -> page_size );

    /* we would not be called if there were no pages available to read */
    STATUS ( STAT_PRG, "%s - counting number of contiguous pages in cache file\n", __func__ );
    num_contig_pages = KCacheTeeFileContigPagesInFileCache ( self, initial_page_idx, end_page_idx );
    STATUS ( STAT_PRG, "%s - count is %u starting from idx %zu\n"
             , __func__
             , num_contig_pages
             , initial_page_idx
        );
    assert ( num_contig_pages != 0 );
    if ( num_contig_pages == 0 )
    {
        * num_read = 0;
        return RC ( rcFS, rcFile, rcReading, rcError, rcUnexpected );
    }

    /* reset the end_page_idx */
    end_page_idx = initial_page_idx + num_contig_pages;

    /* calculate the number of bytes to read in one fell swoop */
    end = end_page_idx * self -> page_size;
    if ( pos + bsize > end )
        bsize = ( size_t ) ( end - pos );

    /* read from the cache file */
    STATUS ( STAT_PRG, "%s - reading %zu bytes from cache file @ %lu\n"
             , __func__
             , bsize
             , pos
        );
    return KFileReadAll_v1 ( self -> cache_file, pos, buffer, bsize, num_read );
}

static
rc_t CC KCacheTeeFileTimedRead ( const KCacheTeeFile_v3 *cself, uint64_t pos,
    void *buffer, size_t bsize, size_t *num_read, struct timeout_t *tm )
{
    rc_t rc = 0;
    size_t count, initial_page_idx;

    /* the "msgq" is "mutable" */
    KCacheTeeFile_v3 * self = ( KCacheTeeFile_v3 * ) cself;

    uint64_t cur_thread_id = CUR_THREAD_ID ();

    /* 1. limit request to file dimensions */
    if ( pos >= self -> source_size || bsize == 0 )
    {
        STATUS ( STAT_GEEK, "%lu: %s - read starts beyond EOF\n", cur_thread_id, __func__ );
        * num_read = 0;
        return 0;
    }
    if ( ( pos + bsize ) > self -> source_size )
    {
        STATUS ( STAT_GEEK, "%lu: %s - read ends beyond EOF\n", cur_thread_id, __func__ );
        bsize = ( size_t ) ( self -> source_size - pos );
    }

    /* 2. transform "pos" into initial page index */
    assert ( self -> page_size != 0 );
    initial_page_idx = ( size_t ) ( pos / self -> page_size );
    STATUS ( STAT_GEEK, "%lu: %s - read starts at page %zu\n", cur_thread_id, __func__, initial_page_idx );

    /* 3. acquire lock */
    STATUS ( STAT_PRG, "%lu: %s - acquiring cache mutex\n", cur_thread_id, __func__ );
    rc = KLockAcquire ( self -> cache_lock );
    if ( rc == 0 )
    {
        /* 4. check for existence of initial page in cache */
        STATUS ( STAT_PRG, "%lu: %s - testing for existence of starting page in cache\n", cur_thread_id, __func__ );
        while ( ! KCacheTeeFilePageInCache ( self, initial_page_idx ) )
        {
            KCacheTeeFileMsg msg;
            
            STATUS ( STAT_PRG, "%lu: %s - starting page not found in cache\n", cur_thread_id, __func__ );

            /* 5. deliver read request message to bg thread */
            msg . pos = pos;
            msg . size = bsize;
            msg . initial_page_idx = initial_page_idx;
            msg . have_tm = false;
            if ( tm != NULL )
            {
                msg . tm = * tm;
                msg . have_tm = true;
            }

            STATUS ( STAT_GEEK, "%lu: %s - populated message object "
                     "{ pos=%lu, size=%zu, tm=%d%s, initial_page_idx=%zu }\n"
                     , cur_thread_id
                     , __func__
                     , msg . pos
                     , msg . size
                     , ( msg . have_tm ) ? ( int ) msg . tm . mS : -1
                     , ( msg . have_tm ) ? "mS (present)" : " (infinite)"
                     , msg . initial_page_idx
                );

            STATUS ( STAT_PRG, "%lu: %s - acquiring queue lock\n", cur_thread_id, __func__ );
            rc = KLockAcquire ( self -> qlock );
            if ( rc != 0 )
            {
                STATUS ( STAT_QA, "%lu: %s - failed to acquire queue lock: %R - releasing cache mutex\n", cur_thread_id, __func__, rc );
                KLockUnlock ( self -> cache_lock );
                return rc;
            }

            STATUS ( STAT_PRG, "%lu: %s - queueing message\n", cur_thread_id, __func__ );
            DLListPushTail ( & self -> msgq, & msg . dad );

            STATUS ( STAT_PRG, "%lu: %s - signaling bg thread\n", cur_thread_id, __func__ );
            rc = KConditionSignal ( self -> bgcond );

            STATUS ( STAT_PRG, "%lu: %s - releasing queue lock\n", cur_thread_id, __func__ );
            KLockUnlock ( self -> qlock );

            if ( rc != 0 )
            {
                STATUS ( STAT_QA, "%lu: %s - failed to signal bg thread: %R - releasing cache mutex\n", cur_thread_id, __func__, rc );
                KLockUnlock ( self -> cache_lock );
                return rc;
            }
            
            /* 6. wait for event */
            STATUS ( STAT_PRG, "%lu: %s - waiting on broadcast from bg thread\n", cur_thread_id, __func__ );
            rc = KConditionTimedWait ( self -> fgcond, self -> cache_lock, tm );

            /* remove msg from queue */
            STATUS ( STAT_PRG, "%lu: %s - acquiring queue lock\n", cur_thread_id, __func__ );
            KLockAcquire ( self -> qlock );

            STATUS ( STAT_PRG, "%lu: %s - unlink msg from msg queue if still there\n", cur_thread_id, __func__ );
            DLListUnlink ( & self -> msgq, & msg . dad );

            STATUS ( STAT_PRG, "%lu: %s - releasing queue lock\n", cur_thread_id, __func__ );
            KLockUnlock ( self -> qlock );

            /* test result of timed wait */
            if ( rc != 0 )
            {
                STATUS ( STAT_QA, "%lu: %s - timed wait failed: %R - releasing cache mutex\n", cur_thread_id, __func__, rc );
                KLockUnlock ( self -> cache_lock );
                return rc;
            }

            STATUS ( STAT_PRG, "%lu: %s - testing for existence of starting page in cache\n", cur_thread_id, __func__ );
        }
        /* end while ( ! in cache ) */

        STATUS ( STAT_PRG, "%lu: %s - starting page found in cache\n", cur_thread_id, __func__ );

        /* 7. try to read from RAM */
        STATUS ( STAT_PRG, "%lu: %s - attempt to read from RAM cache\n", cur_thread_id, __func__ );
        count = KCacheTeeFileReadFromRAM ( self, pos, buffer, bsize, num_read, initial_page_idx );
        if ( count == 0 )
        {
            STATUS ( STAT_PRG, "%lu: %s - page %zu not found in RAM cache\n", cur_thread_id, __func__, initial_page_idx );

            /* 8. read from file */
            STATUS ( STAT_PRG, "%lu: %s - attempt to read from cache file\n", cur_thread_id, __func__ );
            assert ( self -> cache_file != NULL );
            rc = KCacheTeeFileReadFromFile ( self, pos, buffer, bsize, num_read, initial_page_idx );
        }

        /* 9. release lock */
        STATUS ( STAT_PRG, "%lu: %s - releasing cache mutex\n", cur_thread_id, __func__ );
        KLockUnlock ( self -> cache_lock );
    }

    return rc;
}

static
rc_t CC KCacheTeeFileRead ( const KCacheTeeFile_v3 *self, uint64_t pos,
    void *buffer, size_t bsize, size_t *num_read )
{
    struct timeout_t tm;
    /* TBD - need a default timeout on the manager object */
    TimeoutInit ( & tm, 100 * 1000 );
    return KCacheTeeFileTimedRead ( self, pos, buffer, bsize, num_read, & tm );
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

    STATUS ( STAT_PRG, "%s - chunked read from cache-tee file\n", __func__ );

    for ( total = 0; rc == 0 && total < bsize; total += num_read )
    {
        void * chbuf;
        size_t chsize;

        STATUS ( STAT_GEEK, "%s - popping buffer\n", __func__ );
        rc = KChunkReaderNextBuffer ( chunks, & chbuf, & chsize );
        if ( rc == 0 )
        {
            size_t to_read = chsize;
            if ( total + chsize > bsize )
                to_read = bsize - total;
            
            STATUS ( STAT_PRG, "%s - reading from file @ %lu\n", __func__, pos + total );
            rc = KFileReadAll_v1 ( & self -> dad, pos + total, chbuf, to_read, & num_read );
            if ( rc == 0 && num_read != 0 )
            {
                STATUS ( STAT_PRG, "%s - consuming chunk of %zu bytes @ %lu\n"
                         , __func__
                         , num_read
                         , pos + total
                    );
                rc = KChunkReaderConsumeChunk ( chunks, pos + total, chbuf, num_read );
            }
            STATUS ( STAT_GEEK, "%s - returning buffer\n", __func__ );
            KChunkReaderReturnBuffer ( chunks, chbuf, chsize );
        }

        if ( num_read == 0 )
            break;
    }

    STATUS ( STAT_GEEK, "%s - read %zu bytes\n", __func__, total );
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

    STATUS ( STAT_PRG, "%s - timed chunked read from cache-tee file\n", __func__ );

    for ( total = 0; rc == 0 && total < bsize; total += num_read )
    {
        void * chbuf;
        size_t chsize;

        STATUS ( STAT_GEEK, "%s - popping buffer\n", __func__ );
        rc = KChunkReaderNextBuffer ( chunks, & chbuf, & chsize );
        if ( rc == 0 )
        {
            size_t to_read = chsize;
            if ( total + chsize > bsize )
                to_read = bsize - total;
            
            STATUS ( STAT_PRG, "%s - reading from file @ %lu\n", __func__, pos + total );
            rc = KFileTimedReadAll_v1 ( & self -> dad, pos + total, chbuf, to_read, & num_read, tm );
            if ( rc == 0 && num_read != 0 )
            {
                STATUS ( STAT_PRG, "%s - consuming chunk of %zu bytes @ %lu\n"
                         , __func__
                         , num_read
                         , pos + total
                    );
                rc = KChunkReaderConsumeChunk ( chunks, pos + total, chbuf, num_read );
            }
            STATUS ( STAT_GEEK, "%s - returning buffer\n", __func__ );
            KChunkReaderReturnBuffer ( chunks, chbuf, chsize );
        }

        if ( num_read == 0 )
            break;
    }

    STATUS ( STAT_GEEK, "%s - read %zu bytes\n", __func__, total );
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

    /* this is currently a minimum amount,
       but anything more will still be in whole page increments */
    size_t min_read_amount = self -> page_size * self -> cluster_fact;

    STATUS ( STAT_PRG, "BG: %s - entering loop\n", __func__ );
    while ( ! self -> quitting )
    {
        STATUS ( STAT_PRG, "BG: %s - acquiring queue lock\n", __func__ );
        rc = KLockAcquire ( self -> qlock );
        if ( rc != 0 )
        {
            /* TBD - insert wait */
        }
        else
        {
            KCacheTeeFileMsg msg, * dmsg = NULL;

            /* speculatively pop a message */
            dmsg = ( KCacheTeeFileMsg * )
                DLListPopHead ( & self -> msgq );

            if ( dmsg == NULL )
            {
                struct timeout_t tm;
#if _DEBUGGING
memset ( & tm, -1, sizeof tm );
#endif
                TimeoutInit ( & tm, 100 );
                assert ( tm . mS == 100 );
                assert ( tm . prepared == false );

                STATUS ( STAT_PRG, "BG: %s - waiting on fg signal\n", __func__ );
                KConditionTimedWait ( self -> bgcond, self -> qlock, & tm );

                /* pop a message */
                dmsg = ( KCacheTeeFileMsg * )
                    DLListPopHead ( & self -> msgq );
            }

            /* if the message is there, copy it while we still have the lock */
            if ( dmsg != NULL )
                msg = * dmsg;

            /* now release the lock - "dmsg" can no longer be safely dereferenced */
            KLockUnlock ( self -> qlock );

            /* check to see if "msg" has been initialized with a proper message */
            if ( dmsg != NULL )
            {
                size_t num_read;
                size_t end_page_idx;

                STATUS ( STAT_PRG, "BG: %s - received msg { pos=%lu, size=%zu, tm=%d%s, initial_page_idx=%zu }\n"
                         , __func__
                         , msg . pos
                         , msg . size
                         , ( msg . have_tm ) ? ( int ) msg . tm . mS : -1
                         , ( msg . have_tm ) ? "mS (present)" : " (infinite)"
                         , msg . initial_page_idx
                    );

                if ( self -> whole_file )
                {
                    msg . pos = 0;
                    msg . size = self -> source_size;
                    msg . initial_page_idx = 0;
                    STATUS ( STAT_PRG, "BG: %s - mapping request to whole file "
                             "{ pos=%lu, size=%zu, tm=%s, initial_page_idx=%zu }\n"
                             , __func__
                             , msg . pos
                             , msg . size
                             , ( msg . have_tm ) ? "present" : "infinite"
                             , msg . initial_page_idx
                        );
                }

                /* we will deal in pages, not bytes */
                end_page_idx =
                    ( size_t ) ( ( msg . pos + msg . size + self -> page_size - 1 ) / self -> page_size );

                STATUS ( STAT_GEEK, "BG: %s - calculated end_page_idx=%zu\n"
                         , __func__
                         , end_page_idx
                    );

                /* test if any pages appeared since caller posted message */
                STATUS ( STAT_PRG, "BG: %s - testing for existence of page %zu\n"
                         , __func__
                         , msg . initial_page_idx
                    );
                if ( KCacheTeeFilePageInCache ( self, msg . initial_page_idx ) )
                {
                    /* check how many were there */
                    uint32_t num_contig_pages;

                    STATUS ( STAT_PRG, "BG: %s - found. calculating number of pages actually there\n", __func__ );
                    num_contig_pages =
                        KCacheTeeFileContigPagesInFileCache ( self, msg . initial_page_idx, end_page_idx );

                    STATUS ( STAT_PRG, "BG: %s - %u contiguous pages found\n", __func__, num_contig_pages );
                    assert ( num_contig_pages != 0 );

                    /* remove them from the request */
                    msg . initial_page_idx += num_contig_pages;

                    /* tell callers that something changed since they posted message */
                    STATUS ( STAT_PRG, "BG: %s - broadcasting event to all waiting readers\n", __func__ );
                    KConditionBroadcast ( self -> fgcond );
                }
                else
                {
                    STATUS ( STAT_PRG, "BG: %s - page %zu not found.\n", __func__, msg . initial_page_idx );
                }

                STATUS ( STAT_PRG, "BG: %s - testing number of pages to read\n", __func__ );
                if ( msg . initial_page_idx < end_page_idx )
                {
                    /* align starting position */
                    msg . pos &= ~ ( size_t ) ( self -> page_size - 1 );

                    /* determine read amount */
                    msg . size = ( end_page_idx - msg . initial_page_idx ) * self -> page_size;

                    /* potentially amplify */
                    if ( msg . size < min_read_amount )
                        msg . size = min_read_amount;

                    /* issue a chunked read to source file
                       if the request has unsatisfied starting position */
                    if ( msg . have_tm == false )
                    {
                        STATUS ( STAT_PRG, "BG: %s - chunked read of %zu source bytes @ %lu\n"
                                 , __func__
                                 , msg . size
                                 , msg . pos
                            );
                        rc = KFileReadChunked ( self -> source, msg . pos, self -> chunks, msg . size, & num_read );
                        STATUS ( STAT_PRG, "BG: %s - rc=%R, num_read=%zu\n", __func__, rc, num_read );
                    }
                    else
                    {
                        STATUS ( STAT_PRG, "BG: %s - timed chunked read of %zu source bytes @ %lu\n"
                                 , __func__
                                 , msg . size
                                 , msg . pos
                            );
                        rc = KFileTimedReadChunked ( self -> source, msg . pos,
                            self -> chunks, msg . size, & num_read, & msg . tm );
                        STATUS ( STAT_PRG, "BG: %s - rc=%R, num_read=%zu\n", __func__, rc, num_read );
                    }
                }
                else
                {
                    STATUS ( STAT_PRG, "BG: %s - no pages to read, message ignored\n", __func__ );
                }
            }
        } /* end if ( KLockAcquire () ) */
    }
    STATUS ( STAT_PRG, "BG: %s - exiting loop and background thread\n", __func__ );

    return rc;
}

static
rc_t CC KCacheTeeFileRunThread ( const KThread * t, void * data )
{
    rc_t rc;
    KCacheTeeFile_v3 * self = data;

    STATUS ( STAT_PRG, "BG: %s - starting thread\n", __func__ );

    STATUS ( STAT_PRG, "BG: %s - acquiring cache_lock\n", __func__ );
    rc = KLockAcquire ( self -> cache_lock );
    if ( rc != 0 )
    {
        PLOGERR ( klogSys, ( klogSys, rc, "BG: $(func) - failed to acquire cache mutex"
                             , "func=%s"
                             , __func__
                      ) );
        self -> quitting = true;
        return rc;
    }

    STATUS ( STAT_PRG, "BG: %s - signaling FG to sync\n", __func__ );
    rc = KConditionSignal ( self -> fgcond );
    STATUS ( STAT_PRG, "BG: %s - releasing cache_lock\n", __func__ );
    KLockUnlock ( self -> cache_lock );
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

    STATUS ( STAT_PRG, "%s - acquiring cache lock\n", __func__ );
    rc = KLockAcquire ( self -> cache_lock );
    if ( rc != 0 )
    {
        PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to acquire cache mutex"
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
            rc = KConditionWait ( self -> fgcond, self -> cache_lock );
            if ( rc != 0 )
            {
                PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to wait on condition signal"
                                     , "func=%s"
                                     , __func__
                              ) );
            }
        }

        STATUS ( STAT_PRG, "%s - releasing cache lock\n", __func__ );
        KLockUnlock ( self -> cache_lock );
    }

    return rc;
}

static
rc_t KCacheTeeFileInitSync ( KCacheTeeFile_v3 * self )
{
    rc_t rc;

    STATUS ( STAT_PRG, "%s - initializing fg->bg message queue\n", __func__ );
    DLListInit ( & self -> msgq );

    STATUS ( STAT_PRG, "%s - allocating mutices\n", __func__ );
    rc = KLockMake ( & self -> cache_lock );
    if ( rc != 0 )
    {
        PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to create cache mutex"
                             , "func=%s"
                             , __func__
                      ) );
    }
    else
    {
        rc = KLockMake ( & self -> qlock );
        if ( rc != 0 )
        {
            PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to create queue mutex"
                                 , "func=%s"
                                 , __func__
                          ) );
        }
        else
        {
            STATUS ( STAT_PRG, "%s - allocating conditions\n", __func__ );
            rc = KConditionMake ( & self -> bgcond );
            if ( rc != 0 )
            {
                PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to create bg condition"
                                     , "func=%s"
                                     , __func__
                              ) );
            }
            else
            {
                rc = KConditionMake ( & self -> fgcond );
                if ( rc != 0 )
                {
                    PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to create fg condition"
                                         , "func=%s"
                                         , __func__
                                  ) );
                }
            }
        }
    }

    return rc;
}

static
rc_t KCacheTeeFileInitNew ( KCacheTeeFile_v3 * self )
{
    rc_t rc;
    bool unlinked = false;
    uint64_t calculated_eof;

    if ( self -> remove_on_close )
    {
#if WINDOWS
        KProcMgr * procmgr;
        rc = KProcMgrMakeSingleton ( & procmgr );
        if ( rc != 0 )
        {
            PLOGERR ( klogSys, ( klogWarn, rc, "$(func) - failed to access process manager for '%s.cache'. Cannot guarantee cache cleanup."
                                 , "func=%s,path=%s"
                                 , __func__
                                 , self -> path
                          ) );
        }
        else
        {
            KTask * t;
            rc = KRemoveFileTaskMake ( & t, self -> dir, "%s", self -> path );
            if ( rc != 0 )
            {
                PLOGERR ( klogSys, ( klogWarn, rc, "$(func) - failed to create cleanup task for '%s.cache'. Cannot guarantee cache cleanup."
                                     , "func=%s,path=%s"
                                     , __func__
                                     , self -> path
                              ) );
            }
            else
            {
                rc = KProcMgrAddCleanupTask ( procmgr, & self -> rm_file_tkt, t );
                if ( rc != 0 )
                {
                    PLOGERR ( klogSys, ( klogWarn, rc, "$(func) - failed to set cleanup task for '%s.cache'. Cannot guarantee cache cleanup."
                                         , "func=%s,path=%s"
                                         , __func__
                                         , self -> path
                                  ) );
                }
                else
                {
                    self -> procmgr = procmgr;
                    procmgr = NULL;
                }

                KTaskRelease ( t );
            }

            KProcMgrRelease ( procmgr );
        }
#else
        STATUS ( STAT_PRG, "%s - removing cache-file '%s.cache' after creation\n", __func__, self -> path );        
        rc = KDirectoryRemove ( self -> dir, false, "%s.cache", self -> path );
        if ( rc != 0 )
        {
            PLOGERR ( klogSys, ( klogWarn, rc, "$(func) - failed to unlink '$(path).cache' after creation. Will try again on close."
                                 , "func=%s,path=%s"
                                 , __func__
                                 , self -> path
                          ) );
        }
        else
        {
            self -> remove_on_close = false;
            unlinked = true;
        }
#endif
    }

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

    if ( rc != 0 && ! unlinked && ! self -> remove_on_close )
    {
        STATUS ( STAT_QA, "%s - marking cache-file '%s.cache' for removal\n", __func__, self -> path );        
        self -> remove_on_close = true;
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
             , "%s - file size = %lu, rc = %R\n"
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
            
            rc = RC ( rcFS, rcFile, rcOpening, rcData, actual_eof == 0 ? rcEmpty : rcUnequal );
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

                rc = RC ( rcFS, rcFile, rcOpening, rcData, rcUnequal );
            }
            else
            {
                /* at this point, we can just read in the existing bitmap */
                STATUS ( STAT_PRG, "%s - reading bitmap of shared cache file '%s.cache'\n", __func__, self -> path );
                rc = KFileReadExactly ( self -> cache_file, self -> source_size,
                                        ( void * ) self -> bitmap, self -> bmap_size );
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

    if (GetRCState(rc) != rcEmpty)
    {
        PLOGMSG(klogWarn, (klogWarn, "$(func) - stale cache file '$(path).cache'. Reinitializing."
            , "func=%s,path=%s"
            , __func__
            , self->path
            ));
    }

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
        rc = KFileWriteExactly ( self -> cache_file, pos,
            ( const void * )  self -> bitmap, self -> bmap_size + sizeof * self -> tail );
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
rc_t KCacheTeeFileOpen ( KCacheTeeFile_v3 * self, KDirectory * dir, const KFile ** promoted )
{
    rc_t rc;
    uint64_t dummy;
    ( void ) dummy;

    STATUS ( STAT_PRG, "%s - duplicating directory reference %#p for possible promotion\n", __func__, dir );
    rc = KDirectoryAddRef ( dir );
    if ( rc == 0 )
    {
        uint32_t wait_loop_count;
        KLockFile * lock = NULL;
        
        self -> dir = dir;

        for ( wait_loop_count = 0; wait_loop_count < 10; ++ wait_loop_count )
        {
            
            STATUS ( STAT_PRG, "%s - attempting to open file '%s' read-only\n", __func__, self -> path );
            rc = KDirectoryOpenFileRead ( dir, promoted, "%s", self -> path );
            if ( rc == 0 )
                STATUS ( STAT_QA, "%s - file '%s' exists\n", __func__, self -> path );
            else
            {
                STATUS ( STAT_PRG, "%s - attempting to lock file '%s' for update\n", __func__, self -> path );
                rc = KDirectoryCreateLockFile ( dir, & lock, "%s.lock", self -> path );
                if ( GetRCState ( rc ) == rcBusy )
                {
                    STATUS ( STAT_QA, "%s - file '%s' is busy - sleeping\n", __func__, self -> path );
                    
                    /* sleep */
                    KSleepMs ( 250 );
                    continue;
                }

                if ( rc != 0 )
                {
                    STATUS ( STAT_QA, "%s - failed to acquire lock for '%s' - %R\n", __func__, self -> path, rc );
                    return rc;
                }
            
                STATUS ( STAT_PRG
                         , "%s - attempting to open file '%s.cache' shared read/write\n"
                         , __func__
                         , self -> path
                    );
                rc = KDirectoryOpenFileSharedWrite ( dir, & self -> cache_file, true, "%s.cache", self -> path );
#if ! WINDOWS
                STATUS ( STAT_GEEK
                         , "%s - open shared file attempt: fd = %d, rc = %R\n"
                         , __func__
                         , ( rc == 0 ) ? KFileGetSysFile ( self -> cache_file, & dummy ) -> fd : -1
                         , rc
                    );
#endif
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
#if ! WINDOWS
                    STATUS ( STAT_GEEK
                             , "%s - create file attempt: fd = %d, rc = %R\n"
                             , __func__
                             , KFileGetSysFile ( self -> cache_file, & dummy ) -> fd
                             , rc
                        );
#endif
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
#if ! WINDOWS
                    STATUS ( STAT_GEEK
                             , "%s - open read-only file attempt: fd = %d, rc = %R\n"
                             , __func__
                             , KFileGetSysFile ( self -> cache_file, & dummy ) -> fd
                             , rc
                        );
#endif
                    if ( rc == 0 )
                        rc = KCacheTeeFileInitExisting ( self );
                }

                STATUS ( STAT_PRG, "%s - releasing lock on file '%s'\n", __func__, self -> path );
                KLockFileRelease ( lock );

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

            /* always break */
            break;
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
    uint32_t * bitmap;
    KCacheTeeFileTail * tail;

    STATUS ( STAT_PRG, "%s - allocating bitmap index\n", __func__ );

    num_pages = ( self -> source_size + self -> page_size - 1 ) / self -> page_size;
    /* keep this hard-coded on 32-bit entries for legacy */
    num_words = ( size_t ) ( num_pages + 31 ) >> 5;
    assert ( sizeof * bitmap == 4 );
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

    self -> bitmap = ( volatile bmap_t * ) bitmap;
    self -> tail = tail;
    self -> bmap_size = bmsize - ( sizeof * tail );

    return 0;
}

static
rc_t KCacheTeeFileMakeRAMCache ( KCacheTeeFile_v3 * self )
{
    rc_t rc;

    STATUS ( STAT_PRG, "%s - allocating ram cache\n", __func__ );
    rc = KVectorMake ( & self -> ram_cache );
    if ( rc == 0 )
    {
        STATUS ( STAT_PRG, "%s - allocating ram cache MRU index\n", __func__ );
        rc = KVectorMake ( & self -> ram_cache_mru_idx );
    }

    return rc;
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
                size_t page_size, uint32_t cluster_factor, uint32_t ram_pages,
                bool try_promote_on_close, bool remove_on_close )
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

    STATUS(STAT_GEEK, "%s - ram_cache_size=%lu MAX_RAM_CACHE_BYTES=%lu\n", __func__, ram_cache_size, MAX_RAM_CACHE_BYTES);

    if ( ram_cache_size > MAX_RAM_CACHE_BYTES )
        ram_pages = (uint32_t) ( MAX_RAM_CACHE_BYTES / self -> page_size );

    self -> ram_limit = ram_pages;
    self -> try_promote_on_close = try_promote_on_close;
    self -> remove_on_close = remove_on_close;
}

static
rc_t KDirectoryVMakeKCacheTeeFileInt ( KDirectory * self,
    const KFile ** tee, const KFile * source,
    size_t page_size, uint32_t cluster_factor, uint32_t ram_pages,
    bool try_promote_on_close, bool remove_on_close,
    const char * nul_term_cache_path )
{
    rc_t rc;
    KCacheTeeFile_v3 * obj;

    STATUS ( STAT_QA, "%s - making cache-tee file v3\n", __func__ );

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
            KCacheTeeFileBindConstants ( obj, page_size, cluster_factor, ram_pages,
                                         try_promote_on_close, remove_on_close );

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
                        if ( nul_term_cache_path == NULL || nul_term_cache_path [ 0 ] == 0 )
                            STATUS ( STAT_PRG, "%s - no file cache will be used\n", __func__ );
                        else
                        {
                            size_t cache_path_size = string_copy_measure
                                ( obj -> path, sizeof obj -> path, nul_term_cache_path );
                            assert ( cache_path_size < sizeof obj -> path );

                            KCacheTeeFileOpen ( obj, self, tee );
                        }
                        
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

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryVMakeKCacheTeeFile_v3 ( KDirectory * self,
    const KFile ** tee, const KFile * source,
    size_t page_size, uint32_t cluster_factor, uint32_t ram_pages,
    bool try_promote_on_close, bool remove_on_close,
    const char * fmt, va_list args )
{
    rc_t rc;

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
        else if ( try_promote_on_close && remove_on_close )
        {
            rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );
        }
        else
        {
            /* detect case where caller wants no physical cache file */
            if ( fmt == NULL || fmt [ 0 ] == 0 )
            {
                /* if ALSO not caching in RAM, then what's up? */
                if ( ram_pages == 0 )
                {
                    /* just return a reference to the input file */
                    STATUS ( STAT_QA, "%lu: %s - no RAM cache or file cache will be used\n"
                             , CUR_THREAD_ID (), __func__ );
                    rc = KFileAddRef ( source );
                    if ( rc == 0 )
                        * tee = source;
                    return rc;
                }

                rc = KDirectoryVMakeKCacheTeeFileInt ( self, tee, source,
                    page_size, cluster_factor, ram_pages, try_promote_on_close, remove_on_close, fmt );
            }
            else
            {
                KCacheTeeFileTreeNode * new_node = malloc ( sizeof * new_node );
                if ( new_node == NULL )
                    rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
                else
                {
                    /* expand cache path */
                    rc = KDirectoryVResolvePath ( self, true, new_node -> path,
                                                  sizeof new_node -> path, fmt, args );
                    if ( rc != 0 )
                    {
                        PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to resolve cache file path"
                                             , "func=%s"
                                             , __func__
                                      ) );
                    }
                    else
                    {

                        /* protect cache FILE from multiple threads and processes */
                        int status = ENTER_CRIT_SECTION ();
                        if ( status != 0 )
                            rc = RC ( rcFS, rcDirectory, rcConstructing, rcLock, rcUnknown );
                        else
                        {
                            const KCacheTeeFileTreeNode * node = ( const KCacheTeeFileTreeNode * )
                                BSTreeFind ( & open_cache_tee_files, new_node -> path, KCacheTeeFileTreeNodeFind );
                            if ( node != NULL )
                            {
                                free ( new_node );
                                rc = KFileAddRef ( node -> file );
                                if ( rc == 0 )
                                    * tee = node -> file;
                            }
                            else
                            {
                                rc = KDirectoryVMakeKCacheTeeFileInt ( self, & new_node -> file, source,
                                    page_size, cluster_factor, ram_pages, try_promote_on_close,
                                    remove_on_close, new_node -> path );
                                if ( rc != 0 )
                                    free ( new_node );
                                else if ( ( ( const KCacheTeeFile_v3 * ) new_node -> file ) -> cache_file != NULL )
                                {
                                    rc = BSTreeInsertUnique ( & open_cache_tee_files,
                                        & new_node -> dad, NULL, KCacheTeeFileTreeNodeSort );
                                    if ( rc == 0 )
                                        * tee = new_node -> file;
                                    else
                                    {
                                        KFileRelease ( new_node -> file );
                                        free ( new_node );
                                    }
                                }
                                else
                                {
                                    * tee = new_node -> file;
                                    free ( new_node );
                                }
                            }
                        }
            
                        EXIT_CRIT_SECTION ();
                    }
                }
            }
        }
    }

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryMakeKCacheTeeFile_v3 ( KDirectory * self,
    const KFile ** tee, const KFile * source,
    size_t page_size, uint32_t cluster_factor, uint32_t ram_pages,
    bool try_promote_on_close, bool remove_on_close,
    const char * fmt, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, fmt );

    rc = KDirectoryVMakeKCacheTeeFile_v3 ( self, tee, source, page_size, cluster_factor, ram_pages,
                                           try_promote_on_close, remove_on_close, fmt, args );

    va_end ( args );

    return rc;
}

/* --- check for invariants --- */
static rc_t extract_tail( const KFile * self, uint64_t * over_all_size, KCacheTeeFileTail * tail )
{
    rc_t rc = KFileSize ( self, over_all_size );
    if ( rc != 0 )
    {
        PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to obtain file size"
                         , "func=%s"
                         , __func__
                  ) );
    }
    else
    {
        size_t num_read;
        uint64_t pos = * over_all_size - sizeof *tail;
        rc = KFileReadAll_v1 ( self, pos, ( void * )tail, sizeof *tail, &num_read );
        if ( rc != 0 )
        {
            PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to read tail of cachefile"
                             , "func=%s"
                             , __func__
                      ) );
        }
        else
        {
            if ( num_read != sizeof *tail )
            {
                rc = RC ( rcFS, rcFile, rcValidating, rcParam, rcInvalid );
                PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to read enough bytes of tail of cachefile"
                                 , "func=%s"
                                 , __func__
                          ) );
            }
        }
    }
    return rc;
}

static size_t calculate_bitmap_size_in_bytes( const KCacheTeeFileTail * tail )
{
    size_t res = 0;
    size_t num_pages = ( tail -> orig_size + tail -> page_size - 1 ) / tail -> page_size;
    size_t num_words = ( size_t ) ( num_pages + ( BMWORDSIZE - 1 ) ) >> BMWORDBITS;
    res = num_words * ( BMWORDSIZE >> 3 );
    return res;
}

static bmap_t* extract_bitmap( const KFile * self, size_t orig_size, size_t bitmap_size_in_bytes )
{
    bmap_t* res = malloc( bitmap_size_in_bytes );
    if ( res != NULL )
    {
        size_t num_read;
        rc_t rc = KFileReadAll_v1 ( self, orig_size, ( void * )res, bitmap_size_in_bytes, &num_read );
        if ( rc != 0 )
        {
            PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to read bitmap of cachefile"
                             , "func=%s"
                             , __func__
                      ) );
        }
        else if ( num_read != bitmap_size_in_bytes )
        {
            rc = RC ( rcFS, rcFile, rcValidating, rcParam, rcInvalid );
            PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to read enough bytes of bitmap of cachefile"
                             , "func=%s"
                             , __func__
                      ) );
            free( ( void * ) res );
            res = NULL;
        }
    }
    return res;
}

LIB_EXPORT rc_t CC CacheTee3FileIsValid ( struct KFile const * self, bool * is_valid )
{
    rc_t rc = 0;
    if ( self == NULL || is_valid == NULL )
        rc = RC ( rcFS, rcFile, rcValidating, rcParam, rcNull );
    else
    {
        uint64_t over_all_size;
        KCacheTeeFileTail tail;
        rc = extract_tail( self, &over_all_size, &tail );
        if ( rc == 0 )
        {
            if ( tail . orig_size < over_all_size &&
                 tail . page_size < tail . orig_size )
            {
                size_t bitmap_size_in_bytes = calculate_bitmap_size_in_bytes( &tail );
                *is_valid = ( ( tail . orig_size + bitmap_size_in_bytes + sizeof tail ) == over_all_size );
            }
        }
    }
    return rc;
}

/* --- check for completeness --- */

static uint32_t count_set_bits( bmap_t value )
{
    uint32_t ret = 0;
    while ( value > 0 )
    {
        ret += ( value & 1 );
        value >>= 1;
    }
    return ret;
}

static bool is_bitmap_complete( volatile bmap_t * bitmap, size_t bitmap_size_in_bytes,
                                uint64_t orig_size, uint32_t page_size )
{
    size_t bitmap_words = bitmap_size_in_bytes / ( BMWORDSIZE >> 3 );
    size_t num_pages, in_last_word;
    size_t idx;
    /* all but the last word in the bitmap has to be set to 0xFFFFFFFFU */
    for ( idx = 0; idx < ( bitmap_words - 1 ); ++idx )
    {
#if BMWORDSIZE == 32
        if ( bitmap[ idx ] != 0xFFFFFFFFU )
            return false;
#else
    #error "this code does not support this word size"
#endif
    }
    /* how many bits have to be set in the last word? */
    num_pages = ( orig_size + page_size - 1 ) / page_size;
    in_last_word = num_pages - ( ( bitmap_words - 1 ) * BMWORDSIZE );
    return ( count_set_bits( bitmap[ bitmap_words - 1 ] ) == in_last_word );
}

static bool is_bitmap_complete_file( const KFile * self )
{
    bool res = false;
    uint64_t over_all_size;
    KCacheTeeFileTail tail;
    rc_t rc = extract_tail( self, &over_all_size, &tail );
    if ( rc == 0 )
    {
        size_t bitmap_size_in_bytes = calculate_bitmap_size_in_bytes( &tail );
        if ( tail . orig_size + bitmap_size_in_bytes + sizeof tail != over_all_size )
        {
            rc = RC ( rcFS, rcFile, rcValidating, rcParam, rcInvalid );
            PLOGERR ( klogSys, ( klogSys, rc, "$(func) - invalid internal size-values in cachefile"
                             , "func=%s"
                             , __func__
                      ) );
        }
        else
        {
            volatile bmap_t * bitmap = extract_bitmap( self, tail . orig_size, bitmap_size_in_bytes );
            if ( bitmap != NULL )
            {
                res = is_bitmap_complete( bitmap, bitmap_size_in_bytes, tail . orig_size, tail . page_size );
                free( ( void * ) bitmap );
            }
        }
    }
    return res;
}

static bool is_bitmap_complete_v3( const KCacheTeeFile_v3 * self )
{
    if ( self -> cache_file == NULL )
        return false;
    return is_bitmap_complete( self -> bitmap, self -> bmap_size,
                               self -> source_size, self -> page_size );
}

LIB_EXPORT rc_t CC CacheTee3FileIsComplete ( struct KFile const * self, bool * is_complete )
{
    rc_t rc = 0;
    if ( self == NULL || is_complete == NULL )
        rc = RC ( rcFS, rcFile, rcValidating, rcParam, rcNull );
    else
    {
        if ( &( self -> vt -> v1 ) != &KCacheTeeFile_v3_vt )
            *is_complete = is_bitmap_complete_file( self );
        else
            *is_complete = is_bitmap_complete_v3( ( struct KCacheTeeFile_v3 * )self );
    }
    return rc;
}

LIB_EXPORT bool CC KFileIsKCacheTeeFile_v3( const struct KFile * self )
{
    bool res = false;
    if ( self != NULL )
        res = ( &( self -> vt -> v1 ) == &KCacheTeeFile_v3_vt );
    return res;
}

LIB_EXPORT rc_t CC CacheTee3FileGetOriginalSize ( struct KFile const * self, uint64_t * original_size )
{
    rc_t rc = 0;
    if ( self == NULL || original_size == NULL )
        rc = RC ( rcFS, rcFile, rcValidating, rcParam, rcNull );
    else
    {
        if ( &( self -> vt -> v1 ) != &KCacheTeeFile_v3_vt )
        {
            uint64_t over_all_size;
            KCacheTeeFileTail tail;
            rc = extract_tail( self, &over_all_size, &tail );
            if ( rc == 0 )
                *original_size = tail . orig_size;
        }
        else
        {
            KCacheTeeFile_v3 * ctf = ( struct KCacheTeeFile_v3 * )self;
            *original_size = ctf -> source_size;
        }
    }
    return rc;
}

static size_t bitmap_completeness( volatile bmap_t * bitmap, size_t bitmap_size_in_bytes )
{
    size_t res = 0;
    size_t bitmap_words = bitmap_size_in_bytes / ( BMWORDSIZE >> 3 );
    size_t idx;
    /* all but the last word in the bitmap has to be set to 0xFFFFFFFFU */
    for ( idx = 0; idx < bitmap_words; ++idx )
    {
#if BMWORDSIZE == 32
        bmap_t w = bitmap[ idx ];
        if ( w > 0 )
        {
            if ( w == 0xFFFFFFFFU )
                res += BMWORDSIZE;
            else
                res += count_set_bits( w );
        }
#else
    #error "this code does not support this word size"
#endif
    }
    return res;
}

static rc_t get_bitmap_completeness_file( struct KFile const * self, double * percent, uint64_t * bytes_in_cache )
{
    uint64_t over_all_size;
    KCacheTeeFileTail tail;
    rc_t rc = extract_tail( self, &over_all_size, &tail );
    if ( rc == 0 )
    {
        size_t bitmap_size_in_bytes = calculate_bitmap_size_in_bytes( &tail );
        if ( tail . orig_size + bitmap_size_in_bytes + sizeof tail != over_all_size )
        {
            rc = RC ( rcFS, rcFile, rcValidating, rcParam, rcInvalid );
            PLOGERR ( klogSys, ( klogSys, rc, "$(func) - invalid internal size-values in cachefile"
                             , "func=%s"
                             , __func__
                      ) );
        }
        else
        {
            volatile bmap_t * bitmap = extract_bitmap( self, tail . orig_size, bitmap_size_in_bytes );
            if ( bitmap != NULL )
            {
                size_t pages_in_cache = bitmap_completeness( bitmap, bitmap_size_in_bytes );
                * bytes_in_cache = ( pages_in_cache * tail . page_size );
                free( ( void * ) bitmap );
                if ( * bytes_in_cache > 0 && tail . orig_size > 0 )
                {
                    double x = ( float ) * bytes_in_cache;
                    x *= 100;
                    x /= tail . orig_size;
                    *percent = x;
                }
            }
        }
    }
    return rc;
}

static void get_bitmap_completeness_v3( const KCacheTeeFile_v3 * self, double * percent, uint64_t * bytes_in_cache )
{
    if ( self -> cache_file == NULL )
        return;
    else
    {
        size_t pages_in_cache = bitmap_completeness( self -> bitmap, self -> bmap_size );
        * bytes_in_cache = ( pages_in_cache * self -> page_size );
        if ( * bytes_in_cache > 0 && self -> tail -> orig_size > 0 )
        {
            double x = ( float ) * bytes_in_cache;
            x *= 100;
            x /= self -> tail -> orig_size;
            *percent = x;
        }
    }
}

LIB_EXPORT rc_t CC CacheTee3FileGetCompleteness ( struct KFile const * self,
                    double * percent, uint64_t * bytes_in_cache )
{
    rc_t rc = 0;
    if ( self == NULL || percent == NULL || bytes_in_cache == NULL )
        rc = RC ( rcFS, rcFile, rcValidating, rcParam, rcNull );
    else
    {
        *percent = 0.0;
        *bytes_in_cache = 0;
        if ( &( self -> vt -> v1 ) != &KCacheTeeFile_v3_vt )
            rc = get_bitmap_completeness_file( self, percent, bytes_in_cache );
        else
            get_bitmap_completeness_v3( ( struct KCacheTeeFile_v3 * )self, percent, bytes_in_cache );
    }
    return rc;
}

/* --- perform promotion --- */

/* if the given KFile is not of type KCacheTeeFile_v3, we truncate the file to the size of the source
   but we cannot rename it, because KFile does not store its path */
static rc_t finalize_file ( struct KFile * self )
{
    uint64_t over_all_size;
    KCacheTeeFileTail tail;
    rc_t rc = extract_tail( self, &over_all_size, &tail );
    if ( rc == 0 )
    {
        rc = KFileSetSize_v1 ( self, tail . orig_size );
        if ( rc != 0 )
        {
            PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to truncate the cache-file"
                             , "func=%s"
                             , __func__
                      ) );
        }

    }
    return rc;
}

/* if the given KFile is of type KCacheTeeFile_v3, we truncate the file to the size of the source
   and rename it to the path stored in the struct */
static rc_t finalize_v3 ( KCacheTeeFile_v3 * self )
{
    rc_t rc = 0;
    if ( self -> cache_file != NULL )
    {
        rc = KFileSetSize_v1 ( self -> cache_file, self -> source_size );
        if ( rc != 0 )
        {
            PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to truncate the cache-file"
                             , "func=%s"
                             , __func__
                      ) );
        }
        else
        {
            // we have to release the cache-file to rename it...
            rc = KFileRelease ( self -> cache_file );
            if ( rc != 0 )
            {
                PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to release the cache-file"
                                 , "func=%s"
                                 , __func__
                          ) );
            }
            else
            {
                char buffer[ 4096 ];
                size_t num_writ;
                self -> cache_file = NULL;
                
                rc = string_printf ( buffer, sizeof buffer, &num_writ, "%s.cache", self -> path );
                if ( rc != 0 )
                {
                    PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed create the cache-file path"
                                     , "func=%s"
                                     , __func__
                              ) );
                }
                else
                {
                    rc = KDirectoryRename ( self -> dir, true, buffer, self -> path );
                    if ( rc != 0 )
                    {
                        PLOGERR ( klogSys, ( klogSys, rc, "$(func) - failed to rename the cache-file"
                                         , "func=%s"
                                         , __func__
                                  ) );
                    }
                }
            }
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC CacheTee3FileFinalize ( struct KFile * self )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC ( rcFS, rcFile, rcValidating, rcParam, rcNull );
    else
    {
        bool is_complete;
        rc = CacheTee3FileIsComplete ( self, &is_complete );
        if ( rc == 0 )
        {
            if ( !is_complete )
                rc = RC ( rcFS, rcFile, rcValidating, rcParam, rcIncomplete );
            else
            {
                if ( &( self -> vt -> v1 ) != &KCacheTeeFile_v3_vt )
                    rc = finalize_file( self );
                else
                    rc = finalize_v3( ( struct KCacheTeeFile_v3 * )self );
            }
        }
    }
    return rc;
}
