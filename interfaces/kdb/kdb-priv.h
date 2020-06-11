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

#ifndef _h_kdb_kdb_priv_
#define _h_kdb_kdb_priv_

#ifndef _h_kdb_extern_
#include <kdb/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#include <stdarg.h> /* va_list */

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KDBManager;
struct KDatabase;
struct KTable;
struct KIndex;
struct KColumn;
struct KMetadata;
struct KDirectory;
struct VFSManager;
struct VPath;


/*--------------------------------------------------------------------------
 * KDBManager
 */

/* ModDate
 *  return a modification timestamp for table
 */
KDB_EXTERN rc_t CC KDBManagerGetTableModDate ( struct KDBManager const *self,
    KTime_t *mtime, const char *path, ... );
KDB_EXTERN rc_t CC KDBManagerVGetTableModDate ( struct KDBManager const *self,
    KTime_t *mtime, const char *path, va_list args );


/* Make using custom VFSManager */
KDB_EXTERN rc_t CC KDBManagerMakeReadWithVFSManager (
    struct KDBManager const ** mgrp, struct KDirectory const * wd,
    struct VFSManager * vfs );
KDB_EXTERN rc_t CC KDBManagerMakeUpdateWithVFSManager (
    struct KDBManager ** mgrp, struct KDirectory *wd,
    struct VFSManager * vfs );

KDB_EXTERN rc_t CC KDBManagerGetVFSManager ( struct KDBManager const *self,
    struct VFSManager ** vfs );

KDB_EXTERN rc_t CC KDBManagerVPathOpenLocalDBRead ( struct KDBManager const * self,
    struct KDatabase const ** db, struct VPath const * path );
KDB_EXTERN rc_t CC KDBManagerVPathOpenRemoteDBRead ( struct KDBManager const * self,
    struct KDatabase const ** db, struct VPath const * remote, struct VPath const * cache );

/** Unreliable object: do not report occured erros */
KDB_EXTERN int CC KDBManagerVPathTypeUnreliable (
    const struct KDBManager * self, const char *object, va_list args );

/*--------------------------------------------------------------------------
 * KDatabase
 */

/* OpenDirectory
 *  access the directory in use
 */
KDB_EXTERN rc_t CC KDatabaseOpenDirectoryRead ( struct KDatabase const *self, struct KDirectory const **dir );
KDB_EXTERN rc_t CC KDatabaseOpenDirectoryUpdate ( struct KDatabase *self, struct KDirectory **dir );

/* GetPath
 *  return the absolute path to DB
 */
KDB_EXTERN rc_t CC KDatabaseGetPath ( struct KDatabase const *self,
    const char **path );


/*--------------------------------------------------------------------------
 * KTable
 */

/* OpenDirectory
 *  access the directory in use
 */
KDB_EXTERN rc_t CC KTableOpenDirectoryRead ( struct KTable const *self, struct KDirectory const **dir );
KDB_EXTERN rc_t CC KTableOpenDirectoryUpdate ( struct KTable *self, struct KDirectory **dir );

#define KTableGetDirectoryRead KTableOpenDirectoryRead
#define KTableGetDirectoryUpdate KTableOpenDirectoryUpdate

/* GetPath
 *  return the absolute path to table
 */
KDB_EXTERN rc_t CC KTableGetPath ( struct KTable const *self,
    const char **path );


/*--------------------------------------------------------------------------
 * KColumn
 */

/* OpenDirectory
 *  duplicate reference to the directory in use
 *  NB - returned reference must be released
 */
KDB_EXTERN rc_t CC KColumnOpenDirectoryRead ( struct KColumn const *self, struct KDirectory const **dir );
KDB_EXTERN rc_t CC KColumnOpenDirectoryUpdate ( struct KColumn *self, struct KDirectory **dir );

#define KColumnGetDirectoryRead KColumnOpenDirectoryRead
#define KColumnGetDirectoryUpdate KColumnOpenDirectoryUpdate



/*--------------------------------------------------------------------------
 * KIndex
 */

/* MarkModified
 *  make the index think it has been modified, such that it may be committed
 *  useful when forcing conversion from an older version
 */
KDB_EXTERN rc_t CC KIndexMarkModified ( struct KIndex *self );


/* SetMaxId
 *  certain legacy versions of skey were built to know only the starting id
 *  of the NAME_FMT column, but were never given a maximum id. allow them
 *  to be corrected here.
 */
KDB_EXTERN void CC KIndexSetMaxRowId ( struct KIndex const *self, int64_t max_row_id );


#ifdef __cplusplus
}
#endif

#endif /*  _h_kdb_kdb_priv_ */
