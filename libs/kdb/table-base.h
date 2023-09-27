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

#include <kfs/directory.h>

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef KTABLE_IMPL
#define KTABLE_IMPL KTableBase
#endif

struct KDBManager;
struct KDatabase;
struct KTable;
struct KColumn;
struct KMetadata;
struct KIndex;
struct KNamelist;

/*--------------------------------------------------------------------------
 * KTableBase, base structure for KTable implementations
 */
typedef struct KTableBase KTableBase;

typedef struct KTableBase_vt KTableBase_vt;
struct KTableBase_vt
{
    /* Public API */
    rc_t ( CC * whack )             ( KTABLE_IMPL * self );
    rc_t ( CC * addRef )            ( const KTABLE_IMPL * self );
    rc_t ( CC * release )           ( const KTABLE_IMPL * self );
    bool ( CC * locked )            ( const KTABLE_IMPL * self );
    bool ( CC * vExists )           ( const KTABLE_IMPL * self, uint32_t type, const char *name, va_list args );
    bool ( CC * isAlias )           ( const KTABLE_IMPL * self, uint32_t type, char *resolved, size_t rsize, const char *name );
    rc_t ( CC * vWritable )         ( const KTABLE_IMPL * self, uint32_t type, const char *name, va_list args );
    rc_t ( CC * openManagerRead )   ( const KTABLE_IMPL * self, struct KDBManager const **mgr );
    rc_t ( CC * openParentRead )    ( const KTABLE_IMPL * self, struct KDatabase const **db );
    bool ( CC * hasRemoteData )     ( const KTABLE_IMPL * self );
    rc_t ( CC * openDirectoryRead ) ( const KTABLE_IMPL * self, const KDirectory **dir );
    rc_t ( CC * vOpenColumnRead )   ( const KTABLE_IMPL * self, const struct KColumn **colp, const char *name, va_list args );
    rc_t ( CC * openMetadataRead )  ( const KTABLE_IMPL * self, const struct KMetadata **meta );
    rc_t ( CC * vOpenIndexRead )    ( const KTABLE_IMPL * self, const struct KIndex **idxp, const char *name, va_list args );
    rc_t ( CC * getPath )           ( const KTABLE_IMPL * self, const char **path );
    rc_t ( CC * getName )           ( const KTABLE_IMPL * self, char const **rslt);
    rc_t ( CC * listCol )           ( const KTABLE_IMPL * self, struct KNamelist **names );
    rc_t ( CC * listIdx )           ( const KTABLE_IMPL * self, struct KNamelist **names );
    rc_t ( CC * metaCompare )       ( const KTABLE_IMPL * self, const struct KTable *other, const char * path, bool * equal );
};

//TODO: write-side only; decide how to handle
#if 0
KDB_EXTERN rc_t CC KTableLock ( KTABLE_IMPL *self, uint32_t type,
    const char *name, ... );
KDB_EXTERN rc_t CC KTableVLock ( KTABLE_IMPL *self, uint32_t type,
    const char *name, va_list args );

KDB_EXTERN rc_t CC KTableUnlock ( KTABLE_IMPL *self, uint32_t type,
    const char *name, ... );
KDB_EXTERN rc_t CC KTableVUnlock ( KTABLE_IMPL *self, uint32_t type,
    const char *name, va_list args );

KDB_EXTERN rc_t CC KTableRenameColumn ( KTABLE_IMPL *self, bool force,
    const char *from, const char *to );
KDB_EXTERN rc_t CC KTableRenameIndex ( KTABLE_IMPL *self, bool force,
    const char *from, const char *to );

KDB_EXTERN rc_t CC KTableCopyColumn ( KTABLE_IMPL *self, KTable const *source,
    const char *name );
KDB_EXTERN rc_t CC KTableCopyIndex ( KTABLE_IMPL *self, KTable const *source,
    const char *name );

KDB_EXTERN rc_t CC KTableAliasColumn ( KTABLE_IMPL *self, const char *path, const char *alias );
KDB_EXTERN rc_t CC KTableAliasIndex ( KTABLE_IMPL *self, const char *name, const char *alias );

KDB_EXTERN rc_t CC KTableDropColumn ( KTABLE_IMPL *self, const char *name, ... );
KDB_EXTERN rc_t CC KTableDropIndex ( KTABLE_IMPL *self, const char *name, ... );

KDB_EXTERN rc_t CC KTableVDropColumn ( KTABLE_IMPL *self, const char *name, va_list args );
KDB_EXTERN rc_t CC KTableVDropIndex ( KTABLE_IMPL *self, const char *name, va_list args );

KDB_EXTERN rc_t CC KTableReindex ( KTABLE_IMPL *self );

KDB_EXTERN rc_t CC KTableOpenManagerUpdate ( KTABLE_IMPL *self, struct KDBManager **mgr );

KDB_EXTERN rc_t CC KTableOpenParentUpdate ( KTABLE_IMPL *self, struct KDatabase **db );

KDB_EXTERN rc_t CC KTableCreateIndex ( struct KTable *self, KIndex **idx, KIdxType type, KCreateMode cmode, const char *name, ... );
KDB_EXTERN rc_t CC KTableVCreateIndex ( struct KTable *self, KIndex **idx, KIdxType type, KCreateMode cmode, const char *name, va_list args );
KDB_EXTERN rc_t CC KTableOpenIndexUpdate ( struct KTable *self, KIndex **idx, const char *name, ... );
KDB_EXTERN rc_t CC KTableVOpenIndexUpdate ( struct KTable *self, KIndex **idx, const char *name, va_list args );

#endif

// default implelentations where exist
extern rc_t KTableBaseWhack ( KTABLE_IMPL *self );
extern rc_t CC KTableBaseAddRef ( const KTABLE_IMPL *self );
extern rc_t CC KTableBaseRelease ( const KTABLE_IMPL *self );

struct KTableBase
{
    const KTableBase_vt * vt;

    KRefcount refcount;
};

/* Attach
 * Sever
 *  like Release, except called internally
 *  indicates that a child object is letting go...
 */
extern KTABLE_IMPL *KTableAttach ( const KTABLE_IMPL *self );
extern rc_t KTableSever ( const KTABLE_IMPL *self );

#ifdef __cplusplus
}
#endif
