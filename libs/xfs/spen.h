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

#ifndef _spen_h_
#define _spen_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * LYRICS:
 *
 * That file contains pool of KHttpFiles, for accessing dbGaP data
 * Due to extremly sophisticated inner world of mine, I am introducing
 * Pen primitive ( not Pool ), and Pen is 'pen'. There is also small
 * donkey inside. Burro ( donkey ) it is structure which contains 
 * pointer to cached/non_cached HTTP file.
 *
 * So, call "XFSPenMake ()" function and be ready for ride.
 *
 * Enjoy.
 * 
 * TODO: make some adjustments, when Kurt will implement timeout
 *       in KFile interface.
 * 
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))
 //     Forwards
((*/
struct XFSPen;
struct XFSBurro;

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))
 //     XFSPen Make/Dispose methods
((*/
rc_t CC XFSPenMake (
                            const struct XFSPen ** Pen,
                            size_t Capacity
                            );
rc_t CC XFSPenDispose ( const struct XFSPen * self );

/*))
 //     Returns object associated with URL
((*/
rc_t CC XFSPenGet (
                            const struct XFSPen * self,
                            const struct XFSBurro ** Burro,
                            const char * Url
                            );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))
 //     XFSBurro methods
((*/

rc_t CC XFSBurroAddRef ( const struct XFSBurro * self );
rc_t CC XFSBurroRelease ( const struct XFSBurro * self );

bool CC XFSBurroGood ( const struct XFSBurro * self );

/*))
 //     Returns 'CACHED' KFile associated with URL
((*/
rc_t CC XFSBurroCachedFile (
                                const struct XFSBurro * self,
                                const struct KFile ** File
                                );

/*))
 //     Drops old 'CACHED' KFile associated with URL, and returns new
((*/
rc_t CC XFSBurroRecachedFile (
                                const struct XFSBurro * self,
                                const struct KFile ** File
                                );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _spen_h_ */
