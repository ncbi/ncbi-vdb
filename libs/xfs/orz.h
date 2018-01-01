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

#ifndef _orz_h_
#define _orz_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */

/*))))
 ((((   There is some thing which suppose to be a temporary stuff
  ))))  of viewer for dbGaP projects
 ((((*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * LYRICS:
 *
 * That file contains asynchronous resolver stuff.
 * 
 * Original version of dbGaP viewer used Resolver v1.0, which could
 * resolve object one by one. That does not work for large amoutns of
 * object. For example, some Karts could containd tens of thousands
 * objects. The new protocol v3.0 was developed, which allows to submit
 * many objects to resolve. The implementation of new protocol was 
 * faster in about 10-20 times, which is not satisfactory. So, we
 * decided to do asynchronous object resolving. There will be a FIFO
 * where object IDs will be put. There will be thread started, which
 * will resolve information for object IDs by small portions ( N = 50 ).
 * There will be BSTree with resolved information. Prolly that amount
 * will be adaptive ...
 *
 * GapObjects are represented by XFSGapObject, and it contains: name, 
 * ObjectId ( string: accession or id ), remote url, cache path,
 * modified time, size and md5. GapObjects could be retrieved by name
 * only. There is no way to receive full list of objects :MWAHAHAH:
 *
 * XFSGapResolver is a singleton for resolved objects. It should be
 * Initialized before resolving, and Disposed at the end of show.
 * BTW, the instance of resolver does not accessible for users
 *
 * Looks like all those are thread safe, not sure yet :LOL:
 *
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))
 // Defines
((*/

/*))
 // Forwards
((*/
struct XFSGapObject;

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * Some explanations: Start/Stop methods will keep content of resolved
 * objects, when Init/Dispose will reset that content
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*)))
 ///    XFSGapResolver : Init / Dispose and Start / Stop
 ///    If Start method will be called withoud initialisation, it will
 ///    initialize new thread.
 ///    If Dispose method will be called without stopping thread, it
 ///    will stop thread.
(((*/
XFS_EXTERN rc_t CC XFSGapResolverInit ();
XFS_EXTERN rc_t CC XFSGapResolverStart ();
XFS_EXTERN rc_t CC XFSGapResolverStop ();
XFS_EXTERN rc_t CC XFSGapResolverDispose ();

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))
 ///    Accessing different data from XFSGapResolver
(((*/

        /*  GapObjects are distincts by AccessionOrId ...
         */
XFS_EXTERN rc_t CC XFSGapResolverGetObject (
                                const struct XFSGapObject ** Object,
                                const char * AccessionOrId
                                );

XFS_EXTERN bool CC XFSGapResolverHasObject (
                                const char * AccessionOrId
                                );

        /*  Putting accesstion to a Id and ProjectId
         */
XFS_EXTERN rc_t CC XFSGapResolverAddToResolve (
                                uint32_t ProjectId,
                                const char * AccessionOrId
                                );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*))
 //     XFSGOSts ( XFSGapObject Status )
((*/
typedef enum XFSGOSts {
    kgosInvalid = 0,
    kgosReady,
    kgosResolved,
    kgosGood = kgosResolved,
    kgosBroken
}   XFSGOSts;

/*))
 //     XFSGapObject
((*/
XFS_EXTERN rc_t CC XFSGapObjectAddRef (
                                const struct XFSGapObject * self
                                );

XFS_EXTERN rc_t CC XFSGapObjectRelease (
                                const struct XFSGapObject * self
                                );

XFS_EXTERN rc_t CC XFSGapObjectId (
                                const struct XFSGapObject * self,
                                const char ** AccessionOrId
                                );

XFS_EXTERN rc_t CC XFSGapObjectURL (
                                const struct XFSGapObject * self,
                                const char ** Url
                                );

XFS_EXTERN rc_t CC XFSGapObjectCachePath (
                                const struct XFSGapObject * self,
                                const char ** CachePath
                                );

XFS_EXTERN rc_t CC XFSGapObjectModTime (
                                const struct XFSGapObject * self,
                                KTime_t * ModTime
                                );

XFS_EXTERN rc_t CC XFSGapObjectSize (
                                const struct XFSGapObject * self,
                                uint64_t * Size
                                );

XFS_EXTERN bool CC XFSGapObjectGood (
                                const struct XFSGapObject * self
                                );

XFS_EXTERN rc_t CC XFSGapObjectStatus (
                                const struct XFSGapObject * self,
                                enum XFSGOSts * Status
                                );

XFS_EXTERN rc_t CC XFSGapObjectRcAndMsg (
                                const struct XFSGapObject * self,
                                rc_t * RC,
                                const char ** Msg
                                );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _orz_h_ */
