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

#include <kfs/directory.h>

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

struct KDBManager;
struct KDatabase;
struct KColumn;
struct KMetadata;
struct KIndex;
struct KNamelist;

/*--------------------------------------------------------------------------
 * KTableBase, base structure for KTable implementations
 */
typedef struct KTable KTable;

typedef struct KTable_vt KTable_vt;
struct KTable_vt
{
    /* Public API */
    rc_t ( CC * whack )             ( KTable * self );
    rc_t ( CC * addRef )            ( const KTable * self );
    rc_t ( CC * release )           ( const KTable * self );
    bool ( CC * locked )            ( const KTable * self );
    bool ( CC * vExists )           ( const KTable * self, uint32_t type, const char *name, va_list args );
    bool ( CC * isAlias )           ( const KTable * self, uint32_t type, char *resolved, size_t rsize, const char *name );
    rc_t ( CC * vWritable )         ( const KTable * self, uint32_t type, const char *name, va_list args );
    rc_t ( CC * openManagerRead )   ( const KTable * self, struct KDBManager const **mgr );
    rc_t ( CC * openParentRead )    ( const KTable * self, struct KDatabase const **db );
    bool ( CC * hasRemoteData )     ( const KTable * self );
    rc_t ( CC * openDirectoryRead ) ( const KTable * self, const KDirectory **dir );
    rc_t ( CC * vOpenColumnRead )   ( const KTable * self, const struct KColumn **colp, const char *name, va_list args );
    rc_t ( CC * openMetadataRead )  ( const KTable * self, const struct KMetadata **meta );
    rc_t ( CC * vOpenIndexRead )    ( const KTable * self, const struct KIndex **idxp, const char *name, va_list args );
    rc_t ( CC * getPath )           ( const KTable * self, const char **path );
    rc_t ( CC * getName )           ( const KTable * self, char const **rslt);
    rc_t ( CC * listCol )           ( const KTable * self, struct KNamelist **names );
    rc_t ( CC * listIdx )           ( const KTable * self, struct KNamelist **names );
    rc_t ( CC * metaCompare )       ( const KTable * self, const KTable *other, const char * path, bool * equal );
};

// default implelentations where exist
extern rc_t KTableBaseWhack ( KTable *self );
extern rc_t CC KTableBaseAddRef ( const KTable *self );
extern rc_t CC KTableBaseRelease ( const KTable *self );

struct KTable
{
    const KTable_vt * vt;

    KRefcount refcount;
};

/* Attach
 * Sever
 *  like Release, except called internally
 *  indicates that a child object is letting go...
 */
extern KTable *KTableAttach ( const KTable *self );
extern rc_t KTableSever ( const KTable *self );

#ifdef __cplusplus
}
#endif
