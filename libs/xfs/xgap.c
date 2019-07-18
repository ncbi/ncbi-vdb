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

#include "schwarzschraube.h"
#include "zehr.h"
#include "mehr.h"
#include "xgap.h"
#include <xfs/path.h>

#include <sysalloc.h>

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))    XFSGapProject and XFSGapDepot
 //     
((*/

struct _GapDepot {
    BSTree tree;

    struct KLock * mutabor;     /*  this lock is used for operations
                                 *  related to retrieve GapProjects
                                 */
};

struct XFSGapProject {
    BSTNode node;

    KRefcount refcount;

    bool good;
    uint32_t project_id;

    const struct KRepository * repository;
    const struct VResolver * resolver;

    const char * pass;
    size_t pass_size;
    const struct KKey * key;
};

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
_MakeResolver (
            const struct VResolver ** Resolver,
            const struct KRepository * Repository
)
{
    rc_t RCt;
    struct VResolver * TheResolver;

    RCt = 0;
    TheResolver = NULL;

    XFS_CSAN ( Resolver )
    XFS_CAN ( Resolver )
    XFS_CAN ( Repository )

    RCt = KRepositoryMakeResolver (
                                Repository,
                                & TheResolver,
                                XFS_Config_MHR ()
                                );
    if ( RCt == 0 ) {
        * Resolver = TheResolver;
    }

    return RCt;
}   /* _MakeResolver () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSGapProject                                                     */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static const char * _sGapProject_classname = "GapProject";

static
rc_t CC
_GapProjectDispose ( const struct XFSGapProject * self )
{
    struct XFSGapProject * Project = ( struct XFSGapProject * ) self;

    if ( Project != NULL ) {

        Project -> good = false;

        Project -> project_id = 0;

        KRefcountWhack (
                    & ( Project -> refcount ),
                    _sGapProject_classname
                    );

        if ( Project -> repository != NULL ) {
            KRepositoryRelease ( Project -> repository );
            Project -> repository = NULL;
        }

        if ( Project -> resolver != NULL ) {
            VResolverRelease ( Project -> resolver );
            Project -> resolver = NULL;
        }

        if ( Project -> key != NULL ) {
            free ( ( struct KKey * )  Project -> key );
            Project -> key = NULL;
        }

        if ( Project -> pass != NULL ) {
            free ( ( char * ) Project -> pass );
            Project -> pass = NULL;
        }

        Project -> pass_size = 0;

        free ( Project );
    }

    return 0;
}   /* _GapProjectDispose () */

static 
rc_t CC
_GapProjectMake (
                const struct XFSGapProject ** Project,
                uint32_t ProjectId
)
{
    rc_t RCt;
    struct XFSGapProject * TheProject;

    RCt = 0;
    TheProject = NULL;

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

        RCt = _GetRepository (
                            & ( TheProject -> repository ),
                            TheProject -> project_id
                            );
        if ( RCt == 0 ) {

            RCt = _MakeResolver (
                            & ( TheProject -> resolver ),
                            TheProject -> repository
                            );
            if ( RCt == 0 ) {
                TheProject -> good = true;

                * Project = TheProject;
            }
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

static
rc_t CC
_GapProjectReadCharValue (
                        const struct XFSGapProject * self,
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
    XFS_CAN ( self )
    XFS_CAN ( RetValue )
    XFS_CAN ( Reader )

    if ( ! XFSGapProjectIsValid ( self ) ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = Reader (
                self -> repository,
                Bf,
                sizeof ( Bf ),
                & TheRetValSize
                );
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
}   /* _GapProjectReadCharValue () */

LIB_EXPORT
rc_t CC
XFSGapProjectRepositoryName (
                        const struct XFSGapProject * self,
                        char ** RepositoryName
)
{
    return _GapProjectReadCharValue (
                                    self,
                                    RepositoryName,
                                    KRepositoryName
                                    );
}   /* XFSGapProjectRepositoryName () */

LIB_EXPORT
rc_t CC
XFSGapProjectRepositoryDisplayName (
                        const struct XFSGapProject * self,
                        char ** DisplayName
)
{
    return _GapProjectReadCharValue (
                                    self,
                                    DisplayName,
                                    KRepositoryDisplayName
                                    );
}   /* XFSGapProjectRepositoryDisplayName () */


LIB_EXPORT
rc_t CC
XFSGapProjectRepositoryRoot (
                        const struct XFSGapProject * self,
                        char ** RepositoryRoot
)
{
    return _GapProjectReadCharValue (
                                    self,
                                    RepositoryRoot,
                                    KRepositoryRoot
                                    );
}   /* XFSGapProjectRepositoryRoot () */

LIB_EXPORT
rc_t CC
XFSGapProjectWorkspace (
                        const struct XFSGapProject * self,
                        char ** Workspace
)
{
    rc_t RCt;
    char * Root;
    char BF [ XFS_SIZE_1024 ];
    size_t NR;

    RCt = 0;
    Root = NULL;
    * BF = 0;
    NR = 0;

    XFS_CSAN ( Workspace )
    XFS_CAN ( self )
    XFS_CAN ( Workspace )

    RCt = XFSGapProjectRepositoryRoot ( self, & Root );
    if ( RCt == 0 ) {
        RCt = string_printf (
                            BF,
                            sizeof ( BF ) - 1,
                            & NR,
                            "%s/workspace",
                            Root
                            );
        if ( RCt == 0 ) {
            RCt = XFS_StrDup ( BF, ( const char ** ) Workspace );
        }
        free ( Root );
    }

    return RCt;
}   /* XFSGapProjectWorkspace () */

rc_t CC
XFSGapPublicfiles ( char ** Publicfiles )
{
    rc_t RCt;
    const struct XFSGapProject * Project;

    RCt = 0;
    Project = NULL;

    XFS_CSAN ( Publicfiles )
    XFS_CAN ( Publicfiles )

    RCt = XFSGapFindOrCreate ( XFS_PUBLIC_PROJECT_ID, & Project );
    if ( RCt == 0 ) {
        RCt = XFSGapProjectRepositoryRoot ( Project, Publicfiles );

        XFSGapProjectRelease ( Project );
    }

    return RCt;
}   /* XFSGapPublicfiles () */

LIB_EXPORT
rc_t CC
XFSGapKartfiles ( char ** Kartfiles )
{
    rc_t RCt;
    char * UserConfigDir;
    const struct XFSPath * Path;

    RCt = 0;
    UserConfigDir = NULL;
    Path = NULL;

    XFS_CSAN ( Kartfiles )
    XFS_CAN ( Kartfiles )

    RCt = XFSGapUserConfigDir ( & UserConfigDir );
    if ( RCt == 0 ) {
        RCt= XFSPathMake (
                        & Path,
                        false,
                        "%s/kart-files",
                        UserConfigDir
                        );
        if ( RCt == 0 ) {
            RCt = XFS_StrDup (
                            XFSPathGet ( Path ),
                            ( const char ** ) Kartfiles
                            );

            XFSPathRelease ( Path );
        }

        free ( UserConfigDir );
    }

    return RCt;
}   /* XFSGapKartfiles () */

static
const char * CC
_GapUserHomeDir ()
{
    const char * Ret = getenv ( "HOME" );
    if ( Ret == NULL ) {
        Ret = getenv ( "USERPROFILE" );
    }

    return Ret;
}   /* _GapUserHomeDir () */

LIB_EXPORT
rc_t CC
XFSGapUserHome ( char ** UserHome )
{
    rc_t RCt;
    const char * Var;

    RCt = 0;
    Var = NULL;

    XFS_CSAN ( UserHome )
    XFS_CAN ( UserHome )

    Var = _GapUserHomeDir ();
    if ( Var != NULL ) {
        RCt = XFS_StrDup ( Var, ( const char ** ) UserHome );
    }
    else {
        RCt = XFS_RC ( rcInvalid );
    }

    return RCt;
}   /* XFSGapUserHome () */

LIB_EXPORT
rc_t CC
XFSGapUserConfigDir ( char ** UserConfigDir )
{
    rc_t RCt;
    const char * Var;
    const struct XFSPath * Path;

    RCt = 0;
    Var = NULL;
    Path = NULL;

    XFS_CSAN ( UserConfigDir )
    XFS_CAN ( UserConfigDir )

    Var = _GapUserHomeDir ();
    if ( Var == NULL ) {
        RCt = XFS_RC ( rcInvalid );
    }
    else {
        RCt= XFSPathMake ( & Path, false, "%s/.ncbi", Var );
        if ( RCt == 0 ) {
            RCt = XFS_StrDup (
                            XFSPathGet ( Path ) ,
                            ( const char ** ) UserConfigDir
                            );
            XFSPathRelease ( Path );
        }
    }

    return RCt;
}   /* XFSGapUserConfigDir () */

static
rc_t CC
_GapProjectLoadEncriptionKey ( const struct XFSGapProject * self )
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

    if ( ! XFSGapProjectIsValid ( Project ) ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = KRepositoryEncryptionKey (
                                Project -> repository,
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
}   /* _GapProjectLoadEncriptionKey () */

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

    if ( self -> key == NULL ) {
        RCt = _GapProjectLoadEncriptionKey ( self );
    }

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

    if ( self -> key == NULL ) {
        RCt = _GapProjectLoadEncriptionKey ( self );
    }

    if ( RCt == 0 ) {
        * Password = self -> pass;
        * PasswordSize = self -> pass_size;
    }

    return RCt;
}   /* XFSGapProjectPassword () */

LIB_EXPORT
rc_t CC
XFSGapProjectLocateAccession (
                        const struct XFSGapProject * self,
                        const char * Accession,
                        const struct VPath ** Remote,
                        const struct VPath ** Cache
)
{
    rc_t RCt;
    struct VPath * Query;

    RCt = 0;
    Query = NULL;

    XFS_CSAN ( Remote )
    XFS_CSAN ( Cache )
    XFS_CAN ( self )
    XFS_CAN ( Accession )

    if ( ! XFSGapProjectIsValid ( self ) ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Remote == NULL && Cache == NULL ) {
        return 0;
    }

    RCt = VFSManagerMakePath (
                            XFS_VfsManager (),
                            & Query,
                            "ncbi-acc:%s",
                            Accession
                            );
    if ( RCt == 0 ) {
        RCt = VResolverQuery (
                        self -> resolver,
                        0,
                        Query,
                        NULL,
                        Remote,
                        Cache
                        );

        VPathRelease ( Query );
    }

    return RCt;
}   /* XFSGapProjectLocateAccession () */

LIB_EXPORT
rc_t CC
XFSGapProjectLocateObject (
                        const struct XFSGapProject * self,
                        uint32_t ObjectId,
                        const struct VPath ** Remote,
                        const struct VPath ** Cache
)
{
    rc_t RCt;
    struct VPath * Query;

    RCt = 0;
    Query = NULL;

    XFS_CSAN ( Remote )
    XFS_CSAN ( Cache )
    XFS_CAN ( self )

    if ( ! XFSGapProjectIsValid ( self ) ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Remote == NULL && Cache == NULL ) {
        return 0;
    }

    RCt = VFSManagerMakePath (
                            XFS_VfsManager (),
                            & Query,
                            "ncbi-obj:%d",
                            ObjectId
                            );
    if ( RCt == 0 ) {
        RCt = VResolverQuery (
                        self -> resolver,
                        0,
                        Query,
                        NULL,
                        Remote,
                        Cache
                        );

        VPathRelease ( Query );
    }

    return RCt;
}   /* XFSGapProjectLocateObject () */

LIB_EXPORT
rc_t CC
XFSGapProjectAccessionUrlAndPath (
                            const struct XFSGapProject * self,
                            const char * Accession,
                            const char ** RemoteUrl,
                            const char ** CachePath
)
{
    rc_t RCt;
    char BF [ XFS_SIZE_4096 ];
    const struct VPath * rPath;
    const struct VPath * cPath;
    const char * rChar;
    const char * cChar;

    RCt = 0;
    * BF = 0;
    rPath = NULL;
    cPath = NULL;
    rChar = NULL;
    cChar = NULL;

    XFS_CSAN ( RemoteUrl )
    XFS_CSAN ( CachePath )
    XFS_CAN ( self )
    XFS_CAN ( Accession )

    if ( ! XFSGapProjectIsValid ( self ) ) {
        return XFS_RC ( rcInvalid );
    }

    if ( RemoteUrl == NULL && CachePath == NULL ) {
        return 0;
    }

    RCt = XFSGapProjectLocateAccession (
                                    self,
                                    Accession,
                                    RemoteUrl == NULL ? NULL : & rPath,
                                    CachePath == NULL ? NULL : & cPath
                                    );
    if ( RCt == 0 ) {
        if ( rPath != NULL ) {

            RCt = XFS_ReadVUri_ZHR (
                                    rPath,
                                    BF,
                                    sizeof ( BF ),
                                    NULL
                                    );
            if ( RCt == 0 ) {
                RCt = XFS_StrDup ( BF, & rChar );
            }

            VPathRelease ( rPath );
        }

        if ( cPath != NULL ) {
            if ( RCt == 0 ) {

                RCt = XFS_ReadVPath_ZHR (
                                        cPath,
                                        BF,
                                        sizeof ( BF ),
                                        NULL
                                        );

                if ( RCt == 0 ) {
                    RCt = XFS_StrDup ( BF, & cChar );
                }
            }

            VPathRelease ( cPath );
        }

        if ( RCt == 0 ) {
            if ( RemoteUrl != NULL ) {
                * RemoteUrl = rChar;
            }

            if ( CachePath != NULL ) {
                * CachePath = cChar;
            }
        }
    }

    if ( RCt != 0 ) {
        if ( RemoteUrl != NULL ) {
            * RemoteUrl = NULL;
        }

        if ( CachePath != NULL ) {
            * CachePath = NULL;
        }

        if ( rChar != NULL ) {
            free ( ( char * ) rChar );
        }

        if ( cChar != NULL ) {
            free ( ( char * ) cChar );
        }
    }

    return RCt;
}   /* XFSGapProjectAccessionUrlAndPath () */

LIB_EXPORT
rc_t CC
XFSGapProjectObjectUrlAndPath (
                            const struct XFSGapProject * self,
                            uint32_t ObjectId,
                            const char ** RemoteUrl,
                            const char ** CachePath
)
{
    rc_t RCt;
    char BF [ XFS_SIZE_4096 ];
    const struct VPath * rPath;
    const struct VPath * cPath;
    const char * rChar;
    const char * cChar;

    RCt = 0;
    * BF = 0;
    rPath = NULL;
    cPath = NULL;
    rChar = NULL;
    cChar = NULL;

    XFS_CSAN ( RemoteUrl )
    XFS_CSAN ( CachePath )
    XFS_CAN ( self )

    if ( ObjectId == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    if ( ! XFSGapProjectIsValid ( self ) ) {
        return XFS_RC ( rcInvalid );
    }

    if ( RemoteUrl == NULL && CachePath == NULL ) {
        return 0;
    }

    RCt = XFSGapProjectLocateObject (
                                    self,
                                    ObjectId,
                                    RemoteUrl == NULL ? NULL : & rPath,
                                    CachePath == NULL ? NULL : & cPath
                                    );
    if ( RCt == 0 ) {
        if ( rPath != NULL ) {

            RCt = XFS_ReadVUri_ZHR (
                                    rPath,
                                    BF,
                                    sizeof ( BF ),
                                    NULL
                                    );
            if ( RCt == 0 ) {
                RCt = XFS_StrDup ( BF, & rChar );
            }

            VPathRelease ( rPath );
        }

        if ( cPath != NULL ) {
            if ( RCt == 0 ) {

                RCt = XFS_ReadVPath_ZHR (
                                        cPath,
                                        BF,
                                        sizeof ( BF ),
                                        NULL
                                        );

                if ( RCt == 0 ) {
                    RCt = XFS_StrDup ( BF, & cChar );
                }
            }

            VPathRelease ( cPath );
        }

        if ( RCt == 0 ) {
            if ( RemoteUrl != NULL ) {
                * RemoteUrl = rChar;
            }
            if ( CachePath != NULL ) {
                * CachePath = cChar;
            }
        }
    }

    if ( RCt != 0 ) {
        if ( RemoteUrl != NULL ) {
            * RemoteUrl = NULL;
        }
        if ( CachePath != NULL ) {
            * CachePath = NULL;
        }

        if ( rChar != NULL ) {
            free ( ( char * ) rChar );
        }

        if ( cChar != NULL ) {
            free ( ( char * ) cChar );
        }
    }

    return RCt;
}   /* XFSGapProjectObjectUrlAndPath () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* _GapDepot                                                     */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static struct _GapDepot * _sDepot = NULL;

static
struct _GapDepot * CC
_DepotGet ()
{
    return _sDepot;
}   /* _DepotGet () */

static
void CC
_DepotWhackCallback ( BSTNode * Node, void * unused )
{
    if ( Node != NULL ) {
        XFSGapProjectRelease ( ( const struct XFSGapProject * ) Node );
    }
}   /* _DepotWhackCallback () */

static
rc_t CC
_GapDepotDispose ( struct _GapDepot * self )
{
    if ( self != NULL ) {
        BSTreeWhack ( & ( self -> tree ), _DepotWhackCallback, NULL );

        if ( self -> mutabor != NULL ) {
            KLockRelease ( self -> mutabor );
            self -> mutabor = NULL;
        }

        free ( self );
    }

    return 0;
}   /* _GapDepotDispose () */

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
            BSTreeInit ( & ( TheDepot -> tree ) );

            * Depot = TheDepot;
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
    if ( _sDepot == NULL ) {
        return _GapDepotMake ( & _sDepot );
    }

    return 0;
}   /* XFSGapDelete () */

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
_DepotAddCallback (
                const struct BSTNode * N1,
                const struct BSTNode * N2
)
{
    struct XFSGapProject * P1 = ( struct XFSGapProject * ) N1;
    struct XFSGapProject * P2 = ( struct XFSGapProject * ) N2;
    return  P1 -> project_id - P2 -> project_id;
}   /* _DepotAddCallback () */

static
rc_t CC
_DepotAdd_NoLock ( const struct XFSGapProject * Project )
{
    struct _GapDepot * Depot = _DepotGet ();

    XFS_CAN ( Project )
    XFS_CAN ( Depot )

    return BSTreeInsert (
                        & ( Depot -> tree ),
                        ( struct BSTNode * ) Project,
                        _DepotAddCallback
                        );
}   /* _DepotAdd_NoLock () */

static
int64_t CC
_DepotFindCallback ( const void * Item, const struct BSTNode * Node )
{
    uint32_t ItemId, NodeId;

    ItemId = * ( ( uint32_t * ) Item );
    NodeId = Node == NULL
                    ? 0
                    : ( ( struct XFSGapProject * ) Node ) -> project_id
                    ;


    return ItemId - NodeId;
}   /* _DepotFindCallback () */

static
rc_t CC
_GapDepotFind_NoLock (
                    uint32_t ProjectId,
                    const struct XFSGapProject ** Project
)
{
    rc_t RCt;
    struct XFSGapProject * TheProject;
    struct _GapDepot * Depot;

    RCt = 0;
    Depot = _DepotGet ();

    XFS_CSAN ( Project )
    XFS_CAN ( Depot )
    XFS_CAN ( Project )
    XFS_CAN ( Depot )

    TheProject = ( struct XFSGapProject * ) BSTreeFind ( 
                                        & ( Depot -> tree ),
                                        ( const void * ) & ProjectId,
                                        _DepotFindCallback
                                        );
    if ( TheProject == NULL ) {
        RCt = XFS_RC ( rcNotFound );
    }
    else {
        * Project = TheProject;
    }

    return RCt;
}   /* _GapDepotFind () */

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

    RCt = KLockAcquire ( Depot -> mutabor );
    if ( RCt == 0 ) {

        RCt = _GapDepotFind_NoLock ( ProjectId, & Project );

        KLockUnlock ( Depot -> mutabor );
    }

    return RCt == 0;
}   /* XFSGapHas () */

static
rc_t CC
_GapProjectMake_NoLock (
                const struct XFSGapProject ** Project,
                uint32_t ProjectId
)
{
    rc_t RCt;

    RCt = 0;

    XFS_CSAN ( Project )
    XFS_CAN ( Project )

    RCt = _GapProjectMake ( Project, ProjectId );
    if ( RCt == 0 ) {
        RCt = _DepotAdd_NoLock ( * Project );
        if ( RCt != 0 ) {
            * Project = NULL;

            _GapProjectDispose ( * Project );
        }
    }

    return RCt;
}   /* _GapProjectMake_NoLock () */

LIB_EXPORT
rc_t CC
XFSGapFindOrCreate (
                uint32_t ProjectId,
                const struct XFSGapProject ** Project
)
{
    rc_t RCt;
    struct _GapDepot * Depot;
    const struct XFSGapProject * TheProject;

    RCt = 0;
    Depot = _DepotGet ();
    TheProject = NULL;

    XFS_CSAN ( Project )
    XFS_CAN ( Project )

    RCt = KLockAcquire ( Depot -> mutabor );
    if ( RCt == 0 ) {

        RCt = _GapDepotFind_NoLock ( ProjectId, & TheProject );
        if ( GetRCState ( RCt ) == rcNotFound ) {
            RCt = _GapProjectMake_NoLock ( & TheProject, ProjectId );
        }

        if ( RCt == 0 ) {
            RCt = XFSGapProjectAddRef ( TheProject );

            if ( RCt == 0 ) {
                * Project = TheProject;
            }
        }

        KLockUnlock ( Depot -> mutabor );
    }

    return RCt;
}   /* XFSGapFindOrCreate () */

LIB_EXPORT
rc_t CC
XFSGapRemove ( uint32_t ProjectId )
{
    rc_t RCt;
    struct _GapDepot * Depot;
    const struct XFSGapProject * Project;

    RCt = 0;
    Depot = _DepotGet ();
    Project = NULL;

    RCt = KLockAcquire ( Depot -> mutabor );
    if ( RCt == 0 ) {
        RCt = _GapDepotFind_NoLock ( ProjectId, & Project );
        if ( RCt == 0 ) {
            BSTreeUnlink (
                        & ( Depot -> tree ),
                        ( struct BSTNode * ) & ( Project -> node )
                        );
            XFSGapProjectRelease ( Project );
        } 

        KLockUnlock ( Depot -> mutabor );
    }

    return RCt;
}   /* XFSGapRemove () */

