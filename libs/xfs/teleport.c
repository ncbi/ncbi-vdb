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
#include <klib/container.h>

#include "teleport.h"
#include "zehr.h"
#include "schwarzschraube.h"

#include <sysalloc.h>

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))
 (((    Place of teleport alive :lol:
  )))   I will use BSTree to store all teleports, and I will inhitialize
 (((    that tree ond allocate all related structures nce on a programs
  )))   start, and ... sorry guys, but I am going simple drop that 
 (((    on program exit without deallocation, cuz don't know when and
  )))   how we should dispose it, but here will be method for it
 (((*/

static BSTree _sTeleport;
static bool _sTeleportInited = false;

struct _TNode {
    BSTNode node;

    const char * Type;
    const struct XFSTeleport * Teleport;
};

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static
int64_t
_TeleportLookupCallback ( const void * Item, const BSTNode * Node )
{
    return XFS_StringCompare4BST_ZHR (
                                ( const char * ) Item,
                                ( ( struct _TNode * ) Node ) -> Type
                                );
}   /* _TeleportLookupCallback () */

static
const struct _TNode * CC
_TeleportLookup ( const char * Type )
{
    if ( _sTeleportInited == true ) {
        return ( const struct _TNode * ) BSTreeFind (
                                        ( struct BSTree * ) & _sTeleport,
                                        Type,
                                        _TeleportLookupCallback
                                        );
    }

    return NULL;
}   /* _TeleportLookup () */

static
bool CC
_TeleportHas ( const char * Type )
{
    return _TeleportLookup ( Type ) != NULL;
}   /* _TeleportHas () */

static
int64_t
_TeleportAddCallback ( const BSTNode * Item, const BSTNode * Node )
{
    return XFS_StringCompare4BST_ZHR (
                                ( ( struct _TNode * ) Item ) -> Type,
                                ( ( struct _TNode * ) Node ) -> Type
                                );
}   /* _TeleportAddCallback () */

static
rc_t CC
_TeleportAdd ( const char * Name, XFSTeleportProvider_t Provider )
{
    rc_t RCt;
    struct _TNode * Tde;

    RCt = 0; 

    if ( Name == NULL || Provider == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( _TeleportHas ( Name ) == true ) {
        return XFS_RC ( rcInvalid );
    }

    Tde = calloc ( 1, sizeof ( struct _TNode ) );
    if ( Tde == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    if ( XFS_StrDup ( Name, & ( Tde -> Type ) ) != 0 ) {
        free ( Tde );

        return XFS_RC ( rcExhausted );
    }

    RCt = Provider ( & Tde -> Teleport );

    if ( RCt != 0 || Tde -> Teleport == NULL ) {
        free ( ( char * ) Tde -> Type );
        free ( Tde );

        return XFS_RC ( rcInvalid );
    }

    BSTreeInsert (
                ( struct BSTree * ) & _sTeleport,
                ( struct BSTNode * ) Tde,
                _TeleportAddCallback
                );

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

static
rc_t CC
_TeleportInit ()
{
    rc_t RCt;

    RCt = 0;

    if ( _sTeleportInited == true ) {
        return 0;
    }

    BSTreeInit ( & _sTeleport );
    _sTeleportInited = true;

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

        RCt = _TeleportAdd ( KART_FILES_NAME, XFSGapKartFilesProvider );
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

    } while ( false );

    return RCt;
}   /* _TeleportInit () */

static
void CC
_TeleportWhacker ( BSTNode * Node, void * Unused )
{
    struct _TNode * Tode;

    Tode = ( struct _TNode * ) Node;

    if ( Tode != NULL ) {
        if ( Tode -> Type != NULL ) {
            free ( ( char * ) Tode -> Type );

            Tode -> Type = NULL;
        }

        if ( Tode -> Teleport != NULL ) {
            if ( Tode -> Teleport -> DeleteOnWhack ) {
                free ( ( struct XFSTeleport * ) Tode -> Teleport );

            }
            Tode -> Teleport = NULL;
        }

        free ( Tode );
    }
}   /* _TeleportWhacker () */

static
rc_t CC
_TeleportWhack ()
{
    rc_t RCt;

    RCt = 0;

    if ( _sTeleportInited != true ) {
        return XFS_RC ( rcInvalid );
    }

    BSTreeWhack ( & _sTeleport, _TeleportWhacker, NULL );

    _sTeleportInited = false;

    return RCt;
}   /* _TeleportWhack () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
LIB_EXPORT
rc_t CC
XFSTeleportLookup (
            const char * NodeType,
            const struct XFSTeleport ** Teleport
)
{
    rc_t RCt;
    const struct _TNode * tNode;

    RCt = 0;
    tNode = NULL;

    if ( NodeType == NULL || Teleport == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Teleport = NULL;

    if ( _sTeleportInited == false ) {
        /* HOHOA, will init it somewhere else
            RCt = _TeleportInit ();
         */
        return XFS_RC ( rcInvalid );
    }

    if ( RCt == 0 ) {
        tNode = _TeleportLookup ( NodeType );
        if ( tNode == NULL ) {
            RCt = XFS_RC ( rcNotFound );
        }
        else {
            * Teleport = tNode -> Teleport;
        }
    }

    return RCt;
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
