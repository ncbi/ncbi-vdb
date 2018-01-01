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
#include <klib/log.h>
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
#include <vfs/services.h>

#include <kfg/kart.h>

#include "schwarzschraube.h"
#include "zehr.h"
#include "mehr.h"
#include "xgap.h"
#include "orz.h"
#include <xfs/path.h>


#include <sysalloc.h>

#define KART_FILES_NAME "kart-files"

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * JIPPOTAM
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
#ifdef GETTID_NID
#include <sys/syscall.h>
int gettid ()
{
/*
return syscall ( 224 );
return syscall ( 186 );
*/
return syscall ( SYS_gettid );
}
#endif /* GETTID_NID */

#define SEC_MILLISEC
#ifdef SEC_MILLISEC
uint32_t
_sec_millisec ()
{
    struct timeval Tv;
    uint32_t BUBU = 1000;

    if ( gettimeofday ( & Tv, NULL ) == 0 ) {
        return ( ( Tv . tv_sec % BUBU ) * BUBU )
                                        + ( Tv . tv_usec / BUBU );
    }

    return 0;
}   /* _sec_millisec () */
#endif /* SEC_MILLISEC */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * The main joke, is ... there is no such structure as XFSGap ... :LOL:
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))
 // Common sense declarations    
((*/
struct LI_Stru {
    struct VNamelist * list;
    uint32_t project_id;
    bool flag;
};


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * DieBananable
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
struct _DieBananable {
    struct BSTNode node;

    void * data;
};

static
rc_t CC
_DieBananableMake ( struct _DieBananable ** Bananable, void * Data )
{
    struct _DieBananable * Bable = NULL;

        /*  There is no check for Data != NULL, cuz it could
         */
    XFS_CSAN ( Bananable )
    XFS_CAN ( Bananable )

    Bable = calloc ( 1, sizeof ( struct _DieBananable ) );
    if ( Bable == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    Bable -> data = Data;

    * Bananable = Bable;

    return 0;
}   /* _DieBananableMake () */

static
rc_t CC
_DieBananableDispose ( const struct _DieBananable * self )
{
    struct _DieBananable * Bananable = ( struct _DieBananable * ) self;

    if ( Bananable != NULL ) {
        memset ( Bananable, 0, sizeof ( struct _DieBananable ) );

        free ( Bananable );
    }

    return 0;
}   /* _DieBananableDispose () */

static
rc_t CC
_DieBananableGet ( const struct _DieBananable * self, void ** Data )
{
    XFS_CSAN ( Data )
    XFS_CAN ( self )
    XFS_CAN ( Data )

    * Data = self -> data;

    return 0;
}   /* _DieBananableGet () */


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * DieBanana
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

typedef void ( CC * _DieBananaWhacker )
                            ( struct BSTNode *n, void *data );
typedef int64_t ( CC * _DieBananaFinder )
                            ( const void *item, const struct BSTNode *n );
typedef int64_t ( CC * _DieBananaAdder )
                            ( const struct BSTNode *item, const struct BSTNode *n );
typedef void ( CC * _DieBananaEacher )
                            ( struct BSTNode *n, void *data );

struct _DieBanana {
    struct BSTree banana;

    struct KLock * mutabor;


    _DieBananaWhacker whacker;
};

static
rc_t
_DieBananaDispose ( const struct _DieBanana * self )
{
    struct _DieBanana * Banana = ( struct _DieBanana * ) self;

    if ( Banana != NULL ) {
        if ( Banana -> whacker != NULL ) {
            BSTreeWhack (
                        & ( Banana -> banana ),
                        Banana -> whacker,
                        Banana
                        );
            BSTreeInit ( & ( Banana -> banana ) );
        }

        if ( Banana -> mutabor != NULL ) {
            KLockRelease ( Banana -> mutabor );
            Banana -> mutabor = NULL;
        }

        /*****
         * We do not free bana here ... do it outside
         *****/
    }

    return 0;
}   /* _DieBananaDispose () */

static
rc_t
_DieBananaMake (
                const struct _DieBanana ** Banana,
                _DieBananaWhacker Whacker
)
{
    rc_t RCt;
    struct _DieBanana * DieBanana;

    RCt = 0;
    DieBanana = NULL;

    DieBanana = calloc ( 1, sizeof ( struct _DieBanana ) );
    if ( DieBanana == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        DieBanana -> whacker = Whacker;

        RCt = KLockMake ( & ( DieBanana -> mutabor ) );
        if ( RCt == 0 ) {
            BSTreeInit ( & ( DieBanana -> banana ) );

            * Banana = DieBanana;
        }
    }

    if ( RCt != 0 ) {
        * Banana = NULL;

        _DieBananaDispose ( DieBanana );
    }

    return RCt;
}   /* _DieBananaMake () */

/*))
 // NOTE: those general Banana methods aren't locking ...
((*/
static
rc_t CC
_DieBananaFind_NoLock (
                    const struct _DieBanana * self,
                    const void ** Item,
                    const void * Key,
                    _DieBananaFinder BananaFinder
)
{
    rc_t RCt;
    const void * BananaFound;

    RCt = 0;
    BananaFound = NULL;

    XFS_CSAN ( Item )
    XFS_CAN ( self )
    XFS_CAN ( Item )
    XFS_CAN ( Key )
    XFS_CAN ( BananaFinder )

    BananaFound = ( const void * ) BSTreeFind (
                                            & ( self -> banana ),
                                            Key,
                                            BananaFinder
                                            );

    if ( BananaFound == NULL ) {
        RCt = XFS_RC ( rcNotFound );
    }
    else {
        * Item = BananaFound;
    }

    return RCt;
}   /* _DieBananaFind_NoLock () */

static
rc_t CC
_DieBananaAdd_NoLock ( 
                    const struct _DieBanana * self,
                    struct BSTNode * Item,
                    _DieBananaAdder BananaAdder
)
{
    XFS_CAN ( self )
    XFS_CAN ( Item )
    XFS_CAN ( BananaAdder )

    return BSTreeInsert ( ( BSTree * ) & ( self -> banana ), Item, BananaAdder );
}   /* _DieBananaAdd_NoLock () */

static
rc_t CC
_DieBananaForEach_NoLock ( 
                    const struct _DieBanana * self,
                    void * Data,
                    _DieBananaEacher BananaEacher
)
{
    XFS_CAN ( self )
    XFS_CAN ( BananaEacher )

    BSTreeForEach ( ( BSTree * ) & ( self -> banana ), false, BananaEacher, Data );

    return 0;
}   /* _DieBananaForEach_NoLock () */

static
rc_t CC
_DieBananaLock ( const struct _DieBanana * self )
{
    struct _DieBanana * Banana = ( struct _DieBanana * ) self;

    return KLockAcquire ( Banana -> mutabor );
}   /* _DieBananaLock () */

static
rc_t CC
_DieBananaUnlock ( const struct _DieBanana * self )
{
    struct _DieBanana * Banana = ( struct _DieBanana * ) self;

    return KLockUnlock ( Banana -> mutabor );
}   /* _DieBananaUnlock () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSGapProject                                                     */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static const char * _sGapProject_classname = "GapProject";

struct XFSGapProject {
    struct BSTNode node;
    KRefcount refcount;

    bool good;

    uint32_t project_id;

    const char * pass;
    size_t pass_size;
    const char * root;
    const char * name;
    const char * display_name;
    const char * workspace;

    const struct KKey * key;
};

static
rc_t CC
_GapProjectDispose ( const struct XFSGapProject * self )
{
    struct XFSGapProject * Project = ( struct XFSGapProject * ) self;

    if ( Project == NULL ) {
        return 0;
    }

    Project -> good = false;

    Project -> project_id = 0;

    KRefcountWhack ( & ( Project -> refcount ), _sGapProject_classname );

    if ( Project -> key != NULL ) {
        free ( ( struct KKey * )  Project -> key );
        Project -> key = NULL;
    }

    if ( Project -> pass != NULL ) {
        free ( ( char * ) Project -> pass );
        Project -> pass = NULL;
    }
    Project -> pass_size = 0;

    if ( Project -> root != NULL ) { 
        free ( ( char * ) Project -> root );
        Project -> root = NULL;
    }

    if ( Project -> name != NULL ) { 
        free ( ( char * ) Project -> name );
        Project -> name = NULL;
    }

    if ( Project -> display_name != NULL ) { 
        free ( ( char * ) Project -> display_name );
        Project -> display_name = NULL;
    }

    if ( Project -> workspace != NULL ) { 
        free ( ( char * ) Project -> workspace );
        Project -> workspace = NULL;
    }

    free ( Project );

    return 0;
}   /* _GapProjectDispose () */


/*  Some useful forward :LOL:
 */
static rc_t CC _GetRepository
        ( const struct KRepository ** Repository, uint32_t ProjectId ); 

static rc_t CC _ReadProjectPassAndKey
        ( const struct XFSGapProject * self, const struct KRepository * Repository );

static rc_t CC _ReadProjectNames
        ( const struct XFSGapProject * self, const struct KRepository * Repository );

static 
rc_t CC
_GapProjectMake (
                const struct XFSGapProject ** Project,
                uint32_t ProjectId
)
{
    rc_t RCt;
    struct XFSGapProject * TheProject;
    const struct KRepository * Repository;

    RCt = 0;
    TheProject = NULL;
    Repository = NULL;

    XFS_CSAN ( Project )
    XFS_CAN ( Project )

    TheProject = calloc ( 1, sizeof ( struct XFSGapProject ) );
    if ( TheProject == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
            /* until it is good */
        TheProject -> good = false;

        TheProject -> project_id = ProjectId;

        TheProject -> key = NULL;

        KRefcountInit (
                    & ( TheProject -> refcount ),
                    1,
                    _sGapProject_classname,
                    "GapProjectMake",
                    "GapProject"
                    );

        RCt = _GetRepository ( & Repository, TheProject -> project_id );
        if ( RCt == 0 ) {
            RCt = _ReadProjectNames ( TheProject, Repository );
            if ( RCt == 0 ) {
                if ( TheProject -> project_id != XFS_PUBLIC_PROJECT_ID ) {
                    RCt = _ReadProjectPassAndKey ( TheProject, Repository );
                }
                if ( RCt == 0 ) {
                    TheProject -> good = true;

                    * Project = TheProject;
                }
            }

            KRepositoryRelease ( Repository );
        }
    }

    if ( RCt != 0 ) {
        * Project = NULL;

        if ( TheProject != 0 ) {
            _GapProjectDispose ( TheProject );
        }
    }

    return RCt;
}   /* _GapProjectMake () */

LIB_EXPORT
rc_t CC
XFSGapProjectAddRef ( const struct XFSGapProject * self )
{
    rc_t RCt;
    int RefC;

    RCt = 0;
    RefC = 0;

    XFS_CAN ( self );

    RefC = KRefcountAdd (
                        & ( self -> refcount ),
                        _sGapProject_classname
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
}   /* XFSGapProjectAddRef () */

LIB_EXPORT
rc_t CC
XFSGapProjectRelease ( const struct XFSGapProject * self )
{
    rc_t RCt;
    int RefC;

    RCt = 0;
    RefC = 0;

    XFS_CAN ( self );

    RefC = KRefcountDrop (
                        & ( self -> refcount ),
                        _sGapProject_classname
                        );
    switch ( RefC ) {
        case krefOkay :
        case krefZero :
                RCt = 0;
                break;
        case krefWhack :
                RCt = _GapProjectDispose ( self );
                break;
        case krefNegative :
                RCt = XFS_RC ( rcInvalid );
                break;
        default :
                RCt = XFS_RC ( rcUnknown );
                break;
    }

    return RCt;
}   /* XFSGapProjectRelease () */

LIB_EXPORT
rc_t CC
XFSGapProjectIsValid ( const struct XFSGapProject * self )
{
    return self != NULL ? ( self -> good  ) : false;
}   /* XFSGapProjectIsValid () */

LIB_EXPORT
rc_t CC
XFSGapProjectId ( const struct XFSGapProject * self, uint32_t * Id )
{
    XFS_CSA ( Id, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Id )

    * Id = self -> project_id;

    return 0;
}   /* XFSGapProjectId () */

LIB_EXPORT
rc_t CC
XFSGapProjectRepositoryName (
                        const struct XFSGapProject * self,
                        const char ** RepositoryName
)
{
    XFS_CSAN ( RepositoryName )
    XFS_CAN ( self )
    XFS_CAN ( RepositoryName )

    * RepositoryName = self -> name;

    return 0;
}   /* XFSGapProjectRepositoryName () */

LIB_EXPORT
rc_t CC
XFSGapProjectRepositoryDisplayName (
                        const struct XFSGapProject * self,
                        const char ** DisplayName
)
{
    XFS_CSAN ( DisplayName )
    XFS_CAN ( self )
    XFS_CAN ( DisplayName )

    * DisplayName = self -> display_name;

    return 0;
}   /* XFSGapProjectRepositoryDisplayName () */


LIB_EXPORT
rc_t CC
XFSGapProjectRepositoryRoot (
                        const struct XFSGapProject * self,
                        const char ** RepositoryRoot
)
{
    XFS_CSAN ( RepositoryRoot )
    XFS_CAN ( self )
    XFS_CAN ( RepositoryRoot )

    * RepositoryRoot = self -> root;

    return 0;
}   /* XFSGapProjectRepositoryRoot () */

LIB_EXPORT
rc_t CC
XFSGapProjectEncriptionKey (
                        const struct XFSGapProject * self,
                        const struct KKey ** EncriptionKey
)
{
    rc_t RCt = 0;

    XFS_CSAN ( EncriptionKey )
    XFS_CAN ( self )
    XFS_CAN ( EncriptionKey )

    if ( RCt == 0 ) {
        * EncriptionKey = self -> key;
    }

    return RCt;
}   /* XFSGapProjectEncriptionKey () */

LIB_EXPORT
rc_t CC
XFSGapProjectPassword (
                        const struct XFSGapProject * self,
                        const char ** Password,
                        size_t * PasswordSize
)
{
    rc_t RCt = 0;

    XFS_CSAN ( Password )
    XFS_CSA ( PasswordSize, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Password )
    XFS_CAN ( PasswordSize )

    if ( RCt == 0 ) {
        * Password = self -> pass;
        * PasswordSize = self -> pass_size;
    }

    return RCt;
}   /* XFSGapProjectPassword () */

LIB_EXPORT
rc_t CC
XFSGapProjectWorkspace (
                        const struct XFSGapProject * self,
                        const char ** Workspace
)
{
    XFS_CSAN ( Workspace )
    XFS_CAN ( self )
    XFS_CAN ( Workspace )

    * Workspace = self -> workspace;

    return 0;
}   /* XFSGapProjectWorkspace () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* Weird stuff                                                       */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
static
rc_t CC
_GetRepository (
                const struct KRepository ** Repository,
                uint32_t ProjectId
)
{
    rc_t RCt, TmpRCt;
    struct KRepository * TheRepository;
    KRepositoryVector RepositoryVec;
    const struct KRepositoryMgr * RepositoryMgr;
    uint32_t llp, qty, prj;
    char Bf [ XFS_SIZE_128 ];

    RCt = 0;
    TmpRCt = 0;
    TheRepository = NULL;
    RepositoryMgr = NULL;
    llp = qty = prj = 0;
    * Bf = 0;

    XFS_CSAN ( Repository )
    XFS_CAN ( Repository )

    RCt = KConfigMakeRepositoryMgrRead (
                                    XFS_Config_MHR (),
                                    & RepositoryMgr
                                    );
    if ( RCt == 0 ) {

        RCt = KRepositoryMgrUserRepositories (
                                            RepositoryMgr,
                                            & RepositoryVec
                                            );
        if ( RCt == 0 ) {
            qty = VectorLength ( & RepositoryVec );
            for ( llp = 0; llp < qty; llp ++ ) {

                TheRepository = VectorGet ( & RepositoryVec, llp );
                if ( TheRepository == NULL ) {
                    continue;
                }

                TmpRCt = KRepositoryName (
                                        TheRepository,
                                        Bf,
                                        sizeof ( Bf ),
                                        NULL
                                        );
                if ( TmpRCt != 0 ) {
                    continue;
                }

                if ( ProjectId == XFS_PUBLIC_PROJECT_ID ) {
                    if ( strcmp ( Bf, "public" ) != 0 ) {
                        continue;
                    }
                }
                else {
                    TmpRCt = KRepositoryProjectId (
                                                TheRepository,
                                                & prj
                                                );
                    if ( TmpRCt != 0 ) {
                        continue;
                    }

                    if ( prj != ProjectId ) {
                        continue;
                    }
                }

                    /* So, here we do have our repository */
                RCt = KRepositoryAddRef ( TheRepository );
                if ( RCt == 0 ) {
                    * Repository = TheRepository;
                }

                break;
            }

            KRepositoryVectorWhack ( & RepositoryVec );
        }

        KRepositoryMgrRelease ( RepositoryMgr );
    }

    if ( * Repository == NULL ) {
        RCt = XFS_RC ( rcNotFound );
    }

    return RCt;
}   /*  _GetRepository () */

static
rc_t CC
_ReadRepositoryCharValue (
                        const struct KRepository * Repository,
                        char ** RetValue,
                        rc_t ( CC * Reader ) ( 
                                        const struct KRepository * self,
                                        char * Buffer,
                                        size_t BufferSize,
                                        size_t * RetSize
                                        )
)
{
    rc_t RCt;
    char Bf [ XFS_SIZE_1024 ];
    char * TheRetVal;
    size_t TheRetValSize;

    RCt = 0;
    * Bf = 0;
    TheRetVal = NULL;
    TheRetValSize = 0;

    XFS_CSAN ( RetValue )
    XFS_CAN ( Repository )
    XFS_CAN ( RetValue )
    XFS_CAN ( Reader )

    RCt = Reader ( Repository, Bf, sizeof ( Bf ), & TheRetValSize );
    if ( RCt == 0 ) {
        if ( TheRetValSize != 0 ) {
            RCt = XFS_StrDup ( Bf, ( const char ** ) ( & TheRetVal ) );
            if ( RCt == 0 ) {
                * RetValue = TheRetVal;
            } 
        }
        else {
            RCt = XFS_RC ( rcInvalid );
        }
    }

    return RCt;
}   /* _ReadRepositoryCharValue () */

static
rc_t CC
_ReadProjectNames (
                    const struct XFSGapProject * self,
                    const struct KRepository * Repository
)
{
    rc_t RCt;
    struct XFSGapProject * Project;
    char BF [ XFS_SIZE_1024 ];
    size_t NR;

    RCt = 0;
    Project = ( struct XFSGapProject * ) self;
    * BF = 0;
    NR = 0;

    if ( Project -> root != NULL ) {
        free ( ( char * ) Project -> root );
        Project -> root = NULL;
    }

    if ( Project -> name != NULL ) {
        free ( ( char * ) Project -> name );
        Project -> name = NULL;
    }

    if ( Project -> display_name != NULL ) {
        free ( ( char * ) Project -> display_name );
        Project -> display_name = NULL;
    }

    RCt = _ReadRepositoryCharValue (
                                    Repository,
                                    ( char ** ) & ( Project -> root ),
                                    KRepositoryRoot
                                    );
    if ( RCt == 0 ) {
        RCt = _ReadRepositoryCharValue (
                                    Repository,
                                    ( char ** ) & ( Project -> name ),
                                    KRepositoryName
                                    );
        if ( RCt == 0 ) {
            RCt = _ReadRepositoryCharValue (
                            Repository,
                            ( char ** ) & ( Project -> display_name ),
                            KRepositoryDisplayName
                            );

            if ( RCt == 0 ) {
                RCt = string_printf (
                                BF,
                                sizeof ( BF ) - 1,
                                & NR,
                                "%s/workspace",
                                Project -> root
                                );
                if ( RCt == 0 ) {
                    RCt = XFS_StrDup ( BF, & ( Project -> workspace ) );
                }
            }
        }
    }

    return RCt;
}   /* _ReadProjectNames () */

static
rc_t CC
_ReadProjectPassAndKey (
                    const struct XFSGapProject * self,
                    const struct KRepository * Repository
)
{
    rc_t RCt;
    struct XFSGapProject * Project;
    struct KKey * Key;
    char Pass [ XFS_SIZE_4096 ];
    size_t PassSize;
    char * ThePass;

    RCt = 0;
    Project = ( struct XFSGapProject * ) self;
    Key = NULL;
    * Pass = 0;
    ThePass = NULL;
    PassSize = 0;

    XFS_CAN ( Project )

    RCt = KRepositoryEncryptionKey (
                                Repository,
                                Pass,
                                sizeof ( Pass ),
                                & PassSize
                                );
    if ( RCt == 0 ) {
        ThePass = string_dup ( Pass, PassSize );
        if ( ThePass == NULL ) {
            RCt = XFS_RC ( rcExhausted );
        }
        else {
            Key = calloc ( 1, sizeof ( struct KKey ) );
            if ( Key == NULL ) {
                RCt = XFS_RC ( rcExhausted );
            }
            else {
                RCt = KKeyInitRead (
                                    Key,
                                    kkeyAES128,
                                    ThePass,
                                    PassSize
                                    );
                if ( RCt == 0 ) {
                    Project -> key = Key;
                    Project -> pass = ThePass;
                    Project -> pass_size = PassSize;
                }
            }
        }
    }

    return RCt;
}   /* _ReadProjectPassAndKey () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSGapKart                                                        */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
    /*))    No refcounter, cuz ... don't believe in multithreading
     //     or ... may be we already ready for that :D
    ((*/
struct _GapKartItem {
    struct BSTNode node;

    uint32_t project;

    const char * aooi; /* Accession Or Object Id */
    const char * name;
};

static
rc_t CC
_GapKartItemDispose ( const struct _GapKartItem * self )
{
    struct _GapKartItem * Item = ( struct _GapKartItem * ) self;

    if ( Item != NULL ) {
        if ( Item -> aooi != NULL ) {
            free ( ( char * ) Item -> aooi );
            Item -> aooi = NULL;
        }

        if ( Item -> name != NULL ) {
            free ( ( char * ) Item -> name );
            Item -> name = NULL;
        }

        Item -> project = 0;

        free ( Item );
    }

    return 0;
}   /* GapKartItemDispose () */

    /*))    Name could be NULL, accession will be instead
     ((*/
static
rc_t CC
_GapKartItemMake (
                const struct _GapKartItem ** Item,
                uint32_t Project,
                const char * AccessionOrObject,
                const char * Name
)
{
    rc_t RCt;
    struct _GapKartItem * TheItem;

    RCt = 0;
    TheItem = NULL;

    XFS_CSAN ( Item )
    XFS_CAN ( Item )
    XFS_CAN ( AccessionOrObject )

    TheItem = calloc ( 1, sizeof ( struct _GapKartItem ) );
    if ( TheItem == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        TheItem -> project = Project;
        RCt = XFS_StrDup ( AccessionOrObject, & ( TheItem -> aooi ) );
        if ( RCt == 0 ) {
            if ( Name != NULL ) {
                RCt = XFS_StrDup ( Name, & ( TheItem -> name ) );
            }
            else {
                RCt = XFS_StrDup ( AccessionOrObject, & ( TheItem -> name ) );
            }
            if ( RCt == 0 ) {
                * Item = TheItem;
            }
        }
    }

    if ( RCt != 0 ) {
        * Item = NULL;

        if ( TheItem != NULL ) {
            _GapKartItemDispose ( TheItem );
        }
    }

    return RCt;
}   /* _GapKartItemMake () */

static
rc_t CC
_GapKartItemName (
                const struct _GapKartItem * self,
                const char ** Name
)
{
    XFS_CSAN ( Name )
    XFS_CAN ( self )
    XFS_CAN ( Name )

    * Name = self -> name == NULL ? self -> aooi : self -> name;

    return 0;
}   /* _GapKartItemName () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSGapKart                                                        */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
struct XFSGapKart {
    KRefcount refcount;

    bool good;

    const struct XFS_LIdx_ZHR * projects_idx;

    const struct _DieBanana * items;

    const char * dir;
    const char * name;
    const char * path;
};

static const char * _sGapKart_classname = "GapKart";

static
rc_t CC
_GapKartDispose ( const struct XFSGapKart * self )
{
    struct XFSGapKart * Kart = ( struct XFSGapKart * ) self;

    if ( Kart != NULL ) {
        Kart -> good = false;

        KRefcountWhack ( & ( Kart -> refcount ), _sGapKart_classname );

        if ( Kart -> items != NULL ) {
            _DieBananaDispose ( Kart -> items );
            free ( ( struct _DieBanana * ) Kart -> items );
            Kart -> items = NULL;
        }

        if ( Kart -> projects_idx != NULL ) {
            XFS_LIdxDispose_ZHR ( Kart -> projects_idx );
            Kart -> projects_idx = NULL;
        }

        if ( Kart -> dir != NULL ) {
            free ( ( char * ) Kart -> dir );
            Kart -> dir = NULL;
        }

        if ( Kart -> name != NULL ) {
            free ( ( char * ) Kart -> name );
            Kart -> name = NULL;
        }

        if ( Kart -> path != NULL ) {
            free ( ( char * ) Kart -> path );
            Kart -> path = NULL;
        }

        free ( Kart );
    }

    return 0;
}   /* _GapKartDispose () */

static
void CC
_KartItemWhackCallback ( struct BSTNode * Node, void * unused )
{
    if ( Node != NULL ) {
        _GapKartItemDispose ( ( struct _GapKartItem * ) Node );
    }
}   /* _KartItemWhackCallback () */

static rc_t CC _GapKartLoad ( struct XFSGapKart * self );

static
rc_t CC
_GapKartMake (
            const struct XFSGapKart ** Kart,
            const char * Dir,
            const char * Name
)
{
    rc_t RCt;
    struct XFSGapKart * TheKart;
    char Buf [ XFS_SIZE_1024 ];
    size_t NumW;

    RCt = 0;
    TheKart = NULL;
    * Buf = 0;
    NumW = 0;

    XFS_CSAN ( Kart )
    XFS_CAN ( Kart )
    XFS_CAN ( Dir )
    XFS_CAN ( Name )

    TheKart = calloc ( 1, sizeof ( struct XFSGapKart ) );
    if ( TheKart == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
            /* until it is good */
        TheKart -> good = false;

        KRefcountInit (
                        & ( TheKart -> refcount ),
                        1,
                        _sGapKart_classname,
                        "GapKartMake",
                        "GapKart"
                        );

            /*  GapKart Items initialisation
             */
        RCt = _DieBananaMake (
                            & ( TheKart -> items ),
                            _KartItemWhackCallback
                            );
        if ( RCt == 0 ) {
            RCt = XFS_LIdxMake_ZHR ( & ( TheKart -> projects_idx ) );
            if ( RCt == 0 ) {
                RCt = XFS_StrDup ( Dir, & ( TheKart -> dir ) ); 
                if ( RCt == 0 ) {
                    RCt = XFS_StrDup ( Name, & ( TheKart -> name ) ); 
                    if ( RCt == 0 ) {
                        RCt = string_printf (
                                            Buf,
                                            sizeof ( Buf ),
                                            & NumW,
                                            "%s/%s",
                                            Dir,
                                            Name
                                            );
                        if ( RCt == 0 ) {
                            RCt = XFS_StrDup (
                                            Buf,
                                            & ( TheKart -> path )
                                            ); 
                            if ( RCt == 0 ) {
                                RCt = _GapKartLoad ( TheKart );
                                if ( RCt == 0 ) {
                                    /*  We do not add reference here
                                     */
                                    * Kart = TheKart;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if ( RCt != 0 ) {
        * Kart = NULL;

        if ( TheKart != NULL ) {
            _GapKartDispose ( TheKart );
        }
    }

    return RCt;
}   /* _GapKartMake () */

static
int64_t CC
_KartItemAddCallback ( const BSTNode * N1, const BSTNode * N2 )
{
    return XFS_StringCompare4BST_ZHR (
            ( ( struct _GapKartItem * ) N1 ) -> name,
            ( ( struct _GapKartItem * ) N2 ) -> name
            );
}   /* _KartItemAddCallback () */

static
rc_t CC
_AddKartItem (
            struct XFSGapKart * self,
            const struct KartItem * TheKartItem
)
{
    rc_t RCt;
    const struct _GapKartItem * KItem;
    uint64_t ProjectId;
    const char * AccessionOrObject;
    const char * Name;
    const struct String * TheString;

    RCt = 0;
    KItem = NULL;
    ProjectId = 0;
    AccessionOrObject = NULL;
    Name = NULL;
    TheString = NULL;

    XFS_CAN ( self )
    XFS_CAN ( TheKartItem )


    RCt = KartItemProjIdNumber ( TheKartItem, & ProjectId );
    if ( RCt == 0 ) {
        RCt = KartItemAccession ( TheKartItem, & TheString );
        if ( RCt == 0 ) {
            if ( TheString -> len != 0 ) {
                RCt = XFS_SStrDup ( TheString, & AccessionOrObject );
            }
            else {
                RCt = KartItemItemId ( TheKartItem, & TheString );
                if ( TheString -> len != 0 ) {
                    RCt = XFS_SStrDup ( TheString, & AccessionOrObject );
                }
            }
        }
    }

    if ( AccessionOrObject == NULL ) {
        RCt = XFS_RC ( rcInvalid );
    }

    if ( RCt == 0 ) {
        RCt = KartItemName ( TheKartItem, & TheString );
        if ( TheString -> len != 0 ) {
            RCt = XFS_SStrDup ( TheString, & Name );
        }
        else {
            Name = NULL;
        }
    }

    if ( RCt == 0 ) {
        RCt = _GapKartItemMake (
                                & KItem,
                                ProjectId,
                                AccessionOrObject,
                                Name
                                );
        if ( RCt == 0 ) {
            RCt = XFS_LIdxAdd_ZHR ( self -> projects_idx,  ProjectId );
            if ( RCt == 0 ) {
                RCt = _DieBananaAdd_NoLock (
                                        self -> items,
                                        ( struct BSTNode * ) KItem,
                                        _KartItemAddCallback
                                        );

            }
        }
    }

    if ( AccessionOrObject != NULL ) {
        free ( ( char * ) AccessionOrObject );
        AccessionOrObject = NULL;
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

    RCt = 0;
    TheKart = NULL;
    IsKart = false;
    Directory = NULL;

    XFS_CAN ( self )
    XFS_CAN ( self -> path )

    RCt = KDirectoryNativeDir ( & Directory );
    if ( RCt == 0 ) {
        RCt = KartMake ( Directory, self -> path, & TheKart, & IsKart );
        if ( RCt == 0 ) {
            if ( ! IsKart ) {
                RCt = XFS_RC ( rcInvalid );
            }
            else {
                RCt = _DieBananaLock ( self -> items );
                if ( RCt == 0 ) {
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
                    _DieBananaUnlock ( self -> items );
                }
            }

            if ( TheKart != NULL ) {
                KartRelease ( TheKart );
            }
        }

        KDirectoryRelease ( Directory );
    }

    return RCt;
}   /* _GapKartLoad () */

LIB_EXPORT
rc_t CC
XFSGapKartAddRef ( const struct XFSGapKart * self )
{
    rc_t RCt;
    int RefC;

    RCt = 0;
    RefC = 0;

    XFS_CAN ( self );

    RefC = KRefcountAdd (
                        & ( self -> refcount ),
                        _sGapKart_classname
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
}   /* XFSGapKartAddRef () */

LIB_EXPORT
rc_t CC
XFSGapKartRelease ( const struct XFSGapKart * self )
{
    rc_t RCt;
    int RefC;

    RCt = 0;
    RefC = 0;

    XFS_CAN ( self );

    RefC = KRefcountDrop (
                        & ( self -> refcount ),
                        _sGapKart_classname
                        );
    switch ( RefC ) {
        case krefOkay :
        case krefZero :
                RCt = 0;
                break;
        case krefWhack :
                RCt = _GapKartDispose ( self );
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
void
_KartItemListAllCallback ( BSTNode * Node, void * Data )
{
    struct VNamelist * List;
    struct _GapKartItem * Item;
    const char * Name;

    List = ( struct VNamelist * ) Data;
    Item = ( struct _GapKartItem * ) Node;

    if ( List != NULL && Item != NULL ) {
        if ( _GapKartItemName ( Item, & Name ) == 0 ) {
            VNamelistAppend ( List, Name );
        }
    }
}   /* _KartItemListAllCallback () */

LIB_EXPORT
rc_t CC
XFSGapKartList (
                const struct XFSGapKart * self,
                struct KNamelist ** List
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
        RCt = _DieBananaLock ( self -> items );
        if ( RCt == 0 ) {
            _DieBananaForEach_NoLock (
                                    self -> items,
                                    ( void * ) xList,
                                    _KartItemListAllCallback
                                    );

            RCt = VNamelistToNamelist ( xList, List );


            _DieBananaUnlock ( self -> items );
        }
        VNamelistRelease ( xList );
    }

    return RCt;
}   /* XFSGapKartList () */

static
void
_KartItemListCallback ( BSTNode * Node, void * Data )
{
    struct _GapKartItem * Item;
    struct LI_Stru * Stru;
    const char * Name;

    Stru = ( struct LI_Stru * ) Data;
    Item = ( struct _GapKartItem * ) Node;

    if ( Item != NULL && Stru != NULL ) {
        if ( Stru -> project_id == 0
            || Item -> project == Stru -> project_id ) {
            if ( Stru -> flag ) {
                if ( _GapKartItemName ( Item, & Name ) == 0 ) {
                    VNamelistAppend ( Stru -> list, Name );
                }
            }
            else {
                VNamelistAppend ( Stru -> list, Item -> aooi );
            }
        }
    }
}   /* _KartItemListCallback () */

LIB_EXPORT
rc_t CC
_GapKartList (
                const struct XFSGapKart * self,
                struct KNamelist ** List,
                uint32_t ProjectId,
                bool ListNames
)
{
    rc_t RCt;
    struct VNamelist * xList;
    struct LI_Stru Stru;

    RCt = 0;
    xList = NULL;
    Stru . project_id = ProjectId;
    Stru . list = NULL;
    Stru . flag = ListNames;

    XFS_CSAN ( List )
    XFS_CAN ( self )
    XFS_CAN ( List )

    RCt = VNamelistMake ( & xList, 16 /* ????? */ );
    if ( RCt == 0 ) {
        Stru . list = xList;

        RCt = _DieBananaLock ( self -> items );
        if ( RCt == 0 ) {
            _DieBananaForEach_NoLock (
                                    self -> items,
                                    ( void * ) & Stru,
                                    _KartItemListCallback
                                    );

            RCt = VNamelistToNamelist ( xList, List );


            _DieBananaUnlock ( self -> items );
        }
        VNamelistRelease ( xList );
    }

    return RCt;
}   /* _GapKartList () */

LIB_EXPORT
rc_t CC
XFSGapKartListForProject (
                const struct XFSGapKart * self,
                struct KNamelist ** List,
                uint32_t ProjectId
)
{
    return _GapKartList ( self, List, ProjectId, true );
}   /* XFSGapKartListForProject () */

LIB_EXPORT
rc_t CC
_GapKartListIds (
                const struct XFSGapKart * self,
                struct KNamelist ** List,
                uint32_t ProjectId
)
{
    return _GapKartList ( self, List, ProjectId, false );
}   /* _GapKartListIds () */

/*  Returns the name of directory, where kart file is
*/
LIB_EXPORT
rc_t CC
XFSGapKartDirectory (
                const struct XFSGapKart * self,
                const char ** Dir
)
{
    XFS_CSAN ( Dir )
    XFS_CAN ( self )
    XFS_CAN ( Dir )

    * Dir = self -> dir;

    return 0;
}   /* XFSGapKartDirectory () */

/*  Returns the name of file, where kart is stored
*/
LIB_EXPORT
rc_t CC
XFSGapKartName (
                const struct XFSGapKart * self,
                const char ** Name
)
{
    XFS_CSAN ( Name )
    XFS_CAN ( self )
    XFS_CAN ( Name )

    * Name = self -> name;

    return 0;
}   /* XFSGapKartName () */

/*  Returns the name of file, where kart is stored
*/
LIB_EXPORT
rc_t CC
XFSGapKartPath (
                const struct XFSGapKart * self,
                const char ** Path
)
{
    XFS_CSAN ( Path )
    XFS_CAN ( self )
    XFS_CAN ( Path )

    * Path = self -> path;

    return 0;
}   /* XFSGapKartPath () */

LIB_EXPORT
bool CC
XFSGapKartHasDataForProject (
                            const struct XFSGapKart * self,
                            uint32_t ProjectId
)
{
    if ( self != NULL ) {
        return ProjectId == 0
                ? true
                : XFS_LIdxHas_ZHR ( self -> projects_idx, ProjectId )
                ;
    }
    return false;
}   /* XFSGapKartHasDataForProject () */

static
int64_t CC
_ItemFindCallback ( const void * Item, const struct BSTNode * Node )
{
    const char * Str1, * Str2;

    Str1 = ( const char * ) Item;
    Str2 = Node == NULL
                    ? ""
                    : ( ( const struct _GapKartItem * ) Node ) -> name
                    ;

    return XFS_StringCompare4BST_ZHR ( Str1, Str2 );
}   /* _ItemFindCallback () */

    /* IMPORTANT : JOJOBA - thread unsafe ... cuz KartItem is thread
     *             unsafe construct
     */
static
rc_t CC
_KartFindItem (
                const struct XFSGapKart * self,
                const struct _GapKartItem ** Item,
                const char * ItemName
)
{
    rc_t RCt;
    struct _GapKartItem * TheItem;

    RCt = 0;
    TheItem = NULL;

    XFS_CSAN ( Item )
    XFS_CAN ( self )
    XFS_CAN ( Item )
    XFS_CAN ( ItemName )

    RCt = _DieBananaLock ( self -> items );
    if ( RCt == 0 ) {
        RCt = _DieBananaFind_NoLock (
                                    self -> items,
                                    ( const void ** ) & TheItem,
                                    ( const void * ) ItemName,
                                    _ItemFindCallback
                                    );
        if ( RCt == 0 ) {
            * Item = TheItem;
        }

        _DieBananaUnlock ( self -> items );
    }

    return RCt;
}   /* _KartFindItem () */

LIB_EXPORT
rc_t CC
XFSGapKartItemAttributes (
                        const struct XFSGapKart * self,
                        uint32_t * ProjectId,
                        char ** AccessionOrId,
                        const char * ItemName
)
{
    rc_t RCt;
    const struct _GapKartItem * Item;
    const char * AOI;

    RCt = 0;
    Item = NULL;
    AOI = NULL;

    XFS_CSAN ( AccessionOrId )
    XFS_CSA ( ProjectId, 0 )
    XFS_CAN ( self )
    XFS_CAN ( ProjectId )
    XFS_CAN ( AccessionOrId )
    XFS_CAN ( ItemName )

    RCt = _KartFindItem ( self, & Item, ItemName );
    if ( RCt == 0 ) {
        RCt = XFS_StrDup ( Item -> aooi, & AOI );
        if ( RCt == 0 ) {
            * AccessionOrId = ( char * ) AOI;
            * ProjectId = Item -> project;
        }
    }

    return RCt;
}   /* XFSGapKartItemAttributes () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* _GapDepot                                                         */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
struct _GapDepot {
        /*  For forsely update
         */
    struct KLock * mutabor;

        /*  As doctor prescribed : Projects, Kards, and Objects
         */
    const struct _DieBanana * projects;
    const struct _DieBanana * karts;

    const char * public_files;
    const char * kart_files;

    uint64_t version;
};

static struct _GapDepot * _sDepot = NULL;

static
struct _GapDepot * CC
_DepotGet ()
{
    return _sDepot;
}   /* _DepotGet () */

static
void CC
_DepotProjectWhackCallback ( struct BSTNode * Node, void * unused )
{
    if ( Node != NULL ) {
        XFSGapProjectRelease ( ( const struct XFSGapProject * ) Node );
    }
}   /* _DepotProjectWhackCallback () */

static
void CC
_DepotKartWhackCallback ( struct BSTNode * Node, void * unused )
{
    struct _DieBananable * Bananable;
    const struct XFSGapKart * Kart;

    Bananable = ( struct _DieBananable * ) Node;
    Kart = NULL;

    if ( Bananable != NULL ) {
        _DieBananableGet ( Bananable, ( void ** ) & Kart );
        XFSGapKartRelease ( Kart );
        _DieBananableDispose ( Bananable );
    }
}   /* _DepotKartWhackCallback () */

static
rc_t CC
_GapDepotDispose ( struct _GapDepot * self )
{
    if ( self == NULL ) {
        return 0;
    }

        /*)     We are stopping resolver
         (*/
    XFSGapResolverStop ();
    XFSGapResolverDispose ();

        /*  GapProject disposing
         */
    if ( self -> projects != NULL ) {
        _DieBananaDispose ( self -> projects );
        free ( ( struct _DieBanana * ) self -> projects );
        self -> projects = NULL;
    }

    if ( self -> karts != NULL ) {
        _DieBananaDispose ( self -> karts );
        free ( ( struct _DieBanana * ) self -> karts );
        self -> karts = NULL;
    }

    if ( self -> public_files != NULL ) {
        free ( ( char * ) self -> public_files );
        self -> public_files = NULL;
    }

    if ( self -> kart_files != NULL ) {
        free ( ( char * ) self -> kart_files );
        self -> kart_files = NULL;
    }

    if ( self -> mutabor != NULL ) {
        KLockRelease ( self -> mutabor );
        self -> mutabor = NULL;
    }

    free ( self );

    return 0;
}   /* _GapDepotDispose () */

static rc_t CC _GapGetProject (
                                struct _GapDepot * Depot,
                                const struct XFSGapProject ** Project,
                                uint32_t ProjectId
                                );
static rc_t CC _GapKartDepotRefresh ( struct _GapDepot * Depot );

static
rc_t CC
_GapDepotMakePaths ( struct _GapDepot * Depot )
{
    rc_t RCt;
    char * UserConfigDir;
    const struct XFSPath * Path;
    const struct XFSGapProject * Project;

    RCt = 0;
    UserConfigDir = NULL;
    Path = NULL;
    Project = NULL;

    XFS_CAN ( Depot )

    if ( Depot -> public_files != NULL ) {
        free ( ( char * ) Depot -> public_files );
        Depot -> public_files = NULL;
    }

    if ( Depot -> kart_files != NULL ) {
        free ( ( char * ) Depot -> kart_files );
        Depot -> kart_files = NULL;
    }

    RCt = XFS_UserConfigDir_ZHR ( & UserConfigDir );
    if ( RCt == 0 ) {
        RCt= XFSPathMake (
                        & Path,
                        false,
                        "%s/%s",
                        UserConfigDir,
                        KART_FILES_NAME
                        );
        if ( RCt == 0 ) {
            RCt = XFS_StrDup (
                            XFSPathGet ( Path ),
                            & ( Depot -> kart_files )
                            );

            XFSPathRelease ( Path );
        }

        free ( UserConfigDir );
    }

    if ( RCt == 0 ) {
        RCt = _GapGetProject ( Depot, & Project, XFS_PUBLIC_PROJECT_ID );
        if ( RCt == 0 ) {
            RCt = XFSGapProjectRepositoryRoot (
                                    Project,
                                    ( const char ** ) & UserConfigDir
                                    );
            if ( RCt == 0 ) {
                RCt = XFS_StrDup (
                                UserConfigDir,
                                & ( Depot -> public_files )
                                );
            }

            XFSGapProjectRelease ( Project );
        }
    }

    return RCt;
}   /* _GapDepotMakePaths () */

static
rc_t CC
_GapDepotMake ( struct _GapDepot ** Depot )
{
    rc_t RCt;
    struct _GapDepot * TheDepot;

    RCt = 0;
    TheDepot = NULL;

    XFS_CSAN ( Depot )
    XFS_CAN ( Depot )

    TheDepot = calloc ( 1, sizeof ( struct _GapDepot ) );
    if ( TheDepot == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = KLockMake ( & ( TheDepot -> mutabor ) );
        if ( RCt == 0 ) {
                /*  GapProjects initialisation
                 */
            RCt = _DieBananaMake (
                                & ( TheDepot -> projects ),
                                _DepotProjectWhackCallback
                                );
            if ( RCt == 0 ) {
                RCt = _DieBananaMake (
                                    & ( TheDepot -> karts ),
                                    _DepotKartWhackCallback
                                    );
                if ( RCt == 0 ) {
                    RCt = _GapDepotMakePaths ( TheDepot );
                    if ( RCt == 0 ) {
                        if ( RCt == 0 ) {
                                /*) Initializing and starting Resolver
                                 (*/
                            RCt = XFSGapResolverInit ();
                            if ( RCt == 0 ) {
                                RCt = XFSGapResolverStart ();
                                if ( RCt == 0 ) {
                                    * Depot = TheDepot;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if ( RCt != 0 ) {
        * Depot = NULL;

        if ( TheDepot != NULL ) {
            _GapDepotDispose ( TheDepot );
        }
    }

    return RCt;
}   /* _GapDepotMake () */

LIB_EXPORT
rc_t CC
XFSGapInit ()
{
    rc_t RCt;
    struct _GapDepot * Depot;

    RCt = 0;
    Depot = NULL;

    if ( _sDepot == NULL ) {
        RCt = _GapDepotMake ( & Depot );
        if ( RCt == 0 ) {
            _sDepot = Depot;
        }
    }

    return RCt;
}   /* XFSGapInit () */

LIB_EXPORT
rc_t CC
XFSGapDispose ()
{
    struct _GapDepot * Dpt = _sDepot;

    if ( Dpt != NULL ) {
        _sDepot = NULL;

        return _GapDepotDispose ( Dpt );
    }

    return 0;
}   /* XFSGapDelete () */

static
int64_t CC
_ProjectAddCallback (
                const struct BSTNode * N1,
                const struct BSTNode * N2
)
{
    struct XFSGapProject * P1 = ( struct XFSGapProject * ) N1;
    struct XFSGapProject * P2 = ( struct XFSGapProject * ) N2;
    return  P1 -> project_id - P2 -> project_id;
}   /* _ProjectAddCallback () */

static
int64_t CC
_ProjectFindCallback ( const void * Item, const struct BSTNode * Node )
{
    uint32_t ItemId, NodeId;

    ItemId = * ( ( uint32_t * ) Item );
    NodeId = Node == NULL
                    ? 0
                    : ( ( struct XFSGapProject * ) Node ) -> project_id
                    ;


    return ItemId - NodeId;
}   /* _ProjectFindCallback () */

static
rc_t CC
_GapGetProject (
                struct _GapDepot * Depot,
                const struct XFSGapProject ** Project,
                uint32_t ProjectId
)
{
    rc_t RCt;
    const struct XFSGapProject * TheProject;

    RCt = 0;
    TheProject = NULL;

    XFS_CSAN ( Project )
    XFS_CAN ( Depot )
    XFS_CAN ( Project )

    RCt = _DieBananaLock ( Depot -> projects );
    if ( RCt == 0 ) {

        RCt = _DieBananaFind_NoLock (
                                    Depot -> projects,
                                    ( const void ** ) & TheProject,
                                    ( const void * ) & ProjectId,
                                    _ProjectFindCallback
                                    );
        if ( GetRCState ( RCt ) == rcNotFound ) {
            RCt = _GapProjectMake ( & TheProject, ProjectId );
            if ( RCt == 0 ) {
                RCt = _DieBananaAdd_NoLock (
                                        Depot -> projects,
                                        ( struct BSTNode * ) TheProject,
                                        _ProjectAddCallback
                                        );
                if ( RCt != 0 ) {
                    _GapProjectDispose ( * Project );

                    * Project = NULL;
                }
            }
        }

        if ( RCt == 0 ) {
            RCt = XFSGapProjectAddRef ( TheProject );
            if ( RCt == 0 ) {
                * Project = TheProject;
            }
        }

        _DieBananaUnlock ( Depot -> projects );
    }

    return RCt;
}   /* _GapGetProject () */

LIB_EXPORT
rc_t CC
XFSGapGetProject (
                const struct XFSGapProject ** Project,
                uint32_t ProjectId
)
{
    return _GapGetProject ( _DepotGet (), Project, ProjectId );
}   /* XFSGapGetProject () */

LIB_EXPORT
bool CC
XFSGapHas ( uint32_t ProjectId )
{
    rc_t RCt;
    struct _GapDepot * Depot;
    const struct XFSGapProject * Project;

    RCt = 0;
    Project = NULL;
    Depot = _DepotGet ();

    XFS_CAN ( Depot )

    RCt = _DieBananaLock ( Depot -> projects );
    if ( RCt == 0 ) {

        RCt = _DieBananaFind_NoLock (
                                    Depot -> projects,
                                    ( const void ** ) & Project,
                                    ( const void * ) & ProjectId,
                                    _ProjectFindCallback
                                    );

        _DieBananaUnlock ( Depot -> projects );
    }

    return RCt == 0;
}   /* XFSGapHas () */

rc_t CC
XFSGapPublicfiles ( const char ** Publicfiles )
{
    struct _GapDepot * Depot = _DepotGet ();

    XFS_CAN ( Depot )

    * Publicfiles = Depot -> public_files;

    return 0;
}   /* XFSGapPublicfiles () */

LIB_EXPORT
rc_t CC
XFSGapKartfiles ( const char ** Kartfiles )
{
    struct _GapDepot * Depot = _DepotGet ();

    XFS_CAN ( Depot )

    * Kartfiles = Depot -> kart_files;

    return 0;
}   /* XFSGapKartfiles () */

static
int64_t CC
_KartAddCallback ( const BSTNode * N1, const BSTNode * N2 )
{
    const struct _DieBananable * Ban1, * Ban2;
    const struct XFSGapKart * Kar1, * Kar2;
    const char * Str1, * Str2;

    Ban1 = ( const struct _DieBananable * ) N1;
    Ban2 = ( const struct _DieBananable * ) N2;
    Kar1 = Kar2 = NULL;
    Str1 = Str2 = NULL;

    if ( _DieBananableGet ( Ban1, ( void ** ) & Kar1 ) == 0 ) {
        Str1 = Kar1 == NULL ? NULL : ( Kar1 -> name );
    }

    if ( _DieBananableGet ( Ban2, ( void ** ) & Kar2 ) == 0 ) {
        Str2 = Kar2 == NULL ? NULL : ( Kar2 -> name );
    }

    return XFS_StringCompare4BST_ZHR ( Str1, Str2 );
}   /* _KartAddCallback () */

/*  The 'Kart' parameter could be NULL, in that case nothing will
 *  return. If it is not NULL, the reference counter for Kart will
 *  be incremented and Kart instance will be returned
 */
static
rc_t CC
_GapLoadKartFile (
                const struct XFSGapKart ** Kart,
                const struct _DieBanana * Banana,
                const char * KartDir,
                const char * KartName
)
{
    rc_t RCt;
    struct _DieBananable * Bananable;
    const struct XFSGapKart * TheKart;

    RCt = 0;
    Bananable = NULL;
    TheKart = NULL;

    if ( Kart != NULL ) {
        XFS_CSAN ( Kart )
    }
    XFS_CAN ( KartDir )
    XFS_CAN ( KartName )

    if ( ! XFSGapHasKart ( KartName ) ) {
        RCt = _GapKartMake ( & TheKart, KartDir, KartName );
        if ( RCt == 0 ) {
            RCt = _DieBananaLock ( Banana );
            if ( RCt == 0 ) {
                RCt = XFSGapKartAddRef ( TheKart );
                if ( RCt == 0 ) {
                    RCt = _DieBananableMake (
                                            & Bananable,
                                            ( void * ) TheKart
                                            );
                    if ( RCt == 0 ) {
                        RCt = _DieBananaAdd_NoLock (
                                        Banana,
                                        ( struct BSTNode * ) Bananable,
                                        _KartAddCallback
                                        );
pLogMsg ( klogDebug, " <<<[_GapLoadKartFile] RC[$(rc)] NAME[$(name)]", "rc=%u,name=%s", RCt, KartName );
                        if ( RCt == 0 ) {
                            if ( Kart != NULL ) {
                                    /*  We do add reference here
                                     */
                                RCt = XFSGapKartAddRef ( TheKart );
                                if ( RCt == 0 ) {
                                    * Kart = TheKart;
                                }
                            }
                        }
                    }
                }

                _DieBananaUnlock ( Banana );
            }

            /* JOJOBA */
            XFSGapKartRelease ( TheKart );
        }
    }
    else {
        /* JOJOBA ... that is not error, so here should be
         *            some warning printed
        RCt = XFS_RC ( rcExists );
         */
    }

    return RCt;
}   /* _GapLoadKartFile () */

static
int64_t CC
_KartFindCallback ( const void * Item, const struct BSTNode * Node )
{
    const char * Str1, * Str2;
    const struct _DieBananable * Bananable;
    const struct XFSGapKart * Kart;

    Str1 = ( const char * ) Item;
    Bananable = ( const struct _DieBananable * ) Node;

    _DieBananableGet ( Bananable, ( void ** ) & Kart );
    Str2 = Kart == NULL ? NULL : ( Kart -> name );

    return XFS_StringCompare4BST_ZHR ( Str1, Str2 );
}   /* _KartFindCallback () */

static
rc_t CC
_GapGetKart (
            struct _GapDepot * Depot,
            const struct XFSGapKart ** Kart,
            const char * KartName
)
{
    rc_t RCt;
    const struct _DieBananable * Bananable;
    const struct XFSGapKart * TheKart;

    RCt = 0;
    Bananable = NULL;
    TheKart = NULL;

    XFS_CSAN ( Kart )
    XFS_CAN ( Depot )
    XFS_CAN ( Depot -> karts )
    XFS_CAN ( Kart )
    XFS_CAN ( KartName )

    RCt = _DieBananaLock ( Depot -> karts );
    if ( RCt == 0 ) {
        RCt = _DieBananaFind_NoLock (
                                    Depot -> karts,
                                    ( const void ** ) & Bananable,
                                    ( const void * ) KartName,
                                    _KartFindCallback
                                    );
        if ( RCt == 0 ) {
            RCt = _DieBananableGet (
                                    Bananable,
                                    ( void ** ) & TheKart
                                    );
            if ( RCt == 0 ) {
                RCt = XFSGapKartAddRef ( TheKart );
                if ( RCt == 0 ) {
                    * Kart = TheKart;
                }
            }
        }

        _DieBananaUnlock ( Depot -> karts );
    }

    return RCt;
}   /* _GapGetKart () */

LIB_EXPORT
rc_t CC
XFSGapGetKart ( const struct XFSGapKart ** Kart, const char * KartName )
{
    return _GapGetKart ( _DepotGet (), Kart, KartName );
}   /* XFSGapGetKart () */

LIB_EXPORT
bool CC
XFSGapHasKart ( const char * KartName )
{
    rc_t RCt;
    const struct _DieBananable * Bananable;
    struct _GapDepot * Depot;

    RCt = 0;
    Bananable = NULL;
    Depot = _DepotGet ();

    XFS_CAN ( Depot )
    XFS_CAN ( KartName )

    RCt = _DieBananaLock ( Depot -> karts );
    if ( RCt == 0 ) {
        RCt = _DieBananaFind_NoLock (
                                    Depot -> karts,
                                    ( const void ** ) & Bananable,
                                    ( const void * ) KartName,
                                    _KartFindCallback
                                    );

        _DieBananaUnlock ( Depot -> karts );
    }

    return RCt == 0;
}   /* XFSGapHasKart () */

static
void
_KartListCallback ( BSTNode * Node, void * Data )
{
    struct LI_Stru * Stru;
    const struct _DieBananable * Bananable;
    struct XFSGapKart * Kart;

    Stru = ( struct LI_Stru * ) Data;
    Bananable = ( const struct _DieBananable * ) Node;

    if ( _DieBananableGet ( Bananable, ( void ** ) & Kart ) == 0 ) {
        if ( Stru -> list != NULL && Kart != NULL ) {
            if ( Stru -> flag ) {
                if ( Stru -> project_id == 0
                    || XFSGapKartHasDataForProject (
                                                Kart,
                                                Stru -> project_id
                                                )
                ) {
                    VNamelistAppend ( Stru -> list, Kart -> name );
                }
            }
            else {
                VNamelistAppend ( Stru -> list, Kart -> name );
            }
        }
    }
}   /* _KartListCallback () */

static
rc_t CC
_GapListKarts (
                        struct KNamelist ** List,
                        uint32_t ProjectId,
                        bool FilterProjects
)
{
    rc_t RCt;
    struct VNamelist * xList;
    struct _GapDepot * Depot;
    struct LI_Stru Stru;

    RCt = 0;
    xList = NULL;
    Depot = _DepotGet ();
    Stru . list = NULL;
    Stru . project_id = ProjectId;
    Stru . flag = FilterProjects;

    XFS_CSAN ( List )
    XFS_CAN ( Depot )
    XFS_CAN ( List )

    RCt = VNamelistMake ( & xList, 16 /* ????? */ );
    if ( RCt == 0 ) {
        Stru . list = xList;

        RCt = _DieBananaLock ( Depot -> karts );
        if ( RCt == 0 ) {
            _DieBananaForEach_NoLock (
                                    Depot -> karts,
                                    ( void * ) & Stru,
                                    _KartListCallback
                                    );

            RCt = VNamelistToNamelist ( xList, List );


            _DieBananaUnlock ( Depot -> karts );
        }
        VNamelistRelease ( xList );
    }

    return RCt;
}   /* _GapListKarts () */

LIB_EXPORT
rc_t CC
XFSGapListKarts ( struct KNamelist ** List )
{
    return _GapListKarts ( List, 0, false );
}   /* XFSGapListKarts () */

LIB_EXPORT
rc_t CC
XFSGapListKartsForProject (
                            struct KNamelist ** List,
                            uint32_t ProjectId
)
{
    return _GapListKarts ( List, ProjectId, true );
}   /* XFSGapListKartsForProject () */

LIB_EXPORT
rc_t CC
XFSGapRehash ()
{
    rc_t RCt;

    RCt = 0;

    return RCt;
}   /* XFSGapGetObject () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static rc_t CC _ResolveKartItems ( const struct XFSGapKart * Kart );

/*  Kart could be NULL
 */
static
rc_t CC
_GapLoadKartBanana (
                const struct XFSGapKart ** Kart,
                const struct _DieBanana * Banana,
                const char * KartDir,
                const char * KartName
)
{
    rc_t RCt;
    const struct XFSGapKart * TheKart;

    RCt = 0;
    TheKart = NULL;

    if ( Kart != NULL ) {
        XFS_CSAN ( Kart )
    }
    XFS_CAN ( Banana )
    XFS_CAN ( KartDir )
    XFS_CAN ( KartName )

    RCt = _GapLoadKartFile ( & TheKart, Banana, KartDir, KartName );
    if ( RCt == 0 ) {
        RCt = _ResolveKartItems ( TheKart );
        if ( RCt != 0 ) {
            /* This is not error, just bad luck */
            RCt = 0;
        }

        XFSGapKartRelease ( TheKart );
    }

    return RCt;
}   /* _GapLoadKartBanana () */

LIB_EXPORT
rc_t CC
XFSGapLoadKart (
                const struct XFSGapKart ** Kart, /* Not sure that
                                                  * parameter needed
                                                  */
                const char * KartDir,
                const char * KartName
)
{
    struct _GapDepot * Depot = _DepotGet ();

    XFS_CAN ( Depot )
    XFS_CAN ( Depot -> karts )

    return _GapLoadKartBanana (
                                Kart,
                                Depot -> karts,
                                KartDir,
                                KartName
                                );
}   /* XFSGapLoadKart () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* Resolver Stuff                                                    */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static
rc_t CC
_AddKartItemsForProject (
                    const struct XFSGapKart * Kart,
                    uint32_t ProjectId

)
{
    rc_t RCt;
    struct KNamelist * List;
    uint32_t ListCount;
    const char * ListId;

    RCt = 0;
    List = NULL;
    ListCount = 0;
    ListId = NULL;

    XFS_CAN ( Kart )

    RCt = _GapKartListIds ( Kart, & List, ProjectId );
    if ( RCt == 0 ) {
        RCt = KNamelistCount ( List, & ListCount );
        if ( RCt == 0 ) {
            if ( ListCount != 0 ) {
                for ( uint32_t llp = 0; llp < ListCount; llp ++ ) {
                    RCt = KNamelistGet ( List, llp, & ListId );
                    if ( RCt == 0 ) {
                        RCt = XFSGapResolverAddToResolve (
                                                        ProjectId,
                                                        ListId
                                                        );
                    }
                    if ( RCt != 0 ) {
                        RCt = 0;
/* JOJOBA: brag about error and continue
                        break;
*/
                    }
                }
            }

            KNamelistRelease ( List );
        }
    }

    return RCt;
}   /* _AddKartItemsForProject () */

rc_t CC
_ResolveKartItems ( const struct XFSGapKart * Kart )
{
    rc_t RCt;
    uint32_t ProjectCount, ProjectId;

    RCt = 0;
    ProjectCount = ProjectId = 0;

    XFS_CAN ( Kart )

    RCt = XFS_LIdxQty_ZHR ( Kart -> projects_idx, & ProjectCount );
    for ( uint32_t llp = 0; llp < ProjectCount; llp ++ ) {
        RCt = XFS_LIdxGet_ZHR ( Kart -> projects_idx, llp, & ProjectId );
        if ( RCt == 0 ) {
            RCt = _AddKartItemsForProject ( Kart, ProjectId );
        }
        if ( RCt != 0 ) {
            break;
        }
    }

    return RCt;
}   /* _ResolveKartItems () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * Versioning
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
static
rc_t CC
_GapDepotCheckCreateKartDirectory ( const char * Path )
{
    rc_t RCt;
    struct KDirectory * Dir;
    uint32_t PathType;

    RCt = 0;
    Dir = NULL;
    PathType = kptNotFound;

    XFS_CAN ( Path )

    RCt = KDirectoryNativeDir ( & Dir );
    if ( RCt == 0 ) {
        PathType = KDirectoryPathType ( Dir, Path );
        switch ( PathType ) {
            case kptNotFound :
                RCt = KDirectoryCreateDir ( Dir, 0755, kcmCreate, Path );
                break;
            case kptDir :
                break;
            default :
                RCt = XFS_RC ( rcInvalid );
                break;
        }

        KDirectoryRelease ( Dir );
    }

    return RCt;
}   /* _GapDepotCheckCreateKartDirectory () */

static
rc_t CC
_GapDepotVersion ( uint64_t * Version, const char * Path )
{
    rc_t RCt;
    struct KDirectory * Dir;
    KTime_t Date;

    RCt = 0;
    Dir = NULL;
    Date = 0;

    XFS_CSA ( Version, 0 )
    XFS_CAN ( Path )
    XFS_CAN ( Version )

    RCt = _GapDepotCheckCreateKartDirectory ( Path );
    if ( RCt == 0 ) {
        RCt = KDirectoryNativeDir ( & Dir );
        if ( RCt == 0 ) {
            RCt = KDirectoryDate ( Dir, & Date, Path );
            if ( RCt == 0 ) {
                * Version = Date;
            }
            KDirectoryRelease ( Dir );
        }
    }

    return RCt;
}   /* _GapDepotVersion () */

static
rc_t CC
_GapDepotList ( struct KNamelist ** List, const char * Path )
{
    rc_t RCt;
    struct KDirectory * Dir;

    RCt = 0;
    Dir = NULL;

    XFS_CSAN ( List )
    XFS_CAN ( Path )
    XFS_CAN ( List )

    RCt = _GapDepotCheckCreateKartDirectory ( Path );
    if ( RCt == 0 ) {
        RCt = KDirectoryNativeDir ( & Dir );
        if ( RCt == 0 ) {
            RCt = KDirectoryList ( Dir, List, NULL, NULL, Path );
            KDirectoryRelease ( Dir );
        }
    }

    return RCt;
}   /* _GapDepotList () */

rc_t CC
_GapKartDepotRefresh ( struct _GapDepot * Depot )
{
    rc_t RCt;
    uint64_t Version;
    struct KNamelist * List;
    uint32_t Count;
    const char * Name;
    const struct XFSGapKart * Kart;
    const struct _DieBanana * Banana, * TempBanana;
    struct _DieBananable * Bananable;

    RCt = 0;
    Version = 0;
    List = NULL;
    Count = 0;
    Name = NULL;
    Kart = NULL;
    Banana = NULL;
    TempBanana = NULL;
    Bananable = NULL;

    XFS_CAN ( Depot )
    XFS_CAN ( Depot -> kart_files )

    RCt = _GapDepotVersion ( & Version, Depot -> kart_files );
    if ( RCt == 0 ) {
        if ( Version != Depot -> version ) {
            RCt = _DieBananaMake ( & Banana, _DepotKartWhackCallback );
            if ( RCt == 0 ) {
                RCt = _GapDepotList ( & List, Depot -> kart_files );
                if ( RCt == 0 ) {
                    RCt = KNamelistCount ( List, & Count );
                    if ( RCt == 0 ) {
                        for ( uint32_t llp = 0; llp < Count; llp ++ ) {
                            RCt = KNamelistGet ( List, llp, & Name );
                            if ( RCt != 0 ) {
                                break;
                            }

                            RCt = _GapGetKart ( Depot, & Kart, Name );
                            if ( RCt == 0 ) {
                                RCt = _DieBananableMake (
                                                        & Bananable,
                                                        ( void * ) Kart
                                                        );
                                if ( RCt == 0 ) {
                                    RCt = _DieBananaAdd_NoLock (
                                    Banana,
                                    ( struct BSTNode * ) Bananable,
                                    _KartAddCallback
                                    );
                                }
                            }
                            else {
                                RCt = _GapLoadKartBanana (
                                                NULL,
                                                Banana,
                                                Depot -> kart_files,
                                                Name
                                                );
                            }
                            if ( RCt != 0 ) {
                                RCt = 0;

                                    /*  Kart may be damaged,
                                     *  so we are not exiting
                                break;
                                     *
                                     */
                            }

                        }

                        if ( RCt == 0 ) {
                            RCt = KLockAcquire ( Depot -> mutabor );
                            if ( RCt == 0 ) {
                                TempBanana = Depot -> karts;

                                Depot -> karts = Banana;

                                _DieBananaDispose ( TempBanana );
                                free ( ( struct _DieBanana * ) TempBanana );

                                KLockUnlock ( Depot -> mutabor );
                            }

                            Depot -> version = Version;
                        }
                        else {
                        }
                    }
                    KNamelistRelease ( List );
                }
            }
        }
    }

    return RCt;
}   /* _GapKartDepotRefresh () */

LIB_EXPORT
rc_t CC
XFSGapRefreshKarts ()
{
    struct _GapDepot * Depot = _DepotGet ();

    XFS_CAN ( Depot )

    return _GapKartDepotRefresh ( Depot );
}   /* XFSGapRefreshKarts () */

LIB_EXPORT
rc_t CC
XFSGapKartDepotVersion ( uint64_t * Version )
{
    struct _GapDepot * Depot = _DepotGet ();

    XFS_CSA ( Version, 0 )
    XFS_CAN ( Depot )
    XFS_CAN ( Version )

    * Version = Depot -> version;

    return 0;
}   /* XFSGapKartDepotVersion () */
