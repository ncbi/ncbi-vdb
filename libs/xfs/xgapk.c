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

#include "schwarzschraube.h"
#include "xgap.h"
#include "xgapk.h"
#include "zehr.h"

#include <sysalloc.h>

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)) XFSGapKart and XFSGapKartItem structs are living here
 ((*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSGapKartItem ...                                                */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct XFSGapKartItem {
    BSTNode node;
    KRefcount refcount;

    uint32_t project_id;
    uint32_t object_id;

    const char * accession;
    const char * name;
    const char * description;
};

static const char * _sXFSGapKartItem_classname = "XFSGapKartItem";

static
rc_t CC
_KartItemDispose ( struct XFSGapKartItem * self )
{
    if ( self != NULL ) {
        KRefcountWhack (
                    & ( self -> refcount ),
                    _sXFSGapKartItem_classname
                    ); 
        self -> project_id = 0;
        self -> object_id = 0;

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

        free ( self );
    }

    return 0;
}   /* _KartItemDispose () */

static
rc_t CC
_KartItemMake (
            struct XFSGapKartItem ** RetItem,
            const struct KartItem * Item
)
{
    rc_t RCt;
    const struct String * TheString;
    struct XFSGapKartItem * xItem;
    uint64_t ProjectId, ObjectId;

    RCt = 0;
    TheString = NULL;
    xItem = NULL;
    ProjectId = ObjectId = 0;

    XFS_CSAN ( RetItem )
    XFS_CAN ( RetItem )
    XFS_CAN ( Item )

    xItem = calloc ( 1, sizeof ( struct XFSGapKartItem ) );
    if ( xItem == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    KRefcountInit (
                & ( xItem -> refcount ),
                1,
                _sXFSGapKartItem_classname,
                "_KartItemMake",
                "KartItem"
                );

    RCt = KartItemProjIdNumber ( Item, & ProjectId );
    if ( RCt == 0 ) {
        xItem -> project_id = ( uint32_t ) ProjectId;
    }

    if ( RCt == 0 ) {
        RCt = KartItemItemId ( Item, & TheString );
        if ( RCt == 0 && TheString -> len != 0 ) {
            RCt = KartItemItemIdNumber ( Item, & ObjectId );
            if ( RCt == 0 ) {
                xItem -> object_id = ( uint32_t ) ObjectId;
            }
        }
        else {
            xItem -> object_id = 0;
        }
    }

    if ( RCt == 0 ) {
        RCt = KartItemAccession ( Item, & TheString );
        if ( RCt == 0 && TheString -> len != 0 ) {
            RCt = XFS_SStrDup ( TheString, & xItem -> accession );
        }
        else {
            xItem -> accession = NULL;
        }
    }

    if ( RCt == 0 ) {
        RCt = KartItemName ( Item, & TheString );
        if ( RCt == 0 && TheString -> len != 0 ) {
            RCt = XFS_SStrDup ( TheString, & xItem -> name );
        }
        else {
            xItem -> name = NULL;
        }
    }

    if ( RCt == 0 ) {
        RCt = KartItemItemDesc ( Item, & TheString );
        if ( RCt == 0 && TheString -> len != 0 ) {
            RCt = XFS_SStrDup ( TheString, & xItem -> description );
        }
        else {
            xItem -> description = NULL;
        }
    }

    if ( xItem -> object_id == 0 && xItem -> accession == NULL ) {
        RCt = XFS_RC ( rcInvalid );
    }

    if ( RCt == 0 ) {
        * RetItem = xItem;
    }
    else {
        if ( xItem != NULL ) {
            _KartItemDispose ( xItem );
        }
    }

    return RCt;
}   /* _KartItemMake () */

LIB_EXPORT
rc_t CC
XFSGapKartItemAddRef ( const struct XFSGapKartItem * self )
{
    rc_t RCt;

    RCt = 0;

    XFS_CAN ( self )

    switch ( KRefcountAdd ( & ( self -> refcount ), _sXFSGapKartItem_classname ) ) {
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
}   /* XFSGapKartItemAddRef () */

LIB_EXPORT
rc_t CC
XFSGapKartItemRelease ( const struct XFSGapKartItem * self )
{
    rc_t RCt;

    RCt = 0;

    XFS_CAN ( self )

    switch ( KRefcountDrop ( & ( self -> refcount ), _sXFSGapKartItem_classname ) ) {

        case krefOkay :
        case krefZero :
                    RCt = 0;
                    break;
        case krefWhack :
                    RCt = _KartItemDispose ( ( struct XFSGapKartItem * ) self );
                    break;
        case krefNegative :
                    RCt = XFS_RC ( rcInvalid );
                    break;
        default :
                    RCt = XFS_RC ( rcUnknown );
                    break;
    }

    return RCt;
}   /* XFSGapKartItemRelease () */

LIB_EXPORT
uint32_t CC
XFSGapKartItemProjectId ( const struct XFSGapKartItem * self )
{
    return self == NULL ? 0 : self -> project_id;
}   /* XFSGapKartItemProjectId () */

LIB_EXPORT
uint32_t CC
XFSGapKartItemObjectId ( const struct XFSGapKartItem * self )
{
    return self == NULL ? 0 : self -> object_id;
}   /* XFSGapKartItemObjectId () */

LIB_EXPORT
const char * CC
XFSGapKartItemAccession ( const struct XFSGapKartItem * self )
{
    return self == NULL ? NULL : ( self -> accession );
}   /* XFSGapKartItemAccession () */

LIB_EXPORT
const char * CC
XFSGapKartItemName ( const struct XFSGapKartItem * self )
{
    return self == NULL ? NULL : ( self -> name );
}   /* XFSGapKartItemName () */

LIB_EXPORT
const char * CC
XFSGapKartItemDescription ( const struct XFSGapKartItem * self )
{
    return self == NULL ? NULL : ( self -> description );
}   /* XFSGapKartItemDescription () */

LIB_EXPORT
const char * CC
XFSGapKartItemDisplayName ( const struct XFSGapKartItem * self )
{
    if ( self != NULL ) {
        return self -> accession == NULL
                                    ? self -> name
                                    : self -> accession
                                    ;
    }
    return NULL;
}   /* XFSGapKartItemProject () */


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))))
 ((((   This is very stupid and under developed structure, which
  ))))  is very un-effictive.
 ((((   Kart could contain items, which belong to different
  ))))  projects. I need fast way to check if cart contains items
 ((((   which belong to some particular project.
  ))))  Good thing, Kart could contain items for not more than two
 ((((   or three projects so, we could use bubble search.
  ))))  There is structure _LaIn ( LAme INdex )
 ((((*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* _LaIn                                                           */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct _LaIn {
    uint32_t q;     /* items qty */
    uint32_t c;     /* array capasity */
    uint32_t * a;   /* array */
};

static
rc_t CC
_LaInMake ( const struct _LaIn ** LaIn )
{
    struct _LaIn * Ret = NULL;

    XFS_CSAN ( LaIn )
    XFS_CAN ( LaIn )

    Ret = calloc ( 1, sizeof ( struct _LaIn ) );
    if ( Ret == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    * LaIn = Ret;

    return 0;
}   /* _LaInMake () */

static
rc_t CC
_LaInDispose ( const struct _LaIn * self )
{
    struct _LaIn * LaIn = ( struct _LaIn * ) self;

    if ( LaIn != NULL ) {
        if ( LaIn -> a != NULL ) {
            free ( LaIn -> a );
            LaIn -> a = NULL;
        }

        LaIn -> q = 0;
        LaIn -> c = 0;

        free ( LaIn );
    }

    return 0;
}   /* _LaInDispose () */

static
rc_t CC
_LaInRealloc ( struct _LaIn * self, uint32_t Qty )
{
    rc_t RCt;
    uint32_t * NewA;
    uint32_t NewC;

#define POPOTAN 4

    RCt = 0;
    NewA = NULL;
    NewC = 0;

    XFS_CAN ( self )

    if ( Qty < self -> c ) {
        return 0;
    }

    NewC = ( Qty / POPOTAN + 1 ) * POPOTAN;

    NewA = malloc ( sizeof ( uint32_t ) * NewC );
    if ( NewA == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    if ( self -> a != NULL ) {
        if ( self -> q != 0 ) {
            memmove ( NewA, self -> a, sizeof ( uint32_t ) * self -> q );
        }

        free ( self -> a );
    }

    self -> a = NewA;
    self -> c = NewC;

#undef POPOTAN

    return RCt;
}   /* _LaInRealloc () */

static
bool CC
_LaInHas ( const struct _LaIn * self, uint32_t Value )
{
    uint32_t llp = 0;

    if ( self != NULL ) {
        for ( llp = 0; llp < self -> q; llp ++ ) {
            if ( Value == * ( self -> a + llp ) ) {
                return true;
            }
        }
    }

    return false;
}   /* _LaInHas () */

static
rc_t CC
_LaInAdd ( const struct _LaIn * self, uint32_t Value )
{
    rc_t RCt;
    struct _LaIn * LaIn;

    RCt = 0;
    LaIn = ( struct _LaIn * ) self;

    XFS_CAN ( LaIn )

    if ( ! _LaInHas ( LaIn, Value ) ) {
            
        RCt = _LaInRealloc ( LaIn, LaIn -> q + 1 );
        if ( RCt == 0 ) {

            LaIn -> a [ LaIn -> q ] = Value;
            LaIn -> q ++;
        }
    }

    return RCt;
}   /* _LaInAdd () */

#ifdef JOJOBA
/*))  Unnecessary in real life
 ((*/
static
rc_t CC
_LaInQty ( const struct _LaIn * self, uint32_t * Qty )
{
    XFS_CSA ( Qty, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Qty )

    * Qty = self -> q;

    return 0;
}   /* _LaInQty () */

/*))  Unnecessary in real life
 ((*/
static
rc_t CC
_LaInGet (
            const struct _LaIn * self,
            uint32_t Index,
            uint32_t * Value
)
{
    XFS_CSA ( Value, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Value )

    if ( Index < self -> q ) {
        * Value = self -> a [ Index ];
        return 0;
    }

    return XFS_RC ( rcInvalid );
}   /* _LaInGet () */
#endif /* JOJOBA */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSGapKart ...                                                    */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct XFSGapKart {
    BSTNode node;

    BSTree tree;
    KRefcount refcount;

    const char * path;
    const char * name;

    const struct _LaIn * lain;
};

static const char * _sXFSGapKart_classname = "XFSGapKart";

static rc_t CC _GapKartLoad ( struct XFSGapKart * self );

static
rc_t CC
_KartPath (
            const char * KartPath,
            const char * KartName,
            char * Buffer,
            size_t BufferSize
)
{
    rc_t RCt;
    size_t NumW;

    RCt = 0;
    NumW = 0;

    XFS_CAN ( KartPath )
    XFS_CAN ( KartName )
    XFS_CAN ( Buffer )

    RCt = string_printf (
                        Buffer,
                        BufferSize,
                        & NumW,
                        "%s/%s",
                        KartPath,
                        KartName
                        );
    if ( RCt != 0 ) {
        * Buffer = 0;
    }

    return RCt;
}   /* _KartPath () */

LIB_EXPORT
rc_t CC
XFSGapKartMake (
                const struct XFSGapKart ** Kart,
                const char * Path,
                const char * Name
)
{
    rc_t RCt;
    struct XFSGapKart * RetKart;

    RCt = 0;
    RetKart = NULL;

    XFS_CSAN ( Kart )
    XFS_CAN ( Kart )
    XFS_CAN ( Path )

    RetKart = calloc ( 1, sizeof ( struct XFSGapKart ) );
    if ( RetKart == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    BSTreeInit ( & ( RetKart -> tree ) );
    KRefcountInit (
                & ( RetKart -> refcount ),
                1,
                _sXFSGapKart_classname,
                "XFSGapKartMake",
                "Kart"
                );

    RCt = _LaInMake ( & ( RetKart -> lain ) );
    if ( RCt == 0 ) {
        RCt = XFS_StrDup ( Path, & ( RetKart -> path ) );
        if ( RCt == 0 ) {
            RCt = XFS_StrDup ( Name, & ( RetKart -> name ) );
            if ( RCt == 0 ) {
                RCt = _GapKartLoad ( RetKart );
                if ( RCt == 0 ) {
                    * Kart = RetKart;
                }
                else {
                    XFSGapKartDispose ( RetKart );
                }
            }
        }
    }

    return RCt;
}   /* XFSGapKartMake () */

static
void CC
_KartWhackCallback ( BSTNode * Node, void * unused )
{
    if ( Node != NULL ) {
        XFSGapKartItemRelease ( ( struct XFSGapKartItem * ) Node );
    }
}   /* _KartWhackCallback () */

LIB_EXPORT
rc_t CC
XFSGapKartDispose ( const struct XFSGapKart * self )
{
    struct XFSGapKart * Kart = ( struct XFSGapKart * ) self;

    if ( Kart != NULL ) {
        if ( Kart -> lain != NULL ) {
            _LaInDispose ( Kart -> lain );
            Kart -> lain = NULL;
        }

        if ( Kart -> name != NULL ) {
            free ( ( char * ) Kart -> name );
            Kart -> name = NULL;
        }

        if ( Kart -> path != NULL ) {
            free ( ( char * ) Kart -> path );
            Kart -> path = NULL;
        }

        BSTreeWhack ( & ( Kart -> tree ), _KartWhackCallback, NULL );
        KRefcountWhack ( & ( Kart -> refcount ), _sXFSGapKart_classname );

        free ( Kart );
    }

    return 0;
}   /* XFSGapKartDispose () */

LIB_EXPORT
rc_t CC
XFSGapKartAddRef ( const struct XFSGapKart * self )
{
    rc_t RCt;

    RCt = 0;

    XFS_CAN ( self )

    switch ( KRefcountAdd ( & ( self -> refcount ), _sXFSGapKart_classname ) ) {
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
}   /* XFSGapKartAddRef () */

LIB_EXPORT
rc_t CC
XFSGapKartRelease ( const struct XFSGapKart * self )
{
    rc_t RCt;

    RCt = 0;

    XFS_CAN ( self )

    switch ( KRefcountDrop ( & ( self -> refcount ), _sXFSGapKart_classname ) ) {

        case krefOkay :
        case krefZero :
                    RCt = 0;
                    break;
        case krefWhack :
                    RCt = XFSGapKartDispose ( ( struct XFSGapKart * ) self );
                    break;
        case krefNegative :
                    RCt = XFS_RC ( rcInvalid );
                    break;
        default :
                    RCt = XFS_RC ( rcUnknown );
                    break;
    }

    return RCt;
}   /* XFSGapKartRelease () */

static
int64_t CC
_AddKartItemCallback ( const BSTNode * N1, const BSTNode * N2 )
{
    return XFS_StringCompare4BST_ZHR (
        XFSGapKartItemDisplayName ( ( struct XFSGapKartItem * ) N1 ),
        XFSGapKartItemDisplayName ( ( struct XFSGapKartItem * ) N2 )
        );
}   /* _AddKartItemCallback () */

static
rc_t CC
_AddKartItem ( struct XFSGapKart * self, const struct KartItem * Item )
{
    rc_t RCt;
    struct XFSGapKartItem * xItem;

    XFS_CAN ( self )
    XFS_CAN ( Item )

    RCt = _KartItemMake ( & xItem, Item );
    if ( RCt == 0 ) {
        RCt = _LaInAdd (
                        self -> lain,
                        XFSGapKartItemProjectId ( xItem )
                        );
        if ( RCt == 0 ) {
            RCt = BSTreeInsert (
                            & ( self -> tree ),
                            & ( xItem -> node ),
                            _AddKartItemCallback
                            );
        }
    }

    if ( RCt != 0 ) {
        if ( xItem != NULL ) {
            _KartItemDispose ( xItem );
        }
    }

    return RCt;
}   /* _AddKartItem () */

rc_t CC
_GapKartLoad ( struct XFSGapKart * self )
{
    rc_t RCt;
    struct Kart * TheKart;
    const struct KartItem * TheKartItem;
    struct KDirectory * Directory;
    bool IsKart;
    char BF [ XFS_SIZE_1024 ];

    RCt = 0;
    TheKart = NULL;
    IsKart = false;
    Directory = NULL;
    * BF = 0;

    XFS_CAN ( self )
    XFS_CAN ( self -> path )

    RCt = _KartPath ( self -> path, self -> name , BF, sizeof ( BF ) );
    if ( RCt == 0 ) {
        RCt = KDirectoryNativeDir ( & Directory );
        if ( RCt == 0 ) {
            RCt = KartMake ( Directory, BF, & TheKart, & IsKart );
            if ( RCt == 0 ) {
                if ( ! IsKart ) {
                    RCt = XFS_RC ( rcInvalid );
                }
                else {
                    while ( true ) {
                        RCt = KartMakeNextItem (
                                                TheKart,
                                                & TheKartItem
                                                );
                        if ( RCt != 0 ) {   /* Error happened */
                            break;
                        }

                        if ( TheKartItem == NULL ) { /* no more items */
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
    }

    return RCt;
}   /* _GapKartLoad () */

struct _LollyPopp {
    struct VNamelist * list;
    uint32_t project_id;
};

static
void
_ListKartItemsCallback ( BSTNode * Node, void * Data )
{
    struct _LollyPopp * Popp;
    struct XFSGapKartItem * Item;

    Popp = ( struct _LollyPopp * ) Data;
    Item = ( struct XFSGapKartItem * ) Node;

    if ( Popp -> list != NULL && Item != NULL ) {
        if ( Popp -> project_id == 0 || 
            Popp -> project_id == XFSGapKartItemProjectId ( Item )
        ) {
            VNamelistAppend (
                            Popp -> list,
                            XFSGapKartItemDisplayName ( Item )
                            );
        }
    }
}   /* _ListKartItemsCallback () */

LIB_EXPORT
rc_t CC
XFSGapKartList (
                const struct XFSGapKart * self,
                struct KNamelist ** List,
                uint32_t ProjectId
)
{
    rc_t RCt;
    struct VNamelist * xList;
    struct _LollyPopp Popp;

    RCt = 0;
    xList = NULL;
    Popp . list = NULL;
    Popp . project_id = 0;

    XFS_CSAN ( List )
    XFS_CAN ( self )
    XFS_CAN ( List )

    RCt = VNamelistMake ( & xList, 16 /* ????? */ );
    if ( RCt == 0 ) {
        if ( XFSGapKartHasForProject ( self, ProjectId ) ) {
            Popp . list = xList;
            Popp . project_id = ProjectId;

            BSTreeForEach (
                        & ( self -> tree ),
                        false,
                        _ListKartItemsCallback,
                        & Popp
                        );
        }

        RCt = VNamelistToNamelist ( xList, List );

        VNamelistRelease ( xList );
    }

    return RCt;
}   /* XFSGapKartList () */

static
int64_t CC
_KartItemCmp ( const void * Item, const BSTNode * Node )
{
    const char * Str1, * Str2;

    Str1 = ( const char * ) Item;

    Str2 = Node == NULL
        ? NULL
        : XFSGapKartItemDisplayName ( ( struct XFSGapKartItem * ) Node )
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
}   /* _KartItemCmp () */

LIB_EXPORT
const struct XFSGapKartItem * CC
XFSGapKartGet ( const struct XFSGapKart * self, const char * ItemName )
{
    const struct XFSGapKartItem * RetVal = NULL;

    if ( self != NULL && ItemName != NULL ) {
        RetVal = ( const struct XFSGapKartItem * ) BSTreeFind (
                                                    & ( self -> tree ),
                                                    ItemName,
                                                    _KartItemCmp
                                                    );
    }

    return RetVal;
}   /* XFSGapKartGet () */

LIB_EXPORT
const char * CC
XFSGapKartPath ( const struct XFSGapKart * self )
{
    return self == NULL ? NULL : ( self -> path );
}   /* XFSGapKartPath () */

LIB_EXPORT
bool CC
XFSGapKartHasForProject (
                    const struct XFSGapKart * self,
                    uint32_t ProjectId
)
{
    if ( self != NULL ) {
            /*  If ProjectId == 0 we should return everything
             */
        return ProjectId == 0
                            ? true
                            : _LaInHas ( self -> lain, ProjectId )
                            ;
    }

    return false;
}   /* XFSGapKartHasForProject () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* _Kartophoby ...                                                  */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

    /*  Because that object changes only on the creation time,
     *  we should not use any mutexes, and it is thread safe
     */
struct _Kartophoby {
    BSTree tree;
    KRefcount refcount;

    const char * path;
    uint64_t version;
};

static const char * _sKartophoby_classname = "_Kartophoby";

static
void CC
_KartophobyWhackCallback ( BSTNode * Node, void * unused )
{
    if ( Node != NULL ) {
        XFSGapKartRelease ( ( struct XFSGapKart * ) Node );
    }
}   /* _KartophobyWhackCallback () */

static
rc_t CC
_KartophobyDispose ( const struct _Kartophoby * self )
{
    struct _Kartophoby * Phob = ( struct _Kartophoby * ) self;

    if ( Phob != NULL ) {
        BSTreeWhack (
                    & ( Phob -> tree ),
                    _KartophobyWhackCallback,
                    NULL
                    );

        KRefcountWhack (
            & ( Phob -> refcount ),
            _sKartophoby_classname
            );

        if ( Phob -> path != NULL ) {
            free ( ( char * ) Phob -> path );
            Phob -> path = NULL;
        }

        Phob -> version = 0;

        free ( Phob );
    }

    return 0;
}   /* _KartophobyDispose () */

static rc_t CC _KartophobyReadVersion (
                                    struct _Kartophoby * self,
                                    struct KNamelist ** List,
                                    const char * Path
                                    );

static rc_t CC _KartophobyAddKart (
                                    struct _Kartophoby * Phoby,
                                    const char * KartName,
                                    const struct _Kartophoby * LoadFrom
                                    );

static rc_t CC _KartophobyGet (
                                    const struct _Kartophoby * self,
                                    const struct XFSGapKart ** Kart,
                                    const char * KartName
                                    );

static
rc_t CC
_KartophobyMake (
                const struct _Kartophoby ** Phoby,
                const char * Path,
                const struct _Kartophoby * MakeFrom /* Could be NULL */
)
{
    rc_t RCt;
    struct _Kartophoby * ThePhoby;
    struct KNamelist * List;
    uint32_t Count, llp;
    const char * Name;

    RCt = 0;
    ThePhoby = NULL;
    List = NULL;
    Count = llp = 0;
    Name = NULL;

    XFS_CSAN ( Phoby )
    XFS_CAN ( Phoby )
    XFS_CAN ( Path )

        /* Creating Phoby is quite tricky :D
         */
    ThePhoby = calloc ( 1, sizeof ( struct _Kartophoby  ) );
    if ( ThePhoby == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    KRefcountInit (
                & ( ThePhoby -> refcount ),
                1,
                _sKartophoby_classname,
                "_KartophobyMake",
                "_Kartophoby"
                );

        /*  First we should check that path is valid and
         *  read valid version for our phoby and list of Karts
         */
    RCt = _KartophobyReadVersion ( ThePhoby, & List, Path );
    if ( RCt == 0 ) {
        RCt = KNamelistCount ( List, & Count );
        if ( RCt == 0 ) {
            for ( llp = 0; llp < Count; llp ++ ) {
                RCt = KNamelistGet ( List, llp, & Name );
                if ( RCt == 0 ) {
                    RCt = _KartophobyAddKart (
                                            ThePhoby,
                                            Name,
                                            MakeFrom
                                            );
                    if ( RCt != 0 ) {
                        RCt = 0;
                    }
                }
            }
        }

        KNamelistRelease ( List );
    }

    if ( RCt != 0 ) {
        * Phoby = NULL;

        if ( ThePhoby != NULL ) {
            _KartophobyDispose ( ThePhoby );
        }
    }

    if ( RCt == 0 ) {
        * Phoby = ThePhoby;
    }

    return RCt;
}   /* _KartophobyMake () */

static
rc_t CC
_KartophobyAddRef ( const struct _Kartophoby * self )
{
    rc_t RCt;

    RCt = 0;

    XFS_CAN ( self )

    switch ( KRefcountAdd ( & ( self -> refcount ), _sKartophoby_classname ) ) {
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
}   /* _KartophobyAddRef () */

LIB_EXPORT
rc_t CC
_KartophobyRelease ( const struct _Kartophoby * self )
{
    rc_t RCt;

    RCt = 0;

    XFS_CAN ( self )

    switch ( KRefcountDrop ( & ( self -> refcount ), _sKartophoby_classname ) ) {

        case krefOkay :
        case krefZero :
                    RCt = 0;
                    break;
        case krefWhack :
                    RCt = _KartophobyDispose ( self );
                    break;
        case krefNegative :
                    RCt = XFS_RC ( rcInvalid );
                    break;
        default :
                    RCt = XFS_RC ( rcUnknown );
                    break;
    }

    return RCt;
}   /* _KartophobyRelease () */

/*  Check and read directory version ( time ) and return List
 *  if it is not NULL
 */
static
rc_t CC
_VersionAndListForPath (
                        const char * Path,
                        uint64_t * Version,
                        struct KNamelist ** List
)
{
    rc_t RCt;
    struct KDirectory * NatDir;
    KTime_t Date;
    uint32_t PathType;

    RCt = 0;
    NatDir = NULL;
    Date = 0;
    PathType = kptNotFound;

    XFS_CSAN ( List )
    XFS_CSA ( Version, 0 )
    XFS_CAN ( Path )
    XFS_CAN ( Version )

    RCt = KDirectoryNativeDir ( & NatDir );
    if ( RCt == 0 ) {
        PathType = KDirectoryPathType ( NatDir, Path );
        switch ( PathType ) {
            case kptNotFound:
                RCt = KDirectoryCreateDir (
                                            NatDir,
                                            0755,
                                            kcmCreate,
                                            Path
                                            );
                break;
            case kptDir:
                break;
            default:
                RCt = XFS_RC ( rcInvalid );
        }

        if ( RCt == 0 ) {
            RCt = KDirectoryDate ( NatDir, & Date, Path );
            if ( RCt == 0 ) {
                * Version = Date;

                if ( List != NULL ) {
                    RCt = KDirectoryList (
                                        NatDir,
                                        List,
                                        NULL,
                                        NULL,
                                        Path
                                        );
                }
            }
        }

        KDirectoryRelease ( NatDir );
    }

    return RCt;
}   /* _VersionAndListForPath () */

rc_t CC
_KartophobyReadVersion (
                        struct _Kartophoby * self,
                        struct KNamelist ** List,
                        const char * Path
)
{
    rc_t RCt = 0;

    XFS_CSAN ( List )
    XFS_CAN ( self )
    XFS_CAN ( List )

    RCt = XFS_StrDup ( Path, & ( self -> path ) );
    if ( RCt == 0 ) {
        RCt = _VersionAndListForPath (
                                    self -> path,
                                    & ( self -> version),
                                    List
                                    );
    }

    return RCt;
}   /* _KartophobyReadVersion () */

static
int64_t CC
_KartophobyAddKartCallback ( const BSTNode * N1, const BSTNode * N2 )
{
    return XFS_StringCompare4BST_ZHR (
                                ( ( struct XFSGapKart * ) N1 ) -> name,
                                ( ( struct XFSGapKart * ) N2 ) -> name
                                );
}   /* _KartophobyAddKartCallback () */

rc_t CC
_KartophobyAddKart (
                    struct _Kartophoby * self,
                    const char * KartName,
                    const struct _Kartophoby * LoadFrom
)
{
    rc_t RCt;
    const struct XFSGapKart * Kart;

    RCt = 0;
    Kart = NULL;

    XFS_CAN ( self )
    XFS_CAN ( KartName )

        /*)     First we should check if we already have and use it
         (*/
    RCt = _KartophobyGet ( self, & Kart, KartName );
    if ( RCt != 0 ) {
            /*)     We should load kart if that exists
             (*/
        RCt = XFSGapKartMake ( & Kart, self -> path, KartName );
    }

    if ( RCt == 0 ) {
            /*) Here we should add Kart to new Kartophoby
             (*/
        RCt = BSTreeInsert (
                        & ( self -> tree ),
                        & ( ( ( struct XFSGapKart * ) Kart ) -> node ),
                        _KartophobyAddKartCallback
                        );
        if ( RCt != 0 ) {
            XFSGapKartRelease ( Kart );
        }
    }

    return RCt;
}   /* _KartophobyAddKart () */

static
int64_t CC
_KartophobyGetCallback ( const void * Item, const BSTNode * Node )
{
    const char * Str1, * Str2;

    Str1 = ( ( char * ) Item );
    Str2 = Node == NULL
                    ? NULL
                    : ( ( struct XFSGapKart * ) Node ) -> name
                    ;


    return XFS_StringCompare4BST_ZHR ( Str1, Str2 );
}   /* _KartophobyGetCallback () */

rc_t CC
_KartophobyGet (
                const struct _Kartophoby * self,
                const struct XFSGapKart ** Kart,
                const char * KartName
)
{
    rc_t RCt;
    const struct XFSGapKart * TheKart;

    RCt = 0;
    TheKart = NULL;

    XFS_CSAN ( Kart )
    XFS_CAN ( self )
    XFS_CAN ( Kart )
    XFS_CAN ( KartName )

    TheKart = ( const struct XFSGapKart * ) BSTreeFind (
                                            & ( self -> tree ),
                                            KartName,
                                            _KartophobyGetCallback
                                            );
    if ( TheKart != NULL ) {
        RCt = XFSGapKartAddRef ( TheKart );
        if ( RCt == 0 ) {
            * Kart = TheKart;
        }
    }
    else {
        RCt = XFS_RC ( rcNotFound );
    }

    return RCt;
}   /* _KartophobyGet () */

rc_t CC
_KartophobyVersion (
                const struct _Kartophoby * self,
                uint64_t * Version
)
{
    XFS_CSA ( Version, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Version )

    * Version = self -> version;

    return 0;
}   /* _KartophobyVersion () */

static
void
_ListKartophobyCallback ( BSTNode * Node, void * Data )
{
    struct _LollyPopp * Popp;
    struct XFSGapKart * Kart;

    Popp = ( struct _LollyPopp * ) Data;
    Kart = ( struct XFSGapKart * ) Node;

    if ( Popp -> list != NULL && Kart != NULL ) {
        if ( Popp -> project_id == 0 ||
            XFSGapKartHasForProject ( Kart, Popp -> project_id )
        ) {
            VNamelistAppend ( Popp -> list, Kart -> name );
        }
    }
}   /* _ListKartophobyCallback () */

static
rc_t CC
_KartophobyList (
                const struct _Kartophoby * self,
                struct KNamelist ** List,
                uint32_t ProjectId
)
{
    rc_t RCt;
    struct VNamelist * xList;
    struct _LollyPopp Popp;

    RCt = 0;
    xList = NULL;
    Popp . list = NULL;
    Popp . project_id = 0;

    XFS_CSAN ( List )
    XFS_CAN ( self )
    XFS_CAN ( List )

    RCt = VNamelistMake ( & xList, 16 /* ????? */ );
    if ( RCt == 0 ) {
        Popp . list = xList;
        Popp . project_id = ProjectId;

        BSTreeForEach (
                        & ( self -> tree ),
                        false,
                        _ListKartophobyCallback,
                        & Popp
                        );

        RCt = VNamelistToNamelist ( xList, List );

        VNamelistRelease ( xList );
    }

    return RCt;
}   /* _KartophobyList () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* _KartDepot ...                                                    */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct _KartDepot {
    KLock * mutabor;

    char * path;

    const struct _Kartophoby * phoby;
};

static struct _KartDepot * _sDpt = NULL;

static rc_t CC _KartDepotPhoby ( const struct _Kartophoby ** Phoby );
static rc_t CC _KartDepotPhobySet ( const struct _Kartophoby * Phoby );

static
rc_t CC
_KartDepotDispose ( struct _KartDepot * self )
{
    if ( self != NULL ) {
        if ( self -> phoby != NULL ) {
            _KartophobyRelease ( self -> phoby );
            self -> phoby = NULL;
        }

        if ( self -> path != NULL ) {
            free ( self -> path );
            self -> path = NULL;
        }

        if ( self -> mutabor != NULL ) {
            KLockRelease ( self -> mutabor );
            self -> mutabor = NULL;
        }

        free ( self );
    }

    return 0;
}   /* _KartDepotDispose () */

static
rc_t CC
_KartDepotMake ( struct _KartDepot ** Dpt )
{
    rc_t RCt;
    struct _KartDepot * Depot;

    RCt = 0;
    Depot = NULL;

    XFS_CSAN ( Dpt )
    XFS_CAN ( Dpt )

    Depot = calloc ( 1, sizeof ( struct _KartDepot ) );
    if ( Depot == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = KLockMake ( & ( Depot -> mutabor ) );
        if ( RCt == 0 ) {

            RCt = XFSGapKartfiles ( & ( Depot -> path ) );
            if ( RCt == 0 ) {
                RCt = _KartophobyMake ( 
                                    & ( Depot -> phoby ),
                                    Depot -> path,
                                    NULL
                                    );
                if ( RCt == 0 ) {
                    * Dpt = Depot;
                }
            }
        }
    }

    if ( RCt != 0 ) {
        * Dpt = NULL;

        if ( Depot != NULL ) {
            _KartDepotDispose ( Depot );
        }
    }

    return RCt;
}   /* _KartDepotMake () */

static
struct _KartDepot * CC
_KartDepotGet ()
{
    return _sDpt;
}   /* _KartDepotGet () */

LIB_EXPORT
rc_t CC
XFSGapKartDepotInit ()
{
    rc_t RCt;
    struct _KartDepot * Depot;

    RCt = 0;
    Depot = NULL;

    if ( _sDpt == NULL ) {
        RCt = _KartDepotMake ( & Depot );
        if ( RCt == 0 ) {
            _sDpt = Depot;
        }
    }

    return RCt;
}   /* XFSGapKartDepotInit () */

LIB_EXPORT
rc_t CC
XFSGapKartDepotDispose ()
{
    struct _KartDepot * Depot = _sDpt;

    _sDpt = NULL;

    if ( Depot != NULL ) {
        _KartDepotDispose ( Depot );
    }

    return 0;
}   /* XFSGapKartDepotDispose () */

LIB_EXPORT
rc_t CC
XFSGapKartDepotList ( struct KNamelist ** List, uint32_t ProjectId )
{
    rc_t RCt;
    const struct _Kartophoby * Phoby;

    RCt = 0;
    Phoby = NULL;

    XFS_CSAN ( List )
    XFS_CAN ( List )

    RCt = _KartDepotPhoby ( & Phoby );
    if ( RCt == 0 ) {
        RCt = _KartophobyList ( Phoby, List, ProjectId );

        if ( Phoby != NULL ) {
            _KartophobyRelease ( Phoby );
        }
    }

    return RCt;
}   /* XFSGapKartDepotList () */

LIB_EXPORT
rc_t CC
XFSGapKartDepotGet (
                const struct XFSGapKart ** Kart,
                const char * KartName
)
{
    rc_t RCt;
    const struct _Kartophoby * Phoby;

    RCt = 0;
    Phoby = NULL;

    XFS_CSAN ( Kart )
    XFS_CAN ( Kart )
    XFS_CAN ( KartName )

    RCt = _KartDepotPhoby ( & Phoby );
    if ( RCt == 0 ) {
        RCt = _KartophobyGet ( Phoby, Kart, KartName );

        if ( Phoby != NULL ) {
            _KartophobyRelease ( Phoby );
        }
    }

    return RCt;
}   /* XFSGapKartDepotGet () */

LIB_EXPORT
rc_t CC
XFSGapKartDepotRefresh ()
{
    rc_t RCt;
    struct _KartDepot * Dpt;
    uint64_t OldVer;
    uint64_t NewVer;
    const struct _Kartophoby * NewPhoby;
    const struct _Kartophoby * OldPhoby;

    RCt = 0;
    Dpt = _KartDepotGet ();
    OldVer = 0;
    NewVer = 0;
    OldPhoby = NULL;
    NewPhoby = NULL;

    XFS_CAN ( Dpt )

    RCt = _VersionAndListForPath ( Dpt -> path, & NewVer, NULL );
    if ( RCt == 0 ) {
        RCt = _KartDepotPhoby ( & OldPhoby );
        if ( RCt == 0 ) {
            OldVer = OldPhoby == NULL
                                ? ( NewVer - 1)
                                : OldPhoby -> version
                                ;
            if ( OldVer != NewVer ) {
                RCt = _KartophobyMake (
                                    & NewPhoby,
                                    Dpt -> path,
                                    OldPhoby
                                    );
                if ( RCt == 0 ) {
                    RCt = _KartDepotPhobySet ( NewPhoby );

                    _KartophobyRelease ( NewPhoby );
                }
            }

            if ( OldPhoby != NULL ) {
                _KartophobyRelease ( OldPhoby );
            }
        }
    }

    return RCt;
}   /* XFSGapKartDepotRefresh () */

LIB_EXPORT
rc_t CC
XFSGapKartDepotVersion ( uint64_t * Ver )
{
    rc_t RCt;
    const struct _Kartophoby * Phoby;

    RCt = 0;
    Phoby = NULL;

    XFS_CSA ( Ver, 0 )
    XFS_CAN ( Ver )

    RCt = _KartDepotPhoby ( & Phoby );
    if ( RCt == 0 ) {
        RCt = _KartophobyVersion ( Phoby, Ver );

        if ( Phoby != NULL ) {
            _KartophobyRelease ( Phoby );
        }
    }

    if ( RCt != 0 ) {
        * Ver = 0;
    }

    return RCt;
}   /* XFSGapKartDepotVersion () */

LIB_EXPORT
const char * CC
XFSGapKartDepotPath ()
{
    struct _KartDepot * Dpt = _KartDepotGet ();
    return Dpt == NULL ? NULL : ( Dpt -> path );
}   /* XFSGapKartDepotPath () */

rc_t CC
_KartDepotPhoby ( const struct _Kartophoby ** Phoby )
{
    rc_t RCt;
    struct _KartDepot * Dpt;

    RCt = 0;
    Dpt = _KartDepotGet ();

    XFS_CSAN ( Phoby )
    XFS_CAN ( Phoby )
    XFS_CAN ( Dpt )

    RCt = KLockAcquire ( Dpt -> mutabor );
    if ( RCt == 0 ) {
        if  ( Dpt -> phoby != NULL ) {
            RCt = _KartophobyAddRef ( Dpt -> phoby );
            if ( RCt == 0 ) {
                * Phoby = Dpt -> phoby;
            }
        }

        KLockUnlock ( Dpt -> mutabor );
    }

    return RCt;
}   /* _KartDepotPhoby () */

rc_t CC
_KartDepotPhobySet ( const struct _Kartophoby * Phoby )
{
    rc_t RCt;
    struct _KartDepot * Dpt;

    RCt = 0;
    Dpt = _KartDepotGet ();

    XFS_CAN ( Phoby )
    XFS_CAN ( Dpt )

    RCt = KLockAcquire ( Dpt -> mutabor );
    if ( RCt == 0 ) {

        RCt = _KartophobyAddRef ( Phoby );
        if ( RCt == 0 ) {
            if  ( Dpt -> phoby != NULL ) {
                RCt = _KartophobyRelease ( Dpt -> phoby );
            }

            Dpt -> phoby = Phoby;
        }

        KLockUnlock ( Dpt -> mutabor );
    }

    return RCt;
}   /* _KartDepotPhobySet () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

