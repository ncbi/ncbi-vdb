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

#ifndef _xtar_h_
#define _xtar_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */

/*))))
 ((((   There is some interface build to implement access to files
  ))))  through HTTP
 ((((*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct XFSTar;
struct XFSTarEntry;
struct XFSTarReader;
struct KNamelist;

/*))
 //     Tar depot ... initializing depot for Tar archives
((*/
XFS_EXTERN rc_t CC XFSTarDepotInit ();
XFS_EXTERN rc_t CC XFSTarDepotDispose ();
XFS_EXTERN rc_t CC XFSTarDepotClear ();

/*))
 ||     Tar archive make/dispose
 || There is no ArrRef method. FindOrCreate will add referrence to
 || archive each time when it called. If refcount was zero, the 
 || tar archive will be opened for reading. The call of Release 
 || method reduce refcount, and, if resulting count will became zero,
 || tar archive will be closed.
 || To remove tar archive from depot user should use Dispose method.
((*/
XFS_EXTERN rc_t CC XFSTarFindOrCreate (
                            const char * Resource,
                            const struct XFSTar ** Tar
                            );
XFS_EXTERN rc_t CC XFSTarRelease (
                            const struct XFSTar * self
                            );
XFS_EXTERN rc_t CC XFSTarDispose (
                            const struct XFSTar * self
                            );

/*))
 //     Accessing TarItems
((*/

/*||
  ||    Note, while accessing TarEntry by Has adn Get methods
  ||    the Path relative to Tar base URL should be used
  ||*/
XFS_EXTERN const char * CC XFSTarSource (
                            const struct XFSTar * self
                            );
XFS_EXTERN rc_t CC XFSTarGetEntry (
                            const struct XFSTar * self,
                            const char * Path,
                            const struct XFSTarEntry ** Entry
                            );

/*))
 //     TarEntry content
((*/

XFS_EXTERN rc_t CC XFSTarEntryAddRef (
                            const struct XFSTarEntry * self
                            );
XFS_EXTERN rc_t CC XFSTarEntryRelease (
                            const struct XFSTarEntry * self
                            );

XFS_EXTERN bool CC XFSTarEntryIsFolder (
                            const struct XFSTarEntry * self
                            );
XFS_EXTERN const char * CC XFSTarEntryName (
                            const struct XFSTarEntry * self
                            );
XFS_EXTERN const char * CC XFSTarEntryPath (
                            const struct XFSTarEntry * self
                            );
XFS_EXTERN rc_t CC XFSTarEntryList (
                            const struct XFSTarEntry * self,
                            struct KNamelist ** List
                            );
XFS_EXTERN rc_t CC XFSTarEntrySize (
                            const struct XFSTarEntry * self,
                            uint64_t * Size
                            );
XFS_EXTERN rc_t CC XFSTarEntryTime (
                            const struct XFSTarEntry * self,
                            KTime_t * Time
                            );
XFS_EXTERN bool CC XFSTarEntryGood (
                            const struct XFSTarEntry * self
                            );
XFS_EXTERN rc_t CC XFSTarEntryGetChild (
                            const struct XFSTarEntry * self,
                            const char * ChildName,
                            const struct XFSTarEntry ** Child
                            );
/*))
 //     There will no special reader. Entry reads.
((*/
XFS_EXTERN bool CC XFSTarEntryIsOpen (
                            const struct XFSTarEntry * self
                            );

XFS_EXTERN rc_t CC XFSTarEntryOpen (
                            const struct XFSTarEntry * self
                            );

XFS_EXTERN rc_t CC XFSTarEntryRead (
                            const struct XFSTarEntry * self,
                            uint64_t Offset,
                            void * Buffer,
                            size_t BufferSize,
                            size_t * NumRead
                            );

XFS_EXTERN rc_t CC XFSTarEntryClose (
                            const struct XFSTarEntry * self
                            );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _xtar_h_ */
