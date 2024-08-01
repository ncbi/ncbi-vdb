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

#include <klib/refcount.h>

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
struct KDatabase;
struct KTable;
struct KColumn;
struct VFSManager;

/*--------------------------------------------------------------------------
 * KDBManagerBase
 *   base structure for KDBManager implementations
 */
typedef struct KDBManagerBase KDBManagerBase;

typedef struct KDBManager_vt KDBManager_vt;
struct KDBManager_vt
{
    /* Public API */
    rc_t ( CC * whack )                 ( KDBMGR_IMPL * self );
    rc_t ( CC * addRef )                ( const KDBMGR_IMPL * self );
    rc_t ( CC * release )               ( const KDBMGR_IMPL * self );
    rc_t ( CC * version )               ( const KDBMGR_IMPL * self, uint32_t *version );
    bool ( CC * vExists )               ( const KDBMGR_IMPL * self, uint32_t type, const char *name, va_list args );
    rc_t ( CC * vWritable )             ( const KDBMGR_IMPL * self, const char *name, va_list args );
    rc_t ( CC * runPeriodicTasks )      ( const KDBMGR_IMPL * self );
    int ( CC * pathTypeVP )             ( const KDBMGR_IMPL * self, const struct VPath * path );
    int ( CC * vPathType )              ( const KDBMGR_IMPL * self, const char *path, va_list args );
    int ( CC * vPathTypeUnreliable )    ( const KDBMGR_IMPL * self, const char *path, va_list args );
    rc_t ( CC * vOpenDBRead )           ( const KDBMGR_IMPL * self,
        const struct KDatabase **db, const char *path, va_list args,
        const struct VPath *vpath );
    rc_t ( CC * vOpenTableRead )        ( const KDBMGR_IMPL * self, const struct KTable **tbl, const char *path, va_list args );
    rc_t ( CC * openTableReadVPath )    ( struct KDBMGR_IMPL const * self, const struct KTable **tbl, const struct VPath *path );
    rc_t ( CC * vOpenColumnRead )       ( struct KDBMGR_IMPL const * self, const struct KColumn **col, const char *path, va_list args );
    rc_t ( CC * vPathOpenLocalDBRead )  ( struct KDBMGR_IMPL const * self, struct KDatabase const ** p_db, struct VPath const * vpath );
    rc_t ( CC * vPathOpenRemoteDBRead ) ( struct KDBMGR_IMPL const * self, struct KDatabase const ** p_db, struct VPath const * remote, struct VPath const * cache );
    rc_t ( CC * getVFSManager )         ( struct KDBMGR_IMPL const * self, const struct VFSManager ** vfs );
};

struct KDBManagerBase
{
    const KDBManager_vt * vt;

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

#ifdef __cplusplus
}
#endif

