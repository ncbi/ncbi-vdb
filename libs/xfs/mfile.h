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

#ifndef _memfile_h_
#define _memfile_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */

/*))))  Some sick stuff, I need to use for rendering some nodes
 ((((*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * LYRICS:
 *
 * That file which is storing data in memory XFSMFile .
 * Originally, user should create XFSMFilePod, which will be passed
 * to XFSMFileMake method as argument. Pod contains file data and
 * should be created and deleted separately.
 *
 * NOTE : THREAD UNSAFE
 *
 * Note : I need it fast, so will revisit it later, so will attach
 *        some interface later.
 *
 * Nott : Now opens only in Update mode.
 * 
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))
  ||    XFSMFilePod structure and other
  ((*/
struct XFSMFilePod;

/*|     InitialSize could be zero 
  |     BlockSize, if zero, became default value 8k. Also, it is size
  |     of increment value for Pod Data
  |*/
XFS_EXTERN rc_t CC XFSMFilePodMake (
                                struct XFSMFilePod ** Pod,
                                uint64_t InitialSize,
                                uint32_t BlockSize
                                );
XFS_EXTERN rc_t CC XFSMFilePodAddRef (
                                const struct XFSMFilePod * self
                                );
XFS_EXTERN rc_t CC XFSMFilePodRelease (
                                const struct XFSMFilePod * self
                                );
XFS_EXTERN rc_t CC XFSMFilePodData (
                                const struct XFSMFilePod * self,
                                const char ** Data
                                );
XFS_EXTERN rc_t CC XFSMFilePodDataSize (
                                const struct XFSMFilePod * self,
                                uint64_t * Size
                                );
/*))
  ||    XFSMFile.
  ||    That method will create memory file, and 
  ||
  ((*/
XFS_EXTERN rc_t CC XFSMFileMake (
                                struct KFile ** File,
                                struct XFSMFilePod * Pod
                                );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _memfile_h_ */
