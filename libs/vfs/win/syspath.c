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

#include <vfs/manager.h>
#include <vfs/path.h>
#include <klib/defs.h>
#include <klib/rc.h>
#include <klib/log.h>
#include <klib/out.h>
#include <klib/debug.h>

#include "../path-priv.h"

#include <wchar.h>
#include <windows.h>
#include <direct.h>

#include <sysalloc.h>

#include <os-native.h>
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
    rc_t rc = 0;

    /* this is all incorrect - the only reason we STILL use
       wchar_t in Windows is to guarantee UNICODE. most of the
       time, we'll have UTF-8, which is okay. but on older systems,
       it could still be something ancient. should use OS
       to convert from system path to UCS-2. */

    if ( new_path == NULL )
        rc = RC ( rcVFS, rcMgr, rcConstructing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVFS, rcMgr, rcConstructing, rcSelf, rcNull );
        else if ( sys_path == NULL )
            rc = RC ( rcVFS, rcMgr, rcConstructing, rcPath, rcNull );
        else
        {
            size_t src_size = string_size ( sys_path );

            /* transform to wchar_t */
            wchar_t wchar_path [ 4096 ], * dst = wchar_path;
            size_t copy_size, dst_size = src_size * sizeof wchar_path [ 0 ];
            if ( dst_size < sizeof wchar_path )
                dst_size = sizeof wchar_path;
            else
            {
                dst = malloc ( dst_size += sizeof * dst );
                if ( dst == NULL )
                    rc = RC ( rcVFS, rcMgr, rcConstructing, rcMemory, rcExhausted );
            }

            if ( rc == 0 )
            {
                /* we need to call windows to do the conversion, because sys_path can
                   be ascii or multi-byte */
                copy_size = MultiByteToWideChar ( CP_THREAD_ACP, MB_PRECOMPOSED,
                    sys_path, ( int ) src_size, dst,
                    ( int ) ( dst_size / sizeof dst [ 0 ] ) - 1 );
                if ( copy_size == 0 )
                {
                    DWORD status = GetLastError ();
                    DBGMSG ( DBG_VFS, DBG_FLAG_ANY, ( "MultiByteToWideChar: error code - %!.\n", status ) );
                    rc = RC ( rcVFS, rcMgr, rcConstructing, rcPath, rcInvalid );
                }
                else
                {
                    dst [ copy_size ] = 0;
                    rc = VFSManagerWMakeSysPath ( self, new_path, dst );
                }

                if ( dst != wchar_path )
                    free ( dst );

                if ( rc == 0 )
                    return 0;

            }
        }

        * new_path = NULL;
    }

    return rc;
}


static
rc_t transform_to_utf8_and_make_vpath ( const VFSManager * self,
    VPath ** new_path, const wchar_t * src, bool dos_path )
{
    rc_t rc = 0;
    size_t src_size, dst_size;
    uint32_t len = wchar_cvt_string_measure ( src, & src_size, & dst_size );
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
                return RC ( rcVFS, rcMgr, rcConstructing, rcMemory, rcExhausted );
        }

        copy_size = wchar_cvt_string_copy ( dst, dst_size, src, src_size );
        if ( copy_size >= dst_size )
            rc = RC ( rcVFS, rcMgr, rcConstructing, rcBuffer, rcInsufficient );
        else
        {
            size_t i = 0;

            dst [ copy_size ] = 0;

            /* encode drive letter */
            if ( dos_path )
            {
                assert ( isalpha ( dst [ 0 ] ) );
                assert ( dst [ 1 ] == ':' );
                dst [ 1 ] = dst [ 0 ];
                dst [ 0 ] = '/';
                i = 2;
            }

            /* convert '\\' to '/' */
            for ( ; i < copy_size; ++ i )
            {
                if ( dst [ i ] == '\\' )
                    dst [ i ] = '/';
            }

            /* this is the final goal! */
            rc = VFSManagerMakePath ( self, new_path, "%.*s", ( uint32_t ) copy_size, dst );
        }

        if ( dst != utf8_path )
            free ( dst );
    }
    return rc;
}


static
rc_t make_absolute_and_transform_to_utf8_and_make_vpath ( const VFSManager * self,
    VPath ** new_path, const wchar_t * src, bool have_drive )
{
    rc_t rc;
    wchar_t full [ 4096 ];

    /* expand to full path - this is temporary, and will be replaced after KFS is updated */
    DWORD len = GetFullPathNameW ( src, sizeof full / sizeof full [ 0 ], full, NULL );
    if ( len == 0 )
    {
        /* we have an error */
        DWORD status = GetLastError ();
        DBGMSG ( DBG_VFS, DBG_FLAG_ANY, ( "GetFullPathNameW: error code - %u.\n", status ) );
        rc = RC ( rcVFS, rcMgr, rcConstructing, rcPath, rcInvalid );
    }
    else if ( len >= sizeof full / sizeof full [ 0 ] )
    {
        /* the buffer is too small ! */
        wchar_t * big_buf = malloc( ( ++len ) * ( sizeof full[ 0 ] ) );
        if ( big_buf == NULL )
            rc = RC ( rcVFS, rcMgr, rcConstructing, rcMemory, rcExhausted );
        else
        {
            DWORD len2 = GetFullPathNameW ( src, len, big_buf, NULL );
            if ( len2 == 0 )
            {
                /* we have an error */
                DWORD status = GetLastError ();
                DBGMSG ( DBG_VFS, DBG_FLAG_ANY, ( "GetFullPathNameW: error code - %u.\n", status ) );
                rc = RC ( rcVFS, rcMgr, rcConstructing, rcPath, rcInvalid );
            }
            else if ( len2 >= len )
            {
                DBGMSG ( DBG_VFS, DBG_FLAG_ANY, ( "GetFullPathNameW: buffer too small again - %u.\n", len2 ) );
                rc = RC ( rcVFS, rcMgr, rcConstructing, rcPath, rcInvalid );
            }
            else
            {
                /* now we can call the final transform and make */
                rc = transform_to_utf8_and_make_vpath( self, new_path, big_buf, true );
            }
            free( big_buf );
        }
    }
    else
    {
        /* now we can call the final transform and make */
        rc = transform_to_utf8_and_make_vpath( self, new_path, full, true );
    }
    return rc;
}

static
bool could_be_url ( const wchar_t * wide_sys_path )
{
    uint32_t i;
    for ( i = 0; wide_sys_path [ i ] != 0; ++ i )
    {
        if ( wide_sys_path [ i ] == ':' )
        {
            if ( i < 3 || i > 16 )
                break;
            for ( ++ i; wide_sys_path [ i ] != 0; ++ i )
            {
                if ( wide_sys_path [ i ] == '\\' )
                    return false;
            }
            return true;
        }

        if ( wide_sys_path [ i ] == '\\' )
            break;
    }
    return false;
}

static
bool could_be_accession ( const wchar_t * wide_sys_path )
{
    uint32_t i;
    for ( i = 0; wide_sys_path [ i ] != 0; ++ i )
    {
        switch ( wide_sys_path [ i ] )
        {
        case '/':
        case '\\':
            return false;
        }
    }
    return true;
}


LIB_EXPORT rc_t CC VFSManagerWMakeSysPath ( const VFSManager * self,
    VPath ** new_path, const wchar_t * wide_sys_path )
{
    rc_t rc = 0;

    /* what makes Windows paths exciting is that they
       have these drive letters, or they can be UNC,
       but they can be relative to the current drive,
       and they can use back-slashes because they're
       easy to type, or they can use forward-slashes,
       whatever. */

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
            /* test for UNC path */
            if ( ( wide_sys_path [ 0 ] == '\\' && wide_sys_path [ 1 ] == '\\' ) ||
                 ( wide_sys_path [ 0 ] == '/'  && wide_sys_path [ 1 ] == '/'  ) )
            {
                /* it is a UNC-path.
                   reject IO or device namespaces */
                if ( ( wide_sys_path [ 2 ] ==  '?' || wide_sys_path [ 2 ] == '.' ) &&
                     ( wide_sys_path [ 3 ] == '\\' || wide_sys_path [ 3 ] == '/' ) )
                    return RC ( rcVFS, rcPath, rcConstructing, rcPath, rcIncorrect );

                /* produce the VPath-instance from the passed in string */
                rc = transform_to_utf8_and_make_vpath ( self, new_path,  wide_sys_path, false );
            }
            else
            {
                /* it is not a UNC-path.
                   test for drive letter */
                if ( iswalpha ( wide_sys_path [ 0 ] ) && wide_sys_path [ 1 ] == ':' )
                {
                    /* drive letter detected.
                       test for absolute path */
                    if ( wide_sys_path [ 2 ] != '\\' && wide_sys_path [ 2 ] != '/' )
                    {
                        /* drive-relative path.
                           we have make an absolute path first */
                        rc = make_absolute_and_transform_to_utf8_and_make_vpath ( self, new_path, wide_sys_path, true );
                    }
                    else
                    {
                        /* already absolute path.
                           produce the VPath-instance from the passed in string */
                        rc = transform_to_utf8_and_make_vpath ( self, new_path,  wide_sys_path, true );
                    }
                }
                /* test for URI syntax */
                else if ( could_be_url ( wide_sys_path ) )
                    rc = transform_to_utf8_and_make_vpath ( self, new_path,  wide_sys_path, false );
                else if ( could_be_accession ( wide_sys_path ) )
                    rc = transform_to_utf8_and_make_vpath ( self, new_path,  wide_sys_path, false );
                else
                {
                    /* no drive letter detected, we have make a absolute path first */
                    rc = make_absolute_and_transform_to_utf8_and_make_vpath ( self, new_path, wide_sys_path, false );
                }
            }
        }
    }
    return rc;
}

static
void convert_fwd_to_rev_slashes ( char * buffer, size_t bytes )
{
    size_t i;
    for ( i = 0; i < bytes; ++ i )
    {
        if ( buffer [ i ] == '/' )
            buffer [ i ] = '\\';
    }
}

LIB_EXPORT rc_t CC VPathReadSysPath ( const VPath * self,
    char * buffer, size_t buffer_size, size_t * num_read )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcVFS, rcPath, rcReading, rcSelf, rcNull );
    else if ( buffer== NULL && buffer_size != 0 )
        rc = RC ( rcVFS, rcPath, rcReading, rcParam, rcNull );
    else
    {
        size_t dummy, effective_size = buffer_size;
        if ( num_read == NULL )
        {
            num_read = & dummy;
            if ( buffer_size != 0 )
                -- effective_size;
        }

        rc = 0;

        switch ( self -> path_type )
        {
        case vpInvalid:
            rc = RC ( rcVFS, rcPath, rcReading, rcPath, rcInvalid );
            break;
        case vpOID:
        case vpAccession:
        case vpNameOrOID:
        case vpNameOrAccession:
        case vpName:
            /* copy as simple name */
            if ( effective_size < self -> path . size )
                rc = RC ( rcVFS, rcPath, rcReading, rcBuffer, rcInsufficient );
            else
                * num_read = string_copy ( buffer, buffer_size, self -> path . addr, self -> path . size );
            break;
        case vpRelPath:
        case vpUNCPath:
            /* copy and transform slashes */
            if ( effective_size < self -> path . size )
                rc = RC ( rcVFS, rcPath, rcReading, rcBuffer, rcInsufficient );
            else
            {
                * num_read = string_copy ( buffer, buffer_size, self -> path . addr, self -> path . size );
                convert_fwd_to_rev_slashes ( buffer, * num_read );
            }
            break;
        case vpFullPath:
            /* convert to full path */
            if ( effective_size < self -> path . size )
                rc = RC ( rcVFS, rcPath, rcReading, rcBuffer, rcInsufficient );
            else
            {
                * num_read = string_copy ( buffer, buffer_size, self -> path . addr, self -> path . size );
                if ( self -> scheme_type == vpuri_none ||
                    self -> scheme_type == vpuri_ncbi_file ||
                    self -> scheme_type == vpuri_file )
                {
                    assert ( * num_read >= 2 );
                    buffer [ 0 ] = buffer [ 1 ];
                    buffer [ 1 ] = ':';
                    convert_fwd_to_rev_slashes ( buffer, * num_read );
                }
            }
            break;
        case vpAuth:
        case vpHostName:
        case vpEndpoint:
        default:
            rc = RC ( rcVFS, rcPath, rcReading, rcPath, rcIncorrect );
        }
    }

    if ( rc != 0 )
    {
        if ( num_read != NULL )
            * num_read = 0;
        if ( buffer != NULL && buffer_size != 0 )
            buffer [ 0 ] = 0;
    }

    return rc;
}



/* ==========================================
             HACK O' MATIC
 */

LIB_EXPORT rc_t LegacyVPathMakeSysPath ( VPath ** new_path, const char * sys_path )
{
    VFSManager * vfs;
    rc_t rc = VFSManagerMake ( & vfs );
    if ( rc == 0 )
    {
        rc = VFSManagerMakeSysPath ( vfs, new_path, sys_path );
        VFSManagerRelease ( vfs );
    }
    return rc;
}
