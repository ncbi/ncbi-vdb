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
#include <kfs/lockfile.h>
#include <kfs/impl.h>
#include <kfs/file.h>
#include <kfs/directory.h>
#include <kfs/remove-file-task.h>
#include <kproc/task.h>
#include <kproc/impl.h>
#include <kproc/procmgr.h>
#include <klib/text.h>
#include <klib/refcount.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#if _DEBUGGING
#define CRIPPLE_CLEANUP 0
#define DISABLE_CLEANUP 0
#endif

/*--------------------------------------------------------------------------
 * KRemoveLockFileTask
 */
static
rc_t KRemoveLockFileTaskMake ( KTask **task, KDirectory *dir, const char *path )
{
    return KRemoveFileTaskMake ( task, dir, "%s", path );
}


/*--------------------------------------------------------------------------
 * KLockFile
 *  a mutual exclusion lock on a file
 *  the lock is acquired upon creation
 *  and released upon destruction
 *
 * NB - only guaranteed to work when used from a single host
 */
struct KLockFile
{
    KProcMgr *pmgr;
    KTask *cleanup;
    KTaskTicket ticket;
    KRefcount refcount;
};

static
rc_t KLockFileWhack ( KLockFile *self )
{
    rc_t rc = 0;

    /* remove task from cleanup queue */
    if ( self -> pmgr != NULL )
    {
        rc = KProcMgrRemoveCleanupTask ( self -> pmgr, & self -> ticket );
        KProcMgrRelease ( self -> pmgr );
    }

#if ! CRIPPLE_CLEANUP && ! DISABLE_CLEANUP
    /* remove the lock file from file system */
    rc = KTaskExecute ( self -> cleanup );
#endif
    KTaskRelease ( self -> cleanup );

    free ( self );
    return rc;
}


/* AddRef
 */
LIB_EXPORT rc_t CC KLockFileAddRef ( const KLockFile *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KLockFile" ) )
        {
        case krefLimit:
            return RC ( rcFS, rcLock, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

/* Release
 */
LIB_EXPORT rc_t CC KLockFileRelease ( const KLockFile *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KLockFile" ) )
        {
        case krefWhack:
            return KLockFileWhack ( ( KLockFile* ) self );
        case krefNegative:
            return RC ( rcFS, rcLock, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

/* Make
 */
static
rc_t KLockFileMake ( KLockFile **lock, KDirectory *dir, const char *path )
{
    rc_t rc;
    KLockFile *f = malloc ( sizeof * f );
    if ( f == NULL )
        rc = RC ( rcFS, rcLock, rcConstructing, rcMemory, rcExhausted );
    else
    {
        rc = KRemoveLockFileTaskMake ( & f -> cleanup, dir, path );
        if ( rc == 0 )
        {
#if ! DISABLE_CLEANUP
            /* register cleanup task with proc mgr */
            rc = KProcMgrMakeSingleton ( & f -> pmgr );
            if ( rc == 0 )
                rc = KProcMgrAddCleanupTask ( f -> pmgr, & f -> ticket, f -> cleanup );
            else
#else
            f -> pmgr = NULL;
#endif
            {
                /* this is allowed to fail if mgr has not been initialized */
                memset ( & f -> ticket, 0, sizeof f -> ticket );
                rc = 0;
            }

            if ( rc == 0 )
            {
                KRefcountInit ( & f -> refcount, 1, "KLockFile", "make", path );
                * lock = f;
                return 0;
            }

            if ( f -> pmgr != NULL )
                KProcMgrRelease ( f -> pmgr );
        }

        free ( f );
    }

    return rc;
}


/*--------------------------------------------------------------------------
 * KDirectory
 *  interface extensions
 */


/* CreateLockFile
 *  attempts to create a KLockFile
 *
 *  "lock" [ OUT ] - return parameter for newly created lock file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting lock file
 */
LIB_EXPORT rc_t CC KDirectoryVCreateLockFile ( KDirectory *self,
    KLockFile **lock, const char *path, va_list args )
{
    rc_t rc;

    if ( lock == NULL )
        rc = RC ( rcFS, rcFile, rcLocking, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcFS, rcFile, rcLocking, rcSelf, rcNull );
        else if ( path == NULL )
            rc = RC ( rcFS, rcFile, rcLocking, rcPath, rcNull );
        else if ( path [ 0 ] == 0 )
            rc = RC ( rcFS, rcFile, rcLocking, rcPath, rcEmpty );
        else
        {
            char full [ 4096 ];
            rc = KDirectoryVResolvePath ( self, true, full, sizeof full, path, args );
            if ( rc == 0 )
            {
                KFile *lock_file;
                rc = KDirectoryCreateFile ( self, & lock_file, false, 0600, kcmCreate | kcmParents, "%s", full );
                if ( rc == 0 )
                {
                    rc_t rc2;

                    /* no longer need the file - not going to write to it anyway */
                    KFileRelease ( lock_file );

                    /* we have the lock */
                    rc = KLockFileMake ( lock, self, full );
                    if ( rc == 0 )
                        return 0;

                    /* must unlink lockfile */
                    rc2 = KDirectoryRemove ( self, true, "%s", full );
                    if ( rc2 != 0 )
                        /* issue a report */;
                }
                else if ( GetRCState ( rc ) == rcExists )
                {
                    /* map the rc to kproc type values */
                    rc = RC ( rcFS, rcFile, rcLocking, rcLocking, rcBusy );
                }
                else
                {
                    rc = ResetRCContext ( rc, rcFS, rcFile, rcLocking );
                }
            }
        }

        * lock = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryCreateLockFile ( KDirectory *self,
    KLockFile **lock, const char *path, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, path );

    rc = KDirectoryVCreateLockFile ( self, lock, path, args );

    va_end ( args );

    return rc;
}


/* CreateExclusiveAccessFile
 *  opens a file with exclusive write access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "update" [ IN ] - if true, open in read/write mode
 *  otherwise, open in write-only mode
 *
 *  "access" [ IN ] - standard Unix access mode, e.g. 0664
 *
 *  "mode" [ IN ] - a creation mode ( see explanation above ).
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */

LIB_EXPORT rc_t CC KDirectoryCreateExclusiveAccessFile ( KDirectory *self, KFile **f,
    bool update, uint32_t access, KCreateMode mode, const char *path, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, path );

    rc = KDirectoryVCreateExclusiveAccessFile ( self, f, update, access, mode, path, args );

    va_end ( args );

    return rc;
}
