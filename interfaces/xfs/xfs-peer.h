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

#ifndef _xfs_peer_h_
#define _xfs_peer_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*  Forwards and others
 */
struct VPath;
struct KFile;
struct KNamelist;

typedef struct XFSPeer XFSPeer;
typedef union XFSPeer_vt XFSPeer_vt;
typedef struct XFSPeer_vt_v1 XFSPeer_vt_v1;
typedef struct XFSPeerObjectInfo XFSPeerObjectInfo;

/****************************************************************
 *  Those are four most important peer methods which user should 
 *  use. All other methods and structures are for those who is
 *  developing own peer.
 *
 *  So, there are three peers now : Dummy, Prime and Dynamic
 *  and there are three methods which will create one of those
 *  peers. Also, there is one method which will correctly destroy
 *  peer.
 ****************************************************************/

/*  That method will create Prime peer, which is only legal peer in
    TRACE/SRA group, and it is aleays under heavy development
    Not sure yet about parameters for that peer initialisation
 */
XFS_EXTERN rc_t CC XFSPrimePeerCreate(
                            XFSPeer ** Peer
                            );

/*  That method will create Dynamic peer, the peer which will be 
    dynamically loaded from library with name:
                    "lib-dynamic-peer##Name##.so"
    It accepts as parameters Name of library and Data - pointer
    to user defined data.

    NOTE: user is responcible for destroying Data provided
*/
XFS_EXTERN rc_t CC XFSDynamicPeerCreate(
                            XFSPeer ** Peer,
                            const char * Name,
                            void * Data
                            );

/*  That method will create Dummy peer, which is only for debugging
    and testing only. It requests only one parameter: DummyDirectory,
    that is path to 'REAL' directory, which will be mounted to
    mounting point.
 */
XFS_EXTERN rc_t CC XFSDummyPeerCreate(
                            XFSPeer ** Peer,
                            const char * DummyDirectory
                            );

/*  That method will corectly destroy peer object.
 */
XFS_EXTERN rc_t CC XFSPeerDestroy(
                    XFSPeer * Peer
                    );

/****************************************************************
 * Lyrics:
 *
 * That file contains minimal ( I suppose to ) set of methods
 * needed to be implemented to crossbreed Fuse and Docan
 * There is list :
 * 
 *    unlink()
 *    rmdir()
 *    rename()
 *    truncate()
 *    getattr()
 *    mkdir()
 *    utime()
 *    release()
 *    open()
 *    opendir()
 *    releasedir()
 *    read()
 *    write()
 *    readdir()
 *    access()
 *    create()
 *    ftruncate()
 *    fgetattr()
 *    destroy()
 *    init()
 *
 ****************************************************************/


struct XFSPeer {
    const XFSPeer_vt * vt;

        /*  Named it Name, we will initialize peer by it's name,
            cuz I hope it will be implemented through dlopen/dlsum
         */
    char Name [ XFS_SIZE_4096 ];

        /*  Named it ID, but not sure, prolly will rename it later
            Something unique for peer with Name
         */
    char Id [ XFS_SIZE_4096 ];

        /*  Something that Peer could use during work
            NOTE: User is responsible for destroying data associated
                  with that field!
         */
    void * Data;
};

struct XFSPeer_vt_v1 {
        /*  version == 1.x
         */
    uint32_t maj;
    uint32_t min;

        /*  overloaded methods
         */
        /*  Not sure if we will use it, just a place holder
         */
    rc_t ( CC * init ) ( XFSPeer * self );
    rc_t ( CC * destroy ) ( XFSPeer * self );

    rc_t ( CC * unlink ) (
                    XFSPeer * self,
                    const struct VPath * Path
                    );
    rc_t ( CC * rmdir ) (
                    XFSPeer * self,
                    const struct VPath * Path
                    );
    rc_t ( CC * rename ) (
                    XFSPeer * self,
                    const struct VPath * OldPath,
                    const struct VPath * NewPath
                    );
    rc_t ( CC * truncate ) (
                    XFSPeer * self,
                    const struct VPath * ThePath,
                    uint64_t TheSize
                    );
    rc_t ( CC * getattr ) (
                    XFSPeer * self,
                    const struct VPath * ThePath,
                    XFSPeerObjectInfo * TheInfo
                    );
    rc_t ( CC * mkdir ) (
                    XFSPeer * self,
                    const struct VPath * ThePath,
                    uint32_t Access
                    );
    rc_t ( CC * utime ) (
                    XFSPeer * self,
                    const struct VPath * ThePath,
                    KTime_t TimeToSet
                    );
    rc_t ( CC * release ) (
                    XFSPeer * self,
                    const struct VPath * ThePath,
                    struct KFile * File
                    );
    rc_t ( CC * open ) (
                    XFSPeer * self,
                    const struct VPath * ThePath,
                    struct KFile ** File,
                    uint32_t Flags
                    );
    rc_t ( CC * opendir ) (
                    XFSPeer * self,
                    const struct VPath * ThePath,
                    void ** KoreWa
                    );
    rc_t ( CC * releasedir ) (
                    XFSPeer * self,
                    const struct VPath * ThePath,
                    void * SoreWa
                    );
    rc_t ( CC * read ) (
                    XFSPeer * self,
                    const struct VPath * ThePath,
                    struct KFile * TheFile,
                    uint64_t ReadOffset,
                    void * Buffer,
                    size_t BufferSize,
                    size_t *NumBytesReaded
                    );
    rc_t ( CC * write ) (
                    XFSPeer * self,
                    const struct VPath * ThePath,
                    struct KFile * TheFile,
                    uint64_t WriteOffset,
                    void * Buffer,
                    size_t BufferSize,
                    size_t *NumBytesWritten
                    );
    rc_t ( CC * readdir ) (
                    XFSPeer * self,
                    const struct VPath * ThePaht,
                    void * AreWa,
                    struct KNamelist ** TheList
                    );
    rc_t ( CC * access ) (
                    XFSPeer * self,
                    const struct VPath * ThePath,
                    uint32_t NonDefaultPermissiona
                    );
    rc_t ( CC * create ) (
                    XFSPeer * self,
                    const struct VPath * ThePath,
                    struct KFile ** TheFile,
                    uint32_t Flags,
                    uint32_t Access
                    );
    rc_t ( CC * ftruncate ) (
                    XFSPeer * self,
                    const struct VPath * ThePath,
                    struct KFile * TheFile,
                    uint64_t TheSize
                    );
    rc_t ( CC * fgetattr ) (
                    XFSPeer * self,
                    const struct VPath * ThePath,
                    const struct KFile * TheFile,
                    XFSPeerObjectInfo * TheInfo
                    );
};

union XFSPeer_vt {
    XFSPeer_vt_v1 v1;
};

/*  I am introducing that because I need to distinct tree entries by
    type. Not sure if I need it later.
 */
enum XFSPeerObjectType {
      kxfsNotFound
    , kxfsBad
    , kxfsFile
    , kxfsDir
    , kxfsLink /* We will do with Enums later */
};

/*  That structure which will be used by getattr/fgetattr/access
 */
struct XFSPeerObjectInfo {
    enum XFSPeerObjectType Type;

    uint64_t Size;
    int64_t Time;
    uint32_t Access;
};

/*  Since Peer is initialized by name, we collect most useful ones only
    We do suppose that there are two static peers: Dummy and Prime.
    Dummy peer is needed for debug goals only and for pranks.
    Prime peer is that peer whic we suppose to do.
    Peers with names different from these two will be treated as
    dynamical and we will try to load them from dynamic library
    with name "lib-dynamic-peer##Name##.so"

    Something looze: XFSPeerCreate could accept NULL as Id value

    XFSPeerCreate will allocate XFSPeer structure.
    Please do not call that procedure.
 */
XFS_EXTERN rc_t CC XFSPeerCreate(
                    XFSPeer ** Peer,
                    const char * PeerName,
                    const char * PeerId,
                    const XFSPeer_vt * VirtualTable
                    );

/*  That is a part of conspiracy, if You do not like too verbose
 *  programms, comment that part
 */
#define XFS_EXTENDED_LOGGING

#ifdef XFS_EXTENDED_LOGGING
    #define XFSMSG(msg) OUTMSG(msg)
#else   /* XFS_EXTENDED_LOGGING */
    #define XFSMSG(msg) 
#endif  /* XFS_EXTENDED_LOGGING */

/*  Another great masterpiece lol
 */
#define XFS_RC(State)   RC(rcFS, rcNoTarg, rcProcessing, rcNoObj, State)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _xfs_peer_h_ */
