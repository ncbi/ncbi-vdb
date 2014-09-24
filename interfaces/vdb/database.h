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

#ifndef _h_vdb_database_
#define _h_vdb_database_

#ifndef _h_vdb_extern_
#include <vdb/extern.h>
#endif

#ifndef _h_kfs_directory_
#include <kfs/directory.h>
#endif

#ifndef _h_kdb_column_
#include <kdb/column.h>
#endif

#ifndef _h_klib_namelist_
#include <klib/namelist.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct VSchema;
struct KMetadata;
struct VDBManager;


/*--------------------------------------------------------------------------
 * VDatabase
 *  opaque connection to a database within file system
 */
typedef struct VDatabase VDatabase;


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
VDB_EXTERN rc_t CC VDatabaseAddRef ( const VDatabase *self );
VDB_EXTERN rc_t CC VDatabaseRelease ( const VDatabase *self );


/* CreateDB
 *  create a new or open an existing database using manager
 *
 *  "db" [ OUT ] - return parameter for newly opened database
 *
 *  "schema" [ IN ] - schema object containg database
 *  declaration to be used in creating db.
 *
 *  "typespec" [ IN ] - type and optionally version of db schema,
 *  e.g. 'MY_NAMESPACE:MyDatabase' or 'MY_NAMESPACE:MyDatabase#1.1'
 *
 *  "cmode" [ IN ] - creation mode
 *
 *  "path" [ IN ] - NUL terminated string in
 *  wd-native character set giving path to database
 */
VDB_EXTERN rc_t CC VDBManagerCreateDB ( struct VDBManager *self, VDatabase **db,
    struct VSchema const *schema, const char *typespec,
    KCreateMode cmode, const char *path, ... );
VDB_EXTERN rc_t CC VDBManagerVCreateDB ( struct VDBManager *self, VDatabase **db,
    struct VSchema const *schema, const char *typespec,
    KCreateMode cmode, const char *path, va_list args );


/* CreateDB
 *  create a new or open an existing database under parent database
 *
 *  "db" [ OUT ] - return parameter for newly opened database
 *
 *  "member" [ IN ] - name of database member template under parent
 *  the named member is a db template rather than a named database.
 *
 *  "cmode" [ IN ] - creation mode
 *
 *  "name" [ IN ] - NUL terminated string in
 *  db-native character set giving actual table name
 */
VDB_EXTERN rc_t CC VDatabaseCreateDB ( VDatabase *self, VDatabase **db,
    const char *member, KCreateMode cmode, const char *name, ... );
VDB_EXTERN rc_t CC VDatabaseVCreateDB ( VDatabase *self, VDatabase **db,
    const char *member, KCreateMode cmode, const char *name, va_list args );

VDB_EXTERN rc_t CC VDatabaseDropDB ( VDatabase *self, const char *name, ... );
VDB_EXTERN rc_t CC VDatabaseVDropDB ( VDatabase *self, const char *name, va_list args );

VDB_EXTERN rc_t CC VDatabaseDropTable ( VDatabase *self, const char *name, ... );
VDB_EXTERN rc_t CC VDatabaseVDropTable ( VDatabase *self, const char *name, va_list args );
    

/* OpenDBRead
 *  open a database for read using manager
 * OpenDBUpdate
 *  open a database for read/write using manager
 *
 *  "db" [ OUT ] - return parameter for newly opened database
 *
 *  "schema" [ IN, NULL OKAY ] - schema object containg database
 *  declaration to be used in creating db [ needed by manager ].
 *
 *  "path" [ IN ] - NUL terminated string in
 *  wd-native character set giving path to database
 */
VDB_EXTERN rc_t CC VDBManagerOpenDBRead ( struct VDBManager const *self,
    const VDatabase **db, struct VSchema const *schema,
    const char *path, ... );
VDB_EXTERN rc_t CC VDBManagerVOpenDBRead ( struct VDBManager const *self,
    const VDatabase **db, struct VSchema const *schema,
    const char *path, va_list args );

VDB_EXTERN rc_t CC VDBManagerOpenDBUpdate ( struct VDBManager *self,
    VDatabase **db, struct VSchema const *schema,
    const char *path, ... );
VDB_EXTERN rc_t CC VDBManagerVOpenDBUpdate ( struct VDBManager *self,
    VDatabase **db, struct VSchema const *schema,
    const char *path, va_list args );


/* OpenDBRead
 *  open a database for read under parent db
 * OpenDBUpdate
 *  open or create a database for read/write under parent db
 *
 *  "db" [ OUT ] - return parameter for newly opened database
 *
 *  "name" [ IN ] - NUL terminated string in
 *  db-native character set giving name of database member
 */
VDB_EXTERN rc_t CC VDatabaseOpenDBRead ( const VDatabase *self,
    const VDatabase **db, const char *name, ... );
VDB_EXTERN rc_t CC VDatabaseVOpenDBRead ( const VDatabase *self,
    const VDatabase **db, const char *name, va_list args );

VDB_EXTERN rc_t CC VDatabaseOpenDBUpdate ( VDatabase *self,
    VDatabase **db, const char *name, ... );
VDB_EXTERN rc_t CC VDatabaseVOpenDBUpdate ( VDatabase *self,
    VDatabase **db, const char *name, va_list args );


/* Typespec
 *  retrieve database typespec
 *
 *  AVAILABILITY: version 2.2
 *
 *  "ts_buff" [ IN ] and "ts_size" [ IN ] - buffer for return of NUL-terminated
 *  database typespec, e.g. 'NCBI:db:abc #1.1'
 */
VDB_EXTERN rc_t CC VDatabaseTypespec ( const VDatabase *self,
    char *ts_buff, size_t ts_size );


/* Locked
 *  returns true if locked
 */
VDB_EXTERN bool CC VDatabaseLocked ( const VDatabase *self );


/* IsAlias
 *  returns true if object name is an alias
 *  returns path to fundamental name if it was aliased
 *
 *  "type" [ IN ] - a KDBPathType
 *  valid values are kptDatabase, kptTable and kptIndex
 *
 *  "resolved" [ OUT ] and "rsize" [ IN ] - optional output buffer
 *  for fundamenta object name if "alias" is not a fundamental name, 
 *
 *  "name" [ IN ] - NUL terminated object name
 */
VDB_EXTERN bool CC VDatabaseIsAlias ( const VDatabase *self, uint32_t type,
    char *resolved, size_t rsize, const char *name );


/* Writable
 *  returns 0 if object is writable
 *  or a reason why if not
 *
 *  "type" [ IN ] - a KDBPathType
 *  valid values are kptDatabase, kptTable and kptIndex
 *
 *  "path" [ IN ] - NUL terminated path
 */
VDB_EXTERN rc_t CC VDatabaseWritable ( const VDatabase *self, uint32_t type,
    const char *name, ... );
VDB_EXTERN rc_t CC VDatabaseVWritable ( const VDatabase *self, uint32_t type,
    const char *name, va_list args );

/* Lock
 *  apply lock
 *
 *  if object is already locked, the operation is idempotent
 *  and returns an rc state of rcLocked
 *
 *  "type" [ IN ] - a KDBPathType
 *  valid values are kptDatabase, kptTable and kptIndex
 *
 *  "path" [ IN ] - NUL terminated path
 */
VDB_EXTERN rc_t CC VDatabaseLock ( VDatabase *self, uint32_t type,
    const char *name, ... );
VDB_EXTERN rc_t CC VDatabaseVLock ( VDatabase *self, uint32_t type,
    const char *name, va_list args );

/* Unlock
 *  remove lock
 *
 *  if object is already unlocked, the operation is idempotent
 *  and returns an rc state of rcUnlocked
 *
 *  "type" [ IN ] - a KDBPathType
 *  valid values are kptDatabase, kptTable and kptIndex
 *
 *  "path" [ IN ] - NUL terminated path
 */
VDB_EXTERN rc_t CC VDatabaseUnlock ( VDatabase *self, uint32_t type,
    const char *name, ... );
VDB_EXTERN rc_t CC VDatabaseVUnlock ( VDatabase *self, uint32_t type,
    const char *name, va_list args );


/* OpenMetadataRead
 * OpenMetadataUpdate
 *  opens metadata file
 *
 *  "meta" [ OUT ] - return parameter for metadata
 */
VDB_EXTERN rc_t CC VDatabaseOpenMetadataRead ( const VDatabase *self, struct KMetadata const **meta );
VDB_EXTERN rc_t CC VDatabaseOpenMetadataUpdate ( VDatabase *self, struct KMetadata **meta );


/* ColumnCreateParams
 *  sets the creation parameters for physical columns
 *
 *  "cmode" [ IN ] - creation mode
 *
 *  "checksum" [ IN ] - the type of checksum information to
 *  apply when writing blobs
 *
 *  "pgsize" [ IN, DEFAULT ZERO ] - size of internal column "pages"
 *  the default value is indicated by 0 ( zero ).
 *  NB - CURRENTLY THE ONLY SUPPORTED PAGE SIZE IS 1 ( ONE ) BYTE.
 */
VDB_EXTERN rc_t CC VDatabaseColumnCreateParams ( VDatabase *self,
    KCreateMode cmode, KChecksum checksum, size_t pgsize );


/* OpenManager
 *  duplicate reference to manager
 *  NB - returned reference must be released
 */
VDB_EXTERN rc_t CC VDatabaseOpenManagerRead ( const VDatabase *self, struct VDBManager const **mgr );
VDB_EXTERN rc_t CC VDatabaseOpenManagerUpdate ( VDatabase *self, struct VDBManager **mgr );


/* OpenParent
 *  duplicate reference to parent database
 *  NB - returned reference must be released
 */
VDB_EXTERN rc_t CC VDatabaseOpenParentRead ( const VDatabase *self, const VDatabase **par );
VDB_EXTERN rc_t CC VDatabaseOpenParentUpdate ( VDatabase *self, VDatabase **par );


/* OpenSchema
 *  duplicate reference to db schema
 *  NB - returned reference must be released
 */
VDB_EXTERN rc_t CC VDatabaseOpenSchema ( const VDatabase *self, struct VSchema const **schema );


/* lists the tables of the database
 *
 * "names" [ OUT ] - return parameter for table 
 */
VDB_EXTERN rc_t CC VDatabaseListTbl ( const VDatabase *self, KNamelist **names );

/* lists the sub-databases of the database
 *
 * "names" [ OUT ] - return parameter for databases
 */
VDB_EXTERN rc_t CC VDatabaseListDB ( const VDatabase *self, KNamelist **names );

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
VDB_EXTERN rc_t CC VDatabaseGetUserData ( const VDatabase *self, void **data );
VDB_EXTERN rc_t CC VDatabaseSetUserData ( const VDatabase *self,
    void *data, void ( CC * destroy ) ( void *data ) );


#ifdef __cplusplus
}
#endif

#endif /* _h_vdb_database_ */
