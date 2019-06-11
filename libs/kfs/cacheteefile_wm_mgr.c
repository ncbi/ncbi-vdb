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
struct CTWMCleanupTask;
#define KTASK_IMPL struct CTWMCleanupTask
#include <kproc/impl.h>

#include <klib/rc.h>
#include <klib/log.h>
#include <klib/text.h>
#include <klib/container.h>

#include <kproc/procmgr.h>
#include <kproc/lock.h>

#include <kfs/file.h>

/* ===== the entry ( into the lookup-tree of the singleton ) implementation ===== */
struct CTWM_entry
{
    BSTNode node;
    const char * name;    /* key */
    const struct KFile * ctwm;    /* payload */
};

static rc_t make_CTWM_entry( struct CTWM_entry ** entry,
                             const char * name,
                             const struct KFile * ctwm )
{
    rc_t rc = 0;
    struct CTWM_entry * e = malloc( sizeof *e );
    if ( e == NULL )
    {
        rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
        LOGERR( klogErr, rc, "cannot allocate CTWM_entry" );
    }
    else
    {
        e -> name = string_dup ( name, string_size( name ) );
        if ( e -> name == NULL )
        {
            rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
            LOGERR( klogErr, rc, "cannot allocate CTWM_entry.name" );
            free( ( void * ) e );
        }
        else
        {
            e -> ctwm = ctwm;
            *entry = e;
        }
    }
    return rc;
}

static void release_CTWM_entry( struct CTWM_entry * entry )
{
    if ( entry != NULL )
    {
        if ( entry -> name != NULL )
        {
            free( ( void * ) entry -> name );
            entry -> name = NULL;
        }
        if ( entry -> ctwm != NULL )
        {
            KFileRelease( entry -> ctwm );
            entry -> ctwm = NULL;
        }
        free( ( void * ) entry );
    }
}

static int64_t cmp_name_cb1( const void * item, const BSTNode * n )
{
    const char * a_name = item;
    const struct CTWM_entry * e = ( const struct CTWM_entry * )n;
    return string_cmp ( a_name, string_size( a_name ),
                        e -> name, string_size( e -> name ), 1024 );
}

static struct CTWM_entry * find_CTWM_entry( const BSTree * tree, const char * name )
{
    return ( struct CTWM_entry * )BSTreeFind ( tree, name, cmp_name_cb1 );
}

static int64_t cmp_name_cb2( const BSTNode *item, const BSTNode *n )
{
    return cmp_name_cb1( ( ( struct CTWM_entry * ) item ) -> name, n );
}
    
static rc_t enter_CTWM_entry( BSTree * tree, struct CTWM_entry * e )
{
    return BSTreeInsert ( tree, ( BSTNode * ) e, cmp_name_cb2 );
}

static void clear_node_cb( BSTNode *n, void *data )
{
    release_CTWM_entry( ( struct CTWM_entry * ) n );
}

static void clear_CTWM_tree( BSTree * tree )
{
    BSTreeWhack ( tree, clear_node_cb, NULL );
}

/* -----------------------------------------------------------------------------
the reason for the CacheTeeWaterMark-Manager:
    * keep a per process central point to register KCacheTeeFileWM - instances
    * it owns a tree of instances
    * keyed by the basename of the KCacheTeeFileWM - url's
----------------------------------------------------------------------------- */

/* ===== the manager implementation ===== */
struct CTWMManager
{
    BSTree instances;
    KLock * mgr_lock;
};

/* this is the hidden singleton, storing the cacheteefile_WM-instances as void pointers */
static struct CTWMManager * singleton = NULL;

rc_t Make_CTWMCleanup_Task ( void );

static rc_t create_CTWM( void )
{
    rc_t rc = 0;
    if ( singleton != NULL )
        return rc;
    struct CTWMManager * obj = malloc( sizeof *obj );
    if ( obj == NULL )
    {
        rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
        LOGERR( klogErr, rc, "cannot allocate CTWMManager" );
    }
    else
    {
        BSTreeInit( &obj -> instances );
        rc = KLockMake ( &obj -> mgr_lock );
        if ( rc != 0 )
        {
            LOGERR( klogErr, rc, "cannot create CTWMManger-lock" );
        }
        else
        {
            rc = Make_CTWMCleanup_Task ();
            if ( rc != 0 )
            {
                KLockRelease( obj -> mgr_lock );
            }
            else
            {
                singleton = obj;        
            }
        }
        if ( rc != 0 )
        {
            free( ( void * ) obj );
        }
    }
    
    return rc;
}

static void destroy_CTWM( void )
{
    if ( singleton != NULL )
    {
        clear_CTWM_tree( &singleton -> instances );
        KLockRelease( singleton -> mgr_lock );
        free( ( void * ) singleton );
        singleton = NULL;
    }
}

static const struct KFile * get_CTWM( const char * basename )
{
    const struct KFile * res = NULL;
    if ( singleton != NULL )
    {
        rc_t rc = KLockAcquire ( singleton -> mgr_lock );
        if ( rc == 0 )
        {
            struct CTWM_entry * e = find_CTWM_entry( &( singleton -> instances ), basename );
            if ( e != NULL )
                res = e -> ctwm;
            KLockUnlock ( singleton -> mgr_lock );
        }
    }
    return res;
}

static rc_t enter_CTWM( const struct KFile * ctwm, const char * basename )
{
    struct CTWM_entry * entry;
    rc_t rc = make_CTWM_entry( &entry, basename, ctwm );
    if ( rc == 0 )
    {
        rc = KLockAcquire ( singleton -> mgr_lock );
        if ( rc == 0 )
        {
            rc = enter_CTWM_entry( &( singleton -> instances ), entry );
            KLockUnlock ( singleton -> mgr_lock );            
        }

        if ( rc != 0 )
            release_CTWM_entry( entry );
    }
    return rc;
}

LIB_EXPORT rc_t CTWM_register( const struct KFile * ctwm, const char * basename )
{
    rc_t rc = 0;
    if ( ctwm == NULL )
        rc = RC ( rcFS, rcFile, rcAllocating, rcParam, rcNull );
    else if ( basename == NULL )
        rc = RC ( rcFS, rcFile, rcAllocating, rcPath, rcNull );
    else if ( basename [ 0 ] == 0 )
        rc = RC ( rcFS, rcFile, rcAllocating, rcPath, rcEmpty );

    if ( rc == 0 )
    {
        rc = create_CTWM();
        if ( rc == 0 )
        {
            const void * existing = get_CTWM( basename );
            if ( existing == NULL )
            {
                /* there is no entry with the given basename yet... */
                rc = enter_CTWM( ctwm, basename );
            }
            else
            {
                rc = RC ( rcFS, rcFile, rcAllocating, rcPath, rcDuplicate );
            }
        }
    }
    return rc;
}

LIB_EXPORT rc_t CTWM_get( const struct KFile ** ctwm, const char * basename )
{
    rc_t rc = 0;
    if ( ctwm == NULL )
        rc = RC ( rcFS, rcFile, rcAllocating, rcParam, rcNull );
    else
    {
        *ctwm = NULL;
        if ( basename == NULL )
            rc = RC ( rcFS, rcFile, rcAllocating, rcPath, rcNull );
        else if ( basename [ 0 ] == 0 )
            rc = RC ( rcFS, rcFile, rcAllocating, rcPath, rcEmpty );
    }
    
    rc = create_CTWM();
    if ( rc == 0 )
    {
        *ctwm = get_CTWM( basename );
        KFileAddRef( *ctwm );
    }
    return rc;
}

/* ===== the cleanup-task used to free the singleton ===== */

typedef struct CTWMCleanupTask
{
    KTask dad;
    KTaskTicket ticket;
} CTWMCleanupTask;

static rc_t CTWMCleanupTask_Destroy( CTWMCleanupTask * self )
{
    free( self );
    return 0;
}

static rc_t CTWMCleanupTask_Execute( CTWMCleanupTask * self )
{
    destroy_CTWM();     // cleans up the singleton
    return 0;
}

static KTask_vt_v1 vtCTWMCleanupTask =
{
    /* version 1.0 */
    1, 0,

    /* start minor version 0 methods */
    CTWMCleanupTask_Destroy,
    CTWMCleanupTask_Execute
    /* end minor version 0 methods */
};

rc_t Make_CTWMCleanup_Task ( void )
{
    rc_t rc = 0;
    CTWMCleanupTask * t = malloc ( sizeof * t );
    if ( t == NULL )
    {
        rc = RC ( rcPS, rcMgr, rcInitializing, rcMemory, rcExhausted );
        LOGERR( klogErr, rc, "cannot create cleanup-mgr-task" );
    }
    else
    {
        rc = KTaskInit ( &t -> dad,
                        (const union KTask_vt *)&vtCTWMCleanupTask,
                        "CTWMCleanupTask",
                        "CTWMCleanupTask" );
        if ( rc != 0 )
        {
            LOGERR( klogErr, rc, "cannot initialize cleanup-mgr-task" );
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
                    LOGERR( klogErr, rc, "cannot add cleanup-mgr-task" );
                }
                KProcMgrRelease ( proc_mgr );
            }
        }
        
        if ( rc != 0 )
        {
            KTaskRelease ( ( KTask * ) t );
        }
    }
    return rc;
}
