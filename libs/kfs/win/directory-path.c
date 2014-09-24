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

#include "sysdir-priv.h"

#include <kfs/impl.h> /* KSysDir */
#include <kfs/directory.h>
#include <kfs/kfs-priv.h> /* KSysDirOSPath */

#include <klib/rc.h>

#include <wchar.h>

/* PosixStringToSystemString
 * converts posix path string to system path
 *  "buffer" [ OUT ] - NUL terminated system path string
 *  "bsize" [ IN ] - buffer length
 *  "path" [ IN ] - NUL terminated posix path string
 */
LIB_EXPORT rc_t CC KDirectoryPosixStringToSystemString(const KDirectory *self,
    char *buffer,
    size_t bsize,
    const char *path,
    ...)
{
    rc_t rc = 0;

    if (self == NULL) {
        return RC(rcFS, rcDirectory, rcAccessing, rcSelf, rcNull);
    }

    if (buffer == NULL) {
        return RC(rcFS, rcDirectory, rcAccessing, rcBuffer, rcNull);
    }

    if (bsize == 0) {
        return RC(rcFS, rcDirectory, rcAccessing, rcBuffer, rcInsufficient);
    }

    {
        struct KSysDir *sysDir = KDirectoryGetSysDir(self);
        wchar_t wd_path[MAX_PATH];

        va_list args;
        va_start(args, path);
        rc = KSysDirOSPath(sysDir, wd_path, MAX_PATH, path, args);
        va_end(args);

        if (rc == 0) {
            size_t ret = wcstombs(buffer, wd_path, bsize);
            if (ret >= MAX_PATH) {
                return RC(rcExe, rcPath, rcConverting, rcPath, rcExcessive);
            }
        }
    }

    return rc;
}
