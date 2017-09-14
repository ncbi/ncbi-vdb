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
#include <klib/refcount.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kproc/lock.h>

#include <xfs/access.h>
#include <xfs/doc.h>

#include "zehr.h"
#include "schwarzschraube.h"
#include "lreader.h"

#include <sysalloc.h>

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))
 (((    That is Accessibility is making their fuf.
  )))   
 (((*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))
 (((    XFSAccessAgent
  )))   
 (((*/

/*  Right now it is only name. Later I will add refcount and policies
 */
struct XFSAccessAgent {
    BSTNode node;

    const char * Name;
};

rc_t CC
_AccessAgentDispose ( struct XFSAccessAgent * self )
{
    if ( self != NULL ) {
        if ( self -> Name != NULL ) {
            free ( ( char * ) self -> Name );
            self -> Name = NULL;
        }

        free ( self );
    }

    return 0;
}   /* _AccessAgentDispose () */

rc_t CC
_AccessAgentMake ( struct XFSAccessAgent ** Agent, const char * Name )
{
    rc_t RCt;
    struct XFSAccessAgent * xAgent;

    RCt = 0;
    xAgent = NULL;

    XFS_CSAN ( Agent )
    XFS_CAN ( Agent )
    XFS_CAN ( Name )

    xAgent = calloc ( 1, sizeof ( struct XFSAccessAgent ) );
    if ( xAgent == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RCt = XFS_StrDup ( Name, & xAgent -> Name );

    if ( RCt == 0 ) {
        * Agent = xAgent;
    }
    else {
        _AccessAgentDispose ( xAgent );
    }

    return RCt;
}   /* _AccessAgentMake () */

LIB_EXPORT
bool CC
XFSAccessAgentApprove (
                    const struct XFSAccessAgent * self,
                    const char * User,
                    const char * Group,
                    const char * Path
)
{
        /* Group could be NULL */
    if ( self != NULL && User != NULL && Path != NULL ) {
            /* Here we should compare User with Agent name */
        return XFS_StringCompare4BST_ZHR ( User, self -> Name ) == 0;
    }

    return false;
}   /* XFSAccessAgentApprove () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))
 (((    _Access
  )))   
 (((*/
struct _Access {
    struct KLock * mutabor;
    KRefcount refcount;

    const char * Path;

    const char * User;
    const char * Group;

    uint64_t Version;

    struct BSTree Agents;
};

/***************
 ********** JOJOBA: somewhere should be
 *****  Version check and file reload.
 */

static const char * _sAccess_classname = "_Access";

static rc_t CC _AccessAdd_noLock (
                        const struct _Access * self,
                        const char * AgentName,
                        const struct XFSAccessAgent ** Agent
                        );
static rc_t CC _AccessAdd (
                        const struct _Access * self,
                        const char * AgentName,
                        const struct XFSAccessAgent ** Agent
                        );
static rc_t CC _AccessList (
                        const struct _Access * self,
                        struct KNamelist ** List
                        );
static rc_t CC _AccessList_noLock (
                        const struct _Access * self,
                        struct KNamelist ** List
                        );
static rc_t CC _AccessGet (
                        const struct _Access * self,
                        const char * AgentName,
                        const struct XFSAccessAgent ** Agent
                        );


/*  Really that one reads timestamp. If any error it returne 0
 *  Kinda cool.
 */
static
uint64_t CC
_AccessReadVersion ( const struct _Access * self )
{
    struct KDirectory * xDir;
    uint64_t RetVal;

    RetVal = 0;

    xDir = 0;

    if ( self != NULL ) {
        if ( self -> Path != NULL ) {
            if ( KDirectoryNativeDir ( & xDir ) == 0 ) {
                if ( KDirectoryDate ( xDir, ( KTime_t * ) & RetVal, self -> Path ) != 0 ) {
                    RetVal = 0;
                }

                KDirectoryRelease ( xDir );
            }
        }
    }

    return 0;
}   /* _AccessReadVersion () */

static
void CC
_AccessWhackCallback ( BSTNode * Node, void * unused )
{
    if ( Node != NULL ) {
            /*  JOJOBA
             *  will be Release later
             */
        _AccessAgentDispose ( ( struct XFSAccessAgent * ) Node );
    }
}   /* _AccessWhackCallback () */

static
rc_t
_AccessRemoveAgents_noLock ( const struct _Access * self )
{
    if ( self != NULL ) {
        BSTreeWhack (
                    & ( ( ( struct _Access * ) self ) -> Agents ),
                    _AccessWhackCallback,
                    NULL
                    );
    }

    return 0;
}   /* _AccessRemoveAgents_noLock () */

static
rc_t CC
_AccessDispose ( const struct _Access * self )
{
    struct _Access * Acc = ( struct _Access * ) self;

    if ( Acc == NULL ) {
        return 0;
    }

    _AccessRemoveAgents_noLock ( self );
    KRefcountWhack ( & ( Acc -> refcount ), _sAccess_classname );

    Acc -> Version = 0;


    if ( Acc -> mutabor != NULL ) {
        KLockRelease ( Acc -> mutabor );
        Acc -> mutabor = NULL;
    }

    if ( Acc -> Path != NULL ) {
        free ( ( char * ) Acc -> Path );
        Acc -> Path = NULL;
    }

    if ( Acc -> User != NULL ) {
        free ( ( char * ) Acc -> User );
        Acc -> User = NULL;
    }

    if ( Acc -> Group != NULL ) {
        free ( ( char * ) Acc -> Group );
        Acc -> Group = NULL;
    }

    free ( Acc );

    return 0;
}   /* _AccessDispose () */

static
rc_t CC
_AccessLoad ( const struct _Access * self, const char * Path )
{
    rc_t RCt;
    struct _Access * xAcc;
    const struct XFSLineReader * Reader;
    struct KDirectory * NativeDir;
    const struct String Line;
    const char * AgentName;

    RCt = 0;
    xAcc = ( struct _Access * ) self;
    Reader = NULL;
    NativeDir = NULL;
    AgentName = NULL;

    XFS_CAN ( xAcc )
    XFS_CAN ( Path )

// printf ( "<<==[ACC] [%d]\n", __LINE__ );
    RCt = KLockAcquire ( xAcc -> mutabor );
    if ( RCt == 0 ) {

        if ( xAcc -> Path != NULL ) {
            free ( ( char * ) xAcc -> Path );
            xAcc -> Path = NULL;
        }

        RCt = XFS_StrDup ( Path, ( const char ** ) & self -> Path );
        if ( RCt == 0 ) {
            RCt = KDirectoryNativeDir ( & NativeDir );
            if ( RCt == 0 ) {
                if ( KDirectoryPathType ( NativeDir, Path ) == kptFile ) {
                    _AccessRemoveAgents_noLock ( xAcc );

                    RCt = XFSLineReaderOpen ( & Reader, Path );
                    if ( RCt == 0 ) {
                        while ( XFSLineReaderNext ( Reader ) ) {
                            RCt = XFSLineReaderGet ( Reader, & Line );
                            if ( RCt == 0 ) {
                                RCt = XFS_SStrDup ( & Line, & AgentName );
                                if ( RCt == 0 ) {
                                    /*  Adding everybody except yourself
                                     */
                                    if ( strcmp ( AgentName, self -> User ) != 0 ) {
                                        RCt = _AccessAdd_noLock (
                                                            self,
                                                            AgentName,
                                                            NULL
                                                            );
                                    }
                                    free ( ( char * ) AgentName );
                                }
                            }

                            if ( RCt != 0 ) {
                                break;
                            }
                        };

                        XFSLineReaderRelease ( Reader );
                    }
                }
                else {
                    /*  JOJOBA
                     *  apparently that is not an error
                     *  BUT, we should issue WARNING here :LOL:
                     */
                }

                KDirectoryRelease ( NativeDir );
            }
        }

        if ( RCt != 0 ) {
            _AccessRemoveAgents_noLock ( self );
        }

// printf ( "<<==[UNL] [%d]\n", __LINE__ );
        KLockUnlock ( xAcc -> mutabor );
    }

    return RCt;
}   /* _AccessLoad () */

static
rc_t CC
_AccessMake ( const struct _Access ** Acc, const char * Path )
{
    rc_t RCt;
    struct _Access * xAcc;

    RCt = 0;
    xAcc = NULL;

    XFS_CSAN ( Acc )
    XFS_CAN ( Acc )

    xAcc = calloc ( 1, sizeof ( struct _Access ) );
    if ( xAcc == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RCt = KLockMake ( & ( xAcc -> mutabor ) );
    if ( RCt == 0 ) {
        BSTreeInit ( & ( xAcc -> Agents ) );
        KRefcountInit (
                    & ( xAcc -> refcount ),
                    1,
                    _sAccess_classname,
                    "_AccessMake",
                    "_Access"
                    );

        RCt = XFSOwnerUserName ( & ( xAcc -> User ) );
        if ( RCt == 0 ) {
            RCt = XFSOwnerGroupName ( & ( xAcc -> Group ) );

            if ( RCt == 0 ) {
                RCt = _AccessLoad ( xAcc, Path );
                if ( RCt == 0 ) {
                    xAcc -> Version = _AccessReadVersion ( xAcc );
                }
            }
        }
    }

    if ( RCt != 0 ) {
        if ( xAcc != NULL ) {
            _AccessDispose ( xAcc );
        }
        xAcc = NULL;
    }
    else {
        * Acc = xAcc;
    }

    return RCt;
}   /* _AccessMake () */

/*
 * That method has had to load some file from standard location.
 * we don't care if file does not exists, that means that access
 * is granted only for owner of FUSER
 */
static
rc_t CC
_AccessMake4Gap ( const struct _Access ** Acc, uint32_t ProjectId )
{
    rc_t RCt;
    char * Path;

    RCt = 0;
    Path = NULL;

        /*  First we should find a path
         */
    RCt = XFS_UserAccessList_ZHR ( & Path, ProjectId );
    if ( RCt == 0 ) {
        RCt = _AccessMake ( Acc, Path );

        free ( Path );
    }

    return RCt;
}   /* _AccessMake4Gap () */

static
rc_t
_MakeAccessDocument (
                    const struct _Access * self,
                    struct XFSDoc ** TextDoc
)
{
    rc_t RCt;
    struct KNamelist * List;
    uint32_t ListSize, llp;
    struct XFSDoc * xDoc;
    const char * Name;

    RCt = 0;
    List = NULL;
    ListSize = llp = 0;
    xDoc = NULL;
    Name = NULL;

    XFS_CSAN ( TextDoc )
    XFS_CAN ( self )
    XFS_CAN ( TextDoc )

    RCt = XFSTextDocMake ( & xDoc );
    if ( RCt == 0 ) {
        RCt = _AccessList_noLock ( self, & List );
        if ( RCt == 0 ) {
            RCt = KNamelistCount ( List, & ListSize );
            if ( RCt == 0 ) {
                for ( llp = 0; llp < ListSize; llp ++ ) {
                    RCt = KNamelistGet ( List, llp, & Name );
                    if ( RCt != 0 ) {
                        break;
                    }

                        /* It should not happen, but for any case */
                    if ( strcmp ( self -> User, Name ) == 0 ) {
                        continue;
                    }

                    RCt = XFSTextDocAppend ( xDoc, "%s\n", Name );
                    if ( RCt != 0 ) {
                        break;
                    }
                }
            }
        }

        KNamelistRelease ( List );
    }

    if ( RCt == 0 ) {
        * TextDoc = xDoc;
    }
    else {
        * TextDoc = NULL;

        if ( xDoc != NULL ) {
            XFSDocRelease ( xDoc );
        }
    }

    return RCt;
}   /* _MakeAccessDocument () */

static
rc_t CC
_AccessStore ( const struct _Access * self )
{
    rc_t RCt;
    struct KDirectory * NativeDir;
    struct KFile * File;
    struct XFSDoc * Doc;
    const char * DocBuffer;
    size_t DocSize, NumWritten;

    RCt = 0;
    NativeDir = NULL;
    File = NULL;
    Doc = NULL;
    DocBuffer = NULL;
    DocSize = NumWritten = 0;

    XFS_CAN ( self )
    XFS_CAN ( self -> Path )

// printf ( "<<==[ACC] [%d]\n", __LINE__ );
    RCt = KLockAcquire ( self -> mutabor );
    if ( RCt == 0 ) {
        RCt = KDirectoryNativeDir ( & NativeDir );
        if ( RCt == 0 ) {
            RCt = KDirectoryCreateFile ( NativeDir, & File, false, 0600, kcmInit, self -> Path ); 
            if ( RCt == 0 ) {
                RCt = _MakeAccessDocument ( self, & Doc );
                if ( RCt == 0 ) {
                    RCt = XFSDocSize ( Doc, & DocSize );
                    if ( RCt == 0 ) {
                        if ( DocSize == 0 ) {
                            RCt = KFileSetSize ( File, 0 );
                        }
                        else {
                            RCt = XFSDocGet ( Doc, & DocBuffer );
                            if ( RCt == 0 ) {
                                RCt = KFileWriteAll (
                                                File,
                                                0,
                                                DocBuffer,
                                                DocSize,
                                                & NumWritten
                                                );

                            }
                        }
                    }

                    XFSDocRelease ( Doc );
                }

                KFileRelease ( File );
            }

            KDirectoryRelease ( NativeDir );
        }

// printf ( "<<==[UNL] [%d]\n", __LINE__ );
        KLockUnlock ( self -> mutabor );
    }

    return RCt;
}   /* _AccessStore () */

rc_t CC
_AccessAddRef ( const struct _Access * self )
{
    rc_t RCt = 0;

    XFS_CAN ( self )

    switch ( KRefcountAdd ( & ( self -> refcount ), _sAccess_classname ) ) {
        case krefOkay :
                RCt = 0;
                break;
        case krefZero :
        case krefLimit :
        case krefNegative :
                RCt = XFS_RC ( rcInvalid );
                break;
        default :
                RCt = XFS_RC ( rcUnknown );
                break;
        }

    return RCt;
}   /* _AccessAddRef () */

static
rc_t CC
_AccessRelease ( const struct _Access * self )
{
    rc_t RCt = 0;

    XFS_CAN ( self )

    switch ( KRefcountDrop ( & ( self -> refcount ), _sAccess_classname ) ) {
        case krefOkay :
        case krefZero :
            RCt = 0;
            break;
        case krefWhack :
            RCt = _AccessDispose ( ( struct _Access * ) self );
            break;
        case krefNegative :
            RCt = XFS_RC ( rcInvalid );
            break;
        default :
            RCt = XFS_RC ( rcUnknown );
            break;
    }

    return RCt;
}   /* _AccessRelease () */

static
bool CC
_AccessApprove (
                const struct _Access * self,
                const char * User,
                const char * Group,
                const char * Path
)
{
    const struct XFSAccessAgent * xAgent = NULL;

    XFS_CAN ( self )
    XFS_CAN ( User )
    XFS_CAN ( Path )

    if ( strcmp ( User, "root" ) == 0 ) {
        return true;
    }

    if ( strcmp ( User, self -> User ) == 0 ) {
        return true;
    }

        /*  JOJOBA: ugly, but have space to thing
         *  LOL ...
         */
#define DAS_STRUNG "/access-panel"
    if ( strncmp ( Path, DAS_STRUNG, strlen ( DAS_STRUNG) ) == 0 ) {
        return false;
    }
#undef DAS_STRUNG
#define DAS_STRUNG "/cache"
    if ( strncmp ( Path, DAS_STRUNG, strlen ( DAS_STRUNG) ) == 0 ) {
        return false;
    }
#undef DAS_STRUNG
#define DAS_STRUNG "/kart-files"
    if ( strncmp ( Path, DAS_STRUNG, strlen ( DAS_STRUNG) ) == 0 ) {
        return false;
    }
#undef DAS_STRUNG

        /*  First : Find agent for user
         */
    _AccessGet ( self, User, & xAgent );

    return xAgent == NULL
                ? false
                : XFSAccessAgentApprove ( xAgent, User, Group, Path )
                ;
}   /* _AccessApprove () */

static
void
_ListAccessAgentCallback ( BSTNode * Node, void * Data )
{
    struct VNamelist * List;
    struct XFSAccessAgent * xAgent;

    List = ( struct VNamelist * ) Data;
    xAgent = ( struct XFSAccessAgent * ) Node;

    if ( List == NULL || xAgent == NULL ) {
        return;
    }

    if ( xAgent -> Name != NULL ) {
        VNamelistAppend ( List, xAgent -> Name );
    }
}   /* _ListAccessAgentCallback () */

static
rc_t CC
_AccessList_noLock (
                const struct _Access * self,
                struct KNamelist ** List
)
{
    rc_t RCt;
    struct VNamelist * xList;

    RCt = 0;
    xList = NULL;

    XFS_CSAN ( List )
    XFS_CAN ( self )
    XFS_CAN ( List )

    RCt = VNamelistMake ( & xList, 32 );
    if ( RCt == 0 ) {
        BSTreeForEach (
                    & ( self -> Agents ),
                    false,
                    _ListAccessAgentCallback,
                    xList
                    );
        RCt = VNamelistToNamelist ( xList, List );
    }

    return RCt;
}   /* _AccessList_noLock () */

rc_t CC
_AccessList (
                const struct _Access * self,
                struct KNamelist ** List
)
{
    rc_t RCt = 0;

    XFS_CSAN ( List )
    XFS_CAN ( self )
    XFS_CAN ( List )

// printf ( "<<==[ACC] [%d]\n", __LINE__ );
    RCt = KLockAcquire ( self -> mutabor );
    if ( RCt == 0 ) {
        RCt = _AccessList_noLock ( self, List );

// printf ( "<<==[UNL] [%d]\n", __LINE__ );
        KLockUnlock ( self -> mutabor );
    }

    return RCt;
}   /* _AccessList () */

static
bool CC
_AccessHas (
                const struct _Access * self,
                const char * AgentName
)
{
    const struct XFSAccessAgent * xAgent = NULL;

    XFS_CAN ( self )
    XFS_CAN ( AgentName )

    _AccessGet ( self, AgentName, & xAgent );

    return xAgent != NULL;
}   /* _AccessHas () */

static
int64_t CC
_GetAccessAgentCallback (  const void * Item, const BSTNode * Node )
{
    const char * Str1, * Str2;

    Str1 = ( ( char * ) Item );
    Str2 = Node == NULL
                        ? NULL
                        : ( ( struct XFSAccessAgent * ) Node ) -> Name
                        ;


    return XFS_StringCompare4BST_ZHR ( Str1, Str2 );
}   /* _GetAssAgentCallback () */

rc_t CC
_AccessGet_noLock (
                const struct _Access * self,
                const char * AgentName,
                const struct XFSAccessAgent ** Agent
)
{
    rc_t RCt;
    const struct XFSAccessAgent * xAgent;

    RCt = 0;
    xAgent = NULL;

    XFS_CSAN ( Agent );
    XFS_CAN ( self )
    XFS_CAN ( Agent )
    XFS_CAN ( AgentName )

    xAgent = ( const struct XFSAccessAgent * ) BSTreeFind (
                                            & ( self -> Agents ),
                                            AgentName,
                                            _GetAccessAgentCallback
                                            );
    if ( xAgent != NULL ) {
            /*  JOJOBA
             *  Here we should add RefCount ... will do later 
             */
        * Agent = xAgent;
    }
    else {
        RCt = XFS_RC ( rcNotFound );
    }

    return RCt;
}   /* _AccessGet_noLock () */

rc_t CC
_AccessGet (
                const struct _Access * self,
                const char * AgentName,
                const struct XFSAccessAgent ** Agent
)
{
    rc_t RCt = 0;

    XFS_CSAN ( Agent );
    XFS_CAN ( self )
    XFS_CAN ( Agent )
    XFS_CAN ( AgentName )

// printf ( "<<==[ACC] [%d]\n", __LINE__ );
    RCt = KLockAcquire ( self -> mutabor );
    if ( RCt == 0 ) {
        RCt = _AccessGet_noLock ( self, AgentName, Agent );

// printf ( "<<==[UNL] [%d]\n", __LINE__ );
        KLockUnlock ( self -> mutabor );
    }

    return RCt;
}   /* _AccessGet () */


static
int64_t CC
_AddAccessAgentCallback ( const BSTNode * N1, const BSTNode * N2 )
{
    return XFS_StringCompare4BST_ZHR (
        ( ( struct XFSAccessAgent * ) N1 ) -> Name,
        ( ( struct XFSAccessAgent * ) N2 ) -> Name
        );
}   /* _AccessAgentCallback () */

rc_t CC
_AccessAdd_noLock (
                const struct _Access * self,
                const char * AgentName,
                const struct XFSAccessAgent ** Agent
)
{
    rc_t RCt;
    struct XFSAccessAgent * xAgent;
    struct _Access * xAcc;

    RCt = 0;
    xAgent = NULL;
    xAcc = ( struct _Access * ) self;

    XFS_CSAN ( Agent )
    XFS_CAN ( xAcc )
    XFS_CAN ( AgentName )

    RCt = _AccessAgentMake ( & xAgent, AgentName );
    if ( RCt == 0 ) {
        RCt = BSTreeInsert (
                        & ( xAcc -> Agents ),
                        & ( xAgent -> node ),
                        _AddAccessAgentCallback
                        );
    }

    if ( RCt == 0 ) {
        if ( Agent != NULL ) {
            * Agent = xAgent;
        }
    }
    else {
        if ( xAgent != NULL ) {
            _AccessAgentDispose ( xAgent );

            xAgent = NULL;
        }
    }

    return RCt;
}   /* _AccessAdd_noLock () */

rc_t CC
_AccessAdd (
                const struct _Access * self,
                const char * AgentName,
                const struct XFSAccessAgent ** Agent
)
{
    rc_t RCt;
    struct _Access * xAcc;

    RCt = 0;
    xAcc = ( struct _Access * ) self;

    XFS_CSAN ( Agent )
    XFS_CAN ( xAcc )
    XFS_CAN ( AgentName )

// printf ( "<<==[ACC] [%d]\n", __LINE__ );
    RCt = KLockAcquire ( self -> mutabor );
    if ( RCt == 0 ) {
        RCt = _AccessAdd_noLock ( self, AgentName, Agent );

// printf ( "<<==[UNL] [%d]\n", __LINE__ );
        KLockUnlock ( self -> mutabor );

        if ( RCt == 0 ) {
            if ( _AccessStore ( self ) != 0 ) {
                /* JOJOBA */
                /*  we don't care if it is failed, but
                 *  need to complain
                 */
            }
        }
    }

    return RCt;
}   /* _AccessAdd () */

static
rc_t CC
_AccessDel_noLock (
                const struct _Access * self,
                const char * AgentName
)
{
    rc_t RCt;
    const struct XFSAccessAgent * xAgent;
    struct _Access * xAcc;

    RCt = 0;
    xAgent = NULL;
    xAcc = ( struct _Access * ) self;

    XFS_CAN ( xAcc )
    XFS_CAN ( AgentName )


    RCt = _AccessGet ( xAcc, AgentName, & xAgent );
    if ( xAgent != NULL ) {
        BSTreeUnlink ( & ( xAcc -> Agents ), ( struct BSTNode * ) & ( xAgent -> node ) );

        if ( _AccessStore ( self ) != 0 ) {
            /* JOJOBA */
            /*  we don't care if it is failed, but
             *  need to complain
             */
        }

            /*  Should it be here?
             */
        _AccessAgentDispose ( ( struct XFSAccessAgent * ) xAgent );
    }

    return RCt;
}   /* _AccessDel_noLock () */

static
rc_t CC
_AccessDel (
                const struct _Access * self,
                const char * AgentName
)
{
    rc_t RCt;
    const struct XFSAccessAgent * xAgent;
    struct _Access * xAcc;

    RCt = 0;
    xAgent = NULL;
    xAcc = ( struct _Access * ) self;

    XFS_CAN ( xAcc )
    XFS_CAN ( AgentName )


    RCt = _AccessGet ( xAcc, AgentName, & xAgent );
    if ( xAgent != NULL ) {
// printf ( "<<==[ACC] [%d]\n", __LINE__ );

        RCt = _AccessDel_noLock ( self, AgentName );

// printf ( "<<==[UNL] [%d]\n", __LINE__ );
            KLockUnlock ( self -> mutabor );
    }

    return RCt;
}   /* _AccessDel () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))
 (((    _AccessDepot
  )))   
 (((*/
static const struct _Access * _sAccess = NULL;

LIB_EXPORT
rc_t CC
XFSAccessInit ( const char * Path )
{
    XFS_CAN ( Path )

    XFSAccessDispose ();

    return _AccessMake ( & _sAccess, Path );
}   /* XFSAccessInit () */

LIB_EXPORT
rc_t CC
XFSAccessInit4Gap ( uint32_t ProjectId )
{
    XFSAccessDispose ();

    return _AccessMake4Gap ( & _sAccess, ProjectId );
}   /* XFSAccessInit4Gap () */

LIB_EXPORT
rc_t CC
XFSAccessDispose ()
{
    if ( _sAccess != NULL ) {
        _AccessRelease ( _sAccess );
        _sAccess = NULL;
    }
    return 0;
}   /* XFSAccessDispose () */

LIB_EXPORT
bool CC
XFSAccessApprove (
                const char * User,
                const char * Group,
                const char * Path
)
{
    return _sAccess == NULL
                        ? true
                        : _AccessApprove ( _sAccess, User, Group, Path )
                        ;
}   /* XFSAccessApprove () */

LIB_EXPORT
rc_t CC
XFSAccessList ( struct KNamelist ** List )
{
    rc_t RCt;
    struct VNamelist * xList;

    RCt = 0;
    xList = NULL;

    if ( _sAccess != NULL ) {
        return _AccessList ( _sAccess, List );
    }

        /*  We will return empty list for any case
         */
    RCt = VNamelistMake ( & xList, 32 );
    if ( RCt == 0 ) {
        RCt = VNamelistToNamelist ( xList, List );

        VNamelistRelease ( xList );
    }

    return RCt;
}   /* XFSAccessList () */

LIB_EXPORT
bool CC
XFSAccessHas ( const char * AgentName )
{
    return _sAccess == NULL
                        ? false
                        : _AccessHas ( _sAccess, AgentName )
                        ;
}   /* XFSAccessHas () */

LIB_EXPORT
rc_t CC
XFSAccessGet (
            const char * AgentName,
            const struct XFSAccessAgent ** Agent
)
{
    XFS_CSAN ( Agent )

    return _sAccess == NULL
                    ? 0
                    : _AccessGet ( _sAccess, AgentName, Agent )
                    ;
}   /* XFSAccessGet () */

LIB_EXPORT
rc_t CC
XFSAccessAdd (
            const char * AgentName,
            const struct XFSAccessAgent ** Agent
)
{
    XFS_CSAN ( Agent )

    return _sAccess == NULL
                    ? 0
                    : _AccessAdd ( _sAccess, AgentName, Agent )
                    ;
}   /* XFSAccessAdd () */

LIB_EXPORT
rc_t CC
XFSAccessDel ( const char * AgentName )
{
    return _sAccess == NULL
                    ? 0
                    : _AccessDel ( _sAccess, AgentName )
                    ;
}   /* XFSAccessDel () */

