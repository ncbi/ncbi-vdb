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

#ifndef _xhttp_h_
#define _xhttp_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */

/*))))
 ((((   There is some interface build to implement access to files
  ))))  through HTTP
 ((((*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct XFSHttp;
struct XFSHttpEntry;
struct XFSHttpReader;
struct KNamelist;

/*))
 //     Standard method of loading/uploading
((*/
XFS_EXTERN rc_t CC XFSHttpMake (
                            const char * BaseUrl,
                            const struct XFSHttp ** Http
                            );
XFS_EXTERN rc_t CC XFSHttpDispose (
                            const struct XFSHttp * self
                            );

XFS_EXTERN rc_t CC XFSHttpAddRef (
                            const struct XFSHttp * self
                            );
XFS_EXTERN rc_t CC XFSHttpRelease (
                            const struct XFSHttp * self
                            );

/*))
 //     Accessing HttpItems
((*/

/*||
  ||    Note, while accessing HttpEntry by Has adn Get methods
  ||    the Path relative to Http base URL should be used
  ||*/
XFS_EXTERN const char * CC XFSHttpBaseUrl (
                            const struct XFSHttp * self
                            );
XFS_EXTERN bool CC XFSHttpHasEntry (
                            const struct XFSHttp * self,
                            const char * Path
                            );
XFS_EXTERN const struct XFSHttpEntry * CC XFSHttpGetEntry (
                            const struct XFSHttp * self,
                            const char * Path
                            );
XFS_EXTERN rc_t CC XFSHttpGetOrCreateEntry (
                            const struct XFSHttp * self,
                            const char * Path,
                            const struct XFSHttpEntry ** Entry
                            );

/*))
 //     HttpEntry content
((*/

XFS_EXTERN rc_t CC XFSHttpEntryAddRef (
                            const struct XFSHttpEntry * self
                            );
XFS_EXTERN rc_t CC XFSHttpEntryRelease (
                            const struct XFSHttpEntry * self
                            );

XFS_EXTERN bool CC XFSHttpEntryIsFolder (
                            const struct XFSHttpEntry * self
                            );
XFS_EXTERN const char * CC XFSHttpEntryName (
                            const struct XFSHttpEntry * self
                            );
XFS_EXTERN const char * CC XFSHttpEntryUrl (
                            const struct XFSHttpEntry * self
                            );
XFS_EXTERN uint32_t CC XFSHttpEntryUrlHash (
                            const struct XFSHttpEntry * self
                            );
XFS_EXTERN rc_t CC XFSHttpEntryList (
                            const struct XFSHttpEntry * self,
                            struct KNamelist ** List
                            );
XFS_EXTERN rc_t CC XFSHttpEntrySize (
                            const struct XFSHttpEntry * self,
                            uint64_t * Size
                            );
XFS_EXTERN rc_t CC XFSHttpEntryTime (
                            const struct XFSHttpEntry * self,
                            KTime_t * Time
                            );
XFS_EXTERN bool CC XFSHttpEntryGood (
                            const struct XFSHttpEntry * self
                            );
    /*))    Access to children nodes by name
     ((*/
XFS_EXTERN bool CC XFSHttpEntryHas (
                            const struct XFSHttpEntry * self,
                            const char * ChildName
                            );
XFS_EXTERN rc_t CC XFSHttpEntryGet (
                            const struct XFSHttpEntry * self,
                            const char * ChildName,
                            const struct XFSHttpEntry ** Child
                            );

/*))
 //     Special treatment
((*/
XFS_EXTERN rc_t CC XFSHttpRehash ();

/*))
 //     XFSHttpReader
((*/
XFS_EXTERN rc_t CC XFSHttpReaderMake (
                            const struct XFSHttpEntry * Entry,
                            const struct XFSHttpReader ** Reader
                            );

XFS_EXTERN rc_t CC XFSHttpReaderAddRef (
                            const struct XFSHttpReader * self
                            );
XFS_EXTERN rc_t CC XFSHttpReaderRelease (
                            const struct XFSHttpReader * self
                            );

XFS_EXTERN rc_t CC XFSHttpReaderRead (
                            const struct XFSHttpReader * self,
                            uint64_t Offset,
                            void * Buffer,
                            size_t BufferSize,
                            size_t * NumRead
                            );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _xhttp_h_ */
