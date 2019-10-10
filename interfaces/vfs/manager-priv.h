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
#ifndef _h_vfs_manager_priv_
#define _h_vfs_manager_priv_

#ifndef _h_vfs_extern_
#include <vfs/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif


#ifndef _h_kfs_defs_
#include <kfs/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct VFSManager;
struct KDirectory;
struct KFile;
struct VPath;
struct VResolver;
struct SRAPath;
struct KConfig;

/* this resembles the interface functions in manager.h
 * but allows the use of a KDirectory for the base instead of a VPath
 * of a directory
 *
 * this is expected to be more temporary if this code base continues.
 * much longer
 */
VFS_EXTERN rc_t CC VFSManagerResolvePathRelativeDir (const struct VFSManager * self,
                                                     uint32_t flags,
                                                     const struct  KDirectory * base_dir,
                                                     const struct  VPath * in_path,
                                                     struct VPath ** out_path);



/* bad interface.  Bad! Bad!
 * but needed to hack VFS into KDB
 * MORE THAN BAD. REFLECTS A TOTAL MISUNDERSTANDING OF FS.
 */
VFS_EXTERN rc_t CC VFSManagerOpenFileReadDirectoryRelative (const struct VFSManager *self, 
    const struct KDirectory * dir, struct KFile const **f, const struct VPath * path);

VFS_EXTERN rc_t CC VFSManagerOpenFileReadDirectoryRelativeDecrypt (const struct VFSManager *self, 
    const struct KDirectory * dir, struct KFile const **f, const struct VPath * path);

VFS_EXTERN rc_t CC VFSManagerOpenDirectoryReadDirectoryRelative ( const struct VFSManager *self,
    struct KDirectory const * dir, struct KDirectory const **d, const struct VPath * path );

VFS_EXTERN rc_t CC VFSManagerOpenDirectoryReadDirectoryRelativeDecrypt ( const struct VFSManager *self,
    struct KDirectory const * dir, struct KDirectory const **d, const struct VPath * path );

VFS_EXTERN rc_t CC VFSManagerOpenDirectoryUpdateDirectoryRelative ( const struct VFSManager *self,
    struct KDirectory const * dir, struct KDirectory **d, const struct VPath * path );

#define VPathMakeDirectoryRelative LegacyVPathMakeDirectoryRelative
VFS_EXTERN rc_t CC VPathMakeDirectoryRelative ( struct VPath ** new_path,
    struct KDirectory const * dir, const char * posix_path);
VFS_EXTERN rc_t CC VPathMakeDirectoryRelativeVPath ( struct VPath ** new_path,
    struct KDirectory const * dir, const char * posix_path, const struct VPath * vpath );


VFS_EXTERN rc_t CC VFSManagerOpenFileReadDecrypt (const struct VFSManager *self,
                                                  struct KFile const **f,
                                                  const struct VPath * path);


/*
 * quick hack for vdb-validate for immediate testing release
 */
VFS_EXTERN rc_t CC VFSManagerWGAValidateHack (const struct VFSManager * self, 
                                              const struct KFile * file,
                                              const char * path); /* we'll move this to a vpath */

/*
 * Access to VFSManager's instance of configuration object, for testing purposes
 */
VFS_EXTERN const struct KConfig* CC VFSManagerGetConfig(const struct VFSManager * self);

/* Make using a custom KConfig
 */
VFS_EXTERN rc_t CC VFSManagerMakeFromKfg ( struct VFSManager ** pmanager,
    struct KConfig * cfg );

/* Make non-singleton version
 */
VFS_EXTERN rc_t CC VFSManagerMakeLocal ( struct VFSManager ** pmanager,
    struct KConfig * cfg );

/*
 * Set/get pathname of the object_id/object_name bindings file (used for testing; should go away when the functionality
 *  is rolled into KeyRing server)
 * Set to NULL to use the default location ("~/.ncbi/objid.mapping")
 */
VFS_EXTERN void VFSManagerSetBindingsFile(struct VFSManager * self, const char* path);
VFS_EXTERN const char* VFSManagerGetBindingsFile(struct VFSManager * self);


/** Reset VResolver when we need to change protected repository context */
VFS_EXTERN rc_t CC VFSManagerSetResolver
    ( struct VFSManager * self, struct VResolver * resolver );


/* GetCacheRoot
 *  query the cache-root
 *
 *  path [ OUT ] - VPath of location of current cache-root
 */
VFS_EXTERN rc_t CC VFSManagerGetCacheRoot ( const struct VFSManager * self,
    struct VPath const ** path );


/* SetCacheRoot
 *  set a new cache-root
 *
 *  path [ IN ] - VPath of new location of cache-root
 */

VFS_EXTERN rc_t CC VFSManagerSetCacheRoot ( const struct VFSManager * self,
    struct VPath const * path );
    
/* DeleteCacheOlderThan
 *  delete old cached files
 *
 *  days [ IN ] - files older than this will be deleted
 *  
 */
VFS_EXTERN rc_t CC VFSManagerDeleteCacheOlderThan ( const struct VFSManager * self,
    uint32_t days );


/** Unreliable object: do not report occured erros */
VFS_EXTERN rc_t CC VFSManagerOpenDirectoryReadDecryptUnreliable (
    const struct VFSManager *self, struct KDirectory const **d,
    const struct VPath * object );


#ifdef __cplusplus
}
#endif

#endif /* _h_vfs_manager_priv_ */
