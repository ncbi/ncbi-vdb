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
#include <klib/container.h>
#include <klib/refcount.h>
#include <klib/printf.h>
#include <klib/vector.h>
#include <kproc/lock.h>
#include <vfs/path.h>
#include <vfs/manager.h>
#include <vfs/resolver.h>
#include <kns/http.h>

#include <kfg/repository.h>

#include <kfs/file.h>
#include <kfs/directory.h>
#include <kfs/file-impl.h>
#include <kfs/cacheteefile.h>
#include <krypto/key.h>
#include <krypto/encfile.h>
#include <krypto/wgaencrypt.h>

#include <xfs/path.h>
#include <xfs/perm.h>
#include "schwarzschraube.h"
#include "xgap.h"
#include "xgapf.h"
#include "spen.h"
#include "zehr.h"

#include <sysalloc.h>

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*))    Useful defines
 ((*/

    /* Do not comment that line unless You know what it does */
#define _USE_CACHED_CONN_

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))    XFSGapFiles are living here
 ((*/

struct _GapFiles {
    BSTree tree;

    KLock * mutabor;

    const struct XFSPen * pen;
};

typedef enum _EncType {
                    kxfsEncInvalid,
                    kxfsEncReady,
                    kxfsEncNone,
                    kxfsEncEncFile,
                    kxfsEncSraFile,
                    kxfsEncWGAFile,
                    kxfsEncBroken
                    } _EncType;

struct _GapFilePeer {
    BSTNode node;

    KLock * mutabor;
    KRefcount refcount;

    uint32_t project_id;
    const char * accession;
    uint32_t object_id;

    const char * remote_url;
    const char * cache_path;

    XFSStatus status;
    _EncType enc_type;

    uint64_t size;

    const struct KFile * file;
};

struct _GapFile {
    struct KFile file;

    const struct KFile * looney_bin;

    struct _GapFilePeer * peer;
};


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* _GapFiles ...                                                     */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
static struct _GapFiles * _sFiles = NULL;

static
struct _GapFiles *
_FilesGet ()
{
    return _sFiles;
}   /* _FilesGet () */

static rc_t CC _GapFilePeerAddRef ( struct _GapFilePeer * self );
static rc_t CC _GapFilePeerRelease ( struct _GapFilePeer * Peer );
static rc_t CC _GapFilePeerMake (
                                struct _GapFilePeer ** File,
                                uint32_t ProjectId,
                                const char * Accession,
                                uint32_t ObjectId
                                );
static rc_t CC _GapFilePeerDispose ( struct _GapFilePeer * self );

static
void CC
_GapFilesWhackCallback ( BSTNode * Node, void * unused )
{
    if ( Node != NULL ) {
        _GapFilePeerRelease ( ( struct _GapFilePeer* ) Node );
    }
}   /* _GapFilesWhackCallback () */

static
rc_t CC
_GapFilesDispose ( struct _GapFiles * self )
{
    if ( self != NULL ) {
        BSTreeWhack (
                    & ( self -> tree ),
                    _GapFilesWhackCallback,
                    NULL
                    );

        if ( self -> pen != NULL ) {
            XFSPenDispose ( self -> pen );
            self -> pen = NULL;
        }

        if ( self -> mutabor != NULL ) {
            KLockRelease ( self -> mutabor );
            self -> mutabor = NULL;
        }

        free ( self );
    }

    return 0;
}   /* _GapFilesDispose () */

static
rc_t CC
_GapFilesMake ( struct _GapFiles ** Files )
{
    rc_t RCt;
    struct _GapFiles * TheFiles;

    RCt = 0;
    TheFiles = NULL;

    XFS_CSAN ( Files )
    XFS_CAN ( Files )

    TheFiles = calloc ( 1, sizeof ( struct _GapFiles ) );
    if ( TheFiles == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFSPenMake ( & ( TheFiles -> pen ), 65 );
        if ( RCt == 0 ) {
            RCt = KLockMake ( & ( TheFiles -> mutabor ) );
            if ( RCt == 0 ) {
                BSTreeInit ( & ( TheFiles -> tree ) );

                * Files = TheFiles;
            }
        }
    }

    if ( RCt != 0 ) {
        * Files = NULL;

        if ( TheFiles != NULL ) {
            _GapFilesDispose ( TheFiles );
        }
    }

    return RCt;
}   /* _GapFilesDispose () */

LIB_EXPORT
rc_t CC
XFSGapFilesInit ()
{
    rc_t RCt = 0;

    RCt = XFSGapInit ();
    if ( RCt != 0 ) {
        return RCt;
    }

    if ( _sFiles == NULL ) {
        return _GapFilesMake ( & _sFiles );
    }

    return 0;
}   /* XFSGapFilesInit () */

LIB_EXPORT
rc_t CC
XFSGapFilesDispose ()
{
    rc_t RCt;
    struct _GapFiles * Files;

    RCt = 0;
    Files = _sFiles;

    if ( Files != NULL ) {
        _sFiles = NULL;

        RCt = _GapFilesDispose ( Files );
    }

    XFSGapDispose ();

    return RCt;
}   /* XFSGapFilesDispose () */

static
int64_t CC
_GapFilesFindCallback ( const void * Item, const BSTNode * Node )
{
    const char * Str1, * Str2;

    Str1 = ( ( char * ) Item );
    Str2 = Node == NULL
                    ? NULL
                    : ( ( struct _GapFilePeer * ) Node ) -> cache_path
                    ;


    return XFS_StringCompare4BST_ZHR ( Str1, Str2 );
}   /* _GapFilesFindCallback () */

static
rc_t CC
_GapFilesFind_NoLock (
                    struct _GapFiles * self,
                    struct _GapFilePeer ** Peer,
                    const char * CachePath
)
{
    rc_t RCt;
    struct _GapFilePeer * ThePeer;

    RCt = 0;
    ThePeer = NULL;

    XFS_CSAN ( Peer )
    XFS_CAN ( self )
    XFS_CAN ( Peer )
    XFS_CAN ( CachePath )

    ThePeer = ( struct _GapFilePeer * ) BSTreeFind ( 
                                        & ( self -> tree ),
                                        CachePath,
                                        _GapFilesFindCallback
                                        );
    if ( ThePeer == NULL ) {

        RCt = XFS_RC ( rcNotFound );
    }
    else {

        * Peer = ThePeer;
        RCt = 0;
    }

    return RCt;
}   /* _GapFilesFind_NoLock () */

static
int64_t CC
_GapFilesAddCallback (
                    const struct BSTNode * N1,
                    const struct BSTNode * N2
)
{
    const char * Str1, * Str2;


    Str1 = N1 == NULL
                    ? NULL
                    : ( ( ( struct _GapFilePeer * ) N1 ) -> cache_path )
                    ;
    Str2 = N2 == NULL
                    ? NULL
                    : ( ( ( struct _GapFilePeer * ) N2 ) -> cache_path )
                    ;

    return XFS_StringCompare4BST_ZHR ( Str1, Str2 );
}   /* _GapFilesAddCallback () */

static
rc_t CC
_GapFilesAddPeer_NoLock (
                    struct _GapFiles * self,
                    struct _GapFilePeer ** Peer,
                    uint32_t ProjectId,
                    const char * Accession,
                    uint32_t ObjectId
)
{
    rc_t RCt;
    struct _GapFilePeer * ThePeer;

    RCt = 0;
    ThePeer = NULL;

    XFS_CSAN ( Peer )
    XFS_CAN ( self )
    XFS_CAN ( Peer )

    RCt = _GapFilePeerMake (
                        & ThePeer,
                        ProjectId,
                        Accession,
                        ObjectId
                        );
    if ( RCt == 0 ) {
        RCt = BSTreeInsert (
                        & ( self -> tree ),
                        ( struct BSTNode * ) ThePeer,
                        _GapFilesAddCallback
                        );
        if ( RCt == 0 ) {
            * Peer = ThePeer;
        }
    }

    if ( RCt != 0 ) {
        * Peer = NULL;

        if ( ThePeer != NULL ) {
            _GapFilePeerDispose ( ThePeer );
        }
    }

    return RCt;
}   /* _GapFilesAddPeer_NoLock () */

static
rc_t CC
_GapFilesFindOrCreate (
                    struct _GapFilePeer ** Peer,
                    uint32_t ProjectId,
                    const char * Accession,
                    uint32_t ObjectId
)
{
    rc_t RCt;
    struct _GapFiles * Files;
    struct _GapFilePeer * ThePeer;
    const struct XFSGapProject * Project;
    const char * CachePath;

    RCt = 0;
    ThePeer = NULL;
    Files = _FilesGet ();
    Project = NULL;
    CachePath = NULL;

    XFS_CSAN ( Peer )
    XFS_CAN ( Files )
    XFS_CAN ( Peer )

    if ( Accession == NULL && ObjectId == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = XFSGapFindOrCreate ( ProjectId, & Project );
    if ( RCt == 0 ) {
        if ( Accession != NULL ) {
            RCt = XFSGapProjectAccessionUrlAndPath (
                                                Project,
                                                Accession,
                                                NULL,
                                                & CachePath
                                                );
        }
        else {
            RCt = XFSGapProjectObjectUrlAndPath (
                                                Project,
                                                ObjectId,
                                                NULL,
                                                & CachePath
                                                );
        }

        if ( RCt == 0 ) {
            RCt = KLockAcquire ( Files -> mutabor );
            if ( RCt == 0 ) {
                    /* First we trying to locate already ready peer
                     */
                RCt = _GapFilesFind_NoLock (
                                            Files,
                                            & ThePeer,
                                            CachePath
                                            );
                if ( GetRCState ( RCt ) == rcNotFound ) {
                    RCt = _GapFilesAddPeer_NoLock (
                                                Files,
                                                & ThePeer,
                                                ProjectId,
                                                Accession,
                                                ObjectId
                                                );
                }

                if ( RCt == 0 ) {
                    RCt = _GapFilePeerAddRef ( ThePeer );
                    if ( RCt == 0 ) {
                        * Peer = ThePeer;
                    }
                }

                KLockUnlock ( Files -> mutabor );
            }

            free ( ( char * ) CachePath );
        }

        XFSGapProjectRelease ( Project );
    }

    return RCt;
}   /* _GapFilesFindOrCreate () */

static rc_t CC _GapFilePeerOpen (
                            struct _GapFilePeer * self,
                            const struct KFile ** File
                            );
static
rc_t CC
_GapFilesOpen (
                const struct KFile ** File,
                uint32_t ProjectId,
                const char * Accession,
                uint32_t ObjectId
)
{
    rc_t RCt;
    struct _GapFilePeer * Peer;
    const struct KFile * TheFile;

    RCt = 0;
    Peer = NULL;
    TheFile = NULL;

    XFS_CSAN ( File )
    XFS_CAN ( File )

    RCt = _GapFilesFindOrCreate (
                                & Peer,
                                ProjectId,
                                Accession,
                                ObjectId
                                );
    if ( RCt == 0 ) {
        RCt = _GapFilePeerOpen ( Peer, & TheFile );
        if ( RCt == 0 ) {
            * File = TheFile;
        }

        _GapFilePeerRelease ( Peer );
    }

    return RCt;
}   /* _GapFilesOpen () */

LIB_EXPORT
rc_t CC
XFSGapFilesOpenAccession (
                        const struct KFile ** File,
                        uint32_t ProjectId,
                        const char * Accession
)
{
    return _GapFilesOpen ( File, ProjectId, Accession, 0 );
}   /* XFSGapFilesOpenAccession () */

LIB_EXPORT
rc_t CC
XFSGapFilesOpenObjectId (
                        const struct KFile ** File,
                        uint32_t ProjectId,
                        uint32_t ObjectId
)
{
    return _GapFilesOpen ( File, ProjectId, NULL, ObjectId );
}   /* XFSGapFilesOpenObjectId () */

static
rc_t CC
_GapFilesSize (
                uint64_t * Size,
                uint32_t ProjectId,
                const char * Accession,
                uint32_t ObjectId
)
{
    rc_t RCt;
    struct _GapFilePeer * Peer;
    const struct KFile * TheFile;

    RCt = 0;
    Peer = NULL;
    TheFile = NULL;

    XFS_CSA ( Size, 0 )
    XFS_CAN ( Size )

    RCt = _GapFilesFindOrCreate (
                                & Peer,
                                ProjectId,
                                Accession,
                                ObjectId
                                );
    if ( RCt == 0 ) {
        if ( Peer -> size == 0 ) {
            RCt = _GapFilePeerOpen ( Peer, & TheFile );
            if ( RCt == 0 ) {
                KFileRelease ( TheFile );
            }
        }

        if ( RCt == 0 ) {
            * Size = Peer -> size;
        }

        _GapFilePeerRelease ( Peer );
    }

    return RCt;
}   /* _GapFilesSize () */

LIB_EXPORT
rc_t CC
XFSGapFilesAccessionSize (
                        uint64_t * Size,
                        uint32_t ProjectId,
                        const char * Accession
)
{
    return _GapFilesSize ( Size, ProjectId, Accession, 0 );
}   /* XFSGapFilesAccessionSize () */

LIB_EXPORT
rc_t CC
XFSGapFilesObjectIdSize (
                        uint64_t * Size,
                        uint32_t ProjectId,
                        uint32_t ObjectId
)
{
    return _GapFilesSize ( Size, ProjectId, NULL, ObjectId );
}   /* XFSGapFilesObjectIdSize () */

static
rc_t CC
_GapFilesPathDate (
                KTime_t * Time,
                const char * Path,
                bool Completed
)
{
    rc_t RCt;
    struct KDirectory * NatDir;

    RCt = 0;
    NatDir = NULL;

    XFS_CSA ( Time, 0 )
    XFS_CAN ( Time )
    XFS_CAN ( Path )

    RCt = KDirectoryNativeDir ( & NatDir );
    if ( RCt == 0 ) {
        if ( Completed ) {
            RCt = KDirectoryDate ( NatDir, Time, Path );
        }
        else {
            RCt = KDirectoryDate ( NatDir, Time, "%s.cache", Path );
        }

        KDirectoryRelease ( NatDir );
    }

    return RCt;
}   /* _GapFilesPathDate () */

static
rc_t CC
_GapFilesDate (
                KTime_t * Time,
                uint32_t ProjectId,
                const char * Accession,
                uint32_t ObjectId
)
{
    rc_t RCt;
    struct _GapFilePeer * Peer;
    const struct KFile * TheFile;

    RCt = 0;
    Peer = NULL;
    TheFile = NULL;

    XFS_CSA ( Time, 0 )
    XFS_CAN ( Time )

    RCt = _GapFilesFindOrCreate (
                                & Peer,
                                ProjectId,
                                Accession,
                                ObjectId
                                );
    if ( RCt == 0 ) {
            /*  It may be file does not exists
             */
        if ( Peer -> size == 0 ) {
            RCt = _GapFilePeerOpen ( Peer, & TheFile );
            if ( RCt == 0 ) {
                KFileRelease ( TheFile );
            }
        }

        if ( RCt == 0 ) {
            RCt = _GapFilesPathDate (
                                    Time,
                                    Peer -> cache_path,
                                    Peer -> status == kxfsComplete
                                    );
        }

        _GapFilePeerRelease ( Peer );
    }

    return RCt;
}   /* _GapFilesDate () */

LIB_EXPORT
rc_t CC
XFSGapFilesAccessionDate (
                        KTime_t * Time,
                        uint32_t ProjectId,
                        const char * Accession
)
{
    return _GapFilesDate ( Time, ProjectId, Accession, 0 );
}   /* XFSGapFilesAccessionDate () */

LIB_EXPORT
rc_t CC
XFSGapFilesObjectIdDate (
                        KTime_t * Time,
                        uint32_t ProjectId,
                        uint32_t ObjectId
)
{
    return _GapFilesDate ( Time, ProjectId, NULL, ObjectId );
}   /* XFSGapFilesObjectIdDate () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* _GapFilesPeer                                                     */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static const char * _sGapFilePeer_classname = "GapFilePeer";

rc_t CC
_GapFilePeerDispose ( struct _GapFilePeer * self )
{
    if ( self != NULL ) {
        if ( self -> mutabor != NULL ) {
            KLockRelease ( self -> mutabor );
            self -> mutabor = NULL;
        }

        KRefcountWhack (
                        & ( self -> refcount ),
                        _sGapFilePeer_classname
                        );

        self -> project_id = 0;

        if ( self -> accession != NULL ) {
            free ( ( char * ) self -> accession );
            self -> accession = NULL;
        }

        self -> object_id = 0;

        if ( self -> remote_url != NULL ) {
            free ( ( char * ) self -> remote_url );
            self -> remote_url = NULL;
        }

        if ( self -> cache_path != NULL ) {
            free ( ( char * ) self -> cache_path );
            self -> cache_path = NULL;
        }

        self -> status = kxfsReady;
        self -> enc_type = kxfsEncReady;
        self -> size = 0;

        if ( self -> file != NULL ) {
            ( ( struct _GapFile * ) self -> file ) -> peer = NULL;
            KFileRelease ( self -> file );
            self -> file = NULL;
        }

        free ( self );
    }

    return 0;
}   /* _GapFilePeerDispose () */

rc_t CC
_GapFilePeerMake (
                struct _GapFilePeer ** File,
                uint32_t ProjectId,
                const char * Accession,
                uint32_t ObjectId
)
{
    rc_t RCt;
    const struct XFSGapProject * Project;
    struct _GapFilePeer * TheFile;

    RCt = 0;
    Project = NULL;
    TheFile = NULL;

    XFS_CSAN ( File )
    XFS_CAN ( File )

    if ( Accession == NULL && ObjectId == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    TheFile = calloc ( 1, sizeof ( struct _GapFilePeer ) );
    if ( TheFile == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        KRefcountInit (
                    & ( TheFile -> refcount ),
                    1,
                    _sGapFilePeer_classname,
                    "GapFilePeerMake",
                    "GapFilePeer"
                    );
        TheFile -> project_id = ProjectId;
        TheFile -> enc_type = kxfsEncInvalid;

        RCt = XFSGapFindOrCreate ( ProjectId, & Project );

        TheFile -> status = RCt == 0 ? kxfsInvalid : kxfsBroken;
        TheFile -> size = 0;

        if ( RCt == 0 ) {
            if ( Accession != NULL ) {
                RCt = XFS_StrDup (
                                Accession,
                                & ( TheFile -> accession )
                                );
            }
            else {
                TheFile -> object_id = ObjectId;
            }

            if ( RCt == 0 ) {
                RCt = KLockMake ( & ( TheFile -> mutabor ) );
                if ( RCt == 0 ) {

                    if ( Accession != NULL ) {
                        RCt = XFSGapProjectAccessionUrlAndPath (
                                            Project,
                                            Accession,
                                            & ( TheFile -> remote_url ),
                                            & ( TheFile -> cache_path )
                                            );
                    }
                    else {
                        RCt = XFSGapProjectObjectUrlAndPath (
                                            Project,
                                            ObjectId,
                                            & ( TheFile -> remote_url ),
                                            & ( TheFile -> cache_path )
                                            );
                    }

                    if ( RCt == 0 ) {
                        TheFile -> status = kxfsReady;
                        TheFile -> enc_type = kxfsEncReady;
                        TheFile -> size = 0;
                    }
                }
            }

            XFSGapProjectRelease ( Project );
        }
    }

    if ( RCt != 0 ) {
        if ( TheFile != NULL ) {
            TheFile -> status = kxfsBroken;
            TheFile -> enc_type = kxfsEncBroken;
            TheFile -> size = 0;
        }
    }

        /*  We have to return bad files too, to avoid multiply attempt
         *  of initialization of the same files
         */
    if ( TheFile != NULL ) {
        * File = TheFile;
    }

    return RCt;
}   /* _GapFilePeerMake () */

rc_t CC
_GapFilePeerAddRef ( struct _GapFilePeer * self )
{
    rc_t RCt;
    int RefC;

    RCt = 0;
    RefC = 0;

    XFS_CAN ( self );

    RefC = KRefcountAdd (
                        & ( self -> refcount ),
                        _sGapFilePeer_classname
                        );
    switch ( RefC ) {
        case krefOkay :
            RCt = 0;
            break;
        case krefZero :
        case krefLimit :
        case krefNegative :
            RCt = XFS_RC ( rcInvalid );
            break;
        default :
            RCt = XFS_RC ( rcUnknown );
            break;
    }

    return RCt;
}   /* _GapFilePeerAddRef () */

rc_t CC
_GapFilePeerRelease ( struct _GapFilePeer * self )
{
    rc_t RCt;
    int RefC;

    RCt = 0;
    RefC = 0;

    XFS_CAN ( self );

    RefC = KRefcountDrop (
                        & ( self -> refcount ),
                        _sGapFilePeer_classname
                        );
    switch ( RefC ) {
        case krefOkay :
        case krefZero :
                RCt = 0;
                break;
        case krefWhack :
                RCt = _GapFilePeerDispose ( self );
                break;
        case krefNegative :
                RCt = XFS_RC ( rcInvalid );
                break;
        default :
                RCt = XFS_RC ( rcUnknown );
                break;
    }

    return RCt;
}   /* _GapFilePeerRelease () */

static
bool CC
_GapFilePeerGood ( struct _GapFilePeer * self )
{
    if ( self != NULL ) {
        return self -> status == kxfsReady || self -> status == kxfsGood;
    }
    return false;
}   /* _GapFilePeerGood () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* _GapFile ...                                                      */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static
rc_t CC
_GapFileLockPeer ( const struct _GapFile * File )
{
    XFS_CAN ( File );
    XFS_CAN ( File -> peer );
    XFS_CAN ( File -> peer -> mutabor );

    return KLockAcquire ( File -> peer -> mutabor );
}   /* _GapFileLockPeer () */

static
rc_t CC
_GapFileUnlockPeer ( const struct _GapFile * File )
{
    XFS_CAN ( File );
    XFS_CAN ( File -> peer );
    XFS_CAN ( File -> peer -> mutabor );

    return KLockUnlock ( File -> peer -> mutabor );
}   /* _GapFileUnlockPeer () */

/*))
 || _GapFile virtual table
((*/

static rc_t CC _GapFile_destroy (
                                struct KFile * self
                                );
static struct KSysFile_v1 * CC _GapFile_get_sysfile (
                                const struct KFile * self,
                                uint64_t * Offset
                                );
static rc_t CC _GapFile_random_access (
                                const struct KFile * self
                                );
static rc_t CC _GapFile_get_size (
                                const struct KFile * self,
                                uint64_t * Size
                                );
static rc_t CC _GapFile_set_size (
                                struct KFile * self,
                                uint64_t Size
                               );
static rc_t CC _GapFile_read (
                                const struct KFile * self,
                                uint64_t Offset,
                                void * Buffer,
                                size_t BufferSize,
                                size_t * NumRead
                                );
static rc_t CC _GapFile_write (
                                struct KFile * self,
                                uint64_t Offset,
                                const void * Buffer,
                                size_t BufferSize,
                                size_t * NumWrite
                                );
static uint32_t CC _GapFile_get_type (
                                const struct KFile * self
                                );

#ifdef MINOR_2_JOJOBA
static rc_t CC _GapFile_timed_read (
                                const struct KFile * self,
                                uint64_t Offset,
                                void * Buffer,
                                size_t BufferSize,
                                size_t * NumRead,
                                struct timeout_t * Tm
                                );
static rc_t CC _GapFile_timed_write (
                                struct KFile * self,
                                uint64_t Offset,
                                const void * Buffer,
                                size_t BufferSize,
                                size_t * NumWrit,
                                struct timeout_t * Tm
                                );
#endif /* MINOR_2_JOJOBA */

static struct KFile_vt_v1 _svtGapFile = {
                                1,  /* maj */
                                1,  /* min */

                            /* start minor version == 0 */
                                _GapFile_destroy,
                                _GapFile_get_sysfile,
                                _GapFile_random_access,
                                _GapFile_get_size,
                                _GapFile_set_size,
                                _GapFile_read,
                                _GapFile_write,
                            /* end minor version == 0 */

                            /* start minor version == 1 */
                                _GapFile_get_type,
                            /* end minor version == 1 */

#ifdef MINOR_2_JOJOBA
                            /* start minor version == 2 */
                                _GapFile_timed_read,
                                _GapFile_timed_write,
                            /* end minor version == 2 */
#endif /* MINOR_2_JOJOBA */
};

/*))
 || _GapFile constructor
((*/
static
rc_t CC
_GapFileMake (
            const struct KFile ** File,
            const struct KFile * LooneyBin,
            struct _GapFilePeer * Peer
)
{
    rc_t RCt;
    struct _GapFile * GapFile;

    RCt = 0;
    GapFile = NULL;

    XFS_CSAN ( File )
    XFS_CAN ( File )
    XFS_CAN ( LooneyBin )
    XFS_CAN ( Peer )

        /*  First we should check if Peer is good and valid 
         */
    if ( ! _GapFilePeerGood ( Peer ) ) {
        return XFS_RC ( rcInvalid );
    }

    GapFile = calloc ( 1, sizeof ( struct _GapFile ) );
    if ( GapFile == NULL ) {
        RCt = XFS_RC ( rcNull );
    }
    else {
        RCt = KFileInit (
                & ( GapFile -> file ),
                ( const KFile_vt * ) & _svtGapFile,
                "GapFile",
                "GapFile",
                true,
                false
                );
        if ( RCt == 0 ) {
            RCt = _GapFilePeerAddRef ( Peer );
            if ( RCt == 0 ) {
                GapFile -> peer = Peer;
                GapFile -> looney_bin = LooneyBin;

                * File = ( const KFile * ) & ( GapFile -> file );

                Peer -> file = * File;
            }
        }
    }

    if ( RCt != 0 ) {
        * File = NULL;
        KFileRelease ( & ( GapFile -> file ) );
    }

    return RCt;
}   /* _GapFileMake () */

/*))
 || _GapFile virtual table implementation
((*/
rc_t CC
_GapFile_destroy ( struct KFile * self )
{
    struct _GapFile * File;
    struct _GapFilePeer * Peer;

    Peer = NULL;
    File = ( struct _GapFile * ) self;

    if ( File != NULL ) {

        Peer = File -> peer;
        if ( Peer != NULL ) {
            KLockAcquire ( Peer -> mutabor );

            File -> peer = NULL;


            if ( Peer -> file == & ( File -> file ) ) {
                Peer -> file = NULL;
            }

            KLockUnlock ( Peer -> mutabor );

            _GapFilePeerRelease ( Peer );
        }

        if ( File -> looney_bin != NULL ) {
            KFileRelease ( File -> looney_bin );
            File -> looney_bin = NULL;
        }

        free ( File );
    }

    return 0;
}   /* _GapFile_destroy () */

struct KSysFile_v1 * CC
_GapFile_get_sysfile ( const struct KFile * self, uint64_t * Offset )
{
    struct _GapFile * File = ( struct _GapFile * ) self;

    if ( File != NULL ) {
        if ( File -> looney_bin != NULL ) {
            return KFileGetSysFile ( File -> looney_bin, Offset );
        }
    }

    return NULL;
}   /* _GapFile_get_sysfile () */

rc_t CC
_GapFile_random_access ( const struct KFile * self )
{
    struct _GapFile * File = ( struct _GapFile * ) self;

    XFS_CAN ( File )
    XFS_CAN ( File -> looney_bin )

    return KFileRandomAccess ( File -> looney_bin );
}   /* _GapFile_random_access () */

rc_t CC
_GapFile_get_size ( const struct KFile * self, uint64_t * Size )
{
    rc_t RCt;
    struct _GapFile * File;

    RCt = 0;
    File = ( struct _GapFile * ) self;

    XFS_CSA ( Size, 0 )
    XFS_CAN ( File )
    XFS_CAN ( Size )
    XFS_CAN ( File -> looney_bin )

    RCt = _GapFileLockPeer ( File );
    if ( RCt == 0 ) {
        RCt = KFileSize ( File -> looney_bin, Size );

        _GapFileUnlockPeer ( File );
    }

    return RCt;
}   /* _GapFile_get_size () */

rc_t CC
_GapFile_set_size ( struct KFile * self, uint64_t Size )
{
    XFS_CAN ( self )

    return RC (rcFS, rcDirectory, rcUpdating, rcSelf, rcUnsupported);
}   /* _GapFile_set_size () */

rc_t CC
_GapFile_read (
                const struct KFile * self,
                uint64_t Offset,
                void * Buffer,
                size_t BufferSize,
                size_t * NumRead
)
{
    rc_t RCt;
    struct _GapFile * File;

    RCt = 0;
    File = ( struct _GapFile * ) self;

    XFS_CSA ( NumRead, 0 )
    XFS_CAN ( File )
    XFS_CAN ( Buffer )
    XFS_CAN ( NumRead )
    XFS_CAN ( File -> looney_bin )

    RCt = _GapFileLockPeer ( File );
    if ( RCt == 0 ) {
        RCt = KFileReadAll (
                        File -> looney_bin,
                        Offset,
                        Buffer,
                        BufferSize,
                        NumRead
                        );

        _GapFileUnlockPeer ( File );
    }

    return RCt;
}   /* _GapFile_read () */

rc_t CC
_GapFile_write (
                struct KFile * self,
                uint64_t Offset,
                const void * Buffer,
                size_t BufferSize,
                size_t * NumWrite
)
{
    XFS_CSA ( NumWrite, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Buffer )
    XFS_CAN ( NumWrite )

    return RC (rcFS, rcDirectory, rcUpdating, rcSelf, rcUnsupported);
}   /* _GapFile_write () */

uint32_t CC
_GapFile_get_type ( const struct KFile * self )
{
    struct _GapFile * File = ( struct _GapFile * ) self;
    return KFileType ( File -> looney_bin );
}   /* _GapFile_get_type () */


#ifdef MINOR_2_JOJOBA
rc_t CC
_GapFile_timed_read (
                const struct KFile * self,
                uint64_t Offset,
                void * Buffer,
                size_t BufferSize,
                size_t * NumRead,
                struct timeout_t * Tm
)
{
    rc_t RCt;
    struct _GapFile * File;

    RCt = 0;
    File = ( struct _GapFile * ) self;

    XFS_CSA ( NumRead, 0 )
    XFS_CAN ( File )
    XFS_CAN ( Buffer )
    XFS_CAN ( NumRead )
    XFS_CAN ( File -> looney_bin )

    RCt = _GapFileLockPeer ( File );
    if ( RCt == 0 ) {
        KFileTimedRead (
                        File,
                        Offset,
                        Buffer,
                        BufferSize,
                        NumRead,
                        Tm
                        );

        _GapFileUnlockPeer ( File );
    }

    return RCt;
}   /* _GapFile_timed_read () */

rc_t CC
_GapFile_timed_write (
                struct KFile * self,
                uint64_t Offset,
                const void * Buffer,
                size_t BufferSize,
                size_t * NumWrite,
                struct timeout_t * Tm
)
{
    XFS_CSA ( NumWrite, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Buffer )
    XFS_CAN ( NumWrite )
    
    return RC (rcFS, rcDirectory, rcUpdating, rcSelf, rcUnsupported);
}   /* _GapFile_timed_write () */

#endif /* MINOR_2_JOJOBA */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* _GapFilePeerOpen                                                  */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static
rc_t CC
_CheckMakeParentDir ( struct KDirectory * Directory, const char * Path )
{
    rc_t RCt;
    const struct XFSPath * xPath;
    const struct XFSPath * xParent;
    const char * Parent;
    uint32_t PathType;

    RCt = 0;
    xPath = NULL;
    xParent = NULL;
    Parent = NULL;
    PathType = kptNotFound;

    XFS_CAN ( Directory )
    XFS_CAN ( Path )

    RCt = XFSPathMake ( & xPath, false, Path );
    if ( RCt == 0 ) {
        RCt = XFSPathParent ( xPath, & xParent );
        if ( RCt == 0 ) {
            Parent = XFSPathGet ( xParent );

            PathType = KDirectoryPathType ( Directory, Parent );
            if ( PathType == kptNotFound ) {
                RCt = KDirectoryCreateDir (
                                        Directory,
                                        XFSPermRWDefContNum (),
                                        kcmCreate,
                                        Parent
                                        );
            }
            else {
                if ( PathType != kptDir ) {
                    RCt = XFS_RC ( rcInvalid );
                }
            }

            XFSPathRelease ( xParent );
        }

        XFSPathRelease ( xPath );
    }

    return RCt;
}   /* _CheckMakeParentDir () */

static
rc_t CC
_Take_Burro_From_Pen (
                    const struct KFile ** File,
                    const char * Url,
                    bool Cached
)
{
    rc_t RCt;
    struct _GapFiles * Files;
    const struct XFSBurro * Burro;

    RCt = 0;
    Files = _FilesGet ();
    Burro = NULL;

    RCt = XFSPenGet ( Files -> pen, & Burro, Url );
    if ( RCt == 0 ) {
        if ( ! XFSBurroGood ( Burro ) ) {
            RCt = XFS_RC ( rcInvalid );
        }
        else {
            RCt = Cached
                        ? XFSBurroCachedFile ( Burro, File )
                        : XFSBurroRecachedFile ( Burro, File )
                        ;
        }

        XFSBurroRelease ( Burro );
    }

    return RCt;
}   /* _Take_Burro_From_Pen_ () */

static
rc_t CC
_Open_File_1 ( struct _GapFilePeer * self, const struct KFile ** File )
{
    rc_t RCt;
    uint32_t PathType;
    struct KDirectory * NatDir;
    const struct KFile * File_1;
    const struct KFile * File_2;

    RCt = 0;
    PathType = kptNotFound;
    NatDir = NULL;
    File_1 = NULL;
    File_2 = NULL;

    XFS_CSAN ( File )
    XFS_CAN ( self )
    XFS_CAN ( File )

    if ( ! _GapFilePeerGood ( self ) ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = KDirectoryNativeDir ( & NatDir );
    if ( RCt == 0 ) {
            /*  First we should check if file is complete
             */
        if ( self -> status == kxfsReady ) {
                /*  First we should check if cached file exists
                 *  Open and return it
                 */
            PathType = KDirectoryPathType ( NatDir, self -> cache_path );
            if ( PathType == kptNotFound ) {
                    /*  Second, we should create parent directory if
                     *  it does not exists
                     */
                RCt = _CheckMakeParentDir ( NatDir, self -> cache_path );
                if ( RCt == 0 ) {
                    /*  KNS File first
                     */
#ifdef _USE_CACHED_CONN_
                    RCt = _Take_Burro_From_Pen (
                                                & File_1,
                                                self -> remote_url,
                                                true
                                                );
#else /* _USE_CACHED_CONN_ */
                    RCt = KNSManagerMakeHttpFile (
                                                XFS_KnsManager (),
                                                & File_1,
                                                NULL,
                                                0x01010000,
                                                self -> remote_url
                                                );
#endif /* _USE_CACHED_CONN_ */

                    if ( RCt == 0 ) {
                        RCt = KDirectoryMakeCacheTeePromote (
                                            NatDir,
                                            & File_2,
                                            File_1,
                                            32768 * 4, /* Buffer size */
                                            self -> cache_path
                                            );
                        KFileRelease ( File_1 );
                    }
                }
            }
            else {
                if ( PathType == kptFile ) {
                    self -> status = kxfsComplete;
                }
                else {
                    RCt = XFS_RC ( rcInvalid );
                }
            }

        }

        if ( self -> status == kxfsComplete ) {
            RCt = KDirectoryOpenFileRead (
                                        NatDir,
                                        & File_2,
                                        self -> cache_path
                                        );
        }

        KDirectoryRelease ( NatDir );
    }

    if ( RCt == 0 ) {
            /* We suppose that all files from dbGaP has non ZERO size
             * and that size does not change with time
             */
        if ( self -> size == 0 ) { 
            RCt = KFileSize ( File_2, & ( self -> size ) );
        }
        if ( RCt == 0 ) {
            * File = File_2;
        }
    }

    if ( RCt != 0 ) {
        * File = NULL;

        self -> status = kxfsBroken;
        self -> enc_type = kxfsEncBroken;
        self -> size = 0;
    }

    return RCt;
}   /* _Open_File_1 () */

static
rc_t CC
_CheckEncType ( struct _GapFilePeer * self, const struct KFile * File )
{
    rc_t RCt;
    char BF [ XFS_SIZE_128 ];
    size_t NumRead;

    RCt = 0;
    NumRead = 0;

    XFS_CAN ( self )
    XFS_CAN ( File )

    switch ( self -> enc_type ) {
        case kxfsEncInvalid :
        case kxfsEncBroken :
                return XFS_RC ( rcInvalid );

        case kxfsEncNone :
        case kxfsEncEncFile :
        case kxfsEncSraFile :
        case kxfsEncWGAFile :
                return 0;

        case kxfsEncReady :
                break;

        default :
                return XFS_RC ( rcInvalid );
    }

    RCt = KFileReadAll ( File, 0, BF, sizeof ( BF ), & NumRead );
    if ( RCt != 0 ) {
        self -> enc_type = kxfsEncBroken;
        return XFS_RC ( rcInvalid );
    }

    RCt = KFileIsEnc ( BF, NumRead );
    if ( RCt == 0 ) {
        self -> enc_type = KFileIsSraEnc ( BF, NumRead ) == 0
                                                    ? kxfsEncSraFile
                                                    : kxfsEncEncFile
                                                    ;

    }
    else {
        self -> enc_type = KFileIsWGAEnc ( BF, NumRead ) == 0
                                                    ? kxfsEncWGAFile
                                                    : kxfsEncNone
                                                    ;
    }

    return 0;
}   /* _CheckEncType () */

static
rc_t CC
_Open_File_2 (
            struct _GapFilePeer * self,
            const struct KFile ** DecFile,
            const struct KFile * EncFile
)
{
    rc_t RCt;
    const struct KFile * File_2;
    const struct XFSGapProject * Project;
    const struct KKey * Key;
    const char * Password;
    size_t PasswordSize;

    RCt = 0;
    File_2 = NULL;
    Project = NULL;
    Key = NULL;
    Password = NULL;
    PasswordSize = 0;

    XFS_CSAN ( DecFile )
    XFS_CAN ( self )
    XFS_CAN ( DecFile )
    XFS_CAN ( EncFile )

    if ( ! _GapFilePeerGood ( self ) ) {
        return XFS_RC ( rcInvalid );
    }

    switch ( self -> enc_type ) {
        case kxfsEncNone :
            RCt = KFileAddRef ( EncFile );
            if ( RCt == 0 ) {
                File_2 = EncFile;
            }
            break;

        case kxfsEncEncFile :
            RCt = XFSGapFindOrCreate ( self -> project_id, & Project );
            if ( RCt == 0 ) {
                RCt = XFSGapProjectEncriptionKey ( Project, & Key );
                if ( RCt == 0 ) {
                    RCt = KEncFileMakeRead ( & File_2, EncFile, Key );
                }

                XFSGapProjectRelease ( Project );
            }
            break;

        case kxfsEncSraFile :
                /* Really I do not know what to do here
                 */
            RCt = KFileAddRef ( EncFile );
            if ( RCt == 0 ) {
                File_2 = EncFile;
            }
            break;

        case kxfsEncWGAFile :
            RCt = XFSGapFindOrCreate ( self -> project_id, & Project );
            if ( RCt == 0 ) {
                RCt = XFSGapProjectPassword (
                                            Project,
                                            & Password,
                                            & PasswordSize
                                            );
                if ( RCt == 0 ) {
                    RCt = KFileMakeWGAEncRead (
                                            & File_2,
                                            EncFile,
                                            Password,
                                            PasswordSize
                                            );
                }

                XFSGapProjectRelease ( Project );
            }
            break;

        default :
            RCt = XFS_RC ( rcInvalid );
            break;
    }

    if ( RCt == 0 ) {
        * DecFile = File_2;
    }
    else {
        * DecFile = NULL;

        if ( File_2 != NULL ) {
            KFileRelease ( File_2 );
        }
    }

    return RCt;
}   /* _Open_File_2 () */

/*))
  ||    This method, will return file opened for read.
  ||    Unsynchronized. Uses Peer as input, but also
  ||    modifies Peer's fields too
  ((*/
static
rc_t CC
_GapFilePeerOpen_NoLock (
                    struct _GapFilePeer * self,
                    const struct KFile ** File
)
{
    rc_t RCt;
    const struct KFile * File_1;
    const struct KFile * File_2;
    const struct KFile * File_3;

    RCt = 0;
    File_1 = NULL;
    File_2 = NULL;
    File_3 = NULL;

    XFS_CSAN ( File )
    XFS_CAN ( self )
    XFS_CAN ( File )

        /*  The peer is invalid : something went wrong
         */
    if ( ! _GapFilePeerGood ( self ) ) {
        return XFS_RC ( rcInvalid );
    }

        /*  We do have open file already
         */
    File_1 = self -> file;
    if ( File_1 != NULL ) {
        RCt = KFileAddRef ( File_1 );
        if ( RCt == 0 ) {
            * File = File_1;
        }
        return RCt;
    }

        /*  Three stage process
         *
         *  First we shoud open Http and Tee files or just open file
         *  if it is completed
         *
         *  Check type of file ( NoEnc, Enc, SRA/WGA )
         *
         *  Opening Encoded file
         *
         */
    RCt = _Open_File_1 ( self, & File_1 );
    if ( RCt == 0 ) {
            /* Checking Encoding type */
        RCt = _CheckEncType ( self, File_1 );
        if ( RCt == 0 ) {
            RCt = _Open_File_2 ( self, & File_2, File_1 );
            if ( RCt == 0 ) {
                    /* So, if file already dowloaded and it is not
                     * encoded, we may just return file withou
                     * modifying the peer
                     */
                if ( self -> status == kxfsComplete
                    && self -> enc_type == kxfsEncNone
                ) {
                    * File = File_2;
                }
                else {
                    RCt = _GapFileMake ( & File_3, File_2, self );
                    if ( RCt == 0 ) {
                        * File = File_3;
                    }
                }
            }
        }

        KFileRelease ( File_1 );
    }

    if ( RCt != 0 ) {
        * File = NULL;

        if ( File_2 != NULL ) {
            KFileRelease ( File_2 );
        }

        if ( File_3 != NULL ) {
            KFileRelease ( File_3 );
        }
    }

    return RCt;
}   /* _GapFilePeerOpen_NoLock () */

rc_t CC
_GapFilePeerOpen (
                struct _GapFilePeer * self,
                const struct KFile ** File
)
{
    rc_t RCt;
    const struct KFile * TheFile;

    RCt = 0;
    TheFile = NULL;

    XFS_CSAN ( File )
    XFS_CAN ( self )
    XFS_CAN ( File )

    RCt = KLockAcquire ( self -> mutabor );
    if ( RCt == 0 ) {
        RCt = _GapFilePeerOpen_NoLock ( self, & TheFile );
        if ( RCt == 0 ) {
            * File = TheFile;
        }

        KLockUnlock ( self -> mutabor );
    }

    if ( RCt != 0 ) {
        * File = NULL;

        if ( TheFile != NULL ) {
            KFileRelease ( TheFile );
        }
    }

    return RCt;
}   /* _GapFilePeerOpen () */
