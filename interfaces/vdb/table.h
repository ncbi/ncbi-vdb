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

#ifndef _h_vdb_table_
#define _h_vdb_table_

#ifndef _h_vdb_extern_
#include <vdb/extern.h>
#endif

#ifndef _h_kfs_directory_
#include <kfs/directory.h>
#endif

#ifndef _h_kdb_column_
#include <kdb/column.h>
#endif

#ifndef _h_kdb_index_
#include <kdb/index.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct VSchema;
struct KMetadata;
struct KNamelist;
struct VDatabase;
struct VDBManager;


/*--------------------------------------------------------------------------
 * VTable
 *  a collection of columns indexed by row id, metadata, indices
 */
typedef struct VTable VTable;


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
VDB_EXTERN rc_t CC VTableAddRef ( const VTable *self );
VDB_EXTERN rc_t CC VTableRelease ( const VTable *self );


/* CreateTable
 *  create a new or open an existing table using manager
 *
 *  "tbl" [ OUT ] - return parameter for newly opened table
 *
 *  "schema" [ IN ] - schema object containg table
 *  declaration to be used in creating tbl.
 *
 *  "typespec" [ IN ] - type and optionally version of table schema,
 *  e.g. 'MY_NAMESPACE:MyTable' or 'MY_NAMESPACE:MyTable#1.1'
 *
 *  "cmode" [ IN ] - creation mode
 *
 *  "path" [ IN ] - NUL terminated string in
 *  wd-native character set giving path to table
 */
VDB_EXTERN rc_t CC VDBManagerCreateTable ( struct VDBManager *self, VTable **tbl,
    struct VSchema const *schema, const char *typespec,
    KCreateMode cmode, const char *path, ... );
VDB_EXTERN rc_t CC VDBManagerVCreateTable ( struct VDBManager *self, VTable **tbl,
    struct VSchema const *schema, const char *typespec,
    KCreateMode cmode, const char *path, va_list args );


/* CreateTable
 *  create a new or open an existing table under database
 *
 *  "tbl" [ OUT ] - return parameter for newly opened table
 *
 *  "member" [ IN ] - name of table member template under database
 *  the named member is a table template rather than a named table.
 *
 *  "cmode" [ IN ] - creation mode
 *
 *  "name" [ IN ] - NUL terminated string in
 *  db-native character set giving actual table name
 */
VDB_EXTERN rc_t CC VDatabaseCreateTable ( struct VDatabase *self, VTable **tbl,
    const char *member, KCreateMode cmode, const char *name, ... );
VDB_EXTERN rc_t CC VDatabaseVCreateTable ( struct VDatabase *self, VTable **tbl,
    const char *member, KCreateMode cmode, const char *name, va_list args );


/* OpenTableRead
 *  open a table for read using manager
 * OpenTableUpdate
 *  open a table for read/write using manager
 *
 *  "tbl" [ OUT ] - return parameter for newly opened table
 *
 *  "schema" [ IN, NULL OKAY ] - optional schema object with
 *  latest versions of table schema.
 *
 *  "path" [ IN ] - NUL terminated string in
 *  wd-native character set giving path to table
 */
VDB_EXTERN rc_t CC VDBManagerOpenTableRead ( struct VDBManager const *self,
    const VTable **tbl, struct VSchema const *schema,
    const char *path, ... );
VDB_EXTERN rc_t CC VDBManagerVOpenTableRead ( struct VDBManager const *self,
    const VTable **tbl, struct VSchema const *schema,
    const char *path, va_list args );

VDB_EXTERN rc_t CC VDBManagerOpenTableUpdate ( struct VDBManager *self,
    VTable **tbl, struct VSchema const *schema,
    const char *path, ... );
VDB_EXTERN rc_t CC VDBManagerVOpenTableUpdate ( struct VDBManager *self,
    VTable **tbl, struct VSchema const *schema,
    const char *path, va_list args );


/* OpenTableRead
 *  open a table for read under database
 * OpenTableUpdate
 *  open or create a table for read/write under database
 *
 *  "tbl" [ OUT ] - return parameter for newly opened table
 *
 *  "name" [ IN ] - NUL terminated string in
 *  db-native character set giving name of table member
 */
VDB_EXTERN rc_t CC VDatabaseOpenTableRead ( struct VDatabase const *self,
    const VTable **tbl, const char *name, ... );
VDB_EXTERN rc_t CC VDatabaseVOpenTableRead ( struct VDatabase const *self,
    const VTable **tbl, const char *name, va_list args );

VDB_EXTERN rc_t CC VDatabaseOpenTableUpdate ( struct VDatabase *self,
    VTable **tbl, const char *name, ... );
VDB_EXTERN rc_t CC VDatabaseVOpenTableUpdate ( struct VDatabase *self,
    VTable **tbl, const char *name, va_list args );


/* Typespec
 *  retrieve table typespec
 *
 *  AVAILABILITY: version 2.1
 *
 *  "ts_buff" [ IN ] and "ts_size" [ IN ] - buffer for return of NUL-terminated
 *  table typespec, e.g. 'NCBI:tbl:abc #1.1'
 */
VDB_EXTERN rc_t CC VTableTypespec ( const VTable *self,
    char *ts_buff, size_t ts_size );


/* Locked
 *  returns true if locked
 */
VDB_EXTERN bool CC VTableLocked ( const VTable *self );


/* Writable
 *  returns 0 if object is writable
 *  or a reason why if not
 *
 *  "type" [ IN ] - a KDBPathType
 *  valid values are kptIndex and kptColumn
 *
 *  "path" [ IN ] - NUL terminated path
 */
VDB_EXTERN rc_t CC VTableWritable ( const VTable *self, uint32_t type,
    const char * name, ... );
VDB_EXTERN rc_t CC VTableVWritable ( const VTable *self, uint32_t type,
    const char * name, va_list args );

/* Lock
 *  apply lock
 *
 *  if object is already locked, the operation is idempotent
 *  and returns an rc state of rcLocked
 *
 *  "type" [ IN ] - a KDBPathType
 *  valid values are kptIndex and kptColumn
 *
 *  "path" [ IN ] - NUL terminated path
 */
VDB_EXTERN rc_t CC VTableLock ( VTable *self, uint32_t type,
    const char * name, ... );
VDB_EXTERN rc_t CC VTableVLock ( VTable *self, uint32_t type,
    const char * name, va_list args );

/* Unlock
 *  remove lock
 *
 *  if object is already unlocked, the operation is idempotent
 *  and returns an rc state of rcUnlocked
 *
 *  "type" [ IN ] - a KDBPathType
 *  valid values are kptIndex and kptColumn
 *
 *  "path" [ IN ] - NUL terminated path
 */
VDB_EXTERN rc_t CC VTableUnlock ( VTable *self, uint32_t type,
    const char * name, ... );
VDB_EXTERN rc_t CC VTableVUnlock ( VTable *self, uint32_t type,
    const char * name, va_list args );


/* OpenMetadataRead
 * OpenMetadataUpdate
 *  opens metadata file
 *
 *  "meta" [ OUT ] - return parameter for metadata
 */
VDB_EXTERN rc_t CC VTableOpenMetadataRead ( const VTable *self, struct KMetadata const **meta );
VDB_EXTERN rc_t CC VTableOpenMetadataUpdate ( VTable *self, struct KMetadata **meta );


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
VDB_EXTERN rc_t CC VTableColumnCreateParams ( VTable *self,
    KCreateMode cmode, KChecksum checksum, size_t pgsize );


/* CreateIndex
 * VCreateIndex
 *  create a new or open an existing index
 *
 *  "idx" [ OUT ] - return parameter for newly opened index
 *
 *  "type" [ IN ] - type of index to create
 *
 *  "cmode" [ IN ] - creation mode
 *
 *  "name" [ IN ] - NUL terminated string in UTF-8 giving simple name of idx
 */
VDB_EXTERN rc_t CC VTableCreateIndex ( VTable *self, KIndex **idx,
    KIdxType type, KCreateMode cmode, const char *name, ... );
VDB_EXTERN rc_t CC VTableVCreateIndex ( VTable *self, KIndex **idx,
    KIdxType type, KCreateMode cmode, const char *name, va_list args );


/* OpenIndexRead
 * VOpenIndexRead
 *  open an index for read
 *
 *  "idx" [ OUT ] - return parameter for newly opened index
 *
 *  "name" [ IN ] - NUL terminated string in UTF-8 giving simple name of idx
 */
VDB_EXTERN rc_t CC VTableOpenIndexRead ( const VTable *self,
    const KIndex **idx, const char *name, ... );
VDB_EXTERN rc_t CC VTableVOpenIndexRead ( const VTable *self,
    const KIndex **idx, const char *name, va_list args );


/* OpenIndexUpdate
 * VOpenIndexUpdate
 *  open an index for read/write
 *
 *  "idx" [ OUT ] - return parameter for newly opened index
 *
 *  "name" [ IN ] - NUL terminated string in UTF-8 giving simple name of idx
 */

VDB_EXTERN rc_t CC VTableOpenIndexUpdate ( VTable *self,
    KIndex **idx, const char *name, ... );
VDB_EXTERN rc_t CC VTableVOpenIndexUpdate ( VTable *self,
    KIndex **idx, const char *name, va_list args );


/* ListColumns
 *  list readable or writable column names
 *
 *  "names" [ OUT ] - return parameter for namelist
 *
 *  availability: v2.1
 */
VDB_EXTERN rc_t CC VTableListReadableColumns ( const VTable *self, struct KNamelist **names );
VDB_EXTERN rc_t CC VTableListWritableColumns ( VTable *self, struct KNamelist **names );


/* ListCol
 *  alias for VTableListReadableColumns
 *
 *  availability: v2.0
 */
VDB_EXTERN rc_t CC VTableListCol ( const VTable *self, struct KNamelist **names );


/* ListDatatypes
 *  returns list of typedecls for named column
 *
 *  "col" [ IN ] - column name
 *
 *  "dflt_idx" [ OUT, NULL OKAY ] - returns the zero-based index
 *  into "typedecls" of the default datatype for the named column
 *
 *  "typedecls" [ OUT ] - list of datatypes available for named column
 *
 *  availability: v2.1
 */
VDB_EXTERN rc_t CC VTableListReadableDatatypes ( const VTable *self, const char *col,
    uint32_t *dflt_idx, struct KNamelist **typedecls );
VDB_EXTERN rc_t CC VTableListWritableDatatypes ( VTable *self,
    const char *col, struct KNamelist **typedecls );


/* ColumnDatatypes
 *  alias for VTableListReadableDatatypes
 *
 *  availability: v2.0
 */
VDB_EXTERN rc_t CC VTableColumnDatatypes ( const VTable *self, const char *col,
    uint32_t *dflt_idx, struct KNamelist **typedecls );


/* Reindex
 *  optimize column indices
 */
VDB_EXTERN rc_t CC VTableReindex ( VTable *self );


/* OpenManager
 *  duplicate reference to manager
 *  NB - returned reference must be released
 */
VDB_EXTERN rc_t CC VTableOpenManagerRead ( const VTable *self, struct VDBManager const **mgr );
VDB_EXTERN rc_t CC VTableOpenManagerUpdate ( VTable *self, struct VDBManager **mgr );


/* OpenParent
 *  duplicate reference to parent database
 *  NB - returned reference must be released
 */
VDB_EXTERN rc_t CC VTableOpenParentRead ( const VTable *self, struct VDatabase const **db );
VDB_EXTERN rc_t CC VTableOpenParentUpdate ( VTable *self, struct VDatabase **db );


/* OpenSchema
 *  duplicate reference to table schema
 *  NB - returned reference must be released
 */
VDB_EXTERN rc_t CC VTableOpenSchema ( const VTable *self, struct VSchema const **schema );


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
VDB_EXTERN rc_t CC VTableGetUserData ( const VTable *self, void **data );
VDB_EXTERN rc_t CC VTableSetUserData ( const VTable *self,
    void *data, void ( CC * destroy ) ( void *data ) );


#ifdef __cplusplus
}
#endif

#endif /*  _h_vdb_table_ */
