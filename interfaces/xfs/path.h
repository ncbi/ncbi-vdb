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

#ifndef _xfs_path_h_
#define _xfs_path_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*))
 ||   Lyrics.
 ++   That file contains lame method for path handling
 ||
((*/

/*)))
 ///   forwards
(((*/
struct KNamelist;
struct XFSPath;

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))
 (((    Apparently we do suppose that we are working with path 
  )))   composed from VPath ... so only '/' as separator
 (((*/
XFS_EXTERN
rc_t CC
XFSPathMake ( const char * InPath, const struct XFSPath ** OutPath );

XFS_EXTERN
rc_t CC
XFSPathDispose ( const struct XFSPath * self );

XFS_EXTERN
uint32_t CC
XFSPathCount ( const struct XFSPath * self );

XFS_EXTERN
const char * CC
XFSPathGet ( const struct XFSPath * self, uint32_t Index );

XFS_EXTERN
const char * CC
XFSPathName ( const struct XFSPath * self );

XFS_EXTERN
const char * CC
XFSPathGetOrigin ( const struct XFSPath * self );

    /*)     This one is last exclusive
     (*/
XFS_EXTERN
rc_t CC
XFSPathTo (
        const struct XFSPath * self,
        uint32_t Index,
        char * Buffer,
        size_t BufferSize
        );

    /*)     This one is first inclusive
     (*/
XFS_EXTERN
rc_t CC
XFSPathFrom (
        const struct XFSPath * self,
        uint32_t Index,
        char * Buffer,
        size_t BufferSize
        );

    /*)     Debug only
     (*/
XFS_EXTERN
rc_t CC
XFSPatthDump ( const struct XFSPath * self );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _xfs_path_h_ */
