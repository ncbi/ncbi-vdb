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

 /* Something unusual: Lyrics
  *
  * That file contains only one useful function:
  *
  *                       XFS_Private_InitOperations
  *
  * it needed to fill fuse_operations structure.
  *
  * I put here all possible ( for 2.5 ) stubs to fuse_operations
  * functions. However, I will use only several of them while 
  * structure initialisation. So, if You want to extend functionality,
  * please edit already ready stub and add new function to structure
  * initialized .
  *
  */

#include <klib/out.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

#include <sysalloc.h>
#include <string.h> /* we are using memset() */
#include <stdio.h>  /* trash, need to be removed, after adding VFM */
#include <dirent.h> /* trash, need to be removed, after adding VFM */
#include <unistd.h> /* trash, need to be removed, after adding VFM */

#include <klib/namelist.h>
#include <kfs/file.h>
#include <vfs/path.h>
#include <vfs/manager.h>
#include <xfs/xfs-peer.h>

#include "xfs-native-peer-operations.h"


/****************************************************************
 * Something weird, but it could simplify somebody's life
 *
 *      FUSE method            Use/Not  Implemented  Comment
 ****************************************************************/
#define USE_XFS_FUSE_GETATTR      1     /* + */
#define USE_XFS_FUSE_READLINK     0     /* - */ /* ? */
#define USE_XFS_FUSE_GETDIR       0     /* - */ /* Obsolete method */
#define USE_XFS_FUSE_MKNOD        0     /* - */ /* Do not need */
#define USE_XFS_FUSE_MKDIR        1     /* + */
#define USE_XFS_FUSE_UNLINK       1     /* + */
#define USE_XFS_FUSE_RMDIR        1     /* + */
#define USE_XFS_FUSE_SYMLINK      0     /* - */ /* ? */
#define USE_XFS_FUSE_RENAME       1     /* + */
#define USE_XFS_FUSE_LINK         0     /* - */ /* ? */
#define USE_XFS_FUSE_CHMOD        0     /* - */ /* ? */
#define USE_XFS_FUSE_CHOWN        0     /* - */ /* Do not need */
#define USE_XFS_FUSE_TRUNCATE     1     /* + */
#define USE_XFS_FUSE_UTIME        1     /* + */
#define USE_XFS_FUSE_OPEN         1     /* + */
#define USE_XFS_FUSE_READ         1     /* + */
#define USE_XFS_FUSE_WRITE        1     /* + */
#define USE_XFS_FUSE_STATFS       0     /* - */ /* ? */
#define USE_XFS_FUSE_FLUSH        0     /* - */ /* Dummy ... ? */
#define USE_XFS_FUSE_RELEASE      1     /* + */
#define USE_XFS_FUSE_FSYNC        1     /* + */ /* ? */
#define USE_XFS_FUSE_SETXATTR     0     /* - */ /* Do not need */
#define USE_XFS_FUSE_GETXATTR     0     /* - */ /* Do not need */
#define USE_XFS_FUSE_LISTXATTR    0     /* - */ /* Do not need */
#define USE_XFS_FUSE_REMOVEXATTR  0     /* - */ /* Do not need */
#define USE_XFS_FUSE_OPENDIR      1     /* + */
#define USE_XFS_FUSE_READDIR      1     /* + */
#define USE_XFS_FUSE_RELEASEDIR   1     /* + */
#define USE_XFS_FUSE_FSYNCDIR     0     /* - */ /* ? */
#define USE_XFS_FUSE_INIT         1     /* + */ /* Dummy ??? */
#define USE_XFS_FUSE_DESTROY      1     /* + */ /* Dummy ??? */
#define USE_XFS_FUSE_ACCESS       1     /* + */
#define USE_XFS_FUSE_CREATE       1     /* + */
#define USE_XFS_FUSE_FTRUNCATE    1     /* + */ /* ? */
#define USE_XFS_FUSE_FGETATTR     1     /* + */
#define USE_XFS_FUSE_LOCK         0     /* - */ /* Do not need */
#define USE_XFS_FUSE_UTIMENS      0     /* - */ /* Do not need */
#define USE_XFS_FUSE_BMAP         0     /* - */ /* Do not need */
#define USE_XFS_FUSE_IOCTL        0     /* - */ /* Do not need */
#define USE_XFS_FUSE_POLL         0     /* - */ /* Do not need */


/*****************************************************
 * Something unusual
 * NOTE: that procedure is getting presetted peer from
 *       fuse_context structure and context is valid
 *       only while operation call. So, do not use that
 *       method withoud being harmed
 *****************************************************/
static
rc_t
GetPeerFromFuseHaHa ( struct XFSPeer ** Peer )
{
    struct fuse_context * TheContext;

    if ( Peer == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Peer = NULL;

    TheContext = fuse_get_context();
    if ( TheContext == NULL ) {
        OUTMSG ( ( "ERROR: improper usage of 'fuse_get_context()'\n" ) );
        return XFS_RC ( rcNull );
    }

    * Peer = ( struct XFSPeer * ) TheContext -> private_data;
    if ( * Peer == NULL ) {
        return XFS_RC ( rcNull );
    }

    return 0;
}   /* GetPeerFromFuseHaHa () */

/*****************************************************
 * Here are convertors
 *****************************************************/
static
rc_t
StatStructureToXFSPeerObjectInfo (
                    const struct XFSPeerObjectInfo * TheInfo,
                    struct stat * TheStat
)
{
    if ( TheInfo == NULL || TheStat == NULL ) {
        return XFS_RC ( rcNull );
    }

                /* Here we are trying to convert ObjectInfo to stat
                 */
    memset ( TheStat, 0, sizeof ( struct stat ) );
    TheStat -> st_mode = TheInfo -> Access;
    switch ( TheInfo -> Type ) {
        case kxfsLink:
            TheStat -> st_mode |= S_IFLNK;
            break;
        case kxfsFile:
            TheStat -> st_mode |= S_IFREG;
            break;
        case kxfsDir:
            TheStat -> st_mode |= S_IFDIR;
            break;
        default:
            TheStat -> st_mode = 0;
            break;
    }

    if ( TheStat -> st_mode != 0 ) {
        TheStat -> st_uid = getuid();
        TheStat -> st_gid = getgid();
        TheStat -> st_size = TheInfo -> Size;
        TheStat -> st_blksize = XFS_SIZE_4096;
        TheStat -> st_atime = TheInfo -> Time;
        TheStat -> st_mtime = TheInfo -> Time;
        TheStat -> st_ctime = TheInfo -> Time;
    }

        /* TODO */
    return 0;
}   /* StatStructureToXFSPeerObjectInfo () */

/*
 $  Cache for less :)
 $  Apparently, I can not to interpret all errors correctly, and
 $  I will to the same thing as Anton: return EBADF.
 */
static
int
XFS_FUSE_rc_to_errno ( rc_t RCt )
{
    uint32_t Target, Object, State;

    State = GetRCState ( RCt );
    if ( State == rcNoErr ) {
        return 0;
    }

    Target = GetRCTarget ( RCt );
    Object = GetRCObject ( RCt );

    if ( State == rcUnknown && Object == rcTransfer ) {
        return EIO;
    }

    if ( Target == rcNoTarg && Object == rcNoObj ) {
        switch ( State ) {
                    /*  Missed Peer method */
            case rcUnsupported :    return ENOSYS;
                    /*  Some of peer parametes was NULL */
            case rcNull :           return EFAULT;
                    /*  Something is wrong */
            case rcUnknown :        return EBADF;
        }

        return EBADF;
    }

    if ( Target == rcDirectory ) {
        switch ( State ) {
            case rcUnauthorized :   return EACCES;
            case rcExhausted :      return Object == rcStorage
                                                        ? ENOSPC
                                                        : ENOMEM
                                                        ;
            case rcNotFound :       return ENOENT;
            case rcInvalid :
            case rcIncorrect :      return EINVAL;
            case rcExcessive :      return Object == rcParam
                                                        ? EFBIG
                                                        : ENAMETOOLONG
                                                        ;
            case rcExists :         return EEXIST;
            case rcBusy :           return EBUSY;
            case rcIncomplete :     return EINTR;
        }

        return EBADF;
    }

    if ( Target == rcFile ) {
        switch ( State ) {
            case rcExhausted :      return Object == rcStorage
                                                        ? ENOSPC
                                                        : ENOMEM
                                                        ;
            case rcIncomplete :     return EINTR;
            case rcExcessive :      return EFBIG;
            case rcIncorrect :      return EISDIR;
            case rcReadonly :       return EROFS;
            case rcInvalid :        return EINVAL;
        }

        return EBADF;
    }

    if ( Object == rcMemMap ) {
        switch ( State ) {
            case rcNoPerm :         return EACCES;
            case rcInvalid :        return EBADF;
            case rcIncomplete :     return EAGAIN;
            case rcExhausted :      return ENOMEM;
        }

        return EBADF;
    }

    return EBADF;
}   /* XFS_FUSE_rc_to_errno () */


/*****************************************************
 * Most of operations are providing Path and FileInfo
 * so, there is most popular checks in one method
 *****************************************************/
static
rc_t
XFS_FUSE_common_check (
                    const char * ThePath,
                    XFSPeer ** ThePeer,
                    VPath ** TheVPath
)
{
    rc_t RCt;
    VFSManager * Manager;

    if ( ThePath == 0 ) {
        return RC ( rcFS, rcNoTarg, rcAccessing, rcParam, rcNull );
    }

    * TheVPath = NULL;

    RCt = GetPeerFromFuseHaHa ( ThePeer );
    if ( RCt == 0 ) {
        Manager = ( VFSManager * )( * ThePeer ) -> Data;
        if ( Manager == NULL ) {
            RCt = RC ( rcFS, rcNoTarg, rcAccessing, rcParam, rcNull );
        }
        else {
            RCt = VFSManagerMakePath ( Manager, TheVPath, "%s", ThePath );
        }
    }

    return RCt;
}   /* XFS_FUSE_common_check () */


/*****************************************************
 * Operations
 *****************************************************/

/*****************************************************/
#if USE_XFS_FUSE_GETATTR == 1

static
int
XFS_FUSE_getattr ( const char * ThePath, struct stat * TheStat )
{
    rc_t RCt;
    XFSPeer * ThePeer;
    VPath * TheVPath;
    XFSPeerObjectInfo ThePeerObjectInfo;

    XFSMSG ( ( "GETATTR(Fuse): [%s]\n", ThePath ) );

    RCt = XFS_FUSE_common_check ( ThePath, & ThePeer, & TheVPath );

    if ( RCt == 0 ) {
        if ( ThePeer -> vt -> v1.getattr != NULL ) {
            RCt = ThePeer -> vt -> v1.getattr (
                                            ThePeer,
                                            TheVPath,
                                            & ThePeerObjectInfo
                                            );

                /* Here we are trying to convert ObjectInfo to stat
                 */
            if ( RCt == 0 ) {
                RCt = StatStructureToXFSPeerObjectInfo (
                                            & ThePeerObjectInfo,
                                            TheStat
                                            );
            }
            else {
                memset ( TheStat, 0, sizeof ( struct stat ) );
            }
        }
        else {
            XFSMSG ( ( "ERROR: Peer method 'getattr()' is not implemented\n" ) );
            RCt = XFS_RC ( rcUnsupported );
        }

        VPathRelease ( TheVPath );
    }

    return - XFS_FUSE_rc_to_errno ( RCt );
}   /* XFS_FUSE_getattr() */

#endif /* USE_XFS_FUSE_GETATTR == 1 */

/*****************************************************/
#if USE_XFS_FUSE_READLINK == 1

static
int
XFS_FUSE_readlink (
            const char * ThePath,
            char * RetBuf,
            size_t RetBufSize
)
{
    OUTMSG ( ( "READLINK(!): ThePath %s\n", ThePath ) );

    return -EPERM;
}   /* XFS_FUSE_readlink() */

#endif /* USE_XFS_FUSE_READLINK == 1 */

/*****************************************************/
#if USE_XFS_FUSE_GETDIR == 1

static
int
XFS_FUSE_getdir (
            const char * ThePath,
            fuse_dirh_t TheFDH,
            fuse_dirfil_t TheFDF
)
{
    OUTMSG ( ( "GETDIR(!): ThePath %s\n", ThePath ) );

    return -EPERM;
}   /* XFS_FUSE_getdir() */

#endif /* USE_XFS_FUSE_GETDIR == 1 */

/*****************************************************/
#if USE_XFS_FUSE_MKNOD == 1

static
int
XFS_FUSE_mknod ( const char * ThePath, mode_t TheMode, dev_t TheDev )
{
    OUTMSG ( ( "MKNOD(!): ThePath %s\n", ThePath ) );

    return -EPERM;
}   /* XFS_FUSE_mknod() */

#endif /* USE_XFS_FUSE_MKNOD == 1 */

/*****************************************************/
#if USE_XFS_FUSE_MKDIR == 1

static
int
XFS_FUSE_mkdir ( const char * ThePath, mode_t TheMode )
{
    rc_t RCt;
    XFSPeer * ThePeer;
    VPath * TheVPath;

    XFSMSG ( ( "MKDIR(Fuse): [%s][mode=%d]\n", ThePath, TheMode ) );

    RCt = XFS_FUSE_common_check ( ThePath, & ThePeer, & TheVPath );
    if ( RCt == 0 ) {
        if ( ThePeer -> vt -> v1.mkdir != NULL ) {
            RCt = ThePeer -> vt -> v1.mkdir ( ThePeer, TheVPath, TheMode );
        }
        else {
            XFSMSG ( ( "ERROR: Peer method 'mkdir()' is not implemented\n" ) );
            RCt = XFS_RC ( rcUnsupported );

        }

        VPathRelease ( TheVPath );
    }


    return - XFS_FUSE_rc_to_errno ( RCt );
}   /* XFS_FUSE_mkdir() */

#endif /* USE_XFS_FUSE_MKDIR == 1 */

/*****************************************************/
#if USE_XFS_FUSE_UNLINK == 1

static
int
XFS_FUSE_unlink ( const char * ThePath )
{
    rc_t RCt;
    XFSPeer * ThePeer;
    VPath * TheVPath;

    XFSMSG ( ( "UNLINK(Fuse): [%s]\n", ThePath ) );

    RCt = XFS_FUSE_common_check ( ThePath, & ThePeer, & TheVPath );

    if ( RCt == 0 ) {
        if ( ThePeer -> vt -> v1.unlink != NULL ) {
            RCt = ThePeer -> vt -> v1.unlink (
                                ThePeer,
                                ( const struct VPath * ) TheVPath
                                );
        }
        else {
            XFSMSG ( ( "ERROR: Peer method 'unlink()' is not implemented\n" ) );
            RCt = XFS_RC ( rcUnsupported );
        }

        VPathRelease ( TheVPath );
    }

    return - XFS_FUSE_rc_to_errno ( RCt );
}   /* XFS_FUSE_unlink() */

#endif /* USE_XFS_FUSE_UNLINK == 1 */

/*****************************************************/
#if USE_XFS_FUSE_RMDIR == 1

static
int
XFS_FUSE_rmdir ( const char * ThePath )
{
    rc_t RCt;
    XFSPeer * ThePeer;
    VPath * TheVPath;

    XFSMSG ( ( "RMDIR(Fuse): [%s]\n", ThePath ) );

    RCt = XFS_FUSE_common_check ( ThePath, & ThePeer, & TheVPath );
    if ( RCt == 0 ) {
        if ( ThePeer -> vt -> v1.rmdir != NULL ) {
            RCt = ThePeer -> vt -> v1.rmdir (
                                ThePeer,
                                ( const struct VPath * ) TheVPath
                                );
        }
        else {
            XFSMSG ( ( "ERROR: Peer method 'rmdir()' is not implemented\n" ) );

            RCt = XFS_RC ( rcUnsupported );
        }

        VPathRelease ( TheVPath );
    }

    return - XFS_FUSE_rc_to_errno ( RCt );
}   /* XFS_FUSE_rmdir() */

#endif /* USE_XFS_FUSE_RMDIR == 1 */

/*****************************************************/
#if USE_XFS_FUSE_SYMLINK == 1

static
int
XFS_FUSE_symlink ( const char * OldPath, const char * NewPath )
{
    OUTMSG ( ( "SYMLINK(!): OldPath %s\n", OldPath ) );

    return -EPERM;
}   /* XFS_FUSE_symlink() */

#endif /* USE_XFS_FUSE_SYMLINK == 1 */

/*****************************************************/
#if USE_XFS_FUSE_RENAME == 1

static
int
XFS_FUSE_rename ( const char * OldPath, const char * NewPath )
{
    rc_t RCt;
    XFSPeer * ThePeer;
    VPath * OldVPath, * NewVPath;
    VFSManager * Manager;

    XFSMSG ( ( "RENAME(Fuse): from [%s] to [%s]\n", OldPath, NewPath ) );

    RCt = XFS_FUSE_common_check ( OldPath, & ThePeer, & OldVPath );
    if ( RCt == 0 ) {
        Manager = ( VFSManager * ) ThePeer -> Data;
        if ( Manager == NULL ) {
            RCt = XFS_RC ( rcNull );
        }
        else {
            if ( ThePeer -> vt -> v1.rename != NULL ) {
                RCt = VFSManagerMakePath ( Manager, & NewVPath, "%s", NewPath );
                if ( RCt == 0 ) {
                    RCt = ThePeer -> vt -> v1.rename (
                                    ThePeer,
                                    ( const struct VPath * ) OldVPath,
                                    ( const struct VPath * ) NewVPath
                                    );

                    VPathRelease ( NewVPath );
                }
                VPathRelease ( OldVPath );
            }
            else {
                XFSMSG ( ( "ERROR: Peer method 'rename()' is not implemented\n" ) );
                RCt = XFS_RC ( rcUnsupported );
            }
        }
    }

        /* TODO!!! */
    return - XFS_FUSE_rc_to_errno ( RCt );
}   /* XFS_FUSE_rename() */

#endif /* USE_XFS_FUSE_RENAME == 1 */

/*****************************************************/
#if USE_XFS_FUSE_LINK == 1

static
int
XFS_FUSE_link ( const char * OldPath, const char * NewPath )
{
    OUTMSG ( ( "LINK(!): OldPath %s\n", OldPath ) );

    return -EPERM;
}   /* XFS_FUSE_link() */

#endif /* USE_XFS_FUSE_LINK == 1 */

/*****************************************************/
#if USE_XFS_FUSE_CHMOD == 1

static
int
XFS_FUSE_chmod ( const char * ThePath, mode_t TheMode )
{
    char JJJ [ 2000 ];
    sprintf ( JJJ, "/home/iskhakov/HLAM%s", ThePath );

    OUTMSG ( ( "CHMOD(): ThePath %s (%s)[mode = %d]\n", ThePath, JJJ, TheMode ) );

    errno = 0;

    return chmod ( JJJ, TheMode ) == 0 ? 0 : -errno;
}   /* XFS_FUSE_chmod() */

#endif /* USE_XFS_FUSE_CHMOD == 1 */

/*****************************************************/
#if USE_XFS_FUSE_CHOWN == 1

static
int
XFS_FUSE_chown ( const char * ThePath, uid_t TheUid, gid_t TheDid )
{
    OUTMSG ( ( "CHOWN(!): ThePath %s\n", ThePath ) );

    return -EPERM;
}   /* XFS_FUSE_chown() */

#endif /* USE_XFS_FUSE_CHOWN == 1 */

/*****************************************************/
#if USE_XFS_FUSE_TRUNCATE == 1

static
int
XFS_FUSE_truncate ( const char * ThePath, off_t TheSize )
{
    rc_t RCt;
    XFSPeer * ThePeer;
    VPath * TheVPath;

    XFSMSG ( ( "TRUNCATE(Fuse): [%s][SZ=%d] \n", ThePath, TheSize ) );

    RCt = XFS_FUSE_common_check ( ThePath, & ThePeer, &TheVPath );
    if ( RCt == 0 ) {
        if ( ThePeer -> vt -> v1.truncate != NULL ) {
            RCt = ThePeer -> vt -> v1.truncate (
                                            ThePeer,
                                            TheVPath,
                                            TheSize
                                            );
        }
        else {
            XFSMSG ( ( "ERROR: Peer method 'truncate()' is not implemented\n" ) );

            RCt = XFS_RC ( rcUnsupported );
        }

        VPathRelease ( TheVPath );
    }

    return - XFS_FUSE_rc_to_errno ( RCt );
}   /* XFS_FUSE_truncate() */

#endif /* USE_XFS_FUSE_TRUNCATE == 1 */

/*****************************************************/
#if USE_XFS_FUSE_UTIME == 1

static
int
XFS_FUSE_utime ( const char * ThePath, struct utimbuf * TheBuf )
{
    rc_t RCt;
    XFSPeer * ThePeer;
    VPath * TheVPath;

    XFSMSG ( ( "UTIME(Fuse): [%s][AT=%d][MT=%d] \n", ThePath, TheBuf -> actime, TheBuf -> modtime ) );

    RCt = XFS_FUSE_common_check ( ThePath, & ThePeer, &TheVPath );
    if ( RCt == 0 ) {
        if ( ThePeer -> vt -> v1.utime != NULL ) {
            RCt = ThePeer -> vt -> v1.utime (
                                        ThePeer,
                                        TheVPath,
                                        TheBuf -> modtime
                                        );
        }
        else {
            XFSMSG ( ( "ERROR: Peer method 'utime()' is not implemented\n" ) );
            RCt = XFS_RC ( rcUnsupported );
        }

        VPathRelease ( TheVPath );
    }


    return - XFS_FUSE_rc_to_errno ( RCt );
}   /* XFS_FUSE_utime() */

#endif /* USE_XFS_FUSE_UTIME == 1 */

/*****************************************************/
#if USE_XFS_FUSE_OPEN == 1

static
int
XFS_FUSE_open ( const char * ThePath, struct fuse_file_info * TheInfo )
{
    rc_t RCt;
    XFSPeer * ThePeer;
    VPath * TheVPath;
    struct KFile * TheFile;

    XFSMSG ( ( "OPEN(Fuse): [%s][FI=0x%p][flags=%d] \n", ThePath, TheInfo, TheInfo -> flags ) );

    RCt = XFS_FUSE_common_check ( ThePath, & ThePeer, &TheVPath );
    if ( RCt == 0 ) {
        if ( ThePeer -> vt -> v1.open != NULL ) {
            RCt = ThePeer -> vt -> v1.open (
                                        ThePeer,
                                        TheVPath,
                                        & TheFile,
                                        TheInfo -> flags
                                        );
            if ( RCt == 0 ) {
                    /*  Here we should pass KFile to FileInfo
                     */

                TheInfo -> fh = ( uint64_t ) TheFile;
XFSMSG ( ( "OPEN(Fuse cont): [%s][FI=0x%p][FH=%p] \n", ThePath, TheInfo, TheFile ) );
            }
        }
        else {
            XFSMSG ( ( "ERROR: Peer method 'open()' is not implemented\n" ) );
            RCt = XFS_RC ( rcUnsupported );
        }

        VPathRelease ( TheVPath );
    }

    return - XFS_FUSE_rc_to_errno (RCt );
}   /* XFS_FUSE_open() */

#endif /* USE_XFS_FUSE_OPEN == 1 */

/*****************************************************/
#if USE_XFS_FUSE_READ == 1

static
int
XFS_FUSE_read (
            const char * ThePath,
            char * TheBuf,
            size_t TheSizeRead,
            off_t TheOffsetRead,
            struct fuse_file_info * TheFileInfo
)
{
    rc_t RCt;
    XFSPeer * ThePeer;
    VPath * TheVPath;
    KFile * TheFile;
    size_t NumBytesReaded;

    TheFile = ( KFile * ) TheFileInfo -> fh;

    XFSMSG ( ( "READ(Fuse): [%s][FI=0x%p][FH=0x%p][OF=%d SZ=%d]\n", ThePath, TheFileInfo, TheFile, TheOffsetRead, TheSizeRead ) );

    if ( TheFile != 0 ) {
        RCt = XFS_FUSE_common_check ( ThePath, & ThePeer, &TheVPath );
        if ( RCt == 0 ) {
            if ( ThePeer -> vt -> v1.read != NULL ) {
                RCt = ThePeer -> vt -> v1.read (
                                            ThePeer,
                                            TheVPath,
                                            TheFile,
                                            TheOffsetRead,
                                            TheBuf,
                                            TheSizeRead,
                                            & NumBytesReaded
                                            );
                XFSMSG ( ( "READ(Fuse, cont): [%s][FI=0x%p][FH=0x%p][BR=%d]\n", ThePath, TheFileInfo, TheFile, NumBytesReaded ) );
            }
            else {
                XFSMSG ( ( "ERROR: Peer methon 'read()' is not implemented" ) );
                RCt = XFS_RC ( rcUnsupported );
            }

            VPathRelease ( TheVPath );
        }
    }
    else {
            /* TODO ??? */
        RCt = XFS_RC ( rcUnsupported );
    }

    return RCt == 0
                ? NumBytesReaded
                : ( - XFS_FUSE_rc_to_errno ( RCt ) )
                ;
}   /* XFS_FUSE_read() */

#endif /* USE_XFS_FUSE_READ == 1 */

/*****************************************************/
#if USE_XFS_FUSE_WRITE == 1

static
int
XFS_FUSE_write (
            const char * ThePath,
            const char * TheBuf,
            size_t TheSizeWrite,
            off_t TheOffsetWrite,
            struct fuse_file_info * TheFileInfo
)
{
    rc_t RCt;
    XFSPeer * ThePeer;
    VPath * TheVPath;
    KFile * TheFile;
    size_t NumBytesWritten;

    TheFile = ( KFile * ) TheFileInfo -> fh;

    XFSMSG ( ( "WRITE(Fuse): [%s][FI=0x%p][FH=0x%p][OF=%d SZ=%d]\n", ThePath, TheFileInfo, TheFile, TheOffsetWrite, TheSizeWrite ) );

    if ( TheFile != 0 ) {
        RCt = XFS_FUSE_common_check ( ThePath, & ThePeer, &TheVPath );
        if ( RCt == 0 ) {
            if ( ThePeer -> vt -> v1.write != NULL ) {
                RCt = ThePeer -> vt -> v1.write (
                                            ThePeer,
                                            TheVPath,
                                            TheFile,
                                            TheOffsetWrite,
                                            ( void * ) TheBuf,
                                            TheSizeWrite,
                                            & NumBytesWritten
                                            );
                XFSMSG ( ( "WRITE(Fuse, cont): [%s][FI=0x%p][FH=0x%p][BR=%d]\n", ThePath, TheFileInfo, TheFile, NumBytesWritten ) );
            }
            else {
                XFSMSG ( ( "ERROR: Peer methon 'write()' is not implemented" ) );
                RCt = XFS_RC ( rcUnsupported );
            }

            VPathRelease ( TheVPath );
        }
    }
    else {
            /* TODO ??? */
        RCt = XFS_RC ( rcUnsupported );
    }

    return RCt == 0
                ? NumBytesWritten
                : ( - XFS_FUSE_rc_to_errno ( RCt ) )
                ;
}   /* XFS_FUSE_write() */

#endif /* USE_XFS_FUSE_WRITE == 1 */

/*****************************************************/
#if USE_XFS_FUSE_STATFS == 1

static
int
XFS_FUSE_statfs (
            const char * ThePath,
            struct statvfs * TheFSStat
)
{
    OUTMSG ( ( "STATFS(!): ThePath %s\n", ThePath ) );

    return -EPERM;
}   /* XFS_FUSE_statfs() */

#endif /* USE_XFS_FUSE_STATFS == 1 */

/*****************************************************/
#if USE_XFS_FUSE_FLUSH == 1

static
int
XFS_FUSE_flush (
            const char * ThePath,
            struct fuse_file_info * TheFileInfo
)
{
    char JJJ [ 2000 ];
    sprintf ( JJJ, "/home/iskhakov/HLAM%s", ThePath );

    OUTMSG ( ( "FLUSH(DUMMY): ThePath %s (%s)[FI=0x%p] \n", ThePath, JJJ, TheFileInfo ) );

    return 0;
}   /* XFS_FUSE_flush() */

#endif /* USE_XFS_FUSE_FLUSH == 1 */

/*****************************************************/
#if USE_XFS_FUSE_RELEASE == 1

static
int
XFS_FUSE_release (
            const char * ThePath,
            struct fuse_file_info * TheFileInfo
)
{
    rc_t RCt;
    XFSPeer * ThePeer;
    VPath * TheVPath;
    struct KFile * TheFile;

    TheFile = ( struct KFile * ) TheFileInfo -> fh;

    XFSMSG ( ( "RELEASE(Fuse): [%s][FI=0x%p][FH=0x%p] \n", ThePath, TheFileInfo, TheFile ) );

    if ( TheFile != NULL ) {
        RCt = XFS_FUSE_common_check ( ThePath, & ThePeer, & TheVPath );
        if ( RCt == 0 ) {
            if ( ThePeer -> vt -> v1.release != NULL ) {
                RCt = ThePeer -> vt -> v1.release (
                                            ThePeer,
                                            TheVPath,
                                            TheFile
                                            );
                TheFileInfo -> fh = -1;
            }
            else {
                XFSMSG ( ( "ERROR: Peer method 'release()' is not implemented\n" ) );
                RCt = XFS_RC ( rcUnsupported );
            }

            VPathRelease ( TheVPath );
        }
    }
    else {
        RCt = XFS_RC ( rcNull );
    }

    return - XFS_FUSE_rc_to_errno ( RCt );
}   /* XFS_FUSE_release() */

#endif /* USE_XFS_FUSE_RELEASE == 1 */

/*****************************************************/
#if USE_XFS_FUSE_FSYNC == 1

static
int
XFS_FUSE_fsync (
            const char * ThePath,
            int DataSync,
            struct fuse_file_info * TheFileInfo
)
{
    int Result;
    int FileHandle;
    char JJJ [ 2000 ];
    sprintf ( JJJ, "/home/iskhakov/HLAM%s", ThePath );

    FileHandle = ( int ) TheFileInfo -> fh;

    OUTMSG ( ( "FSYNC: ThePath %s (%s)[FI=0x%p][FH=%d][DT=%d]\n", ThePath, JJJ, TheFileInfo, FileHandle, DataSync ) );

    errno = 0;

    Result = DataSync
                ? fsync ( FileHandle )
                : fdatasync ( FileHandle )
                ;

    return - errno;
}   /* XFS_FUSE_fsync() */

#endif /* USE_XFS_FUSE_FSYNC == 1 */

/*****************************************************/
#if USE_XFS_FUSE_SETXATTR == 1

static
int
XFS_FUSE_setxattr (
            const char * ThePath,
            const char * TheName,
            const char * TheValue,
            size_t TheValueSize,
            int TheFlags
)
{
    OUTMSG ( ( "SETXATTR(!): ThePath %s\n", ThePath ) );

    return -EPERM;
}   /* XFS_FUSE_setxattr() */

#endif /* USE_XFS_FUSE_SETXATTR == 1 */

/*****************************************************/
#if USE_XFS_FUSE_GETXATTR == 1

static
int
XFS_FUSE_getxattr (
            const char * ThePath,
            const char * TheName,
            char * TheValue,
            size_t TheValueSize
)
{
    OUTMSG ( ( "GETXATTR(!): ThePath %s\n", ThePath ) );

    return -EPERM;
}   /* XFS_FUSE_getxattr() */

#endif /* USE_XFS_FUSE_GETXATTR == 1 */

/*****************************************************/
#if USE_XFS_FUSE_LISTXATTR == 1

static
int
XFS_FUSE_listxattr (
            const char * ThePath,
            char * TheList,
            size_t TheListSize
)
{
    return -EPERM;
}   /* XFS_FUSE_listxattr() */

#endif /* USE_XFS_FUSE_LISTXATTR == 1 */

/*****************************************************/
#if USE_XFS_FUSE_REMOVEXATTR == 1

static
int
XFS_FUSE_removexattr ( const char * ThePath, const char * TheName)
{
    OUTMSG ( ( "REMOVEXATTR(!): ThePath %s\n", ThePath ) );

    return -EPERM;
}   /* XFS_FUSE_removexattr() */

#endif /* USE_XFS_FUSE_REMOVEXATTR == 1 */

/*****************************************************/
#if USE_XFS_FUSE_OPENDIR == 1

static
int
XFS_FUSE_opendir (
            const char * ThePath,
            struct fuse_file_info * TheFileInfo
)
{
    rc_t RCt;
    XFSPeer * ThePeer;
    VPath * TheVPath;
    uint32_t Flags;
    void * Handle;

    Flags = TheFileInfo -> flags;

    XFSMSG ( ( "OPENDIR(Fuse): [%s][FI=0x%p][flags=%d]\n", ThePath, TheFileInfo, Flags ) );

    RCt = XFS_FUSE_common_check ( ThePath, & ThePeer, & TheVPath );
    if ( RCt == 0 ) {
        if ( ThePeer -> vt -> v1.opendir != NULL ) {
            RCt = ThePeer -> vt -> v1.opendir (
                                            ThePeer,
                                            TheVPath,
                                            & Handle
                                            );
            if ( RCt == 0 ) {
                TheFileInfo -> fh = ( uint64_t ) Handle;

XFSMSG ( ( "OPENDIR(Fuse, cont): [%s][FI=0x%p][FH=0x%p]\n", ThePath, TheFileInfo, Handle ) );
            }
        }
        else {
            XFSMSG ( ( "ERROR: Peer method 'opendir()' is not implemented\n" ) );
            RCt = XFS_RC ( rcUnsupported );
        }

        VPathRelease ( TheVPath );
    }

    return - XFS_FUSE_rc_to_errno ( RCt );
}   /* XFS_FUSE_opendir() */

#endif /* USE_XFS_FUSE_OPENDIR == 1 */

/*****************************************************/
#if USE_XFS_FUSE_READDIR == 1

/*****************************************************************
 * JUST FOR ANY CASE, THAT FUNCTION IS USED IN 'readdir()' TO 
 * FILL BUFFER WITH DIRECTORY INFORMATION
 *
 *      typedef int (*fuse_fill_dir_t) (
 *                  void *buf,
 *                  const char *name,
 *                  const struct stat *stbuf,
 *                  off_t off
 *                  );
 *****************************************************************/

static
int
XFS_FUSE_readdir (
            const char * ThePath,
            void * TheBuffer,
            fuse_fill_dir_t TheFiller,
            off_t TheOffset,
            struct fuse_file_info * TheFileInfo
)
{
    rc_t RCt;
    XFSPeer * ThePeer;
    VPath * TheVPath;
    void * Handle;
    KNamelist * NameList;
    uint32_t ListQty, llp;
    const char *ListEntry;

    Handle = ( void * ) TheFileInfo -> fh;

    XFSMSG ( ( "READDIR(Fuse): [%s][FI=0x%p][FH=0x%p] \n", ThePath, TheFileInfo, Handle ) );

    RCt = XFS_FUSE_common_check ( ThePath, & ThePeer, & TheVPath );
    if ( RCt == 0 ) {
        if ( ThePeer -> vt -> v1.readdir != NULL ) {
            RCt = ThePeer -> vt -> v1.readdir (
                                        ThePeer,
                                        TheVPath,
                                        Handle,
                                        & NameList
                                        );
            if ( RCt == 0 ) { 
                if ( NameList != NULL ) { 
                    RCt = KNamelistCount ( NameList, & ListQty );
                    if ( RCt == 0 ) {
XFSMSG ( ( "READDIR(Fuse): [%s][files found: %d] \n", ThePath, ListQty ) );
                        for( llp = 0; llp < ListQty; llp ++ ) {
                            RCt = KNamelistGet (
                                                NameList,
                                                llp,
                                                & ListEntry
                                                );
                            if ( RCt != 0 ) {
                                break;
                            }
                            TheFiller ( TheBuffer, ListEntry, NULL, 0 );
XFSMSG ( ( "   [%d] %s [%s/%s] \n", llp, ListEntry, ThePath, ListEntry ) );
                        }
                    }
                }
                else {
                    RCt = XFS_RC ( rcNull );
                }
                /* TODO - reading list and filling buffer */

                KNamelistRelease ( NameList );
            }
        }
        else {
            XFSMSG ( ( "ERROR: Peer method 'readdir()' is not implemented\n" ) );
            RCt = XFS_RC ( rcUnsupported );
        }

        VPathRelease ( TheVPath );
    }

    return - XFS_FUSE_rc_to_errno ( RCt );
}   /* XFS_FUSE_readdir() */

#endif /* USE_XFS_FUSE_READDIR == 1 */

/*****************************************************/
#if USE_XFS_FUSE_RELEASEDIR == 1

static
int
XFS_FUSE_releasedir (
            const char * ThePath,
            struct fuse_file_info * TheFileInfo
)
{
    rc_t RCt;
    XFSPeer * ThePeer;
    VPath * TheVPath;
    void * Handle;

    Handle = ( void * ) TheFileInfo -> fh;

    XFSMSG ( ( "RELEASEDIR(Fuse): [%s][FI=0x%p][FH=0x%p] \n", ThePath, TheFileInfo, Handle ) );

    RCt = XFS_FUSE_common_check ( ThePath, & ThePeer, & TheVPath );
    if ( RCt == 0 ) {
        if ( ThePeer -> vt -> v1.releasedir != NULL ) {
            RCt = ThePeer -> vt -> v1.releasedir (
                                            ThePeer,
                                            TheVPath,
                                            Handle
                                            );
            TheFileInfo -> fh = -1;
        }
        else {
            XFSMSG ( ( "ERROR: Peer method 'releasedir()' is not implemented\n" ) );
            RCt = XFS_RC ( rcUnsupported );
        }

        VPathRelease ( TheVPath );
    }

    return - XFS_FUSE_rc_to_errno ( RCt );
}   /* XFS_FUSE_releasedir() */

#endif /* USE_XFS_FUSE_RELEASEDIR == 1 */

/*****************************************************/
#if USE_XFS_FUSE_FSYNCDIR == 1

static
int
XFS_FUSE_fsyncdir (
            const char * ThePath,
            int DataSync,
            struct fuse_file_info * TheFileInfo
)
{
    OUTMSG ( ( "FSYNCDIR(!): ThePath %s\n", ThePath ) );

    return -EPERM;
}   /* XFS_FUSE_fsyncdir() */

#endif /* USE_XFS_FUSE_FSYNCDIR == 1 */

/*****************************************************/
#if USE_XFS_FUSE_INIT == 1

static
void *
XFS_FUSE_init ( struct fuse_conn_info * TheConnInfo )
{
    rc_t RCt;
    XFSPeer * ThePeer;

    RCt = GetPeerFromFuseHaHa ( & ThePeer );

    OUTMSG ( ( "INIT(): TheConnInfo 0x%p Peer %p\n", TheConnInfo, ThePeer ) );

    return RCt != 0 ? NULL : ThePeer;
}   /* XFS_FUSE_init() */

#endif /* USE_XFS_FUSE_INIT == 1 */

/*****************************************************/
#if USE_XFS_FUSE_DESTROY == 1

static
void
XFS_FUSE_destroy ( void * OnoSamoe )
{
    OUTMSG ( ( "DESTROY(Dummy): OnoSamoe 0x%p\n", OnoSamoe ) );

}   /* XFS_FUSE_destroy() */

#endif /* USE_XFS_FUSE_DESTROY == 1 */

/*****************************************************/
#if USE_XFS_FUSE_ACCESS == 1

static
int
XFS_FUSE_access ( const char * ThePath, int NonDefPermissions )
{
    rc_t RCt;
    XFSPeer * ThePeer;
    VPath * TheVPath;

    XFSMSG ( ( "ACCESS(Fuse): [%s][mode=%d]\n", ThePath, NonDefPermissions ) );

    RCt = XFS_FUSE_common_check ( ThePath, & ThePeer, &TheVPath );
    if ( RCt == 0 ) {
        if ( ThePeer -> vt -> v1.access != NULL ) {
            RCt = ThePeer -> vt -> v1.access (
                                        ThePeer,
                                        TheVPath,
                                        NonDefPermissions
                                        );
        }
        else {
            XFSMSG ( ( "ERROR: Peer method 'access()' is not implemented\n" ) );
            RCt = XFS_RC ( rcUnsupported );
        }

        VPathRelease ( TheVPath );
    }


    return - XFS_FUSE_rc_to_errno ( RCt );
}   /* XFS_FUSE_access() */

#endif /* USE_XFS_FUSE_ACCESS == 1 */

/*****************************************************/
#if USE_XFS_FUSE_CREATE == 1

static
int
XFS_FUSE_create (
            const char * ThePath,
            mode_t TheMode,
            struct fuse_file_info * TheFileInfo
)
{
        /*  TODO : I still does not know if it is right to use 
         *  KDirectoryCreateFile, cuz it required 'update' parameter
         *  and that parameter is usually passed as flags to 'open'
         *  I noticed, that 'flags' field from FileInfo struct is
         *  set 'right' always( ?), but I can not find in documentation
         *  if it is correct. BTW, using these flags
         */
    rc_t RCt;
    XFSPeer * ThePeer;
    VPath * TheVPath;
    struct KFile * TheFile;
    uint32_t Flags;

    Flags = TheFileInfo -> flags;

XFSMSG ( ( "CREATE(Fuse): [%s][FI=0x%p][flags=%d][mode=%d]\n", ThePath, TheFileInfo, Flags, TheMode ) );

    RCt = XFS_FUSE_common_check ( ThePath, & ThePeer, & TheVPath );
    if ( RCt == 0 ) {
        if ( ThePeer -> vt -> v1.create != NULL ) {
            RCt = ThePeer -> vt -> v1.create (
                                            ThePeer,
                                            TheVPath,
                                            & TheFile,
                                            Flags,
                                            TheMode
                                            );
            if ( RCt == 0 ) {
                TheFileInfo -> fh = ( uint64_t ) TheFile;

XFSMSG ( ( "CREATE(Fuse, cont): [%s][FI=0x%p][FH=0x%p]\n", ThePath, TheFileInfo, TheFile ) );
            }
        }
        else {
            XFSMSG ( ( "ERROR: Peer method 'create()' is not implemented\n" ) );
            RCt = XFS_RC ( rcUnsupported );
        }

        VPathRelease ( TheVPath );
    }

    return - XFS_FUSE_rc_to_errno ( RCt );
}   /* XFS_FUSE_create() */

#endif /* USE_XFS_FUSE_CREATE == 1 */

/*****************************************************/
#if USE_XFS_FUSE_FTRUNCATE == 1

static
int
XFS_FUSE_ftruncate (
            const char * ThePath,
            off_t TheSize,
            struct fuse_file_info * TheFileInfo
)
{
    rc_t RCt;
    XFSPeer * ThePeer;
    VPath * TheVPath;
    KFile * TheFile;

    TheFile = ( KFile * ) TheFileInfo -> fh;

    XFSMSG ( ( "TRUNCATE(Fuse): [%s][SZ=%d][FH=0x%p] \n", ThePath, TheSize, TheFile ) );

    if ( TheFile != NULL ) {
        RCt = XFS_FUSE_common_check ( ThePath, & ThePeer, &TheVPath );
        if ( RCt == 0 ) {
            if ( ThePeer -> vt -> v1.ftruncate != NULL ) {
                RCt = ThePeer -> vt -> v1.ftruncate (
                                                ThePeer,
                                                TheVPath,
                                                TheFile,
                                                TheSize
                                                );
            }
            else {
                XFSMSG ( ( "ERROR: Peer method 'ftruncate()' is not implemented\n" ) );

                RCt = XFS_RC ( rcUnsupported );
            }

            VPathRelease ( TheVPath );
        }
    }
    else {
        RCt = XFS_RC ( rcNull );
    }

    return - XFS_FUSE_rc_to_errno ( RCt );
}   /* XFS_FUSE_ftruncate() */

#endif /* USE_XFS_FUSE_FTRUNCATE == 1 */

/*****************************************************/
#if USE_XFS_FUSE_FGETATTR == 1

static
int
XFS_FUSE_fgetattr (
            const char * ThePath,
            struct stat * TheStat,
            struct fuse_file_info * TheFileInfo
)
{
    rc_t RCt;
    XFSPeer * ThePeer;
    VPath * TheVPath;
    KFile * TheFile;
    XFSPeerObjectInfo ThePeerObjectInfo;

    TheFile = ( KFile * ) TheFileInfo -> fh;

    XFSMSG ( ( "FGETATTR(Fuse): [%s][FH=0x%p]\n", ThePath, TheFile ) );

    RCt = XFS_FUSE_common_check ( ThePath, & ThePeer, & TheVPath );

    if ( RCt == 0 ) {
        if ( ThePeer -> vt -> v1.fgetattr != NULL ) {
            RCt = ThePeer -> vt -> v1.fgetattr (
                                            ThePeer,
                                            TheVPath,
                                            TheFile,
                                            & ThePeerObjectInfo
                                            );
            if ( RCt == 0 ) {
                StatStructureToXFSPeerObjectInfo (
                                            & ThePeerObjectInfo,
                                            TheStat
                                            );
            }
            else {
                memset ( TheStat, 0, sizeof ( struct stat ) );
            }
        }
        else {
            XFSMSG ( ( "ERROR: Peer method 'fgetattr()' is not implemented\n" ) );
            RCt = XFS_RC ( rcUnsupported );
        }

        VPathRelease ( TheVPath );
    }

    return - XFS_FUSE_rc_to_errno ( RCt );
    /* return RCt == 0 ? 0 : - ENOENT; */
}   /* XFS_FUSE_fgetattr() */

#endif /* USE_XFS_FUSE_FGETATTR == 1 */

/*****************************************************/
#if USE_XFS_FUSE_LOCK == 1

static
int
XFS_FUSE_lock (
            const char * ThePath,
            struct fuse_file_info * TheFileInfo,
            int TheCmd,
		    struct flock * TheFLock
)
{
    OUTMSG ( ( "LOCK(!): ThePath %s\n", ThePath ) );

    return -EPERM;
}   /* XFS_FUSE_lock() */

#endif /* USE_XFS_FUSE_LOCK == 1 */

/*****************************************************/
#if USE_XFS_FUSE_UTIMENS == 1

static
int
XFS_FUSE_utimens (
            const char * ThePath,
            const struct timespec TheTimespec[2]
)
{
    OUTMSG ( ( "UTIMENS(!): ThePath %s\n", ThePath ) );

    return -EPERM;
}   /* XFS_FUSE_utimens() */

#endif /* USE_XFS_FUSE_UTIMENS == 1 */

/*****************************************************/
#if USE_XFS_FUSE_BMAP == 1

static
int
XFS_FUSE_bmap (
            const char * ThePath,
            size_t TheBlockSize,
            uint64_t *TheIdx
)
{
    OUTMSG ( ( "BMAP(!): ThePath %s\n", ThePath ) );

    return -EPERM;
}   /* XFS_FUSE_bmap() */

#endif /* USE_XFS_FUSE_BMAP == 1 */

/*****************************************************/
#if USE_XFS_FUSE_IOCTL == 1

static
int
XFS_FUSE_ioctl (
            const char * ThePath,
            int TheCmd,
            void *TheArg,
            struct fuse_file_info * TheFileInfo,
            unsigned int TheFlags,
            void * TheData
)
{
    OUTMSG ( ( "IOCTL(!): ThePath %s\n", ThePath ) );

    return -EPERM;
}   /* XFS_FUSE_ioctl() */

#endif /* USE_XFS_FUSE_IOCTL == 1 */

/*****************************************************/
#if USE_XFS_FUSE_POLL == 1

static
int
XFS_FUSE_poll (
            const char * ThePath,
            struct fuse_file_info * TheFileInfo,
            struct fuse_pollhandle * ThePollHandle,
            unsigned * TheReventsp
)
{
    OUTMSG ( ( "POLL(!): ThePath %s\n", ThePath ) );

    return -EPERM;
}   /* XFS_FUSE_poll() */

#endif /* USE_XFS_FUSE_POLL == 1 */

/*  Very important method, but very stupid.
 */
rc_t
XFS_Private_InitOperations ( struct fuse_operations * Operations )
{
    if ( Operations == NULL ) {
        return XFS_RC ( rcNull );
    }

    memset ( Operations, 0, sizeof( struct fuse_operations ) );

#if USE_XFS_FUSE_GETATTR == 1
    Operations -> getattr = XFS_FUSE_getattr;
#endif /* USE_XFS_FUSE_GETATTR == 1 */

#if USE_XFS_FUSE_READLINK == 1
    Operations -> readlink = XFS_FUSE_readlink;
#endif /* USE_XFS_FUSE_READLINK == 1 */

#if USE_XFS_FUSE_GETDIR == 1
    Operations -> getdir = XFS_FUSE_getdir;
#endif /* USE_XFS_FUSE_GETDIR == 1 */

#if USE_XFS_FUSE_MKNOD == 1
    Operations -> mknod = XFS_FUSE_mknod;
#endif /* USE_XFS_FUSE_MKNOD == 1 */

#if USE_XFS_FUSE_MKDIR == 1
    Operations -> mkdir = XFS_FUSE_mkdir;
#endif /* USE_XFS_FUSE_MKDIR == 1 */

#if USE_XFS_FUSE_UNLINK == 1
    Operations -> unlink = XFS_FUSE_unlink;
#endif /* USE_XFS_FUSE_UNLINK == 1 */

#if USE_XFS_FUSE_RMDIR == 1
    Operations -> rmdir = XFS_FUSE_rmdir;
#endif /* USE_XFS_FUSE_RMDIR == 1 */

#if USE_XFS_FUSE_SYMLINK == 1
    Operations -> symlink = XFS_FUSE_symlink;
#endif /* USE_XFS_FUSE_SYMLINK == 1 */

#if USE_XFS_FUSE_RENAME == 1
    Operations -> rename = XFS_FUSE_rename;
#endif /* USE_XFS_FUSE_RENAME == 1 */

#if USE_XFS_FUSE_LINK == 1
    Operations -> link = XFS_FUSE_link;
#endif /* USE_XFS_FUSE_LINK == 1 */

#if USE_XFS_FUSE_CHMOD == 1
    Operations -> chmod = XFS_FUSE_chmod;
#endif /* USE_XFS_FUSE_CHMOD == 1 */

#if USE_XFS_FUSE_CHOWN == 1
    Operations -> chown = XFS_FUSE_chown;
#endif /* USE_XFS_FUSE_CHOWN == 1 */

#if USE_XFS_FUSE_TRUNCATE == 1
    Operations -> truncate = XFS_FUSE_truncate;
#endif /* USE_XFS_FUSE_TRUNCATE == 1 */

#if USE_XFS_FUSE_UTIME == 1
    Operations -> utime = XFS_FUSE_utime;
#endif /* USE_XFS_FUSE_UTIME == 1 */

#if USE_XFS_FUSE_OPEN == 1
    Operations -> open = XFS_FUSE_open;
#endif /* USE_XFS_FUSE_OPEN == 1 */

#if USE_XFS_FUSE_READ == 1
    Operations -> read = XFS_FUSE_read;
#endif /* USE_XFS_FUSE_READ == 1 */

#if USE_XFS_FUSE_WRITE == 1
    Operations -> write = XFS_FUSE_write;
#endif /* USE_XFS_FUSE_WRITE == 1 */

#if USE_XFS_FUSE_STATFS == 1
    Operations -> statfs = XFS_FUSE_statfs;
#endif /* USE_XFS_FUSE_STATFS == 1 */

#if USE_XFS_FUSE_FLUSH == 1
    Operations -> flush = XFS_FUSE_flush;
#endif /* USE_XFS_FUSE_FLUSH == 1 */

#if USE_XFS_FUSE_RELEASE == 1
    Operations -> release = XFS_FUSE_release;
#endif /* USE_XFS_FUSE_RELEASE == 1 */

#if USE_XFS_FUSE_FSYNC == 1
    Operations -> fsync = XFS_FUSE_fsync;
#endif /* USE_XFS_FUSE_FSYNC == 1 */

#if USE_XFS_FUSE_SETXATTR == 1
    Operations -> setxattr = XFS_FUSE_setxattr;
#endif /* USE_XFS_FUSE_SETXATTR == 1 */

#if USE_XFS_FUSE_GETXATTR == 1
    Operations -> getxattr = XFS_FUSE_getxattr;
#endif /* USE_XFS_FUSE_GETXATTR == 1 */

#if USE_XFS_FUSE_LISTXATTR == 1
    Operations -> listxattr = XFS_FUSE_listxattr;
#endif /* USE_XFS_FUSE_LISTXATTR == 1 */

#if USE_XFS_FUSE_REMOVEXATTR == 1
    Operations -> removexattr = XFS_FUSE_removexattr;
#endif /* USE_XFS_FUSE_REMOVEXATTR == 1 */

#if USE_XFS_FUSE_OPENDIR == 1
    Operations -> opendir = XFS_FUSE_opendir;
#endif /* USE_XFS_FUSE_OPENDIR == 1 */

#if USE_XFS_FUSE_READDIR == 1
    Operations -> readdir = XFS_FUSE_readdir;
#endif /* USE_XFS_FUSE_READDIR == 1 */

#if USE_XFS_FUSE_RELEASEDIR == 1
    Operations -> releasedir = XFS_FUSE_releasedir;
#endif /* USE_XFS_FUSE_RELESEDIR == 1 */

#if USE_XFS_FUSE_FSYNCDIR == 1
    Operations -> fsyncdir = XFS_FUSE_fsyncdir;
#endif /* USE_XFS_FUSE_FSYNCDIR == 1 */

#if USE_XFS_FUSE_INIT == 1
    Operations -> init = XFS_FUSE_init;
#endif /* USE_XFS_FUSE_INIT == 1 */

#if USE_XFS_FUSE_DESTROY == 1
    Operations -> destroy = XFS_FUSE_destroy;
#endif /* USE_XFS_FUSE_DESTROY == 1 */

#if USE_XFS_FUSE_ACCESS == 1
    Operations -> access = XFS_FUSE_access;
#endif /* USE_XFS_FUSE_ACCESS == 1 */

#if USE_XFS_FUSE_CREATE == 1
    Operations -> create = XFS_FUSE_create;
#endif /* USE_XFS_FUSE_CREATE == 1 */

#if USE_XFS_FUSE_FTRUNCATE == 1
    Operations -> ftruncate = XFS_FUSE_ftruncate;
#endif /* USE_XFS_FUSE_FTRUNCATE == 1 */

#if USE_XFS_FUSE_FGETATTR == 1
    Operations -> fgetattr = XFS_FUSE_fgetattr;
#endif /* USE_XFS_FUSE_FGETATTR == 1 */

#if USE_XFS_FUSE_LOCK == 1
    Operations -> lock = XFS_FUSE_lock;
#endif /* USE_XFS_FUSE_LOCK == 1 */

#if USE_XFS_FUSE_UTIMENS == 1
    Operations -> utimens = XFS_FUSE_utimens;
#endif /* USE_XFS_FUSE_UTIMENS == 1 */

#if USE_XFS_FUSE_BMAP == 1
    Operations -> bmap = XFS_FUSE_bmap;
#endif /* USE_XFS_FUSE_BMAP == 1 */

#if USE_XFS_FUSE_IOCTL == 1
    Operations -> ioctl = XFS_FUSE_ioctl;
#endif /* USE_XFS_FUSE_IOCTL == 1 */

#if USE_XFS_FUSE_POLL == 1
    Operations -> poll = XFS_FUSE_poll;
#endif /* USE_XFS_FUSE_POLL == 1 */

    return 0;
}   /* XFS_Private_InitOperations() */
