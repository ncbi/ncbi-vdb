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

#ifndef _xkart_h_
#define _xkart_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */

/*))))
 ((((   There is some interface build over Kart structure.
  ))))  Called it XFSKart, sorry, weak imagination
 ((((*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct XFSKart;
struct XFSKartItem;
struct KNamelist;

/*))
 //     Standard method of loading/uploading
((*/
XFS_EXTERN rc_t CC XFSKartMake (
                            const char * Path,
                            struct XFSKart ** Kart
                            );
XFS_EXTERN rc_t CC XFSKartDispose (
                            struct XFSKart * self
                            );

XFS_EXTERN rc_t CC XFSKartAddRef (
                            struct XFSKart * self
                            );
XFS_EXTERN rc_t CC XFSKartRelease (
                            struct XFSKart * self
                            );

XFS_EXTERN rc_t CC XFSKartReload (
                            struct XFSKart * self
                            );

/*))
 //     Accessing KartItems
((*/
XFS_EXTERN rc_t CC XFSKartList (
                            struct XFSKart * self,
                            struct KNamelist ** List
                            );
XFS_EXTERN bool CC XFSKartHas (
                            struct XFSKart * self,
                            const char * ItemName
                            );
XFS_EXTERN const struct XFSKartItem * CC XFSKartGet (
                            struct XFSKart * self,
                            const char * ItemName
                            );
XFS_EXTERN const char * CC XFSKartPath (
                            const struct XFSKart * self
                            );

/*))
 //     KartItem content
((*/
XFS_EXTERN rc_t CC XFSKartItemAddRef (
                            const struct XFSKartItem * self
                            );
XFS_EXTERN rc_t CC XFSKartItemRelease (
                            const struct XFSKartItem * self
                            );

XFS_EXTERN const char * CC XFSKartItemProject (
                            const struct XFSKartItem * self
                            );
XFS_EXTERN const char * CC XFSKartItemId (
                            const struct XFSKartItem * self
                            );
XFS_EXTERN const char * CC XFSKartItemAccession (
                            const struct XFSKartItem * self
                            );
XFS_EXTERN const char * CC XFSKartItemName (
                            const struct XFSKartItem * self
                            );
XFS_EXTERN const char * CC XFSKartItemDescription (
                            const struct XFSKartItem * self
                            );
XFS_EXTERN const char * CC XFSKartItemDisplayName (
                            const struct XFSKartItem * self
                            );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _xkart_h_ */
