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

#ifndef _lreader_h_
#define _lreader_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */

/*))))
 ((((   Simple line reader
  ))))
 ((((*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* Lyrics:                                                           */
/* Liner reader creates KMMap over KFile and traverces trhough map   */
/* NOTE: it translates file position, eg. it cuts '\n' at the end    */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))    Forwards
 ((*/
struct XFSLineReader;
struct KFile;

/*))    Opening/creating/disposing
 ((*/
XFS_EXTERN rc_t CC XFSLineReaderOpen (
                                const struct XFSLineReader ** Reader,
                                const char * Path,
                                ...
                                );
XFS_EXTERN rc_t CC XFSLineReaderVOpen (
                                const struct XFSLineReader ** Reader,
                                const char * Path,
                                va_list Args
                                );
XFS_EXTERN rc_t CC XFSLineReaderMake (
                                const struct KFile * File,
                                const struct XFSLineReader ** Reader
                                );

XFS_EXTERN rc_t CC XFSLineReaderAddRef (
                                const struct XFSLineReader * self
                                );
XFS_EXTERN rc_t CC XFSLineReaderRelease (
                                const struct XFSLineReader * self
                                );

/*))    Using and data accessing
 ((*/
XFS_EXTERN bool CC XFSLineReaderGood (
                                const struct XFSLineReader * self
                                );

    /*  If it is possible to fetch a line it moves cursor to next
     *  line and returns true. Overvise it will return false.
     */
XFS_EXTERN bool CC XFSLineReaderNext (
                                const struct XFSLineReader * self
                                );

    /*  Fetches line at current cursor and fills OutLine structure
     */
XFS_EXTERN rc_t CC XFSLineReaderGet (
                                const struct XFSLineReader * self,
                                const struct String * OutLine
                                );

    /*  Return number of line where cursor is set
     */
XFS_EXTERN rc_t CC XFSLineReaderLineNo (
                                const struct XFSLineReader * self,
                                size_t * LineNo
                                );

    /*  Set current cursor to begin of map
     */
XFS_EXTERN rc_t CC XFSLineReaderRewind (
                                const struct XFSLineReader * self
                                );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _lreader_h_ */
