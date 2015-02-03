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

#ifndef _xfs_model_h_
#define _xfs_model_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*))
 ||   Lyrics.
 ++   That file contains interfaces for model which contains formal
 ||   description for XFS Tree.
((*/

/*)))
 ///   XFSModel and other forwards.
(((*/
struct XFSModel;
struct XFSModelNode;
struct KNamelist;

/*))    Common name defines for model property names
 ((*/
#define XFS_MODEL_SOURCE    "source"
#define XFS_MODEL_TEXT      "text"

/*)))
 ///   Methods
(((*/
XFS_EXTERN
const char * CC
XFSModelNodeName ( const struct XFSModelNode * self );

XFS_EXTERN
const char * CC
XFSModelNodeType ( const struct XFSModelNode * self );

XFS_EXTERN
const char * CC
XFSModelNodeAs ( const struct XFSModelNode * self );

XFS_EXTERN
const char * CC
XFSModelNodeLabel ( const struct XFSModelNode * self );

XFS_EXTERN
const char * CC
XFSModelNodeSecurity ( const struct XFSModelNode * self );

XFS_EXTERN
bool CC
XFSModelNodeIsRoot ( const struct XFSModelNode * self );

XFS_EXTERN
rc_t CC
XFSModelNodePropertyNames (
                    const struct XFSModelNode * self,
                    const struct KNamelist ** Properties
                    );

XFS_EXTERN
const char * CC
XFSModelNodeProperty (
                    const struct XFSModelNode * self,
                    const char * PropertyName
                    );

XFS_EXTERN
rc_t CC
XFSModelNodeChildrenNames (
                    const struct XFSModelNode * self,
                    const struct KNamelist ** Children
                    );

XFS_EXTERN
const char * CC
XFSModelNodeChildAlias (
                    const struct XFSModelNode * self,
                    const char * ChildName
                    );

XFS_EXTERN
rc_t CC
XFSModelMake (
            struct XFSModel ** Model,
            const char * Resource,      /* OK to be NULL */
            const char * Version        /* OK to be NULL */
            );

XFS_EXTERN
rc_t CC
XFSModelDispose ( struct XFSModel * self );

XFS_EXTERN
rc_t CC
XFSModelAddRef ( const struct XFSModel * self );

XFS_EXTERN
rc_t CC
XFSModelRelease ( const struct XFSModel * self );

XFS_EXTERN
const struct XFSModelNode * CC
XFSModelRootNode ( const struct XFSModel * self );

XFS_EXTERN
const struct XFSModelNode * CC
XFSModelLookupNode ( const struct XFSModel * self, const char * Name );

XFS_EXTERN
const char * CC
XFSModelResource ( const struct XFSModel * self );

XFS_EXTERN
const char * CC
XFSModelVersion ( const struct XFSModel * self );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
XFS_EXTERN
void CC
XFSModelNodeDDump ( const struct XFSModelNode * self );

XFS_EXTERN
void CC
XFSModelDDump ( const struct XFSModel * self );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _xfs_model_h_ */
