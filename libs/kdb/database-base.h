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

#include <klib/refcount.h>

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
struct KDBManager;
struct KTable;
struct KMetadata;
struct KIndex;

/*--------------------------------------------------------------------------
 * KDatabaseBase
 *   base structure for KDatabase implementations
 */
typedef struct KDatabaseBase KDatabaseBase;

typedef struct KDatabase_vt KDatabase_vt;
struct KDatabase_vt
{
    /* Public API */
    rc_t ( CC * whack )             ( KDATABASE_IMPL * self );
    rc_t ( CC * addRef )            ( const KDATABASE_IMPL * self );
    rc_t ( CC * release )           ( const KDATABASE_IMPL * self );
    bool ( CC * locked )            ( const KDATABASE_IMPL * self );
    bool ( CC * vExists )           ( const KDATABASE_IMPL * self, uint32_t type, const char *name, va_list args );
    bool ( CC * isAlias )           ( const KDATABASE_IMPL * self, uint32_t type, char *resolved, size_t rsize, const char *name );
    rc_t ( CC * vWritable )         ( const KDATABASE_IMPL * self, uint32_t type, const char *name, va_list args );
    rc_t ( CC * openManagerRead )   ( const KDATABASE_IMPL * self, struct KDBManager const **mgr );
    rc_t ( CC * openParentRead )    ( const KDATABASE_IMPL * self, const struct KDatabase **par );
    rc_t ( CC * openDirectoryRead ) ( const KDATABASE_IMPL * self, struct KDirectory const **dir );
    rc_t ( CC * vOpenDBRead )       ( const KDATABASE_IMPL * self, const struct KDatabase **db, const char *name, va_list args );
    rc_t ( CC * vOpenTableRead )    ( const KDATABASE_IMPL * self, const struct KTable **tblp, const char *name, va_list args )    ;
    rc_t ( CC * openMetadataRead )  ( const KDATABASE_IMPL * self, const struct KMetadata **meta );
    rc_t ( CC * vOpenIndexRead )    ( const KDATABASE_IMPL * self, const struct KIndex **idx, const char *name, va_list args );
    rc_t ( CC * listDB )            ( const KDATABASE_IMPL * self, struct KNamelist **names );
    rc_t ( CC * listTbl )           ( const KDATABASE_IMPL * self, struct KNamelist **names );
    rc_t ( CC * listIdx )           ( const KDATABASE_IMPL * self, struct KNamelist **names );
    rc_t ( CC * getPath )           ( const KDATABASE_IMPL * self, const char **path );
};

struct KDatabaseBase
{
    const KDatabase_vt * vt;

    KRefcount refcount;
};

// default implelentations where exist
extern rc_t CC KDatabaseBaseWhack ( KDATABASE_IMPL *self );
extern rc_t CC KDatabaseBaseAddRef ( const KDATABASE_IMPL *self );
extern rc_t CC KDatabaseBaseRelease ( const KDATABASE_IMPL *self );

/* Attach
 * Sever
 *  like AddRef/Release, except called internally
 *  indicates that a child object is letting go...
 */
KDATABASE_IMPL *KDatabaseAttach ( const KDATABASE_IMPL *self );
rc_t KDatabaseSever ( const KDATABASE_IMPL *self );

#ifdef __cplusplus
}
#endif

