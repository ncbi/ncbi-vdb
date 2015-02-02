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

#ifndef _xencro_h_
#define _xencro_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */

/*))))
 ((((   There is some interface build to implement access to files
  ))))  through Krypto package
 ((((*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct XFSEncEntry;

/*))
 //     We are cacheing info: key, size and date of encrypted file
((*/
XFS_EXTERN rc_t CC XFSEncDepotInit ();
XFS_EXTERN rc_t CC XFSEncDepotDispose ();
XFS_EXTERN rc_t CC XFSEncDepotClear ();

/*))
 || Just handles structure which reffer to encrypted file
 || IMPORTANT: we are ReadOnlyMode only, which does means everything
 || Also, Release method does not destroy EncEntry, only Dispose 
 || call will do it. Release method will close associated file only
 || 
((*/
XFS_EXTERN rc_t CC XFSEncEntryFindOrCreate (
                            const char * Path,
                            const char * Passwd,
                            const char * EncodeType, /* could be NULL */
                            const struct XFSEncEntry ** EncEntry
                            );
XFS_EXTERN rc_t CC XFSEncEntryDispose (
                            const struct XFSEncEntry * self
                            );

XFS_EXTERN rc_t CC XFSEncEntryAddRef (
                            const struct XFSEncEntry * self
                            );
XFS_EXTERN rc_t CC XFSEncEntryRelease (
                            const struct XFSEncEntry * self
                            );


XFS_EXTERN const char * CC XFSEncEntryPath (
                            const struct XFSEncEntry * self
                            );
XFS_EXTERN rc_t CC XFSEncEntrySize (
                            const struct XFSEncEntry * self,
                            uint64_t * Size
                            );
XFS_EXTERN rc_t CC XFSEncEntryTime (
                            const struct XFSEncEntry * self,
                            KTime_t * Time
                            );
XFS_EXTERN bool CC XFSEncEntryGood (
                            const struct XFSEncEntry * self
                            );
/*))
 //     There will no special reader. Entry reads.
((*/
XFS_EXTERN bool CC XFSEncEntryIsOpen (
                            const struct XFSEncEntry * self
                            );

XFS_EXTERN rc_t CC XFSEncEntryOpen (
                            const struct XFSEncEntry * self
                            );
XFS_EXTERN rc_t CC XFSEncEntryClose (
                            const struct XFSEncEntry * self
                            );

XFS_EXTERN rc_t CC XFSEncEntryRead (
                            const struct XFSEncEntry * self,
                            uint64_t Offset,
                            void * Buffer,
                            size_t BufferSize,
                            size_t * NumRead
                            );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _xencro_h_ */
