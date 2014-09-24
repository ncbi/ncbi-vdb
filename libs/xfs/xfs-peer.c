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
#include <xfs/xfs-defs.h>
#include <xfs/xfs-peer.h>

#include <sysalloc.h>
#include <stdlib.h>
#include <string.h>

/***********************************
 * The values for these variables were initialized in corresponding
 * files
 */
extern const char * xfsDummyPeerName;
extern const char * xfsPrimePeerName;

LIB_EXPORT
rc_t CC
XFSPeerCreate (
            XFSPeer ** Peer,
            const char * PeerName,
            const char * PeerId,
            const XFSPeer_vt * Virtuhaj
)
{
    rc_t RCt;
    XFSPeer * ThePeer;

    RCt = 0;

    if ( Peer == NULL ) {
        return RC ( rcFS, rcNoTarg, rcConstructing, rcSelf, rcNull );
    }

    ThePeer = * Peer;

    if ( ThePeer != NULL ) {
        return RC ( rcFS, rcNoTarg, rcConstructing, rcSelf, rcUnexpected );
    }

    if ( PeerName == NULL ) {
        return RC ( rcFS, rcNoTarg, rcConstructing, rcParam, rcNull );
    }

    ThePeer = calloc ( 1, sizeof ( XFSPeer ) );
    if ( ThePeer == NULL ) {
        return RC ( rcFS, rcNoTarg, rcAllocating, rcSelf, rcNull );
    }

    string_copy_measure (
                        ThePeer -> Name,
                        sizeof ( ThePeer -> Name ),
                        PeerName
                        );

    if ( PeerId != NULL ) {
        string_copy_measure (
                            ThePeer -> Id,
                            sizeof ( ThePeer -> Id ),
                            PeerId
                            );
    }

    ThePeer -> vt = Virtuhaj;

    RCt = ThePeer -> vt -> v1.init(ThePeer);
    if ( RCt == 0 ) {
        * Peer = ThePeer;
    }
    else {
        XFSPeerDestroy ( ThePeer );

        free ( ThePeer );
    }

    return RCt;
}   /* XFSPeerInit() */

LIB_EXPORT
rc_t CC
XFSPeerDestroy ( XFSPeer * Peer )
{
    rc_t RCt;

    RCt = 0;

    if ( Peer == NULL ) {
        RCt = RC ( rcFS, rcNoTarg, rcDestroying, rcSelf, rcNull );
    }
    else {
        if ( Peer -> vt == NULL ) {
            RCt = RC ( rcFS, rcNoTarg, rcDestroying, rcSelf, rcNull );
        }
        else {
                /*  ??? TODO think about individual peer destruction
                    by name
                 */
            RCt = Peer -> vt -> v1.destroy ( Peer );

                /*  Despite of the operation result we need to
                    delete old object
                 */
            memset ( Peer, 0, sizeof ( XFSPeer ) );

            free ( Peer );
        }
    }

    return RCt;
}   /* XFSPeerDestroy() */
