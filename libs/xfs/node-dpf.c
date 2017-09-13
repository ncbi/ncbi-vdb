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
#include <klib/rc.h>
#include <klib/out.h>
#include <klib/text.h>
#include <klib/namelist.h>
#include <klib/refcount.h>
#include <klib/printf.h>
#include <klib/log.h>
#include <klib/time.h>

#include <xfs/node.h>
#include <xfs/editors.h>
#include <xfs/perm.h>

#include "mehr.h"
#include "schwarzschraube.h"
#include "common.h"
#include "node-dpf.h"

#include <sysalloc.h>

/*)))
  |||
  +++    Node with Extras ... wanna buy bag? SPIRIT is always with YOU
  |||
  (((*/

/*)))
 |||
 +++    Kohl'back's
 |||
(((*/

/*)))  So, Initialization ... of node and other stuff
 (((*/
LIB_EXPORT
rc_t CC
XFSNodeDPF_Init (
                    const struct XFSNode * self,
                    const char * Perm,  /* Default NULL */
                    KTime_t Date,       /* Default 0 */
                    uint32_t Flavor     /* _sFlavorLess (0) */
)
{
    rc_t RCt;
    struct XFSNodeDPF * xNode;

    RCt = 0;
    xNode = ( struct XFSNodeDPF * ) self;

    XFS_CAN ( xNode )

    XFSNodeDPF_Dispose ( self );

    if ( Perm != NULL ) {
        RCt = XFS_StrDup ( Perm, & ( xNode -> permissions ) );
    }

    if ( 0 < Date ) {
        xNode -> date = Date;
    }

    if ( _sFlavorLess < Flavor ) {
        xNode -> flavor = Flavor;
    }

    return RCt;
}   /* XFSNodeDPF_Init () */

LIB_EXPORT
rc_t CC
XFSNodeDPF_Dispose ( const struct XFSNode * self )
{
    struct XFSNodeDPF * xNode = ( struct XFSNodeDPF * ) self;

    if ( xNode != NULL ) {
        if ( xNode -> permissions != NULL ) {
            free ( ( char * ) xNode -> permissions );

            xNode -> permissions = NULL;
        }

        xNode -> date = KTimeStamp ();
        xNode -> flavor = _sFlavorLess;

    }

    return 0;
}   /* XFSNodeDPF_Dispose () */

/*)))  So, Set/Get ... odin otvet
 (((*/
LIB_EXPORT
uint32_t CC
XFSNodeDPF_Flavor ( const struct XFSNode * self )
{
    return ( ( const struct XFSNodeDPF * ) self ) -> flavor;
}   /* XFSNodeDPF_Flavor () */

LIB_EXPORT
uint32_t CC
XFSNodeDPF_vt_flavor ( const struct XFSNode * self )
{
    return ( ( const struct XFSNodeDPF * ) self ) -> flavor;
}   /* XFSNodeDPF_vt_flavor () */

/*)))  So, editors stuff
 (((*/
LIB_EXPORT
rc_t CC
XFSNodeDPF_Permissions (
                        const struct XFSNode * self,
                        const char ** Permissions
)
{
    struct XFSNodeDPF * xNode = ( struct XFSNodeDPF * ) self;

    XFS_CSAN ( Permissions )
    XFS_CAN (xNode )
    XFS_CAN ( Permissions )

    * Permissions = xNode -> permissions;

    return 0;
}   /* XFSNodeDPF_Permissions () */

LIB_EXPORT
rc_t CC
XFSNodeDPF_vt_permissions (
                        const struct XFSAttrEditor * self,
                        const char ** Permissions
                        )
{
    XFS_CAN ( self )

    return XFSNodeDPF_Permissions (
                                XFSEditorNode ( & ( self -> Papahen ) ),
                                Permissions
                                );
}   /* XFSNodeDPF_vt_permissions () */

XFS_EXTERN
rc_t CC
XFSNodeDPF_SetPermissions (
                        const struct XFSNode * self,
                        const char * Permissions
)
{
    struct XFSNodeDPF * xNode = ( struct XFSNodeDPF * ) self;

    XFS_CAN (xNode )

    if ( xNode -> permissions != NULL ) {
        free ( ( char * ) xNode -> permissions );
        xNode -> permissions = NULL;
    }

    return Permissions == NULL
            ? 0
            : XFS_StrDup ( Permissions, & ( xNode -> permissions ) );
            ;
}   /* XFSNodeDPF_SetPermissions () */

LIB_EXPORT
rc_t CC
XFSNodeDPF_vt_set_permissions (
                        const struct XFSAttrEditor * self,
                        const char * Permissions
                        )
{
    XFS_CAN ( self )

    return XFSNodeDPF_SetPermissions (
                                XFSEditorNode ( & ( self -> Papahen ) ),
                                Permissions
                                );
}   /* XFSNodeDPF_vt_set_permissions () */

LIB_EXPORT
rc_t CC
XFSNodeDPF_Date ( const struct XFSNode * self, KTime_t * Date )
{
    struct XFSNodeDPF * xNode = ( struct XFSNodeDPF * ) self;

    XFS_CSA ( Date, 0 )
    XFS_CAN (xNode )
    XFS_CAN ( Date )

    * Date = xNode -> date;

    return 0;
}   /* XFSNodeDPF_Date () */

LIB_EXPORT
rc_t CC
XFSNodeDPF_vt_date ( const struct XFSAttrEditor * self, KTime_t * Time )
{
    XFS_CAN ( self )

    return XFSNodeDPF_Date (
                                XFSEditorNode ( & ( self -> Papahen ) ),
                                Time
                                );
}   /* XFSNodeDPF_vt_date () */

XFS_EXTERN
rc_t CC
XFSNodeDPF_SetDate ( const struct XFSNode * self, KTime_t Date )
{
    struct XFSNodeDPF * xNode = ( struct XFSNodeDPF * ) self;

    XFS_CAN (xNode )

    xNode -> date = Date == 0 ? KTimeStamp () : Date;

    return 0;
}   /* XFSNodeDPF_SetDate () */

LIB_EXPORT
rc_t CC
XFSNodeDPF_vt_set_date (
                        const struct XFSAttrEditor * self,
                        KTime_t Time
)
{
    XFS_CAN ( self )

    return XFSNodeDPF_SetDate (
                                XFSEditorNode ( & ( self -> Papahen ) ),
                                Time
                                );
}   /* XFSNodeDPF_vt_set_date () */
