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

#ifndef _h_kdb_table_
#define _h_kdb_table_

#ifndef _h_kdb_extern_
#include <kdb/extern.h>
#endif

#ifndef _h_kfs_directory_
#include <kfs/directory.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KDatabase;
struct KDBManager;
struct VPath;


/*--------------------------------------------------------------------------
 * KTable
 *  a collection of columns indexed by row id, metadata, indices
 */
typedef struct KTable KTable;


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
KDB_EXTERN rc_t CC KTableAddRef ( const KTable *self );
KDB_EXTERN rc_t CC KTableRelease ( const KTable *self );


/* CreateTable
 * VCreateTable
 *  create a new or open an existing table
 *
 *  "tbl" [ OUT ] - return parameter for newly opened table
 *
 *  "cmode" [ IN ] - creation mode
 *
 *  "cmode_mask" [ IN ] - if a bit of "cmode_mask" is set (1) then
 *  the corresponding bit of "cmode" is used for the table,
 *  otherwise (0) the corresponding bit is taken from db and "cmode"'s
 *  bit is ignored
 *  the mask for setting mode (kcmOpen, kcmInit, kcmCreate) is at least
 *  one bit set in the mask kcmValueMask.
 *
 *  "path" [ IN ] - NUL terminated string in
 *  wd-native character set giving path to table
 */
KDB_EXTERN rc_t CC KDBManagerCreateTable ( struct KDBManager *self,
    KTable **tbl, KCreateMode cmode, const char *path, ... );
KDB_EXTERN rc_t CC KDatabaseCreateTableByMask ( struct KDatabase *self,
    KTable **tbl, KCreateMode cmode, KCreateMode cmode_mask, const char *name, ... );
/* the following function is DEPRECATED, it's left for backward compatibility only */
KDB_EXTERN rc_t CC KDatabaseCreateTable ( struct KDatabase *self,
    KTable **tbl, KCreateMode cmode, const char *name, ... );

/* Default function is added only to make tools/kqsh build possible since
   it requires 1 to 1 mapping between K- and V-functions
*/
/*KDB_EXTERN rc_t CC KDatabaseCreateTableDefault ( struct KDatabase *self,
    KTable **tbl, const char *name, ... );*/
KDB_EXTERN rc_t CC KDBManagerVCreateTable ( struct KDBManager *self,
    KTable **tbl, KCreateMode cmode, const char *path, va_list args );
KDB_EXTERN rc_t CC KDatabaseVCreateTableByMask ( struct KDatabase *self,
    KTable **tbl, KCreateMode cmode, KCreateMode cmode_mask, const char *name, va_list args );
/* the following function is DEPRECATED, it's left for backward compatibility only */
KDB_EXTERN rc_t CC KDatabaseVCreateTable ( struct KDatabase *self,
    KTable **tbl, KCreateMode cmode, const char *name, va_list args );


/* OpenTableRead
 * VOpenTableRead
 * OpenTableReadVPath
 *  open a table for read
 *
 *  "tbl" [ OUT ] - return parameter for newly opened table
 *
 *  "path" [ IN ] - NUL terminated string in
 *  wd-native character set giving path to table
 */
KDB_EXTERN rc_t CC KDBManagerOpenTableRead ( struct KDBManager const *self,
    const KTable **tbl, const char *path, ... );
KDB_EXTERN rc_t CC KDatabaseOpenTableRead ( struct KDatabase const *self,
    const KTable **tbl, const char *name, ... );

KDB_EXTERN rc_t CC KDBManagerVOpenTableRead ( struct KDBManager const *self,
    const KTable **tbl, const char *path, va_list args );
KDB_EXTERN rc_t CC KDatabaseVOpenTableRead ( struct KDatabase const *self,
    const KTable **tbl, const char *name, va_list args );
KDB_EXTERN rc_t CC KDBManagerOpenTableReadVPath ( struct KDBManager const *self,
    const KTable **tbl, const struct VPath *path );


/* OpenTableUpdate
 * VOpenTableUpdate
 *  open a table for read/write
 *
 *  "tbl" [ OUT ] - return parameter for newly opened table
 *
 *  "name" [ IN ] - NUL terminated string in UTF-8 giving name of table
 */
KDB_EXTERN rc_t CC KDBManagerOpenTableUpdate ( struct KDBManager *self,
    KTable **tbl, const char *path, ... );
KDB_EXTERN rc_t CC KDatabaseOpenTableUpdate ( struct KDatabase *self,
    KTable **tbl, const char *name, ... );

KDB_EXTERN rc_t CC KDBManagerVOpenTableUpdate ( struct KDBManager *self,
    KTable **tbl, const char *path, va_list args );
KDB_EXTERN rc_t CC KDatabaseVOpenTableUpdate ( struct KDatabase *self,
    KTable **tbl, const char *name, va_list args );


/* Locked
 *  returns true if table is locked
 */
KDB_EXTERN bool CC KTableLocked ( const KTable *self );


/* Exists
 *  returns true if requested object exists
 *
 *  "type" [ IN ] - a KDBPathType
 *  valid values are kptIndex and kptColumn
 *
 *  "path" [ IN ] - NUL terminated path
 */
KDB_EXTERN bool CC KTableExists ( const KTable *self, uint32_t type,
    const char *name, ... );
KDB_EXTERN bool CC KTableVExists ( const KTable *self, uint32_t type,
    const char *name, va_list args );

/* IsAlias
 *  returns true if object name is an alias
 *  returns path to fundamental name if it was aliased
 *
 *  "type" [ IN ] - a KDBPathType
 *  valid values are kptIndex and kptColumn
 *
 *  "resolved" [ OUT ] and "rsize" [ IN ] - optional output buffer
 *  for fundamenta object name if "alias" is not a fundamental name, 
 *
 *  "name" [ IN ] - NUL terminated object name
 */
KDB_EXTERN bool CC KTableIsAlias ( const KTable *self, uint32_t type,
    char *resolved, size_t rsize, const char *name );

/* Writable
 *  returns 0 if object is writable
 *  or a reason why if not
 *
 *  "type" [ IN ] - a KDBPathType
 *  valid values are kptIndex and kptColumn
 *
 *  "path" [ IN ] - NUL terminated path
 */
KDB_EXTERN rc_t CC KTableWritable ( const KTable *self, uint32_t type,
    const char *name, ... );
KDB_EXTERN rc_t CC KTableVWritable ( const KTable *self, uint32_t type,
    const char *name, va_list args );

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
KDB_EXTERN rc_t CC KTableLock ( KTable *self, uint32_t type,
    const char *name, ... );
KDB_EXTERN rc_t CC KTableVLock ( KTable *self, uint32_t type,
    const char *name, va_list args );

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
KDB_EXTERN rc_t CC KTableUnlock ( KTable *self, uint32_t type,
    const char *name, ... );
KDB_EXTERN rc_t CC KTableVUnlock ( KTable *self, uint32_t type,
    const char *name, va_list args );


/* Rename
 *  renames a contained object
 *
 *  "from" [ IN ] - NUL terminated string in UTF-8
 *  giving name of contained object
 *
 *  "to" [ IN ] - NUL terminated string in UTF-8
 *  giving new name
 */
KDB_EXTERN rc_t CC KTableRenameColumn ( KTable *self, bool force,
    const char *from, const char *to );
KDB_EXTERN rc_t CC KTableRenameIndex ( KTable *self, bool force,
    const char *from, const char *to );


/* Alias
 *  create an alias to an existing contained object
 *
 *  "name" [ IN ] - NUL terminated string in UTF-8
 *  giving name of contained object
 *
 *  "alias" [ IN ] - NUL terminated string in UTF-8
 *  giving aliased name
 */
KDB_EXTERN rc_t CC KTableAliasColumn ( KTable *self, const char *path, const char *alias );
KDB_EXTERN rc_t CC KTableAliasIndex ( KTable *self, const char *name, const char *alias );


/* Drop
 *  drop a contained object
 *
 *  "name" [ IN ] - NUL terminated string in UTF-8 giving column name
 */
KDB_EXTERN rc_t CC KTableDropColumn ( KTable *self, const char *name, ... );
KDB_EXTERN rc_t CC KTableDropIndex ( KTable *self, const char *name, ... );

KDB_EXTERN rc_t CC KTableVDropColumn ( KTable *self, const char *name, va_list args );
KDB_EXTERN rc_t CC KTableVDropIndex ( KTable *self, const char *name, va_list args );


/* Reindex
 *  optimize column indices
 */
KDB_EXTERN rc_t CC KTableReindex ( KTable *self );


/* OpenManager
 *  duplicate reference to manager
 *  NB - returned reference must be released
 */
KDB_EXTERN rc_t CC KTableOpenManagerRead ( const KTable *self, struct KDBManager const **mgr );
KDB_EXTERN rc_t CC KTableOpenManagerUpdate ( KTable *self, struct KDBManager **mgr );


/* OpenParent
 *  duplicate reference to parent database
 *  NB - returned reference must be released
 */
KDB_EXTERN rc_t CC KTableOpenParentRead ( const KTable *self, struct KDatabase const **db );
KDB_EXTERN rc_t CC KTableOpenParentUpdate ( KTable *self, struct KDatabase **db );


/* HasRemoteData
 *  indicates whether some/all table data comes from network resource
 *  such as HttpFile or CacheteeFile
 */
KDB_EXTERN bool CC KTableHasRemoteData ( const KTable *self );

#ifdef __cplusplus
}
#endif

#endif /*  _h_kdb_table_ */
