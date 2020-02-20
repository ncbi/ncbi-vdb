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

#include <kfg/kfg-priv.h>
#include "kfg-priv.h"

struct KfgConfigNamelist;
#define KNAMELIST_IMPL struct KfgConfigNamelist
#include <klib/namelist.h>
#include <klib/impl.h>

#include <klib/container.h>
#include <klib/data-buffer.h> /* KDataBuffer */
#include <klib/debug.h>
#include <klib/log.h>
#include <klib/out.h> /* OUTMSG */
#include <klib/printf.h>
#include <klib/rc.h>
#include <klib/refcount.h>
#include <klib/text.h>
#include <klib/token.h>
#include <klib/klib-priv.h>

#include <kfs/directory.h>
#include <kfs/gzip.h> /* KFileMakeGzipForRead */
#include <kfs/subfile.h> /* KFileMakeSubRead */
#include <kfs/file.h>
#include <kfs/dyload.h>
#include <kfs/mmap.h>
#include <vfs/path.h>
#include <strtol.h>
#include <sysalloc.h>

#include <string.h>
#include <ctype.h>
#include <os-native.h>
#include <stdlib.h>
#include <assert.h>
#include <va_copy.h>

#if !WINDOWS
    #include <sys/utsname.h>
#endif

#include "kfg-parse.h"
#include "config-tokens.h"

#include "../vfs/resolver-cgi.h" /* RESOLVER_CGI */

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/* the leaf name of the user's modifiable settings,
   must reside in the user's $HOME/.ncbi directory */
#define MAGIC_LEAF_NAME "user-settings.mkfg"

static bool s_disable_user_settings = false;

static const char * s_ngc_file = NULL;


/*----------------------------------------------------------------------------*/
static const char default_kfg[] = {
"/config/default = \"true\"\n"
"/repository/user/main/public/apps/file/volumes/flat = \"files\"\n"
"/repository/user/main/public/apps/nakmer/volumes/nakmerFlat = \"nannot\"\n"
"/repository/user/main/public/apps/nannot/volumes/nannotFlat = \"nannot\"\n"
"/repository/user/main/public/apps/refseq/volumes/refseq = \"refseq\"\n"
"/repository/user/main/public/apps/sra/volumes/sraFlat = \"sra\"\n"
"/repository/user/main/public/apps/sraPileup/volumes/withExtFlat = \"sra\"\n"
"/repository/user/main/public/apps/sraRealign/volumes/withExtFlat = \"sra\"\n"
"/repository/user/main/public/apps/wgs/volumes/wgsFlat = \"wgs\"\n"
"/repository/remote/main/CGI/resolver-cgi = "
             "\"https://trace.ncbi.nlm.nih.gov/Traces/names/names.fcgi\"\n"
"/repository/remote/protected/CGI/resolver-cgi = "
             "\"https://trace.ncbi.nlm.nih.gov/Traces/names/names.fcgi\"\n"
"/repository/remote/main/SDL.2/resolver-cgi = "
             "\"https://locate.ncbi.nlm.nih.gov/sdl/2/retrieve\"\n"
"/repository/remote/protected/SDL.2/resolver-cgi = "
             "\"https://locate.ncbi.nlm.nih.gov/sdl/2/retrieve\"\n"
"/tools/ascp/max_rate = \"450m\"\n"
};
/*----------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
 * KConfig
 */
static
rc_t KConfigSever ( const KConfig *self );


/*--------------------------------------------------------------------------
 * KConfigIncluded
 *  node within configuration tree
 */

typedef struct KConfigIncluded KConfigIncluded;
struct KConfigIncluded
{
    BSTNode n;
    bool is_magic_file;
    char path [ 1 ];
};


static
void CC KConfigIncludedWhack ( BSTNode *n, void *ignore )
{
    free ( n );
}

static
int64_t CC KConfigIncludedSort ( const BSTNode *item, const BSTNode *n )
{
    const KConfigIncluded *a = ( const KConfigIncluded* ) item;
    const KConfigIncluded *b = ( const KConfigIncluded* ) n;
    return strcmp ( a -> path, b -> path );
}

enum {
    eInternalFalse = false, /* internal = false: non internal nodes */
    eInternalTrue  = true,  /* internal = true ; internal nodes: read-only */
    eInternalTrueUpdatable
                       /* internal = true ; internal nodes, but can be updated :
                                            "kfg/dir", "kfg/name" */
} EInternal;
typedef uint32_t TInternal;

/*--------------------------------------------------------------------------
 * KConfigNode
 *  node within configuration tree
 */
struct KConfigNode
{
    BSTNode n;

    /* needs to hold a dependency reference to mgr */
    KConfig *mgr;

    /* uncounted reference to parent node */
    KConfigNode *dad;

    /* File node came from; null if created during runtime*/
    KConfigIncluded *came_from;

    /* named children - always unique */
    BSTree children;

    /* named attributes */
    BSTree attr;

    /* node value */
    char * val_buffer;
    String value;

    /* node name */
    String name;

    KRefcount refcount;

    TInternal internal; /* EInternal */
    bool read_only;
    bool dirty;
};

/* replace this once we introduce attributes */
#define KConfigAttrWhack NULL


/* Whack
 */
static
void CC KConfigNodeWhack ( BSTNode *n, void * data )
{
    KConfigNode *self = ( KConfigNode* ) n;
    KConfig *mgr = data;

    if ( mgr == NULL )
    {
        /* just releasing reference */
        KConfigSever ( self -> mgr );
        self -> mgr = NULL;
        self -> read_only = false;
    }
    else
    {
        /* tearing down structure */
        BSTreeWhack ( & self -> children, KConfigNodeWhack, mgr );
        BSTreeWhack ( & self -> attr, KConfigAttrWhack, mgr );
        free ( self -> val_buffer );
        free ( self );
    }
}

/* Init
 */
static
void KConfigNodeInit ( KConfigNode *self, const String *name )
{
    /* clear out here, ignoring the string space */
    memset ( self, 0, sizeof * self );

    /* initialize name early for the sake of KRefcountInit */
    string_copy ( ( char* ) ( self + 1 ), name -> size + 1, name -> addr, name -> size );
    StringInit ( & self -> name, ( char* ) ( self + 1 ), name -> size, name -> len );
    StringInit ( & self -> value, "", 0, 0 );
    KRefcountInit ( & self -> refcount, 0, "KConfigNode", "init", self -> name . addr );
}

/* Make
 */
static
rc_t KConfigNodeMake ( KConfigNode **n, const String *name )
{
    /* don't use calloc, because of string space */
    KConfigNode *cn = malloc ( sizeof * cn + name -> size + 1 );
    if ( cn == NULL )
    {
        rc_t rc = RC ( rcKFG, rcNode, rcCreating, rcMemory, rcExhausted );
        PLOGERR (klogErr, (klogErr, rc, "Unable to create a config item for $(i)", "i=%S", name));
        return rc;
    }
    KConfigNodeInit ( cn, name );
    * n = cn;

    return 0;
}


/* Cmp
 * Sort
 */
static
int64_t CC KConfigNodeCmp ( const void *item, const BSTNode *n )
{
    const String *a = ( const String* ) item;
    const KConfigNode *b = ( const KConfigNode* ) n;
    return StringCompare ( a, & b -> name );
}

static
int64_t CC KConfigNodeSort ( const BSTNode *item, const BSTNode *n )
{
    const KConfigNode *a = ( const KConfigNode* ) item;
    const KConfigNode *b = ( const KConfigNode* ) n;
    return StringCompare ( & a -> name, & b -> name );
}


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
LIB_EXPORT rc_t CC KConfigNodeAddRef ( const KConfigNode *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KConfigNode" ) )
        {
        case krefLimit:
            return RC ( rcKFG, rcNode, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC KConfigNodeRelease ( const KConfigNode *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KConfigNode" ) )
        {
        case krefWhack:
            KConfigNodeWhack ( & ( ( KConfigNode* ) self ) -> n, NULL );
        break;
        case krefNegative:
            return RC ( rcKFG, rcNode, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}


/*--------------------------------------------------------------------------
 * KConfig
 *  configuration parameter manager
 */
struct KConfig
{
    BSTree tree;
    BSTree included;
    KDualRef refcount;
    KConfigIncluded *current_file;

    char * load_path;
    size_t load_path_sz_tmp;

    const char *magic_file_path;
    size_t magic_file_path_size;

    bool disableFileLoadOnKfgCreating;

    bool dirty;
    bool initialized;
};

static atomic_ptr_t G_kfg;

rc_t KConfigAppendToLoadPath(KConfig *self, const char* chunk)
{
    rc_t rc = 0;
    size_t new_sz = 0;

    assert(self);

    if (chunk == NULL || chunk[0] == '\0') {
        return rc;
    }

    if (self->load_path == NULL) {
        self->load_path_sz_tmp = PATH_MAX;
        self->load_path = malloc(self->load_path_sz_tmp);
        if (self->load_path == NULL) {
            return RC ( rcKFG, rcMgr, rcCreating, rcMemory, rcExhausted );
        }
        self->load_path[0] = '\0';
    }

    new_sz = string_size(self->load_path) + 1 + string_size(chunk) + 1;
    if (self->load_path_sz_tmp < new_sz) {
        self->load_path_sz_tmp = 2 * new_sz;
        self->load_path = realloc(self->load_path, self->load_path_sz_tmp);
        if (self->load_path == NULL) {
            return RC ( rcKFG, rcMgr, rcCreating, rcMemory, rcExhausted );
        }
    }

    if (self->load_path[0] != '\0') {
        strcat(self->load_path, ":");
    }
    strcat(self->load_path, chunk);

    return rc;
}


/* Whack
 */
static
rc_t KConfigEmpty ( KConfig * self)
{
    if (self)
    {
        BSTreeWhack ( & self -> tree, KConfigNodeWhack, self );
        BSTreeWhack ( & self -> included, KConfigIncludedWhack, NULL );

        self -> magic_file_path_size = 0;
        free ( ( void* ) self -> magic_file_path );
        self -> magic_file_path = NULL;

        self->load_path_sz_tmp = 0;
        free ( self->load_path );
        self->load_path = NULL;
    }
    return 0;
}

static
rc_t find_home_directory ( KDyld *dyld,
    const KDirectory **dir, bool release_cached_dir )
{
    static const KDirectory * cached_dir = NULL;
    static rc_t cached_rc = 0;
    rc_t rc;

    if (release_cached_dir) {
        rc = KDirectoryRelease(cached_dir);
        cached_dir = NULL;
        cached_rc = 0;
        return rc;
    }
    else if ( cached_dir != NULL )
    {
        rc = KDirectoryAddRef ( cached_dir );
        if ( rc == 0 ) {
            * dir = cached_dir;
        }
        return rc;
    }
    else if ( cached_rc != 0 )
    {
        return cached_rc;
    }

    rc = KDyldHomeDirectory ( dyld, dir, ( fptr_t ) KConfigMake );
    if ( rc == 0  &&  KDirectoryAddRef ( * dir ) == 0 )
    {
        cached_dir = * dir;
    }
    else
    {
        cached_rc = rc;
    }

    return rc;
}

static
rc_t KConfigWhack ( KConfig *self )
{
    bool release_cached_dir = true;
    find_home_directory(NULL, NULL, release_cached_dir);

    if ( self == G_kfg.ptr )
        atomic_test_and_set_ptr ( & G_kfg, NULL, self );

    KConfigEmpty (self);

    free ( self );

    return 0;
}

/* Init
 */
static
void KConfigInit ( KConfig *self, KConfigNode * root )
{
    KDualRefInit ( & self -> refcount, 1, 0, "KConfig", "init", "kfg" );
    BSTreeInit ( & self -> tree );
    self -> dirty = false;
    self -> initialized = false;
    BSTreeInit ( & self -> included );
    BSTreeInsert ( & self -> tree, & root -> n, KConfigNodeSort );
    self -> magic_file_path = NULL;
}


/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC KConfigAddRef ( const KConfig *self )
{
    if ( self != NULL )
    {
        switch ( KDualRefAdd ( & self -> refcount, "KConfig" ) )
        {
        case krefLimit:
            return RC ( rcKFG, rcMgr, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC KConfigRelease ( const KConfig *self )
{
    if ( self != NULL )
    {
        switch ( KDualRefDrop ( & self -> refcount, "KConfig" ) )
        {
        case krefWhack:
            return KConfigWhack ( ( KConfig* ) self );
        case krefLimit:
            return RC ( rcKFG, rcMgr, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

static
KConfig *KConfigAttach ( const KConfig *self )
{
    if ( self != NULL )
    {
        switch ( KDualRefAddDep ( & self -> refcount, "KConfig" ) )
        {
        case krefLimit:
            return NULL;
        }
    }
    return ( KConfig* ) self;
}

static
rc_t KConfigSever ( const KConfig *self )
{
    if ( self != NULL )
    {
        switch ( KDualRefDropDep ( & self -> refcount, "KConfig" ) )
        {
        case krefWhack:
            return KConfigWhack ( ( KConfig* ) self );
        case krefLimit:
            return RC ( rcKFG, rcMgr, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}


/* init_token_source
 */
static
rc_t init_token_source ( KTokenText *tt, KTokenSource *src,
                         char *full, size_t fsize, const char *srcpath, const char *path, va_list args )
{
    size_t num_writ;
    rc_t rc = 0;

    if (args == NULL)
        num_writ = string_copy ( full, fsize, path, string_size ( path ));
    else
        rc = string_vprintf ( full, fsize, & num_writ, path, args );
    if ( rc == 0 )
    {
        String text, fpath;
        StringInit ( & text, full, num_writ, string_len ( full, num_writ ) );
        StringInitCString ( & fpath, srcpath );
        KTokenTextInit ( tt, & text, & fpath );
        KTokenSourceInit ( src, tt );
    }
    return rc;
}

/* Find
 */
static
KToken *KConfigNodeFind ( const KConfigNode *self, const KConfigNode **n, KTokenSource *src, KToken *t )
{
    * n = NULL;

    while ( t -> id != eEndOfInput )
    {
        switch ( t -> id )
        {
        case ePeriod:
            break;
        case eDblPeriod:
            if ( self -> dad == NULL )
                return NULL;
            self = self -> dad;
            break;
        case eDecimal:
        case eHex:
        case eOctal:
        case eIdent:
        case eName:
            self = ( const KConfigNode* ) BSTreeFind
                ( & self -> children, &t -> str, KConfigNodeCmp );
            if ( self == NULL )
                return t;
            break;
        default:
            * n = self;
            return t;
        }

        if ( KTokenizerNext ( kPOSIXPathTokenizer, src, t ) -> id != eFwdSlash )
            break;

        KTokenizerNext ( kPOSIXPathTokenizer, src, t );
    }

    * n = self;
    return t;
}

/* Create
 */
static
KToken *KConfigNodeCreate ( KConfigNode *self, KConfigNode **n, KTokenSource *src, KToken *t,
                            KConfigIncluded *current_file )
{
    bool created = false;
    KConfigNode * nself;
    * n = NULL;

    while ( t -> id != eEndOfInput )
    {
        switch ( t -> id )
        {
        case ePeriod:
            break;
        case eDblPeriod:
            if ( self -> dad == NULL )
                return NULL;
            self = self -> dad;
            break;
        case eDecimal:
        case eHex:
        case eOctal:
        case eName:
        case eIdent:
            nself = ( KConfigNode* ) BSTreeFind
                ( & self -> children, & t -> str, KConfigNodeCmp );
            if ( nself == NULL )
            {
                KConfigNode *child;
                rc_t rc = KConfigNodeMake ( & child, & t -> str );
                if ( rc != 0 )
                    return t;
                BSTreeInsert ( & self -> children, & child -> n, KConfigNodeSort );
                child -> dad = self;
                self = child;
                created = true;
            }
            else
            {
                self = nself;
            }
            break;
        default:
            * n = self;
            return t;
        }

        if ( KTokenizerNext ( kPOSIXPathTokenizer, src, t ) -> id != eFwdSlash )
            break;

        KTokenizerNext ( kPOSIXPathTokenizer, src, t );
    }

    if ( created )
        self -> came_from = current_file;

    * n = self;
    return t;
}


/* OpenNodeRead
 * VOpenNodeRead
 *  opens a configuration node
 *
 *  "node" [ OUT ] - return parameter for indicated configuration node
 *
 *  "path" [ IN, NULL OKAY ] - optional path for specifying named
 *  node within configuration hierarchy. paths will be interpreted as
 *  if they were file system paths, using '/' as separator. the
 *  special values NULL and "" are interpreted as "."
 */
static
rc_t KConfigNodeVOpenNodeReadInt ( const KConfigNode *self, const KConfig *mgr,
                                   const KConfigNode **node, const char *path, va_list args )
{
    rc_t rc;

    if ( node == NULL )
    {
        rc = RC ( rcKFG, rcNode, rcOpening, rcParam, rcNull );
        PLOGERR (klogErr, (klogErr, rc, "faile to provide node to open $(n)", "n=%s", path));
    }
    else
    {
        if ( self == NULL )
        {
            rc = RC ( rcKFG, rcNode, rcOpening, rcSelf, rcNull );
            PLOGERR (klogErr, (klogErr, rc, "failed to provide node reference for opening $(n)", "n=%s", path));
        }
        else
        {
            if ( path == NULL || path [ 0 ] == 0 )
            {
                * node = self;
                rc = 0;
            }
            else
            {
                KTokenText tt;
                KTokenSource src;
                char full [ 4096 ];

                rc = init_token_source ( & tt, & src, full, sizeof full, "", path, args );
                if ( rc == 0 )
                {
                    /* look ahead */
                    KToken t;

                    /* skip over fwd slashes */
                    do
                        KTokenizerNext ( kPOSIXPathTokenizer, & src, & t );
                    while ( t.id == eFwdSlash );

                    /* follow path */
                    if ( KConfigNodeFind ( self, node, & src, & t ) == NULL )
                    {
                        rc = RC ( rcKFG, rcNode, rcOpening, rcPath, rcInvalid );
                        PLOGERR (klogErr, (klogErr, rc, "bad path $(p)", "p=%s", path));
                    }
                    if ( ( self = * node ) == NULL )
                    {
                        rc = SILENT_RC ( rcKFG, rcNode, rcOpening, rcPath, rcNotFound );
                        /* don't complain about this
                           PLOGERR (klogErr, (klogErr, rc, "can't find symbol $(p)", "p=%s", path));
                        */
                    }
                    else if ( t . id != eEndOfInput )
                    {
                        rc = RC ( rcKFG, rcNode, rcOpening, rcPath, rcInvalid );
                        PLOGERR (klogErr, (klogErr, rc, "bad path $(p)", "p=%s", path));
                    }
                }
            }

            if ( rc == 0 )
            {
                /* open node for read */
                if ( self -> read_only )
                {
                    assert ( self -> mgr == mgr );
                    return KConfigNodeAddRef ( self );
                }

                /* check to see if already open */
                if ( atomic32_read ( & self -> refcount ) == 0 )
                {
                    ( ( KConfigNode* ) self ) -> mgr = KConfigAttach ( mgr );
                    ( ( KConfigNode* ) self ) -> read_only = true;
                    return KConfigNodeAddRef ( self );
                }

                rc = RC ( rcKFG, rcNode, rcOpening, rcNode, rcBusy );
            }
        }

        * node = NULL;
    }

    return rc;
}


LIB_EXPORT rc_t CC KConfigNodeGetMgr( const KConfigNode * self, KConfig ** mgr )
{
    if ( self == NULL )
        return RC ( rcKFG, rcNode, rcOpening, rcSelf, rcNull );
    if ( mgr == NULL )
        return RC ( rcKFG, rcNode, rcOpening, rcParam, rcNull );
    *mgr = self->mgr;
    return KConfigAddRef ( *mgr );
}


LIB_EXPORT rc_t CC KConfigNodeVOpenNodeRead ( const KConfigNode *self,
                                              const KConfigNode **node, const char *path, va_list args )
{
    if ( self != NULL )
        return KConfigNodeVOpenNodeReadInt ( self, self -> mgr, node, path, args );

    if ( node == NULL )
        return RC ( rcKFG, rcNode, rcOpening, rcParam, rcNull );

    * node = NULL;
    return RC ( rcKFG, rcNode, rcOpening, rcSelf, rcNull );
}

LIB_EXPORT rc_t CC KConfigNodeOpenNodeRead ( const KConfigNode *self,
                                             const KConfigNode **node, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KConfigNodeVOpenNodeRead ( self, node, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KConfigVOpenNodeRead ( const KConfig *self,
                                          const KConfigNode **node, const char *path, va_list args )
{
    rc_t rc;

    if ( node == NULL )
        rc = RC ( rcKFG, rcMgr, rcOpening, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcKFG, rcMgr, rcOpening, rcSelf, rcNull );
        else if (self->tree.root == NULL)
            rc = RC ( rcKFG, rcMgr, rcOpening, rcPath, rcNotFound );
        else
        {
            return KConfigNodeVOpenNodeReadInt
                ( (const KConfigNode *) self -> tree . root, self, node, path, args );
        }

        * node = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC KConfigOpenNodeRead ( const KConfig *self,
                                         const KConfigNode **node, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KConfigVOpenNodeRead ( self, node, path, args );
    va_end ( args );

    return rc;
}


/* OpenNodeUpdate
 * VOpenNodeUpdate
 *  opens a configuration node
 *
 *  "node" [ OUT ] - return parameter for indicated configuration node
 *
 *  "path" [ IN, NULL OKAY ] - optional path for specifying named
 *  node within configuration hierarchy. paths will be interpreted as
 *  if they were file system paths, using '/' as separator. the
 *  special values NULL and "" are interpreted as "."
 */
static
rc_t KConfigNodeVOpenNodeUpdateInt ( KConfigNode *self, KConfig *mgr,
                                     KConfigNode **node, const char *path, va_list args )
{
    rc_t rc;

    if ( node == NULL )
        rc = RC ( rcKFG, rcNode, rcOpening, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcKFG, rcNode, rcOpening, rcSelf, rcNull );
        else
        {
            if ( path == NULL || path [ 0 ] == 0 )
            {
                * node = self;
                rc = 0;
            }
            else
            {
                KTokenText tt;
                KTokenSource src;
                char full [ 4096 ];

                rc = init_token_source ( & tt, & src, full, sizeof full, "", path, args );
                if ( rc == 0 )
                {
                    /* look ahead */
                    KToken t;

                    do
                        KTokenizerNext ( kPOSIXPathTokenizer, & src, & t );
                    while ( t.id == eFwdSlash);

                    /* follow path */
                    assert ( mgr != NULL );
                    if ( KConfigNodeCreate ( self, node, & src, & t, mgr -> current_file ) == NULL )
                        return RC ( rcKFG, rcNode, rcOpening, rcPath, rcInvalid );
                    if ( ( self = * node ) == NULL )
                        rc = RC ( rcKFG, rcNode, rcOpening, rcMemory, rcExhausted );
                    else if ( t . id != eEndOfInput )
                        rc = RC ( rcKFG, rcNode, rcOpening, rcPath, rcInvalid );
                }
            }

            if ( rc == 0 )
            {
                /* check to see if internal */
                if ( self -> internal == eInternalTrue )
                    rc = RC ( rcKFG, rcNode, rcOpening, rcNode, rcReadonly );
                else
                {
                    /* check to see if open */
                    if ( atomic32_read ( & self -> refcount ) == 0 )
                    {
                        self -> mgr = KConfigAttach ( mgr );
                        assert ( ! self -> read_only );
                        return KConfigNodeAddRef ( self );
                    }

                    rc = RC ( rcKFG, rcNode, rcOpening, rcNode, rcBusy );
                }
            }
        }

        * node = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC KConfigNodeVOpenNodeUpdate ( KConfigNode *self,
                                                KConfigNode **node, const char *path, va_list args )
{
    if ( self != NULL )
        return KConfigNodeVOpenNodeUpdateInt ( self, self -> mgr, node, path, args );

    if ( node == NULL )
        return RC ( rcKFG, rcNode, rcOpening, rcParam, rcNull );

    * node = NULL;
    return RC ( rcKFG, rcNode, rcOpening, rcSelf, rcNull );
}

LIB_EXPORT rc_t CC KConfigNodeOpenNodeUpdate ( KConfigNode *self,
                                               KConfigNode **node, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KConfigNodeVOpenNodeUpdate ( self, node, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KConfigVOpenNodeUpdate ( KConfig *self,
                                            KConfigNode **node, const char *path, va_list args )
{
    rc_t rc;

    if ( node == NULL )
        rc = RC ( rcKFG, rcMgr, rcOpening, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcKFG, rcMgr, rcOpening, rcSelf, rcNull );
        else if (self->tree.root == NULL)
            rc = RC ( rcKFG, rcMgr, rcOpening, rcSelf, rcCorrupt );
        else
        {
            return KConfigNodeVOpenNodeUpdateInt
                ( ( KConfigNode* ) self -> tree . root, self, node, path, args );
        }

        * node = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC KConfigOpenNodeUpdate ( KConfig *self,
                                           KConfigNode **node, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KConfigVOpenNodeUpdate ( self, node, path, args );
    va_end ( args );

    return rc;
}


/* Read
 *  read a node value
 *
 *  "offset" [ IN ] - initial offset into configuration
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_read" [ OUT ] - number of bytes actually read
 *
 *  "remaining" [ OUT, NULL OKAY ] - optional return parameter for
 *  the number of bytes remaining to be read.
 *  specifically, "offset" + "num_read" + "remaining" == sizeof node data
 */
LIB_EXPORT rc_t CC KConfigNodeRead ( const KConfigNode *self,
                                     size_t offset, char *buffer, size_t bsize,
                                     size_t *num_read, size_t *remaining )
{
    rc_t rc;
    size_t dummy;

    if ( remaining == NULL )
        remaining = & dummy;

    if ( num_read == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcKFG, rcNode, rcReading, rcSelf, rcNull );
        else if ( buffer == NULL && bsize != 0 )
            rc = RC ( rcKFG, rcNode, rcReading, rcBuffer, rcNull );
        else if ( offset >= self -> value . size )
            rc = 0;
        else
        {
            size_t avail = * remaining = self -> value . size - offset;
            if ( avail > bsize )
                avail = bsize;
            if ( avail > 0 )
                memmove ( buffer, & self -> value . addr [ offset ], avail );
            * num_read = avail;
            * remaining -= avail;
            return 0;
        }

        * num_read = 0;
    }

    * remaining = 0;

    return rc;
}

static
void KConfigNodeSetDirty ( KConfigNode *self )
{
    KConfig *mgr = self -> mgr;

    if ( mgr == NULL )
        self -> dirty = true;
    else if ( mgr -> initialized )
        self -> dirty = mgr -> dirty = true;
}

/* Write
 *  write a node value or attribute
 *  replaces anything already there
 *
 *  "buffer" [ IN ] and "size" [ IN ] - new value data
 */
LIB_EXPORT rc_t CC KConfigNodeWrite ( KConfigNode *self, const char *buffer, size_t size )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcWriting, rcSelf, rcNull );
    else if ( self -> read_only || self -> internal == eInternalTrue )
        rc = RC ( rcKFG, rcNode, rcWriting, rcSelf, rcReadonly );
    else if ( size == 0 )
    {
        free ( self -> val_buffer ), self -> val_buffer = NULL;
        StringInit ( & self -> value, "", 0, 0 );
        KConfigNodeSetDirty ( self );
        rc = 0;
    }
    else if ( buffer == NULL )
        rc = RC ( rcKFG, rcNode, rcWriting, rcBuffer, rcNull );
    else
    {
        if ( size != self -> value . size )
        {
            void *new_buffer = realloc ( self -> val_buffer, size + 1 );
            if ( new_buffer == NULL )
                return RC ( rcKFG, rcNode, rcWriting, rcMemory, rcExhausted );
            self -> val_buffer = new_buffer;
            self -> value . size = size;
            self -> value . addr = new_buffer;
        }

        assert ( self -> val_buffer != NULL );
        string_copy ( self -> val_buffer, self -> value . size + 1, buffer, size );
        self -> value . len = string_len ( self -> val_buffer, size );

        KConfigNodeSetDirty ( self );

        rc = 0;
    }

    return rc;
}


/* Write Boolean
 *  write a boolean value ( literally the text "true" or "false:
 *  overwrites anything already there
 *
 *  "state" [ IN ] - new value
 */
KFG_EXTERN rc_t CC KConfigNodeWriteBool ( KConfigNode *self, bool state )
{
    rc_t rc;
    size_t written;
    char value[ 8 ];

    if ( state )
        rc = string_printf( value, sizeof value, &written, "true" );
    else
        rc = string_printf( value, sizeof value, &written, "false" );

    if ( rc == 0 )
        rc = KConfigNodeWrite ( self, value, written );
    return rc;
}


/* Append
 *  append data to value
 *
 *  "buffer" [ IN ] and "size" [ IN ] - value data to be appended
 */
LIB_EXPORT rc_t CC KConfigNodeAppend ( KConfigNode *self, const char *buffer, size_t size )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcWriting, rcSelf, rcNull );
    else if ( self -> read_only || self -> internal )
        rc = RC ( rcKFG, rcNode, rcWriting, rcSelf, rcReadonly );
    else if ( size == 0 )
        rc = 0;
    else if ( buffer == NULL )
        rc = RC ( rcKFG, rcNode, rcWriting, rcBuffer, rcNull );
    else
    {
        void *new_buffer = realloc ( self -> val_buffer, self -> value . size + size + 1 );
        if ( new_buffer == NULL )
            return RC ( rcKFG, rcNode, rcWriting, rcMemory, rcExhausted );
        self -> val_buffer = new_buffer;
        string_copy ( & self -> val_buffer [ self -> value . size ], self -> value . size + size + 1, buffer, size );
        self -> value . size += size;
        self -> value . len = string_len ( self -> val_buffer, self -> value . size );

        KConfigNodeSetDirty ( self );

        rc = 0;
    }

    return rc;
}


/* ReadAttr
 *  reads as NUL-terminated string
 *
 *  "name" [ IN ] - NUL terminated attribute name
 *
 *  "buffer" [ OUT ] and "bsize" - return parameter for attribute value
 *
 *  "size" [ OUT ] - return parameter giving size of string
 *  not including NUL byte. the size is set both upon success
 *  and insufficient buffer space error.
 */
LIB_EXPORT rc_t CC KConfigNodeReadAttr ( const KConfigNode *self, const char *name,
                                         char *buffer, size_t bsize, size_t *size )
{
    PLOGMSG (klogFatal, (klogFatal, "$(F) unimplemented", "F=%s", __func__));
    return -1;
}


/* WriteAttr
 *  writes NUL-terminated string
 *
 *  "name" [ IN ] - NUL terminated attribute name
 *
 *  "value" [ IN ] - NUL terminated attribute value
 */
LIB_EXPORT rc_t CC KConfigNodeWriteAttr ( KConfigNode *self,
                                          const char *name, const char *value )
{
    PLOGMSG (klogFatal, (klogFatal, "$(F) unimplemented", "F=%s", __func__));
    return -1;
}


/* Drop
 * VDrop
 *  drop some or all node content
 */
LIB_EXPORT rc_t CC KConfigNodeDropAll ( KConfigNode *self )
{
    if ( self == NULL )
        return RC ( rcKFG, rcNode, rcClearing, rcSelf, rcNull );
    BSTreeWhack ( & self->children, KConfigNodeWhack, self->mgr);
    return 0;
}

LIB_EXPORT rc_t CC KConfigNodeDropAttr ( KConfigNode *self, const char *attr )
{
    PLOGMSG (klogFatal, (klogFatal, "$(F) unimplemented", "F=%s", __func__));
    return -1;
}

LIB_EXPORT rc_t CC KConfigNodeVDropChild ( KConfigNode *self, const char *path, va_list args )
{
    PLOGMSG (klogFatal, (klogFatal, "$(F) unimplemented", "F=%s", __func__));
    return -1;
}

LIB_EXPORT rc_t CC KConfigNodeDropChild ( KConfigNode *self, const char *path, ... )
{
    PLOGMSG (klogFatal, (klogFatal, "$(F) unimplemented", "F=%s", __func__));
    return -1;
}


/* Rename
 *  renames a contained object
 *
 *  "from" [ IN ] - NUL terminated string in UTF-8
 *  giving simple name of existing attr
 *
 *  "to" [ IN ] - NUL terminated string in UTF-8
 *  giving new simple attr name
 */
LIB_EXPORT rc_t CC KConfigNodeRenameAttr ( KConfigNode *self, const char *from, const char *to )
{
    PLOGMSG (klogFatal, (klogFatal, "$(F) unimplemented", "F=%s", __func__));
    return -1;
}

LIB_EXPORT rc_t CC KConfigNodeRenameChild ( KConfigNode *self, const char *from, const char *to )
{
    PLOGMSG (klogFatal, (klogFatal, "$(F) unimplemented", "F=%s", __func__));
    return -1;
}


/*--------------------------------------------------------------------------
 * KConfig
 */

static
rc_t
update_node ( KConfig* self, const char* key, const char* value,
    TInternal internal )
{
    KConfigNode * node;
    rc_t rc = KConfigOpenNodeUpdate ( self, &node, "%s", key);
    if (rc == 0)
    {
/*        pLogMsg(klogInfo, "updating config key $(KEY) with '$(VALUE)'",
                          "KEY=%s,VALUE=%s",
                          key, value);*/
        rc = KConfigNodeWrite (node, value, string_size(value));
        node -> internal = internal;
        if (self->current_file != NULL && self->current_file->is_magic_file) {
            if (node->came_from == NULL || !node->came_from->is_magic_file) {
                node->came_from = self->current_file;
            }
        }
        KConfigNodeRelease ( node );
    }
    return rc;
}

static
rc_t write_nvp(void* pself, const char* name, size_t nameLen, VNamelist* values)
{   /* concatenate all values from the namelist and put the result into config under the given name */
    uint32_t count;
    size_t size=0;
    size_t concatTo=0;
    uint32_t i;

    char* buf;
    KConfig *self = (KConfig *)pself;
    rc_t rc=VNameListCount(values, &count);
    if (rc != 0)
    {
        return rc;
    }
    for (i=0; i < count; ++i)
    {
        const char* val;
        rc=VNameListGet(values, i, &val);
        if (rc != 0)
        {
            return rc;
        }
        size+=string_size(val);
    }

    buf=(char*)malloc(size+1);
    if (buf == 0)
    {
        return RC ( rcKFG, rcMgr, rcLoading, rcMemory, rcExhausted );
    }

    concatTo=0;
    for (i=0; i < count; ++i)
    {
        const char* val;
        rc=VNameListGet(values, i, &val);
        if (rc != 0)
        {
            free(buf);
            return rc;
        }
        string_copy(buf+concatTo, size+1-concatTo, val, string_size(val));
        concatTo+=string_size(val);
    }
    buf[size]=0;

    {   /* create the node */
        String* nameStr;

        /* some old config files may have "dbGaP" in their repository keys misspelled as "dbGap" - fix if seen */
        const char* oldGaPprefix = "/repository/user/protected/dbGap-";
        size_t size = sizeof("/repository/user/protected/dbGap-") - 1;
        bool needsFix = string_cmp(name, string_size(name), oldGaPprefix, size, (uint32_t)size) == 0;

        String tmp;
        StringInit(&tmp, name, nameLen, (uint32_t)nameLen);
        StringCopy((const String**)&nameStr, &tmp);
        if (needsFix)
            ((char*)(nameStr->addr)) [ size - 2 ] = 'P';

        rc = update_node(self, nameStr->addr, buf, false);
        if (needsFix)
        {
            KConfigNode * node;
            rc = KConfigOpenNodeUpdate ( self, &node, "%s", nameStr->addr );
            if (rc == 0)
            {   /* we are likely to be initializing, so have to set the dirty flags directly, not through KConfigNodeSetDirty() */
                self -> dirty = true;
                node -> dirty = true;
                KConfigNodeRelease ( node );
            }
        }
        StringWhack(nameStr);
    }

    free(buf);
    return rc;
}

static
bool look_up_var(void * self, struct KFGToken* pb)
{
    const KConfigNode* node;
    rc_t rc = KConfigOpenNodeRead((KConfig*)self, &node, "%.*s", pb->tokenLength-3, pb->tokenText+2);
    if (rc == 0)
    {
        pb->tokenText   = node->value.addr;
        pb->tokenLength = node->value.len;
        pb->tokenId     = kfgVAR_REF;
    }
    KConfigNodeRelease(node);
    return rc == 0;
}

static
void CC report_error(KFGScanBlock* sb, const char* msg)
{
    pLogMsg(klogErr, "$(file):$(line):$(column): error: token='$(token)', msg='$(msg)'",
                     "file=%s,line=%d,column=%d,token=%.*s,msg=%s",
                     sb->file,
                     sb->lastToken->line_no,
                     sb->lastToken->column_no,
                     sb->lastToken->tokenLength,
                     sb->lastToken->tokenText,
                     msg);
}

#define DISP_RC2(rc, name, msg) (void)((rc == 0) ? 0 : \
    PLOGERR(klogInt, (klogInt, rc, \
        "$(name): $(msg)", "name=%s,msg=%s", name, msg)))

typedef struct
{
    KFile *f; /* NULL for OUTMSG */

    /* total bytes flushed to the file*/
    size_t flushed;

    /* total bytes in buffer */
    size_t buffered;

    rc_t rc;

    /* buffer */
    char buffer [ 32 * 1024 ];

} PrintBuff;

/* Init
 *  initialize your structure
 */
static
void PrintBuffInit ( PrintBuff *pb, KFile *f )
{
    assert ( pb != NULL );
    pb -> f = f; /* NULL for OUTMSG */
    pb -> flushed = 0;
    pb -> buffered = 0;
    pb -> rc = 0;
}

/* Flush
 * Write buffer out to file
 */
static rc_t PrintBuffFlush ( PrintBuff *self )
{
    rc_t rc = 0;

    assert ( self != NULL );
    if ( self -> buffered != 0 )
    {
        size_t num_writ = 0;

        if (self -> f == NULL) {
            OUTMSG(("%.*s", self -> buffered - self -> flushed,
                self -> buffer + self -> flushed));
            num_writ = self -> buffered;
        }
        else {
            rc = KFileWriteAll ( self -> f, self -> flushed,
                self -> buffer, self -> buffered, & num_writ );
        }

        if ( rc == 0 )
        {
            if ( num_writ != self -> buffered )
                rc = RC ( rcKFG, rcBuffer, rcFlushing,
                    rcTransfer, rcIncomplete );
            else
            {
                self -> flushed += num_writ;
                self -> buffered = 0;
            }
        }
    }

    return self -> rc = rc;
}

/* Print
 *  printf style writing to the buffer
 */
static
rc_t PrintBuffPrint ( PrintBuff *self, const char *fmt, ... )
{
    rc_t rc;
    size_t num_writ;
    va_list args1, args2;

    assert ( self != NULL );
    assert ( fmt != NULL );
    assert ( fmt [ 0 ] != 0 );

    va_start ( args1, fmt );
    va_copy ( args2, args1 );

    rc = string_vprintf ( & self -> buffer [ self -> buffered ],
            sizeof self -> buffer - self -> buffered, & num_writ, fmt, args1 );
    if ( rc == 0 )
        self -> buffered += num_writ;
    else if ( GetRCObject ( rc ) == (enum RCObject)rcBuffer
        && GetRCState ( rc ) == rcInsufficient )
    {
        rc = PrintBuffFlush ( self );
        if ( rc == 0 )
        {
            rc = string_vprintf ( & self -> buffer [ self -> buffered ],
             sizeof self -> buffer - self -> buffered, & num_writ, fmt, args2 );
            if ( rc == 0 )
                self -> buffered += num_writ;
        }
    }

    va_end ( args2 );
    va_end ( args1 );

    return self -> rc = rc;
}

static rc_t printIndent(int indent, PrintBuff *pb) {
    rc_t rc = 0;

    int i = 0;
    for (i = 0; i < indent * 2; ++i) {
        rc_t rc2 = PrintBuffPrint(pb, " ");
        if (rc == 0 && rc2 != 0) {
            rc = rc2;
        }
    }

    return rc;
}

static rc_t KConfigNodeReadData(const KConfigNode* self,
    char* buf, size_t blen, size_t* num_read)
{
    rc_t rc = 0;
    size_t remaining = 0;

    assert(buf && blen && num_read);

    rc = KConfigNodeRead(self, 0, buf, blen, num_read, &remaining);

    assert(remaining == 0); /* TODO overflow check */
    assert(*num_read <= blen);

    return rc;
}

char ToHex(uint32_t i)
{
    if (i <= 9)
        return '0' + i;
    return 'A' + (i - 10);
}

static
rc_t CC PrintBuffPrintQuoted ( PrintBuff *self, const String* data )
{
    rc_t rc = PrintBuffPrint(self, "\"");
    const char* str = (const char*)(data->addr);
    uint32_t i;
    for ( i = 0; i < StringLength(data); ++i )
    {
        if (rc != 0)
            break;
        if (str[i] < ' ')
        {
            rc = PrintBuffPrint(self, "\\x%c%c",
                ToHex(str[i]/16), ToHex(str[i]%16) );
        }
        else
        {
            switch (str[i])
            {
            case '"':
                rc = PrintBuffPrint(self, "\\\"");
                break;
            default:
                rc = PrintBuffPrint(self, "%c", str[i]);
            }
        }
    }
    if (rc == 0)
        rc = PrintBuffPrint(self, "\"");
    return rc;
}

static rc_t _printNodeData(const char *name, const char *data, uint32_t dlen,
    bool native, const char *fullpath, bool hide, PrintBuff *pb)
{
    bool secret = false;
    {
        const char d1[] = "download-ticket";
        size_t l1 = sizeof d1 - 1;

        const char d2[] = "aws_access_key_id";
        size_t l2 = sizeof d2 - 1;

        const char d3[] = "aws_secret_access_key";
        size_t l3 = sizeof d3 - 1;

        if ((string_cmp(name,
                string_measure(name, NULL), d1, l1, (uint32_t)l1) == 0) ||
            (string_cmp(name,
                string_measure(name, NULL), d2, l2, (uint32_t)l2) == 0) ||
            (string_cmp(name,
                string_measure(name, NULL), d3, l3, (uint32_t)l3) == 0))
        {
            secret = true;
        }
    }
    if (hide && !native && secret) {
        const char *ellipsis = "";
        const char replace[] =
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
        if (dlen > 70) {
            dlen = 70;
            ellipsis = "...";
        }
        return PrintBuffPrint(pb, "%.*s%s", dlen, replace, ellipsis);
    }
    else if (!native) {
        String s;
        StringInit(&s, data, dlen, dlen);
        return PrintBuffPrintQuoted(pb, &s);
    }
    else {
        rc_t rc = PrintBuffPrint(pb, "%s = ", fullpath);
        if (rc == 0) {
            String s;
            StringInit(&s, data, dlen, dlen);
            rc = PrintBuffPrintQuoted(pb, &s);
        }
        if (rc == 0) {
            rc = PrintBuffPrint(pb, "\n");
        }
        return rc;
    }
}


#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 && !rc) { rc = rc2; } obj = NULL; } while (false)

static rc_t KConfigNodePrintWithIncluded (const KConfigNode *self, int indent,
    const char* root, bool debug, bool native, const char* aFullpath,
    PrintBuff *pb, uint32_t skipCount, va_list args,
    const KConfig * withIncluded )
{
    rc_t rc = 0;
    KNamelist* names = NULL;
    uint32_t count = 0;
    uint32_t i = 0;
    char data[4097] = "";
    size_t num_data = 0;

    assert(self);

    if (!native) {
        rc = printIndent(indent, pb);
        if (rc == 0) {
            bool found = false;
            uint32_t i = 0;
            va_list args_copy;
            if (skipCount > 0)
                va_copy(args_copy, args);
            for (i = 0; i < skipCount; ++i) {
                const char *skip = va_arg(args_copy, const char*);
                if (string_cmp(skip, string_measure(skip, NULL), root,
                        string_measure(root, NULL), string_measure(root, NULL))
                    == 0)
                {
                    rc = PrintBuffPrint
                        (pb, "<%s><!-- skipped --></%s>\n", root, root);
                    found = true;
                    break;
                }
            }
            if (skipCount > 0)
                va_end(args_copy);
            if (found)
                return rc;
            rc = PrintBuffPrint(pb, "<%s>", root);
            if ( withIncluded ) {
//              bool hasAny = false;
                uint32_t count = 0;
                KNamelist * names = NULL;
                rc_t rc = KConfigListIncluded ( withIncluded, & names );
                if ( rc == 0 )
                    rc = KNamelistCount ( names, & count );
                if ( rc == 0 ) {
                    uint32_t i = 0;
                    rc = printIndent(indent, pb);
                    PrintBuffPrint ( pb, "\n  <ConfigurationFiles>\n" );
                    for ( i = 0; i < count && rc == 0; ++i ) {
                        const char * name = NULL;
                        if ( rc == 0 )
                            rc = KNamelistGet(names, i, &name);
                        if (rc == 0) {
                            PrintBuffPrint ( pb, "%s\n", name );
//                          hasAny = true;
                        }
                    }
                    rc = printIndent(indent, pb);
                    PrintBuffPrint ( pb, "  </ConfigurationFiles>" );
                }
                RELEASE ( KNamelist, names );
            }
        }
    }

    if (rc == 0) {
        rc_t rc = KConfigNodeReadData(self, data, sizeof data, &num_data);
        DISP_RC2(rc, "KConfigNodeReadData()", root);
        if (rc == 0 && num_data > 0) {
            _printNodeData(root, data, num_data,
                native, aFullpath, !native, pb);
        }
        if (debug && self->came_from) {
            OUTMSG(("<came_from is_magic_file=\"%s\"/>",
                self->came_from->is_magic_file ? "true" : "false"));
        }
    }

    if (rc == 0) {
        rc = KConfigNodeListChild(self, &names);
        DISP_RC2(rc, "KConfigNodeListChild()", root);
    }
    if (rc == 0) {
        rc = KNamelistCount(names, &count);
        DISP_RC2(rc, "KNamelistCount()", root);
    }

    if (rc == 0) {
        if (count > 0 && !native)
            rc = PrintBuffPrint(pb, "\n");
        for (i = 0; i < count; ++i) {
            char *fullpath = NULL;
            const char* name = NULL;
            const KConfigNode* node = NULL;
            if (rc == 0) {
                rc = KNamelistGet(names, i, &name);
                DISP_RC2(rc, "KNamelistGet()", root);
            }
            if (rc == 0) {
                rc = KConfigNodeOpenNodeRead(self, &node, "%s", name);
                DISP_RC2(rc, "KConfigNodeOpenNodeRead()", name);
            }
            if (rc == 0) {
                size_t bsize = strlen(aFullpath) + 1 + strlen(name) + 1;
                fullpath = malloc(bsize + 1);
                if (fullpath == NULL) {
                    rc = RC(rcKFG, rcStorage, rcAllocating,
                        rcMemory, rcExhausted);
                }
                else {
                    string_printf(fullpath, bsize, NULL,
                        "%s/%s", aFullpath, name);
                }
            }
            if (rc == 0) {
                if (! isdigit(name[0])) {
                    KConfigNodePrintWithIncluded(node, indent + 1, name,
                        debug, native, fullpath, pb, skipCount, args, NULL );
                }
                else {
                    /* XML node names cannot start with a number */
                    size_t dsize = strlen(name) + 2;
                    char *dname =  malloc(dsize);
                    if (dname == NULL) {
                        rc = RC(rcKFG, rcStorage, rcAllocating,
                            rcMemory, rcExhausted);
                    }
                    else {
                        string_printf(dname, dsize, NULL, "_%s", name);
                        KConfigNodePrintWithIncluded ( node, indent + 1, dname,
                            debug, native, fullpath, pb, skipCount, args,
                            NULL );
                        free(dname);
                    }
                }
            }
            KConfigNodeRelease(node);
            free(fullpath);
        }
    }

    if (count > 0 && !native) {
        printIndent(indent, pb);
    }

    if (rc == 0 && !native) {
        rc = PrintBuffPrint(pb, "</%s>\n", root);
    }

    KNamelistRelease(names);

    return rc;
}

static rc_t KConfigNodePrint(const KConfigNode *self, int indent,
    const char* root, bool debug, bool native, const char* aFullpath,
    PrintBuff *pb, uint32_t skipCount, va_list args, const KConfig* cfg )
{
    return KConfigNodePrintWithIncluded ( self, indent, root, debug, native,
            aFullpath, pb, skipCount, args, cfg );
}

static rc_t CC KConfigPrintImpl(const KConfig* self,
    int indent, const char *root, bool debug, bool native,
    PrintBuff *pb, uint32_t skipCount, va_list args)
{
    const KConfig * withIncluded = NULL;
    rc_t rc = 0;

    if (root == NULL) {
        root = "Config";
        withIncluded = self;
    }

    if (self == NULL) {
        OUTMSG(("<%s>", root));
        OUTMSG(("KConfigPrint(const KConfig* self = NULL)\n"));
        OUTMSG(("</%s>\n", root));
    }
    else {
        const KConfigNode* node = NULL;
        if (rc == 0) {
            rc = KConfigOpenNodeRead(self, &node, "/");
            DISP_RC2(rc, "KConfigOpenNodeRead()", "/");
        }
        if (rc == 0)
            KConfigNodePrint ( node, indent, root, debug, native, "", pb,
                               skipCount, args, withIncluded );
        KConfigNodeRelease(node);
    }

    return rc;
}

LIB_EXPORT rc_t CC KConfigPrintDebug(const KConfig* self, const char *path) {
    rc_t rc = 0;

    PrintBuff pb;

    if (rc == 0) {
        PrintBuffInit(&pb, NULL);
    }

    if (rc == 0) {
        rc = KConfigPrintImpl(self, 0, path, true, false, &pb, 0, NULL);
    }

    if (rc == 0) {
        rc = PrintBuffFlush(&pb);
    }

    return rc;
}

/*
 * Set up the parameter block and start parsing lines
 */
static
rc_t parse_file ( KConfig * self, const char* path, const char * src )
{
    KFGParseBlock pb;
    KFGScanBlock sb;

    pb.write_nvp    = write_nvp;

    sb.self         = self;
    sb.file         = path;
    sb.look_up_var  = look_up_var;
    sb.report_error = report_error;

    if  ( ! KFGScan_yylex_init(&sb, src) )
    {   /* out of memory is the only reason we can get false here */
        return RC ( rcKFG, rcMgr, rcParsing, rcMemory, rcExhausted );
    }

    KFG_parse(&pb, &sb); /* may have reported parse errors into log, but we should have been able to extract enough data to proceed regardless */
    KFGScan_yylex_destroy(&sb);

    return 0;
}

LIB_EXPORT rc_t CC
KConfigParse ( KConfig * self, const char* path, const char * src )
{
    if ( self == NULL )
        return RC ( rcKFG, rcMgr, rcLoading, rcSelf, rcNull );
    else if ( src == NULL )
        return RC ( rcKFG, rcMgr, rcLoading, rcParam, rcNull );

    if ( path == NULL )
    {
        path = "UNSPECIFIED";
    }
    return parse_file ( self, path, src );
}

/* LoadFile
 * loads a configuration file
 */
LIB_EXPORT rc_t CC KConfigLoadFile ( KConfig * self, const char * path, const KFile * file )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcKFG, rcMgr, rcLoading, rcSelf, rcNull );
    else if ( file == NULL )
        rc = RC ( rcKFG, rcMgr, rcLoading, rcFile, rcNull );
    else
    {
        const KMMap * mm;
        bool entry_initialized = self -> initialized;
        self -> initialized = false;

        /* populate file-specific predefined nodes */
#define UPDATE_NODES(dir, file)                             \
        rc = update_node(self, "kfg/dir", dir, eInternalTrueUpdatable );     \
        if (rc == 0)                                        \
            rc = update_node(self, "kfg/name", file, eInternalTrueUpdatable )

        if ( path == NULL || path [ 0 ] == 0)
        {
            path = "UNSPECIFIED";
            UPDATE_NODES ( "", "" );
        }
        else
        {
            KDirectory* dir;
            rc = KDirectoryNativeDir(&dir);
            if (rc == 0 )
            {
                char buff [ 4096 ];
                rc = KDirectoryResolvePath ( dir, true, buff, sizeof buff,
                    "%.*s", string_size(path), path );
                if ( rc == 0 )
                {
                    char* name = strrchr (buff, '/');
                    DBGMSG ( DBG_KFG, DBG_FLAG ( DBG_KFG ),
                        ( "KFG: loading file '%s'\n", buff ) );
                    if (name == NULL)
                    {   /* no dir name */
                        UPDATE_NODES("", buff);
                    }
                    else
                    {
                        *name=0; /* nul-terminate dir name; file name follows the 0 */
                        UPDATE_NODES(buff, name+1);
                    }
                }
                KDirectoryRelease ( dir );
            }
            else
            {
                update_node(self, "kfg/dir", "", eInternalTrueUpdatable);
                update_node(self, "kfg/name", "", eInternalTrueUpdatable);
            }
        }
#undef UPDATE_NODES


        rc = KMMapMakeRead ( & mm, file );
        if ( rc == 0 )
        {
            size_t size;
            const void * ptr;
            rc = KMMapAddrRead ( mm, & ptr );
            if ( rc == 0 )
                rc = KMMapSize ( mm, & size );
            if ( rc == 0 )
            {
                /* make a 0-terminated copy for parsing */
                char* buf=malloc(size+1);
                if (buf == 0)
                {
                    rc = RC ( rcKFG, rcMgr, rcLoading, rcMemory, rcExhausted );
                }
                else
                {
                    string_copy(buf, size+1, ptr, size);
                    buf[size]=0;

                    /* Parse the path to populate: */
                 /* update_node(self, "kfg/dir", dir, eInternalTrueUpdatable);*/
               /* update_node(self, "kfg/name", name, eInternalTrueUpdatable);*/

                    /* parse config file */
                    rc = parse_file ( self, path, buf );
                    free(buf);
                }
            }

            KMMapRelease ( mm );
        }
        self -> initialized = entry_initialized;
    }

    return rc;
}

static
bool KConfigNodePrintPath ( KConfigNode *self, PrintBuff *pb )
{
    if ( self -> dad == NULL )
        PrintBuffPrint ( pb, "%S", & self -> name );
    else
    {
        if ( KConfigNodePrintPath ( self -> dad, pb ) )
            return true;

        PrintBuffPrint ( pb, "/%S", & self -> name );
    }
    return pb -> rc != 0;
}

/*vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv KfgSettings vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
typedef struct {
    const char *envrNcbiHome;
    char        envrNcbiSettings[ PATH_MAX ];
    char        dfltNcbiHome    [ PATH_MAX ];
    char        dfltNcbiSettings[ PATH_MAX ];
} KfgSettings;
static
void _KConfigIniKfgSettings ( const KConfig * self, KfgSettings * ks ) {
    size_t bytes = 0;

    assert(ks);
    memset(ks, 0, sizeof *ks);

    /* load environment */
    ks -> envrNcbiHome = getenv ( "NCBI_HOME" );
    {
        const char *value = getenv ( "NCBI_SETTINGS" );
        if ( value != NULL ) {
            string_copy_measure (
                ks -> envrNcbiSettings, sizeof ks -> envrNcbiSettings, value );
        }
        else if ( ks -> envrNcbiHome != NULL ) {
            string_printf ( ks->envrNcbiSettings,
                sizeof ks -> envrNcbiSettings,
                & bytes, "%s/%s", ks -> envrNcbiHome, MAGIC_LEAF_NAME);
        }
    }

    /* default values */
    {
        char home [ PATH_MAX ] = "";
        size_t num_read = 0;
        size_t remaining = 0;
        rc_t rc = KConfigRead
            ( self, "HOME", 0, home, sizeof home, & num_read, & remaining);
        if ( rc != 0 || remaining != 0 ) {
            return;
        }
        string_printf ( ks -> dfltNcbiHome  , sizeof ks -> dfltNcbiHome,
            & bytes, "%.*s/.ncbi", num_read, home );
        string_printf ( ks->dfltNcbiSettings, sizeof ks -> dfltNcbiSettings,
            & bytes, "%.*s/.ncbi/%s", num_read, home, MAGIC_LEAF_NAME );
    }
}

static
bool MayICommitTo(const KConfig *self, const char *path, size_t size)
{
    if ( ! s_disable_user_settings ) {
        return true;
    }
    else {
        size_t bytes = 0;
        char home             [ PATH_MAX ] = "";
        char dfltNcbiSettings [ PATH_MAX ] = "";
        size_t num_read = 0;
        size_t remaining = 0;
        rc_t rc = KConfigRead
            ( self, "HOME", 0, home, sizeof home, & num_read, & remaining);
        if ( rc != 0 || remaining != 0 ) {
            return false;
        }
        string_printf ( dfltNcbiSettings, sizeof dfltNcbiSettings,
            & bytes, "%.*s/.ncbi/%s", num_read, home, MAGIC_LEAF_NAME );
        return string_cmp
            (dfltNcbiSettings, bytes, path, size, sizeof dfltNcbiSettings) != 0;
    }
}

static
rc_t _KConfigGetNcbiHome ( const KConfig * self, const KfgSettings * ks,
    char * buf, size_t size)
{
    size_t num_read = 0;
    size_t remaining = 0;

    if (KConfigRead ( self, "NCBI_HOME", 0, buf, size, & num_read, & remaining)
        != 0)
    {
        assert ( ks );
        if ( ks -> envrNcbiHome != 0 ) {
            string_copy_measure ( buf, size, ks -> envrNcbiHome );
        }
        else {
            string_copy_measure ( buf, size, ks -> dfltNcbiHome );
        }
    }
    else if ( remaining > 0 || num_read >= size ) {
        return RC ( rcKFG, rcNode, rcReading, rcBuffer, rcInsufficient );
    }
    else {
        buf [ num_read ] = '\0';
    }

    return 0;
}

static
rc_t _KConfigGetNcbiSettings ( const KConfig * self, const KfgSettings * ks,
    char * buf, size_t size, const char * root)
{
    size_t num_read = 0;
    size_t remaining = 0;

    rc_t rc = 0;

    assert(ks && buf && size);

    buf[0] = '\0';

    rc = KConfigRead
        ( self, "NCBI_SETTINGS", 0, buf, size, & num_read, & remaining );
    if ( rc != 0 ) {
        if ( ks -> envrNcbiSettings [ 0 ] != '\0' ) {
            string_copy_measure ( buf, size, ks -> envrNcbiSettings );
            rc = 0;
        }
        else if ( root != NULL && root[0] != '\0' ) {
            rc = string_printf
                ( buf, size, & num_read, "%s/%s", root, MAGIC_LEAF_NAME );
        }
        if ( rc != 0 ) {
            string_copy_measure ( buf, size, ks -> dfltNcbiSettings );
            rc = 0;
        }
    }
    else if ( remaining > 0 || num_read >= size ) {
        return RC ( rcKFG, rcNode, rcReading, rcBuffer, rcInsufficient );
    }
    else {
        buf [ num_read ] = '\0';
    }

    return rc;
}

static
void _KConfigSetNcbiHome ( KConfig * self, const KfgSettings * ks,
    const char * ncbi_home)
{
    size_t num_read = 0;
    size_t remaining = 0;
    char buf [ PATH_MAX ] = "";

    if ( KConfigRead ( self, "NCBI_HOME", 0,
            buf, sizeof buf, & num_read, & remaining) == 0 )
    {
        DBGMSG( DBG_KFG, DBG_FLAG ( DBG_KFG ),
            ( "KFG: NCBI_HOME='%.*s'\n", num_read, buf ) );
    }
    else {
        if ( ncbi_home == NULL ) {
            if ( ks -> envrNcbiHome != NULL ) {
                ncbi_home = ks -> envrNcbiHome;
            }
            else {
                ncbi_home = ks -> dfltNcbiHome;
            }
        }

        update_node ( self, "NCBI_HOME", ncbi_home, false );

        DBGMSG ( DBG_KFG, DBG_FLAG ( DBG_KFG ),
            ( "KFG: NCBI_HOME     was set to '%s'\n", ncbi_home ) );
    }
}

static
void _KConfigSetNcbiSettings ( KConfig * self, const KfgSettings * ks,
    const char * ncbi_settings )
{
    size_t num_read = 0;
    size_t remaining = 0;
    char buf [ PATH_MAX ] = "";

    if ( KConfigRead ( self, "NCBI_SETTINGS", 0,
            buf, sizeof buf, & num_read, & remaining) == 0 )
    {
        DBGMSG( DBG_KFG, DBG_FLAG ( DBG_KFG ),
            ( "KFG: NCBI_SETTINGS='%.*s'\n", num_read, buf ) );
    }
    else {
        if ( ncbi_settings == NULL ) {
            if ( ks -> envrNcbiSettings [ 0 ] != '\0' ) {
                ncbi_settings = ks -> envrNcbiSettings;
            }
            else {
                ncbi_settings = ks -> dfltNcbiSettings;
            }
        }

        update_node ( self, "NCBI_SETTINGS", ncbi_settings, false );
        DBGMSG ( DBG_KFG, DBG_FLAG ( DBG_KFG ),
            ( "KFG: NCBI_SETTINGS was set to '%s'\n", ncbi_settings ) );
    }
}
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ KfgSettings ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

static
bool CC WriteDirtyNode ( BSTNode *n, void *data )
{
    KConfigNode *self = ( KConfigNode * ) n;
    PrintBuff *pb = data;

    if ( self -> dirty
        || ( self -> came_from != NULL && self -> came_from -> is_magic_file
             && ! self -> internal ) )
    {
        if ( KConfigNodePrintPath ( self, pb ) )
            return true;

        if ( PrintBuffPrint ( pb, " = " ) != 0 )
            return true;

        if ( PrintBuffPrintQuoted ( pb, &self->value ) != 0 )
            return true;

        if ( PrintBuffPrint ( pb, "\n" ) != 0 )
            return true;
    }

    return BSTreeDoUntil ( & self -> children, false, WriteDirtyNode, pb );
}

static
void CC UnsetDirtyNode ( BSTNode *n, void *data )
{
    KConfigNode *self = ( KConfigNode * ) n;

    if ( self -> dirty )
        self -> dirty = false;

    BSTreeForEach ( & self -> children, false, UnsetDirtyNode, data );
}

static
rc_t path_to_magic_file ( const KConfig *self, char *path, size_t buffer_size, size_t *path_size )
{
    const KConfigNode *node;
    rc_t rc = KConfigOpenNodeRead ( self, & node, "NCBI_SETTINGS" );

    if ( rc == 0 )
    {
        size_t num_read, remaining;
        rc = KConfigNodeRead ( node, 0, path, buffer_size - 1, & num_read, & remaining );

        if ( rc == 0 && remaining != 0 )
           rc = RC ( rcKFG, rcNode, rcReading, rcBuffer, rcInsufficient );

        path[num_read] = '\0';

        KConfigNodeRelease ( node );
    }
    return rc;
}

/* Commit
 *  commits changes to user's private configuration file
 */
LIB_EXPORT rc_t CC KConfigCommit ( KConfig *self )
{
    rc_t rc;
    size_t path_size = 0;
    char magic_file_path [ 4096 ];

    if ( self == NULL )
        return RC ( rcKFG, rcData, rcCommitting, rcSelf, rcNull );

    /* Must only commit dirty nodes*/
    if ( ! self -> dirty )
        return 0;

    rc = path_to_magic_file ( self, magic_file_path, sizeof magic_file_path, & path_size );
    if ( rc == 0 )
    {
        char tmp_file_path [ 4096 ];

        if ( ! MayICommitTo ( self, magic_file_path, path_size ) ) {
            DBGMSG ( DBG_KFG, DBG_FLAG ( DBG_KFG ), (
                "KFG: User Settings Are Disables: Skipped KConfigCommit\n" ) );
            return RC ( rcKFG, rcData, rcCommitting, rcSelf, rcReadonly );
        }

        DBGMSG( DBG_KFG, DBG_FLAG(DBG_KFG),
            ( "KFG: Committing to '%s'\n", magic_file_path ) );
        rc = string_printf ( tmp_file_path, sizeof tmp_file_path, NULL, "%s.tmp", magic_file_path );
        if ( rc == 0 )
        {
            KDirectory *dir;

            /* get current directory */
            rc = KDirectoryNativeDir ( & dir );
            if ( rc == 0 )
            {
                KFile *tmp;

                /* create temp magic file */
                rc = KDirectoryCreateFile ( dir, & tmp, false, 0600, kcmInit | kcmParents, "%s", tmp_file_path );

                if ( rc == 0 )
                {
                    PrintBuff pb;
                    PrintBuffInit ( & pb, tmp );

                    /* issue warning */
                    rc = PrintBuffPrint ( & pb, "## auto-generated configuration file - DO NOT EDIT ##\n\n" );
                    if ( rc == 0 )
                    {
                        /* flush nodes to file */
                        if ( BSTreeDoUntil ( & self -> tree, false, WriteDirtyNode, & pb ) )
                            /* failure */
                            rc = pb . rc;
                        else
                            rc = PrintBuffFlush ( & pb );
                    }

                    KFileRelease ( tmp );

                    if ( rc == 0 )
                    {
                        /* Rename file */
                        rc = KDirectoryRename ( dir, true, tmp_file_path, magic_file_path  );
                        if ( rc == 0 )
                        {
                            /* clear dirty flags */
                            BSTreeForEach (  & self -> tree, false, UnsetDirtyNode, & pb);

                            self -> dirty = false;
                        }
                    }

                    if ( rc != 0 )
                        KDirectoryRemove ( dir, true, "%s", tmp_file_path );
                }

                /* release dir */
                KDirectoryRelease ( dir );
            }
        }
    }
    return rc;
}

static
rc_t record_magic_path ( KConfig *self, const KDirectory *dir, const char *path, size_t sz )
{
    char buff [ 4096 ];
    rc_t rc = KDirectoryResolvePath ( dir, true, buff, sizeof buff, "%.*s", ( int ) sz, path );
    if ( rc == 0 )
    {
        char *magic_file_path;
        sz = string_size ( buff );
        magic_file_path = malloc ( sz + 1 );
        if ( magic_file_path == NULL )
            rc = RC ( rcKFG, rcMgr, rcLoading, rcMemory, rcExhausted );
        else
        {
            free ( (void*) self -> magic_file_path );
            self -> magic_file_path = magic_file_path;
            self -> magic_file_path_size = sz;
            memmove ( magic_file_path, buff, sz + 1 );
        }
    }

    return rc;
}


static
rc_t make_include_path ( KConfigIncluded **p, const KDirectory *dir, const char *path, size_t sz, bool is_magic )
{
    char buff [ 4096 ];
    rc_t rc = KDirectoryResolvePath ( dir, true, buff, sizeof buff, "%.*s", ( int ) sz, path );
    if ( rc == 0 )
    {
        KConfigIncluded *include;
        sz = string_size ( buff );
        include = malloc ( sizeof * include + sz );
        if ( include == NULL )
            rc = RC ( rcKFG, rcMgr, rcLoading, rcMemory, rcExhausted );
        else
        {
            include -> is_magic_file = is_magic;
            string_copy ( include -> path, sz + sizeof include -> path, buff, sz );
            * p = include;
            return 0;
        }
    }
    * p = NULL;
    return rc;
}


static
bool load_from_file_path ( KConfig *self, const KDirectory *dir, const char *path, size_t sz, bool is_magic )
{
    rc_t rc;
    const KFile *cfg_file;

    /* record magic file path, regardless of whether it exists as a file */
    if ( is_magic )
        record_magic_path ( self, dir, path, sz );

    DBGMSG( DBG_KFG, DBG_FLAG(DBG_KFG), ( "KFG: try to load from file '%.*s'\n", (int)sz, path ) );
    rc = KDirectoryOpenFileRead ( dir, & cfg_file, "%.*s", ( int ) sz, path );
    if ( rc == 0 )
    {
        KConfigIncluded *include;
        rc = make_include_path ( & include, dir, path, sz, is_magic );
        if ( rc == 0 )
        {
            BSTNode *existing;
            if ( BSTreeInsertUnique ( & self -> included, & include -> n, & existing, KConfigIncludedSort ) != 0 )
                free ( include );
            else
            {
                self -> current_file = include;
                rc = KConfigLoadFile ( self, include -> path, cfg_file );
                self -> current_file = NULL;
                if ( rc != 0 )
                {
                    BSTreeUnlink ( & self -> included, & include -> n );
                    free ( include );
                }
            }
        }

        KFileRelease ( cfg_file );
    }
    return ( rc == 0 ) ? true : false;
}

typedef struct scan_config_path_data scan_config_path_data;
struct scan_config_path_data
{
    KConfig *self;
    bool loaded;
};

static
rc_t CC scan_config_path ( const KDirectory *dir, uint32_t type, const char *name, void *data )
{
    scan_config_path_data * pb = data;
    switch ( type )
    {
    case kptFile:
    case kptFile | kptAlias:
    {
        size_t sz = string_size ( name );
        if ( sz >= 5 && strcase_cmp ( & name [ sz - 4 ], 4, ".kfg", 4, 4 ) == 0 )
            pb -> loaded |= load_from_file_path ( pb -> self, dir, name, sz, false );

        break;
    }}

    return 0;
}

static
bool scan_config_dir ( KConfig *self, const KDirectory *dir )
{
    scan_config_path_data pb;

    pb . self = self;
    pb . loaded = false;

    KDirectoryVVisit ( dir, false, scan_config_path, & pb, ".", NULL );

    return pb . loaded;
}

static
bool load_from_dir_path ( KConfig *self, const KDirectory *dir, const char *path, size_t sz )
{
    bool loaded = false;
    const KDirectory *cfg_dir;
    rc_t rc = KDirectoryOpenDirRead ( dir, & cfg_dir, false, "%.*s", ( uint32_t ) sz, path );
    if ( rc == 0 )
    {
        DBGMSG( DBG_KFG, DBG_FLAG(DBG_KFG), ( "KFG: try to load from directory '%.*s'\n", (int)sz, path ) );
        loaded = scan_config_dir ( self, cfg_dir );
        KDirectoryRelease ( cfg_dir );
    }
    return loaded;
}

static
bool load_from_path ( KConfig *self, const KDirectory * dir, const char *path, size_t sz )
{
    bool loaded = false;
    const char *naughty = string_chr ( path, sz, '%' );
    if ( naughty == NULL && sz != 0 )
    {
        DBGMSG( DBG_KFG, DBG_FLAG(DBG_KFG), ( "KFG: try to load from path '%.*s'\n", (int)sz, path ) );
        switch ( KDirectoryPathType ( dir, "%.*s", ( int ) sz, path ) & ~ kptAlias )
        {
        case kptFile:
            loaded = load_from_file_path ( self, dir, path, sz, false );
            break;
        case kptDir:
            loaded = load_from_dir_path ( self, dir, path, sz );
            break;
        }
    }
    return loaded;
}

static
bool load_from_path_list ( KConfig *self, const KDirectory *dir, const char *path )
{
    bool loaded = false;
    const char *end = path + string_size ( path );
    while ( path < end )
    {
        const char *sep = string_chr ( path, end - path, ':' );
        if ( sep == NULL )
            sep = end;
        if ( load_from_path ( self, dir, path, sep - path ) )
            loaded = true;
        path = sep + 1;
    }
    return loaded;
}

static
bool load_from_env_variable ( KConfig *self, const KDirectory *dir )
{
    const char * env_list [] =
    {
        "KLIB_CONFIG",
        "VDB_CONFIG",
        "VDBCONFIG"
    };

    int i;
    bool loaded = false;
    for ( i = 0; ! loaded && i < sizeof env_list / sizeof env_list [ 0 ]; ++ i )
    {
        const char *eval = getenv ( env_list [ i ] );
        DBGMSG( DBG_KFG, DBG_FLAG(DBG_KFG), ( "KFG: try to load from env. var '%s'\n", env_list[ i ] ) );
        if ( eval != NULL && eval [ 0 ] != 0 )
        {
            /* rc_t rc = 0; */
            DBGMSG( DBG_KFG, DBG_FLAG(DBG_KFG), ( "KFG: try to load from env. var '%s'\n", eval ) );
            /* rc = */ KConfigAppendToLoadPath(self, eval);
            loaded = load_from_path_list ( self, dir, eval );
            if ( loaded )
                DBGMSG( DBG_KFG, DBG_FLAG(DBG_KFG), ( "KFG: found from env. var '%s'\n", eval ) );
        }
    }

    return loaded;
}

static
rc_t load_env_variable_rc ( KConfig *self, const KDirectory *dir, bool *loaded )
{
    const char * env_list [] =
    {
        "NCBI_VDB_CONFIG"
    };

    int i;
    rc_t rc;

    * loaded = false;

    for ( i = 0; ! * loaded && i < sizeof env_list / sizeof env_list [ 0 ]; ++ i )
    {
        const char *eval = getenv ( env_list [ i ] );
        DBGMSG( DBG_KFG, DBG_FLAG(DBG_KFG), ( "KFG: try to load from env. var '%s'\n", env_list[ i ] ) );
        if ( eval == NULL || eval [ 0 ] == 0 )
            return 0;

        DBGMSG( DBG_KFG, DBG_FLAG(DBG_KFG), ( "KFG: try to load from env. var '%s'\n", eval ) );
        rc = KConfigAppendToLoadPath(self, eval);
        if ( rc != 0 )
            return rc;

        * loaded = load_from_path_list ( self, dir, eval );
        if ( * loaded )
            DBGMSG( DBG_KFG, DBG_FLAG(DBG_KFG), ( "KFG: found from env. var '%s'\n", eval ) );
        else
            return RC ( rcKFG, rcFile, rcListing, rcEnvironment, rcEmpty );

    }

    return 0;
}

static
bool load_from_std_location ( KConfig *self, const KDirectory *dir )
{
    const char * std_locs [] =
    {
#if ! WINDOWS
        "/etc/ncbi",
#else
        "/c/ncbi",
#endif
    };

    int i;
    bool loaded = false;

    const char* NCBI_VDB_NO_ETC_NCBI_KFG = getenv("NCBI_VDB_NO_ETC_NCBI_KFG");
    if (NCBI_VDB_NO_ETC_NCBI_KFG != NULL &&
        NCBI_VDB_NO_ETC_NCBI_KFG[0] != '\0')
    {
        DBGMSG(DBG_KFG, DBG_FLAG(DBG_KFG),
            ("KFG: load from std. location /etc/ncbi is disabled. "
                "NCBI_VDB_NO_ETC_NCBI_KFG='%s'\n", NCBI_VDB_NO_ETC_NCBI_KFG));
        return loaded;
    }

    for ( i = 0; ! loaded && i < sizeof std_locs / sizeof std_locs [ 0 ]; ++ i )
    {
        DBGMSG( DBG_KFG, DBG_FLAG(DBG_KFG), ( "KFG: try to load from std. location '%s'\n", std_locs[ i ] ) );
        /* rc = */ KConfigAppendToLoadPath(self, std_locs [ i ]);
        loaded = load_from_path ( self, dir,
            std_locs [ i ], string_size ( std_locs [ i ] ));
    }
    if ( loaded )
        DBGMSG( DBG_KFG, DBG_FLAG(DBG_KFG), ( "KFG: found from std. location\n" ) );
    return loaded;
}

static
bool load_from_fs_location ( KConfig *self, const char *confdir )
{
    bool loaded = false;
    KDyld *dyld;
    rc_t rc = KDyldMake ( & dyld );
    if ( rc == 0 )
    {
        const KDirectory *dir;
        rc = find_home_directory ( dyld, & dir, false );
        if ( rc == 0 )
        {
            char resolved[PATH_MAX + 1];
            assert ( confdir );
            DBGMSG( DBG_KFG, DBG_FLAG(DBG_KFG),
                ( "KFG: try to load from dyn. loader %s\n", confdir ) );

/* N.B. Duplication of ResolvePath here and in load_from_dir_path ? */
            if (KDirectoryResolvePath
                    (dir, true, resolved, sizeof resolved, confdir) == 0)
            {
                rc = KConfigAppendToLoadPath(self, resolved);
            }
            if ((loaded = load_from_dir_path(self, dir, confdir,
                                       string_measure ( confdir, NULL ))))
                DBGMSG( DBG_KFG, DBG_FLAG(DBG_KFG),
                    ( "KFG: found from dyn. loader %s\n", confdir ) );
            KDirectoryRelease ( dir );
        }
        KDyldRelease ( dyld );
    }
    return loaded;
}

static bool load_from_default_string(KConfig *self) {
    DBGMSG(DBG_KFG, DBG_FLAG(DBG_KFG), ("KFG: loading from default string\n"));
    return parse_file(self,
        "enbedded default configuration string", default_kfg) == 0;
}

LIB_EXPORT rc_t CC KConfigGetLoadPath ( const KConfig *self,
    const char **path )
{
    if (self == NULL) {
        return RC ( rcKFG, rcPath, rcListing, rcSelf, rcNull );
    }

    if (path == NULL) {
        return RC ( rcKFG, rcPath, rcListing, rcParam, rcNull );
    }

    *path = self->load_path;
    return 0;
}

static
bool load_user_settings
    ( KConfig * self, const KDirectory * dir, const char * path )
{
    return load_from_file_path
        (self, dir, path, string_measure ( path, NULL ), true );
}

static
bool load_from_home ( KConfig * self, const KDirectory * dir,
    const KfgSettings * ks, char * ncbi_settings, size_t ncbi_settings_size)
{
    char ncbi_home [ PATH_MAX ] = "";

    DBGMSG( DBG_KFG, DBG_FLAG(DBG_KFG), ( "KFG: checking NCBI_HOME\n" ) );

    if ( _KConfigGetNcbiHome ( self, ks, ncbi_home, sizeof ncbi_home ) != 0 )
    {
        DBGMSG(DBG_KFG, DBG_FLAG(DBG_KFG),
            ("KFG: cannot read NCBI_HOME from configuration\n"));
        return false;
    }
    else {
        bool loaded =
            load_from_path ( self, dir, ncbi_home, string_size ( ncbi_home ) );

        if ( loaded )
        {
            DBGMSG( DBG_KFG, DBG_FLAG(DBG_KFG),
                ( "KFG: found from '%s'\n", ncbi_home ) );
        }

        _KConfigGetNcbiSettings
            ( self, ks, ncbi_settings, ncbi_settings_size, NULL );

        if ( load_user_settings ( self, dir, ncbi_settings ))
            loaded = true;

        _KConfigSetNcbiHome    ( self, ks, ncbi_home );
        _KConfigSetNcbiSettings( self, ks, ncbi_settings );

        return loaded;
    }
}

static
rc_t load_config_files ( KConfig * self,
    const KDirectory * dir, const KfgSettings * ks, bool * loaded_from_dir )
{
    rc_t rc;
    bool loaded;
    KDirectory * wd;

    char ncbi_settings [ PATH_MAX ] = "";

    assert ( loaded_from_dir );
    * loaded_from_dir = false;

    /* if user supplied a starting point, try that */
    if ( dir != NULL )
    {
        char path [ PATH_MAX ] = "";
        rc = KDirectoryResolvePath ( dir, true, path, sizeof path, "." );
        DBGMSG( DBG_KFG, DBG_FLAG(DBG_KFG),
            ( "KFG: try load from supplied directory '%s'\n", path ) );

        loaded = scan_config_dir ( self, dir );
        if ( loaded )
            DBGMSG( DBG_KFG, DBG_FLAG(DBG_KFG), ( "KFG: found from supplied directory\n" ) );

        rc = _KConfigGetNcbiSettings
            ( self, ks, ncbi_settings, sizeof ncbi_settings, path );
        if (rc == 0) {
            if ( load_user_settings ( self, dir, ncbi_settings ))
                loaded = true;
        }

        if ( loaded ) {
            _KConfigSetNcbiHome    ( self, ks, path );
            _KConfigSetNcbiSettings( self, ks, ncbi_settings );
            * loaded_from_dir = true;
            return rc;
        }
    }

    /* open up the native directory */
    rc = KDirectoryNativeDir ( & wd );
    if ( rc != 0 )
        return rc;

    /* try to load from environment variable */
    rc = load_env_variable_rc ( self, wd, & loaded );
    if ( rc != 0 )
        return rc;
    if ( loaded )
        return 0;

    /* load from the other environment variables */
    loaded = load_from_env_variable ( self, wd );

    /* try to load from standard locations */
    if ( ! loaded )
        loaded = load_from_std_location ( self, wd );

    /* check for config as the result of a user install
       i.e. not an admin installation */
    if ( ! loaded )
        loaded = load_from_fs_location ( self, "../etc/ncbi" );
    if ( ! loaded )
        loaded = load_from_fs_location ( self, "ncbi" );

    if ( ! loaded )
        loaded = load_from_default_string ( self );

    if ( ! s_disable_user_settings )
        loaded |= load_from_home
            ( self, wd, ks, ncbi_settings, sizeof ncbi_settings );
    else {
        DBGMSG ( DBG_KFG, DBG_FLAG ( DBG_KFG ), (
            "KFG: User Settings Are Disables: Skipped Load KFG From Home\n" ) );
        _KConfigSetNcbiHome    ( self, ks, NULL );
        _KConfigSetNcbiSettings( self, ks, NULL );
    }

    KDirectoryRelease ( wd );

    if (self->load_path) {
        char* tmp = NULL;
        self->load_path_sz_tmp = string_size(self->load_path) + 1;
        tmp = realloc(self->load_path, self->load_path_sz_tmp);
        if (tmp) {
            self->load_path = tmp;
        }
    }

    return 0;
}

static
void add_predefined_nodes ( KConfig * self, const char * appname )
{
    size_t bytes;
    char buf [ 4096 ];
    const char *value;

    KDirectory *cwd;
    const KDirectory *dir;

#if ! WINDOWS
    struct utsname name;
#endif

    /* Path to libkfg.so */
    KDyld *dyld;
    rc_t rc = KDyldMake ( & dyld );
    if ( rc == 0 )
    {
        rc = find_home_directory ( dyld, & dir, false );
        if ( rc == 0 )
        {
            KDirectoryResolvePath ( dir, true, buf, sizeof buf, "." );
            KDirectoryRelease ( dir );
        }
        KDyldRelease ( dyld );
    }
    update_node ( self, "vdb/lib/paths/kfg", rc == 0 ? buf : "", true );

    /* Architecture */
#if ! WINDOWS
    if (uname(&name) >= 0)
        update_node(self, "kfg/arch/name", name.nodename, true);
    else
#endif
        update_node ( self, "kfg/arch/name", "", true);

    string_printf(buf, sizeof(buf), &bytes, "%u", _ARCH_BITS);
    update_node ( self, "kfg/arch/bits", buf, true );

    /* *OS */
#if LINUX
    #define OS "linux"
#elif MAC
    #define OS "mac"
#elif WINDOWS
    #define OS "win"
#elif SUN
    #define OS "sun"
#else
    #error unrecognized OS
#endif
    update_node ( self, "OS", OS, true );
#undef OS

#if 0
    /* BUILD_LINKAGE */
#if _STATIC
    #define BUILD_LINKAGE "STATIC"
#else
    #define BUILD_LINKAGE "DYNAMIC"
#endif
    update_node ( self, "BUILD_LINKAGE", BUILD_LINKAGE, true );
#undef BUILD_LINKAGE
#endif

    /* BUILD */
#if _PROFILING
    #define BUILD "PROFILE"
#elif _DEBUGGING
    #define BUILD "DEBUG"
#else
    #define BUILD "RELEASE"
#endif
    update_node(self, "BUILD", BUILD, true );
#undef BUILD

    cwd = NULL;

    /* PWD */
    rc = KDirectoryNativeDir ( & cwd );
    if ( rc == 0 )
        rc = KDirectoryResolvePath ( cwd, true, buf, sizeof buf, "." );
    update_node(self, "PWD", rc == 0 ? buf : "", true );

    /* APPPATH */
    if ( appname != NULL && rc == 0 )
    {
        bytes = string_size ( appname );
        value = string_rchr ( appname, bytes, '/' );
        if ( value == NULL )
            value = string_rchr ( appname, bytes, '\\' );
        if ( value != NULL )
            bytes = appname + bytes - value;
        rc = KDirectoryResolvePath ( cwd, true, buf, sizeof buf, "%s", appname);
        if ( rc == 0 && bytes != 0 )
            buf [ string_size(buf) - bytes ] = 0;
        update_node(self, "APPPATH", rc == 0 ? buf : "", true );
    }

    /* APPNAME */
    rc = LogAppName(buf, sizeof(buf), &bytes);
    if ( rc == 0 )
        buf [ bytes ] = 0;
    update_node(self, "APPNAME", rc == 0 ? buf : "", true );

    /* Environment variables */
    /* some of the variables may be undefined, create nodes with empty values for them */
#define DEFINE_ENV(name)                                         \
    value=getenv(name);                                          \
    update_node(self, name, value == NULL ? "" : value, true )

    DEFINE_ENV("HOST");
    DEFINE_ENV("USER");

    value = NULL;
#if WINDOWS
    value = getenv ( "USERPROFILE" );
#endif
    if ( value == NULL ) {
        value = getenv ( "HOME" );
    }

    if (value == NULL)
    {
        update_node(self, "HOME", "", true );
    }
    else
    {
        rc = KDirectoryResolvePath(cwd, true, buf, sizeof(buf), "%s", value);
        if (rc == 0)
        {
            update_node(self, "HOME", buf, true );
        }
        else
            LOGERR (klogErr, rc, "Unable to create a config item for $HOME");
    }

    DEFINE_ENV("VDB_ROOT");
    DEFINE_ENV("VDB_CONFIG");
#undef DEFINE_ENV

    KDirectoryRelease ( cwd );
}

#if WINDOWS

static isexistingdir(const char *path, const KDirectory *dir) {
    return (KDirectoryPathType(dir, path) & ~kptAlias) == kptDir;
}

static bool isletter(char c) {
    return ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z'));
}

static bool sls(const char *b, size_t sz) {
    if (b == NULL || sz < 3) {
        return false;
    }

    return (b[0] == '/' && isletter(b[1]) && b[2] == '/');
}

static bool _DetectRepeatedDrives(const char *buffer,
    size_t bsize, size_t *fixed, const KDirectory *dir)
{
    assert(fixed);

    if (!sls(buffer, bsize)) {
        return false;
    }

    if (!sls(buffer + 2, bsize - 2)) {
        return false;
    }

    if (isexistingdir(buffer, dir)) {
        return false;
    }

    *fixed = 2;

    while (bsize > *fixed + 2) {
        if (isexistingdir(buffer + *fixed, dir)) {
            break;
        }

        if (!sls(buffer + *fixed + 2, bsize - *fixed - 2)) {
            break;
        }

        *fixed += 2;
    }

    return true;
}

static rc_t _KConfigNodeFixRepeatedDrives(KConfigNode *self,
    bool *updated, const KDirectory *dir)
{
    rc_t rc = 0;

    char buffer[PATH_MAX];
    size_t num_read = 0;
    size_t remaining = 0;

    assert(updated);

    rc = KConfigNodeRead(self, 0, buffer, sizeof buffer, &num_read, &remaining);
    if (rc == 0) {
        size_t fixed = 0;
        if (num_read < sizeof buffer) {
            buffer[num_read] = '\0';
        }
        if (_DetectRepeatedDrives(buffer, num_read, &fixed, dir)
            && fixed != 0 && fixed < num_read)
        {
            rc = KConfigNodeWrite(self, buffer + fixed, num_read - fixed);
            if (rc == 0) {
                *updated = true;
            }
        }
    }

    return rc;
}

static rc_t _KConfigNodeFixChildRepeatedDrives(KConfigNode *self,
    const KDirectory *dir, bool *updated, const char *name, ...)
{
    rc_t rc = 0;
    KConfigNode *node = NULL;

    va_list args;
    va_start(args, name);

    rc = KConfigNodeVOpenNodeUpdate(self, &node, name, args);
    if (rc == 0) {
        rc_t rc = _KConfigNodeFixRepeatedDrives(node, updated, dir);
        KConfigNodeRelease(node);
    }

    va_end(args);

    return rc;
}

static rc_t _KConfigFixRepeatedDrives(KConfig *self,
    const KDirectory *pdir, bool *updated)
{
    rc_t rc = 0;
    KDirectory * dir = ( KDirectory * ) pdir;
    KConfigNode *user = NULL;
    if (dir == NULL) {
        rc = KDirectoryNativeDir(&dir);
    }
    rc = KConfigOpenNodeUpdate(self, &user, "/repository/user");
    if (rc == 0) {
        _KConfigNodeFixChildRepeatedDrives(user, dir, updated, "default-path");
    }
    if (rc == 0) {
        uint32_t i = 0;
        uint32_t count = 0;
        KNamelist *categories = NULL;
        rc_t rc = KConfigNodeListChildren(user, &categories);
        if (rc == 0) {     /* main protected ... */
            rc = KNamelistCount(categories, &count);
        }
        for (i = 0; rc == 0 && i < count; ++i) {
            const char *nCategory = NULL;
            rc_t rc = KNamelistGet(categories, i, &nCategory);
            if (rc == 0) { /* main protected ... */
                KConfigNode *category = NULL;
                rc_t rc = KConfigNodeOpenNodeUpdate(user, &category, nCategory);
                if (rc == 0) {
                    uint32_t i = 0;
                    uint32_t count = 0;
                    KNamelist *subcategories = NULL;
                    rc_t rc = KConfigNodeListChildren(category, &subcategories);
                    if (rc == 0) {     /* main protected ... */
                        rc = KNamelistCount(subcategories, &count);
                    }
                    for (i = 0; rc == 0 && i < count; ++i) {
                        const char *name = NULL;
                        rc_t rc = KNamelistGet(subcategories, i, &name);
                        if (rc == 0) {
                            _KConfigNodeFixChildRepeatedDrives(category,
                                dir, updated, "%s/%s", name, "root");
                        }
                    }
                    RELEASE(KNamelist, subcategories);
                    RELEASE(KConfigNode, category);
                }
            }
        }
        RELEASE(KNamelist, categories);
    }
    RELEASE(KConfigNode, user);
    if (pdir == NULL) {
        RELEASE(KDirectory, dir);
    }
    return rc;
}

#endif

static rc_t StringRelease(const String *self) {
    StringWhack(self);
    return 0;
}

static rc_t _KConfigUseWithExtFlatAlg(KConfig * self, bool * updated,
    const char * old_name,
    const char * new_name,
    const char * updated_name)
{
    rc_t rc = 0;

    String * result = NULL;
    size_t size = 0;
    bool newExists = false;

    assert(updated);
    *updated = false;

    rc = KConfigReadString(self, updated_name, &result);
    if (rc == 0) { /* was updated already */
        RELEASE(String, result);
        return rc;
    }

    rc = KConfigReadString(self, old_name, &result);
    if (rc != 0) /* Bad node was not found. Nothing to do. */
        return 0;
    assert(result);
    size = result->size;
    RELEASE(String, result);
    if (size == 0) /* Bad node is already empty. Nothing to do. */
        return 0;

    rc = KConfigReadString(self, new_name, &result);
    if (rc == 0) { /* Good node was found. */
        RELEASE(String, result);
        newExists = true;
    }

    /* Need to create new node. */
    if (!newExists)
        rc = KConfigWriteString(self, new_name, "sra");

    if (rc == 0) {
        /* Clear old node */
        rc = KConfigWriteString(self, old_name, "");

        if (rc == 0)
            rc = KConfigWriteString(self, updated_name, "updated");

        if (rc == 0)
            *updated = true;
    }

    return rc;
}

static rc_t _KConfigUsePileupAppWithExtFlatAlg(KConfig * self,
    bool * updated)
{
    return _KConfigUseWithExtFlatAlg(self, updated,
        "/repository/user/main/public/apps/sraPileup/volumes/flat",
        "/repository/user/main/public/apps/sraPileup/volumes/withExtFlat",
        "/repository/user/main/public/apps/sraPileup/withExtFlat");
}

static rc_t _KConfigUseRealignAppWithExtFlatAlg(KConfig * self,
    bool * updated)
{
    return _KConfigUseWithExtFlatAlg(self, updated,
        "/repository/user/main/public/apps/sraRealign/volumes/flat",
        "/repository/user/main/public/apps/sraRealign/volumes/withExtFlat",
        "/repository/user/main/public/apps/sraRealign/withExtFlat");
}

static rc_t _KConfigUpdateDefault( KConfig * self, bool * updated,
    const char * node_name, 
    const char * node2_name,
    const char * old_value,
    const char * new_value,
    const char * updated_name )
{
    rc_t rc = 0;

    String * result = NULL;

    String sOldValue;

    bool update1 = false, update2 = false;

    assert(node_name && old_value && new_value && updated_name);

    StringInitCString(&sOldValue, old_value);

    assert ( updated );
    * updated = false;

    rc = KConfigReadString ( self, updated_name, & result );
    if ( rc == 0 ) { /* was updated already */
        free ( result );
        return rc;
    }

    rc = KConfigReadString ( self, node_name, & result );
    if ( rc == 0 ) { /* when found: update just value = old-value */
        if ( StringEqual ( & sOldValue, result ) )
            update1 = * updated = true;
        free ( result );
        result = NULL;
    }
    else /* don't update when node did not exist */
        rc = 0;

    if (node2_name != NULL) {
        rc = KConfigReadString(self, node2_name, &result);
        if (rc == 0) { /* when found: update just value = old-value */
            if (StringEqual(&sOldValue, result))
                update2 = * updated = true;
            free(result);
            result = NULL;
        }
        else /* don't update when node did not exist */
            rc = 0;
    }

    if ( * updated ) {
        assert ( rc == 0 );
        if (update1)
            rc = KConfigWriteString ( self, node_name, new_value);
        if (rc == 0 && update2)
            rc = KConfigWriteString(self, node2_name, new_value);
        if ( rc == 0 )
            rc = KConfigWriteString ( self, updated_name, "updated" );
    }

    return rc;
}

static rc_t _KConfigLowerAscpRate ( KConfig * self, bool * updated ) {
    return _KConfigUpdateDefault(self, updated,
        "/tools/ascp/max_rate", NULL, "1000m", "450m",
        "/tools/ascp/max_rate/450m");
}

static rc_t _KConfigUseTraceCgi(KConfig * self, bool * updated) {
    return _KConfigUpdateDefault(self, updated,
        "/repository/remote/main/CGI/resolver-cgi",
        "/repository/remote/protected/CGI/resolver-cgi",
        "https://www.ncbi.nlm.nih.gov/Traces/names/names.fcgi",
        RESOLVER_CGI,
        "/repository_remote/CGI/resolver-cgi/trace");
}

/* create Accession as Directory repository when it does not exist */
static rc_t _KConfigCheckAd(KConfig * self) {
    const KConfigNode * kfg = NULL;
    rc_t rc = KConfigOpenNodeRead(self, &kfg, "/repository/user/ad");
    if (rc != 0) {
        rc = 0;
        /* create Accession as Directory repository
           when it does not exist */
        if (rc == 0)
            rc = KConfigWriteString(self,
                "/repository/user/ad/public/apps/file/volumes/flatAd", ".");
        if (rc == 0)
            rc = KConfigWriteString(self,
                "/repository/user/ad/public/apps/sra/volumes/sraAd", ".");
        if (rc == 0)
            rc = KConfigWriteString(self,
                "/repository/user/ad/public/apps/sraPileup/volumes/ad", ".");
        if (rc == 0)
            rc = KConfigWriteString(self,
                "/repository/user/ad/public/apps/sraRealign/volumes/ad", ".");
        if (rc == 0)
            rc = KConfigWriteString(self,
                "/repository/user/ad/public/apps/refseq/volumes/refseqAd",
                ".");
        if (rc == 0)
            rc = KConfigWriteString(self,
                "/repository/user/ad/public/root", ".");
    }
    else
        rc = KConfigNodeRelease(kfg);
    return rc;
}

static
rc_t KConfigFill ( KConfig * self, const KDirectory * cfgdir,
    const char * appname, bool local )
{
    KConfigNode * root;
    String empty;
    rc_t rc;

    CONST_STRING ( & empty, "" );

    rc = KConfigNodeMake ( & root, & empty );
    if (rc == 0)
    {
        bool loaded_from_dir = false;

        KfgSettings ks;

        KConfigInit ( self, root );

        add_predefined_nodes ( self, appname );

        _KConfigIniKfgSettings ( self, &ks );

        if ( ! self -> disableFileLoadOnKfgCreating )
            rc = load_config_files ( self, cfgdir, & ks, & loaded_from_dir );

        if ( rc == 0 ) {
         /* commit changes made to magic file nodes
            during parsing (e.g. fixed spelling of dbGaP names) */
            KConfigCommit ( self );
        }
    }

    return rc;
}


extern rc_t ReportKfg ( const ReportFuncs *f, uint32_t indent,
    uint32_t configNodesSkipCount, va_list args );

/* "cfg" [ OUT ] - return parameter for mgr
   if ( "local" == true ) do not initialize G_kfg
   if ( cfgdir != NULL ) then initialize G_kfg. It is used in unit tests */
static
rc_t KConfigMakeImpl ( KConfig ** cfg, const KDirectory * cfgdir, bool local,
    bool disableFileLoadOnKfgCreating )
{
    rc_t rc;
    static const char * appname = NULL;

    static bool latch;
    if ( ! latch )
    {
        appname = ReportInitConfig ( ReportKfg );
        latch = true;
    }

    if ( cfg == NULL )
        rc = RC ( rcKFG, rcMgr, rcCreating, rcParam, rcNull );
    else
    {
        KConfig * mgr = NULL;

        if ( cfgdir != NULL ) {
            /* local = true;
            ALWAYS create and/or return a singleton object. */
        }

        if ( ! local ) {
            if ( G_kfg . ptr ) { /* if already made, just attach */
                rc = KConfigAddRef ( G_kfg.ptr );
                if (rc == 0) {
                    * cfg = G_kfg . ptr;
                }
                return rc;
            }
        }

        mgr = calloc ( 1, sizeof * mgr );
        if ( mgr == NULL )
            rc = RC ( rcKFG, rcMgr, rcCreating, rcMemory, rcExhausted );
        else
        {
            mgr -> disableFileLoadOnKfgCreating = disableFileLoadOnKfgCreating;
            rc = KConfigFill (mgr, cfgdir, appname, local);

            mgr -> initialized = true;


            if ( rc == 0 ) {
                rc_t rc = 0;

                bool updated = false;

                if ( ! s_disable_user_settings ) {
                    bool updatd2 = false;

                    rc = _KConfigLowerAscpRate ( mgr,  & updated );
                    if (rc == 0) {
                        rc = _KConfigUseTraceCgi(mgr, &updatd2);
                        updated |= updatd2;
                    }

                    if (rc == 0) {
                        rc = _KConfigUsePileupAppWithExtFlatAlg(mgr, &updatd2);
                        updated |= updatd2;
                    }

                    if (rc == 0) {
                        rc = _KConfigUseRealignAppWithExtFlatAlg(mgr, &updatd2);
                        updated |= updatd2;
                    }

                    if ( rc == 0 && updated ) {
                        rc = KConfigCommit ( mgr );
                        updated = false;
                    }
                }

#if WINDOWS /* VDB-1554: fix incorrect posix paths in configuration nodes */
                rc = _KConfigFixRepeatedDrives ( mgr, cfgdir, & updated );
                if ( rc == 0 && updated )
                    rc = KConfigCommit ( mgr );
#endif

                if ( rc == 0 )
                    _KConfigCheckAd ( mgr );
            }

            DBGMSG ( DBG_KFG, DBG_FLAG ( DBG_KFG ), ( "\n" ) );

            if ( rc == 0 )
            {
                if ( local )
                {
                    * cfg = mgr;
                }
                else
                {
                    KConfig * prev = atomic_test_and_set_ptr ( & G_kfg, mgr, NULL );
                    if ( prev != NULL )
                    {
                        /* the global singleton was already instantiated: hand out that one */
                        rc = KConfigAddRef ( G_kfg.ptr );
                        if ( rc == 0 )
                        {
                            * cfg = G_kfg . ptr;
                        }
                        /* and we have to deallocate the object we just made! */
                        KConfigEmpty ( mgr );
                        free( ( void * ) mgr);
                    }
                    else
                    {
                        * cfg = mgr;
                    }
                        
                }
                return rc;
            }

            KConfigWhack ( mgr );
        }

        * cfg = NULL;
    }

    return rc;
}

/* Make
 *  create a process-global configuration manager
 *
 *  "cfg" [ OUT ] - return parameter for mgr
 */
LIB_EXPORT rc_t CC KConfigMake(KConfig **cfg, const KDirectory *cfgdir)
{
    return KConfigMakeImpl(cfg, cfgdir, false, false);
}

/* call KConfigMake; do not initialize G_kfg */
LIB_EXPORT
rc_t CC KConfigMakeLocal(KConfig **cfg, const KDirectory *cfgdir)
{
    return KConfigMakeImpl(cfg, cfgdir, true, false);
}

/* call KConfigMake; do not load any file except user settings ( optionally ) */
LIB_EXPORT
rc_t CC KConfigMakeEmpty ( KConfig ** cfg )
{
    return KConfigMakeImpl ( cfg, NULL, false, true );
}

/*--------------------------------------------------------------------------
 * KNamelist
 */
typedef struct KfgConfigNamelist KfgConfigNamelist;
struct KfgConfigNamelist
{
    KNamelist dad;
    size_t count;
    const char *namelist [ 1 ];
};

/* Whack
 */
static
rc_t CC KfgConfigNamelistWhack ( KfgConfigNamelist *self )
{
    free ( self );
    return 0;
}

/* Count
 */
static
rc_t CC KfgConfigNamelistCount ( const KfgConfigNamelist *self,
uint32_t *count )
{
    * count = ( uint32_t ) self -> count;
    return 0;
}

/* Get
 */
static
rc_t CC KfgConfigNamelistGet ( const KfgConfigNamelist *self,
    uint32_t idx, const char **name )
{
    if ( ( size_t ) idx >= self -> count )
        return RC ( rcDB, rcNamelist, rcAccessing, rcParam, rcInvalid );
    * name = self -> namelist [ idx ];
    return 0;
}

/* Make
 */
static KNamelist_vt_v1 vtKfgConfigNamelist =
{
    /* version 1.0 */
    1, 0,

    /* start minor version 0 methods */
    KfgConfigNamelistWhack,
    KfgConfigNamelistCount,
    KfgConfigNamelistGet
    /* end minor version 0 methods */
};

 static
 rc_t KfgConfigNamelistMake ( KNamelist **names, uint32_t count )
 {
     rc_t rc;
     KfgConfigNamelist *self = malloc ( sizeof * self -
         sizeof self -> namelist + count * sizeof self -> namelist [ 0 ] );
     if ( self == NULL )
         rc = RC ( rcKFG, rcMetadata, rcListing, rcMemory, rcExhausted );
     else
     {
         self -> count = 0;

         rc = KNamelistInit ( & self -> dad,
             ( const KNamelist_vt* ) & vtKfgConfigNamelist );
         if ( rc == 0 )
         {
             * names = & self -> dad;
             return 0;
         }

         free ( self );
     }

     return rc;
 }

/* List
 *  create metadata node listings
 */
static
void CC BSTNodeCount ( BSTNode *n, void *data )
{
    * ( uint32_t* ) data += 1;
}

static
void CC KConfigNodeGrabName ( BSTNode *n, void *data )
{
    KfgConfigNamelist *list = data;
    list -> namelist [ list -> count ++ ]
        = ( ( const KConfigNode* ) n ) -> name . addr;
}

/* ListChildren
 *  list all named children
 */
LIB_EXPORT rc_t CC KConfigNodeListChildren ( const KConfigNode *self,
    KNamelist **names )
{
    if ( names == NULL )
        return RC ( rcKFG, rcNode, rcListing, rcParam, rcNull );

    * names = NULL;

    if ( self != NULL )
    {
        rc_t rc;

        uint32_t count = 0;
        BSTreeForEach ( & self -> children, 0, BSTNodeCount, & count );

        rc = KfgConfigNamelistMake ( names, count );
        if ( rc == 0 )
            BSTreeForEach
                ( & self -> children, 0, KConfigNodeGrabName, * names );

        return rc;
    }

    return RC ( rcKFG, rcNode, rcListing, rcSelf, rcNull );
}

static
void CC KConfigGrabName ( BSTNode *n, void *data )
{
    KfgConfigNamelist *list = data;
    list -> namelist [ list -> count ++ ]
        = ( ( const KConfigIncluded* ) n ) -> path;
}

/* ListIncluded
 *  list all included files
 */
LIB_EXPORT rc_t CC KConfigListIncluded ( const KConfig *self,
    KNamelist **names )
{
    if ( names == NULL )
        return RC ( rcKFG, rcMgr, rcListing, rcParam, rcNull );

    * names = NULL;

    if ( self != NULL )
    {
        rc_t rc;

        uint32_t count = 0;
        BSTreeForEach ( & self -> included, 0, BSTNodeCount, & count );

        rc = KfgConfigNamelistMake ( names, count );
        if ( rc == 0 )
            BSTreeForEach
                ( & self -> included, 0, KConfigGrabName, * names );

        return rc;
    }

    return RC ( rcKFG, rcMgr, rcListing, rcSelf, rcNull );
}

/************** Internal node-reading helpers *************/

/* ReadNodeValueFixed
 * copy the node's value into the caller's fixed size buffer and 0-terminate
 */
static rc_t ReadNodeValueFixed ( const KConfigNode* self, char *buf, size_t bsize )
{
    size_t num_read, remaining;
    rc_t rc = KConfigNodeRead ( self, 0, buf, bsize - 1, &num_read, &remaining);
    if (rc == 0)
    {
        if (remaining != 0)
            rc = RC ( rcKFG, rcNode, rcReading, rcBuffer, rcInsufficient );
        else
            buf[num_read] = '\0';
    }
    return rc;
}

/**********************************************************/

/* ReadBool
 *  read a boolean node value
 *
 * self [ IN ] - KConfigNode object
 * result [ OUT ] - return value (true if "TRUE", false if "FALSE"; rc != 0 if neither)
 *
 */
LIB_EXPORT rc_t CC KConfigNodeReadBool ( const KConfigNode *self, bool *result )
{
    rc_t rc;

    if ( result == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcParam, rcNull );
    else
    {
        * result = false;

        if ( self == NULL )
            rc = RC ( rcKFG, rcNode, rcReading, rcSelf, rcNull );
        else
        {
            char buf [ 6 ];
            rc = ReadNodeValueFixed ( self, buf, sizeof buf );
            if ( rc == 0 )
            {
                switch ( tolower ( buf [ 0 ] ) )
                {
                case 'f':
                    if ( buf [ 1 ] == 0 )
                        return 0;
                    if ( strncasecmp ( buf, "false", sizeof buf ) == 0 )
                        return 0;
                    break;

                case 'n':
                    if ( buf [ 1 ] == 0 )
                        return 0;
                    if ( strncasecmp ( buf, "no", sizeof buf ) == 0 )
                        return 0;
                    break;

                case 't':
                    * result = true;
                    if ( buf [ 1 ] == 0 )
                        return 0;
                    if ( strncasecmp ( buf, "true", sizeof buf ) == 0 )
                        return 0;
                    break;

                case 'y':
                    * result = true;
                    if ( buf [ 1 ] == 0 )
                        return 0;
                    if ( strncasecmp ( buf, "yes", sizeof buf ) == 0 )
                        return 0;
                    break;
                }

                * result = false;
                rc = RC ( rcKFG, rcNode, rcReading, rcFormat, rcIncorrect );
            }
        }
    }

    return rc;
}

/* ReadI64
 *  read an integer node value
 *
 * self [ IN ] - KConfigNode object
 * result [ OUT ] - return value (rc != 0 if cannot be converted)
 *
 */
LIB_EXPORT rc_t CC KConfigNodeReadI64 ( const KConfigNode *self, int64_t *result )
{
    rc_t rc;

    if ( result == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcParam, rcNull );
    else
    {
        * result = 0;

        if ( self == NULL )
            rc = RC ( rcKFG, rcNode, rcReading, rcSelf, rcNull );
        else
        {
            /* allow for leading zeros */
            char buf [ 256 ];

            rc = ReadNodeValueFixed(self, buf, sizeof(buf));
            if (rc == 0)
            {
                char* endptr;
                int64_t res = strtoi64(buf, &endptr, 0);
                if ( *endptr == '\0' )
                    *result = res;
                else
                    rc = RC(rcKFG, rcNode, rcReading, rcFormat, rcIncorrect);
            }
        }
    }
    return rc;
}

/* ReadU64
 *  read an unsiged node value
 *
 * self [ IN ] - KConfigNode object
 * result [ OUT ] - return value (rc != 0 if cannot be converted)
 *
 */
LIB_EXPORT rc_t CC KConfigNodeReadU64 ( const KConfigNode *self, uint64_t* result )
{
    rc_t rc;

    if ( result == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcParam, rcNull );
    else
    {
        * result = 0;

        if ( self == NULL )
            rc = RC ( rcKFG, rcNode, rcReading, rcSelf, rcNull );
        else
        {
            /* allow for leading zeros */
            char buf [ 256 ];

            rc = ReadNodeValueFixed(self, buf, sizeof(buf));
            if (rc == 0)
            {
                char* endptr;
                int64_t res = strtou64(buf, &endptr, 0);
                if ( *endptr == '\0' )
                    *result = res;
                else
                    rc = RC(rcKFG, rcNode, rcReading, rcFormat, rcIncorrect);
            }
        }
    }
    return rc;
}

/* ReadF64
 *  read a floating point node value
 *
 * self [ IN ] - KConfigNode object
 * result [ OUT ] - return value (rc != 0 if cannot be converted)
 *
 */
LIB_EXPORT rc_t CC KConfigNodeReadF64( const KConfigNode *self, double* result )
{
    rc_t rc;

    if ( result == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcParam, rcNull );
    else
    {
        * result = 0.0;

        if ( self == NULL )
            rc = RC ( rcKFG, rcNode, rcReading, rcSelf, rcNull );
        else
        {
            /* allow for leading zeros, trailing digits */
            char buf [ 256 ];

            rc = ReadNodeValueFixed(self, buf, sizeof(buf));
            if (rc == 0)
            {
                char* endptr;
                double res = strtod(buf, &endptr);
                if ( *endptr == '\0' )
                    *result = res;
                else
                    rc = RC(rcKFG, rcNode, rcReading, rcFormat, rcIncorrect);
            }
        }
    }
    return rc;
}

#if 0
/*** NB - code needs to be located in VFS ***/
/* ReadNodeValueFixed
 * Allocate a character buffer of sufficient size, copy the node's value into it, 0-terminate. Caller deallocates.
 */
static rc_t ReadNodeValueVariable( const KConfigNode* self, char** buf )
{
    size_t num_read, to_read;
    /* first we ask about the size to be read */
    rc_t rc = KConfigNodeRead ( self, 0, NULL, 0, &num_read, &to_read );
    if ( rc == 0 )
    {
        char* value = malloc( to_read + 1 );
        if ( value )
        {
            rc = ReadNodeValueFixed( self, value, to_read + 1 );
            if ( rc == 0 )
                *buf = value;
            else
                free (value);
        }
        else
            rc = RC( rcKFG, rcNode, rcReading, rcMemory, rcExhausted );
    }
    return rc;
}

/* ReadFloat
 *  read a VPath node value
 *
 * self [ IN ] - KConfigNode object
 * result [ OUT ] - return value (rc != 0 if cannot be converted)
 *
 */
LIB_EXPORT rc_t CC KConfigNodeReadVPath ( const KConfigNode *self, struct VPath** result )
{
    rc_t rc;

    if ( result == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcParam, rcNull );
    else
    {
        * result = NULL;

        if ( self == NULL )
            rc = RC ( rcKFG, rcNode, rcReading, rcSelf, rcNull );
        else
        {
            char* buf;
            rc = ReadNodeValueVariable ( self, &buf );
            if ( rc == 0)
            {
                rc = VPathMake(result, buf);
                free(buf);
            }
        }
    }
    return rc;
}
#endif

/* ReadString
 *  read a String node value
 *
 * self [ IN ] - KConfigNode object
 * result [ OUT ] - return value (rc != 0 if cannot be converted)
 *
 */
LIB_EXPORT rc_t CC KConfigNodeReadString ( const KConfigNode *self, String** result )
{
    rc_t rc;

    if ( result == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcParam, rcNull );
    else
    {
        * result = NULL;

        if ( self == NULL )
            rc = RC ( rcKFG, rcNode, rcReading, rcSelf, rcNull );
        else
        {
            size_t num_read, to_read;

            /* first we ask about the size to be read */
            rc = KConfigNodeRead ( self, 0, NULL, 0, &num_read, &to_read );
            if ( rc == 0 )
            {
                String *value = malloc ( sizeof * value + to_read + 1 );
                if ( value == NULL )
                    rc = RC( rcKFG, rcNode, rcReading, rcMemory, rcExhausted );
                else
                {
                    /* initialize in absence of data - assume ASCII */
                    StringInit ( value, (char*)( value + 1 ), to_read, (uint32_t)to_read );

                    /* read the actual data */
                    rc = ReadNodeValueFixed(self, (char*)value->addr, to_read + 1);
                    if ( rc == 0 )
                    {
                        /* measure length of data to handle non-ASCII */
                        value -> len = string_len ( value -> addr, value -> size );
                        *result = value;
                    }
                    else
                    {
                        rc = RC(rcKFG, rcNode, rcReading, rcFormat, rcIncorrect);
                        free ( value );
                    }
                }
            }
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KConfigRead ( const KConfig * self, const char * path,
    size_t offset, char * buffer, size_t bsize,
    size_t * num_read, size_t * remaining )
{
    const KConfigNode * node = NULL;

    rc_t rc = KConfigOpenNodeRead ( self, & node, "%s", path );
    if ( rc == 0) {
        rc_t rc2 = 0;

        rc = KConfigNodeRead
            ( node, offset, buffer, bsize, num_read, remaining );

        rc2 = KConfigNodeRelease ( node );
        if ( rc == 0 ) {
            rc = rc2;
        }
    }

    return rc;
}

/* this macro wraps a call to KConfigNodeGetXXX in a node-accessing
   code to implement the corresponding KConfigGetXXX function */
#define NODE_TO_CONFIG_ACCESSOR(fn) \
    const KConfigNode* node;                                \
    rc_t rc = KConfigOpenNodeRead ( self, &node, "%s", path );   \
    if ( rc == 0)                                           \
    {                                                       \
        rc_t rc2;                                           \
        rc = fn(node, result);                              \
        rc2 = KConfigNodeRelease(node);                     \
        if (rc == 0)                                        \
            rc = rc2;                                       \
    }                                                       \
    return rc;

/* THESE FUNCTIONS ARE PROTECTED AGAINST BAD "self" AND "path"
   BY KConfigOpenNodeRead, BUT THE CONVERSION FUNCTIONS ARE NOT */
LIB_EXPORT rc_t CC KConfigReadBool ( const KConfig* self, const char* path, bool* result )
{
    NODE_TO_CONFIG_ACCESSOR(KConfigNodeReadBool);
}
LIB_EXPORT rc_t CC KConfigReadI64 ( const KConfig* self, const char* path, int64_t* result )
{
    NODE_TO_CONFIG_ACCESSOR(KConfigNodeReadI64);
}
LIB_EXPORT rc_t CC KConfigReadU64 ( const KConfig* self, const char* path, uint64_t* result )
{
    NODE_TO_CONFIG_ACCESSOR(KConfigNodeReadU64);
}

LIB_EXPORT rc_t CC KConfigReadF64 ( const KConfig* self, const char* path, double* result )
{
    NODE_TO_CONFIG_ACCESSOR(KConfigNodeReadF64);
}

LIB_EXPORT rc_t CC KConfigReadString ( const KConfig* self, const char* path, struct String** result )
{
    NODE_TO_CONFIG_ACCESSOR(KConfigNodeReadString);
}

#define DISP_RC(rc, msg) (void)((rc == 0) ? 0 : LOGERR(klogInt, rc, msg))

LIB_EXPORT rc_t CC KConfigPrintPartial
    (const KConfig *self, int indent, uint32_t skipCount, va_list args)
{
    rc_t rc = 0;

    PrintBuff pb;

    if (rc == 0) {
        PrintBuffInit(&pb, NULL);
    }

    if (rc == 0) {
        rc = KConfigPrintImpl
            (self, indent, NULL, false, false, &pb, skipCount, args);
    }

    if (rc == 0) {
        rc = PrintBuffFlush(&pb);
    }

    return rc;
}

LIB_EXPORT rc_t CC KConfigPrint(const KConfig* self, int indent) {
    return KConfigPrintPartial(self, indent, 0, NULL);
}

LIB_EXPORT rc_t CC KConfigToFile(const KConfig* self, KFile *file) {
    rc_t rc = 0;
    PrintBuff pb;
    PrintBuffInit(&pb, file);
    if (rc == 0) {
        rc = KConfigPrintImpl(self, 0, NULL, false, true, &pb, 0, NULL);
    }
    if (rc == 0) {
        rc = PrintBuffFlush(&pb);
    }
    return rc;
}

LIB_EXPORT void CC KConfigDisableUserSettings ( void )
{
    s_disable_user_settings = true;
}

LIB_EXPORT void CC KConfigSetNgcFile(const char * path) { s_ngc_file = path; }
const char * KConfigGetNgcFile(void) { return s_ngc_file; }

static
rc_t open_file ( const KFile **f, const char *path )
{
    /* get current directory */
    KDirectory *wd;
    rc_t rc = KDirectoryNativeDir ( & wd );
    if ( rc == 0 )
    {
        rc = KDirectoryOpenFileRead ( wd, f, "%s", path );
        KDirectoryRelease ( wd );
    }

    return rc;
}

/* KFS_EXTERN rc_t CC KFileMakeGzip2ForRead ( struct KFile const **gz, struct KFile const *src );
#include <stdio.h> aprintf */
#define aprintf( a, b ) ( ( void ) 0 )
static
rc_t decode_ncbi_gap ( KDataBuffer *mem, const KFile *orig )
{
    char hdr [ 8 ];
    size_t num_read;
    rc_t rc = KFileReadAll ( orig, 0, hdr, sizeof hdr, & num_read );
aprintf("decode_ncbi_gap %d\n", __LINE__);
    if ( rc == 0 && num_read == sizeof hdr )
    {
        if (memcmp(hdr, "ncbi_gap", sizeof hdr) != 0) {
            rc = RC(rcKFG, rcFile, rcReading, rcFile, rcWrongType);
        }
        else {
            uint64_t eof;
aprintf("decode_ncbi_gap %d\n", __LINE__);
            rc = KFileSize ( orig, & eof );
aprintf("decode_ncbi_gap %d\n", __LINE__);
            if ( rc == 0 )
            {
                const KFile *sub;
aprintf("decode_ncbi_gap %d\n", __LINE__);
                rc = KFileMakeSubRead ( & sub, orig, sizeof hdr,
                    eof - sizeof hdr );
aprintf("decode_ncbi_gap %d\n", __LINE__);
                if ( rc == 0 )
                {
                    const KFile *gzip;
aprintf("decode_ncbi_gap %d\n", __LINE__);

/* aprintf          rc = KFileMakeGzip2ForRead ( & gzip, sub ); */
                    rc = KFileMakeGzipForRead ( & gzip, sub );
aprintf("decode_ncbi_gap %d\n", __LINE__);
                    if ( rc == 0 )
                    {
aprintf("decode_ncbi_gap %d\n", __LINE__);
                        rc = KDataBufferMakeBytes ( mem, 0 );
aprintf("decode_ncbi_gap %d\n", __LINE__);
                        if ( rc == 0 )
                        {
                            size_t total, to_read;

                            /* after all of that, we're ready to decompress */
                            for ( total = 0; ; )
                            {
                                char *buff;

aprintf("decode_ncbi_gap %d\n", __LINE__);
                                rc = KDataBufferResize ( mem,
                                    total + 32 * 1024 );
aprintf("decode_ncbi_gap %d\n", __LINE__);
                                if ( rc != 0 )
                                    break;

                                buff = mem -> base;
aprintf("decode_ncbi_gap %d\n", __LINE__);
                                to_read = ( size_t ) mem -> elem_count - total;
aprintf("decode_ncbi_gap %d\n", __LINE__);

                                rc = KFileReadAll ( gzip, total,
                                    & buff [ total ], to_read, & num_read );
aprintf("decode_ncbi_gap %d\n", __LINE__);
                                if ( rc != 0 )
                                    break;

                                total += num_read;

                                if ( num_read < to_read )
                                {
aprintf("decode_ncbi_gap %d\n", __LINE__);
                                    buff [ total ] = 0;
aprintf("decode_ncbi_gap %d\n", __LINE__);
                                    mem -> elem_count = total;
                                    break;
                                }
                            }
                        }

aprintf("decode_ncbi_gap %d\n", __LINE__);
                        KFileRelease ( gzip );
aprintf("decode_ncbi_gap %d\n", __LINE__);
                    }

aprintf("decode_ncbi_gap %d\n", __LINE__);
                    KFileRelease ( sub );
aprintf("decode_ncbi_gap %d\n", __LINE__);
                }
            }
        }
    }

aprintf("decode_ncbi_gap %d\n", __LINE__);
    return rc;
}

typedef struct {
    const char *projectId;
    const char *encryptionKey;
    const char *downloadTicket;
    const char *description;
} KGapConfig;

static
rc_t _KConfigNncToKGapConfig(const KConfig *self, char *text, KGapConfig *kgc)
{
    size_t len = 0;
    int i = 0;

    assert(self && text && kgc);

    memset(kgc, 0, sizeof *kgc);
    len = string_size(text);

    {
        const char version[] = "version ";
        size_t l = sizeof version - 1;
        if (string_cmp(version, l, text, len, (uint32_t)l) != 0) {
            return RC(rcKFG, rcMgr, rcUpdating, rcFormat, rcUnrecognized);
        }
        text += l;
        len -= l;
    }

    {
        const char version[] = "1.0";
        size_t l = sizeof version - 1;
        if (string_cmp(version, l, text, l, (uint32_t)l) != 0) {
            return RC(rcKFG, rcMgr, rcUpdating, rcFormat, rcUnsupported);
        }
        text += l;
        len -= l;
    }

    while (len > 0 && (text[0] == '\r' || text[0] == '\n')) {
        ++text;
        --len;
    }

    for (i = 0; ; ++i) {
        const char *p = NULL;
        if (i == 0) {
            p = strtok(text, "|");
        }
        else {
            p = strtok (NULL, "|");
        }
        if (p == NULL) {
            break;
        }
        switch (i) {
            case 0:
                kgc->projectId = p;
                break;
            case 1:
                kgc->encryptionKey = p;
                break;
            case 2:
                kgc->downloadTicket = p;
                break;
            case 3:
                kgc->description = p;
                break;
        }
    }

    if (!kgc->projectId || !kgc->encryptionKey || !kgc->downloadTicket ||
        !kgc->description)
    {
        return RC(rcKFG, rcMgr, rcUpdating, rcFile, rcInvalid);
    }

    return 0;
}

LIB_EXPORT rc_t KConfigFixMainResolverCgiNode ( KConfig * self ) {
    rc_t rc = 0;

    KConfigNode *node = NULL;
    struct String *result = NULL;

    assert(self);

    if (rc == 0) {
        rc = KConfigOpenNodeUpdate(self, &node,
            "/repository/remote/main/CGI/resolver-cgi");
    }

    if (rc == 0) {
        rc = KConfigNodeReadString(node, &result);
    }

    if (rc == 0) {
        String http;
        CONST_STRING ( & http,
                   "http://www.ncbi.nlm.nih.gov/Traces/names/names.cgi" );
        assert(result);
        if ( result->size == 0 || StringEqual ( & http, result ) ) {
            const char https[] = RESOLVER_CGI;
            rc = KConfigNodeWrite ( node, https, sizeof https );
        }
    }

    free(result);

    KConfigNodeRelease(node);

    return rc;
}

/* We need to call it from KConfigFixProtectedResolverCgiNode:
 * otherwise we call names.cgi, not SDL for dbGaP resolution. */
static rc_t KConfigFixProtectedSdlCgiNode(KConfig * self) {
    rc_t rc = 0;

    KConfigNode *node = NULL;
    struct String *result = NULL;

    assert(self);

    if (rc == 0)
        rc = KConfigOpenNodeUpdate(self, &node,
            "/repository/remote/protected/SDL.2/resolver-cgi");

    if (rc == 0)
        rc = KConfigNodeReadString(node, &result);

    if (rc == 0) {
        assert(result);
        if (result->size == 0) {
            const char https[] = SDL_CGI;
            rc = KConfigNodeWrite(node, https, sizeof https);
        }
    }

    free(result);

    KConfigNodeRelease(node);

    return rc;
}

LIB_EXPORT rc_t KConfigFixProtectedResolverCgiNode ( KConfig * self ) {
    rc_t rc = 0;

    KConfigNode *node = NULL;
    struct String *result = NULL;

    assert(self);

    if (rc == 0) {
        rc = KConfigOpenNodeUpdate(self, &node,
            "/repository/remote/protected/CGI/resolver-cgi");
    }

    if (rc == 0) {
        rc = KConfigNodeReadString(node, &result);
    }

    if (rc == 0) {
        String http;
        CONST_STRING ( & http,
                   "http://www.ncbi.nlm.nih.gov/Traces/names/names.cgi" );
        assert(result);
        if ( result->size == 0 || StringEqual ( & http, result ) ) {
            const char https[] = RESOLVER_CGI;
            rc = KConfigNodeWrite ( node, https, sizeof https );
        }
    }

    free(result);

    KConfigNodeRelease(node);

    if (rc == 0)
        rc = KConfigFixProtectedSdlCgiNode(self);

    return rc;
}

static rc_t _KConfigNodeUpdateChild(KConfigNode *self,
    const char *name, const char *val)
{
    rc_t rc = 0;

    KConfigNode *node = NULL;

    assert(self && name && val);

    if (rc == 0) {
        rc = KConfigNodeOpenNodeUpdate(self, &node, "%s", name);
    }

    if (rc == 0) {
        rc = KConfigNodeWrite(node, val, string_size(val));
    }

    {
        rc_t rc2 = KConfigNodeRelease(node);
        if (rc2 != 0 && rc == 0) {
            rc = rc2;
        }
    }

    return rc;
}

static rc_t _KConfigMkPwdFileAndNode(KConfig *self,
    KConfigNode *rep, const KGapConfig *kgc)
{
    rc_t rc = 0;
    const KConfigNode *ncbiHome = NULL;
    KConfigNode *node = NULL;
    struct String *result = NULL;
    char encryptionKeyPath[PATH_MAX];
    KDirectory *dir = NULL;
    KFile *encryptionKeyFile = NULL;
    size_t num_writ = 0;

    assert(self && kgc && rep);

    if (rc == 0) {
        rc = KConfigOpenNodeRead(self, &ncbiHome, "NCBI_HOME");
    }
    if (rc == 0) {
        rc = KConfigNodeReadString(ncbiHome, &result);
    }
    KConfigNodeRelease(ncbiHome);
    ncbiHome = NULL;

    if (rc == 0) {
        size_t num_writ = 0;
        assert(result && result->addr);
        rc = string_printf(encryptionKeyPath, sizeof encryptionKeyPath,
            &num_writ, "%s/dbGaP-%s.enc_key", result->addr, kgc->projectId);
        if (rc == 0) {
            assert(num_writ < sizeof encryptionKeyPath);
        }
    }
    free(result);
    result = NULL;

    if (rc == 0) {
        rc = KDirectoryNativeDir(&dir);
    }

    if (rc == 0) {
        rc = KDirectoryCreateFile(dir, &encryptionKeyFile,
                                  false, 0600, kcmInit | kcmParents, "%s", encryptionKeyPath);
    }

    KDirectoryRelease(dir);

    if (rc == 0) {
        assert(kgc->encryptionKey);
        rc = KFileWrite(encryptionKeyFile, 0,
            kgc->encryptionKey, string_size(kgc->encryptionKey), &num_writ);
        if (rc == 0) {
            assert(num_writ == string_size(kgc->encryptionKey));
        }
    }

    if (rc == 0) {
        rc = KFileWrite(encryptionKeyFile, string_size(kgc->encryptionKey),
            "\n", 1, &num_writ);
        if (rc == 0) {
            assert(num_writ == 1);
        }
    }

    {
        rc_t rc2 = KFileRelease(encryptionKeyFile);
        if (rc2 != 0 && rc == 0) {
            rc = rc2;
        }
    }

    if (rc == 0) {
        rc = _KConfigNodeUpdateChild(rep,
            "encryption-key-path", encryptionKeyPath);
    }

    KConfigNodeRelease(node);

    return rc;
}

static rc_t _mkNotFoundDir(const char *repoParentPath) {
    rc_t rc = 0;

    KPathType type = kptNotFound;

    KDirectory *wd = NULL;
    rc = KDirectoryNativeDir(&wd);

    if (rc == 0) {
        type = KDirectoryPathType(wd, "%s", repoParentPath);
        if (type == kptNotFound) {
            rc = KDirectoryCreateDir(wd, 0777, kcmCreate|kcmParents, "%s", repoParentPath);
        }
    }

    KDirectoryRelease(wd);

    return rc;
}

static rc_t _KConfigDBGapRepositoryNodes(KConfig *self,
    KConfigNode *rep, const KGapConfig *kgc, const char *repoParentPath,
    const char **newRepoParentPath)
{
    rc_t rc = 0;

    assert(self && rep && kgc);

    if (rc == 0) {
        rc = _KConfigMkPwdFileAndNode(self, rep, kgc);
    }

    if (rc == 0) {
        assert(kgc->downloadTicket);
        rc = _KConfigNodeUpdateChild(rep,
            "download-ticket", kgc->downloadTicket);
    }

    if (rc == 0) {
        rc = _KConfigNodeUpdateChild(rep, "description", kgc->description);
    }

    if (rc == 0) {
        rc = _KConfigNodeUpdateChild(rep, "apps/file/volumes/flat", "files");
    }
    if (rc == 0) {
        const char name [] = "apps/sra/volumes/sraFlat";
        const KConfigNode * node = NULL;
        rc = KConfigNodeOpenNodeRead ( rep, & node, name );
        if ( rc != 0 )
            rc = _KConfigNodeUpdateChild ( rep, name, "sra" );
        else
            KConfigNodeRelease ( node );
    }

    if (rc == 0) {
        rc = _KConfigNodeUpdateChild(rep, "cache-enabled", "true");
    }

    if (rc == 0) {
        static char rootPath[PATH_MAX] = "";
        if (repoParentPath == NULL) {
            size_t num_writ = 0;
            const KConfigNode *home = NULL;
            String *result = NULL;

            if (rc == 0) {
                rc = KConfigOpenNodeRead(self, &home, "HOME");
            }

            if (rc == 0) {
                rc = KConfigNodeReadString(home, &result);
            }

            if (rc == 0) {
                assert(result && result->addr);
                rc = string_printf(rootPath, sizeof rootPath, &num_writ,
                    "%s/ncbi/dbGaP-%s", result->addr, kgc->projectId);
            }

            if (rc == 0) {
                repoParentPath = rootPath;
            }

            free(result);
            KConfigNodeRelease(home);
        }

        if (rc == 0) {
            rc = _KConfigNodeUpdateChild(rep, "root", repoParentPath);
        }
        if (rc == 0) {
            rc = _mkNotFoundDir(repoParentPath);
        }
        if (rc == 0 && newRepoParentPath != NULL) {
            *newRepoParentPath = repoParentPath;
        }
    }

    return rc;
}

static rc_t _KConfigAddDBGapRepository(KConfig *self,
    const KGapConfig *kgc, const char *repoParentPath,
    const char **newRepoParentPath)
{
    rc_t rc = 0;

    KConfigNode *rep = NULL;

    char repNodeName[512] = "";

    assert(self && kgc);

    if (rc == 0) {
        size_t num_writ = 0;
        rc = string_printf(repNodeName, sizeof repNodeName, &num_writ,
            "/repository/user/protected/dbGaP-%s", kgc->projectId);
        if (rc == 0) {
            assert(num_writ < sizeof repNodeName);
        }
    }

    if (rc == 0) {
        rc = KConfigOpenNodeUpdate(self, &rep, "%s", repNodeName);
    }

    if (rc == 0) {
        rc = _KConfigDBGapRepositoryNodes(self, rep, kgc, repoParentPath,
            newRepoParentPath);
    }

    KConfigNodeRelease(rep);

    return rc;
}

LIB_EXPORT rc_t CC KConfigImportNgc(KConfig *self,
    const char *ngcPath, const char *repoParentPath,
    const char **newRepoParentPath)
{
    if (self == NULL) {
        return RC(rcKFG, rcMgr, rcUpdating, rcSelf, rcNull);
    }

    if (ngcPath == NULL) {
        return RC(rcKFG, rcMgr, rcUpdating, rcParam, rcNull);
    }
    else {
        const KFile *orig = NULL;
        rc_t rc = open_file ( & orig, ngcPath );
/*    DBGMSG(DBG_KFG, DBG_FLAG(DBG_KFG), ("KConfigImportNgc %d\n", __LINE__));*/
        if (rc != 0) {
            return rc;
        }
        else {
            KGapConfig kgc;

            KDataBuffer mem;
            memset ( & mem, 0, sizeof mem );

/*DBGMSG(DBG_KFG, DBG_FLAG(DBG_KFG), ("KConfigImportNgc %d\n", __LINE__));
aprintf("KConfigImportNgc %d\n", __LINE__); */
            rc = decode_ncbi_gap ( & mem, orig );
/*DBGMSG(DBG_KFG, DBG_FLAG(DBG_KFG), ("KConfigImportNgc %d\n", __LINE__));
aprintf("KConfigImportNgc %d\n", __LINE__);*/
            KFileRelease ( orig );
            orig = NULL;

            if (rc == 0) {
/*DBGMSG(DBG_KFG, DBG_FLAG(DBG_KFG), ("KConfigImportNgc %d\n", __LINE__));
aprintf("KConfigImportNgc %d\n", __LINE__);*/
                rc = _KConfigNncToKGapConfig(self, mem.base, &kgc);
/*DBGMSG(DBG_KFG, DBG_FLAG(DBG_KFG), ("KConfigImportNgc %d\n", __LINE__));
aprintf("KConfigImportNgc %d\n", __LINE__);*/
            }

            if (rc == 0) {
/*DBGMSG(DBG_KFG, DBG_FLAG(DBG_KFG), ("KConfigImportNgc %d\n", __LINE__));
aprintf("KConfigImportNgc %d\n", __LINE__);*/
                rc = KConfigFixProtectedResolverCgiNode(self);
/*DBGMSG(DBG_KFG, DBG_FLAG(DBG_KFG), ("KConfigImportNgc %d\n", __LINE__));
aprintf("KConfigImportNgc %d\n", __LINE__);*/
            }

            if (rc == 0) {
/*DBGMSG(DBG_KFG, DBG_FLAG(DBG_KFG), ("KConfigImportNgc %d\n", __LINE__));
aprintf("KConfigImportNgc %d\n", __LINE__);*/
                rc = _KConfigAddDBGapRepository(self, &kgc, repoParentPath,
                    newRepoParentPath);
/*DBGMSG(DBG_KFG, DBG_FLAG(DBG_KFG), ("KConfigImportNgc %d\n", __LINE__));
aprintf("KConfigImportNgc %d\n", __LINE__);*/
            }

/*DBGMSG(DBG_KFG, DBG_FLAG(DBG_KFG), ("KConfigImportNgc %d\n", __LINE__));
aprintf("KConfigImportNgc %d\n", __LINE__);*/
            KDataBufferWhack ( & mem );
        }

        return rc;
    }
}


LIB_EXPORT rc_t CC KConfigWriteBool( KConfig *self, const char * path, bool value )
{
    KConfigNode * node;
    rc_t rc = KConfigOpenNodeUpdate ( self, &node, "%s", path );
    if ( rc == 0 )
    {
        rc = KConfigNodeWriteBool ( node, value );
        KConfigNodeRelease ( node );
    }
    return rc;
}


LIB_EXPORT rc_t CC KConfigWriteString( KConfig *self, const char * path, const char * value )
{
    KConfigNode * node;
    rc_t rc = KConfigOpenNodeUpdate ( self, &node, "%s", path );
    if ( rc == 0 )
    {
        rc = KConfigNodeWrite ( node, value, string_size( value ) );
        KConfigNodeRelease ( node );
    }
    return rc;
}


LIB_EXPORT rc_t CC KConfigWriteSString( KConfig *self, const char * path, struct String const * value )
{
    KConfigNode * node;
    rc_t rc = KConfigOpenNodeUpdate ( self, &node, "%s", path );
    if ( rc == 0 )
    {
        rc = KConfigNodeWrite ( node, value->addr, value->size );
        KConfigNodeRelease ( node );
    }
    return rc;
}
