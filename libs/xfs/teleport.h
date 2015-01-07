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

#ifndef _teleport_h_
#define _teleport_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */

/*))))
 ((((   This is unlegit file, which is access point to methods
  ))))  which are needed to construct XFSNode from XFSModelNode
 ((((   Will change it in future, but it is right now that way
  ))))
 ((((*/

/*))
 //  Forwards
((*/
struct XFSTeleport;
struct XFSModel;
struct XFSModelNode;
struct XFSNode;

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))))
 ////   Constructor, and validator. Think that it more than enough
((((*/
typedef rc_t ( CC * XFSNodeC_t ) (
                            const struct XFSModel * Model,
                            const struct XFSModelNode * Template,
                            const char * Alias,
                            const struct XFSNode ** Node
                            );

typedef rc_t ( CC * XFSNodeV_t ) (
                            const struct XFSModel * Model,
                            const struct XFSModelNode * Template,
                            const char * Alias,
                            uint32_t Flags
                            );

typedef rc_t ( CC * XFSTeleportProvider_t ) (
                            const struct XFSTeleport ** Teleport
                            );

/*))))
 ((((   Each type of node constructor and volidator by name
  ))))  Constructor could not be NULL, but Validator could.
 ((((*/
struct XFSTeleport {
        /* ??? mb. version */
    XFSNodeC_t Constructor;
    XFSNodeV_t Validator;

    bool DeleteOnWhack;
};

/*))))
 ////   Here we are going... I mean, getting teleport by name
((((*/
XFS_EXTERN
rc_t CC
XFSTeleportLookup (
            const char * NodeType,
            const struct XFSTeleport ** Teleport
);

/*))))
 ////   Here we are initem'n'disposing Teleport
((((*/
XFS_EXTERN rc_t CC XFSTeleportInit ();
XFS_EXTERN rc_t CC XFSTeleportDispose ();

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _teleport_h_ */
