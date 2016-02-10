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
#include <klib/namelist.h>
#include <klib/refcount.h>
#include <klib/printf.h>
#include <kproc/lock.h>

#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/tar.h>

#include <xfs/path.h>

#include "schwarzschraube.h"
#include "xtar.h"
#include "zehr.h"
#include "lockdpt.h"

#include <sysalloc.h>

#include <ctype.h>
#include <time.h>

/*||*\
  || Lyrics:
  || That file contains interface to TAR archive
  || Tar archive is readed by using KDirectoryOpenTarArchive from
  || kfs/tar.h
  ||
\*||*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)) XFSTar and XFSTarEntry and _TarDpt ( TarDepot ) structs are
 ||  living here
((*/

/*))
 //  EntryDepot this is responsible for sharing Tar resources
((*/
struct _TarDpt {
    BSTree tree;

    KLock * mutabor;
};

/*)) 
 //  Path to a file and related KDirectory
((*/
struct XFSTar {
    BSTNode node;

    KRefcount refcount;
    KLock * mutabor;

    const char * source;
    const struct KDirectory * dir;

    XFSStatus status;
};

/*)) 
 //  Just a path to entry in TAR archive
((*/
struct XFSTarEntry {
    KRefcount refcount; /* ??? */

    const struct XFSTar * tar;    /* Way we go */

    const char * name;
    const char * path;

    bool is_folder;

    XFSStatus status;

    const struct KFile * file;
};

/*) Usefuls
 (*/
static const char * _sXFSTar_classname = "XFSTar";
static const char * _sXFSTarEntry_classname = "XFSTarEntry";

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* _TarDpt Methods ...                                               */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static struct _TarDpt * _sTarDpt = NULL;

static
struct _TarDpt * CC
_Dpt ()
{
    return _sTarDpt;
}   /* _Dpt () */

static
void CC
_TarDptWhackCallback ( BSTNode * Node, void * unused )
{
    if ( Node != NULL ) {
        XFSTarDispose ( ( struct XFSTar * ) Node );
    }
}   /* _TarDptWhackCallback () */

static
rc_t CC
_TarDptDisposeImpl ( struct _TarDpt * self )
{
    rc_t RCt;

    RCt = 0;

    if ( self != NULL ) { 
        BSTreeWhack (
                    & ( self -> tree ),
                    _TarDptWhackCallback,
                    NULL
                    );

        if ( self -> mutabor != NULL ) {
            KLockRelease ( self -> mutabor );
            self -> mutabor = NULL;
        }

        free ( self );
    }

    return RCt;
}   /* _TarDptDisposeImpl () */

LIB_EXPORT
rc_t CC
XFSTarDepotDispose ()
{
    struct _TarDpt * TarDpt;

    TarDpt = _sTarDpt;
    _sTarDpt = NULL;

    if ( TarDpt != NULL ) {
        _TarDptDisposeImpl ( TarDpt );
    }

    return 0;
}   /* XFSTarDepotDispose () */

static
rc_t CC
_TarDptMakeImpl ( struct _TarDpt ** TarDpt )
{
    rc_t RCt;
    struct _TarDpt * TheD;

    RCt = 0;
    TheD = NULL;

    if ( TarDpt != NULL ) {
        * TarDpt = NULL;
    }

    if ( TarDpt == NULL ) {
        return XFS_RC ( rcNull );
    }

    TheD = calloc ( 1, sizeof ( struct _TarDpt ) );
    if ( TheD == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RCt = KLockMake ( & ( TheD -> mutabor ) );
    if ( RCt == 0 ) {
        BSTreeInit ( & ( TheD -> tree ) );

        * TarDpt = TheD;
    }

    if ( RCt != 0 ) {
        * TarDpt = NULL;
        _TarDptDisposeImpl ( TheD );
    }

    return RCt;
}   /* _TarDptMakeImpl () */

LIB_EXPORT
rc_t CC
XFSTarDepotInit ()
{
    rc_t RCt;
    struct _TarDpt * TarDpt;

    RCt = 0;
    TarDpt = NULL;

    if ( _sTarDpt == NULL ) {

        RCt = _TarDptMakeImpl ( & TarDpt );
        if ( RCt == 0 ) {
            if ( _sTarDpt == NULL ) {
                _sTarDpt = TarDpt;
            }
            else {
                _TarDptDisposeImpl ( TarDpt );
            }
        }
    }

    return RCt;
}   /* XFSTarDepotInit () */

static
int64_t CC
_TarCmpCallback ( const void * Item, const BSTNode * Node )
{
    const char * Str1, * Str2;

    Str1 = ( const char * ) Item;

    Str2 = Node == NULL
                ? NULL
                : ( ( struct XFSTar * ) Node ) -> source
                ;
    return XFS_StringCompare4BST_ZHR ( Str1, Str2 );
}   /* _TarCmpCallback () */

static
rc_t CC
_TarDptGetNoLock (
                struct _TarDpt * self,
                const char * Source,
                const struct XFSTar ** Tar
)
{
    const struct XFSTar * RetVal = NULL;

    if ( Tar != NULL ) {
        * Tar = NULL;
    }

    if ( self == NULL || Source == NULL || Tar == NULL ) {
        return XFS_RC ( rcNull );
    }

    RetVal = ( const struct XFSTar * ) BSTreeFind (
                                                & ( self -> tree ),
                                                Source,
                                                _TarCmpCallback
                                                );


    * Tar = RetVal;

    return 0;
}   /* _TarDptGetNoLock () */

static
int64_t CC
_TarAddCallback ( const BSTNode * N1, const BSTNode * N2 )
{
    return XFS_StringCompare4BST_ZHR (
            ( ( struct XFSTar * ) N1 ) -> source,
            ( ( struct XFSTar * ) N2 ) -> source
            );
}   /* _TarAddCallback () */

static
rc_t CC
_TarDptAddNoLock ( struct _TarDpt * self, const struct XFSTar * Tar )
{
    rc_t RCt;
    const struct XFSTar * TheTar;
    const char * Source;

    RCt = 0;
    TheTar = NULL;
    Source = NULL;

    if ( self == NULL || Tar == NULL ) {
        return XFS_RC ( rcNull );
    }

    Source = Tar -> source;

    RCt = _TarDptGetNoLock ( self, Source, & TheTar );
    if ( RCt == 0 ) {
        if ( TheTar == NULL ) {
            RCt = BSTreeInsert (
                        & ( self -> tree ),
                        ( struct BSTNode * ) & ( Tar -> node ),
                        _TarAddCallback
                        );
            if ( RCt != 0 ) {
                XFSTarRelease ( Tar );
            }
        }
    }

    return RCt;
}   /* _TarDptAddNoLock () */

static
rc_t CC
_TarDptDelNoLock ( struct _TarDpt * self, const struct XFSTar * Tar )
{
    if ( self == NULL || Tar == NULL ) {
        return XFS_RC ( rcNull );
    }

    BSTreeUnlink ( & ( self -> tree ), ( struct BSTNode * ) & ( Tar -> node ) );
    XFSTarDispose ( Tar );

    return 0;
}   /* _TarDptDelNoLock () */

static
rc_t CC
_TarDptClearNoLock ( struct _TarDpt * self )
{
    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    BSTreeWhack ( & ( self -> tree ), _TarDptWhackCallback, NULL );
    BSTreeInit ( & ( self -> tree ) );

    return 0;
}   /* _TarDptClearNoLock () */

LIB_EXPORT
rc_t CC
XFSTarDepotClear ()
{
    rc_t RCt;
    struct _TarDpt * D;

    RCt = 0;
    D = _Dpt ();

    if ( D == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = KLockAcquire ( D -> mutabor );
    if ( RCt == 0 ) {
        RCt = _TarDptClearNoLock ( D );

        KLockUnlock ( D -> mutabor );
    }

    return RCt;
}   /* XFSTarDepotClear () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSTar Methods ...                                                */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
static
rc_t CC
_TarOpen ( const struct XFSTar * self )
{
    rc_t RCt;
    struct XFSTar * Tar;
    const struct KDirectory * Dir;
    struct KDirectory * NativeDir;

    RCt = 0;
    Dir = NativeDir = NULL;
    Tar = ( struct XFSTar * ) self;

    if ( Tar == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = KLockAcquire ( Tar -> mutabor );
    if ( RCt == 0 ) { 
        if ( Tar -> status != kxfsReady ) {
            RCt = XFS_RC ( rcInvalid );
        }
        else {
            if ( Tar -> dir == NULL ) {
                RCt = KDirectoryNativeDir ( & NativeDir );
                if ( RCt == 0 ) {
                    RCt = KDirectoryOpenTarArchiveRead_silent (
                                                        NativeDir,
                                                        & Dir,
                                                        1,
                                                        Tar -> source
                                                        );
                    if ( RCt == 0 ) {
                        Tar -> dir = Dir;
                        Tar -> status = kxfsGood;
                    }
                    else {
                        Tar -> status = kxfsBroken;
                    }

                    KDirectoryRelease ( NativeDir );
                }
            }
        }

        KLockUnlock ( Tar -> mutabor );
    }

    return RCt;
}   /* _TarOpen () */

static
rc_t CC
_TarClose ( const struct XFSTar * self )
{
    rc_t RCt;
    struct XFSTar * Tar;

    RCt = 0;
    Tar = ( struct XFSTar * ) self;

    if ( Tar == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = KLockAcquire ( Tar -> mutabor );
    if ( RCt == 0 ) {
        if ( Tar -> status == kxfsGood
                || Tar -> status == kxfsReady /* shouldn't happen */
        ) {
            if ( Tar -> dir != NULL ) {
                KDirectoryRelease ( Tar -> dir );

                Tar -> dir = NULL;
                Tar -> status = kxfsReady;
            }
        }
        else {
            RCt = XFS_RC ( rcInvalid );
        }

        KLockUnlock ( Tar -> mutabor );
    }

    return RCt;
}   /* _TarClose () */

LIB_EXPORT
rc_t CC
XFSTarDispose ( const struct XFSTar * self )
{
    struct XFSTar * Tar = ( struct XFSTar * ) self;

    if ( Tar != NULL ) {
        Tar -> status = kxfsInvalid;

        if ( Tar -> source != NULL ) {
            free ( ( char * ) Tar -> source );

            Tar -> source = NULL;
        }

        if ( Tar -> dir != NULL ) {
            KDirectoryRelease ( Tar -> dir );

            Tar -> dir = NULL;
        }

        if ( Tar -> mutabor != NULL ) {
            KLockRelease ( Tar -> mutabor );

            Tar -> mutabor = NULL;
        }

        KRefcountWhack ( & ( Tar -> refcount ), _sXFSTar_classname );

        free ( Tar );
    }

    return 0;
}   /* XFSTarDispose () */

static
rc_t CC
_TarAddRef ( const struct XFSTar * self )
{
    rc_t RCt;
    struct XFSTar * Tar;

    RCt = 0;
    Tar = ( struct XFSTar * ) self;

    if ( Tar == NULL ) {
        return XFS_RC ( rcNull );
    }

    switch ( KRefcountAdd ( & ( Tar -> refcount ), _sXFSTar_classname ) ) {
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
}   /* _TarAddRef () */

LIB_EXPORT
rc_t CC
XFSTarRelease ( const struct XFSTar * self )
{
    rc_t RCt;
    struct XFSTar * Tar;

    RCt = 0;
    Tar = ( struct XFSTar * ) self;

    if ( Tar == NULL ) {
        return XFS_RC ( rcNull );
    }

    switch ( KRefcountDrop ( & ( Tar -> refcount ), _sXFSTar_classname ) ) {

        case krefOkay :
        case krefZero :
                    RCt = 0;
                    break;
        case krefWhack :
                    RCt = _TarClose ( ( struct XFSTar * ) Tar );
                    break;
        case krefNegative :
                    RCt = XFS_RC ( rcInvalid );
                    break;
        default :
                    RCt = XFS_RC ( rcUnknown );
                    break;
    }

    return RCt;
}   /* XFSTarRelease () */

static
rc_t CC
_TarMake ( const char * Resource, const struct XFSTar ** Tar )
{
    rc_t RCt;
    struct XFSTar * RetTar;

    RCt = 0;
    RetTar = NULL;

    if ( Tar != NULL ) {
        * Tar = NULL;
    }

    if ( Tar == NULL || Resource == NULL ) {
        return XFS_RC ( rcNull );
    }

    RetTar = calloc ( 1, sizeof ( struct XFSTar ) );
    if ( RetTar == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RetTar -> status = kxfsInvalid;

    RCt = KLockMake ( & ( RetTar -> mutabor ) );
    if ( RCt == 0 ) {
        KRefcountInit (
                    & ( RetTar -> refcount ),
                    1,
                    _sXFSTar_classname,
                    "_TarMake",
                    "Tar"
                    );

        RCt = XFS_StrDup ( Resource, & ( RetTar -> source ) );
        if ( RCt == 0 ) {
            RetTar -> status = kxfsReady;

            * Tar = RetTar;
        }
    }

    if ( RCt != 0 ) {
        * Tar = NULL;

        XFSTarDispose ( RetTar );
    }

    return RCt;
}   /* _TarMake () */

static
rc_t CC
_TarDptFindOrCreateNoLock (
                    struct _TarDpt * self,
                    const char * Resource,
                    const struct XFSTar ** Tar
)
{
    rc_t RCt;
    const struct XFSTar * TheTar;

    RCt = 0;
    TheTar = NULL;

    if ( Tar != NULL ) {
        * Tar = NULL;
    }

    if ( self == NULL || Resource == NULL || Tar == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = _TarDptGetNoLock ( self, Resource, & TheTar );
    if ( RCt == 0 ) {
        if ( TheTar != NULL ) {
            RCt = _TarAddRef ( TheTar );
        }
        else {
            RCt = _TarMake ( Resource, & TheTar );
        }

    }

    if ( RCt == 0 ) {
        RCt = _TarDptAddNoLock ( self, TheTar );
        if ( RCt == 0 ) {
            * Tar = TheTar;
        }
    }

    if ( RCt != 0 ) {
        * Tar = NULL;

        if ( TheTar != NULL ) {
            _TarDptDelNoLock ( self, TheTar );
        }
    }

    return RCt;
}   /* _TarDptFindOrCreateNoLock () */

LIB_EXPORT
rc_t CC
XFSTarFindOrCreate ( const char * Resource, const struct XFSTar ** Tar )
{
    rc_t RCt;
    struct _TarDpt * Dpt;
    const struct XFSTar * TheTar;

    RCt = 0;
    TheTar = NULL;
    Dpt = _Dpt ();

    if ( Tar != NULL ) {
        * Tar = NULL;
    }

    if ( Dpt == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Resource == NULL || Tar == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = KLockAcquire ( Dpt -> mutabor );
    if ( RCt == 0 ) {
        RCt = _TarDptFindOrCreateNoLock ( Dpt, Resource, & TheTar );

        KLockUnlock ( Dpt -> mutabor );
    }

        /* Here we opens TAR directory, it will use another lock
         */
    if ( RCt == 0 ) {
        RCt = _TarOpen ( TheTar );
        if ( RCt == 0 ) {
            * Tar = TheTar;
        }
        else {
            XFSTarRelease ( TheTar );
        }
    }

    return RCt;
}   /* XFSTarFindOrCreate () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSTarEntry Make/Dispose Methods ...                             */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
static
rc_t CC
_TarEntryDispose ( struct XFSTarEntry * self )
{
    if ( self != NULL ) {
        self -> status = kxfsInvalid;

        if ( XFSTarEntryIsOpen ( self ) ) {
            XFSTarEntryClose ( self );
        }

        if ( self -> tar != NULL ) {
            XFSTarRelease ( self -> tar );
            self -> tar = NULL;
        }

        KRefcountWhack (
                    & ( self -> refcount ),
                    _sXFSTarEntry_classname
                    ); 

        if ( self -> name != NULL ) {
            free ( ( char * ) self -> name ) ;

            self -> name = NULL;
        }

        if ( self -> path != NULL ) {
            free ( ( char * ) self -> path ) ;

            self -> path = NULL;
        }

        self -> is_folder = false;

        free ( self );
    }

    return 0;
}   /* _TarEntryDispose () */

static
rc_t CC
_TarCreateEntry (
            const struct XFSTar * self,
            const char * Path,
            const struct XFSTarEntry ** Entry
)
{
    rc_t RCt;
    struct XFSTarEntry * RetEntry;
    size_t PathSize;
    uint32_t PathType;

    RCt = 0;
    RetEntry = NULL;
    PathSize = 0;
    PathType = kptNotFound;

    if ( Entry != NULL ) {
        * Entry = NULL;
    }

    if ( self == NULL || Path == NULL || Entry == NULL ) {
        return XFS_RC ( rcNull );
    }

    PathSize = string_size ( Path );
    if ( PathSize == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    RetEntry = calloc ( 1, sizeof ( struct XFSTarEntry ) );
    if ( RetEntry == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RetEntry -> status = kxfsReady;

    RCt = _TarAddRef ( self ); 
    if ( RCt == 0 ) {
        RetEntry -> tar = self;

        RCt = XFS_StrDup ( Path, & ( RetEntry -> path ) );
        if ( RCt == 0 ) {
            RCt = XFS_NameFromPath_ZHR ( Path, & ( RetEntry -> name ), false );
            if ( RCt == 0 ) {
                KRefcountInit (
                            & ( RetEntry -> refcount ),
                            1,
                            _sXFSTarEntry_classname,
                            "_TarCreateEntry",
                            "TarEntry"
                            );
                    /* Here we are going to check if it directory 
                     */
                PathType = KDirectoryPathType ( self -> dir, "./%s", Path );
                switch ( PathType ) {
                    case kptFile :
                        RetEntry -> is_folder = false;
                        break;
                    case kptDir :
                        RetEntry -> is_folder = true;
                        break;
                    default :
                        RetEntry -> status = kxfsBroken;
                        RCt = XFS_RC ( rcInvalid );
                        break;
                }
                if ( RCt == 0 ) {
                    RetEntry -> status = kxfsGood;

                    * Entry = RetEntry;
                }
            }
        }
    }

    if ( RCt != 0 ) {
        * Entry = NULL;

        if ( RetEntry != NULL ) {
            _TarEntryDispose ( RetEntry );
        }
    }

    return RCt;
}   /* _TarCreateEntry () */

LIB_EXPORT
rc_t CC
XFSTarEntryAddRef ( const struct XFSTarEntry * self )
{
    rc_t RCt;

    RCt = 0;

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    switch ( KRefcountAdd ( & ( self -> refcount ), _sXFSTarEntry_classname ) ) {
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
}   /* XFSTarEntryAddRef () */

LIB_EXPORT
rc_t CC
XFSTarEntryRelease ( const struct XFSTarEntry * self )
{
    rc_t RCt;

    RCt = 0;

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    switch ( KRefcountDrop ( & ( self -> refcount ), _sXFSTarEntry_classname ) ) {

        case krefOkay :
        case krefZero :
                    RCt = 0;
                    break;
        case krefWhack :
                    RCt = _TarEntryDispose ( ( struct XFSTarEntry * ) self );
                    break;
        case krefNegative :
                    RCt = XFS_RC ( rcInvalid );
                    break;
        default :
                    RCt = XFS_RC ( rcUnknown );
                    break;
    }

    return RCt;
}   /* XFSTarEntryRelease () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSTar Methods ...                                               */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

LIB_EXPORT
const char * CC
XFSTarSource ( const struct XFSTar * self )
{
    if ( self != NULL ) {
        return self -> source;
    }

    return NULL;
}   /* XFSTarBaseUrl () */

LIB_EXPORT
rc_t CC
XFSTarGetEntry (
            const struct XFSTar * self,
            const char * Path,
            const struct XFSTarEntry ** Entry
)
{
    return _TarCreateEntry ( self, Path, Entry );
}   /* XFSTarGetEntry () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSTarEntry Methods ...                                          */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
LIB_EXPORT
bool CC
XFSTarEntryIsFolder ( const struct XFSTarEntry * self )
{
    return self == NULL ? false : ( self -> is_folder );
}   /* XFSTarEntryIsFolder () */

LIB_EXPORT
const char * CC
XFSTarEntryName ( const struct XFSTarEntry * self )
{
    return self == NULL ? NULL : ( self -> name );
}   /* XFSTarEntryName () */

LIB_EXPORT
const char * CC
XFSTarEntryPath ( const struct XFSTarEntry * self )
{
    return self == NULL ? NULL : ( self -> path );
}   /* XFSTarEntryPath () */

LIB_EXPORT
rc_t CC
XFSTarEntryList (
                const struct XFSTarEntry * self,
                struct KNamelist ** List
)
{
    rc_t RCt;

    RCt = 0;

    if ( List != NULL ) {
        * List = NULL;
    }

    if ( self == NULL || List == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( ! self -> is_folder || ! ( self -> status == kxfsGood ) ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = KLockAcquire ( self -> tar -> mutabor );
    if ( RCt == 0 ) {
        RCt = KDirectoryList (
                            self -> tar -> dir,
                            List,
                            NULL,
                            NULL,
                            "./%s",
                            self -> path
                            );
        KLockUnlock ( self -> tar -> mutabor );
    }

    return RCt;
}   /* XFSTarEntryList () */

LIB_EXPORT
rc_t CC
XFSTarEntrySize ( const struct XFSTarEntry * self, uint64_t * Size )
{
    rc_t RCt = 0;

    if ( Size != NULL ) {
        * Size = 0;
    }

    if ( self == NULL || Size == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( self -> status != kxfsGood ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = KLockAcquire ( self -> tar -> mutabor );
    if ( RCt == 0 ) {
        if ( ! self -> is_folder ) {
            RCt = KDirectoryFileSize (
                                    self -> tar -> dir,
                                    Size,
                                    "./%s",
                                    self -> path
                                    );
        }

        KLockUnlock ( self -> tar -> mutabor );
    }

    return RCt;
}   /* XFSTarEntrySize () */

LIB_EXPORT
rc_t CC
XFSTarEntryTime ( const struct XFSTarEntry * self, KTime_t * Time )
{
    rc_t RCt = 0;

    if ( Time != NULL ) {
        * Time = 0;
    }

    if ( self == NULL || Time == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( self -> status != kxfsGood ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = KLockAcquire ( self -> tar -> mutabor );
    if ( RCt == 0 ) {
        RCt = KDirectoryDate (
                            self -> tar -> dir,
                            Time,
                            "./%s",
                            self -> path
                            );
        KLockUnlock ( self -> tar -> mutabor );
    }

    return RCt;
}   /* XFSTarEntryTime () */

LIB_EXPORT
bool CC
XFSTarEntryGood ( const struct XFSTarEntry * self )
{
    bool RV = false;

    if ( self != NULL ) {
        if ( KLockAcquire ( self -> tar -> mutabor ) == 0 ) {
            RV = self -> status == kxfsGood;

            KLockUnlock ( self -> tar -> mutabor );
        }
    }
    return RV;
}   /* XFSTarEntryGood () */

LIB_EXPORT
rc_t CC
XFSTarEntryGetChild (
                const struct XFSTarEntry * self,
                const char * ChildName,
                const struct XFSTarEntry ** Child
)
{
    rc_t RCt;
    char BF [ XFS_SIZE_1024 ];
    size_t NumW;

    RCt = 0;
    * BF = 0;
    NumW = 0;

    if ( Child != NULL ) {
        * Child = NULL;
    }

    if ( self == NULL || ChildName == NULL || Child == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = string_printf (
                        BF,
                        sizeof ( BF ),
                        & NumW,
                        "%s/%s",
                        self -> path,
                        ChildName
                        );
    if ( RCt == 0 ) {
        RCt = XFSTarGetEntry ( self -> tar, BF, Child );
    }

    return RCt;
}   /* XFSTarEntryGetChild () */

LIB_EXPORT
bool CC
XFSTarEntryIsOpen ( const struct XFSTarEntry * self ) 
{
    bool RetVal = false;

    if ( self != NULL ) {
        if ( KLockAcquire ( self -> tar -> mutabor ) == 0 ) {
            RetVal = self -> file != NULL;

            KLockUnlock ( self -> tar -> mutabor );
        }
    }
    return RetVal;
}   /* XFSTarEntryIsOpen () */

LIB_EXPORT
rc_t CC
XFSTarEntryOpen ( const struct XFSTarEntry * self )
{
    rc_t RCt;
    struct XFSTarEntry * Entry;

    RCt = 0;
    Entry = ( struct XFSTarEntry * ) self;

    if ( Entry == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( Entry -> status != kxfsGood || Entry -> is_folder == true ) {
        RCt = XFS_RC ( rcInvalid );
    }

    RCt = KLockAcquire ( Entry -> tar -> mutabor );
    if ( RCt == 0 ) {
        RCt = KDirectoryOpenFileRead (
                                Entry -> tar -> dir,
                                & ( Entry -> file ),
                                "./%s",
                                Entry -> path
                                );
        if ( RCt != 0 ) {
            Entry -> status = kxfsBroken;
        }

        KLockUnlock ( Entry -> tar -> mutabor );
    }

    return RCt;
}   /* XFSTarEntryOpent () */

LIB_EXPORT
rc_t CC
XFSTarEntryRead (
                const struct XFSTarEntry * self,
                uint64_t Offset,
                void * Buffer,
                size_t BufferSize,
                size_t * NumRead
)
{
    rc_t RCt = 0;

    if ( self == NULL || Buffer == NULL || NumRead == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( self -> file == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( BufferSize == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = KLockAcquire ( self -> tar -> mutabor );
    if ( RCt == 0 ) {
        RCt = KFileRead (
                        self -> file,
                        Offset,
                        Buffer,
                        BufferSize,
                        NumRead
                        );
        KLockUnlock ( self -> tar -> mutabor );
    }

    return RCt;
}   /* XFSTarEntryRead () */

LIB_EXPORT
rc_t CC
XFSTarEntryClose ( const struct XFSTarEntry * self )
{
    rc_t RCt;
    struct XFSTarEntry * Entry;

    RCt = 0;
    Entry = ( struct XFSTarEntry * ) self;

    if ( Entry == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( Entry -> file == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = KLockAcquire ( Entry -> tar -> mutabor );
    if ( RCt == 0 ) {
        KFileRelease ( Entry -> file );
        Entry -> file = NULL;

        KLockUnlock ( Entry -> tar -> mutabor );
    }

    return RCt;
}   /* XFSTarEntryClose () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

