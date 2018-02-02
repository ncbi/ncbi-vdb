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
#include <klib/namelist.h>
#include <klib/printf.h>

#include "teleport.h"
#include "zehr.h"
#include "schwarzschraube.h"
#include "hdict.h"

#include <sysalloc.h>

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))
 (((    Place of teleport alive :lol:
  )))   HashDict is used to store all teleports, and I will inhitialize
 (((    that tree ond allocate all related structures nce on a programs
  )))   start, and ... sorry guys, but I am going simple drop that 
 (((    on program exit without deallocation, cuz don't know when and
  )))   how we should dispose it, but here will be method for it
 (((*/

static const struct XFSHashDict * _sTeleport = NULL;

struct _TEntry {
    const struct XFSTeleport * Teleport;

    const char * Type;
};

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static
rc_t CC
_TEntryDispose ( struct _TEntry * self )
{
    if ( self != NULL ) {
        if ( self -> Type != NULL ) {
            free ( ( char * ) self -> Type );

            self -> Type = NULL;
        }

        if ( self -> Teleport != NULL ) {
            if ( self -> Teleport -> DeleteOnWhack ) {
                free ( ( struct XFSTeleport * ) self -> Teleport );

            }
            self -> Teleport = NULL;
        }

        free ( self );
    }

    return 0;
}   /* _TEntryDispose () */

static
rc_t CC
_TEntryMake (
            struct _TEntry ** Entry,
            const char * Type,
            XFSTeleportProvider_t Provider
)
{
    rc_t RCt;
    struct _TEntry * TheEntry;

    RCt = 0;
    TheEntry = NULL;

    XFS_CSAN ( Entry )
    XFS_CAN ( Entry )

    TheEntry = calloc ( 1, sizeof ( struct _TEntry ) );
    if ( TheEntry == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFS_StrDup ( Type, & ( TheEntry -> Type ) );
        if ( RCt == 0 ) {
            RCt = Provider ( & ( TheEntry -> Teleport ) );
            if ( RCt == 0 ) {
                if ( TheEntry -> Teleport != NULL ) {
                    * Entry = TheEntry;
                }
                else {
                    RCt = XFS_RC ( rcInvalid );
                }
            }
        }
    }

    if ( RCt != 0 ) {
        * Entry = NULL;

        if ( TheEntry != NULL ) {
            _TEntryDispose ( TheEntry );
        }
    }

    return RCt;
}   /* _TEntryMake () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static
const struct _TEntry * CC
_TeleportLookup ( const char * Type )
{
    const struct _TEntry * Entry = NULL;

    if ( _sTeleport != NULL ) {
        if ( XFSHashDictGet (
                            _sTeleport,
                            ( const void ** ) & Entry,
                            Type
                            ) != 0
        ) {
            Entry = NULL;
        }
    }

    return Entry;
}   /* _TeleportLookup () */

static
bool CC
_TeleportHas ( const char * Type )
{
    return _TeleportLookup ( Type ) != NULL;
}   /* _TeleportHas () */

static
rc_t CC
_TeleportAdd ( const char * Type, XFSTeleportProvider_t Provider )
{
    rc_t RCt;
    struct _TEntry * Ten;

    RCt = 0; 
    Ten = NULL;

    XFS_CAN ( Type )
    XFS_CAN ( Provider )

    if ( _TeleportHas ( Type ) == true ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = _TEntryMake ( & Ten, Type, Provider );
    if ( RCt == 0 ) {
        RCt = XFSHashDictAdd (
                            _sTeleport,
                            ( const void * ) Ten,
                            Ten -> Type
                            );
    }

    if ( RCt != 0 ) {
        if ( Ten != NULL ) {
            _TEntryDispose ( Ten );

            Ten = NULL;
        }
    }

    return RCt;
}   /* _TeleportAdd () */

#define CACHE_NAME                  "cache"
XFS_EXTERN rc_t CC XFSGapCacheProvider (
                                const struct XFSTeleport ** Teleport
                                );

#define FILE_NAME                   "file"
XFS_EXTERN rc_t CC XFSFileProvider (
                                const struct XFSTeleport ** Teleport
                                );

#define ENCRYPTED_FILE_NAME         "encrypted-file"
XFS_EXTERN rc_t CC XFSEncryptedFileProvider (
                                const struct XFSTeleport ** Teleport
                                );

#define DIRECTORY_NAME             "directory"
XFS_EXTERN rc_t CC XFSDirectoryProvider (
                                const struct XFSTeleport ** Teleport
                                );

#define README_NAME                "readme"
XFS_EXTERN rc_t CC XFSReadMeProvider (
                                const struct XFSTeleport ** Teleport
                                );

#define KART_KARTS_NAME            "karts"
XFS_EXTERN rc_t CC XFSGapKartsProvider (
                                const struct XFSTeleport ** Teleport
                                );

#define KART_NAME                   "kart"
XFS_EXTERN rc_t CC XFSGapKartProvider (
                                const struct XFSTeleport ** Teleport
                                );

#define KART_FILES_NAME                   "kart-files"
XFS_EXTERN rc_t CC XFSGapKartFilesProvider (
                                const struct XFSTeleport ** Teleport
                                );

#define LINK_NAME                   "link"
XFS_EXTERN rc_t CC XFSLinkProvider (
                                const struct XFSTeleport ** Teleport
                                );

#define LOCAL_REPOSITORY_NAME       "local-repository"
XFS_EXTERN rc_t CC XFSLocalRepositoryProvider (
                                const struct XFSTeleport ** Teleport
                                );

#define REMOTE_REPOSITORY_NAME      "remote-repository"
XFS_EXTERN rc_t CC XFSRemoteRepositoryProvider (
                                const struct XFSTeleport ** Teleport
                                );

/* TODO !!!
#define REMOTE_FILE_NAME            "remote-file"
XFS_EXTERN rc_t CC XFSRemoteFileProvider (
                                const struct XFSTeleport ** Teleport
                                );
*/

#define WORKSPACE_NAME              "workspace"
XFS_EXTERN rc_t CC XFSWorkspaceProvider (
                                const struct XFSTeleport ** Teleport
                                );

#define SIMPLE_CONTAINER_NAME       "simple-container"
XFS_EXTERN rc_t CC XFSSimpleContainerProvider (
                                const struct XFSTeleport ** Teleport
                                );

#define TAR_ARCHIVE_NAME            "tar-archive"
XFS_EXTERN rc_t CC XFSTarArchiveProvider (
                                const struct XFSTeleport ** Teleport
                                );

#define GAP_PROJECT_NAME            "gap-project"
XFS_EXTERN rc_t CC XFSGapProjectProvider (
                                const struct XFSTeleport ** Teleport
                                );

#define GAP_FILE_NAME               "gap-file"
XFS_EXTERN rc_t CC XFSGapFileProvider (
                                const struct XFSTeleport ** Teleport
                                );

#define ACCESS_PANEL_NAME               "access-panel"
XFS_EXTERN rc_t CC XFSAccessPanelProvider (
                                const struct XFSTeleport ** Teleport
                                );

static
void CC
_TeleportWhacker ( const void * Data )
{
    if ( Data != NULL ) {
        _TEntryDispose ( ( struct _TEntry * ) Data );
    }
}   /* _TeleportWhacker () */

static
rc_t CC
_TeleportInit ()
{
    rc_t RCt;

    RCt = 0;

    if ( _sTeleport != NULL ) {
        return 0;
    }

    RCt = XFSHashDictMake ( & _sTeleport, _TeleportWhacker );
    if ( RCt == 0 ) {
            /* Here we are adding Teleports */
        do {
            RCt = _TeleportAdd ( CACHE_NAME, XFSGapCacheProvider );
            if ( RCt != 0 ) { 
                break;
            }

            RCt = _TeleportAdd ( FILE_NAME, XFSFileProvider );
            if ( RCt != 0 ) { 
                break;
            }

            RCt = _TeleportAdd (
                            ENCRYPTED_FILE_NAME,
                            XFSEncryptedFileProvider
                            );
            if ( RCt != 0 ) { 
                break;
            }

            RCt = _TeleportAdd ( DIRECTORY_NAME, XFSDirectoryProvider );
            if ( RCt != 0 ) { 
                break;
            }

            RCt = _TeleportAdd ( README_NAME, XFSReadMeProvider );
            if ( RCt != 0 ) { 
                break;
            }

            RCt = _TeleportAdd ( KART_KARTS_NAME, XFSGapKartsProvider );
            if ( RCt != 0 ) { 
                break;
            }

            RCt = _TeleportAdd ( KART_NAME, XFSGapKartProvider );
            if ( RCt != 0 ) { 
                break;
            }

            RCt = _TeleportAdd (
                            KART_FILES_NAME,
                            XFSGapKartFilesProvider
                            );
            if ( RCt != 0 ) { 
                break;
            }

            RCt = _TeleportAdd ( LINK_NAME, XFSLinkProvider );
            if ( RCt != 0 ) { 
                break;
            }

            RCt = _TeleportAdd (
                            LOCAL_REPOSITORY_NAME,
                            XFSLocalRepositoryProvider
                            );
            if ( RCt != 0 ) { 
                break;
            }

            RCt = _TeleportAdd (
                            REMOTE_REPOSITORY_NAME,
                            XFSRemoteRepositoryProvider
                            );
            if ( RCt != 0 ) { 
                break;
            }

            RCt = _TeleportAdd ( WORKSPACE_NAME, XFSWorkspaceProvider );
            if ( RCt != 0 ) { 
                break;
            }

            RCt = _TeleportAdd (
                            SIMPLE_CONTAINER_NAME,
                            XFSSimpleContainerProvider
                            );
            if ( RCt != 0 ) { 
                break;
            }

            RCt = _TeleportAdd (
                            TAR_ARCHIVE_NAME,
                            XFSTarArchiveProvider
                            );
            if ( RCt != 0 ) { 
                break;
            }

            RCt = _TeleportAdd (
                            GAP_PROJECT_NAME,
                            XFSGapProjectProvider
                            );
            if ( RCt != 0 ) { 
                break;
            }

            RCt = _TeleportAdd (
                            GAP_FILE_NAME,
                            XFSGapFileProvider
                            );
            if ( RCt != 0 ) { 
                break;
            }

            RCt = _TeleportAdd (
                            ACCESS_PANEL_NAME,
                            XFSAccessPanelProvider
                            );
            if ( RCt != 0 ) { 
                break;
            }

        } while ( false );
    }

    return RCt;
}   /* _TeleportInit () */

static
rc_t CC
_TeleportWhack ()
{
    if ( _sTeleport != NULL ) {
        XFSHashDictDispose ( _sTeleport );

        _sTeleport = NULL;
    }

    return 0;
}   /* _TeleportWhack () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
LIB_EXPORT
rc_t CC
XFSTeleportLookup (
            const char * Type,
            const struct XFSTeleport ** Teleport
)
{
    const struct _TEntry * Entry = NULL;

    XFS_CSAN ( Teleport )
    XFS_CAN ( Type )
    XFS_CAN ( Teleport )

    Entry = _TeleportLookup ( Type );

    * Teleport = Entry == NULL ? NULL : ( Entry -> Teleport );

    return Teleport == NULL ? XFS_RC ( rcNotFound ) : 0;
}   /* XFSTeleportLookup () */

LIB_EXPORT
rc_t CC
XFSTeleportInit ()
{
    return _TeleportInit ();
}   /* XFSTeleportInit () */

LIB_EXPORT
rc_t CC
XFSTeleportDispose ()
{
    return _TeleportWhack ();
}   /* XFSTeleportDispose () */
