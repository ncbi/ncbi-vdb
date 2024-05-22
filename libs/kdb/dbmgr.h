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

#include <kdb/manager.h>

#include <klib/container.h>
#include <klib/refcount.h>

#define KDBMGR_IMPL KDBManager
#include "manager-base.h"

#ifndef KONST
#define KONST
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KRWLock;
struct KSymbol;
struct KDirectory;
struct VFSManager;
struct KIndex;

/*--------------------------------------------------------------------------
 * KDBManager
 *  handle to library
 */
struct KDBManager
{
    KDBManagerBase dad;

    /* root directory */
    struct KDirectory KONST *wd;

    /* open objects */
    struct KRWLock *open_objs_lock;
    BSTree open_objs;

    /* other managers needed by the KDB manager */
    struct VFSManager * vfsmgr;
};

rc_t KDBManagerWhack ( KDBManager *self );

/* Make - PRIVATE
 */
rc_t KDBManagerMake ( KDBManager **mgrp, struct KDirectory const *wd,
    const char *op, struct VFSManager *vmanager, KDBManager_vt * vt );

/* Attach
 * Sever
 */
KDBManager *KDBManagerAttach ( const KDBManager *self );
rc_t KDBManagerSever ( const KDBManager *self );

rc_t CC KDBManagerCommonVersion ( const KDBManager *self, uint32_t *version );
bool CC KDBManagerCommonVExists ( const KDBManager *self, uint32_t requested, const char *name, va_list args );

/* CheckOpen
 *  tests if object is open and returns an error code
 *  of 0 if not, or <target>, rcBusy if so, where
 *  <target> is generated from object type.
 */
rc_t KDBManagerCheckOpen ( const KDBManager *self, const char *path );

/* OpenObjectBusy
 *   Does this path reference an object already open?
 */
bool KDBManagerOpenObjectBusy ( const KDBManager *self, const char *path );

/* OpenObjectFind
 *   return a reference to an object with this "symbol" - path
 */
struct KSymbol *KDBManagerOpenObjectFind ( const KDBManager *self, const char *path );

/* OpenObjectAdd
 *   Put this symbol in the list of open objects
 */
rc_t KDBManagerOpenObjectAdd ( KDBManager *self, struct KSymbol *obj );

/* OpenObjectDelete
 *   remove this symbol in the list of open objects
 */
rc_t KDBManagerOpenObjectDelete ( KDBManager *self, struct KSymbol *obj );

rc_t CC KDBCmnManagerGetVFSManager ( const KDBManager *self, const struct VFSManager **vmanager );

#ifdef __cplusplus
}
#endif

