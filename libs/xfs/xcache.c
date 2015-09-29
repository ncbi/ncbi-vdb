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
#include <klib/vector.h>

#include <kfs/directory.h>
#include <kfs/file.h>

#include <kfg/repository.h>

#include <xfs/node.h>
#include "schwarzschraube.h"
#include "xcache.h"
#include "bean.h"
#include "zehr.h"
#include "mehr.h"
#include "common.h"

#include <sysalloc.h>

#include <string.h>

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* Repository stuff                                                  */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/



static
rc_t CC
_RepositoryManager ( const struct KRepositoryMgr ** RepositoryMgr )
{
    rc_t RCt;
    const struct KRepositoryMgr * Mgr;
    const struct KConfig * Kfg;

    RCt = 0;
    Mgr = NULL;
    Kfg = XFS_Config_MHR ();

    XFS_CSAN ( RepositoryMgr )
    XFS_CAN ( RepositoryMgr )
    XFS_CAN ( Kfg )

    RCt = KConfigMakeRepositoryMgrRead ( Kfg, & Mgr );
    if ( RCt == 0 ) {
        * RepositoryMgr = Mgr;
    }

    return RCt;
}   /* _RepositoryManager () */

static
rc_t CC
_GetUserRepositories ( KRepositoryVector * Vector )
{
    rc_t RCt;
    const struct KRepositoryMgr * RepositoryMgr;

    RCt = 0;
    RepositoryMgr = NULL;

    RCt = _RepositoryManager ( & RepositoryMgr );
    if ( RCt == 0 ) {

        RCt = KRepositoryMgrUserRepositories ( RepositoryMgr, Vector );

        KRepositoryMgrRelease ( RepositoryMgr );
    }

    return RCt;
}   /* _GetUserRepositories () */


LIB_EXPORT
rc_t CC
_RepositoryRootName (
                    uint32_t ProjectId,
                    const char ** Root,
                    const char ** Name
)
{
    rc_t RCt;
    KRepositoryVector RepositoryVec;
    const struct KRepository * Repository;
    uint32_t llp, qty, prj;
    char Bf [ XFS_SIZE_512 ];
    char Bff [ XFS_SIZE_1024 ];

    RCt = 0;
    Repository = NULL;
    llp = qty = prj = 0;
    * Bf = 0;
    * Bff = 0;

    XFS_CSAN ( Root )
    XFS_CSAN ( Name )
    XFS_CAN ( Root )
    XFS_CAN ( Name )

    VectorInit ( & RepositoryVec, 0, 5 );

    RCt = _GetUserRepositories ( & RepositoryVec );
    if ( RCt == 0 ) {
        qty = VectorLength ( & RepositoryVec );
        for ( llp = 0; llp < qty; llp ++ ) {
            Repository = VectorGet ( & RepositoryVec, llp );
            if ( Repository != NULL ) {
                RCt = KRepositoryName (
                                    Repository,
                                    Bf,
                                    sizeof ( Bf ),
                                    NULL
                                    );
                if ( RCt != 0 ) {
                    RCt = 0;
                    continue;
                }

                if ( ProjectId == 0 ) {
                    if ( strcmp ( Bf, "public" ) != 0 ) {
                        continue;
                    }
                }
                else {
                    RCt = KRepositoryProjectId ( Repository, & prj );
                    if ( RCt != 0 ) {
                        RCt = 0;
                        continue;
                    }

                    if ( prj != ProjectId ) {
                        continue;
                    }
                }

                    /* So, if we are here, it is right repository :)
                     */
                RCt = KRepositoryRoot (
                                    Repository,
                                    Bff,
                                    sizeof ( Bff ),
                                    NULL
                                    );
                if ( RCt == 0 ) {
                    RCt = XFS_StrDup ( Bff, Root );
                    if ( RCt == 0 ) {
                        RCt = XFS_StrDup ( Bf, Name );
                    }
                }

                /* We are here only if ProjectId is correct
                 */
                break;
            }
        }
    }

    KRepositoryVectorWhack ( & RepositoryVec );

    if ( RCt != 0 ) {
        if ( * Root != NULL ) {
            free ( ( char * ) * Root );
            * Root = NULL;
        }

        if ( * Name != NULL ) {
            free ( ( char * ) * Name );
            * Name = NULL;
        }
    }

    return RCt;
}   /* _RepositoryRootName () */

static
bool CC
_EndsWith( const char * Str, const char * End )
{
    uint32_t StrLen, EndLen;

    if ( Str == NULL || End == NULL ) {
        return false;
    }

    StrLen = string_len ( Str, string_size ( Str ) );
    EndLen = string_len ( End, string_size ( End ) );

    if ( StrLen >= EndLen && EndLen > 0 ) {
        return string_cmp (
                        Str + ( StrLen - EndLen ),
                        EndLen,
                        End,
                        EndLen,
                        EndLen
                        ) == 0;
    }

    return false;
}

static
rc_t CC
_DoAddBean (
            const char * Name, 
            const char * Resource,
            const struct XFSBeanSack * Sack
)
{
    rc_t RCt;
    const struct XFSBean * Bean;

    RCt = 0;
    Bean = NULL;

    if ( _EndsWith ( Name, ".cache" ) || _EndsWith ( Name, ".lock" ) ) {
        return 0;
    }

    RCt = XFSBeanMake ( & Bean, Name, Resource, 1 );
    if ( RCt == 0 ) {

        RCt = XFSBeanSackAdd ( Sack, Bean );

        XFSBeanRelease ( Bean );
    }

    return RCt;
}   /* _DoAddBean () */

static
rc_t CC
_DoRepositoryScan (
                struct KDirectory * Dir,
                const char * Root,
                const struct XFSBeanSack * Sack
)
{
    rc_t RCt;
    struct KNamelist * List;
    uint32_t llp, qty, PathType;
    const char * Name;
    char FullPath [ XFS_SIZE_4096 ];

    RCt = 0;
    List = NULL;
    llp = qty = 0;
    PathType = kptBadPath;
    Name = NULL;
    * FullPath = 0;

    RCt = KDirectoryList ( Dir, & List, NULL, NULL, Root );
    if ( RCt == 0 ) {

        RCt = KNamelistCount ( List, & qty );
        if ( RCt == 0 ) {
            for ( llp = 0; llp < qty; llp ++ ) {
                RCt = KNamelistGet ( List, llp, & Name );
                if ( RCt == 0 ) {
                    RCt = KDirectoryResolvePath (
                                                Dir,
                                                true,
                                                FullPath,
                                                sizeof ( FullPath ),
                                                "%s/%s",
                                                Root, Name
                                                );
                    if ( RCt == 0 ) {
                        PathType = KDirectoryPathType ( Dir, FullPath );
                        if ( RCt == 0 ) {
                            switch ( PathType ) {
                                case kptDir :
                                        RCt = _DoRepositoryScan (
                                                            Dir,
                                                            FullPath,
                                                            Sack
                                                            );
                                        break;
                                case kptFile :
                                        RCt = _DoAddBean (
                                                        Name,
                                                        FullPath,
                                                        Sack
                                                        );
                                        break;
                                default :
                                        break;
                            }
                        }
                    }
                }

                if ( RCt != 0 ) {
                    break;
                }
            }
        }

        KNamelistRelease ( List );
    }

    return RCt;
}   /* _DoRepositoryScan () */

static
rc_t CC
_RepositoryScan ( const char * Root, const struct XFSBeanSack * Sack )
{
    rc_t RCt;
    struct KDirectory * NatDir;

    RCt = 0;
    NatDir = NULL;

    RCt = KDirectoryNativeDir ( & NatDir );
    if ( RCt == 0 ) {
        RCt = _DoRepositoryScan ( NatDir, Root, Sack );

        KDirectoryRelease ( NatDir );
    }

    return RCt;
}   /* _RepositoryScan () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* struct _GapBeanSuite                                              */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
struct _GapBeanSuite {
    struct XFSBeanSuite suite;

    uint32_t project_id;    /* ZERO - public */
    const char * root;
    const char * name;
};

/*) Suite virtual tabele
 (*/
static rc_t CC _GapBeanSuite_dispose_v1 (
                                const struct XFSBeanSuite * self
                                );
static rc_t CC _GapBeanSuite_render_v1 (
                                const struct XFSBeanSuite * self,
                                const char * Name,
                                const struct XFSNode ** Node
                                );
static rc_t CC _GapBeanSuite_refresh_v1 (
                                const struct XFSBeanSuite * self
                                );

static const struct XFSBeanSuite_vt_v1 _sBeanSackSuteVT_v1 = {
                                1, 1,   /* maj min */
                                _GapBeanSuite_dispose_v1,
                                _GapBeanSuite_render_v1,
                                _GapBeanSuite_refresh_v1,
                                };
static
rc_t CC
_GapBeanSuiteMake (
            const struct _GapBeanSuite ** Suite,
            uint32_t ProjectId
)
{
    rc_t RCt;
    struct _GapBeanSuite * TheSuite;

    RCt = 0;
    TheSuite = NULL;

    XFS_CSAN ( Suite )
    XFS_CAN ( Suite )

    TheSuite = calloc ( 1, sizeof ( struct _GapBeanSuite ) );
    if ( TheSuite == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFSBeanSuiteInit (
            & ( TheSuite -> suite ),
            ( const union XFSBeanSuite_vt * ) & _sBeanSackSuteVT_v1
            );
        if ( RCt == 0 ) {

            RCt = _RepositoryRootName (
                                    ProjectId,
                                    & TheSuite -> root,
                                    & TheSuite -> name
                                    );
            if ( RCt == 0 ) {
                TheSuite -> project_id = ProjectId; 

                    /* We do at in anyway ...
                     */
                XFSBeanSuiteRefresh ( & TheSuite -> suite );

                * Suite = TheSuite;
            }
        }
    }

    if ( RCt != 0 ) {
        * Suite = NULL;

        if ( TheSuite != NULL ) {
            XFSBeanSuiteDispose ( & ( TheSuite -> suite ) );
        }
    }

    return RCt;
}   /* _GapBeanSuiteMake () */

rc_t CC
_GapBeanSuite_dispose_v1 ( const struct XFSBeanSuite * self )
{
    struct _GapBeanSuite * Suite = ( struct _GapBeanSuite * ) self;
    if ( Suite != NULL ) {
        if ( Suite -> root != NULL ) {
            free ( ( char * ) Suite -> root );
            Suite -> root = NULL;
        }

        if ( Suite -> name != NULL ) {
            free ( ( char * ) Suite -> name );
            Suite -> name = NULL;
        }

        Suite -> project_id = 0;

        free ( Suite );
    }

    return 0;
}   /* _GapBeanSuite_dispose_v1 () */

rc_t CC
_GapBeanSuite_render_v1 (
                    const struct XFSBeanSuite * self,
                    const char * Name,
                    const struct XFSNode ** Node
)
{
    rc_t RCt;
    const struct XFSBeanSack * Sack;
    const struct XFSBean * Bean;
    struct XFSNode * TheNode;
    const char * BeanResource;

    RCt = 0;
    Sack = NULL;
    Bean = NULL;
    TheNode = NULL;
    BeanResource = NULL;

    RCt = XFSBeanSuiteGetSack ( self, & Sack );
    if ( RCt == 0 ) {
        RCt = XFSBeanSackFind ( Sack, & Bean, Name );
        if ( RCt == 0 ) {
            RCt = XFSBeanResource ( Bean, & BeanResource );
            if ( RCt == 0 ) {
                RCt = XFSFileNodeMake (
                                    & TheNode,
                                    BeanResource,
                                    Name,
                                    "r--r--r--"
                                    );
                if ( RCt == 0 ) {
                    * Node = TheNode;
                }
            }

            XFSBeanRelease ( Bean );
        }

        XFSBeanSackRelease ( Sack );
    }

    if ( RCt != 0 ) {
        * Node = NULL;

        if ( TheNode != NULL ) {
            XFSNodeDispose ( TheNode );
        }
    }

    return RCt;
}   /* _GapBeanSuite_render_v1 () */

rc_t CC
_GapBeanSuite_refresh_v1 ( const struct XFSBeanSuite * self )
{
    rc_t RCt;
    const struct XFSBeanSack * Sack;
    const struct _GapBeanSuite * Suite;

    RCt = 0;
    Sack = NULL;
    Suite = ( const struct _GapBeanSuite * ) self;

    XFS_CAN ( self )

    RCt = XFSBeanSackMake ( & Sack, 0 );
    if ( RCt == 0 ) {

        RCt = _RepositoryScan ( Suite -> root, Sack );
        if ( RCt == 0 ) {

            RCt = XFSBeanSuiteSetSack ( self, Sack );
        }

        XFSBeanSackRelease ( Sack );
    }

    return RCt;
}   /* _GapBeanSuite_refresh_v1 () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* struct XFSGapCache                                                */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
struct XFSGapCache {
    uint32_t project_id;
    bool read_only;

    const struct _GapBeanSuite * suite;
};

LIB_EXPORT
rc_t CC
XFSGapCacheMake (
                struct XFSGapCache ** Cache,
                uint32_t ProjectId,
                bool ReadOnly
)
{
    rc_t RCt;
    struct XFSGapCache * TheCache;
    const struct _GapBeanSuite * Suite;

    RCt = 0;
    TheCache = NULL;

    TheCache = calloc ( 1, sizeof ( struct XFSGapCache ) );
    if ( TheCache == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = _GapBeanSuiteMake ( & Suite, ProjectId );
        if ( RCt == 0 ) {

            TheCache -> project_id = ProjectId;
            TheCache -> read_only = ReadOnly;
            TheCache -> suite = Suite;

            * Cache = TheCache;
        }
    }

    if ( RCt != 0 ) {
        * Cache = NULL;

        if ( TheCache != NULL ) {
            XFSGapCacheDispose ( TheCache );
        }
    }

    return RCt;
}   /* XFSGapCacheMake () */

LIB_EXPORT
rc_t CC
XFSGapCacheDispose ( struct XFSGapCache * self )
{
    if ( self != NULL ) {
        if ( self -> suite != NULL ) {
            XFSBeanSuiteRelease ( & ( self -> suite -> suite ) );

            self -> suite = NULL;
        }

        self -> project_id = 0;
        self -> read_only = false;

        free ( self );
    }

    return 0;
}   /* XFSGapCacheDispose () */

LIB_EXPORT
rc_t CC
XFSGapCacheList (
                const struct XFSGapCache * self,
                const struct KNamelist ** Names
)
{
    rc_t RCt;
    const struct XFSBeanSack * Sack;

    RCt = 0;
    Sack = NULL;

    XFS_CSAN ( Names )
    XFS_CAN ( self )
    XFS_CAN ( Names )

    RCt = XFSBeanSuiteGetSack ( & ( self -> suite -> suite ), & Sack );
    if ( RCt == 0 ) {
        RCt = XFSBeanSackNames ( Sack, Names );

        XFSBeanSackRelease ( Sack );
    }

    return RCt;
}   /* XFSGapCacheList () */

LIB_EXPORT
rc_t CC
XFSGapCacheFind (
                const struct XFSGapCache * self,
                const struct XFSNode ** Node,
                const char * NodeName
)
{
    XFS_CSAN ( Node )
    XFS_CAN ( self )
    XFS_CAN ( Node )
    XFS_CAN ( NodeName )
    XFS_CAN ( self -> suite )

    return XFSBeanSuiteFind (
                        & ( self -> suite -> suite ),
                        NodeName,
                        Node
                        );
}   /* XFSGapCacheFind () */

LIB_EXPORT
rc_t CC
XFSGapCacheDeleteNode (
                const struct XFSGapCache * self,
                const char * NodeName
)
{
    XFS_CAN ( self )
    XFS_CAN ( NodeName )
    XFS_CAN ( self -> suite )

    if ( self -> read_only ) {
        return XFS_RC ( rcReadonly );
    }

    return XFSBeanSuiteUnlink (
                            & ( self -> suite -> suite ),
                            NodeName
                            );
}   /* XFSGapCacheDeleteNode () */

LIB_EXPORT
rc_t CC
XFSGapCacheProjectId (
                const struct XFSGapCache * self,
                uint32_t * ProjectId
)
{
    XFS_CSA ( ProjectId, 0 )
    XFS_CAN ( self )
    XFS_CAN ( ProjectId )

    * ProjectId = self -> project_id;

    return 0;
}   /* XFSGapCacheProjectId () */

LIB_EXPORT
rc_t CC
XFSGapCacheReadOnly (
                const struct XFSGapCache * self,
                bool * ReadOnly
)
{
    XFS_CSA ( ReadOnly, true )
    XFS_CAN ( self )
    XFS_CAN ( ReadOnly )

    * ReadOnly = self -> read_only;

    return 0;
}   /* XFSGapCacheReadOnly () */
