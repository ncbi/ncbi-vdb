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

struct CacheTeeWMCleanupTask;
#define KTASK_IMPL struct CacheTeeWMCleanupTask
#include <kproc/impl.h>

#include <kfs/directory.h>
#include <klib/rc.h>
#include <klib/log.h>
#include <klib/text.h>
#include <kproc/procmgr.h>

typedef struct CacheTeeWMCleanupTask
{
    KTask dad;
    const char * cache_file;
    KTaskTicket ticket;
} CacheTeeWMCleanupTask;


static rc_t CacheTeeWMCleanupTask_Destroy( CacheTeeWMCleanupTask * self )
{
    if ( self -> cache_file != NULL )
        free( ( void* ) self -> cache_file );
    free( self );
    return 0;
}

static rc_t CacheTeeWMCleanupTask_Execute( CacheTeeWMCleanupTask * self )
{
    KDirectory * dir;
    rc_t rc = KDirectoryNativeDir( &dir );
    if ( rc != 0 )
    {
        LOGERR( klogErr, rc, "cannot create native directory for clenaup" );
    }
    else
    {
        rc = KDirectoryRemove ( dir, true, "%s", self -> cache_file );
        if ( rc != 0 )
        {
            PLOGERR( klogErr, ( klogErr, rc, "cannot remove cache file '$(path)'", 
                    "path=%s", self -> cache_file ) );
        }
        KDirectoryRelease( dir );
    }
    return rc;
}

static KTask_vt_v1 vtCacheTeeWMCleanupTask =
{
    /* version 1.0 */
    1, 0,

    /* start minor version 0 methods */
    CacheTeeWMCleanupTask_Destroy,
    CacheTeeWMCleanupTask_Execute
    /* end minor version 0 methods */
};

rc_t Make_CacheTeeWMCleanup_Task ( struct CacheTeeWMCleanupTask **task, const char * cache_file )
{
    rc_t rc = 0;
    CacheTeeWMCleanupTask * t = malloc ( sizeof * t );
    if ( t == NULL )
    {
        rc = RC ( rcPS, rcMgr, rcInitializing, rcMemory, rcExhausted );
        LOGERR( klogErr, rc, "cannot create cleanup-task" );
    }
    else
    {
        t -> cache_file = string_dup ( cache_file, string_size( cache_file ) );
        if ( t -> cache_file == NULL )
        {
            rc = RC ( rcPS, rcMgr, rcInitializing, rcMemory, rcExhausted );
            LOGERR( klogErr, rc, "cannot create cleanup-task" );
        }
        else
        {
            rc = KTaskInit ( &t -> dad,
                            (const union KTask_vt *)&vtCacheTeeWMCleanupTask,
                            "CacheTeeWMCleanupTask",
                            "CacheTeeWMCleanupTask" );
            if ( rc != 0 )
            {
                LOGERR( klogErr, rc, "cannot initialize cleanup-task" );
            }
            else
            {
                struct KProcMgr * proc_mgr;
                rc = KProcMgrMakeSingleton ( &proc_mgr );
                if ( rc != 0 )
                {
                    LOGERR( klogErr, rc, "cannot access process-manager" );
                }
                else
                {
                    KTask * kt = &t -> dad;
                    rc = KProcMgrAddCleanupTask ( proc_mgr, &( t -> ticket ), kt );
                    if ( rc != 0 )
                    {
                        LOGERR( klogErr, rc, "cannot add cleanup-task" );
                    }
                    else
                    {
                        *task = ( CacheTeeWMCleanupTask * ) kt;
                    }
                    KProcMgrRelease ( proc_mgr );
                }
            }
        }
        
        if ( rc != 0 )
        {
            KTaskRelease ( ( KTask * ) t );
        }
    }
    return rc;
}

rc_t Execute_and_Release_CacheTeeWMCleanup_Task ( struct CacheTeeWMCleanupTask * self )
{
    rc_t rc = 0;
    if ( self == NULL )
    {
        rc = RC ( rcPS, rcMgr, rcInitializing, rcParam, rcInvalid );
        LOGERR( klogErr, rc, "cannot release null-ptr-task" );
    }
    else
    {
        struct KProcMgr * proc_mgr;
        rc = KProcMgrMakeSingleton ( &proc_mgr );
        if ( rc != 0 )
        {
            LOGERR( klogErr, rc, "cannot access process-manager" );
        }
        else
        {
            rc = KProcMgrRemoveCleanupTask ( proc_mgr, &( self -> ticket ) );
            if ( rc == 0 )
            {
                LOGERR( klogErr, rc, "cannot remove cleanup-task" );
            }
            else
            {
                rc = KTaskExecute ( ( KTask * )self );
                if ( rc != 0 )
                {
                    LOGERR( klogErr, rc, "cannot execute cleanup-task" );
                }
            }
            KProcMgrRelease ( proc_mgr );
        }
        KTaskRelease ( ( KTask * )self );
    }
    return rc;
}
