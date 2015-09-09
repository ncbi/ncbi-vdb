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

#ifndef _xgapprj_tmp_h_
#define _xgapprj_tmp_h_

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
 * That file contains set of structures which will implement FUSE/DOKAN
 * access to dbGaP data.
 * 
 * NOTE: because of temporary nature of that package, all names will
 *       hafe suffix '_Temp'
 *
 * PRIMITEVES:
 *
 * XFSDbGapProjectDepot_Temp - set of available dbGaP projects.
 *    1) should be inited on start with XFSDbGapProjectDepotInit_Temp,
 *       and should be disposed with XFSDbGapProjectDispose_Temp
 *    2) provides methods: Has, FindOrCreate, Delete by dbGaP ProjectId
 *    3) thread safe
 *    4) struct itself hidden
 *
 * XFSDbGapProject_Temp - dbGaP project. Unique by ProjectId (uint32_t)
 *    1) general purpose to provide access to dbGaP data by ItemId or
 *       Accession. 
 *    2) cashes KFile to dbGaP data, and provides operations Open/Close/         *       Read.
 *    3) thread safe. Access to partial files is synchronized and to
 *       downloaded is unsynchronized
 *
 * XFSDbGapKart_Temp - user kart containing refferences to dbGaP objects
 *    1) returns list of items by dbGap ProjectId
 *
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))
 //     Forwards
((*/
struct XFSDbGapProject_Temp;

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))
 //     XFSDbGapProjectDepot_Temp
((*/
XFS_EXTERN rc_t CC XFSDbGapProjectDepotInit_Temp ();
XFS_EXTERN rc_t CC XFSDbGapProjectDepotDispose_Temp ();

    /*  Don't know if we need that method. Added for a sake of sake
     *  Return true if Project with such ID exists
     */
XFS_EXTERN bool CC XFSDbGapProjectDepotHas_Temp (
                                                uint32_t ProjectId
                                                );
    /*  Usint method FindOrCreate, to avoid collisions from different
     *  creative people :lol:
     */
XFS_EXTERN rc_t CC XFSDbGapProjectDepotFindOrCreate_Temp (
                                uint32_t ProjectId,
                                struct XFSDbGapProject_Temp ** Project
                                );
    /*  Don't know if we need that method. Added for a sake of sake
     */
XFS_EXTERN rc_t CC XFSDbGapProjectDepotDelete_Temp (
                                                uint32_t ProjectId
                                                );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*))
 //     XFSDbGapProject_Temp
((*/

XFS_EXTERN rc_t CC XFSDbGapProjectAddRef_Temp (
                            const struct XFSDbGapProject_Temp * self
                            );
XFS_EXTERN rc_t CC XFSDbGapProjectRelease_Temp (
                            const struct XFSDbGapProject_Temp * self
                            );

XFS_EXTERN rc_t CC XFSDbGapProjectID_Temp (
                            const struct XFSDbGapProject_Temp * self,
                            uint32_t * Id
                            );
XFS_EXTERN rc_t CC XFSDbGapProjectOpenFileRead_Temp (
                            const struct XFSDbGapProject_Temp * self,
                            const char * AccessionOrObjectID,
                            const struct KFile ** File
                            );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*))
 //     XFSDbGapKart_Temp
((*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _xgapprj_tmp_h_ */
