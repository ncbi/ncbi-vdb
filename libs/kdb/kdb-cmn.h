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

#include <kdb/manager.h>

#ifdef __cplusplus
extern "C" {
#endif

struct KDirectory;
struct KDBManager;

/*
 * This symbol is inserted where the KDB is being tweaked to allow
 * VFS URI syntax in opening KDB database objects initially to support
 * krypto passwords more fully.  By specifying the password source
 * individually for opens two KDB objects with different passwords can be opened.
 */
#define SUPPORT_VFS_URI 1

/*--------------------------------------------------------------------------
 * KDB utility
 */

struct KDBContents;
rc_t KDBVGetPathContents(KDBContents const **result, const struct KDirectory *dir, KPathType type, char const *path, va_list args);
rc_t KDBGetPathContents(KDBContents const **result, const struct KDirectory *dir, KPathType type, char const *path, ...);

/* PathType
 *  checks type of path
 */
int KDBPathTypeDir ( const struct KDirectory *dir, int type,bool *zombies, const char *path );
int KDBPathType ( const struct KDirectory *dir, bool *zombies, const char *path );

/* GetObjModDate
 *  extract mod date from a path
 */
rc_t KDBGetObjModDate ( const struct KDirectory *dir, KTime_t *mtime );

/* GetPathModDate
 *  extract mod date from a path
 */
rc_t KDBVGetPathModDate ( const struct KDirectory *dir,
    KTime_t *mtime, const char *path, va_list args );

/* KDBMakeSubPath
 *  adds a namespace to path spec
 */
rc_t KDBMakeSubPath ( struct KDirectory const *dir,
    char *subpath, size_t subpath_max, const char *ns,
    uint32_t ns_size, const char *path, ... );
/* VMakeSubPath
 *  adds a namespace to path spec
 */
rc_t KDBVMakeSubPath ( const struct KDirectory *dir,
    char *subpath, size_t subpath_max, const char *ns,
    uint32_t ns_size, const char *path, va_list args );

#ifdef __cplusplus
}
#endif

