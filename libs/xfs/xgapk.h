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

#ifndef _xgapk_h_
#define _xgapk_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */

/*))))
 ((((   There is some interface build over Kart structure.
  ))))  Called it XFSGapKart, sorry, weak imagination
 ((((*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct XFSGapKartItem;
struct XFSGapKart;
struct KNamelist;

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*))
  ||    KartItem : user could not create or destroy it, only change
  ||    referrence count. Item contains following data:
  ||
  ||       project_id, accession, object_id, name and description.
  ||
  ||    One method called as DisplayName, it is just Accession or
  ||    Name if Accession equals NULL.
 ((*/
XFS_EXTERN rc_t CC XFSGapKartItemAddRef (
                            const struct XFSGapKartItem * self
                            );
XFS_EXTERN rc_t CC XFSGapKartItemRelease (
                            const struct XFSGapKartItem * self
                            );

XFS_EXTERN uint32_t CC XFSGapKartItemProjectId (
                            const struct XFSGapKartItem * self
                            );
XFS_EXTERN uint32_t CC XFSGapKartItemObjectId (
                            const struct XFSGapKartItem * self
                            );
XFS_EXTERN const char * CC XFSGapKartItemAccession (
                            const struct XFSGapKartItem * self
                            );
XFS_EXTERN const char * CC XFSGapKartItemName (
                            const struct XFSGapKartItem * self
                            );
XFS_EXTERN const char * CC XFSGapKartItemDescription (
                            const struct XFSGapKartItem * self
                            );
    /*  This method will return name which will be used as filename
     *  if Accession != NULL it will be Accession, overwise ItemName
     */
XFS_EXTERN const char * CC XFSGapKartItemDisplayName (
                            const struct XFSGapKartItem * self
                            );


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*))
  ||    Kart : collection of KartItems.
  ||
  ||    Kart content could not be changed, so we may pass it from one
  ||    collection to another. 
  ||    Theoretically, Kart could contain items for different projects
  ||    so, list method for Kart requires ProjectId
  ||
 ((*/
/*))
 //     Standard method of loading/uploading
((*/
XFS_EXTERN rc_t CC XFSGapKartMake (
                            const struct XFSGapKart ** Kart,
                            const char * Path,
                            const char * Name
                            );
XFS_EXTERN rc_t CC XFSGapKartDispose (
                            const struct XFSGapKart * self
                            );

XFS_EXTERN rc_t CC XFSGapKartAddRef (
                            const struct XFSGapKart * self
                            );
XFS_EXTERN rc_t CC XFSGapKartRelease (
                            const struct XFSGapKart * self
                            );

/*))
 //     Accessing content
((*/
    /*  If ProjectId == 0 it will return all items 
     */
XFS_EXTERN rc_t CC XFSGapKartList (
                            const struct XFSGapKart * self,
                            struct KNamelist ** List,
                            uint32_t ProjectId
                            );
XFS_EXTERN const struct XFSGapKartItem * CC XFSGapKartGet (
                            const struct XFSGapKart * self,
                            const char * ItemName
                            );

XFS_EXTERN const char * CC XFSGapKartPath (
                            const struct XFSGapKart * self
                            );
XFS_EXTERN bool CC XFSGapKartHasForProject (
                            const struct XFSGapKart * self,
                            uint32_t ProjectId
                            );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*))
 ||    KartDepot : collection of Kart.
 ||
 ||    KartDepot is represented by real directory, where Kart files
 ||    are stored.
 ||    Content of KartDepot could be changed, there is Refresh method
 ||    You may list stored Karts as by ProjectId and without
 ||    There is only one KartDepot directory, so ...  
 ||
((*/

XFS_EXTERN rc_t CC XFSGapKartDepotInit ();

XFS_EXTERN rc_t CC XFSGapKartDepotDispose ();

    /* If ProjectId == 0, it will return list of all kart files
     */
XFS_EXTERN rc_t CC XFSGapKartDepotList (
                                    struct KNamelist ** List,
                                    uint32_t ProjectId
                                    );

XFS_EXTERN rc_t CC XFSGapKartDepotGet (
                                    const struct XFSGapKart ** Kart,
                                    const char * KartName
                                    );

XFS_EXTERN rc_t CC XFSGapKartDepotRefresh ();

XFS_EXTERN rc_t CC XFSGapKartDepotVersion ( uint64_t * Version );

XFS_EXTERN const char * CC XFSGapKartDepotPath ();

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _xgapk_h_ */
