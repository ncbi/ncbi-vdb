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

#ifndef _h_klib_refcount_
#include <klib/refcount.h>
#endif

#ifndef KDATABASE_IMPL
#define KDATABASE_IMPL KDatabaseBase
#endif

#include <kfs/directory.h>

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
struct KDatabase;
struct KDBManager;
struct KTable;

/*--------------------------------------------------------------------------
 * KDatabaseBase
 *   base structure for KTable implementations
 */
typedef struct KDatabaseBase KDatabaseBase;

typedef struct KDatabase_vt KDatabase_vt;
struct KDatabase_vt
{
    /* Public API */
    rc_t ( CC * whack )             ( KDATABASE_IMPL *self );
    rc_t ( CC * addRef )            ( const KDATABASE_IMPL *self );
    rc_t ( CC * release )           ( const KDATABASE_IMPL *self );
    bool ( CC * locked )            ( const KDATABASE_IMPL *self );
    bool ( CC * vExists )           ( const KDATABASE_IMPL *self, uint32_t type, const char *name, va_list args );
    bool ( CC * isAlias )           ( const KDATABASE_IMPL *self, uint32_t type, char *resolved, size_t rsize, const char *name );
    rc_t ( CC * vWritable )         ( const KDATABASE_IMPL *self, uint32_t type, const char *name, va_list args );
    rc_t ( CC * openManagerRead )   ( const KDATABASE_IMPL *self, struct KDBManager const **mgr );
    rc_t ( CC * openParentRead )    ( const KDATABASE_IMPL *self, const struct KDatabase **par );
    rc_t ( CC * openDirectoryRead ) ( struct KDatabase const *self, struct KDirectory const **dir );
    rc_t ( CC * vOpenDBRead )       ( const KDATABASE_IMPL *self, const struct KDatabase **db, const char *name, va_list args );
    rc_t ( CC * vOpenTableRead )    ( const KDATABASE_IMPL *self, const struct KTable **tblp, const char *name, va_list args )    ;
};

struct KDatabaseBase
{
    KDatabase_vt * vt;

    KRefcount refcount;
};

// default implelentations where exist
extern rc_t CC KDatabaseBaseBaseWhack ( KDATABASE_IMPL *self );
extern rc_t CC KDatabaseBaseBaseAddRef ( const KDATABASE_IMPL *self );
extern rc_t CC KDatabaseBaseBaseRelease ( const KDATABASE_IMPL *self );

/* Attach
 * Sever
 *  like AddRef/Release, except called internally
 *  indicates that a child object is letting go...
 */
KDATABASE_IMPL *KDatabaseAttach ( const KDATABASE_IMPL *self );
rc_t KDatabaseSever ( const KDATABASE_IMPL *self );

// write side only public API
// KCreateMode KDatabaseGetCmode ( const KDATABASE_IMPL *self);
// KCreateMode KDatabaseSetCmode ( KDATABASE_IMPL *self, KCreateMode new_val);

// KChecksum KDatabaseGetChecksum ( const KDATABASE_IMPL *self);
// KChecksum KDatabaseSetChecksum ( KDATABASE_IMPL *self, KChecksum new_val);

// KDB_EXTERN rc_t CC KDBManagerCreateDB ( struct KDBManager *self, KDATABASE_IMPL **db, KCreateMode cmode, const char *path, ... );
// KDB_EXTERN rc_t CC KDatabaseCreateDB ( KDATABASE_IMPL *self, KDatabase **db, KCreateMode cmode, const char *name, ... );

// KDB_EXTERN rc_t CC KDBManagerVCreateDB ( struct KDBManager *self, KDATABASE_IMPL **db, KCreateMode cmode, const char *path, va_list args );
// KDB_EXTERN rc_t CC KDatabaseVCreateDB ( KDATABASE_IMPL *self, KDatabase **db, KCreateMode cmode, const char *name, va_list args );

// KDB_EXTERN rc_t CC KDBManagerOpenDBUpdate ( struct KDBManager *self, KDATABASE_IMPL **db, const char *path, ... );
// KDB_EXTERN rc_t CC KDatabaseOpenDBUpdate ( KDATABASE_IMPL *self, KDatabase **db, const char *name, ... );

// KDB_EXTERN rc_t CC KDBManagerVOpenDBUpdate ( struct KDBManager *self, KDATABASE_IMPL **db, const char *path, va_list args );
// KDB_EXTERN rc_t CC KDatabaseVOpenDBUpdate ( KDATABASE_IMPL *self, KDatabase **db, const char *name, va_list args );

// KDB_EXTERN rc_t CC KDatabaseLock ( KDATABASE_IMPL *self, uint32_t type, const char *name, ... );
// KDB_EXTERN rc_t CC KDatabaseVLock ( KDATABASE_IMPL *self, uint32_t type, const char *name, va_list args );
// KDB_EXTERN rc_t CC KDatabaseUnlock ( KDATABASE_IMPL *self, uint32_t type, const char *name, ... );
// KDB_EXTERN rc_t CC KDatabaseVUnlock ( KDATABASE_IMPL *self, uint32_t type, const char *name, va_list args );

// KDB_EXTERN rc_t CC KDatabaseRenameDB ( KDATABASE_IMPL *self, bool force, const char *from, const char *to );
// KDB_EXTERN rc_t CC KDatabaseRenameTable ( KDATABASE_IMPL *self, bool force, const char *from, const char *to );
// KDB_EXTERN rc_t CC KDatabaseRenameIndex ( KDATABASE_IMPL *self, bool force, const char *from, const char *to );

// KDB_EXTERN rc_t CC KDatabaseAliasDB ( KDATABASE_IMPL *self, const char *obj, const char *alias );
// KDB_EXTERN rc_t CC KDatabaseAliasTable ( KDATABASE_IMPL *self, const char *obj, const char *alias );
// KDB_EXTERN rc_t CC KDatabaseAliasIndex ( KDATABASE_IMPL *self, const char *obj, const char *alias );

// KDB_EXTERN rc_t CC KDatabaseDropDB ( KDATABASE_IMPL *self, const char *name, ... );
// KDB_EXTERN rc_t CC KDatabaseDropTable ( KDATABASE_IMPL *self, const char *name, ... );
// KDB_EXTERN rc_t CC KDatabaseDropIndex ( KDATABASE_IMPL *self, const char *name, ... );

// KDB_EXTERN rc_t CC KDatabaseVDropDB ( KDATABASE_IMPL *self, const char *name, va_list args );
// KDB_EXTERN rc_t CC KDatabaseVDropTable ( KDATABASE_IMPL *self, const char *name, va_list args );
// KDB_EXTERN rc_t CC KDatabaseVDropIndex ( KDATABASE_IMPL *self, const char *name, va_list args );

// KDB_EXTERN rc_t CC KDatabaseOpenManagerUpdate ( KDATABASE_IMPL *self, struct KDBManager **mgr );

// KDB_EXTERN rc_t CC KDatabaseOpenParentUpdate ( KDATABASE_IMPL *self, KDatabase **par );

// KDB_EXTERN rc_t CC KDatabaseMetaCopy ( KDATABASE_IMPL *self, const KDatabase *src,
//                                        const char * node_path, const char * tbl_name,
//                                        bool src_node_has_to_exist );

#ifdef __cplusplus
}
#endif

