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
#include <klib/refcount.h>
#include <klib/printf.h>
#include <klib/log.h>

#include <xfs/model.h>
#include <xfs/node.h>
#include <xfs/doc.h>


#include "teleport.h"
#include "common.h"

#include <sysalloc.h>


/*)))
 |||
 +++    ReadMeNode lives here
 |||
(((*/

/*))
 ((     Node make/dispose
  ))
 ((*/

static
rc_t CC
_ReadMeNodeConstructor (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            const struct XFSNode ** Node
)
{
    rc_t RCt;
    struct XFSNode * TheNode;
    const char * NodeName;
    const char * TextOrSource;
    struct XFSDoc * ReadMe;

    RCt = 0;
    TheNode = NULL;
    NodeName = NULL;
    TextOrSource = NULL;
    ReadMe = NULL;

    if ( Node != NULL ) {
        * Node = NULL;
    }

    if ( Model == NULL || Template == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    NodeName = Alias == NULL ? XFSModelNodeName ( Template ) : Alias;

    /*) There could be "text" and "source" parameters, we will leave
      | source for a future development then, and will use it as text
      (*/
    TextOrSource = XFSModelNodeProperty ( Template, XFS_MODEL_TEXT );
    if ( TextOrSource == NULL ) {
        TextOrSource = XFSModelNodeProperty ( Template, XFS_MODEL_SOURCE );
    }

    if ( TextOrSource == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = XFSTextDocMake ( & ReadMe );
    if ( RCt == 0 ) {

        RCt = XFSTextDocAppend ( ReadMe, "README:\n%s\n", TextOrSource );
        if ( RCt == 0 ) {
            RCt = XFSDocNodeMakeWithFlavor (
                                & TheNode,
                                ReadMe,
                                NodeName,
                                XFSModelNodeSecurity ( Template ),
                                _sFlavorOfReadMe
                                );
            if ( RCt == 0 ) {
                * Node = TheNode;
            }
            else {
                if ( TheNode != NULL ) {
                    XFSNodeDispose ( TheNode );
                }
            }
        }

        XFSDocRelease ( ReadMe );
    }

    return RCt;
}   /* _ReadMeNodeConstructor () */

/*)))
 |||
 +++    ReadMeNode has a Teleport, and it is HERE
 |||
(((*/
static
rc_t CC
_ReadMeConstructor (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            const struct XFSNode ** Node
)
{
    rc_t RCt;

    RCt = _ReadMeNodeConstructor (
                            Model,
                            Template,
                            Alias,
                            Node
                            );

/*
pLogMsg ( klogDebug, "_ReadMeConstructor ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,templat=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _ReadMeConstructor () */

static
rc_t CC
_ReadMeValidator (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            uint32_t Flags
)
{
    rc_t RCt;

    RCt = 0;

/*
pLogMsg ( klogDebug, "_ReadMeValidator ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,templat=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _ReadMeValidator () */

static const struct XFSTeleport _sReadMeTeleport = {
                                            _ReadMeConstructor,
                                            _ReadMeValidator,
                                            false
                                            };


LIB_EXPORT
rc_t CC
XFSReadMeProvider ( const struct XFSTeleport ** Teleport )
{
    if ( Teleport == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Teleport = & _sReadMeTeleport;

    return 0;
}   /* XFSReadMeProvider () */

