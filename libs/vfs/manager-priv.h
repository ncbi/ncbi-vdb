#ifndef _h_libs_vfs_manager_
#define _h_libs_vfs_manager_

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
* =========================================================================== */


#include <klib/text.h> /* String */
#include <klib/container.h> /* BSTreeInit */
#include <klib/refcount.h>
#include <vfs/resolver.h>

#ifdef __cplusplus
extern "C" {
#endif

struct VFSManager;
struct KConfig;
struct KCipherManager;
struct KLock;

/* Get ".noqual" extension.
    *   accepts VFSManager* == NULL */
const String * VFSManagerExtNoqual   (const struct VFSManager *);
/* Get old ".noqual" extension. */
const String * VFSManagerExtNoqualOld(const struct VFSManager *);
/* Get ".sra" extension. */
const String * VFSManagerExtSra      (const struct VFSManager *);

rc_t VFSManagerCheckRunDir(const KDirectory* dir, const struct VPath* path);

/******************************************************************************/
/* Cache of names resolve results / SDL responses */
typedef enum {
    eSCSEmpty,
    eSCSCachedWhenNull,
    eSCSCachedWhenNotNull,
    eSCSFound,
} ESdlCacheState;
uint32_t VFSManagerSdlCacheCount
(const struct VFSManager * self, ESdlCacheState * state);
rc_t VFSManagerSdlCacheClear(struct VFSManager * self);
/******************************************************************************/

/*--------------------------------------------------------------------------
 * VFSManager
 */

/* currently expected to be a singleton and not use a vtable but
 * be fully fleshed out here */
struct VFSManager
{
    /* the current directory in the eyes of the O/S when created */
    KDirectory * cwd;

    /* configuration manager */
    struct KConfig * cfg;

    /* krypto's cipher manager */
    struct KCipherManager * cipher;

    /* SRAPath will be replaced with a VResolver */
    struct VResolver * resolver;

    /* network manager */
    struct KNSManager * kns;

    /**************************************************************************/
    /* Cache of names resolve results / SDL responses */
    bool notCachingSdlResponse;
    ESdlCacheState trSdlState;
    BSTree trSdl;
    struct KLock * trSdlMutex;
    /**************************************************************************/

    /* path to a global password file */
    char *pw_env;

    /* encryption key storage */
    struct KKeyStore* keystore;

    KRefcount refcount;

    VRemoteProtocols protocols;
};

#ifdef __cplusplus
}
#endif


#endif /* _h_libs_vfs_manager_ */
