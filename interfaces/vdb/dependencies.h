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


#ifndef _h_vdb_dependencies_
#define _h_vdb_dependencies_


#ifndef _h_vdb_extern_
#include <vdb/extern.h>
#endif

#ifndef _h_klib_rc_
#include <klib/rc.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct VDatabase;
struct VTable;


/*--------------------------------------------------------------------------
 * VDBDependencies
 *  information about VDatabase dependencies
 *
 *  object contains zero or more dependencies
 *
 *  each dependency has a number of properties, accessible via index
 */
typedef struct VDBDependencies VDBDependencies;


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored */
VDB_EXTERN rc_t CC VDBDependenciesAddRef ( const VDBDependencies *self );
VDB_EXTERN rc_t CC VDBDependenciesRelease ( const VDBDependencies *self );

/* ListDependencies
 *  create dependencies object: list dependencies
 *
 *  Cache could be updated after the fuction call, if caching is enabled.
 *
 *  "dep" [ OUT ] - return for VDBDependencies object
 *
 *  "missing" [ IN ] - if true, list only missing dependencies
 *  otherwise, list all dependencies
 */
VDB_EXTERN rc_t CC VDatabaseListDependencies ( struct VDatabase const *self,
    const VDBDependencies **dep, bool missing );

/* ListDependenciesWithCaching
 *  create dependencies object: list dependencies
 *
 *  Allows to control cache update inside the function.
 *
 *  "dep" [ OUT ] - return for VDBDependencies object
 *
 *  "missing" [ IN ] - if true, list only missing dependencies
 *  otherwise, list all dependencies
 *
 *  "disableCaching" [ IN ] - if true, disable caching inside of the function
 *  otherwise, do not change the caching state
 */
VDB_EXTERN rc_t CC VDatabaseListDependenciesWithCaching (
    struct VDatabase const *self,
    const VDBDependencies **dep, bool missing,
    bool disableCaching );

/* Count
 *  retrieve the number of dependencies
 *
 *  "count" [ OUT ] - return parameter for dependencies count
 */
VDB_EXTERN rc_t CC VDBDependenciesCount ( const VDBDependencies *self,
    uint32_t *count );


/* Type
 *  describe the dependency object type ( see kbd/manager.h )
 *
 *  "type" [ OUT ] - a KDBPathType
 *
 *  "idx" [ IN ] - zero-based index of dependency
 */
VDB_EXTERN rc_t CC VDBDependenciesType ( const VDBDependencies *self,
    uint32_t *type, uint32_t idx );


/* Local
 *  retrieve local property
 *
 *  "local" [ OUT ] - true if object is stored internally
 *
 *  "idx" [ IN ] - zero-based index of dependency
 */
VDB_EXTERN rc_t CC VDBDependenciesLocal ( const VDBDependencies *self,
    bool *local, uint32_t idx );


/* SeqId
 *  retrieve sequence id of object
 *
 *  "seq_id" [ OUT ] - returned pointer should not be freed.
 *   becomes invalid after VDBDependenciesRelease
 *
 *  "idx" [ IN ] - zero-based index of dependency
 */
VDB_EXTERN rc_t CC VDBDependenciesSeqId ( const VDBDependencies *self,
    const char **seq_id, uint32_t idx );


/* Name
 *  retrieve object name
 *
 * "name" [ OUT ] - returner pointed should not be freed.
 *  it becomes invalid after VDBDependenciesRelease
 *
 *  "idx" [ IN ] - zero-based index of dependency
 */
VDB_EXTERN rc_t CC VDBDependenciesName ( const VDBDependencies *self,
    const char **name, uint32_t idx );


/* Circular
 *  retrieve circular property
 *  reference sequences may be circular
 *
 *  "circular" [ OUT ] - true if dependency is a circular reference table
 *
 *  "idx" [ IN ] - zero-based index of dependency
 */
VDB_EXTERN rc_t CC VDBDependenciesCircular ( const VDBDependencies *self,
    bool *circular, uint32_t idx );


/* Path
 *  returns [Local] path for resolved dependency,
 *  returns NULL for local and missing dependency.
 *
 *  "path" [ OUT ] - returned pointer should not be freed.
 *   it becomes invalid after VDBDependenciesRelease
 *
 *  "idx" [ IN ] - zero-based index of dependency
 */
VDB_EXTERN rc_t CC VDBDependenciesPath ( const VDBDependencies *self,
    const char **path, uint32_t idx );

/* VPath
 *  returns [Local] path for resolved dependency,
 *  returns NULL for local and missing dependency.
 *
 *  "path" [ OUT ]
 *
 *  "idx" [ IN ] - zero-based index of dependency
 */
VDB_EXTERN rc_t CC VDBDependenciesVPath(const VDBDependencies *self,
    const struct VPath **path, uint32_t idx);

/* PathRemote
 *  returns Remote path for dependency,
 *  returns NULL for not found dependency.
 *
 *  "path" [ OUT ] - returned pointer should not be freed.
 *   it becomes invalid after VDBDependenciesRelease
 *
 *  "idx" [ IN ] - zero-based index of dependency
 */
VDB_EXTERN rc_t CC VDBDependenciesPathRemote ( const VDBDependencies *self,
    const char **path, uint32_t idx );

/* PathCache
 *  returns Cache path for dependency,
 *  returns NULL for not found dependency.
 *
 *  "path" [ OUT ] - returned pointer should not be freed.
 *   it becomes invalid after VDBDependenciesRelease
 *
 *  "idx" [ IN ] - zero-based index of dependency
 */
VDB_EXTERN rc_t CC VDBDependenciesPathCache ( const VDBDependencies *self,
    const char **path, uint32_t idx );


/* RemoteAndCache
 *  returns Cache and remote path and rc_t for dependency.
 *
 *  "idx" [ IN ] - zero-based index of dependency
 */
VDB_EXTERN rc_t CC VDBDependenciesRemoteAndCache(const VDBDependencies *self,
    uint32_t idx, rc_t *remoteRc, const struct VPath **remote,
    rc_t *cacheRc, const struct VPath **cache);


/* Error
 *  trying to analyze rc code and object
 *  in order to recognize dependencies error
 *
 *  db OR table - only one is required
 */
VDB_EXTERN bool CC UIError( rc_t rc,
    struct VDatabase const *db, struct VTable const *table);

/* report error(s) via PLOGERR based on an object
 * log_list - if true logs the missing items list also
 */
VDB_EXTERN void CC UIDatabaseLOGError( rc_t rc,
    struct VDatabase const *db, bool log_list );
VDB_EXTERN void CC UITableLOGError( rc_t rc,
    struct VTable const *table, bool log_list );

/* return human-readable string explaining rc code.
 * the string should not be freed
 */
VDB_EXTERN const char* CC UIDatabaseGetErrorString(rc_t rc);

#ifdef __cplusplus
}
#endif


#endif /* _h_vdb_dependencies_ */
