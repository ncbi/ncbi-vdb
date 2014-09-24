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

#ifndef _h_xfs_priv_
#define _h_xfs_priv_

#include <xfs/xfs-defs.h>


/*  Forwards forewer
 */
struct XFSPeer;
struct KThread;

typedef union XFSControl_vt XFSControl_vt;
typedef struct XFSControlArgs XFSControlArgs;

/*
 *  XFSControl structure
 *
 *  Note, that structure is using two members : Peer and Control.
 *  Peer is an object which is rendering tree, and it is accepting
 *  callbacks from Control, which is definitely implementation of
 *  Fuse/Dokan interface. THAT IS VERY IMPORTANT that Peer should
 *  be initialized before Control.
 *
 */
struct XFSControl {
    const XFSControl_vt *vt;

        /*  "struct XFSPeer", which should be responsible for rendering
            tree structure into Fuse/Dokan terms of filesystem
            NOTE: it should be initialized before Control
        */
    struct XFSPeer * Peer;

        /*  "struct fuse" for Fuse, and something else for all
            other platforms
            "struct fuse" contains pointer for "fuse_session",
            and it contains pointer for "fuse_chan", so it will
            allow to define practically everything in environment.
            BTW, I introduced ContolOpt to store fuze_chan here.
            NOTE: it should be initialized after Peer
         */
    void * Control;
    void * ControlOpt;

        /*  That is for storeing of arguments to run fuse
            I am not sure if we need it, but arguments for initializing
            fuse and docan are passing in difeerent way.
            BTW, prolly I need to use BSTree here, not sure.
         */
    XFSControlArgs * Arguments;

        /*  That is a thread which will start Fuse/Dokan loop
            and it will be called from XFSStart() method
         */
    KThread * Thread;
};

/*
 *  Vertuhai table.
 */
typedef struct XFSControl_vt_v1 XFSControl_vt_v1;
struct XFSControl_vt_v1 {
        /*  version == 1.x
         */
    uint32_t maj;
    uint32_t min;

        /*  overloaded methods
         */
        /*  version 0.0
         */
    rc_t ( CC * init ) ( XFSControl * self );
    rc_t ( CC * destroy ) ( XFSControl * self );
    rc_t ( CC * mount ) ( XFSControl * self );
    rc_t ( CC * loop ) ( XFSControl * self );
    rc_t ( CC * unmount ) ( XFSControl * self );
};

union XFSControl_vt {
    XFSControl_vt_v1 v1;
};

/*  Note, more than temporal structure.
 */
struct XFSControlArgs {
    char Label [ XFS_SIZE_4096 ];
    char MountPoint [ XFS_SIZE_4096 ];
};

#endif /* _h_xfs_priv_ */
