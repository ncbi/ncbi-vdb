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

#include <kfs/extern.h>
#include <kfs/lockfile.h>
#include <kfs/directory.h>
#include <klib/rc.h>


/*--------------------------------------------------------------------------
 * KDirectory
 *  interface extensions
 */


/* CreateExclusiveAccessFile
 *  opens a file with exclusive write access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "update" [ IN ] - if true, open in read/write mode
 *  otherwise, open in write-only mode
 *
 *  "access" [ IN ] - standard Unix access mode, e.g. 0664
 *
 *  "mode" [ IN ] - a creation mode ( see explanation above ).
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
LIB_EXPORT rc_t CC KDirectoryVCreateExclusiveAccessFile ( KDirectory *self, struct KFile **f,
    bool update, uint32_t access, KCreateMode mode, const char *path, va_list args )
{
    /* Windows should naturally behave as we want here... */
    rc_t rc = KDirectoryVCreateFile ( self, f, update, access, mode, path, args );
    if ( rc != 0 )
    {
        if ( GetRCState ( rc ) == rcExists )
            rc = RC ( rcFS, rcFile, rcLocking, rcLocking, rcBusy );
        else
            rc = ResetRCContext ( rc, rcFS, rcFile, rcLocking );
    }
    return rc;
}
