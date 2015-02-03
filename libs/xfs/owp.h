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

#ifndef _owp_h_
#define _owp_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))   Object With Properties (OWP), draft version
 ((
  ))   That is an Object which contais set of pairs of chars : key and
 ((    property.
  ))
 ((    Temporarily, I do not have methods like delete/replace/list, but
  ))   could add them later
 ((*/
struct XFSOwp;
struct KNamelist;

XFS_EXTERN rc_t CC XFSOwpMake ( struct XFSOwp ** Owp );
XFS_EXTERN rc_t CC XFSOwpInit ( struct XFSOwp * self );
XFS_EXTERN rc_t CC XFSOwpWhack ( struct XFSOwp * self );
XFS_EXTERN rc_t CC XFSOwpDispose ( struct XFSOwp * self );
XFS_EXTERN rc_t CC XFSOwpClear ( struct XFSOwp * self );

XFS_EXTERN bool CC XFSOwpHas (
                                const struct XFSOwp * self,
                                const char * Key
                                );
XFS_EXTERN const char * CC XFSOwpGet (
                                const struct XFSOwp * self,
                                const char * Key
                                );
    /*)) Property value could be NULL
     ((*/
XFS_EXTERN rc_t CC XFSOwpSet (
                                const struct XFSOwp * self,
                                const char * Key,
                                const char * Propety
                                );

XFS_EXTERN rc_t CC XFSOwpListKeys (
                                const struct XFSOwp * self,
                                const struct KNamelist ** Keys
                                );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _owp_h_ */
