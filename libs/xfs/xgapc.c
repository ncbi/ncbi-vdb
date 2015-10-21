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

#include <kfg/kart.h>

#include <xfs/path.h>
#include "schwarzschraube.h"
#include "xgap.h"
#include "xgapc.h"
#include "zehr.h"

#include <sysalloc.h>

#include <string.h>

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)) XFSGapCache and XFSGapCacheEntry structs are living here
 ((*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSGapCacheEntry ...                                              */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct XFSGapCacheEntry {
    BSTNode node;
    KRefcount refcount;

    XFSCEType type;
    const char * name;
    const char * path;
    const void * handle;
};

static const char * _sXFSGapCacheEntry_classname = "XFSGapCacheEntry";

static
uint64_t CC
_CachePathVersion ( const char * Path )
{
    struct KDirectory * Dir;
    KTime_t Time;

    Dir = NULL;
    Time = 0;

    if ( Path != NULL ) {
        if ( KDirectoryNativeDir ( & Dir ) == 0 ) {
            if ( KDirectoryDate ( Dir, & Time, Path ) != 0 ) {
                Time = 0;
            }

            KDirectoryRelease ( Dir );
        }
    }

    return Time;
}   /* _CachePathVersion () */

static
XFSCEType CC
_CachePathType ( const char * Path, bool Terminal )
{
    rc_t RCt;
    struct KDirectory * NatDir;
    uint32_t PathType;
    XFSCEType Type;

    RCt = 0;
    NatDir = NULL;
    PathType = kptNotFound;
    Type = kxfsceInvalid;

    if ( Path != NULL ) {
        RCt = KDirectoryNativeDir ( & NatDir );
        if ( RCt == 0 ) {
            PathType = KDirectoryPathType ( NatDir, Path );

            switch ( PathType ) {
                case kptFile:
                    Type = kxfsceFile;
                    break;
                case kptDir:
                    Type = Terminal ? kxfsceDir : kxfsceCache;
                    break;
                default:
                    Type = kxfsceInvalid;
                    break;
            }

            KDirectoryRelease ( NatDir );
        }
    }

printf ( " [CPT] [%d] [%d] [%s] [%d]\n", __LINE__, RCt, Path, Type );
    return RCt == 0 ? Type : kxfsceInvalid;
}   /* _CachePathType () */

static
rc_t CC
_GetPathName ( const char * Path, const char ** Name )
{
    rc_t RCt;
    const struct XFSPath * xPath;

    RCt = 0;
    xPath = NULL;

    XFS_CSAN ( Name )
    XFS_CAN ( Path )
    XFS_CAN ( Name )

    RCt = XFSPathMake ( & xPath, true, Path );
    if ( RCt == 0 ) {
        RCt = XFS_StrDup ( XFSPathName ( xPath ), Name );
printf ( " [-JP-] [%d] [%s] [%s]\n", __LINE__, Path, * Name );

        XFSPathRelease ( xPath );
    }

    if ( RCt != 0 ) {
        if ( * Name != NULL ) {
            free ( ( char * ) * Name );
            * Name = NULL;
        }
    }

    return 0;
}   /* _GetPathName () */

static
rc_t CC
_CacheEntryDispose ( const struct XFSGapCacheEntry * self )
{
    struct XFSGapCacheEntry * Entry = ( struct XFSGapCacheEntry * ) self;
    if ( Entry != NULL ) {
        Entry -> type = kxfsceInvalid;

        KRefcountWhack (
                    & ( Entry -> refcount ),
                    _sXFSGapCacheEntry_classname
                    ); 

        if ( Entry -> handle != NULL ) {
            if ( Entry -> type == kxfsceCache ) {
                if ( Entry -> handle != NULL ) {
                    XFSGapCacheEntryRelease ( self -> handle );
                }
            }
            else {
                Entry -> handle = NULL;
            }
        }

        if ( Entry -> path != NULL ) {
            free ( ( char * ) Entry -> path );
            Entry -> path = NULL;
        }

        if ( Entry -> name != NULL ) {
            free ( ( char * ) Entry -> name );
            Entry -> name = NULL;
        }

        free ( Entry );
    }

    return 0;
}   /* _CacheEntryDispose () */

static
rc_t CC
_CacheEntryMake (
            struct XFSGapCacheEntry ** Entry,
            const char * Path,
            bool Terminal
)
{
    rc_t RCt;
    struct XFSGapCacheEntry * xEntry;
    const struct XFSGapCache * xSubCache;
    XFSCEType Type;

    RCt = 0;
    xEntry = NULL;
    xSubCache = NULL;
    Type = kxfsceInvalid;

    XFS_CSAN ( Entry )
    XFS_CAN ( Entry )
    XFS_CAN ( Path )

    Type = _CachePathType ( Path, Terminal );
printf ( " [EMK] [%d] [%s] [%d]\n", __LINE__, Path, Type );
    if ( Type == kxfsceInvalid || Type == kxfsceBroken ) {
        return XFS_RC ( rcInvalid );
    }

    xEntry = calloc ( 1, sizeof ( struct XFSGapCacheEntry ) );
    if ( xEntry == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    xEntry -> type = Type;

    KRefcountInit (
                & ( xEntry -> refcount ),
                1,
                _sXFSGapCacheEntry_classname,
                "_CacheEntryMake",
                "CacheEntry"
                );

    RCt = XFS_StrDup ( Path, & ( xEntry -> path ) );
    if ( RCt == 0 ) {
        RCt = _GetPathName ( Path, & ( xEntry -> name ) );
        if ( RCt == 0 ) {
            if ( xEntry -> type == kxfsceCache ) {
                RCt = XFSGapCacheMake ( & xSubCache, Path, Terminal );
                if ( RCt == 0 ) {
                    xEntry -> handle = xSubCache;
    
                }
            }
            else {
                xEntry -> handle = NULL;
            }

            if ( RCt == 0 ) {
                * Entry = xEntry;
            }
        }
    }

    if ( RCt == 0 ) {
        * Entry = xEntry;
    }
    else {
        * Entry = NULL;

        if ( xEntry != NULL ) {
            _CacheEntryDispose ( xEntry );
        }
    }

    return RCt;
}   /* _CacheEntryMake () */

LIB_EXPORT
rc_t CC
XFSGapCacheEntryAddRef ( const struct XFSGapCacheEntry * self )
{
    rc_t RCt;

    RCt = 0;

    XFS_CAN ( self )

    switch ( KRefcountAdd (
                        & ( self -> refcount ),
                        _sXFSGapCacheEntry_classname
                        )
    ) {
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
}   /* XFSGapCacheEntryAddRef () */

LIB_EXPORT
rc_t CC
XFSGapCacheEntryRelease ( const struct XFSGapCacheEntry * self )
{
    rc_t RCt;

    RCt = 0;

    XFS_CAN ( self )

    switch ( KRefcountDrop (
                        & ( self -> refcount ),
                        _sXFSGapCacheEntry_classname
                        )
    ) {
        case krefOkay :
        case krefZero :
                    RCt = 0;
                    break;
        case krefWhack :
                    RCt = _CacheEntryDispose ( ( struct XFSGapCacheEntry * ) self );
                    break;
        case krefNegative :
                    RCt = XFS_RC ( rcInvalid );
                    break;
        default :
                    RCt = XFS_RC ( rcUnknown );
                    break;
    }

    return RCt;
}   /* XFSGapCacheEntryRelease () */

LIB_EXPORT
const char * CC
XFSGapCacheEntryName ( const struct XFSGapCacheEntry * self )
{
    return self == NULL ? NULL : ( self -> name );
}   /* XFSGapCacheEntryName () */

LIB_EXPORT
const char * CC
XFSGapCacheEntryPath ( const struct XFSGapCacheEntry * self )
{
    return self == NULL ? NULL : ( self -> path );
}   /* XFSGapCacheEntryPath () */

LIB_EXPORT
XFSCEType CC
XFSGapCacheEntryType ( const struct XFSGapCacheEntry * self )
{
    return self == NULL ? kxfsceInvalid : ( self -> type );
}   /* XFSGapCacheEntryType () */

LIB_EXPORT
const void * CC
XFSGapCacheEntryHandle ( const struct XFSGapCacheEntry * self )
{
    return self == NULL ? NULL : ( self -> handle );
}   /* XFSGapCacheEntryHandle () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* _CacheStub ...                                                    */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct _CacheStub {
    struct BSTree tree;
    KRefcount refcount;

    const char * name;
    const char * path;

    uint64_t version;
    size_t qty;

    bool terminal;
};

static const char * _sCacheStub_classname = "_CacheStub";

static
void CC
_CacheStubWhackCallback ( BSTNode * Node, void * unused )
{
    if ( Node != NULL ) {
        XFSGapCacheEntryRelease ( ( struct XFSGapCacheEntry * ) Node );
    }
}   /* _CacheStubWhackCallback () */

LIB_EXPORT
static
rc_t CC
_CacheStubDispose ( const struct _CacheStub * self )
{
    struct _CacheStub * Stub = ( struct _CacheStub * ) self;

    if ( Stub != NULL ) {
        KRefcountWhack (
                        & ( Stub -> refcount ),
                        _sCacheStub_classname
                        );

        BSTreeWhack (
                    & ( Stub -> tree ),
                    _CacheStubWhackCallback,
                    NULL
                    );

        if ( Stub -> name != NULL ) {
            free ( ( char * ) Stub -> name );
            Stub -> name = NULL;
        }

        if ( Stub -> path != NULL ) {
            free ( ( char * ) Stub -> path );
            Stub -> path = NULL;
        }

        Stub -> version = 0;
        Stub -> qty = 0;
        Stub -> terminal = true;

        free ( Stub );
    }

    return 0;
}   /* _CacheStubDispose () */

static rc_t CC _CacheStubScan ( const struct _CacheStub * self );

static
rc_t CC
_CacheStubMake (
                const struct _CacheStub ** Stub,
                const char * Path,
                bool Terminal
)
{
    rc_t RCt;
    struct _CacheStub * xStub;
    XFSCEType Type;

    RCt = 0;
    xStub = NULL;

    XFS_CSAN ( Stub )
    XFS_CAN ( Stub )
    XFS_CAN ( Path )

    Type = _CachePathType ( Path, Terminal );
printf ( " [SMK] [%d] [%s] [%d]\n", __LINE__, Path, Type );
    if ( Type != kxfsceCache ) {
        return XFS_RC ( rcInvalid );
    }

    xStub = calloc ( 1, sizeof ( struct _CacheStub ) );
    if ( xStub == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    KRefcountInit (
                & ( xStub -> refcount ),
                1,
                _sCacheStub_classname,
                "_CacheStubMake",
                "_CacheStub"
                );
    BSTreeInit ( & ( xStub -> tree ) );

    RCt = XFS_StrDup ( Path, & ( xStub -> path ) );
    if ( RCt == 0 ) {
        RCt = _GetPathName ( Path, & ( xStub -> name ) );
        if ( RCt == 0 ) {
            xStub -> version = 0;
            xStub -> qty = 0;
            xStub -> terminal = Terminal;

            RCt = _CacheStubScan ( xStub );
            if ( RCt == 0 ) {
                * Stub = xStub;
            }
        }
    }

    if ( RCt != 0 ) {
        * Stub = NULL;

        if ( xStub != NULL ) {
            _CacheStubDispose ( xStub );
        }
    }

    return RCt;
}   /* _CacheStubMake () */

static
rc_t CC
_CacheStubAddRef ( const struct _CacheStub * self )
{
    rc_t RCt;

    RCt = 0;

    XFS_CAN ( self )

    switch ( KRefcountAdd (
                        & ( self -> refcount ),
                        _sCacheStub_classname
                        )
    ) {
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
}   /* _CacheStubAddRef () */

static
rc_t CC
_CacheStubRelease ( const struct _CacheStub * self )
{
    rc_t RCt;

    RCt = 0;

    XFS_CAN ( self )

    switch ( KRefcountDrop (
                        & ( self -> refcount ),
                        _sCacheStub_classname
                        )
    ) {
        case krefOkay :
        case krefZero :
                    RCt = 0;
                    break;
        case krefWhack :
                    RCt = _CacheStubDispose ( ( struct _CacheStub * ) self );
                    break;
        case krefNegative :
                    RCt = XFS_RC ( rcInvalid );
                    break;
        default :
                    RCt = XFS_RC ( rcUnknown );
                    break;
    }

    return RCt;
}   /* _CacheStubRelease () */

static
int64_t CC
_AddCacheEntryCallback ( const BSTNode * N1, const BSTNode * N2 )
{
    return XFS_StringCompare4BST_ZHR (
        XFSGapCacheEntryName ( ( struct XFSGapCacheEntry * ) N1 ),
        XFSGapCacheEntryName ( ( struct XFSGapCacheEntry * ) N2 )
        );
}   /* _AddCacheEntryCallback () */

static
rc_t CC
_CacheStubAddEntry (
                const struct _CacheStub * self,
                const char * Name,
                bool Terminal
)
{
    rc_t RCt;
    struct XFSGapCacheEntry * Entry;
    char BB [ XFS_SIZE_4096 ];
    struct KDirectory * NatDir;

    RCt = 0;
    Entry = NULL;
    * BB = 0;
    NatDir = NULL;

    XFS_CAN ( self )
    XFS_CAN ( Name )

    RCt = KDirectoryNativeDir ( & NatDir );
    if ( RCt == 0 ) {
        RCt = KDirectoryResolvePath (
                                    NatDir,
                                    true,
                                    BB,
                                    sizeof ( BB ), 
                                    "%s/%s",
                                    self -> path,
                                    Name
                                    );
        if ( RCt == 0 ) {
printf ( " [AddEn][%d][%s][%d]\n", __LINE__, BB, Terminal );
            // RCt = _CacheEntryMake ( & Entry, BB, false );
            RCt = _CacheEntryMake ( & Entry, BB, Terminal );
            if ( RCt == 0 ) {
                RCt = BSTreeInsert (
                                ( struct BSTree * ) & ( self -> tree ),
                                & ( Entry -> node ),
                                _AddCacheEntryCallback
                                );
                if ( RCt != 0 ) {
                    _CacheEntryDispose ( Entry );
                    Entry = NULL;
                }
                else {
                    ( ( struct _CacheStub * ) self ) -> qty ++;
                }
            }
        }

        KDirectoryRelease ( NatDir );
    }

    return RCt;
}   /* _CacheStubAddEntry () */

static
bool CC
_CacheDirListCallback (
                    const struct KDirectory * self,
                    const char * Name,
                    void * Unused
)
{
    if ( XFS_StrEndsWith ( Name, ".cache" ) ) {
        return false;
    }

    if ( XFS_StrEndsWith ( Name, ".lock" ) ) {
        return false;
    }


    if ( strcmp ( Name, "workspace" ) == 0 ) {
        return false;
    }

    return true;
}   /* _CacheDirListCallback () */

rc_t CC
_CacheStubScan ( const struct _CacheStub * self )
{
    rc_t RCt;
    struct KDirectory * NatDir;
    struct KNamelist * List;
    uint32_t Qty, llp;
    const char * Name;

    RCt = 0;
    NatDir = NULL;
    List = NULL;
    Qty = llp = 0;
    Name = NULL;

    XFS_CAN ( self )
    XFS_CAN ( self -> path )

        /* Getting version */
    ( ( struct _CacheStub * ) self ) -> version =
                                    _CachePathVersion ( self -> path );
    if ( self -> version != 0 ) {
        RCt = KDirectoryNativeDir ( & NatDir );
        if ( RCt == 0 ) {
printf ( " [KDirectoryList] [%d] [%s]\n", __LINE__, self -> path );
            RCt = KDirectoryList (
                                NatDir,
                                & List,
                                _CacheDirListCallback,
                                NULL,
                                self -> path
                                );
            if ( RCt == 0 ) {
                RCt = KNamelistCount ( List, & Qty );
                if ( RCt == 0 ) {
                    for ( llp = 0; llp < Qty; llp ++ ) {
                        RCt = KNamelistGet ( List, llp, & Name );
                        if ( RCt == 0 ) {
printf ( " [KDirectoryList] [%d] [%d] [%s]\n", __LINE__, llp, Name );
                            RCt = _CacheStubAddEntry (
                                                    self,
                                                    Name,
                                                    false
                                                    );
                        }
                        if ( RCt != 0 ) {
                            break;
                        }
                    }
                }
            }

            KDirectoryRelease ( NatDir );
        }
    }

    return RCt;
}   /* _CacheStubScan () */

static
void
_ListCacheEtnriesCallback ( BSTNode * Node, void * Data )
{
    struct VNamelist * List;
    struct XFSGapCacheEntry * Entry;
    const struct XFSGapCache * Cache;
    bool DoAdd;

    List = ( struct VNamelist * ) Data;
    Entry = ( struct XFSGapCacheEntry * ) Node;
    Cache = NULL;
    DoAdd = false;

    if ( List != NULL && Entry != NULL ) {
        DoAdd = true;

        if ( Entry -> type == kxfsceCache ) {
            Cache = ( const struct XFSGapCache * ) Entry -> handle;
            if ( Cache == NULL || XFSGapCacheIsEmpty ( Cache ) ) {
                DoAdd = false;
            }
        }

        if ( DoAdd ) {
            VNamelistAppend ( List, XFSGapCacheEntryName ( Entry ) );
        }
    }
}   /* _ListCacheEtnriesCallback () */

static
rc_t CC
_CacheStubList (
            const struct _CacheStub * self,
            const struct KNamelist ** List
)
{
    rc_t RCt;
    struct VNamelist * xList;

    RCt = 0;
    xList = NULL;

    XFS_CSAN ( List )
    XFS_CAN ( self )
    XFS_CAN ( List )

    RCt = VNamelistMake ( & xList, 16 /* ????? */ );
    if ( RCt == 0 ) {
        BSTreeForEach (
                    & ( self -> tree ),
                    false,
                    _ListCacheEtnriesCallback,
                    xList
                    );

        RCt = VNamelistToConstNamelist ( xList, List );

        VNamelistRelease ( xList );
    }


    RCt = 0;

    return RCt;
}   /* _CacheStubList () */

static
int64_t CC
_CacheStubFindCallback ( const void * Item, const BSTNode * Node )
{
    const char * Str1, * Str2;

    Str1 = ( const char * ) Item;

    Str2 = Node == NULL
        ? NULL
        : XFSGapCacheEntryName ( ( struct XFSGapCacheEntry * ) Node )
        ;

    return XFS_StringCompare4BST_ZHR ( Str1, Str2 );
}   /* _CacheStubFindCallback () */

static
rc_t CC
_CacheStubFind (
                    const struct _CacheStub * self,
                    const struct XFSGapCacheEntry ** Entry,
                    const char * EntryName
)
{
    rc_t RCt;
    const struct XFSGapCacheEntry * xEntry;

    RCt = 0;
    xEntry = NULL;

    if ( self != NULL && EntryName != NULL ) {
        xEntry = ( const struct XFSGapCacheEntry * ) BSTreeFind (
                                                & ( self -> tree ),
                                                EntryName,
                                                _CacheStubFindCallback
                                                );
        if ( xEntry == NULL ) {
            RCt = XFS_RC ( rcNotFound );
        }
        else {
            RCt = XFSGapCacheEntryAddRef ( xEntry );
            if ( RCt == 0 ) {
                * Entry = xEntry;
            }
        }
    }

    return RCt;
}   /* _CacheStubFind () */

static
bool CC
_CacheStubIsEmpty ( const struct _CacheStub * self )
{
    return self == NULL ? true : ( self -> qty == 0 ) ;
}   /* _CacheStubIsEmpty () */

static
uint64_t CC
_CacheStubVersion ( const struct _CacheStub * self )
{
    return self == NULL ? 0 : ( self -> version ) ;
}   /* _CacheStubVersion () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSGapCache ...                                                    */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct XFSGapCache {
    KLock * mutabor;

    const char * path;

    const struct _CacheStub * stub;
};

LIB_EXPORT
rc_t CC XFSGapCacheMake (
                    const struct XFSGapCache ** Cache,
                    const char * Path,
                    bool Terminal
)
{
    rc_t RCt;
    struct XFSGapCache * xCache;
    const struct _CacheStub * Stub;

    RCt = 0;
    xCache = NULL;
    Stub = NULL;

    XFS_CSAN ( Cache )
    XFS_CAN ( Cache )
    XFS_CAN ( Path )

    xCache = calloc ( 1, sizeof ( struct XFSGapCache ) );
    if ( xCache == 0 ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = KLockMake ( & ( xCache -> mutabor ) );
        if ( RCt == 0 ) {
            RCt = XFS_StrDup ( Path, & ( xCache -> path ) );
            if ( RCt == 0 ) {
                RCt = _CacheStubMake ( & Stub, Path, Terminal );
printf ( " [GKM] [%d] [%s] [%d]\n", __LINE__, Path, Terminal );
                if ( RCt == 0 ) {
                    xCache -> stub = Stub;
                }
                else {
                    RCt = 0;
                }

                * Cache = xCache;
            }
        }
    }

    if ( RCt != 0 ) {
        * Cache = NULL;

        if ( xCache != NULL ) {
            XFSGapCacheDispose ( xCache );
        }
    }

    return RCt;
}   /* XFSGapCacheMake () */

LIB_EXPORT
rc_t CC
XFSGapCacheDispose ( const struct XFSGapCache * self )
{
    struct XFSGapCache * Cache = ( struct XFSGapCache * ) self;

    if ( Cache != NULL ) {
        if ( Cache -> mutabor != NULL ) {
            KLockRelease ( Cache -> mutabor );
            Cache -> mutabor = NULL;
        }

        if ( Cache -> stub != NULL ) {
            _CacheStubRelease ( Cache -> stub );
            Cache -> stub = NULL;
        }

        if ( Cache -> path != NULL ) {
            free ( ( char * ) Cache -> path );
            Cache -> path = NULL;
        }

        free ( Cache );
    }

    return 0;
}   /* XFSGapCacheDispose () */

static
rc_t CC
_CacheGetStub (
            const struct XFSGapCache * self,
            const struct _CacheStub ** Stub
)
{
    rc_t RCt;

    RCt = 0;

    XFS_CSAN ( Stub )
    XFS_CAN ( self )
    XFS_CAN ( Stub )

    RCt = KLockAcquire ( self -> mutabor );
    if ( RCt == 0 ) {
        if ( self -> stub != NULL ) {
            RCt = _CacheStubAddRef ( self -> stub );
            if ( RCt == 0 ) {
                * Stub = self -> stub;
            }
        }
        else {
            RCt = XFS_RC ( rcEmpty );
        }

        KLockUnlock ( self -> mutabor );
    }

    return RCt;
}   /* _CacheGetStub () */

LIB_EXPORT
rc_t CC
XFSGapCacheVersion (
                const struct XFSGapCache * self,
                uint64_t * Version
)
{
    rc_t RCt;
    const struct _CacheStub * Stub;

    RCt = 0;
    Stub = NULL;

    XFS_CSA ( Version, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Version )

    RCt = _CacheGetStub ( self, & Stub );
    if ( RCt == 0 ) {
        * Version = _CacheStubVersion ( Stub );

        _CacheStubRelease ( Stub );
    }

    return RCt;
}   /* XFSGapCacheVersion () */

LIB_EXPORT
bool CC
XFSGapCacheIsEmpty ( const struct XFSGapCache * self )
{
    bool RetVal;
    const struct _CacheStub * Stub;

    RetVal = true;
    Stub = NULL;

    XFS_CAN ( self )

    if ( _CacheGetStub ( self, & Stub ) == 0 ) {
        RetVal = _CacheStubIsEmpty ( Stub );

        _CacheStubRelease ( Stub );
    }

    return RetVal;
}   /* XFSGapCacheVersion () */

LIB_EXPORT
rc_t CC
XFSGapCacheList (
                const struct XFSGapCache * self,
                const struct KNamelist ** List
)
{
    rc_t RCt;
    const struct _CacheStub * Stub;

    RCt = 0;
    Stub = NULL;

    XFS_CSAN ( List )
    XFS_CAN ( self )
    XFS_CAN ( List )

    RCt = _CacheGetStub ( self, & Stub );
    if ( RCt == 0 ) {
        RCt = _CacheStubList ( Stub, List );

        _CacheStubRelease ( Stub );
    }
    else {
        if ( GetRCState ( RCt ) == rcEmpty ) {
            RCt = 0;
        }
    }

    return RCt;
}   /* XFSGapCacheList () */

LIB_EXPORT
rc_t CC
XFSGapCacheFind (
                const struct XFSGapCache * self,
                const struct XFSGapCacheEntry ** Entry,
                const char * Name
)
{
    rc_t RCt;
    const struct _CacheStub * Stub;

    RCt = 0;
    Stub = NULL;

    XFS_CSAN ( Entry )
    XFS_CAN ( self )
    XFS_CAN ( Entry )

    RCt = _CacheGetStub ( self, & Stub );
    if ( RCt == 0 ) {
        RCt = _CacheStubFind ( Stub, Entry, Name );

        _CacheStubRelease ( Stub );
    }

    return RCt;
}   /* XFSGapCacheFind () */

static
rc_t CC
_CacheSetStub (
            const struct XFSGapCache * self,
            const struct _CacheStub * Stub
)
{
    rc_t RCt;
    struct XFSGapCache * Cache;

    RCt = 0;
    Cache = ( struct XFSGapCache * ) self;

    XFS_CAN ( Cache )
    XFS_CAN ( Stub )

    RCt = KLockAcquire ( Cache -> mutabor );
    if ( RCt == 0 ) {

        RCt = _CacheStubAddRef ( Stub );
        if ( RCt == 0 ) {
            if  ( Cache -> stub != NULL ) {
                RCt = _CacheStubRelease ( Cache -> stub );
            }

            Cache -> stub = Stub;
        }

        KLockUnlock ( Cache -> mutabor );
    }

    return RCt;
}   /* _CacheSetStub () */

LIB_EXPORT
rc_t CC
XFSGapCacheRefresh ( const struct XFSGapCache * self )
{
    rc_t RCt;
    const struct _CacheStub * Stub;
    uint64_t OldVer;
    uint64_t NewVer;
    const struct KNamelist * List;
    uint32_t Count, llp;
    const char * Name;
    const struct XFSGapCacheEntry * Entry;

    RCt = 0;
    Stub = NULL;
    OldVer = 0;
    NewVer = 0;
    List = NULL;
    Count = llp = 0;
    Name = NULL;
    Entry = NULL;

    XFS_CAN ( self )

printf ( " [RFR] [%d] [%s] [%d]\n", __LINE__, self -> path, false );
    NewVer = _CachePathVersion ( self -> path );
    if ( NewVer != 0 ) {
        RCt = XFSGapCacheVersion ( self, & OldVer );
        if ( RCt == 0 ) {
            if ( OldVer != NewVer ) {
                    /*  Simple way - versions aren't coincide
                     */
                RCt = _CacheStubMake ( & Stub, self -> path, false );
printf ( " [RFR] [%d] [%s] [%d]\n", __LINE__, self -> path, false );
                if ( RCt == 0 ) {
                    RCt = _CacheSetStub ( self, Stub );

                    _CacheStubRelease ( Stub );
                }
            }
            else {
                    /*  Checking sub caches
                     */
                RCt = XFSGapCacheList ( self, & List );
                if ( RCt == 0 ) {
                    RCt = KNamelistCount ( List, & Count );
                    if ( RCt == 0 ) {
                        for ( llp = 0; llp < Count; llp ++ ) {
                            RCt = KNamelistGet ( List, llp, & Name );
                            if ( RCt == 0 ) {
                                RCt = XFSGapCacheFind ( self, & Entry, Name );
                                if ( RCt == 0 ) {
                                    if ( XFSGapCacheEntryType ( Entry ) == kxfsceCache ) {
                                        XFSGapCacheRefresh ( ( const struct XFSGapCache * ) XFSGapCacheEntryHandle ( Entry ) );

                                    }

                                    XFSGapCacheEntryRelease ( Entry );
                                }
                            }

                                /*  There could be errors
                                 */
                            if ( RCt != 0 ) { 
                                RCt = 0;
                            }
                        }
                    }
                }
            }
        }
    }

    return RCt;
}   /* XFSGapCacheRefresh () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
