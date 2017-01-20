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

#ifndef _h_vdb_manager_
#define _h_vdb_manager_

#ifndef _h_vdb_extern_
#include <vdb/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KDirectory;
struct VPath;

/*--------------------------------------------------------------------------
 * VDBManager
 *  opaque handle to library
 */
typedef struct VDBManager VDBManager;


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
VDB_EXTERN rc_t CC VDBManagerAddRef ( const VDBManager *self );
VDB_EXTERN rc_t CC VDBManagerRelease ( const VDBManager *self );


/* MakeRead
 * MakeUpdate
 *  create library handle for specific use
 *  NB - only one of the functions will be implemented
 *
 *  "wd" [ IN, NULL OKAY ] - optional working directory for
 *  accessing the file system. mgr will attach its own reference.
 */
VDB_EXTERN rc_t CC VDBManagerMakeRead ( const VDBManager **mgr, struct KDirectory const *wd );
VDB_EXTERN rc_t CC VDBManagerMakeUpdate ( VDBManager **mgr, struct KDirectory *wd );


/* Version
 *  returns the library version
 */
VDB_EXTERN rc_t CC VDBManagerVersion ( const VDBManager *self, uint32_t *version );


/* Writable
 *  returns 0 if table is writable
 *  rcLocked if locked, rcReadonly if read-only
 *  other code upon error
 *
 *  "path" [ IN ] - NUL terminated path
 */
VDB_EXTERN rc_t CC VDBManagerWritable ( const VDBManager *self,
    const char *path, ... );
VDB_EXTERN rc_t CC VDBManagerVWritable ( const VDBManager *self,
    const char *path, va_list args );

/* Lock
 *  apply lock
 *
 *  if object is already locked, the operation is idempotent
 *  and returns an rc state of rcLocked
 *
 *  "path" [ IN ] - NUL terminated path
 */
VDB_EXTERN rc_t CC VDBManagerLock ( VDBManager *self, const char *path, ... );
VDB_EXTERN rc_t CC VDBManagerVLock ( VDBManager *self, const char *path, va_list args );

/* Unlock
 *  remove lock
 *
 *  if object is already unlocked, the operation is idempotent
 *  and returns an rc state of rcUnlocked
 *
 *  "path" [ IN ] - NUL terminated path
 */
VDB_EXTERN rc_t CC VDBManagerUnlock ( VDBManager *self, const char *path, ... );
VDB_EXTERN rc_t CC VDBManagerVUnlock ( VDBManager *self, const char *path, va_list args );


/* Drop
 *  drop an object based on its path
 *
 *  "path" [ IN ] - NUL terminated string in UTF-8 giving path to the vdb object
 */
VDB_EXTERN rc_t CC VDBManagerDrop ( VDBManager *self, uint32_t obj_type, const char *path, ... );
VDB_EXTERN rc_t CC VDBManagerVDrop ( VDBManager *self, uint32_t obj_type, const char *path, va_list args );


/* AddSchemaIncludePath
 *  add a path to schema for locating input files
 */
VDB_EXTERN rc_t CC VDBManagerAddSchemaIncludePath ( const VDBManager *self,
    const char *path, ... );
VDB_EXTERN rc_t CC VDBManagerVAddSchemaIncludePath ( const VDBManager *self,
    const char *path, va_list args );


/* AddLoadLibraryPath
 *  add a path to loader for locating dynamic libraries
 */
VDB_EXTERN rc_t CC VDBManagerAddLoadLibraryPath ( const VDBManager *self,
    const char *path, ... );
VDB_EXTERN rc_t CC VDBManagerVAddLoadLibraryPath ( const VDBManager *self,
    const char *path, va_list args );


/* GetUserData
 * SetUserData
 *  store/retrieve an opaque pointer to user data
 *
 *  "data" [ OUT ] - return parameter for getting data
 *  "data" [ IN ] - parameter for setting data
 *
 *  "destroy" [ IN, NULL OKAY ] - optional destructor param
 *  invoked from destructor of "self"
 */
VDB_EXTERN rc_t CC VDBManagerGetUserData ( const VDBManager *self, void **data );
VDB_EXTERN rc_t CC VDBManagerSetUserData ( const VDBManager *self,
    void *data, void ( CC * destroy ) ( void *data ) );


/* GetObjVersion
 *  returns the vdb-version of the given object
 *
 *  "version" [ OUT ] - discovered vdb-version
 *
 *  "path"    [ IN ] - path to the object to be examined ( database or table )
 */
VDB_EXTERN rc_t CC VDBManagerGetObjVersion ( const VDBManager *self,
    ver_t * version, const char *path );


/* GetObjModDate
 *  returns the load/modification timestamp of the given object
 *
 *  "ts" [ OUT ] - discovered modification timestamp written in object,
 *  or from filesystem
 *
 *  "path"    [ IN ] - path to the object to be examined ( database or table )
 */
VDB_EXTERN rc_t CC VDBManagerGetObjModDate ( const VDBManager *self,
    KTime_t * ts, const char *path );


/* PathType
 *  check the path type of an object/directory path.
 *
 *  this is an extension of the KDirectoryPathType ( see <kdb/manager.h> )
 *  and will return the KDirectory values if a path type is not specifically
 *  a VDB or KDB object.
 */
VDB_EXTERN int CC VDBManagerPathType ( const VDBManager * self,
    const char *path, ... );
VDB_EXTERN int CC VDBManagerVPathType ( const VDBManager * self,
    const char *path, va_list args );


/* GetCacheRoot
 *  query the cache-root
 *
 *  path [ OUT ] - VPath of location of current cache-root
 */
VDB_EXTERN rc_t CC VDBManagerGetCacheRoot ( const VDBManager * self,
    struct VPath const ** path );


/* SetCacheRoot
 *  set a new cache-root
 *
 *  path [ IN ] - VPath of new location of cache-root
 *
 *  The change to the cache-root is temporary, it is not automatically stored
 *  in the configuration-file. If the manager is released, the change will be
 *  forgotten. If the user wants to make the change permanent, he has to obtain
 *  a reference to the config-manager 'KConfig' and call KConfigCommit().
 */
VDB_EXTERN rc_t CC VDBManagerSetCacheRoot ( const VDBManager * self,
    struct VPath const * path );

/* DeleteCacheOlderThan
 *  delete old cached files
 *
 *  days [ IN ] - files older than this will be deleted
 *
 */
VDB_EXTERN rc_t CC VDBManagerDeleteCacheOlderThan ( const VDBManager * self,
    uint32_t days );


#ifdef __cplusplus
}
#endif

#endif /* _h_vdb_manager_ */
