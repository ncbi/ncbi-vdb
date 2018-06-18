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

#ifndef _h_kproc_procmgr_
#define _h_kproc_procmgr_

#ifndef _h_kproc_extern_
#include <kproc/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
struct KTask;
struct KTaskTicket;

/*--------------------------------------------------------------------------
 * KProcMgr
 */
typedef struct KProcMgr KProcMgr;


/* Init
 *  initialize the proc mgr
 *  creates the singleton object
 */
KPROC_EXTERN rc_t CC KProcMgrInit ( void );

/* Whack
 *  tear down proc mgr
 *  runs any outstanding cleanup tasks
 *  deletes the singleton object
 *  intended to be called from an "atexit()" or similar task
 */
KPROC_EXTERN rc_t CC KProcMgrWhack ( void );


/* MakeSingleton
 *  access singleton process manager
 *  returns a new reference
 *  or an error code if not initialized
 *  VDB-2 ONLY
 */
KPROC_EXTERN rc_t CC KProcMgrMakeSingleton ( KProcMgr ** mgr );


/* AddRef
 * Release
 *  here to support refcounting API
 *  not generally useful in that the proc mgr
 *  will stay around until process exit
 */
KPROC_EXTERN rc_t CC KProcMgrAddRef ( const KProcMgr *self );
KPROC_EXTERN rc_t CC KProcMgrRelease ( const KProcMgr *self );


/* AddCleanupTask
 *  add a task to be performed at process exit time
 *
 *  "ticket" [ OUT ] - an id that can be used later to remove task
 *
 *  "task" [ IN ] - task object that will be executed at process
 *   exit, unless previously removed ( see RemoveCleanupTask ).
 *   NB - a new reference to "task" will be created
 */
KPROC_EXTERN rc_t CC KProcMgrAddCleanupTask ( KProcMgr *self,
    struct KTaskTicket *ticket, struct KTask *task );


/* RemoveCleanupTask
 *  remove a task from exit queue
 *  releases reference to task object
 *  returns an rcNotFound code if task no longer exists
 */
KPROC_EXTERN rc_t CC KProcMgrRemoveCleanupTask ( KProcMgr *self,
    struct KTaskTicket const *ticket );


/* OnMainThread
 *  returns true if running on main thread
 */
KPROC_EXTERN bool CC KProcMgrOnMainThread ( void );

/* GetPID
 *  returns the process-id
 */
KPROC_EXTERN rc_t CC KProcMgrGetPID ( const KProcMgr * self, uint32_t * pid );

/* GetHostName
 *  returns the name of the host-computer
 */
KPROC_EXTERN rc_t CC KProcMgrGetHostName ( const KProcMgr * self, char * buffer, size_t buffer_size );

#ifdef __cplusplus
}
#endif

#endif /* _h_kproc_procmgr_ */
