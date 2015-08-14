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
#include <kproc/lock.h>

#include <kfs/file.h>
#include <kfs/file-impl.h>

#include "schwarzschraube.h"
#include "xgapprj_tmp.h"

#include <sysalloc.h>

#include <string.h>

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))    XFSDbGapProjectDepot_Temp, XFSDbGapProject_Temp and
 //     XFSDbGapKart_Temp are living here
((*/

/*))    I know it should be named as XFSDbGapProjectDepot_Temp ... but
 ((*/
struct _DbGapDepot_Temp {
    BSTree tree;

    KLock * mutabor;
};

struct XFSDbGapProject_Temp {
    BSTNode node;

    BSTree tree;

    KLock * mutabor;
    KRefcount refcount;

    uint32_t id;
};

struct _DbGapHolder_Temp {
    BSTNode node;

    struct KLock * mutabor;
    KRefcount refcount;

    const char * accession;
    uint32_t object_id;

    bool completed;

    const struct KFile * file;
};

struct _DbGapFile_Temp {
    struct KFile file;

    const struct KFile * looney_bin;

    struct _DbGapHolder_Temp * holder;
};

/*) Usefuls
 (*/
static rc_t CC _DbGapProjectMake_Temp (
                                uint32_t ProjectId,
                                struct XFSDbGapProject_Temp ** Project
                                );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSDbGapProjectDepot_Temp ...                                     */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static struct _DbGapDepot_Temp * _sDepot = NULL;

static
void CC
_DbGapDepotWhackCallback_Temp ( BSTNode * Node, void * unused )
{
    if ( Node != NULL ) {
        XFSDbGapProjectRelease_Temp (
                            ( const struct XFSDbGapProject_Temp * ) Node
                            );
    }
}   /* _DbGapDepotWhackCallback_Temp () */

static
rc_t CC
_DbGapDepotDispose_Temp ( struct _DbGapDepot_Temp * self )
{
    if ( self != NULL ) {
        BSTreeWhack (
                    & ( self -> tree ),
                    _DbGapDepotWhackCallback_Temp,
                    NULL
                    );

        if ( self -> mutabor != NULL ) {
            KLockRelease ( self -> mutabor );

            self -> mutabor = NULL;
        }

        free ( self );
    }

    return 0;
}   /* _DbGapDepotDispose_Temp () */

static
rc_t CC
_DbGapDepotMake_Temp ( struct _DbGapDepot_Temp ** Depot )
{
    rc_t RCt;
    struct _DbGapDepot_Temp * Dpt;

    RCt = 0;
    Dpt = NULL;

    XFS_CSAN ( Depot )
    XFS_CAN ( Depot )

    Dpt = calloc ( 1, sizeof ( struct _DbGapDepot_Temp ) );
    if ( Dpt == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = KLockMake ( & ( Dpt -> mutabor ) );
        if ( RCt == 0 ) {
            BSTreeInit ( & ( Dpt -> tree ) );

            * Depot = Dpt;
        }
    }

    if ( RCt != 0 ) {
        * Depot = NULL;

        if ( Dpt != NULL ) {
            _DbGapDepotDispose_Temp ( Dpt );

            Dpt = NULL;
        }
    }

    return RCt;
}   /* _DbGapDepotMake_Temp () */

LIB_EXPORT
rc_t CC
XFSDbGapProjectDepotInit_Temp ()
{
    rc_t RCt;
    struct _DbGapDepot_Temp * Depot;

    RCt = 0;
    Depot = NULL;

    if ( _sDepot == NULL ) {
        RCt = _DbGapDepotMake_Temp ( & Depot );
        if ( RCt == 0 ) {
            _sDepot = Depot;
        }
    }

    return RCt;
}   /* XFSDbGapProjectDepotInit_Temp () */

LIB_EXPORT
rc_t CC
XFSDbGapProjectDepotDispose_Temp ()
{
    rc_t RCt;
    struct _DbGapDepot_Temp * Depot;

    RCt = 0;
    Depot = NULL;

    Depot = _sDepot;
    if ( Depot != NULL ) {
        _sDepot = NULL;

        _DbGapDepotDispose_Temp ( Depot );
    }

    return RCt;
}   /* XFSDbGapProjectDepotDispose_Temp () */

static
struct _DbGapDepot_Temp *
_Depot_Temp ()
{
    return _sDepot;
}   /* _Depot_Temp () */

static
int64_t CC
_DbGapDepotFindCallback_Temp ( const void * Item, const BSTNode * Node )
{
    uint32_t ItemId, NodeId;

    ItemId = * ( ( uint32_t * ) Item );
    NodeId = Node == NULL
                    ? 0
                    : ( ( struct XFSDbGapProject_Temp * ) Node ) -> id
                    ;


    return ItemId - NodeId;
}   /* _DbGapDepotFindCallback_Temp () */

static
rc_t CC
_DbGapDepotFindNoLock_Temp (
                        struct _DbGapDepot_Temp * self,
                        uint32_t Id,
                        struct XFSDbGapProject_Temp ** Project
)
{
    rc_t RCt;
    struct XFSDbGapProject_Temp * TheProject;

    RCt = 0;
    TheProject = NULL;

    XFS_CSAN ( Project )
    XFS_CAN ( self )
    XFS_CAN ( Project )

    TheProject = ( struct XFSDbGapProject_Temp * ) BSTreeFind ( 
                                        & ( self -> tree ),
                                        ( const void * ) & Id,
                                        _DbGapDepotFindCallback_Temp
                                        );
    if ( TheProject == NULL ) {

        RCt = XFS_RC ( rcNotFound );
    }
    else {

        * Project = TheProject;
        RCt = 0;
    }

    return RCt;
}   /* _DbGapDepotFindNoLock_Temp () */

// TODO static


LIB_EXPORT
bool CC
XFSDbGapProjectDepotHas_Temp ( uint32_t ProjectId )
{
    rc_t RCt;
    struct _DbGapDepot_Temp * Depot;
    struct XFSDbGapProject_Temp * Project;
    bool RetVal;

    RCt = 0;
    Project = NULL;
    RetVal = false;
    Depot = _Depot_Temp ();

    if ( Depot != NULL ) {

        RCt = KLockAcquire ( Depot -> mutabor );
        if ( RCt == 0 ) {
            RCt = _DbGapDepotFindNoLock_Temp (
                                            Depot,
                                            ProjectId,
                                            & Project
                                            );
            if ( RCt == 0 ) {
                RetVal = true;
            }

            KLockUnlock ( Depot -> mutabor );
        }
    }

    return RetVal;
}   /* XFSDbGapProjectDepotHas_Temp () */

LIB_EXPORT
rc_t CC
XFSDbGapProjectDepotFindOrCreate_Temp (
                                uint32_t ProjectId,
                                struct XFSDbGapProject_Temp ** Project
)
{
    rc_t RCt;
    struct _DbGapDepot_Temp * Depot;
    struct XFSDbGapProject_Temp * TheProject;

    RCt = 0;
    Depot = _Depot_Temp ();
    TheProject = NULL;

    XFS_CAN ( Depot )
    XFS_CSAN ( Project )
    XFS_CAN ( Project )

    RCt = KLockAcquire ( Depot -> mutabor );
    if ( RCt == 0 ) {
        RCt = _DbGapDepotFindNoLock_Temp (
                                        Depot,
                                        ProjectId,
                                        & TheProject
                                        );
        if ( GetRCState ( RCt ) == rcNotFound ) {
            RCt = _DbGapProjectMake_Temp (
                                        ProjectId,
                                        & TheProject
                                        );
        }

        if ( RCt == 0 ) {
            RCt = XFSDbGapProjectAddRef_Temp ( TheProject );
            if ( RCt == 0 ) {
                * Project = TheProject;
            }
        }

        KLockUnlock ( Depot -> mutabor );
    }

    return RCt;
}   /* XFSDbGapProjectDepotFindOrCreate_Temp () */

LIB_EXPORT
rc_t CC
XFSDbGapProjectDepotDelete_Temp ( uint32_t ProjectId )
{
    rc_t RCt;
    struct _DbGapDepot_Temp * Depot;
    struct XFSDbGapProject_Temp * Project;

    RCt = 0;
    Depot = _Depot_Temp ();
    Project = NULL;

    XFS_CAN ( Depot );

    RCt = KLockAcquire ( Depot -> mutabor );
    if ( RCt == 0 ) {
        RCt = _DbGapDepotFindNoLock_Temp (
                                        Depot,
                                        ProjectId,
                                        & Project
                                        );

        if ( RCt == 0 ) {
            BSTreeUnlink ( & ( Depot -> tree ), & ( Project -> node ) );
        }

        KLockRelease ( Depot -> mutabor );
    }

    return RCt;
}   /* XFSDbGapProjectDepotDelete_Temp () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSDbGapProject_Temp ...                                          */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
static const char * _sDbGapProject_classname = "DbGapProject";

rc_t CC
_DbGapProjectDispose_Temp ( struct XFSDbGapProject_Temp * self )
{
    if ( self != NULL ) {
        if ( self -> mutabor != NULL ) {
            KLockRelease ( self -> mutabor );
            self -> mutabor = NULL;
        }

        KRefcountWhack (
                    & ( self -> refcount ),
                    _sDbGapProject_classname
                    );

        self -> id = 0;

        free ( self );
    }

    return 0;
}   /* _DbGapProjectDipsose_Temp () */

rc_t CC
_DbGapProjectMake_Temp (
                        uint32_t ProjectId,
                        struct XFSDbGapProject_Temp ** Project
)
{
    rc_t RCt;
    struct XFSDbGapProject_Temp * TheProject;

    RCt = 0;
    TheProject = NULL;

    XFS_CSAN ( Project )
    XFS_CAN ( Project )

    TheProject = calloc ( 1, sizeof ( struct XFSDbGapProject_Temp ) );
    if ( TheProject == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = KLockMake ( & ( TheProject -> mutabor ) );
        if ( RCt == 0 ) {
            KRefcountInit (
                        & ( TheProject -> refcount ),
                        1,
                        _sDbGapProject_classname,
                        "DbGapProjectMake",
                        "DbGapProject"
                        );

            TheProject -> id = ProjectId;
            * Project = TheProject;
        }
    }

    if ( RCt != 0 ) {
        * Project = NULL;

        if ( TheProject != NULL ) {
            _DbGapProjectDispose_Temp ( TheProject );
        }
    }

    return RCt;
}   /* _DbGapProjectMake_Temp () */

LIB_EXPORT
rc_t CC
XFSDbGapProjectAddRef_Temp ( const struct XFSDbGapProject_Temp * self )
{
    rc_t RCt;
    struct XFSDbGapProject_Temp * Project;
    int RefC;

    RCt = 0;
    Project = ( struct XFSDbGapProject_Temp * ) self;
    RefC = 0;

    XFS_CAN ( self );


    RefC = KRefcountAdd (
                        & ( Project -> refcount ),
                        _sDbGapProject_classname
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
}   /* XFSDbGapProjectAddRef_Temp () */

LIB_EXPORT
rc_t CC
XFSDbGapProjectRelease_Temp ( const struct XFSDbGapProject_Temp * self )
{
    rc_t RCt;
    struct XFSDbGapProject_Temp * Project;
    int RefC;

    RCt = 0;
    RefC = 0;
    Project = ( struct XFSDbGapProject_Temp  * ) self;

    XFS_CAN ( Project );

    RefC = KRefcountDrop (
                        & ( Project -> refcount ),
                        _sDbGapProject_classname
                        );
    switch ( RefC ) {
        case krefOkay :
        case krefZero :
                RCt = 0;
                break;
        case krefWhack :
                RCt = _DbGapProjectDispose_Temp ( Project );
                break;
        case krefNegative :
                RCt = XFS_RC ( rcInvalid );
                break;
        default :
                RCt = XFS_RC ( rcUnknown );
                break;
    }

    return RCt;
}   /* XFSDbGapProjectAddRef_Temp () */

LIB_EXPORT
rc_t CC
XFSDbGapProjectID_Temp (
                    const struct XFSDbGapProject_Temp * self,
                    uint32_t * Id
)
{
    XFS_CSA ( Id, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Id )

    * Id = self -> id;

    return 0;
}   /* XFSDbGapProjectAddRef_Temp () */

static
rc_t CC
_DbGapProjectFindHolderNoLock_Temp ()
{
    // TODO
    return 0;
}   /* _DbGapProjectFindHolderNoLock_Temp () */

static
rc_t CC
_DbGapProjectFindOrCreateHolder_Temp (
                            const struct XFSDbGapProject_Temp * self,
                            const char * Accession,
                            uint32_t ObjectId,
                            struct _DbGapHolder_Temp ** Holder
)
{
    rc_t RCt;

    RCt = 0; 

    XFS_CSAN ( Holder )
    XFS_CAN ( self )
    XFS_CAN ( Holder )

    if ( Accession == NULL && ObjectId == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = KLockAcquire ( self -> mutabor );
    if ( RCt == 0 ) {
        RCt = _DbGapProjectFindHolderNoLock_Temp (
                                                self,
                                                Accession,
                                                ObjectId,
                                                Holder
                                                );
        // if (  TODO
        KLockUnlock ( self -> mutabor );
    }

    return RCt;;
}   /* _DbGapProjectFindOrCreate_Temp () */

LIB_EXPORT
rc_t CC
XFSDbGapProjectOpenFileRead_Temp (
                            const struct XFSDbGapProject_Temp * self,
                            const char * AccessionOrObjectID,
                            const struct KFile ** File
)
{
    rc_t RCt;

    RCt = 0;

    return RCt;
}   /* XFSDbGapProjectAddRef_Temp () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSDbGapProject_Temp ...                                          */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
static const char * _sDbGapHolder_classname = "DbGapHolder";

static
rc_t CC
_DbGapHolderDispose_Temp ( struct _DbGapHolder_Temp * Holder )
{
    if ( Holder != NULL ) {
        KRefcountWhack (
                        & ( Holder -> refcount ),
                        _sDbGapHolder_classname
                        );

        if ( Holder -> mutabor != NULL ) {
            KLockRelease ( Holder -> mutabor );
            Holder -> mutabor = NULL;
        }

        if ( Holder -> file != NULL ) {
                /*  if file is not NULL, it is rihgt file 
                 */
            ( ( struct _DbGapFile_Temp * ) Holder -> file ) -> holder = NULL;

            KFileRelease ( Holder -> file );
            Holder -> file = NULL;
        }

        if ( Holder -> accession != NULL ) {
            free ( ( char * ) Holder -> accession );
            Holder -> accession = NULL;
        }

        free ( Holder );
    }

    return 0;
}   /* _DbGapHolderDispose_Temp () */

static
rc_t CC
_DbGapHolderMake_Temp (
                struct _DbGapHolder_Temp ** Holder,
                const char * Accession,
                uint32_t ObjectId
)
{
    rc_t RCt;
    struct _DbGapHolder_Temp * TheHolder;

    RCt = 0;
    TheHolder = NULL;

    XFS_CSAN ( Holder )
    XFS_CAN ( Holder )

    if ( Accession == NULL && ObjectId == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    TheHolder = calloc ( 1, sizeof ( struct _DbGapHolder_Temp ) );
    if ( TheHolder == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = KLockMake ( & ( TheHolder -> mutabor ) );
        if ( RCt == 0 ) {
            KRefcountInit (
                            & ( TheHolder -> refcount ),
                            1,
                            _sDbGapHolder_classname,
                            "DbGapHolderMake",
                            "DbGapHolder"
                            );
            if ( Accession != NULL ) {
                RCt = XFS_StrDup (
                                Accession,
                                & ( TheHolder -> accession )
                                );
            }
            else {
                TheHolder -> object_id = ObjectId;
            }

            if ( RCt == 0 ) {
                TheHolder -> completed = false;
                TheHolder -> file = NULL;
            }
        }
    }

    return RCt;
}   /* _DbGapHolderMake_Temp () */

static
rc_t CC
_DbGapHolderLock_Temp ( const struct _DbGapHolder_Temp * Holder )
{
    XFS_CAN ( Holder );

    return KLockAcquire ( Holder -> mutabor );
}   /* _DbGapHolderLock_Temp () */

static
rc_t CC
_DbGapHolderUnlock_Temp ( const struct _DbGapHolder_Temp * Holder )
{
    XFS_CAN ( Holder );

    return KLockUnlock ( Holder -> mutabor );
}   /* _DbGapHolderUnlock_Temp () */


static
rc_t CC
_DbGapHolderAddRef_Temp ( const struct _DbGapHolder_Temp * self )
{
    rc_t RCt;
    struct _DbGapHolder_Temp * Holder;
    int RefC;

    RCt = 0;
    Holder = ( struct _DbGapHolder_Temp * ) self;
    RefC = 0;

    XFS_CAN ( Holder );


    RefC = KRefcountAdd (
                        & ( Holder -> refcount ),
                        _sDbGapHolder_classname
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
}   /* _DbGapHolderAddRef_Temp () */

LIB_EXPORT
rc_t CC
_DbGapHolderRelease_Temp ( const struct _DbGapHolder_Temp * self )
{
    rc_t RCt;
    struct _DbGapHolder_Temp * Holder;
    int RefC;

    RCt = 0;
    RefC = 0;
    Holder = ( struct _DbGapHolder_Temp  * ) self;

    XFS_CAN ( Holder );

    RefC = KRefcountDrop (
                        & ( Holder -> refcount ),
                        _sDbGapHolder_classname
                        );
    switch ( RefC ) {
        case krefOkay :
        case krefZero :
                RCt = 0;
                break;
        case krefWhack :
                RCt = _DbGapHolderDispose_Temp ( Holder );
                break;
        case krefNegative :
                RCt = XFS_RC ( rcInvalid );
                break;
        default :
                RCt = XFS_RC ( rcUnknown );
                break;
    }

    return RCt;
}   /* _DbGapHolderRelease_Temp () */


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))
 || _DbGapFile_Temp virtual table
((*/

static rc_t CC _DbGapFile_destroy (
                                struct KFile * self
                                );
static struct KSysFile_v1 * CC _DbGapFile_get_sysfile (
                                const struct KFile * self,
                                uint64_t * Offset
                                );
static rc_t CC _DbGapFile_random_access (
                                const struct KFile * self
                                );
static rc_t CC _DbGapFile_get_size (
                                const struct KFile * self,
                                uint64_t * Size
                                );
static rc_t CC _DbGapFile_set_size (
                                struct KFile * self,
                                uint64_t Size
                               );
static rc_t CC _DbGapFile_read (
                                const struct KFile * self,
                                uint64_t Offset,
                                void * Buffer,
                                size_t BufferSize,
                                size_t * NumRead
                                );
static rc_t CC _DbGapFile_write (
                                struct KFile * self,
                                uint64_t Offset,
                                const void * Buffer,
                                size_t BufferSize,
                                size_t * NumWrite
                                );
static uint32_t CC _DbGapFile_get_type (
                                const struct KFile * self
                                );

#ifdef MINOR_2_JOJOBA
static rc_t CC _DbGapFile_timed_read (
                                const struct KFile * self,
                                uint64_t Offset,
                                void * Buffer,
                                size_t BufferSize,
                                size_t * NumRead,
                                struct timeout_t * Tm
                                );
static rc_t CC _DbGapFile_timed_write (
                                struct KFile * self,
                                uint64_t Offset,
                                const void * Buffer,
                                size_t BufferSize,
                                size_t * NumWrit,
                                struct timeout_t * Tm
                                );
#endif /* MINOR_2_JOJOBA */

static struct KFile_vt_v1 _svtDbGapFile_Temp = {
                                1,  /* maj */
                                1,  /* min */

                            /* start minor version == 0 */
                                _DbGapFile_destroy,
                                _DbGapFile_get_sysfile,
                                _DbGapFile_random_access,
                                _DbGapFile_get_size,
                                _DbGapFile_set_size,
                                _DbGapFile_read,
                                _DbGapFile_write,
                            /* end minor version == 0 */

                            /* start minor version == 1 */
                                _DbGapFile_get_type,
                            /* end minor version == 1 */

#ifdef MINOR_2_JOJOBA
                            /* start minor version == 2 */
                                _DbGapFile_timed_read,
                                _DbGapFile_timed_write,
                            /* end minor version == 2 */
#endif /* MINOR_2_JOJOBA */
};

/*))
 || _DbGapFile_Temp constructor
((*/
static
rc_t CC
_DbGapFileMake_Temp (
                    struct KFile ** File,
                    struct _DbGapHolder_Temp * Holder
)
{
    rc_t RCt;
    struct _DbGapFile_Temp * GapFile;

    RCt = 0;
    GapFile = NULL;

    XFS_CSAN ( File )
    XFS_CAN ( File )

    GapFile = calloc ( 1, sizeof ( struct _DbGapFile_Temp ) );
    if ( GapFile == NULL ) {
        RCt = XFS_RC ( rcNull );
    }
    else {
        RCt = KFileInit (
                & ( GapFile -> file ),
                ( const KFile_vt * ) & _svtDbGapFile_Temp,
                "DbGapFile_Temp",
                "DbGapFile",
                true,
                false
                );
        if ( RCt == 0 ) {
            RCt = _DbGapHolderAddRef_Temp ( Holder );
            if ( RCt == 0 ) {
                GapFile -> holder = Holder;
                GapFile -> looney_bin = NULL;

                * File = & ( GapFile -> file );
            }
        }
    }

    if ( RCt != 0 ) {
        * File = NULL;
        KFileRelease ( & ( GapFile -> file ) );
    }

    return RCt;
}   /* _DbGapFileMake () */

/*))
 || _DbGapFile virtual table implementation
((*/
rc_t CC
_DbGapFile_destroy ( struct KFile * self )
{
    struct _DbGapFile_Temp * File = ( struct _DbGapFile_Temp * ) self;

    if ( File != NULL ) {
        if ( File -> holder != NULL ) {
            _DbGapHolderLock_Temp ( File -> holder );

            _DbGapHolderRelease_Temp ( File -> holder );

            if ( File -> holder -> file == ( struct KFile * ) File ) {
                File -> holder -> file = NULL;
            }

            _DbGapHolderUnlock_Temp ( File -> holder );

            File -> holder = NULL;
        }

        if ( File -> looney_bin != NULL ) {
            KFileRelease ( File -> looney_bin );
            File -> looney_bin = NULL;
        }

        free ( File );
    }

    return 0;
}   /* _DbGapFile_destroy () */

struct KSysFile_v1 * CC
_DbGapFile_get_sysfile ( const struct KFile * self, uint64_t * Offset )
{
    struct _DbGapFile_Temp * File = ( struct _DbGapFile_Temp * ) self;

    if ( File != NULL ) {
        if ( File -> looney_bin != NULL ) {
            return KFileGetSysFile ( File -> looney_bin, Offset );
        }
    }

    return NULL;
}   /* _DbGapFile_get_sysfile () */

rc_t CC
_DbGapFile_random_access ( const struct KFile * self )
{
    struct _DbGapFile_Temp * File = ( struct _DbGapFile_Temp * ) self;

    XFS_CAN ( File )
    XFS_CAN ( File -> looney_bin )

    return KFileRandomAccess ( File -> looney_bin );
}   /* _DbGapFile_random_access () */

rc_t CC
_DbGapFile_get_size ( const struct KFile * self, uint64_t * Size )
{
    rc_t RCt;
    struct _DbGapFile_Temp * File;

    RCt = 0;
    File = ( struct _DbGapFile_Temp * ) self;

    XFS_CSA ( Size, 0 )
    XFS_CAN ( File )
    XFS_CAN ( Size )
    XFS_CAN ( File -> looney_bin )
    XFS_CAN ( File -> holder )

    RCt = _DbGapHolderLock_Temp ( File -> holder );
    if ( RCt == 0 ) {
        RCt = KFileSize ( File -> looney_bin, Size );

        _DbGapHolderUnlock_Temp ( File -> holder );
    }

    return RCt;
}   /* _DbGapFile_get_size () */

rc_t CC
_DbGapFile_set_size ( struct KFile * self, uint64_t Size )
{
    XFS_CAN ( self )

    return RC (rcFS, rcDirectory, rcUpdating, rcSelf, rcUnsupported);
}   /* _DbGapFile_set_size () */

rc_t CC
_DbGapFile_read (
                const struct KFile * self,
                uint64_t Offset,
                void * Buffer,
                size_t BufferSize,
                size_t * NumRead
)
{
    rc_t RCt;
    struct _DbGapFile_Temp * File;

    RCt = 0;
    File = ( struct _DbGapFile_Temp * ) self;

    XFS_CSA ( NumRead, 0 )
    XFS_CAN ( File )
    XFS_CAN ( Buffer )
    XFS_CAN ( NumRead )
    XFS_CAN ( File -> looney_bin )

    RCt = _DbGapHolderLock_Temp ( File -> holder );
    if ( RCt == 0 ) {
        RCt = KFileRead (
                        File -> looney_bin,
                        Offset,
                        Buffer,
                        BufferSize,
                        NumRead
                        );

        _DbGapHolderUnlock_Temp ( File -> holder );
    }

    return RCt;
}   /* _DbGapFile_read () */

rc_t CC
_DbGapFile_write (
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
}   /* _DbGapFile_write () */

uint32_t CC
_DbGapFile_get_type ( const struct KFile * self )
{
    struct _DbGapFile_Temp * File = ( struct _DbGapFile_Temp * ) self;
    return KFileType ( File -> looney_bin );
}   /* _DbGapFile_get_type () */


#ifdef MINOR_2_JOJOBA
rc_t CC
_DbGapFile_timed_read (
                const struct KFile * self,
                uint64_t Offset,
                void * Buffer,
                size_t BufferSize,
                size_t * NumRead,
                struct timeout_t * Tm
)
{
    rc_t RCt;
    struct _DbGapFile_Temp * File;

    RCt = 0;
    File = ( struct _DbGapFile_Temp * ) self;

    XFS_CSA ( NumRead, 0 )
    XFS_CAN ( File )
    XFS_CAN ( Buffer )
    XFS_CAN ( NumRead )
    XFS_CAN ( File -> looney_bin )

    RCt = _DbGapHolderLock_Temp ( File -> holder );
    if ( RCt == 0 ) {
        KFileTimedRead (
                        File,
                        Offset,
                        Buffer,
                        BufferSize,
                        NumRead,
                        Tm
                        );

        _DbGapHolderUnlock_Temp ( File -> holder );
    }

    return RCt;
}   /* _DbGapFile_timed_read () */

rc_t CC
_DbGapFile_timed_write (
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
}   /* _DbGapFile_timed_write () */

#endif /* MINOR_2_JOJOBA */

