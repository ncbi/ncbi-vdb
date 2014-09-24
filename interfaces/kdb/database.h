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

#ifndef _h_kdb_database_
#define _h_kdb_database_

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
struct KDBManager;


/*--------------------------------------------------------------------------
 * KDatabase
 *  opaque connection to a database within file system
 */
typedef struct KDatabase KDatabase;


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
KDB_EXTERN rc_t CC KDatabaseAddRef ( const KDatabase *self );
KDB_EXTERN rc_t CC KDatabaseRelease ( const KDatabase *self );


/* CreateDB
 * VCreateDB
 *  create a new or open an existing database
 *
 *  "db" [ OUT ] - return parameter for newly opened database
 *
 *  "cmode" [ IN ] - creation mode
 *
 *  "path" [ IN ] - NUL terminated string in
 *  wd-native character set giving path to database
 */
KDB_EXTERN rc_t CC KDBManagerCreateDB ( struct KDBManager *self,
    KDatabase **db, KCreateMode cmode, const char *path, ... );
KDB_EXTERN rc_t CC KDatabaseCreateDB ( KDatabase *self,
    KDatabase **db, KCreateMode cmode, const char *name, ... );

KDB_EXTERN rc_t CC KDBManagerVCreateDB ( struct KDBManager *self,
    KDatabase **db, KCreateMode cmode, const char *path, va_list args );
KDB_EXTERN rc_t CC KDatabaseVCreateDB ( KDatabase *self,
    KDatabase **db, KCreateMode cmode, const char *name, va_list args );


/* OpenDBRead
 * VOpenDBRead
 *  open a database for read
 *
 *  "db" [ OUT ] - return parameter for newly opened database
 *
 *  "path" [ IN ] - NUL terminated string in
 *  wd-native character set giving path to database
 */
KDB_EXTERN rc_t CC KDBManagerOpenDBRead ( struct KDBManager const *self,
    const KDatabase **db, const char *path, ... );
KDB_EXTERN rc_t CC KDatabaseOpenDBRead ( const KDatabase *self,
    const KDatabase **db, const char *name, ... );

KDB_EXTERN rc_t CC KDBManagerVOpenDBRead ( struct KDBManager const *self,
    const KDatabase **db, const char *path, va_list args );
KDB_EXTERN rc_t CC KDatabaseVOpenDBRead ( const KDatabase *self,
    const KDatabase **db, const char *name, va_list args );


/* OpenDBUpdate
 * VOpenDBUpdate
 *  open a database for read/write
 *
 *  "db" [ OUT ] - return parameter for newly opened database
 *
 *  "path" [ IN ] - NUL terminated string in
 *  wd-native character set giving path to database
 */
KDB_EXTERN rc_t CC KDBManagerOpenDBUpdate ( struct KDBManager *self,
    KDatabase **db, const char *path, ... );
KDB_EXTERN rc_t CC KDatabaseOpenDBUpdate ( KDatabase *self,
    KDatabase **db, const char *name, ... );

KDB_EXTERN rc_t CC KDBManagerVOpenDBUpdate ( struct KDBManager *self,
    KDatabase **db, const char *path, va_list args );
KDB_EXTERN rc_t CC KDatabaseVOpenDBUpdate ( KDatabase *self,
    KDatabase **db, const char *name, va_list args );


/* Locked
 *  returns true if locked
 */
KDB_EXTERN bool CC KDatabaseLocked ( const KDatabase *self );


/* Exists
 *  returns true if requested object exists
 *
 *  "type" [ IN ] - a KDBPathType
 *  valid values are kptDatabase, kptTable and kptIndex
 *
 *  "name" [ IN ] - NUL terminated object name
 */
KDB_EXTERN bool CC KDatabaseExists ( const KDatabase *self, uint32_t type,
    const char *name, ... );
KDB_EXTERN bool CC KDatabaseVExists ( const KDatabase *self, uint32_t type,
    const char *name, va_list args );

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
KDB_EXTERN bool CC KDatabaseIsAlias ( const KDatabase *self, uint32_t type,
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
KDB_EXTERN rc_t CC KDatabaseWritable ( const KDatabase *self, uint32_t type,
    const char *name, ... );
KDB_EXTERN rc_t CC KDatabaseVWritable ( const KDatabase *self, uint32_t type,
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
KDB_EXTERN rc_t CC KDatabaseLock ( KDatabase *self, uint32_t type,
    const char *name, ... );
KDB_EXTERN rc_t CC KDatabaseVLock ( KDatabase *self, uint32_t type,
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
KDB_EXTERN rc_t CC KDatabaseUnlock ( KDatabase *self, uint32_t type,
    const char *name, ... );
KDB_EXTERN rc_t CC KDatabaseVUnlock ( KDatabase *self, uint32_t type,
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
KDB_EXTERN rc_t CC KDatabaseRenameDB ( KDatabase *self, bool force, const char *from, const char *to );
KDB_EXTERN rc_t CC KDatabaseRenameTable ( KDatabase *self, bool force, const char *from, const char *to );
KDB_EXTERN rc_t CC KDatabaseRenameIndex ( KDatabase *self, bool force, const char *from, const char *to );


/* Alias
 *  create an alias to an existing contained object
 *
 *  "obj" [ IN ] - NUL terminated string in UTF-8
 *  giving name of contained object
 *
 *  "alias" [ IN ] - NUL terminated string in UTF-8
 *  giving aliased name
 */
KDB_EXTERN rc_t CC KDatabaseAliasDB ( KDatabase *self, const char *obj, const char *alias );
KDB_EXTERN rc_t CC KDatabaseAliasTable ( KDatabase *self, const char *obj, const char *alias );
KDB_EXTERN rc_t CC KDatabaseAliasIndex ( KDatabase *self, const char *obj, const char *alias );


/* Drop
 *  drop a contained object
 *
 *  "name" [ IN ] - NUL terminated string in UTF-8 giving name of db
 */
KDB_EXTERN rc_t CC KDatabaseDropDB ( KDatabase *self, const char *name, ... );
KDB_EXTERN rc_t CC KDatabaseDropTable ( KDatabase *self, const char *name, ... );
KDB_EXTERN rc_t CC KDatabaseDropIndex ( KDatabase *self, const char *name, ... );

KDB_EXTERN rc_t CC KDatabaseVDropDB ( KDatabase *self, const char *name, va_list args );
KDB_EXTERN rc_t CC KDatabaseVDropTable ( KDatabase *self, const char *name, va_list args );
KDB_EXTERN rc_t CC KDatabaseVDropIndex ( KDatabase *self, const char *name, va_list args );


/* OpenManager
 *  duplicate reference to manager
 *  NB - returned reference must be released
 */
KDB_EXTERN rc_t CC KDatabaseOpenManagerRead ( const KDatabase *self, struct KDBManager const **mgr );
KDB_EXTERN rc_t CC KDatabaseOpenManagerUpdate ( KDatabase *self, struct KDBManager **mgr );


/* OpenParent
 *  duplicate reference to parent database
 *  NB - returned reference must be released
 */
KDB_EXTERN rc_t CC KDatabaseOpenParentRead ( const KDatabase *self, const KDatabase **par );
KDB_EXTERN rc_t CC KDatabaseOpenParentUpdate ( KDatabase *self, KDatabase **par );


#ifdef __cplusplus
}
#endif

#endif /* _h_kdb_database_ */
