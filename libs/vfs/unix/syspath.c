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

#include <vfs/extern.h>

#include "path-priv.h"

#include <vfs/manager.h>
#include <klib/text.h>
#include <klib/refcount.h>
#include <klib/rc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sysalloc.h>


/*--------------------------------------------------------------------------
 * VFSManager
 */


/* MakeSysPath
 *  make a path object from an OS native filesystem path string
 *
 *  "new_path" [ OUT ] - return parameter for new path object
 *
 *  "sys_path" [ IN ] - a UTF-8 NUL-terminated string
 *  representing a native filesystem path
 *
 *  "wide_sys_path" [ IN ] - a wide NUL-terminated string
 *  representing a native filesystem path, where
 *  wchar_t is either USC-2 or UTF-32 depending upon libraries
 */
LIB_EXPORT rc_t CC VFSManagerMakeSysPath ( const VFSManager * self,
    VPath ** new_path, const char * sys_path )
{
    rc_t rc;

    if ( sys_path != NULL )
    {
        /* POSIX paths are our normal path type */
        return VFSManagerMakePath ( self, new_path, "%s", sys_path );
    }

    if ( new_path == NULL )
        rc = RC ( rcVFS, rcMgr, rcConstructing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVFS, rcMgr, rcConstructing, rcSelf, rcNull );
        else
            rc = RC ( rcVFS, rcMgr, rcConstructing, rcPath, rcNull );

        * new_path = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC VFSManagerWMakeSysPath ( const VFSManager * self,
    VPath ** new_path, const wchar_t * wide_sys_path )
{
    rc_t rc;

    if ( new_path == NULL )
        rc = RC ( rcVFS, rcMgr, rcConstructing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVFS, rcMgr, rcConstructing, rcSelf, rcNull );
        else if ( wide_sys_path == NULL )
            rc = RC ( rcVFS, rcMgr, rcConstructing, rcPath, rcNull );
        else
        {
            size_t src_size, dst_size;
            uint32_t len = wchar_cvt_string_measure ( wide_sys_path, & src_size, & dst_size );
            if ( len == 0 )
                rc = RC ( rcVFS, rcMgr, rcConstructing, rcPath, rcEmpty );
            else
            {
                /* transform to UTF-8 */
                size_t copy_size;
                char utf8_path [ 4096 ], *dst = utf8_path;
                if ( dst_size < sizeof utf8_path )
                    dst_size = sizeof utf8_path;
                else
                {
                    dst = malloc ( ++ dst_size );
                    if ( dst == NULL )
                        rc = RC ( rcVFS, rcMgr, rcConstructing, rcMemory, rcExhausted );
                }

                copy_size = wchar_cvt_string_copy ( dst, dst_size, wide_sys_path, src_size );
                if ( copy_size >= dst_size )
                    rc = RC ( rcVFS, rcMgr, rcConstructing, rcBuffer, rcInsufficient );
                else
                {
                    dst [ copy_size ] = 0;
                    rc = VFSManagerMakePath ( self, new_path, "%s", dst );
                }

                if ( dst != utf8_path )
                    free ( dst );

                if ( rc == 0 )
                    return 0;
            }
        }

        * new_path = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC VPathReadSysPath ( const VPath * self,
    char * buffer, size_t buffer_size, size_t * num_read )
{
    return VPathReadPath ( self, buffer, buffer_size, num_read );
}

/* ==========================================
             HACK O' MATIC
 */

LIB_EXPORT rc_t LegacyVPathMakeSysPath ( VPath ** new_path, const char * sys_path )
{
    return LegacyVPathMake ( new_path, sys_path );
}
