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
#include <klib/time.h>
#include <kfs/file.h>
#include <kfs/directory.h>
#include <vfs/manager.h>
#include <vfs/path.h>
#include <xfs/xfs-defs.h>
#include <xfs/xfs-peer.h>

#include <string.h>             /*  we are using memset
                                 */
#include <fcntl.h>              /*  using O_RDONLY, O_WRONLY, O_RDWR
                                 *  and that is not cool. But VFS is
                                 *  using these in Win too. Prolly we
                                 *  will introduce some special type
                                 *  for that in future.
                                 */

/*  Something unusual
 */
extern const char *xfsDummyPeerName;
const char * xfsDummyPeerName = "Dummy";

/*  Forwards
 */
static rc_t CC XFS_DUMMY_PEER_init_v1 ( XFSPeer * self );
static rc_t CC XFS_DUMMY_PEER_destroy_v1 ( XFSPeer * self );
static rc_t CC XFS_DUMMY_PEER_unlink_v1 (
                                XFSPeer * self,
                                const struct VPath * Path
                                );
static rc_t CC XFS_DUMMY_PEER_rmdir_v1 (
                                XFSPeer * self,
                                const struct VPath * Path
                                );
static rc_t CC XFS_DUMMY_PEER_rename_v1 (
                                XFSPeer * self,
                                const struct VPath * OldPath,
                                const struct VPath * NewPath
                                );
static rc_t CC XFS_DUMMY_PEER_truncate_v1 (
                                XFSPeer * self,
                                const struct VPath * ThePath,
                                uint64_t TheSize 
                                );
static rc_t CC XFS_DUMMY_PEER_getattr_v1 (
                                XFSPeer * self,
                                const struct VPath * ThePath,
                                XFSPeerObjectInfo * TheInfo
                                );
static rc_t CC XFS_DUMMY_PEER_mkdir_v1 (
                                XFSPeer * self,
                                const struct VPath * ThePath,
                                uint32_t Access
                                );
static rc_t CC XFS_DUMMY_PEER_utime_v1 (
                                XFSPeer * self,
                                const struct VPath * ThePath,
                                KTime_t TimeToSet
                                );
static rc_t CC XFS_DUMMY_PEER_release_v1 (
                                XFSPeer * self,
                                const struct VPath * ThePath,
                                struct KFile * File
                                );
static rc_t CC XFS_DUMMY_PEER_open_v1 (
                                XFSPeer * self,
                                const struct VPath * ThePath,
                                struct KFile ** File,
                                uint32_t Flags
                                );
static rc_t CC XFS_DUMMY_PEER_opendir_v1 (
                                XFSPeer * self,
                                const struct VPath * ThePath,
                                void ** KoreWa
                                );
static rc_t CC XFS_DUMMY_PEER_releasedir_v1 (
                                XFSPeer * self,
                                const struct VPath * ThePath,
                                void * SoreWa
                                );
static rc_t CC XFS_DUMMY_PEER_read_v1 (
                                XFSPeer * self,
                                const struct VPath * ThePath,
                                struct KFile * TheFile,
                                uint64_t ReadOffset,
                                void * Buffer,
                                size_t BufferSize,
                                size_t *NumBytesReaded
                                );
static rc_t CC XFS_DUMMY_PEER_write_v1 (
                                XFSPeer * self,
                                const struct VPath * ThePath,
                                struct KFile * TheFile,
                                uint64_t WriteOffset,
                                void * Buffer,
                                size_t BufferSize,
                                size_t *NumBytesWritten
                                );
static rc_t CC XFS_DUMMY_PEER_readdir_v1 (
                                XFSPeer * self,
                                const struct VPath * ThePath,
                                void * AreWa,
                                struct KNamelist ** TheList
                                );
static rc_t CC XFS_DUMMY_PEER_access_v1 (
                                XFSPeer * self,
                                const struct VPath * ThePath,
                                uint32_t NonDefaultPermissions
                                );
static rc_t CC XFS_DUMMY_PEER_create_v1 (
                                XFSPeer * self,
                                const struct VPath * ThePath,
                                struct KFile ** TheFile,
                                uint32_t Flags,
                                uint32_t Access
                                );
static rc_t CC XFS_DUMMY_PEER_ftruncate_v1 (
                                XFSPeer * self,
                                const struct VPath * ThePath,
                                struct KFile * TheFile,
                                uint64_t TheSize 
                                );
static rc_t CC XFS_DUMMY_PEER_fgetattr_v1 (
                                XFSPeer * self,
                                const struct VPath * ThePath,
                                const struct KFile * TheFile,
                                XFSPeerObjectInfo * TheInfo
                                );

/*  DummyPeer virtual table
 */
static struct XFSPeer_vt_v1 XFSDummyPeer_VT_V1 = {
            1,    /* maj */
            1,    /* min */
            XFS_DUMMY_PEER_init_v1,         /* +? */
            XFS_DUMMY_PEER_destroy_v1,      /* +? */
            XFS_DUMMY_PEER_unlink_v1,       /* + */
            XFS_DUMMY_PEER_rmdir_v1,        /* + */
            XFS_DUMMY_PEER_rename_v1,       /* + */
            XFS_DUMMY_PEER_truncate_v1,     /* + */
            XFS_DUMMY_PEER_getattr_v1,      /* + */
            XFS_DUMMY_PEER_mkdir_v1,        /* + */
            XFS_DUMMY_PEER_utime_v1,        /* + */
            XFS_DUMMY_PEER_release_v1,      /* + */
            XFS_DUMMY_PEER_open_v1,         /* + */
            XFS_DUMMY_PEER_opendir_v1,
            XFS_DUMMY_PEER_releasedir_v1,
            XFS_DUMMY_PEER_read_v1,         /* + */
            XFS_DUMMY_PEER_write_v1,        /* + */
            XFS_DUMMY_PEER_readdir_v1,
            XFS_DUMMY_PEER_access_v1,       /* + */
            XFS_DUMMY_PEER_create_v1,       /* + */
            XFS_DUMMY_PEER_ftruncate_v1,    /* + */
            XFS_DUMMY_PEER_fgetattr_v1      /* + */
};


LIB_EXPORT
rc_t CC
XFSDummyPeerCreate ( XFSPeer ** self, const char * DummyPoint )
{
    rc_t RCt;

    RCt = 0;

    if ( DummyPoint == NULL ) {
        return RC ( rcFS, rcNoTarg, rcCreating, rcParam, rcNull );
    }

    RCt = XFSPeerCreate (
                    self,
                    xfsDummyPeerName,
                    DummyPoint,
                    (const XFSPeer_vt *) &XFSDummyPeer_VT_V1
                    );

    return RCt;
}   /* XFSDummyPeerCreate() */

/*  Really trashy method, which reads Path value to buffer, and, if
    there was error of reading, or Path is NULL, the Filler value will
    be copied to buffer. Filler should be 0 terminated string, or NULL
    If Filler is NULL, the "NULL" will be copied to buffer then.
 */
static
rc_t
XFSPeer_ReadVPath (
            const struct VPath * Path,
            char * Buffer,
            size_t BufferSize,
            const char * Filler
)
{
    rc_t RCt;
    size_t NR;
    const char * DefaultFiller = "NULL";

    RCt = 0;

    if ( Buffer != NULL && 0 < BufferSize ) {
        if ( Path != NULL ) {
            RCt = VPathReadPath ( Path, Buffer, BufferSize, & NR );
        }
        else {
            RCt = RC ( rcFS, rcNoTarg, rcLogging, rcPath, rcNull );

            string_copy_measure (
                            Buffer,
                            sizeof ( Buffer ),
                            ( Filler == NULL ? DefaultFiller : Filler )
                            );
        }
    }
    else {
        RCt = RC ( rcFS, rcNoTarg, rcLogging, rcBuffer, rcNull );
    }

    return RCt;
}   /* XFSPeer_ReadVPath () */

/*  There is some important method
 *  I am not sure if I could do that freely, but I am.
 *  TODO!!!
 */
static
rc_t
XFSPeer_ResolvePath_v1 (
                    XFSPeer * self,
                    const struct VPath * ThePath,
                    struct VPath ** TheResolvedPath
)
{
    rc_t RCt;
    struct VPath * TempPath;
    char Buffer [ XFS_SIZE_4096 ];
    size_t NumRead, PathSize;

    RCt = 0;

    if ( self == NULL || ThePath == NULL || TheResolvedPath == NULL ) {
        return XFS_RC ( rcNull );
    }

    * TheResolvedPath = NULL;

    PathSize = string_copy_measure (
                                Buffer,
                                sizeof ( Buffer ),
                                self -> Id
                                );
    if ( * ( Buffer + PathSize - 1 ) == '/' ) {
        Buffer [ PathSize -1 ] = 0;
        PathSize --;
    }

    RCt = VPathReadPath (
                        ThePath,
                        Buffer + PathSize,
                        sizeof ( Buffer ) - PathSize,
                        & NumRead
                        );
    if ( RCt == 0 ) {
        RCt = VPathMake ( & TempPath, Buffer );
        if ( RCt == 0 ) {
            * TheResolvedPath = TempPath;
        }
    }

    if ( RCt == 0 ) {
        XFSMSG ( ( "  # RES(Dummy): [%s] to [%s]\n", Buffer + PathSize, Buffer ) );
    }
    else {
        XFSMSG ( ( "  # RES(Dummy): [%s] to [ERROR]\n", Buffer + PathSize ) );
    }

    return RCt;
}   /* XFSPeer_ResolvePath_v1 () */

/*  Another important method ... Common Parameter check for some methods
 *  You should remember that RetManager and ResolvedPath could be NULL,
 *  in that case these parameters will be not returned
 *  TODO!!!
 */
static
rc_t
XFSPeer_CheckCommonParameterReturnGoodOnes (
                    XFSPeer * self,
                    const struct VPath * ThePath,
                    uint32_t Context,
                    VFSManager ** RetManager,
                    struct VPath ** ResolvedPath

)
{
    rc_t RCt;

    RCt = 0;

    if ( self == NULL || ThePath == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( RetManager != NULL ) {
        * RetManager = ( VFSManager * ) self -> Data;

        if ( * RetManager == NULL ) {
            return RC ( rcFS, rcNoTarg, Context, rcMgr, rcNull );
        }
    }

    if ( ResolvedPath != NULL ) {
        RCt = XFSPeer_ResolvePath_v1 ( self, ThePath, ResolvedPath );
    }

    return RCt;
}   /* XFSPeer_CheckCommonParametersReturnGoodOnes () */

/*  Another important method, mostly mock up - task to fill up 
 *  XFSPeerObjectInfo structure. I put it outside of getattr()
 *  because I think that it could be make overloadable in future
 *  BTW, here will be similar method for fgetattr() based on KFile
 *  That method is using KDirectory.
 */
static
rc_t
XFSPeer_FillXFSPeerObjectInfo (
                        const VFSManager * TheManager,
                        const VPath * TheVPath,
                        XFSPeerObjectInfo * TheInfo
)
{
    rc_t RCt;
    KDirectory * TheDirectory;
    char ThePath [ XFS_SIZE_4096 ];
    KPathType TheType;

        /*  First we are resetting that good ObjectInfo structure
         */
    memset ( TheInfo, 0, sizeof ( XFSPeerObjectInfo ) );
    TheInfo -> Type = kxfsBad;

        /*  Secont we should get KDirectory instance from Manager
         */
    RCt = VFSManagerGetCWD ( TheManager, & TheDirectory );
    if ( RCt != 0 ) {
        return RCt;
    }

        /*  Thirt we should convert VPath to char, which KDirectory
         *  understands well
         */
    RCt = XFSPeer_ReadVPath (
                            TheVPath,
                            ThePath,
                            sizeof ( ThePath ),
                            ""
                            );
    if ( RCt != 0 ) {
        return RCt;
    }

        /*  Now we should retrieve file type and fall out if something
         *  is wrong
         */
    TheType = KDirectoryPathType ( TheDirectory, "%s", ThePath );
    switch ( TheType ) {
        case kptFile:
            TheInfo -> Type = kxfsFile;
            break;
        case kptDir:
            TheInfo -> Type = kxfsDir;
            break;
        case kptNotFound:
            TheInfo -> Type = kxfsNotFound;
            return RC ( rcFS, rcNoTarg, rcResolving, rcPath, rcNotFound );
        case kptBadPath:
        default:
            TheInfo -> Type = kxfsBad;
            return RC ( rcFS, rcNoTarg, rcResolving, rcPath, rcUnknown );
    }

        /*  Now we are goint to get size/date and access info
         */
        /*  About size of directory, I do not know why, but Kurt
            prohibited getting size for directory, do not know why
            see kfs/unix/sysdir.c:
            1.1          (rodarmer 13-Nov-09):         if ( S_ISDIR ( st . st_mode ) )
            1.1          (rodarmer 13-Nov-09):             return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcIncorrect );
         */
    RCt = TheInfo -> Type == kxfsFile
                        ? KDirectoryFileSize (
                                        TheDirectory,
                                        & ( TheInfo -> Size ),
                                        "%s", ThePath
                                        )
                        : 0
                        ;

    if ( RCt == 0 ) { 
        RCt = KDirectoryDate (
                            TheDirectory,
                            & ( TheInfo -> Time ),
                            "%s", ThePath
                            );
        if ( RCt == 0 ) {
            RCt = KDirectoryAccess (
                                TheDirectory,
                                & ( TheInfo -> Access ),
                                "%s", ThePath
                                );
        }
    }

    return RCt;
}   /* XFSPeer_FillXFSPeerObjectInfo () */

/*  That method is fully complimentary to XFSPeer_FillXFSPeerObjectInfo
 *  the only different is it is acting like 'fstat()' throught handle.
 *  Works for files only :)
 */
static
rc_t
XFSPeer_FillXFSPeerFileInfo (
                        const VFSManager * TheManager,
                        const struct VPath * TheVPath,
                        const struct KFile * TheFile,
                        XFSPeerObjectInfo * TheInfo
)
{
    rc_t RCt;
    KDirectory * TheDirectory;
    char ThePath [ XFS_SIZE_4096 ];
    KPathType TheType;

        /*  To be sure that we are right, we are cheking and returning
            file itself
         */
    if ( TheFile == NULL ) {
        return XFSPeer_FillXFSPeerObjectInfo (
                                            TheManager,
                                            TheVPath,
                                            TheInfo
                                            );
    }


        /*  First we are resetting that good ObjectInfo structure
         */
    memset ( TheInfo, 0, sizeof ( XFSPeerObjectInfo ) );
    TheInfo -> Type = kxfsBad;

        /*  Secont we should get KDirectory instance from Manager
         */
    RCt = VFSManagerGetCWD ( TheManager, & TheDirectory );
    if ( RCt != 0 ) {
        return RCt;
    }

        /*  Thirt we should convert VPath to char, which KDirectory
         *  understands well
         */
    RCt = XFSPeer_ReadVPath (
                            TheVPath,
                            ThePath,
                            sizeof ( ThePath ),
                            ""
                            );
    if ( RCt != 0 ) {
        return RCt;
    }

        /*  Now we should retrieve file type and fall out if something
         *  is wrong
         */
    TheType = KFileType ( TheFile );
    switch ( TheType ) {
        case kptFile:
            TheInfo -> Type = kxfsFile;
            break;
        case kptNotFound:
            TheInfo -> Type = kxfsNotFound;
            return XFS_RC ( rcUnknown );
        default:
            TheInfo -> Type = kxfsBad;
            return XFS_RC ( rcUnknown );
    }

        /*  Now we are goint to get size/date and access info
         */
    RCt = KFileSize ( TheFile, & ( TheInfo -> Size ) );
    if ( RCt == 0 ) { 
        RCt = KDirectoryDate (
                        TheDirectory,
                        & ( TheInfo -> Time ),
                        "%s", ThePath
                        );
        if ( RCt == 0 ) {
            RCt = KDirectoryAccess (
                                TheDirectory,
                                & ( TheInfo -> Access ),
                                "%s", ThePath
                                );
        }
    }

    return RCt;
}   /* XFSPeer_FillXFSPeerFileInfo () */

/*  Peer virtual methods
 */
rc_t CC
XFS_DUMMY_PEER_init_v1 ( XFSPeer * self )
{
    VFSManager * TheVFSManager;
    rc_t RCt;

    RCt = 0;
    TheVFSManager = NULL;

    if ( self != NULL ) {
        RCt = VFSManagerMake ( & TheVFSManager );

        if ( RCt == 0 ) {
            self -> Data = TheVFSManager;
        }
    }
    else {
        RCt = XFS_RC ( rcNull );
    }

    return RCt;
}   /* XFS_DUMMY_PEER_init_v1 () */

rc_t CC
XFS_DUMMY_PEER_destroy_v1 ( XFSPeer * self )
{
    VFSManager * TheVFSManager;
    rc_t RCt;

    RCt = 0;
    TheVFSManager = NULL;

    if ( self != NULL ) {
        TheVFSManager = ( VFSManager * )self -> Data;

            /*  Hard to tell if we need to destroy that manager or
                it will be destroyed automatically after releasing
                But, releasing looks for me as more clear solution
                If You need to destroy it, just uncomment next 
                commented line
             */
/* #define YES_DESTROY_VFS_MANAGER */
#ifdef YES_DESTROY_VFS_MANAGER
        VFSManagerDestroy ( TheVFSManager );
#else
        VFSManagerRelease ( (const VFSManager * ) TheVFSManager );
#endif /* YES_DESTROY_VFS_MANAGER */
    }
    else {
        RCt = XFS_RC ( rcNull );
    }

    return RCt;
} /* XFS_DUMMY_PEER_destroy_v1 () */

/*  Because VFS is using the same function for unlink and rmdir
 */
static
rc_t CC
XFS_DUMMY_PEER_unlink_rmdir_v1 (
                        XFSPeer * self,
                        const struct VPath * Path,
                        const char * Operation
)
{
    VFSManager * TheVFSManager;
    struct VPath * ResolvedPath;
    rc_t RCt;

#ifdef XFS_EXTENDED_LOGGING
    {
        char BF [ XFS_SIZE_4096 ];

        XFSPeer_ReadVPath ( Path, BF, sizeof ( BF ), NULL );

        OUTMSG ( ( " %s(Dummy): [%s]\n", Operation, BF ) );
    }
#endif /* XFS_EXTENDED_LOGGING */

    RCt = XFSPeer_CheckCommonParameterReturnGoodOnes (
                                                    self,
                                                    Path,
                                                    rcRemoving,
                                                    & TheVFSManager,
                                                    & ResolvedPath
                                                    );
    if ( RCt == 0 ) {
        RCt = VFSManagerRemove ( TheVFSManager, true, ResolvedPath );

        VPathRelease ( ResolvedPath );
    }

    return RCt;
}   /* XFS_DUMMY_PEER_unlink_rmdir_v1 () */


rc_t CC
XFS_DUMMY_PEER_unlink_v1 (
                        XFSPeer * self,
                        const struct VPath * Path
)
{
    return XFS_DUMMY_PEER_unlink_rmdir_v1 ( self, Path, "UNLINK" );
} /* XFS_DUMMY_PEER_unlink_v1 () */

rc_t CC
XFS_DUMMY_PEER_rmdir_v1 (
                        XFSPeer * self,
                        const struct VPath * Path
)
{
    return XFS_DUMMY_PEER_unlink_rmdir_v1 ( self, Path, "RMDIR" );
} /* XFS_DUMMY_PEER_rmdir_v1 () */

rc_t CC
XFS_DUMMY_PEER_rename_v1 (
                        XFSPeer * self,
                        const struct VPath * OldPath,
                        const struct VPath * NewPath
)
{
    VFSManager * TheVFSManager;
    struct VPath * OldResolvedPath, * NewResolvedPath;
    char OldPathChar [ XFS_SIZE_4096 ], NewPathChar [ XFS_SIZE_4096 ];
    KDirectory * TheKDirectory;
    rc_t RCt;

#ifdef XFS_EXTENDED_LOGGING
    {
        char OBF [ XFS_SIZE_4096 ], NBF [ XFS_SIZE_4096 ];

        XFSPeer_ReadVPath ( OldPath, OBF, sizeof ( OBF ), NULL );
        XFSPeer_ReadVPath ( NewPath, NBF, sizeof ( NBF ), NULL );

        OUTMSG ( ( " RENAME(Dummy): [%s] to [%s]\n", OBF, NBF ) );
    }
#endif /* XFS_EXTENDED_LOGGING */

    RCt = 0;

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( OldPath == NULL || NewPath == NULL ) {
        return XFS_RC ( rcNull );
    }

    TheVFSManager = ( VFSManager * ) self -> Data;

    RCt = XFSPeer_ResolvePath_v1 ( self, OldPath, & OldResolvedPath );
    if ( RCt == 0 ) {
        RCt = XFSPeer_ResolvePath_v1 ( self, NewPath, & NewResolvedPath );
        if ( RCt == 0 ) {
                /* TODO!!! what should we do here ? */

            RCt = XFSPeer_ReadVPath (
                            OldResolvedPath,
                            OldPathChar,
                            sizeof ( OldPathChar ),
                            NULL
                            );

            if ( RCt == 0 ) {
                RCt = XFSPeer_ReadVPath (
                                NewResolvedPath,
                                NewPathChar,
                                sizeof ( NewPathChar ),
                                NULL
                                );
            }

            if ( RCt == 0 ) {
                RCt = VFSManagerGetCWD ( TheVFSManager, & TheKDirectory );
                if ( RCt == 0 ) {
                    RCt = KDirectoryRename (
                                        TheKDirectory,
                                        true,
                                        OldPathChar,
                                        NewPathChar
                                        );
                    KDirectoryRelease ( TheKDirectory );
                }
            }

            VPathRelease ( NewResolvedPath );
        }

        VPathRelease ( OldResolvedPath );
    }

    return RCt;
} /* XFS_DUMMY_PEER_rename_v1 () */

rc_t CC
XFS_DUMMY_PEER_truncate_v1 (
                            XFSPeer * self,
                            const struct VPath * ThePath,
                            uint64_t TheSize 
)
{
    rc_t RCt;
    VFSManager * TheManager;
    KDirectory * TheDirectory;
    struct VPath * ResolvedPath;
    char CharPath [ XFS_SIZE_4096 ];

#ifdef XFS_EXTENDED_LOGGING
    {
        char BF [ XFS_SIZE_4096 ];

        XFSPeer_ReadVPath ( ThePath, BF, sizeof ( BF ), NULL );

        OUTMSG ( ( " TRUNCATE(Dummy): [%s][SZ=%d]\n", BF, TheSize ) );
    }
#endif /* XFS_EXTENDED_LOGGING */

    RCt = XFSPeer_CheckCommonParameterReturnGoodOnes (
                                                    self,
                                                    ThePath,
                                                    rcResizing,
                                                    & TheManager,
                                                    & ResolvedPath
                                                    );
    if ( RCt == 0 ) {

        RCt = VFSManagerGetCWD ( TheManager, & TheDirectory );
        if ( RCt == 0 ) {
            RCt = XFSPeer_ReadVPath (
                                    ResolvedPath,
                                    CharPath,
                                    sizeof ( CharPath ),
                                    ""
                                    );
            if ( RCt == 0 ) {
                RCt = KDirectorySetFileSize (
                                        TheDirectory,
                                        TheSize,
                                        "%s", CharPath
                                        );

            }
        }

        VPathRelease ( ResolvedPath );
    }

    return RCt;
}   /* XFS_DUMMY_PEER_truncate_v1 () */

rc_t CC
XFS_DUMMY_PEER_getattr_v1 (
                        XFSPeer * self,
                        const struct VPath * ThePath,
                        XFSPeerObjectInfo * TheInfo
)
{
    rc_t RCt;
    VFSManager * TheVFSManager;
    struct VPath * ResolvedPath;

#ifdef XFS_EXTENDED_LOGGING
    {
        char BF [ XFS_SIZE_4096 ];

        XFSPeer_ReadVPath ( ThePath, BF, sizeof ( BF ), NULL );

        OUTMSG ( ( " GETATTR(Dummy): [%s][FI=0x%p]\n", BF, TheInfo ) );
    }
#endif /* XFS_EXTENDED_LOGGING */

    if ( TheInfo == NULL ) {
        return XFS_RC ( rcNull );
    }

    memset ( TheInfo, 0, sizeof ( XFSPeerObjectInfo ) );

    RCt = XFSPeer_CheckCommonParameterReturnGoodOnes (
                                                    self,
                                                    ThePath,
                                                    rcAccessing,
                                                    & TheVFSManager,
                                                    & ResolvedPath
                                                    );
    if ( RCt == 0 ) {
        RCt = XFSPeer_FillXFSPeerObjectInfo (
                                        TheVFSManager,
                                        ResolvedPath,
                                        TheInfo
                                        );

        VPathRelease ( ResolvedPath );
    }

    return RCt;
}   /* XFS_DUMMY_PEER_getattr_v1 () */

rc_t CC
XFS_DUMMY_PEER_mkdir_v1 (
                    XFSPeer * self,
                    const struct VPath * ThePath,
                    uint32_t Access
)
{
    rc_t RCt;
    VFSManager * TheVFSManager;
    struct VPath * ResolvedPath;
    KDirectory * TheDirectory;
    char CharPath [ XFS_SIZE_4096 ];

#ifdef XFS_EXTENDED_LOGGING
    {
        char BF [ XFS_SIZE_4096 ];

        XFSPeer_ReadVPath ( ThePath, BF, sizeof ( BF ), NULL );

        OUTMSG ( ( " MKDIR(Dummy): [%s][mode=%d]\n", BF, Access ) );
    }
#endif /* XFS_EXTENDED_LOGGING */

    RCt = XFSPeer_CheckCommonParameterReturnGoodOnes (
                                                    self,
                                                    ThePath,
                                                    rcCreating,
                                                    & TheVFSManager,
                                                    & ResolvedPath
                                                    );
    if ( RCt == 0 ) {
        RCt = VFSManagerGetCWD ( TheVFSManager, & TheDirectory );
        if ( RCt == 0 ) {
            RCt = XFSPeer_ReadVPath (
                                ResolvedPath,
                                CharPath,
                                sizeof ( CharPath ),
                                ""
                                );
            if ( RCt == 0 ) {
                RCt = KDirectoryCreateDir (
                                        TheDirectory,
                                        Access,
                                        kcmCreate,
                                        "%s", CharPath
                                        );
            }
        }

        VPathRelease ( ResolvedPath );
    }

    return RCt;
}   /* XFS_DUMMY_PEER_mkdir_v1 () */

rc_t CC
XFS_DUMMY_PEER_utime_v1 (
                        XFSPeer * self,
                        const struct VPath * ThePath,
                        KTime_t TimeToSet
)
{
    rc_t  RCt;
    VFSManager * TheVFSManager;
    struct VPath * ResolvedPath;
    KDirectory * TheDirectory;
    char CharPath [ XFS_SIZE_4096 ];

#ifdef XFS_EXTENDED_LOGGING
    {
        char BF [ XFS_SIZE_4096 ];

        XFSPeer_ReadVPath ( ThePath, BF, sizeof ( BF ), NULL );

        OUTMSG ( ( " UTIME(Dummy): [%s][TM=%d]\n", BF, TimeToSet ) );
    }
#endif /* XFS_EXTENDED_LOGGING */

    RCt = XFSPeer_CheckCommonParameterReturnGoodOnes (
                                                    self,
                                                    ThePath,
                                                    rcAccessing,
                                                    & TheVFSManager,
                                                    & ResolvedPath
                                                    );
    if ( RCt == 0 ) {

        RCt = VFSManagerGetCWD ( TheVFSManager, & TheDirectory );
        if ( RCt == 0 ) {
            RCt = XFSPeer_ReadVPath (
                                ResolvedPath,
                                CharPath,
                                sizeof ( CharPath ),
                                ""
                                );
            if ( RCt == 0 ) {
                RCt = KDirectorySetDate (
                                    TheDirectory,
                                    false,
                                    TimeToSet,
                                    "%s", CharPath
                                    );
            }
        }

        VPathRelease ( ResolvedPath );
    }

    return RCt;
}   /* XFS_DUMMY_PEER_utume_v1 () */

rc_t CC
XFS_DUMMY_PEER_release_v1 (
                            XFSPeer * self,
                            const struct VPath * ThePath,
                            struct KFile * TheFile
                            )
{
    rc_t RCt;
    VFSManager * TheVFSManager;
    struct VPath * ResolvedPath;

#ifdef XFS_EXTENDED_LOGGING
    {
        char BF [ XFS_SIZE_4096 ];

        XFSPeer_ReadVPath ( ThePath, BF, sizeof ( BF ), NULL );

        OUTMSG ( ( " RELEASE(Dummy): [%s][FH=0x%p]\n", BF, TheFile ) );
    }
#endif /* XFS_EXTENDED_LOGGING */

    if ( TheFile == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = XFSPeer_CheckCommonParameterReturnGoodOnes (
                                                    self,
                                                    ThePath,
                                                    rcOpening,
                                                    & TheVFSManager,
                                                    & ResolvedPath
                                                    );
    if ( RCt == 0 ) {
        RCt = KFileRelease ( TheFile );

        VPathRelease ( ResolvedPath );
    }

    return RCt;
}   /* XFS_DUMMY_PEER_release_v1 () */

rc_t CC
XFS_DUMMY_PEER_open_v1 (
                        XFSPeer * self,
                        const struct VPath * ThePath,
                        struct KFile ** File,
                        uint32_t Flags
)
{
    rc_t RCt;
    VFSManager * TheVFSManager;
    struct VPath * ResolvedPath;

#ifdef XFS_EXTENDED_LOGGING
    {
        const char * StrFlags;

        char BF [ XFS_SIZE_4096 ];

        XFSPeer_ReadVPath ( ThePath, BF, sizeof ( BF ), NULL );

        StrFlags = ( ( Flags & O_RDWR ) == O_RDWR ) 
                        ?   "O_RDWR"
                        :   ( ( ( Flags & O_WRONLY ) == O_WRONLY )
                                ?   "O_WRONLY"
                                :   "O_RDONLY" )
                        ;

        OUTMSG ( ( " OPEN(Dummy): [%s][%s]\n", BF, StrFlags ) );
    }
#endif /* XFS_EXTENDED_LOGGING */

    if ( File == NULL ) {
        return XFS_RC ( rcNull );
    }

    * File = NULL;

    RCt = XFSPeer_CheckCommonParameterReturnGoodOnes (
                                                    self,
                                                    ThePath,
                                                    rcOpening,
                                                    & TheVFSManager,
                                                    & ResolvedPath
                                                    );
    if ( RCt == 0 ) {
        if ( ( Flags & O_WRONLY ) == O_WRONLY ) {
            RCt = VFSManagerOpenFileWrite (
                                        TheVFSManager,
                                        File,
                                        ( Flags & O_RDWR) == O_RDWR,
                                        ResolvedPath
                                        );
        }
        else {
            RCt = VFSManagerOpenFileRead (
                                     TheVFSManager,
                                     ( struct KFile const ** ) File,
                                     ResolvedPath
                                     );
        }

        if ( RCt != 0 ) {
            *File = NULL;
        }

        VPathRelease ( ResolvedPath );
    }

#ifdef XFS_EXTENDED_LOGGING
    {
        char BF [ XFS_SIZE_4096 ];

        XFSPeer_ReadVPath ( ThePath, BF, sizeof ( BF ), NULL );

        OUTMSG ( ( " OPEN(Dummy, cont): [%s][FH=0x%p]\n", BF, * File ) );
    }
#endif /* XFS_EXTENDED_LOGGING */

    return RCt;
}   /* XFS_DUMMY_PEER_open_v1 () */

    /*  The methods opendir and releasedir are fake. KDirectory
     *  does not need any Dir to list directory entries. So, we
     *  will fake these, for a case if we will need to implement
     *  it if future. So, now it is returning hash value for path.
     */
rc_t CC
XFS_DUMMY_PEER_opendir_v1 (
                        XFSPeer * self,
                        const struct VPath * ThePath,
                        void ** KoreWa
)
{
    rc_t RCt;
    VFSManager * TheVFSManager;
    struct VPath * ResolvedPath;
    char CharPath [ XFS_SIZE_4096 ];
    int64_t HashValue;

#ifdef XFS_EXTENDED_LOGGING
    {
        char BF [ XFS_SIZE_4096 ];

        XFSPeer_ReadVPath ( ThePath, BF, sizeof ( BF ), NULL );

        OUTMSG ( ( " OPENDIR(Dummy): [%s]\n", BF ) );
    }
#endif /* XFS_EXTENDED_LOGGING */

    if ( KoreWa == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = XFSPeer_CheckCommonParameterReturnGoodOnes (
                                                    self,
                                                    ThePath,
                                                    rcOpening,
                                                    & TheVFSManager,
                                                    & ResolvedPath
                                                    );
    if ( RCt == 0 ) {
        RCt = XFSPeer_ReadVPath (
                                ResolvedPath,
                                CharPath,
                                sizeof ( CharPath ),
                                ""
                                );

        HashValue = string_hash ( CharPath, string_size ( CharPath ) );

        * KoreWa = ( void * ) HashValue;

#ifdef XFS_EXTENDED_LOGGING
    {
        char BF [ XFS_SIZE_4096 ];

        XFSPeer_ReadVPath ( ThePath, BF, sizeof ( BF ), NULL );

        OUTMSG ( ( " OPENDIR(Dummy, cont): [%s][FH=0x%p]\n", BF, HashValue ) );
    }
#endif /* XFS_EXTENDED_LOGGING */

        VPathRelease ( ResolvedPath );
    }

    return RCt;
}   /* XFS_DUMMY_PEER_opendir_v1 () */

rc_t CC
XFS_DUMMY_PEER_releasedir_v1 (
                        XFSPeer * self,
                        const struct VPath * ThePath,
                        void * SoreWa
)
{
    rc_t RCt;
    VFSManager * TheVFSManager;
    struct VPath * ResolvedPath;
    char CharPath [ XFS_SIZE_4096 ];
    int64_t HashValue;

    HashValue = SoreWa == NULL ? 0 : ( int64_t ) SoreWa;

#ifdef XFS_EXTENDED_LOGGING
    {
        char BF [ XFS_SIZE_4096 ];

        XFSPeer_ReadVPath ( ThePath, BF, sizeof ( BF ), NULL );

        OUTMSG ( ( " RELEASEDIR(Dummy): [%s][FH=0x%p]\n", BF, HashValue ) );
    }
#endif /* XFS_EXTENDED_LOGGING */

    if ( SoreWa == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = XFSPeer_CheckCommonParameterReturnGoodOnes (
                                                    self,
                                                    ThePath,
                                                    rcOpening,
                                                    & TheVFSManager,
                                                    & ResolvedPath
                                                    );
    if ( RCt == 0 ) {
        RCt = XFSPeer_ReadVPath (
                                ResolvedPath,
                                CharPath,
                                sizeof ( CharPath ),
                                ""
                                );

        if ( RCt == 0 ) {
            if ( HashValue != string_hash (
                                        CharPath,
                                        string_size ( CharPath )
                                        ) ) {
                RCt = RC ( rcFS, rcNoTarg, rcClosing, rcDirectory, rcInvalid );
            }
        }

        VPathRelease ( ResolvedPath );
    }

    return RCt;
}   /* XFS_DUMMY_PEER_releasedir_v1 () */

static
rc_t CC
XFS_DUMMY_PEER_read_write_v1 (
                        XFSPeer * self,
                        const struct VPath * ThePath,
                        struct KFile * TheFile,
                        uint64_t Offset,
                        void * Buffer,
                        size_t Size,
                        size_t *NumBytes,
                        bool Read
)
{
    rc_t RCt;
    VFSManager * TheVFSManager;
    struct VPath * ResolvedPath;
    const char * Operation;
    int32_t Context;

    Operation = Read ? "READ" : "WRITE";
    Context = Read ? rcReading : rcWriting;

#ifdef XFS_EXTENDED_LOGGING
    {
        char BF [ XFS_SIZE_4096 ];

        XFSPeer_ReadVPath ( ThePath, BF, sizeof ( BF ), NULL );

        OUTMSG ( ( " %s(Dummy): [%s][FH=0x%p][OF=%d SZ=%d]\n", Operation, BF, TheFile, Offset, Size ) );
    }
#endif /* XFS_EXTENDED_LOGGING */

    if ( TheFile == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = XFSPeer_CheckCommonParameterReturnGoodOnes (
                                                    self,
                                                    ThePath,
                                                    Context,
                                                    & TheVFSManager,
                                                    & ResolvedPath
                                                    );
    if ( RCt == 0 ) {
        if ( Read ) {
            RCt = KFileReadAll (
                            TheFile,
                            Offset,
                            Buffer,
                            Size,
                            NumBytes
                            );
        }
        else {
            RCt = KFileWriteAll (
                            TheFile,
                            Offset,
                            Buffer,
                            Size,
                            NumBytes
                            );
        }

        VPathRelease ( ResolvedPath );
    }

#ifdef XFS_EXTENDED_LOGGING
    {
        char BF [ XFS_SIZE_4096 ];

        XFSPeer_ReadVPath ( ThePath, BF, sizeof ( BF ), NULL );

        OUTMSG ( ( " %s(Dummy, cont): [%s][FH=0x%p][BR=%d]\n", Operation, BF, TheFile, * NumBytes ) );
    }
#endif /* XFS_EXTENDED_LOGGING */

    return RCt;
}   /* XFS_DUMMY_PEER_read_write_v1 () */

rc_t CC
XFS_DUMMY_PEER_read_v1 (
                        XFSPeer * self,
                        const struct VPath * ThePath,
                        struct KFile * TheFile,
                        uint64_t ReadOffset,
                        void * Buffer,
                        size_t BufferSize,
                        size_t *NumBytesReaded
)
{
    return XFS_DUMMY_PEER_read_write_v1 (
                                    self,
                                    ThePath,
                                    TheFile,
                                    ReadOffset,
                                    Buffer,
                                    BufferSize,
                                    NumBytesReaded,
                                    true
                                );
}   /* XFS_DUMMY_PEER_read_v1 () */

rc_t CC
XFS_DUMMY_PEER_write_v1 (
                        XFSPeer * self,
                        const struct VPath * ThePath,
                        struct KFile * TheFile,
                        uint64_t WriteOffset,
                        void * Buffer,
                        size_t BufferSize,
                        size_t *NumBytesWritten
)
{
    return XFS_DUMMY_PEER_read_write_v1 (
                                    self,
                                    ThePath,
                                    TheFile,
                                    WriteOffset,
                                    Buffer,
                                    BufferSize,
                                    NumBytesWritten,
                                    false
                                    );
}   /* XFS_DUMMY_PEER_write_v1 () */

rc_t CC
XFS_DUMMY_PEER_readdir_v1 (
                        XFSPeer * self,
                        const struct VPath * ThePath,
                        void * AreWa,
                        struct KNamelist ** TheList
)
{
    rc_t RCt;
    VFSManager * TheVFSManager;
    struct VPath * ResolvedPath;
    char CharPath [ XFS_SIZE_4096 ];
    KDirectory * TheDirectory;
    int64_t HashValue;

    HashValue = AreWa == NULL ? 0 : ( int64_t ) AreWa;

#ifdef XFS_EXTENDED_LOGGING
    {
        char BF [ XFS_SIZE_4096 ];

        XFSPeer_ReadVPath ( ThePath, BF, sizeof ( BF ), NULL );

        OUTMSG ( ( " READDIR(Dummy): [%s][FH=0x%p]\n", BF, HashValue ) );
    }
#endif /* XFS_EXTENDED_LOGGING */

    if ( AreWa == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = XFSPeer_CheckCommonParameterReturnGoodOnes (
                                                    self,
                                                    ThePath,
                                                    rcOpening,
                                                    & TheVFSManager,
                                                    & ResolvedPath
                                                    );
    if ( RCt == 0 ) {
        RCt = XFSPeer_ReadVPath (
                                ResolvedPath,
                                CharPath,
                                sizeof ( CharPath ),
                                ""
                                );

        if ( RCt == 0 ) {
            if ( HashValue != string_hash (
                                        CharPath,
                                        string_size ( CharPath )
                                        ) ) {
                RCt = RC ( rcFS, rcNoTarg, rcClosing, rcDirectory, rcInvalid );
            }
            else {
                RCt = VFSManagerGetCWD (
                                    TheVFSManager,
                                    & TheDirectory
                                    );
                if ( RCt == 0 ) {
                    RCt = KDirectoryList (
                                        TheDirectory,
                                        TheList,
                                        NULL,
                                        NULL,
                                        "%s", 
                                        CharPath
                                        );
                }
            }
        }

        VPathRelease ( ResolvedPath );
    }

    return RCt;
}   /* XFS_DUMMY_PEER_readdir_v1 () */

rc_t CC
XFS_DUMMY_PEER_access_v1 (
                        XFSPeer * self,
                        const struct VPath * ThePath,
                        uint32_t NonDefaultPermissions
)
{
    rc_t RCt;
    VFSManager * TheVFSManager;
    struct VPath * ResolvedPath;
    KDirectory * TheDirectory;
    char CharPath [ XFS_SIZE_4096 ];

#ifdef XFS_EXTENDED_LOGGING
    {
        char BF [ XFS_SIZE_4096 ];

        XFSPeer_ReadVPath ( ThePath, BF, sizeof ( BF ), NULL );

        OUTMSG ( ( " ACCESS(Dummy): [%s][mode=%d]\n", BF, NonDefaultPermissions ) );
    }
#endif /* XFS_EXTENDED_LOGGING */

    RCt = XFSPeer_CheckCommonParameterReturnGoodOnes (
                                                    self,
                                                    ThePath,
                                                    rcAccessing,
                                                    & TheVFSManager,
                                                    & ResolvedPath
                                                    );
    if ( RCt == 0 ) {

        RCt = VFSManagerGetCWD ( TheVFSManager, & TheDirectory );
        if ( RCt == 0 ) {
            RCt = XFSPeer_ReadVPath (
                                ResolvedPath,
                                CharPath,
                                sizeof ( CharPath ),
                                ""
                                );
            if ( RCt == 0 ) {
                RCt = KDirectorySetAccess (
                                    TheDirectory,
                                    false,
                                    NonDefaultPermissions,
                                    07777,
                                    "%s", 
                                    CharPath
                                    );
            }
        }

        VPathRelease ( ResolvedPath );
    }

    return RCt;
}   /* XFS_DUMMY_PEER_access_v1 () */

rc_t CC
XFS_DUMMY_PEER_create_v1 (
                        XFSPeer * self,
                        const struct VPath * ThePath,
                        struct KFile ** TheFile,
                        uint32_t Flags,
                        uint32_t Access
)
{
    rc_t RCt;
    VFSManager * TheVFSManager;
    struct VPath * ResolvedPath;
    KDirectory * TheDirectory;
    char CharPath [ XFS_SIZE_4096 ];

#ifdef XFS_EXTENDED_LOGGING
    {
        const char * StrFlags;

        char BF [ XFS_SIZE_4096 ];

        XFSPeer_ReadVPath ( ThePath, BF, sizeof ( BF ), NULL );

        StrFlags = ( ( Flags & O_RDWR ) == O_RDWR ) 
                        ?   "O_RDWR"
                        :   ( ( ( Flags & O_WRONLY ) == O_WRONLY )
                                ?   "O_WRONLY"
                                :   "O_RDONLY" )
                        ;

        OUTMSG ( ( " CREATE(Dummy): [%s][%s][mode=%d]\n", BF, StrFlags, Access ) );
    }
#endif /* XFS_EXTENDED_LOGGING */

    if ( TheFile == NULL ) {
        return XFS_RC ( rcNull );
    }

    * TheFile = NULL;

    RCt = XFSPeer_CheckCommonParameterReturnGoodOnes (
                                                    self,
                                                    ThePath,
                                                    rcCreating,
                                                    & TheVFSManager,
                                                    & ResolvedPath
                                                    );
    if ( RCt == 0 ) {
        RCt = VFSManagerGetCWD ( TheVFSManager, & TheDirectory );
        if ( RCt == 0 ) {
            RCt = XFSPeer_ReadVPath (
                                ResolvedPath,
                                CharPath,
                                sizeof ( CharPath ),
                                ""
                                );
            if ( RCt == 0 ) {
                RCt = KDirectoryCreateFile (
                                    TheDirectory,
                                    TheFile,
                                    ( Flags & O_RDWR) == O_RDWR,
                                    Access,
                                    Flags,
                                    "%s", CharPath
                                    );
            }
        }

        VPathRelease ( ResolvedPath );
    }

    return RCt;
}   /* XFS_DUMMY_PEER_create_v1 () */

rc_t CC
XFS_DUMMY_PEER_ftruncate_v1 (
                            XFSPeer * self,
                            const struct VPath * ThePath,
                            struct KFile * TheFile,
                            uint64_t TheSize 
)
{
    rc_t RCt;
    VFSManager * TheManager;
    struct VPath * ResolvedPath;

#ifdef XFS_EXTENDED_LOGGING
    {
        char BF [ XFS_SIZE_4096 ];

        XFSPeer_ReadVPath ( ThePath, BF, sizeof ( BF ), NULL );

        OUTMSG ( ( " FTRUNCATE(Dummy): [%s][SZ=%d][FH=0x%p]\n", BF, TheSize, TheFile ) );
    }
#endif /* XFS_EXTENDED_LOGGING */

    if ( TheFile == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = XFSPeer_CheckCommonParameterReturnGoodOnes (
                                                    self,
                                                    ThePath,
                                                    rcResizing,
                                                    & TheManager,
                                                    & ResolvedPath
                                                    );
    if ( RCt == 0 ) {
        RCt = KFileSetSize ( TheFile, TheSize );

        VPathRelease ( ResolvedPath );
    }

    return RCt;
}   /* XFS_DUMMY_PEER_ftruncate_v1 () */

rc_t CC
XFS_DUMMY_PEER_fgetattr_v1 (
                        XFSPeer * self,
                        const struct VPath * ThePath,
                        const struct KFile * TheFile,
                        XFSPeerObjectInfo * TheInfo
)
{
    rc_t RCt;
    VFSManager * TheVFSManager;
    struct VPath * ResolvedPath;

#ifdef XFS_EXTENDED_LOGGING
    {
        char BF [ XFS_SIZE_4096 ];

        XFSPeer_ReadVPath ( ThePath, BF, sizeof ( BF ), NULL );

        OUTMSG ( ( " FGETATTR(Dummy): [%s][FI=0x%p][FH=0x%p]\n", BF, TheInfo, TheFile ) );
    }
#endif /* XFS_EXTENDED_LOGGING */

    if ( TheInfo == NULL || TheFile == NULL ) {
        return XFS_RC ( rcNull );
    }

    memset ( TheInfo, 0, sizeof ( XFSPeerObjectInfo ) );

    RCt = XFSPeer_CheckCommonParameterReturnGoodOnes (
                                                    self,
                                                    ThePath,
                                                    rcAccessing,
                                                    & TheVFSManager,
                                                    & ResolvedPath
                                                    );
    if ( RCt == 0 ) {
        RCt = XFSPeer_FillXFSPeerFileInfo (
                                        TheVFSManager,
                                        ResolvedPath,
                                        TheFile,
                                        TheInfo
                                        );

        VPathRelease ( ResolvedPath );
    }

    return RCt;
}   /* XFS_DUMMY_PEER_fgetattr_v1 () */
