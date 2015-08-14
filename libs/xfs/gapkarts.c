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
#include <klib/refcount.h>
#include <klib/printf.h>
#include <klib/namelist.h>

#include <kfs/file.h>
#include <kfs/directory.h>
#include <kfg/kart.h>

#include <xfs/model.h>
#include <xfs/tree.h>
#include <xfs/node.h>
#include <xfs/path.h>
#include <xfs/editors.h>
#include <xfs/perm.h>
#include <xfs/doc.h>

#include "mehr.h"
#include "zehr.h"
#include "schwarzschraube.h"
#include "ncon.h"
#include "teleport.h"
#include "common.h"
#include "contnode.h"

#include <sysalloc.h>

#include <stdio.h>
#include <string.h>     /* memset */

/*)))
 |||    That is first draft approach for reading Kart files.
 |||    Represents KartNode container with KartItemNodes.
(((*/

/*)))
 |||
 +++    KartNode and KartItemNode
 |||
(((*/

struct XFSGapKartsNode {
    struct XFSContNode node;

    uint32_t project_id;

    const char * perm;
};

static
rc_t CC
_ExtractName ( const char * Path, char * Buffer, size_t BufferSize )
{
    rc_t RCt;
    const char * pStart;
    const char * pEnd;
    const char * Start;
    const char * End;

    RCt = 0;
    pStart = NULL;
    pEnd = NULL;
    Start = NULL;
    End = NULL;

    XFS_CAN ( Path )
    XFS_CAN ( Buffer )
    XFS_CA ( BufferSize, 0 )

    pStart = Path;
    pEnd = pStart + string_size ( Path ) - 1;

        /* Looking for begin of filename
         */
    while ( pStart < pEnd ) {
        if ( * pEnd == '/' ) {
            Start = pEnd + 1;
            break;
        }
        pEnd --;
    }

    if ( Start == NULL ) {
        Start = pStart;
    }

    pEnd = pStart + string_size ( Path ) - 1;
    End = pEnd;

         /* Looking for end of filename
          */
    while ( pStart < pEnd ) {
        if ( * pEnd == '.' ) {
            End = pEnd;
            break;
        }
        pEnd --;
    }

    string_copy ( Buffer, BufferSize, Start, End - Start );

    return RCt;
}   /* _ExtractName () */

static
rc_t CC
_LoadKart ( struct XFSGapKartsNode * Node, const char * Path )
{
    rc_t RCt;
    struct XFSNode * KartNode;
    char Name [ XFS_SIZE_128 ];

    RCt = 0;
    KartNode = NULL;
    * Name = 0;

    XFS_CAN ( Node )
    XFS_CAN ( Path )

    RCt = _ExtractName ( Path, Name, sizeof ( Name ) );
    if ( RCt == 0 ) {
        RCt = XFSKartNodeMake ( & KartNode, Name, Path, NULL );
        if ( RCt == 0 ) {
            RCt = XFSContNodeAddChild ( 
                                    & ( Node -> node . node ),
                                    KartNode
                                    );
        }
        else {
                /*  Some Karts could be damaged 
                 |  we should think about it later
                 */
            KOutMsg ( "Invalid Kart file [%s]\n", Path );
            RCt = 0;
        }
    }

    return RCt;
}   /* _LoadKart () */

static 
rc_t CC
_LoadKarts (
            struct XFSGapKartsNode * Node,
            const char * ComaSeparatedPaths
)
{
    rc_t RCt;
    struct KNamelist * Paths;
    uint32_t qty, llp;
    const char * Path;

    RCt = 0;
    Paths = NULL;
    qty = llp = 0;
    Path = NULL;

    XFS_CAN ( Node )

        /* No Karts defined
         */
    if ( ComaSeparatedPaths == NULL ) {
        return 0;
    }

    RCt = XFS_SimpleTokenize_ZHR ( ComaSeparatedPaths, ',', & Paths );
    if ( RCt == 0 ) {

        RCt = KNamelistCount ( Paths, & qty );
        if ( RCt == 0 ) {
            for ( llp = 0; llp < qty; llp ++ ) {
                RCt = KNamelistGet ( Paths, llp, & Path );
                if ( RCt == 0 ) {
                    RCt = _LoadKart ( Node, Path );
                }

                RCt = 0;
            }
        }

        KNamelistRelease ( Paths );
    }

    if ( RCt != 0 ) {
            /*  Sorry, we just were unable to load any Kart
             */
        RCt = 0;
    }

#ifdef JOJOBA
    struct KDirectory * Directory;
    KNamelist * List;
    const char * Name;
    uint32_t Idx;
    uint32_t ListCount;

    Directory = NULL;
    List = NULL;
    Name = NULL;
    Idx = 0;
    ListCount = 0;

    RCt = KDirectoryNativeDir ( & Directory );
    if ( RCt == 0 ) {
        RCt = KDirectoryList (
                            Directory,
                            & List,
                            NULL,
                            NULL,
                            Node -> path
                            );
        if ( RCt == 0 ) {
            RCt = KNamelistCount ( List, & ListCount );
            if ( RCt == 0 ) {
                for ( Idx = 0; Idx < ListCount; Idx ++ ) {
                    RCt = KNamelistGet ( List, Idx, & Name );
                    if ( RCt == 0 ) {
                        RCt = _LoadKartItem ( Directory, Node, Name );
                    }
                    if ( RCt != 0 ) {
                        break;
                    }
                }
            }

            KNamelistRelease ( List );
        }

        KDirectoryRelease ( Directory );
    }

#endif /* JOJOBA */

    return RCt;
}   /* _LoadKarts () */

static
rc_t CC
_GapKartsNodeDispose ( struct XFSContNode * self )
{
    rc_t RCt;
    struct XFSGapKartsNode * Node;

    RCt = 0;
    Node = ( struct XFSGapKartsNode * ) self;

    if ( Node == NULL ) {
        return 0;
    }

    if ( Node -> perm != NULL ) {
        free ( ( char * ) Node -> perm );
        Node -> perm = NULL;
    }

    Node -> project_id = 0;

    free ( Node );

    return RCt;
}   /* _GapKartsNodeDispose () */

static
rc_t CC
_GapKartsNodeMake (
                const char * Name,
                uint32_t ProjectId,
                const char * Perm,
                struct XFSGapKartsNode ** Node
)
{
    rc_t RCt;
    struct XFSGapKartsNode * KartNode;

    RCt = 0;
    KartNode = NULL;

    if ( Node != NULL ) { 
        * Node = NULL;
    }

    if ( Node == NULL || Name == NULL ) {
        return XFS_RC ( rcNull );
    }

    KartNode = calloc ( 1, sizeof ( struct XFSGapKartsNode ) );
    if ( KartNode == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFSContNodeInit (
                            & ( KartNode -> node . node ),
                            Name,
                            Perm,
                            _sFlavorOfKartCollection,
                            _GapKartsNodeDispose
                            );
        if ( RCt == 0 ) {
            if ( Perm != NULL ) {
                RCt = XFS_StrDup ( Perm, & ( KartNode -> perm ) );
            }
            if ( RCt == 0 ) {
                * Node = KartNode;
            }
        }
    }

    return RCt;
}   /* _GapKartsNodeMake () */

LIB_EXPORT
rc_t CC
XFSDbGapKartsNodeMake (
            struct XFSNode ** Node,
            const char * Name,
            const char * ComaSeparatedPaths,
            uint32_t ProjectId,
            const char * Perm
)
{
    rc_t RCt;
    struct XFSGapKartsNode * KartNode;

    RCt = 0;
    KartNode = NULL;

    if ( Node != NULL ) { 
        * Node = NULL;
    }

    if ( Node == NULL || Name == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = _GapKartsNodeMake (
                            Name,
                            ProjectId,
                            Perm,
                            & KartNode
                            );
    if ( RCt == 0 ) {
        RCt = _LoadKarts ( KartNode, ComaSeparatedPaths );
        if ( RCt == 0 ) {
            * Node = & ( KartNode -> node . node );
        }
    }

    if ( RCt != 0 ) {
        * Node = NULL;

        if ( KartNode != NULL ) {
            XFSNodeDispose ( & ( KartNode -> node . node ) );
            KartNode = NULL;
        }
    }

/*
printf ( "XFSKartNodeMake ND[0x%p] NM[%s] TP[%d]\n", ( void * ) Node, Name, Type );
*/

    return RCt;
}   /* XFSDbGapKartsNodeMake () */

static
rc_t CC
_GapKartsNodeMakeFromModel (
                    const char * Name,
                    const struct XFSModel * Model,
                    const struct XFSModelNode * Template,
                    struct XFSNode ** Node
)
{
    rc_t RCt;
    struct XFSGapKartsNode * KartNode;
    const char * TempStr;
    uint32_t ProjectId;

    RCt = 0;
    KartNode = NULL;
    TempStr = NULL;
    ProjectId = 0;

    if ( Node != NULL ) {
        * Node = NULL;
    }

    if ( Name == NULL || Model == NULL || Template == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    TempStr = XFSModelNodeProperty ( Template, XFS_MODEL_PROJECTID );
    if ( TempStr == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    ProjectId = atol ( TempStr );
    if ( ProjectId == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = _GapKartsNodeMake (
                Name,
                ProjectId,
                XFSModelNodeSecurity ( Template ),
                & KartNode
                );
    if ( RCt == 0 ) {
        RCt = _LoadKarts (
                KartNode,
                XFSModelNodeProperty ( Template, XFS_MODEL_KARTFILES )
                );
        if ( RCt == 0 ) {
            * Node = & ( KartNode -> node . node );
        }
    }

    if ( RCt != 0 ) {
        * Node = NULL;

        if ( KartNode != NULL ) {
            XFSNodeDispose ( & ( KartNode -> node . node ) );
            KartNode = NULL;
        }
    }

/*
printf ( "KartNodeMakeFromModel ND[0x%p] NM[%s] TP[%d]\n", ( void * ) Node, Name, Type );
*/

    return RCt;
}   /* _GapKartsNodeMakeFromModel () */

/*)))
 |||
 +++  There KartNode Find
 |||
(((*/

/*)))
 |||
 +++  There KartItemNode Find
 |||
(((*/

/*)))
 |||
 +++  Unified DirEditor
 |||
(((*/

/*)))
 |||
 +++  Unified Discribe
 |||
(((*/

/*)))
 |||
 +++    FileNode lives here
 |||
(((*/

/*))
 ((     Node make/dispose
  ))
 ((*/

static
rc_t CC
_GapKartsNodeConstructorEx (
                        const struct XFSModel * Model,
                        const struct XFSModelNode * Template,
                        const char * Alias,
                        XFSNType Type,
                        const struct XFSNode ** Node
)
{
    rc_t RCt;
    struct XFSNode * TheNode;
    const char * NodeName;

    RCt = 0;
    TheNode = NULL;
    NodeName = NULL;

    if ( Model == NULL || Template == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Node = NULL;

    NodeName = Alias == NULL ? XFSModelNodeName ( Template ) : Alias;

    RCt = _GapKartsNodeMakeFromModel (
                                    NodeName,
                                    Model,
                                    Template,
                                    & TheNode
                                    );

    if ( RCt == 0 ) {
        * Node = TheNode;
    }
    else {
        * Node = NULL;

        if ( TheNode != NULL ) {
            XFSNodeDispose ( TheNode );
        }
    }

    return RCt;
}   /* _GapKartsNodeConstructorEx () */

/*)))
 |||
 +++    FileNode has a Teleport, and it is HERE
 |||
(((*/
static
rc_t CC
_GapKartsNodeConstructor (
                        const struct XFSModel * Model,
                        const struct XFSModelNode * Template,
                        const char * Alias,
                        const struct XFSNode ** Node
)
{
    rc_t RCt = 0;

    RCt = _GapKartsNodeConstructorEx (
                            Model,
                            Template,
                            Alias,
                            kxfsFile,
                            Node
                            );

/*
printf ( "_GapKartsNodeConstructor ( 0x%p, 0x%p (\"%s\"), \"%s\" )\n", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/


    return RCt;
}   /* _GapKartsNodeConstructor () */

static
rc_t CC
_GapKartsNodeValidator (
                        const struct XFSModel * Model,
                        const struct XFSModelNode * Template,
                        const char * Alias,
                        uint32_t Flags
)
{
    rc_t RCt;

    RCt = 0;

/*
printf ( "_GapKartsNodeValidator ( 0x%p, 0x%p (\"%s\"), \"%s\" )\n", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _GapKartsNodeValidator () */

static const struct XFSTeleport _sGapKartsNodeTeleport = {
                                        _GapKartsNodeConstructor,
                                        _GapKartsNodeValidator,
                                        false
                                        };


LIB_EXPORT
rc_t CC
XFSGapKartsProvider ( const struct XFSTeleport ** Teleport )
{
    if ( Teleport == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Teleport = & _sGapKartsNodeTeleport;

    return 0;
}   /* XFSGapKartsProvider () */
