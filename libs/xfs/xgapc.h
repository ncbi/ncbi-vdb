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

#ifndef _xgapc_h_
#define _xgapc_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */

/*))))
 ((((   There is interface to cached files Gap and Public
  ))))  Called it XFSGapCache, sorry, weak imagination
 ((((*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct XFSGapCacheEntry;
struct XFSGapCache;
struct KNamelist;

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*))
  ||    CacheEntry : user could not create or destroy it, only change
  ||    referrence count. Item contains following data:
  ||
  ||       path - real path to entry object
  ||       type - type of entry object: File, Dir or Cache
  ||       handle - for entry type Cache it is instance of Cache
  ||
  ||    Really I am not sure about Type and Handle, but ... as 
  ||    temporary solution ...
  ||
 ((*/
typedef enum XFSCEType {
    kxfsceInvalid = 0,
    kxfsceFile,
    kxfsceDir,
    kxfsceCache,
    kxfsceBroken
} XFSCEType; 

XFS_EXTERN rc_t CC XFSGapCacheEntryAddRef (
                                const struct XFSGapCacheEntry * self
                                );

XFS_EXTERN rc_t CC XFSGapCacheEntryRelease (
                                const struct XFSGapCacheEntry * self
                                );

XFS_EXTERN const char * CC XFSGapCacheEntryName (
                                const struct XFSGapCacheEntry * self
                                );

XFS_EXTERN const char * CC XFSGapCacheEntryPath (
                                const struct XFSGapCacheEntry * self
                                );

XFS_EXTERN XFSCEType CC XFSGapCacheEntryType (
                                const struct XFSGapCacheEntry * self
                                );

XFS_EXTERN const void * CC XFSGapCacheEntryHandle (
                                const struct XFSGapCacheEntry * self
                                );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*))
  ||    Cache : collection of CacheEntries.
  ||
 ((*/
XFS_EXTERN rc_t CC XFSGapCacheMake (
                                const struct XFSGapCache ** Cache,
                                const char * Path,
                                bool Terminal
                                );

XFS_EXTERN rc_t CC XFSGapCacheDispose (
                                const struct XFSGapCache * self
                                );

XFS_EXTERN rc_t CC XFSGapCacheVersion (
                                const struct XFSGapCache * self,
                                uint64_t * Version
                                );

XFS_EXTERN bool CC XFSGapCacheIsEmpty (
                                const struct XFSGapCache * self
                                );

XFS_EXTERN rc_t CC XFSGapCacheList (
                                const struct XFSGapCache * self,
                                const struct KNamelist ** List
                                );

XFS_EXTERN rc_t CC XFSGapCacheFind (
                                const struct XFSGapCache * self,
                                const struct XFSGapCacheEntry ** Entry,
                                const char * Name
                                );

XFS_EXTERN rc_t CC XFSGapCacheRefresh (
                                const struct XFSGapCache * self
                                );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/ 
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _xgapc_h_ */
