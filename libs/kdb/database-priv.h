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

#ifndef _h_database_priv_
#define _h_database_priv_

#ifndef _h_kdb_database_
#include <kdb/database.h>
#endif

#include <kdb/column.h>

#ifndef _h_klib_symbol_
#include <klib/symbol.h>
#endif

#ifndef KONST
#define KONST
#endif

#define KDATABASE_IMPL KDatabase
#include "database-base.h"
#ifdef __cplusplus

extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
struct KDBManager;
struct KDirectory;
struct KMD5SumFmt;


/*--------------------------------------------------------------------------
 * KDatabase
 *  connection to a database within file system
 */

struct KDatabase
{
    KDatabaseBase dad;

    /* manager reference */
    struct KDBManager KONST *mgr;

    /* if a sub-directory */
    struct KDatabase KONST *parent;

    /* database directory */
    struct KDirectory KONST *dir;

    /* MD5 format object */
    struct KMD5SumFmt *md5;

    /* open references */
    uint32_t opencount;

    /* fits into manager symbol table */
    KSymbol sym;

    /* create and checksum modes for columns */
    KCreateMode cmode;
    KChecksum checksum;

    /* true if database was opened read-only */
    bool read_only;

    /* symbol text space */
    char path [ 1 ];
};

/* Attach
 * Sever
 *  like AddRef/Release, except called internally
 *  indicates that a child object is letting go...
 */
KDatabase *KDatabaseAttach ( const KDatabase *self );
rc_t KDatabaseSever ( const KDatabase *self );

// write side only public API
// KCreateMode KDatabaseGetCmode ( const KDatabase *self);
// KCreateMode KDatabaseSetCmode ( KDatabase *self, KCreateMode new_val);

// KChecksum KDatabaseGetChecksum ( const KDatabase *self);
// KChecksum KDatabaseSetChecksum ( KDatabase *self, KChecksum new_val);

// KDB_EXTERN rc_t CC KDBManagerCreateDB ( struct KDBManager *self, KDatabase **db, KCreateMode cmode, const char *path, ... );
// KDB_EXTERN rc_t CC KDatabaseCreateDB ( KDatabase *self, KDatabase **db, KCreateMode cmode, const char *name, ... );

// KDB_EXTERN rc_t CC KDBManagerVCreateDB ( struct KDBManager *self, KDatabase **db, KCreateMode cmode, const char *path, va_list args );
// KDB_EXTERN rc_t CC KDatabaseVCreateDB ( KDatabase *self, KDatabase **db, KCreateMode cmode, const char *name, va_list args );

// KDB_EXTERN rc_t CC KDBManagerOpenDBUpdate ( struct KDBManager *self, KDatabase **db, const char *path, ... );
// KDB_EXTERN rc_t CC KDatabaseOpenDBUpdate ( KDatabase *self, KDatabase **db, const char *name, ... );

// KDB_EXTERN rc_t CC KDBManagerVOpenDBUpdate ( struct KDBManager *self, KDatabase **db, const char *path, va_list args );
// KDB_EXTERN rc_t CC KDatabaseVOpenDBUpdate ( KDatabase *self, KDatabase **db, const char *name, va_list args );

// KDB_EXTERN rc_t CC KDatabaseLock ( KDatabase *self, uint32_t type, const char *name, ... );
// KDB_EXTERN rc_t CC KDatabaseVLock ( KDatabase *self, uint32_t type, const char *name, va_list args );
// KDB_EXTERN rc_t CC KDatabaseUnlock ( KDatabase *self, uint32_t type, const char *name, ... );
// KDB_EXTERN rc_t CC KDatabaseVUnlock ( KDatabase *self, uint32_t type, const char *name, va_list args );

// KDB_EXTERN rc_t CC KDatabaseRenameDB ( KDatabase *self, bool force, const char *from, const char *to );
// KDB_EXTERN rc_t CC KDatabaseRenameTable ( KDatabase *self, bool force, const char *from, const char *to );
// KDB_EXTERN rc_t CC KDatabaseRenameIndex ( KDatabase *self, bool force, const char *from, const char *to );

// KDB_EXTERN rc_t CC KDatabaseAliasDB ( KDatabase *self, const char *obj, const char *alias );
// KDB_EXTERN rc_t CC KDatabaseAliasTable ( KDatabase *self, const char *obj, const char *alias );
// KDB_EXTERN rc_t CC KDatabaseAliasIndex ( KDatabase *self, const char *obj, const char *alias );

// KDB_EXTERN rc_t CC KDatabaseDropDB ( KDatabase *self, const char *name, ... );
// KDB_EXTERN rc_t CC KDatabaseDropTable ( KDatabase *self, const char *name, ... );
// KDB_EXTERN rc_t CC KDatabaseDropIndex ( KDatabase *self, const char *name, ... );

// KDB_EXTERN rc_t CC KDatabaseVDropDB ( KDatabase *self, const char *name, va_list args );
// KDB_EXTERN rc_t CC KDatabaseVDropTable ( KDatabase *self, const char *name, va_list args );
// KDB_EXTERN rc_t CC KDatabaseVDropIndex ( KDatabase *self, const char *name, va_list args );

// KDB_EXTERN rc_t CC KDatabaseOpenManagerUpdate ( KDatabase *self, struct KDBManager **mgr );

// KDB_EXTERN rc_t CC KDatabaseOpenParentUpdate ( KDatabase *self, KDatabase **par );

// KDB_EXTERN rc_t CC KDatabaseMetaCopy ( KDatabase *self, const KDatabase *src,
//                                        const char * node_path, const char * tbl_name,
//                                        bool src_node_has_to_exist );

#ifdef __cplusplus
}
#endif

#endif /* _h_database_priv_ */
