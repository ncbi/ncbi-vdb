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
#include <kproc/lock.h>

#include "schwarzschraube.h"
#include "lockdpt.h"
#include "zehr.h"

#include <sysalloc.h>


/*||*\
  || Lyrics:
  || The will be array of KLock instances and lock will be acquired
  || by hash value
\*||*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)) XFSLockDepot structure is living here
 ((*/

struct XFSLockDepot {
    struct KLock ** mutabors;
    uint32_t qty;
};

LIB_EXPORT
rc_t CC
XFSLockDepotMake ( uint32_t DepotSize, struct XFSLockDepot ** Depot )
{
    rc_t RCt;
    struct XFSLockDepot * RetDepot;
    struct KLock * Lock;
    uint32_t llp;

    RCt = 0;
    RetDepot = NULL;
    Lock = NULL;
    llp = 0;

    if ( Depot != NULL ) {
        * Depot = NULL;
    }

    if ( Depot == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( DepotSize == 0 ) {
        DepotSize = XFS_DEFAULT_DEPOT_SIZE;
    }

    RetDepot = calloc ( 1, sizeof ( struct XFSLockDepot ) );
    if ( RetDepot == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RetDepot -> qty = DepotSize;

    RetDepot -> mutabors = calloc (
                            RetDepot -> qty,
                            sizeof ( struct KLock * )
                            );
    if ( RetDepot -> mutabors == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }

    if ( RCt == 0 ) {
        for ( llp = 0; llp < RetDepot -> qty; llp ++ ) {
            RCt = KLockMake ( & Lock );
            if ( RCt != 0 ) {
                break;
            }

            RetDepot -> mutabors [ llp ] = Lock;
        }

        if ( RCt == 0 ) {
            * Depot = RetDepot;
        }
    }

    if ( RCt != 0 ) {
        XFSLockDepotDispose ( RetDepot );

        RetDepot = NULL;
        * Depot = NULL;
    }

    return RCt;
}   /*  XFSLockDepotMake () */

    /*\ I am going to drop all bad LOCKS, so return 0 on any
    |*| ocassion and will think later
    \*/
LIB_EXPORT
rc_t CC
XFSLockDepotDispose ( struct XFSLockDepot * self )
{
    uint32_t llp;
    struct KLock * Lock;

    llp = 0;
    Lock = NULL;

    if ( self == NULL ) {
        return 0;
    }

        /*\ variable qty is initialize at first place, so it is
        |*| safe to check it first
        \*/
    if ( self -> qty != 0 ) {
        if ( self -> mutabors != NULL ) {
            for ( llp = 0; llp < self -> qty; llp ++ ) {
                Lock = self -> mutabors [ llp ];
                if ( Lock != NULL ) {
                    KLockRelease ( Lock );
                }

                self -> mutabors [ llp ] = NULL;
            }

            free ( self -> mutabors );

            self -> mutabors = NULL;
        }

        self -> qty = 0;
    }

    free ( self );

    return 0;
}   /* XFSLockDepotDispose () */

LIB_EXPORT
rc_t CC
XFSLockDepotAcquire ( struct XFSLockDepot * self, uint32_t HashValue )
{
    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( self -> qty == 0 || self -> mutabors == NULL ) {
        return XFS_RC ( rcInvalid );
    } 

    return KLockAcquire ( self -> mutabors [ HashValue % self -> qty ] );
}   /* XFSLockDepotAcquire () */

LIB_EXPORT
rc_t CC
XFSLockDepotUnlock ( struct XFSLockDepot * self, uint32_t HashValue )
{
    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( self -> qty == 0 || self -> mutabors == NULL ) {
        return XFS_RC ( rcInvalid );
    } 

    return KLockUnlock ( self -> mutabors [ HashValue % self -> qty ] );
}   /* XFSLockDepotUnlock () */
