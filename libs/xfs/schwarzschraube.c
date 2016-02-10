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

#include <klib/rc.h>
#include <klib/out.h>
#include <klib/text.h>
#include <klib/log.h>
#include <klib/printf.h>

#include <vfs/manager.h>
#include <kns/manager.h>
#include <kfs/directory.h>

#include "schwarzschraube.h"

#include <sysalloc.h>

 /*))))
   |||| That file contains unsoted methods
   ((((*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))  if Source is NULL, it will be passed to Destination
 ((*/
LIB_EXPORT
rc_t CC
XFS_StrDup ( const char * Src,  const char ** Dst )
{
    char ** TheDst;

    TheDst = ( char ** ) Dst;

    if ( TheDst == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( Src == NULL ) {
        * TheDst = NULL;

        return 0;
    }

    * TheDst = string_dup_measure ( Src, NULL );
    if ( * TheDst == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    return 0;
}   /* XFS_StrDup () */

/*))  if Source is NULL, it will be passed to Destination
 ((*/
LIB_EXPORT
rc_t CC
XFS_SStrDup ( const struct String * Src,  const char ** Dst )
{
    char ** TheDst = ( char ** ) Dst;

    if ( TheDst == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( Src == NULL ) {
        * TheDst = NULL;

        return 0;
    }

    if ( Src -> size == 0 ) {
        * TheDst = NULL;

        return 0;
    }

    * TheDst = string_dup ( Src -> addr, Src -> size );
    if ( * TheDst == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    return 0;
}   /* XFS_SStrDup () */

LIB_EXPORT
rc_t CC
XFS_StrEndsWith ( const char * Str, const char * End )
{
    uint32_t StrLen, EndLen;

    if ( Str == NULL || End == NULL ) {
        return false;
    }

    StrLen = string_len ( Str, string_size ( Str ) );
    EndLen = string_len ( End, string_size ( End ) );

    if ( StrLen >= EndLen && EndLen > 0 ) {
        return string_cmp (
                        Str + ( StrLen - EndLen ),
                        EndLen,
                        End,
                        EndLen,
                        EndLen
                        ) == 0;
    }

    return false;
}   /* XFS_StrEndsWith () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))   Sometime we need VFSManager, and it is very costly to
 |||    make it each time on demand ... so here is it
 |||    NOTE: Initializing and Destroying code arent thread safe!!!
 |||          will add mutexes if it really need to :)
(((*/

static struct VFSManager * _sVfsManager = NULL;

LIB_EXPORT
rc_t CC
XFS_VfsManagerInit ()
{
    rc_t RCt;
    struct VFSManager * Manager;

    RCt = 0;
    Manager = NULL;

    if ( _sVfsManager == NULL ) {
        RCt = VFSManagerMake ( & Manager );
        if ( RCt == 0 ) {
            if ( Manager == NULL ) {
                RCt = XFS_RC ( rcNull );
            }
            else {
                _sVfsManager = Manager;
            }
        }
    }

    return RCt;
}   /* XFS_VfsManagerInit () */

LIB_EXPORT
rc_t CC
XFS_VfsManagerDispose ()
{
    rc_t RCt;
    struct VFSManager * Manager;

    RCt = 0;
    Manager = _sVfsManager;

    if ( Manager != NULL ) {
        _sVfsManager = NULL;

        RCt = VFSManagerRelease ( Manager );
    }

    return RCt;
}   /* XFS_VfsManagerDispose () */

LIB_EXPORT
const struct VFSManager * CC
XFS_VfsManager ()
{
    return _sVfsManager;
}   /* XFS_VfsManager () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))   Sometime we need KNSManager, and it is very costly to
 |||    make it each time on demand ... so here is it
 |||    NOTE: Initializing and Destroying code arent thread safe!!!
 |||          will add mutexes if it really need to :)
(((*/
static struct KNSManager * _sKnsManager = NULL;

LIB_EXPORT
rc_t CC
XFS_KnsManagerInit ()
{
    rc_t RCt;
    struct KNSManager * Manager;

    RCt = 0;
    Manager = NULL;

    if ( _sKnsManager == NULL ) {
        RCt = KNSManagerMake ( & Manager );
        if ( RCt == 0 ) {
            if ( Manager == NULL ) {
                RCt = XFS_RC ( rcNull );
            }
            else {
                _sKnsManager = Manager;
            }
        }
    }

    return RCt;
}   /* XFS_KnsManagerInit () */

LIB_EXPORT
rc_t CC
XFS_KnsManagerDispose ()
{
    rc_t RCt;
    struct KNSManager * Manager;

    RCt = 0;
    Manager = _sKnsManager;

    if ( Manager != NULL ) {
        _sKnsManager = NULL;

        RCt = KNSManagerRelease ( Manager );
    }

    return RCt;
}   /* XFS_KnsManagerDispose () */

LIB_EXPORT
const struct KNSManager * CC
XFS_KnsManager ()
{
    return _sKnsManager;
}   /* XFS_KnsManager () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))   Simple resolving path Absolute/Relative
 |||    Sometimes it necessary
(((*/

LIB_EXPORT
rc_t CC
XFS_ResolvePath (
                bool Absolute,
                char * Resolved,
                size_t ResolvedSize,
                const char * Format,
                ...
)
{
    rc_t RCt;
    va_list Args;

    RCt = 0;

    XFS_CAN ( Format )

    va_start ( Args, Format );

    RCt = XFS_VResolvePath (
                            Absolute,
                            Resolved,
                            ResolvedSize,
                            Format,
                            Args
                            );

    va_end ( Args );

    return RCt;
}   /* XFS_ResolvePath () */

LIB_EXPORT
rc_t CC
XFS_VResolvePath (
                bool Absolute,
                char * Resolved,
                size_t ResolvedSize,
                const char * Format,
                va_list Args
)
{
    rc_t RCt;
    struct KDirectory * NatDir;

    RCt = 0;
    NatDir = NULL;

    XFS_CAN ( Resolved )
    XFS_CAN ( Format )
    XFS_CA ( ResolvedSize, 0 )

    RCt = KDirectoryNativeDir ( & NatDir );
    if ( RCt == 0 ) {
        RCt = KDirectoryVResolvePath (
                                    NatDir,
                                    Absolute,
                                    Resolved,
                                    ResolvedSize,
                                    Format,
                                    Args
                                    );
        KDirectoryRelease ( NatDir );
    }

    if ( RCt != 0 ) {
        * Resolved = 0;
    }

    return 0;
}   /* XFS_VResolvePath () */
