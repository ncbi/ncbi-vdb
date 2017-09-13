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

#ifndef _xfs_node_dpf_h_
#define _xfs_node_dpf_h_

#include <xfs/xfs-defs.h>
#include <xfs/node.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*))
 ||   Lyrics.
 ++   That file contains XFSNodeDPF node with Date and Permissions
 ||   and Flavor. And standard functions to set these values
((*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))   Node itself? Abyrvalk
 ((*/
struct XFSNodeDPF {
    struct XFSNode node;

    const char * permissions;   /* Permissions in format
                                 * "rwxrwxrwx u:g:o"
                                 */
    KTime_t date;               /* Date in fomrat "morning/lunch/evening
                                 */
    uint32_t flavor;
};

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))
 ||
 ++   Standard initialization of node and members of VT
 ||   
((*/
    /*  Note: that method does not allocate any node ... only ints
     */
XFS_EXTERN rc_t CC XFSNodeDPF_Init (
                            const struct XFSNode * self,
                            const char * Perm,  /* Default NULL */
                            KTime_t Date,       /* Default 0 */
                            uint32_t Flavor     /* _sFlavorLess (0) */
                            );

    /*  Note: that method does not frees any node ... only disposes some
     */
XFS_EXTERN rc_t CC XFSNodeDPF_Dispose (
                            const struct XFSNode * self
                            );

/*))
 ||
 ++   Getters/setters which could be used from members of VT and on init
 ||   
((*/
XFS_EXTERN uint32_t CC XFSNodeDPF_Flavor (
                                    const struct XFSNode * self
                                    );

XFS_EXTERN rc_t CC XFSNodeDPF_Permissions (
                                    const struct XFSNode * self,
                                    const char ** Permissions
                                    );

XFS_EXTERN rc_t CC XFSNodeDPF_SetPermissions (
                                    const struct XFSNode * self,
                                    const char * Permissions
                                    );

XFS_EXTERN rc_t CC XFSNodeDPF_Date (
                                    const struct XFSNode * self,
                                    KTime_t * Date
                                    );

XFS_EXTERN rc_t CC XFSNodeDPF_SetDate (
                                    const struct XFSNode * self,
                                    KTime_t Date
                                    );

/*))
 ||
 ++   Getters/setters which could be used as VT members
 ||   
((*/
XFS_EXTERN uint32_t CC XFSNodeDPF_vt_flavor (
                                    const struct XFSNode * self
                                    );

XFS_EXTERN rc_t CC XFSNodeDPF_vt_permissions (
                                    const struct XFSAttrEditor * self,
                                    const char ** Permissions
                                    );

XFS_EXTERN rc_t CC XFSNodeDPF_vt_set_permissions (
                                    const struct XFSAttrEditor * self,
                                    const char * Permissions
                                    );

XFS_EXTERN rc_t CC XFSNodeDPF_vt_date (
                                    const struct XFSAttrEditor * self,
                                    KTime_t * Date
                                    );

XFS_EXTERN rc_t CC XFSNodeDPF_vt_set_date (
                                    const struct XFSAttrEditor * self,
                                    KTime_t Date
                                    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _xfs_node_dpf_h_ */
