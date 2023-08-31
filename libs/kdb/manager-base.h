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

#pragma once

#ifndef _h_klib_refcount_
#include <klib/refcount.h>
#endif

#ifndef KDBMGR_IMPL
#define KDBMGR_IMPL KDBManagerBase
#endif

#include <kfs/directory.h>

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
struct VPath;

/*--------------------------------------------------------------------------
 * KDBManagerBase
 *   base structure for KDBManager implementations
 */
typedef struct KDBManagerBase KDBManagerBase;

typedef struct KDBManager_vt KDBManager_vt;
struct KDBManager_vt
{
    /* Public API */
    rc_t ( CC * whack )                 ( KDBMGR_IMPL *self );
    rc_t ( CC * addRef )                ( const KDBMGR_IMPL *self );
    rc_t ( CC * release )               ( const KDBMGR_IMPL *self );
    rc_t ( CC * version )               ( const KDBMGR_IMPL *self, uint32_t *version );
    bool ( CC * vExists )               ( const KDBMGR_IMPL *self, uint32_t type, const char *name, va_list args );
    rc_t ( CC * vWritable )             ( const KDBMGR_IMPL *self, const char *name, va_list args );
    rc_t ( CC * runPeriodicTasks )      ( const KDBMGR_IMPL *self );
    int ( CC * pathTypeVP )             ( const KDBMGR_IMPL * self, const struct VPath * path );
    int ( CC * vPathType )              ( const KDBMGR_IMPL * self, const char *path, va_list args );
    int ( CC * vPathTypeUnreliable )    ( const KDBManager * self, const char *path, va_list args );
// LIB_EXPORT rc_t CC KDBManagerOpenDBRead ( const KDBManager *self,
// LIB_EXPORT rc_t CC KDBManagerOpenTableRead ( const KDBManager *self,
// LIB_EXPORT rc_t CC KDBManagerOpenTableReadVPath ( const KDBManager *self,
// LIB_EXPORT rc_t CC KDBManagerOpenColumnRead ( const KDBManager *self,
// LIB_EXPORT rc_t CC KDBManagerVPathOpenLocalDBRead ( struct KDBManager const * self, struct KDatabase const ** p_db, struct VPath const * vpath )
// LIB_EXPORT rc_t CC KDBManagerVPathOpenRemoteDBRead ( struct KDBManager const * self, struct KDatabase const ** p_db, struct VPath const * remote, struct VPath const * cache )

};

// Public write side-only API

// KDB_EXTERN rc_t CC KDBManagerLock ( KDBManager *self, const char *path, ... );
// KDB_EXTERN rc_t CC KDBManagerVLock ( KDBManager *self, const char *path, va_list args );

// KDB_EXTERN rc_t CC KDBManagerUnlock ( KDBManager *self, const char *path, ... );
// KDB_EXTERN rc_t CC KDBManagerVUnlock ( KDBManager *self, const char *path, va_list args );

// KDB_EXTERN rc_t CC KDBManagerDrop ( KDBManager *self, uint32_t obj_type, const char *path, ... );
// KDB_EXTERN rc_t CC KDBManagerVDrop ( KDBManager *self, uint32_t obj_type, const char *path, va_list args );

struct KDBManagerBase
{
    KDBManager_vt * vt;

    KRefcount refcount;
};

// default implelentations where exist
extern rc_t CC KDBManagerBaseWhack ( KDBMGR_IMPL *self );
extern rc_t CC KDBManagerBaseAddRef ( const KDBMGR_IMPL *self );
extern rc_t CC KDBManagerBaseRelease ( const KDBMGR_IMPL *self );

/* Attach
 * Sever
 *  like AddRef/Release, except called internally
 *  indicates that a child object is letting go...
 */
KDBMGR_IMPL *KDBManagerAttach ( const KDBMGR_IMPL *self );
rc_t KDBManagerSever ( const KDBMGR_IMPL *self );

// write side only public API
// ...

#ifdef __cplusplus
}
#endif

