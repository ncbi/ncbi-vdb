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

#ifndef _lockdpt_h_
#define _lockdpt_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */

/*))))
 ((((   I suppose here could be many different items to lock, and I
  ))))  want to reduce amount of mutexes
 ((((
  ))))  In general, there will be array of mutexes, and user will be
 ((((   able to lock/unlock processing by providing the mutex number
  ))))
 ((((*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct XFSLockDepot;

#define XFS_DEFAULT_DEPOT_SIZE  256

    /*)) If DepotSize == 0, it will produce array of
     //  XFS_DEFAULT_DEPOT_SIZE locks
    ((*/
XFS_EXTERN rc_t CC XFSLockDepotMake (
                                uint32_t DepotSize,
                                struct XFSLockDepot ** Depot
                                );
XFS_EXTERN rc_t CC XFSLockDepotDispose (
                                struct XFSLockDepot * self
                                );

XFS_EXTERN rc_t CC XFSLockDepotAcquire (
                                struct XFSLockDepot * self,
                                uint32_t HashValue
                                );
XFS_EXTERN rc_t CC XFSLockDepotUnlock (
                                struct XFSLockDepot * self,
                                uint32_t HashValue
                                );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _lockdpt_h_ */
