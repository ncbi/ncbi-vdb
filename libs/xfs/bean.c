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

#include "schwarzschraube.h"
#include "bean.h"
#include "zehr.h"

#include <sysalloc.h>

#include <string.h>

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* struct XFSBean                                                    */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
static const char * _sXFSBean_classname = "XFSBean";

struct XFSBean {
        BSTNode node;

        KRefcount refcount;

        const char * name;
        const char * resource;
        uint64_t version;
};

LIB_EXPORT
rc_t CC
XFSBeanMake (
            const struct XFSBean ** Bean,
            const char * Name,
            const char * Resource,
            uint64_t Version
)
{
    rc_t RCt;
    struct XFSBean * TheBean;

    RCt = 0;
    TheBean = NULL;

    XFS_CSAN ( Bean )
    XFS_CAN ( Bean )
    XFS_CAN ( Name )
    XFS_CAN ( Resource )

    TheBean = calloc ( 1, sizeof ( struct XFSBean ) );
    if ( TheBean == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFS_StrDup ( Name, & ( TheBean -> name ) );
        if ( RCt == 0 ) {
            RCt = XFS_StrDup ( Resource, & ( TheBean -> resource ) );
            if ( RCt == 0 ) {
                TheBean -> version = Version;

                KRefcountInit (
                            & ( TheBean -> refcount ),
                        1,
                        _sXFSBean_classname,
                        "XFSBeanMake",
                        "Bean"
                        );
                * Bean = ( const struct XFSBean * ) TheBean;
            }
        }
    }

    if ( RCt != 0 ) {
        * Bean = NULL;

        if ( TheBean != NULL ) {
            XFSBeanDispose ( TheBean );
        }
    }

    return RCt;
}   /* XFSBeanMake () */

LIB_EXPORT
rc_t CC
XFSBeanDispose ( const struct XFSBean * self )
{
    struct XFSBean * Bean = ( struct XFSBean * ) self;

    if ( Bean != NULL ) {
        KRefcountWhack ( & ( Bean -> refcount ), _sXFSBean_classname );

        if ( Bean -> name != NULL ) {
            free ( ( char * ) Bean -> name );
            Bean -> name = NULL;
        }

        if ( Bean -> resource != NULL ) {
            free ( ( char * ) Bean -> resource );
            Bean -> resource = NULL;
        }

        Bean -> version = 0;

        free ( Bean );
    }

    return 0;
}   /* XFSBeanDispose () */

LIB_EXPORT
rc_t CC
XFSBeanAddRef ( const struct XFSBean * self )
{
    rc_t RCt;
    int Cnt;

    RCt = 0;
    Cnt = 0;

    XFS_CAN ( self )

    Cnt = KRefcountAdd ( & ( self -> refcount ), _sXFSBean_classname );
    switch ( Cnt ) {
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
}   /* XFSBeanAddRef () */

LIB_EXPORT
rc_t CC
XFSBeanRelease ( const struct XFSBean * self )
{
    rc_t RCt;
    int Cnt;

    RCt = 0;
    Cnt = 0;

    XFS_CAN ( self )

    Cnt = KRefcountDrop ( & ( self -> refcount ), _sXFSBean_classname );
    switch ( Cnt ) {
        case krefOkay :
        case krefZero :
                    RCt = 0;
                    break;
        case krefWhack :
                    RCt = XFSBeanDispose ( self );
                    break;
        case krefNegative :
                    RCt = XFS_RC ( rcInvalid );
                    break;
        default :
                    RCt = XFS_RC ( rcUnknown );
                    break;
    }

    return RCt;
}   /* XFSBeanRelease () */

LIB_EXPORT
rc_t CC
XFSBeanName ( const struct XFSBean * self, const char ** Name )
{
    XFS_CSAN ( Name )
    XFS_CAN ( self )
    XFS_CAN ( Name )
    XFS_CAN ( self -> name )

    * Name = self -> name;

    return 0;
}   /* XFSBeanName () */

LIB_EXPORT
rc_t CC
XFSBeanResource ( const struct XFSBean * self, const char ** Resource )
{
    XFS_CSAN ( Resource )
    XFS_CAN ( self )
    XFS_CAN ( Resource )
    XFS_CAN ( self -> resource )

    * Resource = self -> resource;

    return 0;
}   /* XFSBeanResource () */

LIB_EXPORT
rc_t CC
XFSBeanVersion ( const struct XFSBean * self, uint64_t * Version )
{
    XFS_CSA ( Version, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Version )
    XFS_CAN ( self -> resource )

    * Version = self -> version;

    return 0;
}   /* XFSBeanVersion () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* struct XFSBeanSack                                                */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
static const char * _sXFSBeanSack_classname = "XFSBeanSack";

struct XFSBeanSack {
        BSTree tree;

        KRefcount refcount;

        uint64_t version;

        const struct KNamelist * namelist;
        uint64_t namelist_version;
};

LIB_EXPORT
rc_t CC
XFSBeanSackMake (
                const struct XFSBeanSack ** Sack,
                uint64_t InitialVersion
)
{
    rc_t RCt;
    struct XFSBeanSack * TheSack;

    RCt = 0;
    TheSack = NULL;

    XFS_CSAN ( Sack )
    XFS_CAN ( Sack )

    TheSack = calloc ( 1, sizeof ( struct XFSBeanSack ) );
    if ( TheSack == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        TheSack -> version = InitialVersion == 0 ? 1 : InitialVersion;
        TheSack -> namelist = NULL;
        TheSack -> namelist_version = 0;
        BSTreeInit ( & ( TheSack -> tree ) );
        KRefcountInit (
                    & ( TheSack -> refcount ),
                    1,
                    _sXFSBeanSack_classname,
                    "XFSBeanSackMake",
                    "BeanSackMake"
                    );
        * Sack = TheSack;
    }

        /* Right now it is impossibe situation
         */
    if ( RCt != 0 ) {
        * Sack = NULL;

        if ( TheSack != NULL ) {
            XFSBeanSackDispose ( TheSack );
        }
    }

    return RCt;
}   /* XFSBeanSackMake () */

static
void CC
_BeanSackWhackCallback ( BSTNode * Node, void * unused )
{
    if ( Node != NULL ) {
        XFSBeanRelease ( ( struct XFSBean * ) Node );
    }
}   /* _BeanSackWhackCallback () */

LIB_EXPORT
rc_t CC
XFSBeanSackDispose ( const struct XFSBeanSack * self )
{
    struct XFSBeanSack * Sack = ( struct XFSBeanSack * ) self;

    if ( Sack != NULL ) {
        BSTreeWhack (
                    & ( Sack -> tree ),
                    _BeanSackWhackCallback,
                    NULL
                    );
        KRefcountWhack (
                    & ( Sack -> refcount ),
                    _sXFSBeanSack_classname
                    );

        if ( Sack -> namelist != NULL ) {
            KNamelistRelease ( Sack -> namelist );
            Sack -> namelist = NULL;
        }

        Sack -> namelist_version = 0;
        Sack -> version = 0;
    }

    return 0;
}   /* XFSBeanSackDispose () */

LIB_EXPORT
rc_t CC
XFSBeanSackAddRef ( const struct XFSBeanSack * self )
{
    rc_t RCt;
    int Cnt;

    RCt = 0;
    Cnt = 0;

    XFS_CAN ( self )

    Cnt = KRefcountAdd (
                    & ( self -> refcount ),
                    _sXFSBeanSack_classname
                    );
    switch ( Cnt ) {
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
}   /* XFSBeanSackAddRef () */

LIB_EXPORT
rc_t CC
XFSBeanSackRelease ( const struct XFSBeanSack * self )
{
    rc_t RCt;
    int Cnt;

    RCt = 0;
    Cnt = 0;

    XFS_CAN ( self )

    Cnt = KRefcountDrop (
                        & ( self -> refcount ),
                        _sXFSBeanSack_classname
                        );
    switch ( Cnt ) {
        case krefOkay :
        case krefZero :
                    RCt = 0;
                    break;
        case krefWhack :
                    RCt = XFSBeanSackDispose ( self );
                    break;
        case krefNegative :
                    RCt = XFS_RC ( rcInvalid );
                    break;
        default :
                    RCt = XFS_RC ( rcUnknown );
                    break;
    }

    return RCt;
}   /* XFSBeanSackRelease () */

static
void
_UpdateNameListCallback ( BSTNode * Node, void * Data )
{
    struct VNamelist * List;
    struct XFSBean * Bean;

    List = ( struct VNamelist * ) Data;
    Bean = ( struct XFSBean * ) Node;

    if ( List != NULL && Bean != NULL ) {
        VNamelistAppend ( List, Bean -> name );
    }
}   /* _UpdateNameListCallback () */

static
rc_t CC
_CheckUpdateNamelist ( const struct XFSBeanSack * self )
{
    rc_t RCt;
    struct XFSBeanSack * Sack;
    const struct KNamelist * kList;
    struct VNamelist * List;

    RCt = 0;
    Sack = ( struct XFSBeanSack * ) self;
    kList = NULL;
    List = NULL;

    XFS_CAN ( Sack )

    if ( Sack -> namelist_version == Sack -> version ) {
        return 0;
    }

    RCt = VNamelistMake ( & List, 32 );
    if ( RCt == 0 ) {
        BSTreeForEach (
                    & ( Sack -> tree ),
                    false,
                    _UpdateNameListCallback,
                    List
                    );
        RCt = VNamelistToConstNamelist ( List, & kList );
        if ( RCt == 0 ) {

            KNamelistRelease ( Sack -> namelist );

            Sack -> namelist = kList;
            Sack -> namelist_version = Sack -> version;
        }

        VNamelistRelease ( List );
    }

    return RCt;
}   /* _CheckUpdateNamelist () */

LIB_EXPORT
rc_t CC
XFSBeanSackNames (
                const struct XFSBeanSack * self,
                const struct KNamelist ** Names
)
{
    rc_t RCt = 0;

    XFS_CSAN ( Names )
    XFS_CAN ( self )
    XFS_CAN ( Names )

    RCt = _CheckUpdateNamelist ( self );
    if ( RCt == 0 ) {

        RCt = KNamelistAddRef ( self -> namelist );
        if ( RCt == 0 ) {
            * Names = self -> namelist;
        }
    }

    return RCt;
}   /* XFSBeanSackNames () */

LIB_EXPORT
rc_t CC
XFSBeanSackVersion (
                const struct XFSBeanSack * self,
                uint64_t * Version
)
{
    XFS_CSA ( Version, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Version )

    * Version = self -> version;

    return 0;
}   /* XFSBeanSackVersion () */

LIB_EXPORT
rc_t CC
XFSBeanSackSetVersion (
                const struct XFSBeanSack * self,
                uint64_t Version
)
{
    struct XFSBeanSack * Sack = ( struct XFSBeanSack * ) self;

    XFS_CAN ( Sack )

    Sack -> version = Version == 0 ? 1 : Version;

    return 0;
}   /* XFSBeanSackSetVersion () */

static
int64_t CC
_BeanSackFindCallback ( const void * Item, const BSTNode * Node )
{
    const char * Str1, * Str2;

    Str1 = ( const char * ) Item;

    Str2 = Node == NULL ? NULL : ( ( struct XFSBean * ) Node ) -> name;

    return XFS_StringCompare4BST_ZHR ( Str1, Str2 );
}   /* _BeanSackFindCallback () */

LIB_EXPORT
rc_t CC
XFSBeanSackFind (
                const struct XFSBeanSack * self,
                const struct XFSBean ** Bean,
                const char * Name
)
{
    rc_t RCt;
    const struct XFSBean * TheBean;

    RCt = 0;
    TheBean = NULL;

    XFS_CSAN ( Bean )
    XFS_CAN ( self )
    XFS_CAN ( Bean )
    XFS_CAN ( Name )

    TheBean = ( const struct XFSBean * ) BSTreeFind (
                                                & ( self -> tree ),
                                                Name,
                                                _BeanSackFindCallback
                                                );
    if ( TheBean == NULL ) {
        RCt = XFS_RC ( rcNotFound );
    }
    else {
        RCt = XFSBeanAddRef ( TheBean );
        if ( RCt == 0 ) {
            * Bean = TheBean;
        }
    }

    return RCt;
}   /* XFSBeanSackFind () */

static
int64_t CC
_BeanSackAddCallback ( const BSTNode * N1, const BSTNode * N2 )
{
    return XFS_StringCompare4BST_ZHR (
                                ( ( struct XFSBean * ) N1 ) -> name,
                                ( ( struct XFSBean * ) N2 ) -> name
                                );
}   /* _BeanSackAddCallback () */

LIB_EXPORT
rc_t CC
XFSBeanSackAdd (
                const struct XFSBeanSack * self,
                const struct XFSBean * Bean
)
{
    rc_t RCt = 0;

    XFS_CAN ( self )
    XFS_CAN ( Bean )

    RCt = XFSBeanAddRef ( Bean );
    if ( RCt == 0 ) {
        RCt = BSTreeInsert (
                            ( struct BSTree * ) & ( self -> tree ),
                            ( struct BSTNode * ) & ( Bean -> node ),
                            _BeanSackAddCallback
                            );
        if ( RCt != 0 ) {
            XFSBeanRelease ( Bean );
        }
    }

    return RCt;
}   /* XFSBeanSackAdd () */

LIB_EXPORT
rc_t CC
XFSBeanSackDel (
                const struct XFSBeanSack * self,
                const struct XFSBean * Bean
)
{
    rc_t RCt = 0;

    XFS_CAN ( self )
    XFS_CAN ( Bean )

    RCt = BSTreeUnlink ( 
                    ( struct BSTree * ) & ( self -> tree ),
                    ( struct BSTNode * ) & ( Bean -> node )
                    );
    if ( RCt == 0 ) {
        XFSBeanRelease ( Bean );
    }

    return RCt;
}   /* XFSBeanSackDel () */


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* struct XFSBeanSackDepot                                           */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
struct XFSBeanSackDepot {
    struct KLock * mutabor;

    const struct XFSBeanSack * sack;

    uint64_t version;
};

static
rc_t CC
XFSBeanSackDepotDispose ( const struct XFSBeanSackDepot * self )
{
    struct XFSBeanSackDepot * Depot = ( struct XFSBeanSackDepot * ) self;

    if ( Depot != NULL ) {

        if ( Depot -> mutabor != NULL ) {
            KLockRelease ( Depot -> mutabor );
            Depot -> mutabor = NULL;
        }

        if ( Depot -> sack != NULL ) {
            XFSBeanSackRelease ( ( struct XFSBeanSack * ) Depot -> sack );
            Depot -> sack = NULL;
        }

        Depot -> version = 0;

        free ( Depot );
    }

    return 0;
}   /* XFSBeanSackDepotDispose () */

static
rc_t CC
XFSBeanSackDepotMake ( const struct XFSBeanSackDepot ** Depot )
{
    rc_t RCt;
    struct XFSBeanSackDepot * TheDepot;

    RCt = 0;
    TheDepot = NULL;

    XFS_CSAN ( Depot )
    XFS_CAN ( Depot )

    TheDepot = calloc ( 1, sizeof ( struct XFSBeanSackDepot ) );
    if ( TheDepot == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = KLockMake ( & ( TheDepot -> mutabor ) );
        if ( RCt == 0 ) {

            TheDepot -> version = 0;
            TheDepot -> sack = NULL;

            * Depot = TheDepot;
        }
    }

    if ( RCt != 0 ) {
        * Depot = NULL;

        if ( TheDepot != NULL ) {
            XFSBeanSackDepotDispose ( TheDepot );
        }
    }

    return RCt;
}   /* XFSBeanSackDepotMake () */

static
rc_t CC
XFSBeanSackDepotGet (
                    const struct XFSBeanSackDepot * self,
                    const struct XFSBeanSack ** Sack
)
{
    rc_t RCt;
    struct XFSBeanSackDepot * Depot;

    RCt = 0;
    Depot = ( struct XFSBeanSackDepot * ) self;

    XFS_CSAN ( Sack )
    XFS_CAN ( Depot )
    XFS_CAN ( Sack )

    RCt = KLockAcquire ( Depot -> mutabor );
    if ( RCt == 0 ) {
        if ( Depot -> sack != NULL ) {
            RCt = XFSBeanSackAddRef ( Depot -> sack );
            if ( RCt == 0 ) {
                * Sack = Depot -> sack;
            }
        }

        KLockUnlock ( Depot -> mutabor );
    }

    return RCt;
}   /* XFSBeanSackDepotGet () */

static
rc_t CC
XFSBeanSackDepotSet (
                    const struct XFSBeanSackDepot * self,
                    const struct XFSBeanSack * Sack
)
{
    rc_t RCt;
    struct XFSBeanSackDepot * Depot;

    RCt = 0;
    Depot = ( struct XFSBeanSackDepot * ) self;

    XFS_CAN ( self )

    RCt = KLockAcquire ( Depot -> mutabor );
    if ( RCt == 0 ) {

        if ( Sack != NULL ) {
            RCt = XFSBeanSackAddRef ( Sack );
        }

        if ( RCt == 0 ) {
            if ( Depot -> sack != NULL ) {
                XFSBeanSackRelease ( Depot -> sack );
                Depot -> sack = NULL;
            }

            Depot -> sack = Sack;
            Depot -> version ++;
        }

        KLockUnlock ( Depot -> mutabor );
    }

    return RCt;
}   /* XFSBeanSackDepotSet () */

static
rc_t CC
XFSBeanSackDepotVersion (
                    const struct XFSBeanSackDepot * self,
                    uint64_t * Version
)
{
    XFS_CSA ( Version, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Version )

    * Version = self -> version;

    return 0;
}   /* XFSBeanSackDepotVersion () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* struct XFSBeanSuite                                           */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
static const char * _sXFSBeanSuite_classname = "XFSBeanSuite";

LIB_EXPORT
rc_t CC
XFSBeanSuiteInit (
                    const struct XFSBeanSuite * Suite,
                    const union XFSBeanSuite_vt * VTable
)
{
    rc_t RCt;
    struct XFSBeanSuite * TheSuite;
    const struct XFSBeanSackDepot * Depot; 

    RCt = 0;
    TheSuite = ( struct XFSBeanSuite * ) Suite;
    Depot = NULL;

    XFS_CAN ( TheSuite )
    XFS_CAN ( VTable )

    RCt = XFSBeanSackDepotMake ( & Depot );
    if ( RCt == 0 ) {

        KRefcountInit (
                    & ( TheSuite -> refcount ),
                    1,
                    _sXFSBeanSuite_classname,
                    "XFSBeanSuiteInit",
                    "BeanSuite"
                    );

        TheSuite -> vt = VTable;
        TheSuite -> depot = Depot;
    }

    return RCt;
}   /* XFSBeanSuiteMake () */

LIB_EXPORT
rc_t CC
XFSBeanSuiteDispose ( const struct XFSBeanSuite * self )
{
    rc_t RCt;
    struct XFSBeanSuite * Suite;

    RCt = 0;
    Suite = ( struct XFSBeanSuite * ) self;

    if ( Suite != NULL ) {
        KRefcountWhack (
                        & ( Suite -> refcount ),
                        _sXFSBeanSuite_classname
                        );

        if ( Suite -> depot != NULL ) {
            XFSBeanSackDepotDispose ( Suite -> depot );
            Suite -> depot = NULL;
        }

        if ( Suite -> vt != NULL ) {
            switch ( Suite -> vt -> v1.maj ) {
                case 1 :
                    if ( Suite -> vt -> v1 . dispose != NULL ) {
                        RCt = Suite -> vt -> v1 . dispose ( self );
                    }
                    break;
                default :
                    RCt = XFS_RC ( rcBadVersion );
                    break;
            }
        }
        else {
            free ( Suite );

            RCt = XFS_RC ( rcInvalid );
        }
    }

    return RCt;
}   /* XFSBeanSuiteDispose () */

LIB_EXPORT
rc_t CC
XFSBeanSuiteAddRef ( const struct XFSBeanSuite * self )
{
    rc_t RCt;
    int Cnt;

    RCt = 0;
    Cnt = 0;

    XFS_CAN ( self )

    Cnt = KRefcountAdd (
                        & ( self -> refcount ),
                        _sXFSBeanSuite_classname
                        );
    switch ( Cnt ) {
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
}   /* XFSBeanSuiteAddRef () */

LIB_EXPORT
rc_t CC
XFSBeanSuiteRelease ( const struct XFSBeanSuite * self )
{
    rc_t RCt;
    int Cnt;

    RCt = 0;
    Cnt = 0;

    XFS_CAN ( self )

    Cnt = KRefcountDrop (
                        & ( self -> refcount ),
                        _sXFSBeanSuite_classname
                        );
    switch ( Cnt ) {
        case krefOkay :
        case krefZero :
                    RCt = 0;
                    break;
        case krefWhack :
                    RCt = XFSBeanSuiteDispose ( self );
                    break;
        case krefNegative :
                    RCt = XFS_RC ( rcInvalid );
                    break;
        default :
                    RCt = XFS_RC ( rcUnknown );
                    break;
    }

    return RCt;
}   /* XFSBeanSuiteRelease () */

LIB_EXPORT
rc_t CC
XFSBeanSuiteFind (
                    const struct XFSBeanSuite * self,
                    const char * Name,
                    const struct XFSNode ** Node
)
{
    rc_t RCt;
    const struct XFSNode * TheNode;

    RCt = 0;
    TheNode = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( self )
    XFS_CAN ( Name )
    XFS_CAN ( Node )
    XFS_CAN ( self -> vt )

    switch ( self -> vt -> v1.maj ) {
        case 1 :
            if ( self -> vt -> v1 . render != NULL ) {
                RCt = self -> vt -> v1 . render (
                                                self,
                                                Name,
                                                & TheNode
                                                );
                if ( RCt == 0 ) {
                    * Node = TheNode;
                }
            }
            break;
        default :
            RCt = XFS_RC ( rcBadVersion );
            break;
    }

    return RCt;
}   /* XFSBeanSuiteFind () */

LIB_EXPORT
rc_t CC
XFSBeanSuiteRefresh ( const struct XFSBeanSuite * self )
{
    rc_t RCt = 0;

    XFS_CAN ( self )
    XFS_CAN ( self -> vt )

    switch ( self -> vt -> v1.maj ) {
        case 1 :
            if ( self -> vt -> v1 . refresh != NULL ) {
                RCt = self -> vt -> v1 . refresh ( self );
            }
            break;
        default :
            RCt = XFS_RC ( rcBadVersion );
            break;
    }

    return RCt;
}   /* XFSBeanSuiteRefresh () */

LIB_EXPORT
rc_t CC
XFSBeanSuiteUnlink (
                    const struct XFSBeanSuite * self,
                    const char * Name
)
{
    rc_t RCt = 0;

    XFS_CAN ( self )
    XFS_CAN ( Name )
    XFS_CAN ( self -> vt )

    switch ( self -> vt -> v1.maj ) {
        case 1 :
            if ( self -> vt -> v1 . unlink != NULL ) {
                RCt = self -> vt -> v1 . unlink ( self, Name );
            }
            break;
        default :
            RCt = XFS_RC ( rcBadVersion );
            break;
    }

    return RCt;
}   /* XFSBeanSuiteUnlink () */

LIB_EXPORT
rc_t CC
XFSBeanSuiteGetSack (
                        const struct XFSBeanSuite * self,
                        const struct XFSBeanSack ** Sack
)
{
    XFS_CSAN ( Sack )
    XFS_CAN ( self )
    XFS_CAN ( Sack )
    XFS_CAN ( self -> depot )

    return XFSBeanSackDepotGet ( self -> depot, Sack );
}   /* XFSBeanSuiteGetSack () */

LIB_EXPORT
rc_t CC
XFSBeanSuiteSetSack (
                        const struct XFSBeanSuite * self,
                        const struct XFSBeanSack * Sack
)
{
    XFS_CAN ( self )
    XFS_CAN ( Sack )
    XFS_CAN ( self -> depot )

    return XFSBeanSackDepotSet ( self -> depot, Sack );
}   /* XFSBeanSuiteSetSack () */

LIB_EXPORT
rc_t CC
XFSBeanSuiteVersion (
                        const struct XFSBeanSuite * self,
                        uint64_t * Version
)
{
    XFS_CSA ( Version, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Version )
    XFS_CAN ( self -> depot )

    return XFSBeanSackDepotVersion ( self -> depot, Version );

    return 0;
}   /* XFSBeanSuiteVersion () */
