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

#include <sra/path-extern.h>

struct SRARunlist;
#define KNAMELIST_IMPL struct SRARunlist
#include <klib/impl.h>

struct NCBISRAPath;
#define SRAPATH_IMPL struct NCBISRAPath
#include <sra/impl.h>

#include "libsrapath.vers.h"

#include <vfs/manager.h>
#include <sra/srapath.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/mmap.h>
#include <kfs/dyload.h>
#include <klib/container.h>
#include <klib/vector.h>
#include <klib/text.h>
#include <klib/rc.h>
#include <klib/printf.h>
#include <kfg/config.h>
#include <kfg/kfg-priv.h>
#include <klib/debug.h>
#include <klib/log.h>
#include <sysalloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <os-native.h>
#include <assert.h>

#include "path-priv.h"

/*--------------------------------------------------------------------------
 * SRAPathString
 */
enum
{
    alg_none,
    alg_ncbi,
    alg_ddbj,
    alg_ebi,
    alg_refseq,
    alg_wgs
};

typedef struct SRAPathString SRAPathString;
struct SRAPathString
{
    DLNode n;
    uint8_t alg;
    char path [ 1 ];
};

/* Whack
 */
static
void CC SRAPathStringWhack ( DLNode *n, void *ignore )
{
    free ( n );
}

/* Make
 */
static
rc_t SRAPathStringMake ( DLList *list, const char *path, size_t sz, uint8_t alg )
{
    SRAPathString *s = (SRAPathString*) malloc ( sizeof * s + sz );
    if ( s == NULL )
        return RC ( rcSRA, rcMgr, rcUpdating, rcMemory, rcExhausted );

    s -> alg = alg;
    string_copy ( s -> path, sz + 1, path, sz );
    DLListPushTail ( list, & s -> n );

    return 0;
}


/* Find
 */
typedef struct SRAPathFindInfo SRAPathFindInfo;
struct SRAPathFindInfo
{
    const char *path;
    size_t size;
    uint8_t alg;
    bool found;
};

static
bool CC SRAPathStringFind ( DLNode *n, void *data )
{
    const SRAPathString *p = ( const SRAPathString* ) n;
    SRAPathFindInfo *pb = ( SRAPathFindInfo* ) data;
    if ( memcmp ( p -> path, pb -> path, pb -> size ) == 0 )
    {
        if ( p -> path [ pb -> size ] == 0 )
        {
            pb -> alg = p -> alg;
            return pb -> found = true;
        }
    }
    return false;
}

/*--------------------------------------------------------------------------
 * NCBIRepository
 *  represents a repository: replication server(s) / volume(s)
 */
typedef struct NCBIRepository NCBIRepository;
struct NCBIRepository
{
    DLNode n;

    uint8_t type; /* repository type: alg_ncbi (sra: ncbi/ebi/ddbj), alg_wgs, alg_refseq*/

    /* replication servers */
    DLList repsrv;

    /* volumes upon each repserver */
    DLList vols;
};
/* Init
 */
static
rc_t CC SRARepoMake(NCBIRepository** repo)
{
    *repo = (NCBIRepository*)malloc(sizeof(NCBIRepository));
    if (* repo == 0)
    {
        return RC ( rcSRA, rcMgr, rcInitializing, rcMemory, rcExhausted );
    }
    (*repo)->type = alg_ncbi;
    (*repo)->n.next = NULL;
    (*repo)->n.prev = NULL;
    DLListInit(&(*repo)->repsrv);
    DLListInit(&(*repo)->vols);
    return 0;
}
/* Whack
 */
static
void CC SRARepoWhack ( DLNode *n, void *ignore )
{
    if (n != NULL)
    {
        DLListWhack ( & ((NCBIRepository*) n) -> repsrv, SRAPathStringWhack, NULL );
        DLListWhack ( & ((NCBIRepository*) n) -> vols, SRAPathStringWhack, NULL );
        free ( (NCBIRepository*) n );
    }
}

static 
const char* AlgToStr(uint8_t alg)
{
    switch (alg)
    {
    case alg_ncbi: 
    case alg_ddbj:
    case alg_ebi: return "SRA";
    case alg_wgs: return "WGS";
    case alg_refseq: return "REFSEQ";
    default: return "<unknown>";
    }
}


static 
void CC LogVolume( DLNode *n, void *data )
{
    const SRAPathString* self = (const SRAPathString*)n;
    PATH_DEBUG (("        \"%s\", type=%s\n", 
                  self->path,
                  AlgToStr(self->alg)));
}

static 
void CC LogServer( DLNode *n, void *data )
{
    const SRAPathString* self = (const SRAPathString*)n;
    PATH_DEBUG (("        \"%s\"\n", self->path));
}

static 
void CC LogRepository ( DLNode *n, void *data )
{
    if (n != NULL)
    {
        const NCBIRepository* self = (const NCBIRepository*)n;
        PATH_DEBUG (("    type=%s\n", AlgToStr(self->type)));
        PATH_DEBUG (("    servers:\n"));
        DLListForEach ( & self->repsrv, false, LogServer, NULL );
        PATH_DEBUG (("    volumes:\n"));
        DLListForEach ( & self->vols, false, LogVolume, NULL );
    }
}

/*--------------------------------------------------------------------------
 * NCBISRAPath
 *  manages accession -> path conversion
 */
typedef struct NCBISRAPath NCBISRAPath;
struct NCBISRAPath
{
    struct SRAPath dad;

    /* working directory */
    const KDirectory *dir;

    /* repositories */
    DLList repos;
    NCBIRepository* dflt_repo; /* default repository (likely to be removed in the future versions) */ 

    atomic32_t refcount;
};

static void LogPathInfo(const NCBISRAPath* self)
{
    PATH_DEBUG (("NCBISRAPath configuration:\n"));
    DLListForEach ( & self->repos, false, LogRepository, NULL );
    PATH_DEBUG (("default repository:\n"));
    LogRepository( (DLNode*) self->dflt_repo, NULL );
}

/* Whack
 */
static
rc_t SRAPathWhack ( NCBISRAPath *self )
{
    rc_t rc = KDirectoryRelease ( self -> dir );
    if ( rc == 0 )
    {
        DLListWhack ( & self -> repos, SRARepoWhack, NULL );
        SRARepoWhack ( (DLNode*) self -> dflt_repo, NULL );
        free ( self );
    }
    return rc;
}


/* AddRef
 * Release
 */
static
rc_t CC NCBISRAPathAddRef ( const NCBISRAPath *cself )
{
    atomic32_inc ( & ( ( NCBISRAPath* ) cself ) -> refcount );
    return 0;
}

static
rc_t CC NCBISRAPathRelease ( const NCBISRAPath *cself )
{
    NCBISRAPath *self = ( NCBISRAPath* ) cself;
    if ( atomic32_dec_and_test ( & self -> refcount ) )
        return SRAPathWhack ( self );
    return 0;
}


/* Version
 *  returns the library version
 */
static
rc_t CC NCBISRAPathVersion ( const NCBISRAPath *self, uint32_t *version )
{
    * version = LIBSRAPATH_VERS;
    return 0;
}


/* Clear
 *  forget existing server and volume paths for the default repository
 */
static
rc_t CC NCBISRAPathClear ( NCBISRAPath *self )
{
    DLListWhack ( & self -> dflt_repo -> repsrv, SRAPathStringWhack, NULL );
    DLListWhack ( & self -> dflt_repo -> vols, SRAPathStringWhack, NULL );
    DLListInit ( & self -> dflt_repo -> repsrv );
    DLListInit ( & self -> dflt_repo -> vols );

    return 0;
}


/* AddPath
 *  add an alternate replication or volume path
 *
 *  "path" [ IN ] and "size" [ IN ] - sets a search path
 *
 *  "alt" [ IN ] - use std or alternate volume path algorithm
 */
static
rc_t SRAPathAddSubPath ( DLList *list, const char *path, size_t size, uint8_t alg )
{
    /* see if it's already there */
    SRAPathFindInfo pb;
    pb . path = path;
    pb . size = size;
    pb . found = 0;
    DLListDoUntil ( list, 0, SRAPathStringFind, & pb );
    if ( pb . found )
        return 0;

    /* create a new one */
    return SRAPathStringMake ( list, path, size, alg );
}

static
rc_t SRAPathAddPath ( DLList *list, const char *path, uint8_t alg )
{
    rc_t rc;
    if ( path == NULL )
        rc = RC ( rcSRA, rcMgr, rcUpdating, rcString, rcNull );
    else if ( path [ 0 ] == 0 )
        rc = RC ( rcSRA, rcMgr, rcUpdating, rcString, rcEmpty );
    else
    {
        /* treat path as a Unix-style multi-path */
        size_t size = string_size ( path );
        while ( 1 )
        {
            /* find separator */
            const char *sep = string_chr ( path, size, ':' );
            if ( sep == NULL )
                break;

            /* add sub-path */
            rc = SRAPathAddSubPath ( list, path, sep - path, alg );
            if ( rc != 0 )
                return rc;

            /* consume ':' */
            ++ sep;

            /* pop from string */
            size -= sep - path;
            path = sep;
        }

        return SRAPathAddSubPath ( list, path, size, alg );
    }
    return rc;
}

/* AddRepPath
 *  add a replication path to a repository
 *
 *  "rep" [ IN ] - NUL-terminated server search path
 *  may be a compound path with ':' separator characters, e.g.
 *  "/panfs/traces01:/panfs/traces31"
 *
 *  NB - servers are searched in the order provided,
 *  first to last, until one of them satisfies a request,
 *  at which time the successful server is placed at the
 *  head of the search path.
 */
static
rc_t CC NCBISRAPathAddRepPath ( NCBIRepository* repo, const char *rep )
{
    return SRAPathAddPath ( & repo -> repsrv, rep, alg_none );
}
static
rc_t CC NCBISRAPathAddRepPathDefault ( NCBISRAPath *self, const char *rep )
{
    return NCBISRAPathAddRepPath ( self -> dflt_repo, rep );
}

/* AddVolPath
 *  add a volume path to the default repository
 *
 *  "vol" [ IN ] - NUL-terminated volume search path
 *  may be a compound path with ':' separator characters, e.g.
 *  "sra2:sra1:sra0"
 *
 *  NB - volumes are searched in the order provided,
 *  first to last. they are never re-ordered.
 */
static
rc_t SRAPathAddAlgVolPath ( NCBIRepository* repo, const char *vol, uint8_t alg )
{
    return SRAPathAddPath ( & repo -> vols, vol, alg );
}
static
rc_t CC NCBISRAPathAddVolPathDefault ( NCBISRAPath *self, const char *vol )
{
    return SRAPathAddAlgVolPath ( self -> dflt_repo, vol, alg_ncbi);
}

/* Config
 *  configure an existing path manager
 */
static
rc_t SRAPathConfigValue ( const KConfig *kfg, const char *node_path,
    char *value, size_t value_size, const char *dflt )
{
    const KConfigNode *node;
    rc_t rc = KConfigOpenNodeRead ( kfg, & node, "%s", node_path );
    if ( rc == 0 )
    {
        size_t num_read, remaining;
        rc = KConfigNodeRead ( node, 0, value, value_size - 1, & num_read,  & remaining );
        if ( rc == 0 )
        {
            if ( remaining != 0 )
                rc = RC ( rcSRA, rcMgr, rcConstructing, rcString, rcExcessive );
            else
                value [ num_read ] = 0;
        }

        KConfigNodeRelease ( node );
    }

    if ( rc != 0 )
    {
        if ( dflt != NULL && dflt [ 0 ] != 0 )
        {
            size_t num_read = string_copy_measure ( value, value_size, dflt );
            rc = 0;
            if ( num_read == value_size )
                rc = RC ( rcSRA, rcMgr, rcConstructing, rcString, rcExcessive );
        }
        else
	    	value[0] = 0;     
    }
    return rc;
}

static 
rc_t ConfigVolume(NCBIRepository* repo, KConfig * kfg, const char* keyPref, const char* keySuff, uint8_t alg )
{
    char value [ 4096 ];
    char key [ 4096 ];
    size_t pSize = string_size(keyPref);
    size_t sSize = string_size(keySuff);
    if ( pSize + sSize >= sizeof(key))
    {
        return RC ( rcSRA, rcMgr, rcConstructing, rcString, rcExcessive );
    }
    string_copy(key, sizeof(key), keyPref, pSize);
    string_copy(key + pSize, sizeof(key) - pSize, keySuff, sSize);
    key[pSize+sSize] = 0;
    value[0] = '\0';
    if ( SRAPathConfigValue ( kfg, key, value, sizeof value, NULL ) == 0 )        
        SRAPathAddAlgVolPath ( repo, value, alg );
    return 0;
}

static
rc_t ConfigRepo(KConfig * kfg, const char *dflt, const char* reps, const char* volPref, uint8_t type, NCBIRepository** repo)
{
    char value [ 4096 ] = "";
    rc_t rc;

    /* set up a new repo */ 
    rc = SRARepoMake(repo);
    if ( rc == 0 )
    {
        (*repo)->type = type;

        /* set up servers */
        if ( SRAPathConfigValue ( kfg, reps, value, sizeof value, dflt ) == 0 )
            NCBISRAPathAddRepPath ( *repo, value );

        if ( type == alg_ncbi )
        {
            /* set up NCBI volumes */
            rc = ConfigVolume( *repo, kfg, volPref, "/ncbi/volumes", alg_ncbi );

            /* set up EBI volumes */
            if (rc == 0)
                rc = ConfigVolume( *repo, kfg, volPref, "/ebi/volumes", alg_ebi );

            /* set up DDBJ volumes */
            if (rc == 0)
                rc = ConfigVolume( *repo, kfg, volPref, "/ddbj/volumes", alg_ddbj );
        }
        else if (rc == 0)
        {
                rc = ConfigVolume( *repo, kfg, volPref, "/volumes", type);
        }
    }
    return rc;
}

static
rc_t ConfigRepoSet(DLList* repos, KConfig * kfg, const char* kfgPath, const char *dflt, uint8_t type)
{
    const KConfigNode *node;

    rc_t rc = KConfigOpenNodeRead ( kfg, & node, "%s", kfgPath );
    if ( rc == 0 )
    {
        KNamelist* children;
        rc = KConfigNodeListChild ( node, &children );
        if ( rc == 0 )
        {
            uint32_t count;
            rc = KNamelistCount ( children, &count );
            if ( rc == 0 )
            {
                uint32_t i;
                for (i = 0; i < count; ++i)
                {
                    const char* name;
                    rc = KNamelistGet ( children, i, &name );
                    if ( rc == 0 )
                    {
                        #define BufSize 4096
                        char buf[ BufSize ];
                        size_t bSize = string_size(kfgPath);
                        string_copy(buf, BufSize, kfgPath, bSize);
                        if (bSize + string_size(name) < sizeof(buf))
                        {
                            NCBIRepository* repo;
                            string_copy(buf + bSize, sizeof(buf) - bSize, name, string_size(name) + 1);
                            rc = ConfigRepo( kfg, dflt, buf, buf, type, &repo );
                            DLListPushTail( repos, (DLNode*) repo );
                        }
                        #undef BufSize
                    }
                    else
                    {
                        rc = RC ( rcSRA, rcMgr, rcConstructing, rcString, rcExcessive );
                    }
                    if ( rc != 0 )
                    {
                        break;
                    }
                }
            }
            KNamelistRelease ( children );
        }

        KConfigNodeRelease ( node );
    }
    if (GetRCState(rc) == rcNotFound)
    {
        return 0;
    }
    return rc;
}

static
rc_t SRAPathConfig ( NCBISRAPath *self )
{
    const char *dflt;
    KConfig * kfg;
    rc_t rc = KConfigMakeLocal ( & kfg, NULL );
    assert ( ( rc == 0 && kfg != NULL ) || ( rc != 0 && kfg == NULL ) );

    /* look for defaults */
    dflt = getenv ( "SRAPATH" );

    /* locate and configure all repositories */
    if ( rc == 0 )
        rc = ConfigRepoSet( & self->repos, kfg, "/sra/repository/", dflt, alg_ncbi );
    if ( rc == 0 )
        rc = ConfigRepoSet( & self->repos, kfg, "/wgs/repository/", dflt, alg_wgs );
    if ( rc == 0 )
        rc = ConfigRepoSet( & self->repos, kfg, "/refseq/repository/", dflt, alg_refseq );

    if ( rc == 0 )
    {   /* default repository for backwards compatibility */
        rc = ConfigRepo( kfg, dflt, "sra/servers", "sra", alg_ncbi, &self->dflt_repo );

        /* set up REFSEQ volumes */
        if (rc == 0)
            rc = ConfigVolume( self->dflt_repo, kfg, "", "/refseq/volumes", alg_refseq );

        /* set up WGS volumes */
        if (rc == 0)
            rc = ConfigVolume( self->dflt_repo, kfg, "sra", "/wgs/volumes", alg_wgs );
    }

    LogPathInfo(self);

    /* kfg may be NULL */        
    KConfigRelease ( kfg );

    return rc;
}


/* ParseRunAccession
 *  accession is a string matching <prefix><number>
 *  extract prefix length and accession number
 */
static
rc_t SRAPathParseRunAccession ( const char *accession, uint32_t *prefix, uint32_t *number )
{
    char *end;
    uint32_t i;

    for ( i = 0; accession [ i ] != 0; ++ i )
    {
        if ( isdigit ( accession [ i ] ) )
            break;
    }

    if ( i == 0 )
        return RC ( rcSRA, rcMgr, rcResolving, rcPath, rcInvalid );

    * prefix = i;
    * number = ( uint32_t ) strtoul ( & accession [ i ], & end, 10 );

    if (( end [ 0 ] == 0 ) || ( end [ 0 ] == '.' ))
    {
        return 0;
    }
    return RC ( rcSRA, rcMgr, rcResolving, rcPath, rcInvalid );
}


/* Full
 *  creates full path from server, volume & accession
 *
 *  "rep" [ IN ] - NUL terminated full path of replication
 *  server, e.g. "/panfs/traces01"
 *
 *  "vol" [ IN ] - NUL terminated relative path of volume,
 *  e.g. "sra2"
 *
 *  "accession" [ IN ] - NUL terminated run accession,
 *  e.g. "SRR000001"
 *
 *  "path" [ OUT ] and "path_max" [ IN ] - return buffer for
 *  NUL-terminated full path to accession.
 */
static
rc_t SRAPathFullInt ( const NCBISRAPath *self, const char *rep, const char *vol,
    const char *accession, char *path, size_t path_max, size_t bank_size )
{
    /* pick apart accession */
    uint32_t prefix, number;
    rc_t rc = SRAPathParseRunAccession ( accession, & prefix, & number );
    if ( rc == 0 )
    {
        size_t len;
        size_t total = 0;
        uint32_t bank = number / bank_size;

        if ( rep [ 0 ] != 0 && vol [ 0 ] != 0 )
        {
            /* normally write all parts at once */
            if ( rep [ strlen (rep) - 1 ] == '/') /* check for need or not a '/' between rep and vol */
                rc= string_printf ( path, path_max, &len, "%s%s/%.*s/%06u/%s"
                                 , rep
                                 , vol
                                 , ( int ) prefix, accession
                                 , bank
                                 , accession );
            else
                rc= string_printf  ( path, path_max, &len, "%s/%s/%.*s/%06u/%s"
                                 , rep
                                 , vol
                                 , ( int ) prefix, accession
                                 , bank
                                 , accession );
        }
        else
        {
            /* allow for individual rep-server and volume parts */
            if ( rep [ 0 ] != 0 )
            {
                rc = string_printf ( path, path_max, &len, "%s/", rep );
                if ( rc != 0 || ( total = len ) >= path_max )
                    return RC ( rcSRA, rcMgr, rcAccessing, rcBuffer, rcInsufficient );
            }
            else if ( vol [ 0 ] != 0 )
            {
                rc = string_printf ( & path [ total ], path_max - total, &len, "%s/", vol );
                if ( rc != 0 || ( total = len ) >= path_max )
                    return RC ( rcSRA, rcMgr, rcAccessing, rcBuffer, rcInsufficient );
            }

            /* append the accession */
            rc = string_printf ( & path [ total ], path_max - total, &len, "%.*s/%06u/%s"
                             , ( int ) prefix, accession
                             , bank
                             , accession );
        }

        /* common detection of buffer overflow */
        if ( rc != 0 || ( total += len ) >= path_max )
            rc = RC ( rcSRA, rcMgr, rcAccessing, rcBuffer, rcInsufficient );
    }

    return rc;
}

static
rc_t SRAPathFullEBI ( const NCBISRAPath *self, const char *rep, const char *vol,
    const char *accession, char *path, size_t path_max )
{
    /* pick apart accession */
    uint32_t prefix, number;
    rc_t rc = SRAPathParseRunAccession ( accession, & prefix, & number );
    if ( rc == 0 )
    {
        size_t len;
        size_t total = 0;
        uint32_t bank = number / 1000;

        if ( rep [ 0 ] != 0 && vol [ 0 ] != 0 )
        {
            /* normally write all parts at once */
            if ( rep [ strlen (rep) - 1 ] == '/') /* check for need or not a '/' between rep and vol */
                rc= string_printf ( path, path_max, &len, "%s%s/%.*s/%.*s%03u/%s"
                                 , rep
                                 , vol
                                 , ( int ) prefix, accession
                                 , ( int ) prefix, accession
                                 , bank
                                 , accession );
            else
                rc= string_printf  ( path, path_max, &len, "%s/%s/%.*s/%.*s%03u/%s"
                                 , rep
                                 , vol
                                 , ( int ) prefix, accession
                                 , ( int ) prefix, accession
                                 , bank
                                 , accession );
        }
        else
        {
            /* allow for individual rep-server and volume parts */
            if ( rep [ 0 ] != 0 )
            {
                rc = string_printf ( path, path_max, &len, "%s/", rep );
                if ( rc < 0 || ( total = len ) >= path_max )
                    return RC ( rcSRA, rcMgr, rcAccessing, rcBuffer, rcInsufficient );
            }
            else if ( vol [ 0 ] != 0 )
            {
                rc = string_printf ( & path [ total ], path_max - total, &len, "%s/", vol );
                if ( rc != 0 || ( total = len ) >= path_max )
                    return RC ( rcSRA, rcMgr, rcAccessing, rcBuffer, rcInsufficient );
            }

            /* append the accession */
            rc = string_printf ( & path [ total ], path_max - total, &len, "%.*s/%.*s%03u/%s"
                             , ( int ) prefix, accession
                             , ( int ) prefix, accession
                             , bank
                             , accession );
        }

        /* common detection of buffer overflow */
        if ( len < 0 || ( total += len ) >= path_max )
            rc = RC ( rcSRA, rcMgr, rcAccessing, rcBuffer, rcInsufficient );
    }

    return rc;
}

/*
* Refseq-style accessions come in 2 naming flavors, flat or SRA-like.
* If an accession with a flat name does not exist, use SRAPathFullInt to create an SRA-like path
*/
static
rc_t SRAPathFullREFSEQ ( const NCBISRAPath *self, const char *rep, const char *vol,
    const char *accession, char *path, size_t path_max )
{
    /* pick apart accession */
    uint32_t prefix, number;
    rc_t rc = SRAPathParseRunAccession ( accession, & prefix, & number );
    if ( rc == 0 )
    {
        size_t len;
        size_t total = 0;

        if ( rep [ 0 ] != 0 && vol [ 0 ] != 0 )
        {
            /* normally write all parts at once */
            if ( rep [ strlen (rep) - 1 ] == '/') /* check for need or not a '/' between rep and vol */
                rc= string_printf ( path, path_max, &len, "%s%s/%s"
                                 , rep
                                 , vol
                                 , accession );
            else
                rc= string_printf  ( path, path_max, &len, "%s/%s/%s"
                                 , rep
                                 , vol
                                 , accession );
        }
        else
        {
            /* allow for individual rep-server and volume parts */
            if ( rep [ 0 ] != 0 )
            {
                rc = string_printf ( path, path_max, &len, "%s/", rep );
                if ( rc != 0 || ( total = len ) >= path_max )
                    return RC ( rcSRA, rcMgr, rcAccessing, rcBuffer, rcInsufficient );
            }
            else if ( vol [ 0 ] != 0 )
            {
                rc = string_printf ( & path [ total ], path_max - total, &len, "%s/", vol );
                if ( rc != 0 || ( total = len ) >= path_max )
                    return RC ( rcSRA, rcMgr, rcAccessing, rcBuffer, rcInsufficient );
            }

            /* append the accession */
            rc = string_printf ( & path [ total ], path_max - total, &len, "%s"
                             , accession );
        }

        /* common detection of buffer overflow */
        if ( rc != 0 || ( total += len ) >= path_max )
            rc = RC ( rcSRA, rcMgr, rcAccessing, rcBuffer, rcInsufficient );
    }

    return rc;
}

/*
 * Try to locate in RefSeq Archives:
 * check for pattern '(\w{4}\d{2})[\.\d]+'; the archive is $1
 * use the scheme "x-ncbi-legrefseq" for vfs to recognize special case
 */
static
rc_t SRAPathFullREFSEQArchive(NCBISRAPath const *self,
                              char const rep[],
                              char const vol[],
                              char const accession[],
                              char path[],
                              size_t path_max
                             )
{
    size_t const rep_sz = strlen(rep);
    size_t const vol_sz = strlen(vol);
    char const *const rep_sep = (rep_sz > 0 && rep[rep_sz - 1] != '/') ? "/" : "";
    char const *const vol_sep = (vol_sz > 0 && vol[vol_sz - 1] != '/') ? "/" : "";
    size_t sz;
    unsigned i;
    VFSManager *vfs;
    rc_t rc = VFSManagerMake(&vfs);
    VPath *vpath;
    KDirectory const *dir;
    KPathType type;
    
    if (rc)
        return rc;
    
    for (i = 0; i < 4; ++i) {
        int const ch = accession[i];
        
        if (ch == 0 || !isalpha(ch))
            return RC(rcSRA, rcMgr, rcAccessing, rcPath, rcIncorrect);
    }
    for ( ; ; ++i) {
        int const ch = accession[i];
        
        if (ch == 0)
            break;
        if (ch != '.' && !isdigit(ch))
            return RC(rcSRA, rcMgr, rcAccessing, rcPath, rcIncorrect);
    }
    if (i < 8)
        return RC(rcSRA, rcMgr, rcAccessing, rcPath, rcIncorrect);
    
    rc = string_printf(path, path_max, &sz, "x-ncbi-legrefseq:%s%s%s%s%.6s", rep, rep_sep, vol, vol_sep, accession);
    if (rc) return rc;
    i = sz;
    
    rc = VPathMake(&vpath, path + 17);
    if (rc) return rc;

    rc = VFSManagerOpenDirectoryRead(vfs, &dir, vpath);
    VPathRelease(vpath);
    VFSManagerRelease(vfs);
    if (rc) return rc;
    
    type = KDirectoryPathType(dir, "tbl/%s", accession);
    KDirectoryRelease(dir);
    
    if (type != kptDir)
        return RC(rcSRA, rcMgr, rcAccessing, rcPath, rcIncorrect);

    rc = string_printf(path + i, path_max - i, &sz, "#tbl/%s", accession);
    
    return rc;
}

/*
* WGS style naming: accession name ABCD01 resolves into WGS/AB/CD/ABCD01
*/
static
rc_t SRAPathFullWGS( const NCBISRAPath *self, const char *rep, const char *vol,
    const char *accession, char *path, size_t path_max )
{
    /* pick apart accession */
    uint32_t prefix, number;
    rc_t rc = SRAPathParseRunAccession ( accession, & prefix, & number );
    if ( rc == 0 )
    {
        size_t len;
        size_t total = 0;

        if ( prefix < 4 )
        {
            return RC ( rcSRA, rcMgr, rcResolving, rcName, rcTooShort );
        }

        if ( rep [ 0 ] != 0 && vol [ 0 ] != 0 )
        {
            /* normally write all parts at once */
            if ( rep [ strlen (rep) - 1 ] == '/') /* check for need or not a '/' between rep and vol */
                rc= string_printf ( path, path_max, &len, "%s%s/WGS/%.2s/%.2s/%s"
                                 , rep
                                 , vol
                                 , accession
                                 , accession+2
                                 , accession );
            else
                rc= string_printf ( path, path_max, &len, "%s/%s/WGS/%.2s/%.2s/%s"
                                 , rep
                                 , vol
                                 , accession
                                 , accession+2
                                 , accession );
        }
        else
        {
            /* allow for individual rep-server and volume parts */
            if ( rep [ 0 ] != 0 )
            {
                rc = string_printf ( path, path_max, &len, "%s/", rep );
                if ( rc != 0 || ( total = len ) >= path_max )
                    return RC ( rcSRA, rcMgr, rcAccessing, rcBuffer, rcInsufficient );
            }
            else if ( vol [ 0 ] != 0 )
            {
                rc = string_printf ( & path [ total ], path_max - total, &len, "%s/", vol );
                if ( rc != 0 || ( total = len ) >= path_max )
                    return RC ( rcSRA, rcMgr, rcAccessing, rcBuffer, rcInsufficient );
            }

            /* append the accession */
                rc= string_printf ( & path [ total ], path_max - total, &len, "WGS/%.2s/%.2s/%s"
                                 , accession
                                 , accession+2
                                 , accession );
        }

        /* common detection of buffer overflow */
        if ( rc != 0 || ( total += len ) >= path_max )
            rc = RC ( rcSRA, rcMgr, rcAccessing, rcBuffer, rcInsufficient );
    }

    return rc;
}

static
rc_t ApplyAlg( const NCBISRAPath *self, const char *rep, const char *vol, 
    const char *accession, char *path, size_t path_max, NCBIRepository *repo, bool* found)
{
    SRAPathFindInfo pb;
    /* see if we recognize volume */
    pb . path = vol;
    pb . size = strlen ( vol );
    pb . found = 0;

    DLListDoUntil ( & repo -> vols, 0, SRAPathStringFind, & pb );
    if ( pb . found )
    {
        *found = true;
        switch (pb . alg)
        {
        case alg_ebi:
            return SRAPathFullEBI ( self, rep, vol, accession, path, path_max );
        case alg_refseq:
        {
            rc_t rc = SRAPathFullREFSEQ ( self, rep, vol, accession, path, path_max );
            if ( rc == 0) /* check for existence of accession at the root of the volume, and if not found try to apply the default path-building scheme */
            {
                switch ( KDirectoryPathType ( self -> dir, "%s", path ) )
                {
                case kptNotFound:
                case kptBadPath:
                    return SRAPathFullREFSEQArchive( self, rep, vol, accession, path, path_max );
                default:
                	return 0;
                }
            }
		}            
        case alg_wgs:
            return SRAPathFullWGS ( self, rep, vol, accession, path, path_max );
        default:
            break;
        }
    }
    *found = false;
    return 0;
}

static
rc_t CC NCBISRAPathFull ( const NCBISRAPath *self, const char *rep, const char *vol,
    const char *accession, char *path, size_t path_max )
{
    NCBIRepository *repo;
    bool found;
    rc_t rc;

    /* loop through repositories */ 
    for ( repo = ( NCBIRepository* ) DLListHead ( & self -> repos );
          repo != NULL; repo = ( NCBIRepository* ) DLNodeNext ( & repo -> n ) )
    {
        rc = ApplyAlg( self, rep, vol, accession, path, path_max, repo, &found);
        if (found)
        {
            return rc;
        }
    }
    /* try the default repository */
    rc = ApplyAlg( self, rep, vol, accession, path, path_max, self -> dflt_repo, &found);
    if (found)
    {
        return rc;
    }
    /* internal version */
    return SRAPathFullInt ( self, rep, vol, accession, path, path_max, 1024 );
}


/* Test
 *  returns true if path appears to be accession
 *  the test is a heuristic, and may return false positives
 *
 *  "path" [ IN ] - NUL terminated path to run
 */
static
bool SRAPathTestTable ( const KDirectory *dir, uint32_t type, const char *path )
{
    /* differentiate between legacy and current structure */
    switch ( KDirectoryPathType ( dir, "%s/idx", path ) )
    {
    case kptNotFound:
        switch ( KDirectoryPathType ( dir, "%s/meta", path ) )
        {
        case kptFile:
        case kptFile | kptAlias:
            switch ( KDirectoryPathType ( dir, "%s/skey", path ) )
            {
            case kptFile:
            case kptFile | kptAlias:
                return true;
            }
            break;
        }
        break;

    case kptDir:
    case kptDir | kptAlias:
        switch ( KDirectoryPathType ( dir, "%s/md/cur", path ) )
        {
        case kptFile:
        case kptFile | kptAlias:
            return true;
        }
        break;
    }

    /* not a table */
    return false;
}

static
bool SRAPathTestDatabase ( const KDirectory *dir, uint32_t type, const char *path )
{
    /* don't have a more stringent requirement for database at this time */
    return true;
}

static
bool SRAPathTestFile ( const KDirectory *dir, uint32_t type, const char *path )
{
    /* TBD - need to look at magic information */
    return false;
}

static
bool SRAPathTestInt ( const KDirectory *dir, uint32_t type, const char *path )
{
    switch ( type )
    {
    case kptDir:
    case kptDir | kptAlias:
        break;

    case kptFile:
    case kptFile | kptAlias:
        return SRAPathTestFile ( dir, type, path );

    default:
        return false;
    }

    /* detect apparent tables */
    switch ( KDirectoryPathType ( dir, "%s/col", path ) )
    {
    case kptDir:
    case kptDir | kptAlias:
        return SRAPathTestTable ( dir, type, path );
    }

    /* detect apparent databases */
    switch ( KDirectoryPathType ( dir, "%s/tbl", path ) )
    {
    case kptDir:
    case kptDir | kptAlias:
        return SRAPathTestDatabase ( dir, type, path );
    }

    /* not a recognized accession */
    return false;
}

static
bool CC NCBISRAPathTest ( const NCBISRAPath *self, const char *path )
{
    return SRAPathTestInt ( self -> dir,
                            KDirectoryPathType ( self -> dir, "%s", path ), path );
}


/* FindOnServer
 *  find accession on rep-server 
 */
static
rc_t SRAPathFindOnServer ( const NCBISRAPath *self, const NCBIRepository *repo, const SRAPathString *srv,
    const char *accession, char *path, size_t path_max, int vol_type )
{
    const SRAPathString *vol;

    PATH_DEBUG (("SRAPathFindOnServer(%s)\n", srv->path));

    for ( vol = ( const SRAPathString* ) DLListHead ( & repo -> vols );
          vol != NULL; vol = ( const SRAPathString* ) DLNodeNext ( & vol -> n ) )
    {
        if ( vol_type == alg_none || vol_type == vol->alg )
        {
            rc_t rc;

            PATH_DEBUG (("SRAPathFindOnServer trying volume %s\n", vol->path));

            switch ( vol -> alg )
            {
            case alg_ebi:
                rc = SRAPathFullEBI ( self, srv -> path, vol -> path, accession, path, path_max );
                break;
            case alg_refseq:
                rc = SRAPathFullREFSEQ ( self, srv -> path, vol -> path, accession, path, path_max );
                if ( rc == 0) /* check for existence of accession at the root of the volume, and if not found try to apply the default path-building scheme */
                {
                    switch ( KDirectoryPathType ( self -> dir, "%s", path ) )
                    {
                    case kptNotFound:
                    case kptBadPath:
                        rc = SRAPathFullREFSEQArchive( self, srv -> path, vol -> path, accession, path, path_max );
                        if (rc == 0) {
                            PATH_DEBUG (("SRAPathFindOnServer: found(%s)\n", path));
                            return 0;
                        }
                        break;
                    default:
                    	return 0;
                    }
                }
                break;
            case alg_wgs:
                rc = SRAPathFullWGS ( self, srv -> path, vol -> path, accession, path, path_max );
                break;
            default:
                rc = SRAPathFullInt ( self, srv -> path, vol -> path, accession, path, path_max, 1024 );
                break;
            }
            if ( rc == 0 )
            {
                switch ( KDirectoryPathType ( self -> dir, "%s", path ) )
                {
                case kptNotFound:
                case kptBadPath:
                    break;
                default:
                    PATH_DEBUG (("SRAPathFindOnServer: found(%s)\n", path));
                    return 0;
                }
            }
            else
            {
                if ( GetRCState( rc ) == rcInsufficient )
                    return rc;
            }
        }
    }

    return RC ( rcSRA, rcMgr, rcSelecting, rcPath, rcNotFound );
}

/* FindInRepo
 *  find accession in a repository
 */
static
rc_t SRAPathFindInRepo ( const NCBISRAPath *self, NCBIRepository *repo, const char *accession, 
                         char *path, size_t path_max, size_t *rep_len, int vol_type )
{
    SRAPathString *srv;

    PATH_DEBUG (("SRAPathFindInRepo(%s)\n", AlgToStr(repo->type)));

    /* look for accession on a rep-server */
    for ( srv = ( SRAPathString* ) DLListHead ( & repo -> repsrv ); srv != NULL; srv = ( SRAPathString* ) DLNodeNext ( & srv -> n ) )
    {
        /* try with this server */
        rc_t rc = SRAPathFindOnServer ( self, repo, srv, accession, path, path_max, vol_type );
        if ( rc == 0 )
        {
            /* make sure server is at head of list */
            if ( DLNodePrev ( & srv -> n ) != NULL )
            {
                DLListUnlink ( & repo -> repsrv, & srv -> n );
                DLListPushHead ( & repo -> repsrv, & srv -> n );
            }

            if ( rep_len != NULL )
                * rep_len = strlen ( srv -> path );
            
            return 0;
        }

        if ( GetRCState ( rc ) != rcNotFound )
            return rc;
    }

    return RC ( rcSRA, rcMgr, rcSelecting, rcPath, rcNotFound );
}

/* FindInRepoByType
 *  find accession in a repository of a given type
 */
static
rc_t SRAPathFindInRepoByType ( const NCBISRAPath *self, const char *accession, char *path, size_t path_max, size_t *rep_len, int repo_type, int vol_type )
{
    /* loop through all repositories */
    NCBIRepository *repo; 
    for ( repo = ( NCBIRepository* ) DLListHead ( & self -> repos ); repo != NULL; repo = ( NCBIRepository* ) DLNodeNext ( & repo -> n ) )
    {
        if ( repo->type == repo_type && SRAPathFindInRepo(self, repo, accession, path, path_max, rep_len, vol_type) == 0 )
            return 0;
    }
    return RC ( rcSRA, rcMgr, rcSelecting, rcPath, rcNotFound );
}

/* FastFind
 * Uses heuristics to select the repository most likely to contain the accession, then tries to locate the acecssion in the repository.
*/
static
rc_t FindFast( const NCBISRAPath *cself, const char *accession, char *path, size_t path_max, size_t *rep_len )
{
    /*TODO: look up cache first */
    
    /* recognize known naming schemes */
    size_t size = string_size(accession);
    if ( string_cmp(accession, size, "SRR", 3, 3) == 0 )
        return SRAPathFindInRepoByType(cself, accession, path, path_max, rep_len, alg_ncbi, alg_ncbi);

    if ( string_cmp(accession, size, "ERR", 3, 3) == 0 )
        return SRAPathFindInRepoByType(cself, accession, path, path_max, rep_len, alg_ncbi, alg_ebi);

    if ( string_cmp(accession, size, "DRR", 3, 3) == 0 )
        return SRAPathFindInRepoByType(cself, accession, path, path_max, rep_len, alg_ncbi, alg_ddbj);
        
    if ( string_chr(accession, size, '.') != NULL )
        return SRAPathFindInRepoByType(cself, accession, path, path_max, rep_len, alg_refseq, alg_none);    
        
    if ( size > 2 && isdigit(accession[size-1]) && isdigit(accession[size-2]) && ! isdigit(accession[size-3]) ) 
        return SRAPathFindInRepoByType(cself, accession, path, path_max, rep_len, alg_wgs, alg_none);    
        
    return RC ( rcSRA, rcMgr, rcSelecting, rcPath, rcNotFound );
}

/* Find
 *  finds location of run within rep-server/volume matrix
 *
 *  "accession" [ IN ] - NUL terminated run accession,
 *   e.g. "SRR000001"
 *
 *  "path" [ OUT ] and "path_max" [ IN ] - return buffer for
 *  NUL-terminated full path to accession.
 *
 *  returns 0 if path exists, rc state rcNotFound if
 *  path cannot be found, and rcInsufficient if buffer is
 *  too small.
 */
static
rc_t CC NCBISRAPathFindWithRepLen ( const NCBISRAPath *cself, const char *accession, char *path, size_t path_max, size_t *rep_len )
{
    rc_t rc;
    NCBIRepository *repo;

    PATH_DEBUG(("NCBISRAPathFindWithRepLen(%s)\n", accession));

    rc = FindFast( cself, accession, path, path_max, rep_len );
    if ( rc == 0 )
        return 0;
        
    /* loop through all repositories */ 
    for ( repo = ( NCBIRepository* ) DLListHead ( & cself -> repos ); repo != NULL; repo = ( NCBIRepository* ) DLNodeNext ( & repo -> n ) )
    {
        rc = SRAPathFindInRepo(cself, repo, accession, path, path_max, rep_len, alg_none);
        if ( rc == 0 )
            return 0;
    }
    /* default repository */
    return SRAPathFindInRepo(cself, cself -> dflt_repo, accession, path, path_max, rep_len, alg_none);
}

struct SRAPath_vt_v2 vtSRAPath =
{
    2, 1,
    NCBISRAPathAddRef,
    NCBISRAPathRelease,
    NCBISRAPathVersion,
    NCBISRAPathClear,
    NCBISRAPathAddRepPathDefault,
    NCBISRAPathAddVolPathDefault,
    NCBISRAPathFull,
    NCBISRAPathTest,
    NCBISRAPathFindWithRepLen
};

/* Make
 *  create path manager
 *
 *  the path manager should already be configured with
 *  standard search paths, but can be augmented by using
 *  the Add*Path messages.
 *
 *  "dir" [ IN, NULL OKAY ] - optional root directory to use
 *  attaches a new reference
 */
MOD_EXPORT
rc_t CC SRAPathMakeImpl ( SRAPath **pm, const KDirectory *dir )
{
    rc_t rc;

    if ( pm == NULL )
        rc = RC ( rcSRA, rcMgr, rcConstructing, rcParam, rcNull );
    else
    {
        NCBISRAPath *p = (NCBISRAPath *) malloc ( sizeof * p );
        if ( p == NULL )
            rc = RC ( rcSRA, rcMgr, rcConstructing, rcMemory, rcExhausted );
        else
        {
            p -> dad . vt = ( SRAPath_vt* ) & vtSRAPath;
            p -> dir = dir;
            if ( dir != NULL )
                rc = KDirectoryAddRef ( dir );
            else
            {
                KDirectory *wd;
                rc = KDirectoryNativeDir ( & wd );
                p -> dir = wd;
            }

            if ( rc != 0 )
                free ( p );
            else
            {
                DLListInit ( & p -> repos );
                p -> dflt_repo = NULL;
                atomic32_set ( & p -> refcount, 1 );

                /* the object is now complete */
                rc = SRAPathConfig ( p );
                if ( rc == 0 )
                {
                    * pm = & p -> dad;
                    return 0;
                }

                SRAPathWhack ( p );
            }
        }

        * pm = NULL;
    }

    return rc;
}
