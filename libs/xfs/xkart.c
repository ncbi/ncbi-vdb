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

#include <kfs/directory.h>
#include <kfs/file.h>

#include <kfg/kart.h>

#include "schwarzschraube.h"
#include "xkart.h"
#include "zehr.h"

#include <sysalloc.h>

#include <string.h>

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)) XFSKart and XFSKartItem structs are living here
 ((*/

/*|||\\
  |||// NOTE, these methods are thread non safe, and I will do them 
//|||       thread safe later, if it is necessary.
\\|||*/

struct XFSKart {
    BSTree tree;
    KRefcount refcount;

    const char * path;
};

struct XFSKartItem {
    BSTNode node;
    KRefcount refcount;

    const char * displayname;

    const char * project;
    const char * id;
    const char * accession;
    const char * name;
    const char * description;

        /* We will add status here after
         */
};

/*) Usefuls
 (*/
static const char * _sXFSKart_classname = "XFSKart";
static const char * _sXFSKartItem_classname = "XFSKartItem";

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSKart Methods ...                                               */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
LIB_EXPORT
rc_t CC
XFSKartMake ( const char * Path, struct XFSKart ** Kart )
{
    rc_t RCt;
    struct XFSKart * RetKart;

    RCt = 0;
    RetKart = NULL;

    if ( Kart != NULL ) {
        * Kart = NULL;
    }

    if ( Kart == NULL || Path == NULL ) {
        return XFS_RC ( rcNull );
    }

    RetKart = calloc ( 1, sizeof ( struct XFSKart ) );
    if ( RetKart == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    BSTreeInit ( & ( RetKart -> tree ) );
    KRefcountInit (
                & ( RetKart -> refcount ),
                1,
                _sXFSKart_classname,
                "XFSKartMake",
                "Kart"
                );

    RCt = XFS_StrDup ( Path, & ( RetKart -> path ) );
    if ( RCt == 0 ) {
        RCt = XFSKartReload ( RetKart );
        if ( RCt == 0 ) {
            * Kart = RetKart;
        }
        else {
            XFSKartDispose ( RetKart );
        }
    }

    return RCt;
}   /* XFSKartMake () */

static
void CC
_KartWhackCallback ( BSTNode * Node, void * unused )
{
    if ( Node != NULL ) {
        XFSKartItemRelease ( ( struct XFSKartItem * ) Node );
    }
}   /* _KartWhackCallback () */

LIB_EXPORT
rc_t CC
XFSKartDispose ( struct XFSKart * self )
{
    if ( self != NULL ) {
        if ( self -> path != NULL ) {
            free ( ( char * ) self -> path );
            self -> path = NULL;
        }

        BSTreeWhack ( & ( self -> tree ), _KartWhackCallback, NULL );
        KRefcountWhack ( & ( self -> refcount ), _sXFSKart_classname );

        free ( self );
    }

    return 0;
}   /* XFSKartDispose () */

LIB_EXPORT
rc_t CC
XFSKartAddRef ( struct XFSKart * self )
{
    rc_t RCt;

    RCt = 0;

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    switch ( KRefcountAdd ( & ( self -> refcount ), _sXFSKart_classname ) ) {
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
}   /* XFSKartAddRef () */

LIB_EXPORT
rc_t CC
XFSKartRelease ( struct XFSKart * self )
{
    rc_t RCt;

    RCt = 0;

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    switch ( KRefcountDrop ( & ( self -> refcount ), _sXFSKart_classname ) ) {

        case krefOkay :
        case krefZero :
                    RCt = 0;
                    break;
        case krefWhack :
                    RCt = XFSKartDispose ( ( struct XFSKart * ) self );
                    break;
        case krefNegative :
                    RCt = XFS_RC ( rcInvalid );
                    break;
        default :
                    RCt = XFS_RC ( rcUnknown );
                    break;
    }

    return RCt;
}   /* XFSKartRelease () */

static
rc_t CC
_DisplayName (
            const char * Prj,
            const char * Id,
            const char * Nm,
            const char ** DisplayName
)
{
    rc_t RCt;
    char * TempStr;
    size_t AlloS;
    size_t nwr;

    const char * Prj_S = "pr_";
    const char * Id_S = "id_";
    const char * Sep = ".";

    RCt = 0;
    TempStr = NULL;
    AlloS = 0;
    nwr = 0;

    if ( DisplayName != NULL ) {
        * DisplayName = NULL;
    }

    if ( Prj == NULL || Id == NULL || Nm == NULL || DisplayName == NULL ) {
        return XFS_RC ( rcNull );
    }

        /* Size to allocate */
    AlloS =
        string_size ( Prj ) + string_size ( Prj_S ) + string_size ( Sep )
        + string_size ( Id ) + string_size ( Id_S ) + string_size ( Sep )
        + string_size ( Nm )
        + 1
        ;

    TempStr = calloc ( AlloS, sizeof ( char ) );
    if ( TempStr == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RCt = string_printf (
                        TempStr,
                        AlloS, 
                        & nwr,
                        "%s%s%s%s%s%s%s",
                        Prj_S,
                        Prj,
                        Sep,
                        Id_S,
                        Id,
                        Sep,
                        Nm
                        );
    if ( RCt == 0 ) {
        * DisplayName = TempStr;
    }

    if ( RCt != 0 ) {
        * DisplayName = NULL;

        if ( TempStr != NULL ) {
            free ( TempStr );
        }
    }

    return RCt;
}   /* _DisplayName () */

static
int64_t CC
_AddKartItemCallback ( const BSTNode * N1, const BSTNode * N2 )
{
    return XFS_StringCompare4BST_ZHR (
                ( ( struct XFSKartItem * ) N1 ) -> displayname,
                ( ( struct XFSKartItem * ) N2 ) -> displayname
                );
}   /* _AddKartItemCallback () */

static
rc_t CC
_KartItemDispose ( struct XFSKartItem * self )
{
    if ( self != NULL ) {
        KRefcountWhack (
                    & ( self -> refcount ),
                    _sXFSKartItem_classname
                    ); 
        if ( self -> project != NULL ) {
            free ( ( char * ) self -> project ) ;
            self -> project = NULL;
        }
        if ( self -> id != NULL ) {
            free ( ( char * ) self -> id ) ;
            self -> id = NULL;
        }
        if ( self -> accession != NULL ) {
            free ( ( char * ) self -> accession ) ;
            self -> accession = NULL;
        }
        if ( self -> name != NULL ) {
            free ( ( char * ) self -> name ) ;
            self -> name = NULL;
        }
        if ( self -> description != NULL ) {
            free ( ( char * ) self -> description ) ;
            self -> description = NULL;
        }
        if ( self -> displayname != NULL ) {
            free ( ( char * ) self -> displayname ) ;
            self -> displayname = NULL;
        }

        free ( self );
    }

    return 0;
}   /* _KartItemDispose () */

static
rc_t CC
_AddKartItem ( struct XFSKart * self, const struct KartItem * Item )
{
    rc_t RCt;
    const struct String * TheString;
    struct XFSKartItem * xItem;

    RCt = 0;
    TheString = NULL;
    xItem = NULL;

    if ( self == NULL || Item == NULL ) {
        return XFS_RC ( rcNull );
    }

    xItem = calloc ( 1, sizeof ( struct XFSKartItem ) );
    if ( xItem == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    KRefcountInit (
                & ( xItem -> refcount ),
                1,
                _sXFSKartItem_classname,
                "_AddKartItem",
                "KartItem"
                );

    RCt = KartItemProjId ( Item, & TheString );
    if ( RCt == 0 ) {
        RCt = XFS_SStrDup ( TheString, & xItem -> project );
    }

    if ( RCt == 0 ) {
        RCt = KartItemItemId ( Item, & TheString );
        if ( RCt == 0 ) {
            RCt = XFS_SStrDup ( TheString, & xItem -> id );
        }
    }

    if ( RCt == 0 ) {
        RCt = KartItemAccession ( Item, & TheString );
        if ( RCt == 0 ) {
            RCt = XFS_SStrDup ( TheString, & xItem -> accession );
        }
    }

    if ( RCt == 0 ) {
        RCt = KartItemName ( Item, & TheString );
        if ( RCt == 0 ) {
            RCt = XFS_SStrDup ( TheString, & xItem -> name );
        }
    }

    if ( RCt == 0 ) {
        RCt = KartItemItemDesc ( Item, & TheString );
        if ( RCt == 0 ) {
            RCt = XFS_SStrDup ( TheString, & xItem -> description );
        }
    }

    if ( RCt == 0 ) {
        RCt = _DisplayName (
                        xItem -> project,
                        xItem -> id,
                        ( xItem -> accession == NULL
                                            ? xItem -> name
                                            : xItem -> accession
                        ),
                        & xItem -> displayname
                        );
    }

    if ( RCt == 0 ) {
        RCt = BSTreeInsert (
                        & ( self -> tree ),
                        & ( xItem -> node ),
                        _AddKartItemCallback
                        );
    }

    if ( RCt != 0 ) {
        _KartItemDispose ( xItem );
    }

    return RCt;
}   /* _AddKartItem () */

LIB_EXPORT
rc_t CC
XFSKartReload ( struct XFSKart * self )
{
    rc_t RCt;
    struct Kart * TheKart;
    const struct KartItem * TheKartItem;
    struct KDirectory * Directory;
    bool IsKart;

    RCt = 0;
    TheKart = NULL;
    IsKart = NULL;
    Directory = NULL;

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( self -> path == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = KDirectoryNativeDir ( & Directory );
    if ( RCt == 0 ) {
        RCt = KartMake ( Directory, self -> path, & TheKart, & IsKart );
        if ( RCt == 0 ) {
            if ( ! IsKart ) {
                RCt = XFS_RC ( rcInvalid );
            }
            else {
                while ( true ) {
                    RCt = KartMakeNextItem ( TheKart, & TheKartItem );
                    if ( RCt != 0 ) {   /* Error happened */
                        break;
                    }

                    if ( TheKartItem == NULL ) {    /* no more items */
                        break;
                    }

                    RCt = _AddKartItem ( self, TheKartItem );

                    KartItemRelease ( TheKartItem );

                    if ( RCt != 0 ) {
                        break;
                    }
                }
            }

            if ( TheKart != NULL ) {
                KartRelease ( TheKart );
            }
        }

        KDirectoryRelease ( Directory );
    }

    return RCt;
}   /* XFSKartReload () */

static
void
_ListKartItemsCallback ( BSTNode * Node, void * Data )
{
    struct VNamelist * List;
    struct XFSKartItem * Item;

    List = ( struct VNamelist * ) Data;
    Item = ( struct XFSKartItem * ) Node;

    if ( List != NULL && Item != NULL ) {
        VNamelistAppend ( List, Item -> displayname );
    }
}   /* _ListKartItemsCallback () */

LIB_EXPORT
rc_t CC
XFSKartList ( struct XFSKart * self, struct KNamelist ** List )
{
    rc_t RCt;
    struct VNamelist * xList;

    RCt = 0;
    xList = NULL;

    if ( List != NULL ) {
        * List = NULL;
    }

    if ( self == NULL || List == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = VNamelistMake ( & xList, 16 /* ????? */ );
    if ( RCt == 0 ) {
        BSTreeForEach (
                    & ( self -> tree ),
                    false,
                    _ListKartItemsCallback,
                    xList
                    );

        RCt = VNamelistToNamelist ( xList, List );

        VNamelistRelease ( xList );
    }

    return RCt;
}   /* XFSKartList () */

LIB_EXPORT
bool CC
XFSKartHas ( struct XFSKart * self, const char * ItemName )
{
    return XFSKartGet ( self, ItemName ) != NULL;
}   /* XFSKartHas () */

static
int64 CC
_CartItemCmp ( const void * Item, const BSTNode * Node )
{
    const char * Str1, * Str2;

    Str1 = ( const char * ) Item;

    Str2 = Node == NULL
                ? NULL
                : ( ( struct XFSKartItem * ) Node ) -> displayname
                ;

    if ( Str1 == NULL || Str2 == NULL ) {
        if ( Str1 != NULL ) {
            return 4096;
        }

        if ( Str2 != NULL ) {
            return - 4096;
        }

        return 0;
    }

    return strcmp ( Str1, Str2 );
}   /* _CartItemCmp () */

LIB_EXPORT
const struct XFSKartItem * CC
XFSKartGet ( struct XFSKart * self, const char * ItemName )
{
    const struct XFSKartItem * RetVal = NULL;

    if ( self != NULL && ItemName != NULL ) {
        RetVal = ( const struct XFSKartItem * ) BSTreeFind (
                                                    & ( self -> tree ),
                                                    ItemName,
                                                    _CartItemCmp
                                                    );
    }

    return RetVal;
}   /* XFSKartGet () */

LIB_EXPORT
const char * CC
XFSKartPath ( const struct XFSKart * self )
{
    return self == NULL ? NULL : ( self -> path );
}   /* XFSKartPath () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSKart Methods ...                                               */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
LIB_EXPORT
rc_t CC
XFSKartItemAddRef ( const struct XFSKartItem * self )
{
    rc_t RCt;

    RCt = 0;

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    switch ( KRefcountAdd ( & ( self -> refcount ), _sXFSKartItem_classname ) ) {
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
}   /* XFSKartItemAddRef () */

LIB_EXPORT
rc_t CC
XFSKartItemRelease ( const struct XFSKartItem * self )
{
    rc_t RCt;

    RCt = 0;

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    switch ( KRefcountDrop ( & ( self -> refcount ), _sXFSKartItem_classname ) ) {

        case krefOkay :
        case krefZero :
                    RCt = 0;
                    break;
        case krefWhack :
                    RCt = _KartItemDispose ( ( struct XFSKartItem * ) self );
                    break;
        case krefNegative :
                    RCt = XFS_RC ( rcInvalid );
                    break;
        default :
                    RCt = XFS_RC ( rcUnknown );
                    break;
    }

    return RCt;
}   /* XFSKartItemRelease () */


LIB_EXPORT
const char * CC
XFSKartItemProject ( const struct XFSKartItem * self )
{
    return self == NULL ? NULL : ( self -> project );
}   /* XFSKartItemProject () */

LIB_EXPORT
const char * CC
XFSKartItemId ( const struct XFSKartItem * self )
{
    return self == NULL ? NULL : ( self -> id );
}   /* XFSKartItemId () */

LIB_EXPORT
const char * CC
XFSKartItemAccession ( const struct XFSKartItem * self )
{
    return self == NULL ? NULL : ( self -> accession );
}   /* XFSKartItemAccession () */

LIB_EXPORT
const char * CC
XFSKartItemName ( const struct XFSKartItem * self )
{
    return self == NULL ? NULL : ( self -> name );
}   /* XFSKartItemName () */

LIB_EXPORT
const char * CC
XFSKartItemDescription ( const struct XFSKartItem * self )
{
    return self == NULL ? NULL : ( self -> description );
}   /* XFSKartItemDescription () */

LIB_EXPORT
const char * CC
XFSKartItemDisplayName ( const struct XFSKartItem * self )
{
    return self == NULL ? NULL : ( self -> displayname );
}   /* XFSKartItemProject () */


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
