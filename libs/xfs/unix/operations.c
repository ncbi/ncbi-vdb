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
#include <klib/namelist.h>
#include <klib/refcount.h>
#include <klib/defs.h>
#include <vfs/path.h>
#include <vfs/manager.h>


#include <xfs/tree.h>
#include <xfs/node.h>
#include <xfs/handle.h>
#include <xfs/editors.h>
#include <xfs/perm.h>
#include <xfs/path.h>

#include "schwarzschraube.h"

#include <sysalloc.h>
#include <string.h> /* we are using memset() */
#include <stdio.h>  /* trash, need to be removed, after adding VFM */
#include <dirent.h> /* trash, need to be removed, after adding VFM */
#include <unistd.h> /* trash, need to be removed, after adding VFM */

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

#include "operations.h"


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
#define USE_XFS_FUSE_CHMOD        1     /* - */ /* ? */
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
_FUSE_tree_depot ( const struct XFSTreeDepot ** Depot )
{
    struct fuse_context * TheContext;

    if ( Depot == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Depot = NULL;

    TheContext = fuse_get_context();
    if ( TheContext == NULL ) {
        OUTMSG ( ( "ERROR: improper usage of 'fuse_get_context()'\n" ) );
        return XFS_RC ( rcNull );
    }

    * Depot = ( const struct XFSTreeDepot * ) TheContext -> private_data;
    if ( * Depot == NULL ) {
        return XFS_RC ( rcNull );
    }

    return 0;
}   /* _FUSE_tree_depot () */

static
rc_t
_FUSE_make_v_path (
                const char * ThePath,
                const struct VPath ** DasPath
)
{
    rc_t RCt;
    struct VPath * Pth;

    RCt = 0;
    Pth = NULL;

    if ( ThePath == NULL || DasPath == NULL ) {
        return XFS_RC ( rcNull );
    }
    * DasPath = NULL;

    RCt = VFSManagerMakePath ( XFS_VfsManager (), & Pth, ThePath );
    if ( RCt == 0 ) {
        * DasPath = Pth;
    }


    return RCt;
}   /* _FUSE_make_v_path () */

static
rc_t 
_FUSE_get_node (
                const struct VPath * Path,
                const struct XFSNode ** Node
)
{
    rc_t RCt;
    const struct XFSTreeDepot * Depot;
    const struct XFSNode * TheNode;

    RCt = 0;

    if ( Path == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Node = NULL;

    RCt = _FUSE_tree_depot ( & Depot );
    if ( RCt == 0 ) {
        if ( Depot == NULL ) {
            RCt = XFS_RC ( rcInvalid );
        }
        else {
            RCt = XFSTreeDepotFindNodeForPath (
                                            Depot,
                                            Path,
                                            & TheNode
                                            );
            if ( RCt == 0 ) {
                * Node = TheNode;
            }
        }
    }

    return RCt;
}   /* _FUSE_get_node () */

static
rc_t
_FUSE_get_path_and_node (
                    const char * ThePath,
                    const struct VPath ** DasPath,
                    const struct XFSNode ** Node,
                    XFSNType * NodeType
)
{
    rc_t RCt;
    const struct XFSNode * RNode;
    const struct VPath * RPath;
    XFSNType Type;
    const struct XFSAttrEditor * Editor;

    RCt = 0;
    RNode = NULL;
    RPath = NULL;
    Type = kxfsNotFound;
    Editor = NULL;

    if ( ThePath == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = _FUSE_make_v_path ( ThePath, & RPath );
    if ( RCt == 0 ) {

        RCt = _FUSE_get_node ( RPath, & RNode );
        if ( RCt == 0 ) {
            if ( NodeType != NULL ) {
                RCt = XFSNodeAttrEditor ( RNode, & Editor );
                if ( RCt == 0 ) {
                    RCt = XFSAttrEditorType ( Editor, & Type );

                    XFSEditorDispose ( & ( Editor -> Papahen ) );
                }
            }
        }
    }

    if ( RCt == 0 ) {
        if ( DasPath != NULL ) {
            * DasPath = RPath;
        }
        else {
            VPathRelease ( RPath );
        }

        if ( Node != NULL ) {
            * Node = RNode;
        }
        else {
            XFSNodeRelease ( RNode );
        }

        if ( NodeType != NULL ) {
            * NodeType = Type;
        }
    }
    else {
        if ( RPath != NULL ) {
            VPathRelease ( RPath );
        }

        if ( RNode != NULL ) {
            XFSNodeRelease ( RNode );
        }
    }

    return RCt;
}   /* _FUSE_get_path_and_node () */

static
rc_t
_FUSE_get_parent_node (
                    const char * Path,
                    const struct XFSNode ** Parent,
                    XFSNType * ParentType,      /* Could be NULL */
                    char ** ChildName           /* Could be NULL */
)
{
    rc_t RCt;
    const struct XFSTreeDepot * Depot;
    char BB [ XFS_SIZE_4096 ];
    const struct XFSPath * xPath;
    uint32_t xPathQ;
    const struct XFSNode * xNode;
    const struct XFSAttrEditor * xEditor;
    char * xName;

    RCt = 0;
    Depot = NULL;
    * BB = 0;
    xPath = NULL;
    xPathQ = 0;
    xNode = NULL;
    xEditor = NULL;
    xName = NULL;

    if ( Parent == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Parent = NULL;

    if ( Path == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( ParentType != NULL ) {
        * ParentType = kxfsNotFound;
    }

    if ( ChildName != NULL ) {
        * ChildName = NULL;
    }

        /* TreeDepot is a key */
    RCt = _FUSE_tree_depot ( & Depot );
    if ( RCt != 0 ) {
        return RCt;
    }

        /* Path to parent node is ... */
    RCt = XFSPathMake ( Path, & xPath );
    if ( RCt == 0 ) {
        xPathQ = XFSPathCount ( xPath );
        if ( xPathQ < 2 ) {
            RCt = XFS_RC ( rcInvalid );
        }
        else {
                /* So, there is a path to parent */
            RCt = XFSPathTo ( xPath, xPathQ - 1, BB, sizeof ( BB ) );
            if ( RCt == 0 ) {
                    /* Looking for node */
                RCt = XFSTreeDepotFindNode ( Depot, BB, & xNode );
                if ( RCt == 0 ) {
                    if ( ParentType != NULL ) {
                        RCt = XFSNodeAttrEditor ( xNode, & xEditor );
                        if ( RCt == 0 ) {
                            RCt = XFSAttrEditorType ( xEditor, ParentType );
                            XFSEditorDispose ( & ( xEditor -> Papahen ) );
                        }
                    }

                    if ( RCt == 0 ) {
                        if ( ChildName != NULL ) {
                            RCt = XFS_StrDup (
                                            XFSPathName ( xPath ),
                                            ( const char ** ) & xName
                                            );
                            if ( RCt == 0 ) {
                                * ChildName = xName;
                            }
                        }
                    }

                    if ( RCt == 0 ) {
                        * Parent = xNode;
                    }
                }
            }
        }

        XFSPathDispose ( xPath );
    }

    if ( RCt != 0 ) {
        * Parent = NULL;
        if ( ParentType != NULL ) {
            * ParentType = kxfsNotFound;
        }
        if ( ChildName != NULL ) {
            * ChildName = NULL;
        }
        if ( xNode != NULL ) {
            XFSNodeRelease ( xNode );
        }
        if ( xName != NULL ) {
            free ( xName );
        }
    }

    return RCt;
}   /* _FUSE_get_parent_node () */

static
rc_t
_FUSE_delete_file_dir ( const char * Path )
{
    rc_t RCt;
    const struct XFSNode * Parent;
    char * Child;
    const struct XFSDirEditor * Editor;

    RCt = 0;

    if ( Path == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = _FUSE_get_parent_node ( Path, & Parent, NULL, & Child );
    if ( RCt == 0 ) {
        RCt = XFSNodeDirEditor ( Parent, & Editor );
        if ( RCt == 0 ) {
            RCt = XFSDirEditorDelete ( Editor, Child );

            XFSEditorDispose ( & ( Editor -> Papahen ) );
        }

        XFSNodeRelease ( Parent );
        free ( Child );
    }

    return RCt;
}   /* _FUSE_delete_file_dir () */

/*****************************************************
 * Here are convertors
 *****************************************************/
 /*)
 |*|  Converts permission string to stat mode_t
 (*/
static
rc_t
_FUSE_char_to_perm ( const char * Perm, XFSNType Type, mode_t * Mode )
{
    rc_t RCt;
    uint32_t Temp;

    RCt = 0;
    Temp = 0;

    if ( Mode == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Mode = 0;

    if ( Perm == NULL ) {
        Temp = Type == kxfsFile ? 0644 : 0744;
    }
    else {
        RCt = XFSPermToAccess ( Perm, & Temp );
    }

    if ( RCt == 0 ) {
        switch ( Type ) {
            case kxfsFile : Temp |= S_IFREG; break;
            case kxfsDir:   Temp |= S_IFDIR; break;
            case kxfsLink:  Temp |= S_IFLNK; break;
            default:        Temp = 0;        break;
        }

        if ( Temp == 0 ) {
            RCt = XFS_RC ( rcInvalid );
        }
        else {
            * Mode = Temp;
        }
    }

    return RCt;
}   /* _FUSE_char_to_perm () */

static
rc_t
_FUSE_stat_for_node ( const struct XFSNode * Node, struct stat * Stat )
{
    rc_t RCt;
    XFSNType Type;
    KTime_t Time;
    uint64_t Size;
    const struct XFSAttrEditor * Editor;
    const char * Perm;

    RCt = 0;
    Editor = NULL;
    Perm = NULL;

    if ( Node == NULL || Stat == NULL ) {
        return XFS_RC ( rcNull ) ;
    }

    RCt = XFSNodeAttrEditor ( Node, & Editor );
    if ( RCt != 0 || Editor == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    /*) Here we are doing security ... later 
     /  TODO Stat -> st_mode = SomeAccess
    (*/

    RCt = XFSAttrEditorType ( Editor, & Type );
    if ( RCt == 0 ) {
        Stat -> st_mode = 0;

        RCt = XFSAttrEditorPermissions ( Editor, & Perm );
        if ( RCt == 0 ) {
            RCt = _FUSE_char_to_perm ( Perm, Type, & ( Stat -> st_mode ) );

            if ( RCt == 0 && Stat -> st_mode != 0 ) {
                if ( XFSAttrEditorDate ( Editor, & Time ) == 0 ) {
                    if ( XFSAttrEditorSize ( Editor, & Size ) == 0 ) {
                        Stat -> st_uid = getuid();
                        Stat -> st_gid = getgid();
                        Stat -> st_blksize = XFS_SIZE_4096;
                        Stat -> st_size = Size;
                        Stat -> st_atime = Time;
                        Stat -> st_mtime = Time;
                        Stat -> st_ctime = Time;
                    }
                }
            }
        }
    }

    XFSEditorDispose ( & ( Editor -> Papahen ) );

    return RCt;
}   /* _FUSE_stat_for_node () */

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
 * Operations
 *****************************************************/

/*****************************************************/
#if USE_XFS_FUSE_GETATTR == 1

static
int
XFS_FUSE_getattr ( const char * ThePath, struct stat * TheStat )
{
    rc_t RCt;
    const struct XFSNode * Node;
    XFSNType Type;

    RCt = 0;
    Node = NULL;
    Type = kxfsNotFound;

    XFSMSG ( ( "GETATTR(Fuse): [%s]\n", ThePath ) );

    if ( ThePath == NULL || TheStat == NULL ) {
        return EINVAL * - 1;
    }
    memset ( TheStat, 0, sizeof ( struct stat ) );

    RCt = _FUSE_get_path_and_node ( ThePath, NULL, & Node, & Type );
    if ( RCt == 0 ) {
        if ( Type == kxfsNotFound ) {
            XFSNodeRelease ( Node );

            return ENOENT * - 1;
        }

        RCt = _FUSE_stat_for_node ( Node, TheStat );

        XFSNodeRelease ( Node );
    }

    return XFS_FUSE_rc_to_errno ( RCt ) * - 1;
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
    const struct XFSNode * Parent;
    char * Child;
    const struct XFSDirEditor * Editor;

    RCt = 0;
    Parent = NULL;
    Child = NULL;
    Editor = NULL;

    XFSMSG ( ( "MKDIR(Fuse): [%s][mode=%d]\n", ThePath, TheMode ) );

    if ( ThePath == NULL ) {
        return EINVAL * - 1;
    }

    RCt = _FUSE_get_parent_node ( ThePath, & Parent, NULL, & Child );
    if ( RCt == 0 ) {
        RCt = XFSNodeDirEditor ( Parent, & Editor );
        if ( RCt == 0 ) {
            RCt = XFSDirEditorCreateDir ( Editor, Child );

            XFSEditorDispose ( & ( Editor -> Papahen ) );
        }

        XFSNodeRelease ( Parent );
        free ( Child );
    }

    return XFS_FUSE_rc_to_errno ( RCt ) * - 1;
}   /* XFS_FUSE_mkdir() */

#endif /* USE_XFS_FUSE_MKDIR == 1 */

/*****************************************************/
#if USE_XFS_FUSE_UNLINK == 1

static
int
XFS_FUSE_unlink ( const char * ThePath )
{
    rc_t RCt;

    RCt = 0;

    XFSMSG ( ( "UNLINK(Fuse): [%s]\n", ThePath ) );

    if ( ThePath == NULL ) {
        return EINVAL * - 1;
    }

    RCt = _FUSE_delete_file_dir ( ThePath );

    return XFS_FUSE_rc_to_errno ( RCt ) * - 1;
}   /* XFS_FUSE_unlink() */

#endif /* USE_XFS_FUSE_UNLINK == 1 */

/*****************************************************/
#if USE_XFS_FUSE_RMDIR == 1

static
int
XFS_FUSE_rmdir ( const char * ThePath )
{
    rc_t RCt;

    RCt = 0;

    XFSMSG ( ( "RMDIR(Fuse): [%s]\n", ThePath ) );

    if ( ThePath == NULL ) {
        return EINVAL * - 1;
    }

    RCt = _FUSE_delete_file_dir ( ThePath );

    return XFS_FUSE_rc_to_errno ( RCt ) * - 1;
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
    const struct XFSNode * OldDir, * NewDir;
    char * OldName, * NewName;
    const struct XFSDirEditor * Editor;

    RCt = 0;
    OldDir = NewDir = NULL;
    OldName = NewName = NULL;
    Editor = NULL;


    XFSMSG ( ( "RENAME(Fuse): from [%s] to [%s]\n", OldPath, NewPath ) );

    if ( OldPath == NULL || NewPath == NULL ) {
        return EINVAL * - 1;
    }

    RCt = _FUSE_get_parent_node ( OldPath, & OldDir, NULL, & OldName );
    if ( RCt == 0 ) {
        RCt = _FUSE_get_parent_node ( NewPath, & NewDir, NULL, & NewName );

        if ( RCt == 0 ) {
            RCt = XFSNodeDirEditor ( OldDir, & Editor );
            if ( RCt == 0 ) {
                RCt = XFSDirEditorMove ( Editor, OldName, NewDir, NewName );
                XFSEditorDispose ( & ( Editor -> Papahen ) );
            }

            XFSNodeRelease ( NewDir );
            free ( NewName );
        }

        XFSNodeRelease ( OldDir );
        free ( OldName );
    }

    return XFS_FUSE_rc_to_errno ( RCt ) * - 1;
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
    rc_t RCt;
    const struct XFSAttrEditor * Editor;
    const struct XFSNode * Node;
    char Buf [ 16 ];

    RCt = 0;
    Editor = NULL;
    Node = NULL;
    * Buf = 0;

    XFSMSG ( ( "CHMOD(Fuse): [%s][mode=%d]\n", ThePath, TheMode ) );

    if ( ThePath == NULL ) {
        return EINVAL * - 1;
    }

    RCt = XFSPermAccessToChar ( TheMode, Buf, sizeof ( Buf ) );
    if ( RCt == 0 ) {
        RCt = _FUSE_get_path_and_node ( ThePath, NULL, & Node, NULL );
        if ( RCt == 0 ) {
            RCt = XFSNodeAttrEditor ( Node, & Editor );
            if ( RCt == 0 ) {
                RCt = XFSAttrEditorSetPermissions ( Editor, Buf );
            }

            XFSNodeRelease ( Node );
        }
    }

    return XFS_FUSE_rc_to_errno ( RCt ) * - 1;
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
    const struct XFSAttrEditor * Editor;
    const struct XFSNode * Node;

    RCt = 0;
    Editor = NULL;
    Node = NULL;

    XFSMSG ( ( "TRUNCATE(Fuse): [%s][SZ=%d] \n", ThePath, TheSize ) );

    if ( ThePath == NULL ) {
        return EINVAL * - 1;
    }

    RCt = _FUSE_get_path_and_node ( ThePath, NULL, & Node, NULL );
    if ( RCt == 0 ) {
        RCt = XFSNodeAttrEditor ( Node, & Editor );
        if ( RCt == 0 ) {
            RCt = XFSAttrEditorSetSize ( Editor, TheSize );
        }

        XFSNodeRelease ( Node );
    }

    return XFS_FUSE_rc_to_errno ( RCt ) * - 1;
}   /* XFS_FUSE_truncate() */

#endif /* USE_XFS_FUSE_TRUNCATE == 1 */

/*****************************************************/
#if USE_XFS_FUSE_UTIME == 1

static
int
XFS_FUSE_utime ( const char * ThePath, struct utimbuf * TheBuf )
{
    rc_t RCt;
    const struct XFSAttrEditor * Editor;
    const struct XFSNode * Node;

    RCt = 0;
    Editor = NULL;
    Node = NULL;

    XFSMSG ( ( "UTIME(Fuse): [%s][AT=%d][MT=%d] \n", ThePath, TheBuf -> actime, TheBuf -> modtime ) );

    if ( ThePath == NULL ) {
        return EINVAL * - 1;
    }

    RCt = _FUSE_get_path_and_node ( ThePath, NULL, & Node, NULL );
    if ( RCt == 0 ) {
        RCt = XFSNodeAttrEditor ( Node, & Editor );
        if ( RCt == 0 ) {
            RCt = XFSAttrEditorSetDate ( Editor, TheBuf -> modtime );
        }

        XFSNodeRelease ( Node );
    }

    return XFS_FUSE_rc_to_errno ( RCt ) * - 1;
}   /* XFS_FUSE_utime() */

#endif /* USE_XFS_FUSE_UTIME == 1 */

/*****************************************************/
#if USE_XFS_FUSE_OPEN == 1

static
int
XFS_FUSE_open ( const char * ThePath, struct fuse_file_info * TheInfo )
{
    rc_t RCt;
    const struct XFSNode * Node;
    XFSNType Type;
    int Flags;
    const struct XFSFileEditor * Editor;
    XFSNMode Mode;
    const struct XFSHandle * Handle;

    RCt = 0;
    Node = NULL;
    Type = kxfsNotFound;
    Flags = TheInfo == NULL ? 0xbad : TheInfo -> flags;
    Editor = NULL;
    Mode = kxfsNone;
    Handle = NULL;

    XFSMSG ( ( "OPEN(Fuse): [%s][FI=0x%p][flags=%d] \n", ThePath, TheInfo, Flags ) );

    if ( ThePath == NULL || TheInfo == NULL ) {
        return EINVAL * - 1;
    }

    if ( ( Flags & O_RDWR ) == O_RDWR ) {
        Mode = kxfsReadWrite;
    }
    else {
        if ( ( Flags & O_WRONLY ) == O_WRONLY ) {
            Mode = kxfsWrite;
        }
        else {
            Mode = kxfsRead;
        }

    }

    RCt = _FUSE_get_path_and_node ( ThePath, NULL, & Node, & Type );
    if ( RCt == 0 ) {
        if ( Type == kxfsNotFound ) {
            XFSNodeRelease ( Node );
            return ENOENT * - 1;
        }

        RCt = XFSNodeFileEditor ( Node, & Editor );
        if ( RCt == 0 ) {
            RCt = XFSFileEditorOpen ( Editor, Mode );
            if ( RCt == 0 ) {
                RCt = XFSHandleMake ( Node, & Handle );
                if ( RCt == 0 ) {
                    XFSHandleSet ( Handle, ( void * ) Editor );
                    TheInfo -> fh = ( uint64_t ) Handle;
                }
            }
        }

        XFSNodeRelease ( Node );
    }

    if ( RCt != 0 ) {
        if ( Editor != NULL ) {
            XFSEditorDispose ( & ( Editor -> Papahen ) );
        }
    }

    XFSMSG ( ( "OPEN(Fuse cont): [%s][FI=0x%p][FH=%p] \n", ThePath, TheInfo, Handle ) );

    return XFS_FUSE_rc_to_errno ( RCt ) * - 1;
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
    const struct XFSHandle * Handle;
    const struct XFSFileEditor * Editor;
    size_t NumBytesReaded;

    RCt = 0;
    Editor = NULL;
    Handle = TheFileInfo == NULL
                        ? NULL
                        : ( const struct XFSHandle * ) TheFileInfo -> fh
                        ;
    NumBytesReaded = 0;


    XFSMSG ( ( "READ(Fuse): [%s][FI=0x%p][FH=0x%p][OF=%d SZ=%d]\n", ThePath, TheFileInfo, Handle, TheOffsetRead, TheSizeRead ) );

    if ( ThePath == NULL || TheBuf == NULL || TheFileInfo == NULL ) {
        return EINVAL * - 1;
    }

    if ( Handle == NULL ) {
        return EBADF * - 1;
    }

    Editor = ( const struct XFSFileEditor * ) XFSHandleGet ( Handle );
    if ( Editor == NULL ) {
        return EBADF * - 1;
    }

    RCt = XFSFileEditorRead (
                        Editor,
                        TheOffsetRead,
                        TheBuf,
                        TheSizeRead,
                        & NumBytesReaded
                        );

    return RCt == 0
                ? NumBytesReaded
                : ( XFS_FUSE_rc_to_errno ( RCt ) * - 1)
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
    const struct XFSHandle * Handle;
    const struct XFSFileEditor * Editor;
    size_t NumBytesWritten;

    RCt = 0;
    Editor = NULL;
    Handle = TheFileInfo == NULL
                        ? NULL
                        : ( const struct XFSHandle * ) TheFileInfo -> fh
                        ;
    NumBytesWritten = 0;

    XFSMSG ( ( "WRITE(Fuse): [%s][FI=0x%p][FH=0x%p][OF=%d SZ=%d]\n", ThePath, TheFileInfo, Handle, TheOffsetWrite, TheSizeWrite ) );

    if ( ThePath == NULL || TheBuf == NULL || TheFileInfo == NULL ) {
        return EINVAL * - 1;
    }

    if ( Handle == NULL ) {
        return EBADF * - 1;
    }

    Editor = ( const struct XFSFileEditor * ) XFSHandleGet ( Handle );
    if ( Editor == NULL ) {
        return EBADF * - 1;
    }

    RCt = XFSFileEditorWrite (
                        Editor,
                        TheOffsetWrite,
                        TheBuf,
                        TheSizeWrite,
                        & NumBytesWritten
                        );

    return RCt == 0
                ? NumBytesWritten
                : ( XFS_FUSE_rc_to_errno ( RCt ) * - 1)
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
    const struct XFSHandle * Handle;
    const struct XFSFileEditor * Editor;

    RCt = 0;
    Editor = NULL;
    Handle = TheFileInfo == NULL
                        ? NULL
                        : ( const struct XFSHandle * ) TheFileInfo -> fh
                        ;

    XFSMSG ( ( "RELEASE(Fuse): [%s][FI=0x%p][FH=0x%p]\n", ThePath, TheFileInfo, Handle ) );

    if ( ThePath == NULL || TheFileInfo == NULL ) {
        return EINVAL * - 1;
    }

    if ( Handle != NULL ) {
        Editor = ( const struct XFSFileEditor * ) XFSHandleGet ( Handle );

        if ( Editor != NULL ) {
            XFSFileEditorClose ( Editor );
            XFSEditorDispose ( & ( Editor -> Papahen ) );
            XFSHandleSet ( Handle, NULL );
        }

        XFSHandleRelease ( Handle );
    }

    return XFS_FUSE_rc_to_errno ( RCt ) * - 1;
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
    OUTMSG ( ( "FSYNC: ThePath %s [FI=0x%p][DT=%d]\n", ThePath, TheFileInfo, DataSync ) );
	return 0;
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
    const struct XFSNode * Node;
    const struct XFSHandle * Handle;
    XFSNType Type;

    RCt = 0;
    Node = NULL;
    Handle = NULL;
    Type = kxfsBadPath;

    if ( ThePath == NULL || TheFileInfo == NULL ) {
        return EINVAL * - 1;
    }

    XFSMSG ( ( "OPENDIR(Fuse): [%s][FI=0x%p][flags=%d]\n", ThePath, TheFileInfo, TheFileInfo -> flags ) );

    RCt = _FUSE_get_path_and_node ( ThePath, NULL, & Node, & Type );
    if ( RCt == 0 ) {
        if ( Type != kxfsDir ) {
            XFSNodeRelease ( Node );

            return ENOENT * - 1;
        }

        RCt = XFSHandleMake ( Node, & Handle );
        if ( RCt == 0 ) {
            TheFileInfo -> fh = ( uint64_t ) Handle;
        }

        XFSNodeRelease ( Node );
    }

    return XFS_FUSE_rc_to_errno ( RCt ) * - 1;
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
    const struct XFSNode * Node;
    const struct XFSHandle * Handle;
    const struct XFSDirEditor * Editor;
    const struct KNamelist * List;
    uint32_t ListQty, llp;
    const char * Name;

    RCt = 0;
    Node = NULL;
    Handle = NULL;
    Editor = NULL;
    List = NULL;
    ListQty = llp = 0;
    Name = NULL;

    if ( ThePath == NULL || TheFileInfo == NULL ) {
        return EINVAL * - 1;
    }

    Handle = ( const struct XFSHandle * ) TheFileInfo -> fh;

    XFSMSG ( ( "READDIR(Fuse): [%s][FI=0x%p][FH=0x%p] \n", ThePath, TheFileInfo, ( void * ) Handle ) );

    if ( Handle != NULL ) {
        Node = XFSHandleNode ( Handle );
        if ( Node == NULL ) {
            RCt = XFS_RC ( rcInvalid );
        }
        else {
            RCt = XFSNodeDirEditor ( Node, & Editor );
            if ( RCt == 0 ) {
                if ( Editor == NULL ) {
                    RCt = XFS_RC ( rcInvalid );
                }
                else {
                    RCt = XFSDirEditorList ( Editor, & List );
                    if ( RCt == 0 ) {
                        RCt = KNamelistCount ( List, & ListQty );
                        if ( RCt == 0 ) {
                            for ( llp = 0; llp < ListQty; llp ++ ) {
                                RCt = KNamelistGet ( List, llp, & Name );
                                if ( RCt == 0 ) { 
                                    TheFiller (
                                            TheBuffer,
                                            Name,
                                            NULL,
                                            0
                                            );
                                }

                                if ( RCt != 0 ) {
/* Do we need that? TODO!!!
                                    break;
*/
                                    RCt = 0; /* Right ? */
                                }
                            }
                        }

                        KNamelistRelease ( List );
                    }

                    XFSEditorDispose ( & ( Editor -> Papahen ) );
                }
            }
            else {
                RCt = XFS_RC ( rcInvalid );
            }
        }
    }
    else {
        RCt = XFS_RC ( rcInvalid );
    }

    return XFS_FUSE_rc_to_errno ( RCt ) * - 1;
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
    const struct XFSHandle * Handle;

    RCt = 0;
    Handle = NULL;

    if ( ThePath == NULL || TheFileInfo == NULL ) {
        return EINVAL * - 1;
    }

    Handle = ( const struct XFSHandle * ) TheFileInfo -> fh;

    XFSMSG ( ( "RELEASEDIR(Fuse): [%s][FI=0x%p][FH=0x%p] \n", ThePath, TheFileInfo, ( void * ) Handle ) );

    if ( Handle != NULL ) {
        RCt = XFSHandleRelease ( Handle );
    }
    else {
        RCt = XFS_RC ( rcInvalid );
    }

    return XFS_FUSE_rc_to_errno ( RCt ) * - 1;
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
    const struct XFSTreeDepot * Depot;

    RCt = _FUSE_tree_depot ( & Depot );

    OUTMSG ( ( "INIT(): TheConnInfo 0x%p TreeDepot %p\n", TheConnInfo, Depot ) );

    return RCt != 0 ? NULL : ( void * ) Depot;
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
XFS_FUSE_access ( const char * ThePath, int Access )
{
    rc_t RCt;
    const struct XFSAttrEditor * Editor;
    const struct XFSNode * Node;
    bool xOK, rOK, wOK, fOK;
    XFSNType Type;
    const char * Perm;
    const struct XFSPerm * xPerm;
    const struct XFSAuth * Auth;

    RCt = 0;
    Editor = NULL;
    Node = NULL;
    Type = kxfsNotFound;
    Perm = NULL;
    xPerm = NULL;
    Auth = NULL;

    xOK = ( Access && X_OK ) == X_OK; /* Can Execute */
    wOK = ( Access && W_OK ) == W_OK; /* Can Write */
    rOK = ( Access && R_OK ) == R_OK; /* Can Read */
    fOK = ( Access && F_OK ) == F_OK; /* File Exists */

    XFSMSG ( ( "ACCESS(Fuse): [%s][mode=%d][x=%d][w=%d][r=%d][f=%d]\n", ThePath, Access, xOK, wOK, rOK, fOK ) );

    if ( ThePath == NULL ) {
        return - 1;
    }

    if ( ! xOK && ! wOK && ! rOK && ! fOK ) {
        return - 1;
    }

    RCt = _FUSE_get_path_and_node ( ThePath, NULL, & Node, & Type );
    if ( RCt == 0 ) {
            /* Check for fOK */
        if ( Type == kxfsNotFound ) {
            RCt = 1;
        }
        else {
            if ( xOK || wOK || rOK ) {
                RCt = XFSNodeAttrEditor ( Node, & Editor );
                if ( RCt == 0 ) {
                    RCt = XFSAttrEditorPermissions ( Editor, & Perm );
                    if ( RCt == 0 ) {
                        RCt = XFSPermMake ( Perm, & xPerm );
                        if ( RCt == 0 ) {
                            Auth = XFSPermAuth ( xPerm, kxfsUser );
                            if ( Auth == NULL ) {
                                RCt = 1;
                            }
                            else {
                                if ( xOK ) {
                                    RCt = ! XFSAuthCanExecute ( Auth );
                                }
                                if ( rOK && RCt == 0 ) {
                                    RCt = ! XFSAuthCanRead ( Auth );
                                }
                                if ( wOK && RCt == 0 ) {
                                    RCt = ! XFSAuthCanWrite ( Auth );
                                }
                            }

                            XFSPermDispose ( xPerm );
                        }
                    }

                    XFSEditorDispose ( & ( Editor -> Papahen ) );
                }
            }
        }

        XFSNodeRelease ( Node );
    }

    return RCt == 0 ? 0 : - 1;
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
    rc_t RCt;
    uint32_t Flags;
    const struct XFSNode * Node;
    const struct XFSDirEditor *Editor;
    const struct XFSHandle * Handle;
    char * Child;
    XFSNMode Mode;

    RCt = 0;
    Node = NULL;
    Editor = NULL;
    Handle = NULL;
    Child = NULL;
    Mode = kxfsNone;

    Flags = TheFileInfo -> flags;

XFSMSG ( ( "CREATE(Fuse): [%s][FI=0x%p][flags=%d][mode=%d]\n", ThePath, TheFileInfo, Flags, TheMode ) );

    if ( ThePath == NULL ) {
        return EINVAL * - 1;
    }

    if ( ( Flags & O_RDWR ) == O_RDWR ) {
        Mode = kxfsReadWrite;
    }
    else {
        if ( ( Flags & O_WRONLY ) == O_WRONLY ) {
            Mode = kxfsWrite;
        }
        else {
            Mode = kxfsRead;
        }
    }

    RCt = _FUSE_get_parent_node ( ThePath, & Node, NULL, & Child );
    if ( RCt == 0 ) {
        RCt = XFSNodeDirEditor ( Node, & Editor );
        if ( RCt == 0 ) {
            RCt = XFSDirEditorCreate ( Editor, Child, Mode, & Handle );
            if ( RCt != 0 ) {
                XFSEditorDispose ( & ( Editor -> Papahen ) );
                TheFileInfo -> fh = 0;
            }
            else {
                TheFileInfo -> fh = ( uint64_t ) Handle;
            }
        }

        XFSNodeRelease ( Node );
        free ( Child );
    }

XFSMSG ( ( "CREATE(Fuse, cont): [%s][FI=0x%p][FH=0x%p][mode=%d]\n", ThePath, TheFileInfo, Handle, Mode ) );

    return XFS_FUSE_rc_to_errno ( RCt ) * - 1;
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
    const struct XFSAttrEditor * Editor;
    const struct XFSNode * Node;

    RCt = 0;
    Editor = NULL;
    Node = NULL;

    XFSMSG ( ( "FTRUNCATE(Fuse): [%s][SZ=%d]\n", ThePath, TheSize ) );

    if ( ThePath == NULL ) {
        return EINVAL * - 1;
    }

    RCt = _FUSE_get_path_and_node ( ThePath, NULL, & Node, NULL );
    if ( RCt == 0 ) {
        RCt = XFSNodeAttrEditor ( Node, & Editor );
        if ( RCt == 0 ) {
            RCt = XFSAttrEditorSetSize ( Editor, TheSize );
        }

        XFSNodeRelease ( Node );
    }

    return XFS_FUSE_rc_to_errno ( RCt ) * - 1;
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
    const struct XFSHandle * Handle;
    const struct XFSNode * Node;

    RCt = 0;
    Handle = ( const struct XFSHandle * ) TheFileInfo -> fh;
    Node = NULL;

    XFSMSG ( ( "FGETATTR(Fuse): [%s][FI=0x%p][FG=0x%p]\n", ThePath, TheFileInfo, Handle ) );

    if ( ThePath == NULL || TheStat == NULL || TheFileInfo == NULL ) {
        return EINVAL * - 1;
    }

    memset ( TheStat, 0, sizeof ( struct stat ) );

    Node = XFSHandleNode ( Handle );
    if ( Node == NULL ) {
        return EINVAL * - 1;
    }

    RCt = _FUSE_stat_for_node ( Node, TheStat );

    return XFS_FUSE_rc_to_errno ( RCt ) * - 1;
}   /* XFS_FUSE_fgetattr () */

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
