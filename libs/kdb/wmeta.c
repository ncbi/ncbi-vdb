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

#define TRACK_REFERENCES 0

struct KMDataNodeNamelist;
#define KNAMELIST_IMPL struct KMDataNodeNamelist

#include <kdb/extern.h>
#include "wkdb-priv.h"
#include "dbmgr-priv.h"
#include "database-priv.h"
#include "wtable-priv.h"
#include "wcolumn-priv.h"
#include "kdbfmt-priv.h"

#include <kdb/meta.h>
#include <kdb/namelist.h>
#include <klib/container.h>
#include <klib/pbstree.h>
#include <klib/namelist.h>
#include <klib/impl.h>
#include <kfs/file.h>
#include <kfs/mmap.h>
#include <kfs/md5.h>
#include <klib/symbol.h>
#include <klib/refcount.h>
#include <klib/debug.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <atomic.h>
#include <os-native.h>
#include <sysalloc.h>
#include <strtol.h>

#ifndef TRACK_REFERENCES
#define TRACK_REFERENCES 0
#endif

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <byteswap.h>
#include <assert.h>

#define KMETADATAVERS 2
#define NODE_SIZE_LIMIT ( 25 * 1024 * 1024 )
#define NODE_CHILD_LIMIT ( 100 * 1024 )


typedef struct KMDataNodeInflateData KMDataNodeInflateData;
struct KMDataNodeInflateData
{
    KMetadata *meta;
    KMDataNode *par;
    BSTree *bst;
    size_t node_size_limit;
    uint32_t node_child_limit;
    rc_t rc;
    bool byteswap;
};

/*--------------------------------------------------------------------------
 * KMetadata
 *  a versioned, hierarchical structure
 */
struct KMetadata
{
    BSTNode n;

    KDirectory *dir;
    KDBManager *mgr;

    /* owner */
    KDatabase *db;
    KTable *tbl;
    KColumn *col;

    KMD5SumFmt * md5;

    /* root node */
    KMDataNode *root;

    KSymbol sym;

    KRefcount refcount;
    uint32_t opencount;
    uint32_t vers;
    uint32_t rev;
    uint8_t read_only;
    uint8_t dirty;
    bool byteswap;

    char path [ 1 ];
};

static
KMetadata *KMetadataAttach ( const KMetadata *self );

static
rc_t KMetadataSever ( const KMetadata *self );


/*--------------------------------------------------------------------------
 * KMAttrNode
 */
typedef struct KMAttrNode KMAttrNode;
struct KMAttrNode
{
    BSTNode n;
    void *value;
    size_t vsize;
    char name [ 1 ];
};

static
int64_t CC KMAttrNodeCmp ( const void *item, const BSTNode *n )
{
#define a ( ( const char* ) item )
#define b ( ( const KMAttrNode* ) n )

    return strcmp ( a, b -> name );

#undef a
#undef b
}

static
int64_t CC KMAttrNodeSort ( const BSTNode *item, const BSTNode *n )
{
#define a ( ( const KMAttrNode* ) item )
#define b ( ( const KMAttrNode* ) n )

    return strcmp ( a -> name, b -> name );

#undef a
#undef b
}

static
void CC KMAttrNodeWhack ( BSTNode *n, void *data )
{
    free ( n );
}

static
bool CC KMAttrNodeInflate ( PBSTNode *n, void *data )
{
    KMAttrNode *b;
    KMDataNodeInflateData *pb = data;

    const char *name = n -> data . addr;
    size_t size = strlen ( name );
    if ( size >= n -> data . size )
    {
        pb -> rc = RC ( rcDB, rcMetadata, rcConstructing, rcData, rcCorrupt );
        return true;
    }

    /* v2 attributes are more easily allocated
       as a single (name,value) block because
       there are no other associated structures */
    b = malloc ( sizeof * b + n -> data . size );
    if ( b == NULL )
    {
        pb -> rc = RC ( rcDB, rcMetadata, rcConstructing, rcMemory, rcExhausted );
        return true;
    }

    b -> value = & b -> name [ 1 + size ];
    b -> vsize = n -> data . size - size - 1;
    memmove ( b -> name, name, n -> data . size );
    BSTreeInsert ( pb -> bst, & b -> n, KMAttrNodeSort );
    return false;
}

static
rc_t KMAttrNodeRename ( const KMAttrNode *self,
    KMAttrNode **renamed, const char *name )
{
    size_t size = strlen ( name );
    KMAttrNode *b = malloc ( sizeof * b + self -> vsize + size );
    if ( b == NULL )
        return RC ( rcDB, rcNode, rcRenaming, rcMemory, rcExhausted );

    b -> value = & b -> name [ 1 + size ];
    b -> vsize = self -> vsize;
    strcpy ( b -> name, name );
    memmove ( b -> value, self -> value, self -> vsize );
    * renamed = b;
    return 0;
}

static
rc_t KMAttrNodeMake ( KMAttrNode **np,
    const char *name, const void *value, size_t vsize )
{
    size_t size = strlen ( name );
    KMAttrNode *n = malloc ( sizeof * n + vsize + size );
    if ( n == NULL )
        return RC ( rcDB, rcNode, rcConstructing, rcMemory, rcExhausted );

    n -> value = & n -> name [ 1 + size ];
    n -> vsize = vsize;
    strcpy ( n -> name, name );
    if ( vsize != 0 )
        memmove ( n -> value, value, vsize );
    * np = n;
    return 0;
}


/*--------------------------------------------------------------------------
 * KMDataNode
 *  a node with an optional value,
 *  optional attributes, and optional children
 *
 *  nodes are identified by path, relative to a starting node,
 *  where "/" serves as a path separator.
 */
struct KMDataNode
{
    BSTNode n;
    KMDataNode *par;
    KMetadata *meta;
    void *value;
    size_t vsize;
    BSTree attr;
    BSTree child;
    KRefcount refcount;
    uint8_t read_only;
    char name [ 1 ];
};

static
int64_t CC KMDataNodeCmp ( const void *item, const BSTNode *n )
{
#define a ( ( const char* ) item )
#define b ( ( const KMDataNode* ) n )

    return strcmp ( a, b -> name );

#undef a
#undef b
}

static
int64_t CC KMDataNodeSort ( const BSTNode *item, const BSTNode *n )
{
#define a ( ( const KMDataNode* ) item )
#define b ( ( const KMDataNode* ) n )

    return strcmp ( a -> name, b -> name );

#undef a
#undef b
}

static
void CC KMDataNodeWhack ( BSTNode *n, void *data )
{
    KMDataNode *self = ( KMDataNode* ) n;

    REFMSG ( "KMDataNode", "flush", & self -> refcount );

    self -> meta = NULL;
    atomic32_inc ( & self -> refcount );
    KMDataNodeRelease ( self );
}

/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
LIB_EXPORT rc_t CC KMDataNodeAddRef ( const KMDataNode *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KMDataNode" ) )
        {
        case krefLimit:
            return RC ( rcDB, rcMetadata, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC KMDataNodeRelease ( const KMDataNode *cself )
{
    KMDataNode *self = ( KMDataNode* ) cself;
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KMDataNode" ) )
        {
        case krefOkay:
            return KMetadataSever ( self -> meta );
        case krefWhack:

            if ( self -> meta != NULL )
            {
                self -> read_only = 0;
                return KMetadataSever ( self -> meta );
            }

            KRefcountWhack ( & self -> refcount, "KMDataNode" );

            BSTreeWhack ( & self -> attr, KMAttrNodeWhack, NULL );
            BSTreeWhack ( & self -> child, KMDataNodeWhack, NULL );
            free ( self -> value );
            free ( self );
            break;

        case krefLimit:
            return RC ( rcDB, rcMetadata, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

/* Inflate
 */
static
bool CC KMDataNodeInflate_v1 ( PBSTNode *n, void *data )
{
    void *value;
    KMDataNode *b;
    KMDataNodeInflateData *pb = data;

    /* v1 metadata are flat, with the name
       stored as a NUL terminated string
       followed by value payload */
    const char *name = n -> data . addr;
    size_t size = strlen ( name );
    if ( size >= n -> data . size )
    {
        pb -> rc = RC ( rcDB, rcMetadata, rcConstructing, rcData, rcCorrupt );
        return true;
    }

    b = malloc ( sizeof * b + size );
    if ( b == NULL )
    {
        pb -> rc = RC ( rcDB, rcMetadata, rcConstructing, rcMemory, rcExhausted );
        return true;
    }
     
    b -> par = pb -> par;
    b -> meta = pb -> meta;
    b -> value = ( void* ) ( name + size + 1 );
    b -> vsize = n -> data . size - size - 1;
    BSTreeInit ( & b -> attr );
    BSTreeInit ( & b -> child );
    KRefcountInit ( & b -> refcount, 0, "KMDataNode", "inflate", name );
    b -> read_only = 0;
    strcpy ( b -> name, name );
     
    /* a name with no associated value */
    if ( b -> vsize == 0 )
    {
        b -> value = NULL;
        BSTreeInsert ( pb -> bst, & b -> n, KMDataNodeSort );
        return false;
    }

    /* allocate value because v2+ code expects this */
    value = malloc ( b -> vsize );
    if ( value != NULL )
    {
        memmove ( value, b -> value, b -> vsize );
        b -> value = value;
        BSTreeInsert ( pb -> bst, & b -> n, KMDataNodeSort );
        return false;
    }

    pb -> rc = RC ( rcDB, rcMetadata, rcConstructing, rcMemory, rcExhausted );
    free ( b );
    return true;
}

static
rc_t KMDataNodeInflateAttr ( KMDataNode *n, bool byteswap )
{
    PBSTree *bst;
    rc_t rc = PBSTreeMake ( & bst, n -> value, n -> vsize, byteswap );
    if ( rc != 0 )
        rc = RC ( rcDB, rcMetadata, rcConstructing, rcData, rcCorrupt );
    else
    {
        KMDataNodeInflateData pb;
        size_t bst_size = PBSTreeSize ( bst );
        
        pb . meta = n -> meta;
        pb . par = n;
        pb . bst = & n -> attr;
        pb . node_size_limit = 0;
        pb . node_child_limit = 0;
        pb . rc = 0;
        pb . byteswap = byteswap;
        PBSTreeDoUntil ( bst, 0, KMAttrNodeInflate, & pb );
        rc = pb . rc;
        
        PBSTreeWhack ( bst );
        
        n -> value = ( char* ) n -> value + bst_size;
        n -> vsize -= bst_size;
    }
    return rc;
}

static
bool CC KMDataNodeInflate ( PBSTNode *n, void *data );

static
rc_t KMDataNodeInflateChild ( KMDataNode *n,
    size_t node_size_limit, uint32_t node_child_limit, bool byteswap )
{
    PBSTree *bst;
    rc_t rc = PBSTreeMake ( & bst, n -> value, n -> vsize, byteswap );
    if ( rc != 0 )
        rc = RC ( rcDB, rcMetadata, rcConstructing, rcData, rcCorrupt );
    else
    {
        uint32_t bst_count = PBSTreeCount ( bst );
        size_t bst_size = PBSTreeSize ( bst );
        bool inflate = node_child_limit == 0;
        if ( ! inflate ) {
            if ( bst_count > node_child_limit )
            {
                PLOGMSG ( klogWarn, ( klogWarn,
                                      "refusing to inflate metadata node '$(node)' "
                                      "within file '$(path)': "
                                      "number of children ($(num_children)) "
                                      "exceeds limit ($(limit))."
                                      , "node=%s,path=%s,num_children=%u,limit=%u"
                                      , n -> name
                                      , n -> meta -> path
                                      , bst_count
                                      , node_child_limit )
                    );
            }
            else if ( bst_size > node_size_limit )
            {
                PLOGMSG ( klogWarn, ( klogWarn,
                                      "refusing to inflate metadata node '$(node)' "
                                      "within file '$(path)': "
                                      "node size ($(node_size)) exceeds limit ($(limit))."
                                     , "node=%s,path=%s,node_size=%zu,limit=%zu"
                                     , n -> name
                                     , n -> meta -> path
                                     , bst_size
                                     , node_size_limit )
                    );
            }
            else {
                inflate = true;
            }
        }

        if ( inflate ) {
            KMDataNodeInflateData pb;

            pb . meta = n -> meta;
            pb . par = n;        
            pb . bst = & n -> child;
            pb . node_size_limit = node_size_limit;
            pb . node_child_limit = node_child_limit;
            pb . rc = 0;
            pb . byteswap = byteswap;
            PBSTreeDoUntil ( bst, 0, KMDataNodeInflate, & pb );
            rc = pb . rc;
        }
        
        PBSTreeWhack ( bst );
        
        n -> value = ( char* ) n -> value + bst_size;
        n -> vsize -= bst_size;
    }
    return rc;
}

static
bool CC KMDataNodeInflate ( PBSTNode *n, void *data )
{
    KMDataNode *b;
    KMDataNodeInflateData *pb = data;

    /* v2 names are preceded by a decremented length byte
       that has its upper two bits dedicated to
       signaling existence of attributes & children */
    const char *name = n -> data . addr;
    int bits = * ( ( const uint8_t* ) name ++ );
    size_t size = ( bits >> 2 ) + 1;
    if ( size >= n -> data . size )
    {
        pb -> rc = RC ( rcDB, rcMetadata, rcConstructing, rcData, rcCorrupt );
        return true;
    }

    b = malloc ( sizeof * b + size );
    if ( b == NULL )
    {
        pb -> rc = RC ( rcDB, rcMetadata, rcConstructing, rcMemory, rcExhausted );
        return true;
    }

    b -> par = pb -> par;
    b -> meta = pb -> meta;
    b -> value = ( void* ) ( name + size );
    b -> vsize = n -> data . size - size - 1;
    BSTreeInit ( & b -> attr );
    BSTreeInit ( & b -> child );
    memmove ( b -> name, name, size );
    b -> name [ size ] = 0;
    KRefcountInit ( & b -> refcount, 0, "KMDataNode", "inflate", b -> name );
    b -> read_only = 0;

    pb -> rc = ( bits & 1 ) != 0 ? KMDataNodeInflateAttr ( b, pb -> byteswap ) : 0;
    if ( pb -> rc == 0 )
    {
        pb -> rc = ( bits & 2 ) != 0 ?
            KMDataNodeInflateChild ( b, pb -> node_size_limit, pb -> node_child_limit, pb -> byteswap ) : 0;
        if ( pb -> rc == 0 )
        {
            void *value;

            if ( b -> vsize == 0 )
            {
                b -> value = NULL;
                BSTreeInsert ( pb -> bst, & b -> n, KMDataNodeSort );
                return false;
            }

            value = malloc ( b -> vsize );
            if ( value != NULL )
            {
                memmove ( value, b -> value, b -> vsize );
                b -> value = value;
                BSTreeInsert ( pb -> bst, & b -> n, KMDataNodeSort );
                return false;
            }
            pb -> rc = RC ( rcDB, rcMetadata, rcConstructing, rcMemory, rcExhausted );

            BSTreeWhack ( & b -> child, KMDataNodeWhack, NULL );
        }

        BSTreeWhack ( & b -> attr, KMAttrNodeWhack, NULL );
    }

    free ( b );
    return true;
}


/* Find
 */
static
rc_t KMDataNodeFind ( const KMDataNode *cself, KMDataNode **np, char **path )
{
    KMDataNode *found, *self= ( KMDataNode* ) cself;

    char *end, *name = * path;

    /* interpret leading '/' */
    if ( name [ 0 ] == '/' )
    {
        while ( self -> par != NULL )
            self = self -> par;
        ++ name;
    }

    /* find node */
    for ( found = self; name != NULL; self = found, name = end )
    {
        /* segment path */
        end = strchr ( name, '/' );
        if ( end != NULL )
            * end ++ = 0;

        /* interpret special paths */
        switch ( name [ 0 ] )
        {
        case 0:
            continue;
        case '.':
            switch ( name [ 1 ] )
            {
            case 0:
                continue;
            case '.':
                if ( name [ 2 ] == 0 )
                {
                    /* this is a backup */
                    if ( ( self = self -> par ) == NULL )
                    {
                        * np = NULL;
                        return RC ( rcDB, rcMetadata, rcSelecting, rcPath, rcInvalid );
                    }
                    continue;
                }
                break;
            }
            break;
        }

        /* find actual path */
        found = ( KMDataNode* ) BSTreeFind ( & self -> child, name, KMDataNodeCmp );
        if ( found == NULL )
        {
            /* not found also gets partially found state */
            if ( end != NULL )
                end [ -1 ] = '/';
            * path = name;
            * np = self;
            return SILENT_RC ( rcDB, rcMetadata, rcSelecting, rcPath, rcNotFound );
        }
    }

    /* the current self is our guy */
    * np = self;
    return 0;
}

/* Make
 */
static
rc_t KMDataNodeMake ( KMDataNode *self, KMDataNode **np, char *name )
{
    rc_t rc;
    char *end;

    /* create nodes */
    for ( ; name != NULL; name = end )
    {
        BSTNode *x;
        size_t size;
        KMDataNode *n;

        /* segment path */
        end = strchr ( name, '/' );
        if ( end != NULL )
            * end ++ = 0;

        /* interpret special paths */
        switch ( name [ 0 ] )
        {
        case 0:
            continue;
        case '.':
            switch ( name [ 1 ] )
            {
            case 0:
                continue;
            case '.':
                if ( name [ 2 ] == 0 )
                {
                    /* this is a backup */
                    if ( ( self = self -> par ) == NULL )
                        return RC ( rcDB, rcMetadata, rcInserting, rcPath, rcInvalid );
                    continue;
                }
                break;
            }
            break;
        case '*':
            if ( name [ 1 ] == 0 )
                return RC ( rcDB, rcMetadata, rcInserting, rcPath, rcInvalid );
        }

        /* we use 2 bits of a size byte to indicate
           whether a node has attributes and/or children,
           so limit the node leaf name size to 6 bits.
           since empty names are not allowed, subtract
           one from length to give range 0..64 => 1..64 */
        size = strlen ( name );
        if ( size > ( 0x3F + 1 ) )
            return RC ( rcDB, rcMetadata, rcInserting, rcPath, rcExcessive );

        n = malloc ( sizeof * n + size );
        if ( n == NULL )
            return RC ( rcDB, rcMetadata, rcInserting, rcMemory, rcExhausted );
        n -> par = self;
        n -> meta = self -> meta;
        n -> value = NULL;
        n -> vsize = 0;
        BSTreeInit ( & n -> attr );
        BSTreeInit ( & n -> child );
        strcpy ( n -> name, name );
        KRefcountInit ( & n -> refcount, 0, "KMDataNode", "make", n -> name );
        n -> read_only = 0;

        /* insert */
        rc = BSTreeInsertUnique ( & self -> child,
            & n -> n, & x, KMDataNodeSort );
        if ( rc != 0 )
        {
            free ( n );
            n = ( KMDataNode* ) x;
        }

        /* transition */
        self = n;
    }

    /* the current self is our guy */
    * np = self;
    return 0;
}


/* OpenNodeRead
 * VOpenNodeRead
 *  opens a metadata node
 *
 *  "node" [ OUT ] - return parameter for indicated metadata node
 *
 *  "path" [ IN, NULL OKAY ] - optional path for specifying named
 *  node within metadata hierarchy. when NULL, empty, ".", or "/",
 *  return root node in "node". path separator is "/".
 */
LIB_EXPORT rc_t CC KMetadataOpenNodeRead ( const KMetadata *self,
    const KMDataNode **node, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KMetadataVOpenNodeRead ( self, node, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KMetadataVOpenNodeRead ( const KMetadata *self,
    const KMDataNode **node, const char *path, va_list args )
{
    rc_t rc = 0;

    if ( node == NULL )
        rc = RC ( rcDB, rcMetadata, rcOpening, rcParam, rcNull );
    else if ( self == NULL )
    {
        * node = NULL;
        rc = RC ( rcDB, rcMetadata, rcOpening, rcSelf, rcNull );
    }
    else
        rc = KMDataNodeVOpenNodeRead ( self -> root, node, path, args );

    DBGMSG(DBG_KDB, DBG_FLAG(DBG_KDB_KDB),
            ("KMetadataVOpenNodeRead(%s) = %d\n", path, rc));

    return rc;
}

LIB_EXPORT rc_t CC KMDataNodeOpenNodeRead ( const KMDataNode *self,
    const KMDataNode **node, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KMDataNodeVOpenNodeRead ( self, node, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KMDataNodeVOpenNodeRead ( const KMDataNode *self,
    const KMDataNode **node, const char *path, va_list args )
{
    rc_t rc;
    KMDataNode *found;
    char full [ 4096 ], *p = full;

    if ( node == NULL )
        return RC ( rcDB, rcNode, rcOpening, rcParam, rcNull );

    * node = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcNode, rcOpening, rcSelf, rcNull );

    /* handle special NULL path */
    if ( path == NULL )
        full [ 0 ] = 0;
    else
    {
        int len;

        /* generate full path */
        if ( args == NULL )
            len = snprintf ( full, sizeof full, "%s", path );
        else
            len = vsnprintf ( full, sizeof full, path, args );
        if ( len < 0 || len >= sizeof full )
            return RC ( rcDB, rcNode, rcOpening, rcPath, rcExcessive );
    }
    
    rc = KMDataNodeFind ( self, & found, & p );
    if ( rc == 0 )
    {
        /* check if the node is not open */
        if ( atomic32_read ( & found -> refcount ) == 0 )
        {
            /* mark as read-only, since we're the first to open */
            found -> read_only = 1;
        }
        /* disallow open if already open for write */
        else if ( ! found -> read_only )
        {
            return RC ( rcDB, rcNode, rcOpening, rcNode, rcBusy );
        }

        KMetadataAttach ( found -> meta );
        KMDataNodeAddRef ( found );
        * node = found;
    }

    DBGMSG(DBG_KDB, DBG_FLAG(DBG_KDB_KDB),
            ("KMDataNodeVOpenNodeRead(%s) = %d\n", full, rc));

    return rc;
}


/* OpenNodeUpdate
 * VOpenNodeUpdate
 *  opens a metadata node
 *
 *  "node" [ OUT ] - return parameter for indicated metadata node
 *
 *  "path" [ IN, NULL OKAY ] - optional path for specifying named
 *  node within metadata hierarchy. when NULL, empty, ".", or "/",
 *  return root node in "node". path separator is "/".
 */
LIB_EXPORT rc_t CC KMetadataOpenNodeUpdate ( KMetadata *self,
    KMDataNode **node, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KMetadataVOpenNodeUpdate ( self, node, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KMetadataVOpenNodeUpdate ( KMetadata *self,
    KMDataNode **node, const char *path, va_list args )
{
    rc_t rc;

    if ( node == NULL )
        rc = RC ( rcDB, rcMetadata, rcOpening, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcMetadata, rcOpening, rcSelf, rcNull );
        else if ( self -> read_only )
            rc = RC ( rcDB, rcMetadata, rcOpening, rcNode, rcReadonly );
        else {
            rc = KMDataNodeVOpenNodeUpdate ( self -> root, node, path, args );
            DBGMSG(DBG_KDB, DBG_FLAG(DBG_KDB_KDB),
                        ("KMetadataVOpenNodeUpdate(%s) = %d\n", path, rc));
            return rc;
        }

        * node = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC KMDataNodeOpenNodeUpdate ( KMDataNode *self,
    KMDataNode **node, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KMDataNodeVOpenNodeUpdate ( self, node, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KMDataNodeVOpenNodeUpdate ( KMDataNode *self,
    KMDataNode **node, const char *path, va_list args )
{
    rc_t rc = 0;
    KMDataNode *found;
    char full [ 4096 ], *p = full;

    if ( node == NULL )
        return RC ( rcDB, rcNode, rcOpening, rcParam, rcNull );

    * node = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcNode, rcOpening, rcSelf, rcNull );

    /* handle special NULL path */
    if ( path == NULL )
        full [ 0 ] = 0;
    else
    {
        int len;

        /* generate full path */
        if ( args == NULL )
            len = snprintf ( full, sizeof full, "%s", path );
        else
            len = vsnprintf ( full, sizeof full, path, args );
        if ( len < 0 || len >= sizeof full )
            return RC ( rcDB, rcNode, rcOpening, rcPath, rcExcessive );
    }

    /* don't allow update when open for read */
    if ( self -> read_only ) {
        rc = RC ( rcDB, rcNode, rcOpening, rcNode, rcReadonly );
        DBGMSG(DBG_KDB, DBG_FLAG(DBG_KDB_KDB),
                    ("KMDataNodeVOpenNodeUpdate(%s) = %d\n", full, rc));
        return rc;
    }

    /* find an existing one */
    rc = KMDataNodeFind ( self, & found, & p );
    if ( rc == 0 )
    {
        if ( atomic32_read ( & found -> refcount ) != 0 )
            return RC ( rcDB, rcNode, rcOpening, rcNode, rcBusy );
    }
    else if ( GetRCState ( rc ) == rcNotFound )
    {
        rc = KMDataNodeMake ( found, & found, p );
        if ( rc != 0 )
            return rc;
    }
    else
    {
        return rc;
    }

    KMetadataAttach ( found -> meta );
    KMDataNodeAddRef ( found );
    found -> read_only = false;
    * node = found;

    DBGMSG(DBG_KDB, DBG_FLAG(DBG_KDB_KDB),
                ("KMDataNodeVOpenNodeUpdate(%s) = %d\n", full, rc));

    return rc;
}


/* ByteOrder
 *  indicates whether original byte order is reversed
 *  under current architecture.
 *
 *  the byte order of the column is established by
 *  the host architecture when created.
 *
 *  "reversed" [ OUT ] - if true, the original byte
 *  order is reversed with regard to host native byte order.
 */
LIB_EXPORT rc_t CC KMDataNodeByteOrder ( const KMDataNode *self, bool *reversed )
{
    if ( self != NULL )
        return KMetadataByteOrder ( self -> meta, reversed );

    if ( reversed == NULL )
        return RC ( rcDB, rcMetadata, rcAccessing, rcParam, rcNull );

    * reversed = false;
    return RC ( rcDB, rcMetadata, rcAccessing, rcSelf, rcNull );
}


/* Read
 *  read a node value or attribute
 *
 *  "offset" [ IN ] - initial offset into metadata
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_read" [ OUT ] - number of bytes actually read
 *
 *  "remaining" [ OUT, NULL OKAY ] - optional return parameter for
 *  the number of bytes remaining to be read.
 *  specifically, "offset" + "num_read" + "remaining" == sizeof node data
 */
LIB_EXPORT rc_t CC KMDataNodeRead ( const KMDataNode *self,
    size_t offset, void *buffer, size_t bsize,
    size_t *num_read, size_t *remaining )
{
    rc_t rc;

    size_t dummy;
    if ( remaining == NULL )
        remaining = & dummy;

    if ( num_read == NULL )
        rc = RC ( rcDB, rcNode, rcReading, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcNode, rcReading, rcSelf, rcNull );
        else if ( buffer == NULL && bsize != 0 )
            rc = RC ( rcDB, rcNode, rcReading, rcBuffer, rcNull );
        else
        {
            size_t to_read, avail = self -> vsize;
            if ( offset > avail )
                offset = avail;

            to_read = avail -= offset;
            if ( to_read > bsize )
                to_read = bsize;

            if ( to_read > 0 )
                memmove ( buffer, ( const char* ) self -> value + offset, to_read );

            * num_read = to_read;
            * remaining = avail - to_read;

            return 0;
        }

        * num_read = 0;
    }

    * remaining = 0;
    return rc;
}


/* Addr - PRIVATE
 *  reach into node and get address
 *  returns raw pointer and node size
 */
LIB_EXPORT rc_t CC KMDataNodeAddr ( const KMDataNode *self,
    const void **addr, size_t *size )
{
    rc_t rc;

    size_t dummy;
    if ( size == NULL )
        size = & dummy;

    if ( addr == NULL )
        rc = RC ( rcDB, rcMetadata, rcReading, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcMetadata, rcReading, rcSelf, rcNull );
        else
        {
            * addr = self -> value;
            * size = self -> vsize;

            return 0;
        }

        * addr = NULL;
    }

    * size = 0;
    return rc;
}


/* Write
 *  write a node value or attribute
 *  overwrites anything already there
 *
 *  "buffer" [ IN ] and "size" [ IN ] - new value data
 */
LIB_EXPORT rc_t CC KMDataNodeWrite ( KMDataNode *self, const void *buffer, size_t size )
{
    if ( self == NULL )
        return RC ( rcDB, rcNode, rcWriting, rcSelf, rcNull );
    if ( self -> meta == NULL )
        return RC ( rcDB, rcNode, rcWriting, rcMetadata, rcNull );
    if ( buffer == NULL && size != 0 )
        return RC ( rcDB, rcNode, rcWriting, rcBuffer, rcNull );

    /* don't allow update when open for read or root node */
    if ( self -> read_only || self -> par == NULL )
        return RC ( rcDB, rcNode, rcWriting, rcNode, rcReadonly );

    if ( size == self -> vsize )
    {
        if ( size == 0 )
            return 0;

        memmove ( self -> value, buffer, size );
    }
    else
    {
        void *value;

        if ( buffer == NULL || size == 0 )
        {
            value = NULL;
            size = 0;
        }
        else
        {
            value = malloc ( size );
            if ( value == NULL )
                return RC ( rcDB, rcNode, rcWriting, rcMemory, rcExhausted );
            memmove ( value, buffer, size );
        }
            
        if ( self -> value != NULL )
            free ( self -> value );

        self -> value = value;
        self -> vsize = size;
    }
    
    self -> meta -> dirty = true;

    return 0;
}


/* Append
 *  append data to value
 *
 *  "buffer" [ IN ] and "size" [ IN ] - value data to be appended
 */
LIB_EXPORT rc_t CC KMDataNodeAppend ( KMDataNode *self, const void *buffer, size_t size )
{
    if ( self == NULL )
        return RC ( rcDB, rcNode, rcWriting, rcSelf, rcNull );
    if ( self -> meta == NULL )
        return RC ( rcDB, rcNode, rcWriting, rcMetadata, rcNull );
    if ( buffer == NULL && size != 0 )
        return RC ( rcDB, rcNode, rcWriting, rcBuffer, rcNull );

    /* don't allow update when open for read or root node */
    if ( self -> read_only || self -> par == NULL )
        return RC ( rcDB, rcNode, rcWriting, rcNode, rcReadonly );

    if ( size != 0 )
    {
        void *value = realloc ( self -> value, self -> vsize + size );
        if ( value == NULL )
            return RC ( rcDB, rcNode, rcWriting, rcMemory, rcExhausted );
        memmove ( ( char* ) value + self -> vsize, buffer, size );
        self -> value = value;
        self -> vsize += size;
        self -> meta -> dirty = true;
    }
    return 0;
}


/* Read ( formatted )
 *  reads as integer or float value in native byte order
 *
 *  "bXX" [ OUT ] - return parameter for numeric value
 */
LIB_EXPORT rc_t CC KMDataNodeReadB8 ( const KMDataNode *self, void *b8 )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, b8, 1,
        & num_read, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
            return RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
        if ( num_read < 1 )
            return RC ( rcDB, rcMetadata, rcReading, rcTransfer, rcIncomplete );
    }
    return rc;
}

LIB_EXPORT rc_t CC KMDataNodeReadB16 ( const KMDataNode *self, void *b16 )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, b16, 2,
        & num_read, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
            return RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
        if ( num_read < 2 )
            return RC ( rcDB, rcMetadata, rcReading, rcTransfer, rcIncomplete );

        if ( self -> meta -> byteswap )
            * ( uint16_t* ) b16 = bswap_16 ( * ( const uint16_t* ) b16 );
    }
    return rc;
}

LIB_EXPORT rc_t CC KMDataNodeReadB32 ( const KMDataNode *self, void *b32 )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, b32, 4,
        & num_read, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
            return RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
        if ( num_read < 4 )
            return RC ( rcDB, rcMetadata, rcReading, rcTransfer, rcIncomplete );

        if ( self -> meta -> byteswap )
            * ( uint32_t* ) b32 = bswap_32 ( * ( const uint32_t* ) b32 );
    }
    return rc;
}

LIB_EXPORT rc_t CC KMDataNodeReadB64 ( const KMDataNode *self, void *b64 )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, b64, 8,
        & num_read, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
            return RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
        if ( num_read < 8 )
            return RC ( rcDB, rcMetadata, rcReading, rcTransfer, rcIncomplete );

        if ( self -> meta -> byteswap )
            * ( uint64_t* ) b64 = bswap_64 ( * ( const uint64_t* ) b64 );
    }
    return rc;
}

LIB_EXPORT rc_t CC KMDataNodeReadB128 ( const KMDataNode *self, void *b128 )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, b128, 16,
        & num_read, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
            return RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
        if ( num_read < 16 )
            return RC ( rcDB, rcMetadata, rcReading, rcTransfer, rcIncomplete );

        if ( self -> meta -> byteswap )
        {
            uint64_t *b64 = b128;
            uint64_t tmp = bswap_64 ( b64 [ 0 ] );
            b64 [ 0 ] = bswap_64 ( b64 [ 1 ] );
            b64 [ 1 ] = tmp;
        }
    }
    return rc;
}


/* ReadAs ( formatted )
 *  reads as integer or float value in native byte order
 *  casts smaller-sized values to desired size, e.g.
 *    uint32_t to uint64_t
 *
 *  "i" [ OUT ] - return parameter for signed integer
 *  "u" [ OUT ] - return parameter for unsigned integer
 *  "f" [ OUT ] - return parameter for double float
 */
LIB_EXPORT rc_t CC KMDataNodeReadAsI16 ( const KMDataNode *self, int16_t *i )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, i, sizeof * i,
        & num_read, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
            return RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );

        switch ( num_read )
        {
        case 1:
            * i = ( ( const int8_t* ) i ) [ 0 ];
            break;
        case 2:
            if ( self -> meta -> byteswap )
                * i = bswap_16 ( * i );
            break;
        default:
            return RC ( rcDB, rcMetadata, rcReading, rcTransfer, rcIncomplete );
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KMDataNodeReadAsU16 ( const KMDataNode *self, uint16_t *u )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, u, sizeof * u,
        & num_read, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
            return RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );

        switch ( num_read )
        {
        case 1:
            * u = ( ( const uint8_t* ) u ) [ 0 ];
            break;
        case 2:
            if ( self -> meta -> byteswap )
                * u = bswap_16 ( * u );
            break;
        default:
            return RC ( rcDB, rcMetadata, rcReading, rcTransfer, rcIncomplete );
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KMDataNodeReadAsI32 ( const KMDataNode *self, int32_t *i )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, i, sizeof * i,
        & num_read, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
            return RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );

        switch ( num_read )
        {
        case 1:
            * i = ( ( const int8_t* ) i ) [ 0 ];
            break;
        case 2:
            if ( self -> meta -> byteswap )
                * i = bswap_16 ( ( ( const int16_t* ) i ) [ 0 ] );
            else
                * i = ( ( const int16_t* ) i ) [ 0 ];
            break;
        case 4:
            if ( self -> meta -> byteswap )
                * i = bswap_32 ( * i );
            break;
        default:
            return RC ( rcDB, rcMetadata, rcReading, rcTransfer, rcIncomplete );
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KMDataNodeReadAsU32 ( const KMDataNode *self, uint32_t *u )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, u, sizeof * u,
        & num_read, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
            return RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );

        switch ( num_read )
        {
        case 1:
            * u = ( ( const uint8_t* ) u ) [ 0 ];
            break;
        case 2:
            if ( self -> meta -> byteswap )
                * u = bswap_16 ( ( ( const uint16_t* ) u ) [ 0 ] );
            else
                * u = ( ( const uint16_t* ) u ) [ 0 ];
            break;
        case 4:
            if ( self -> meta -> byteswap )
                * u = bswap_32 ( * u );
            break;
        default:
            return RC ( rcDB, rcMetadata, rcReading, rcTransfer, rcIncomplete );
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KMDataNodeReadAsI64 ( const KMDataNode *self, int64_t *i )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, i, sizeof * i,
        & num_read, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
            return RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );

        switch ( num_read )
        {
        case 1:
            * i = ( ( const int8_t* ) i ) [ 0 ];
            break;
        case 2:
            if ( self -> meta -> byteswap )
                * i = bswap_16 ( ( ( const int16_t* ) i ) [ 0 ] );
            else
                * i = ( ( const int16_t* ) i ) [ 0 ];
            break;
        case 4:
            if ( self -> meta -> byteswap )
                * i = bswap_32 ( ( ( const int32_t* ) i ) [ 0 ] );
            else
                * i = ( ( const int32_t* ) i ) [ 0 ];
            break;
        case 8:
            if ( self -> meta -> byteswap )
                * i = bswap_64 ( * i );
            break;
        default:
            return RC ( rcDB, rcMetadata, rcReading, rcTransfer, rcIncomplete );
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KMDataNodeReadAsU64 ( const KMDataNode *self, uint64_t *u )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, u, sizeof * u,
        & num_read, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
            return RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );

        switch ( num_read )
        {
        case 1:
            * u = ( ( const uint8_t* ) u ) [ 0 ];
            break;
        case 2:
            if ( self -> meta -> byteswap )
                * u = bswap_16 ( ( ( const uint16_t* ) u ) [ 0 ] );
            else
                * u = ( ( const uint16_t* ) u ) [ 0 ];
            break;
        case 4:
            if ( self -> meta -> byteswap )
                * u = bswap_32 ( ( ( const uint32_t* ) u ) [ 0 ] );
            else
                * u = ( ( const uint32_t* ) u ) [ 0 ];
            break;
        case 8:
            if ( self -> meta -> byteswap )
                * u = bswap_64 ( * u );
            break;
        default:
            return RC ( rcDB, rcMetadata, rcReading, rcTransfer, rcIncomplete );
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KMDataNodeReadAsF64 ( const KMDataNode *self, double *f )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, f, sizeof * f,
        & num_read, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
            return RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );

        switch ( num_read )
        {
        case 4:
            if ( self -> meta -> byteswap )
                * ( uint32_t* ) f = bswap_32 ( * ( const uint32_t* ) f );
            * f = ( ( const float* ) f ) [ 0 ];
            break;
        case 8:
            if ( self -> meta -> byteswap )
                * ( uint64_t* ) f = bswap_64 ( * ( const uint64_t* ) f );
            break;
        default:
            return RC ( rcDB, rcMetadata, rcReading, rcTransfer, rcIncomplete );
        }
    }
    return rc;
}


/* Read ( formatted )
 *  reads as C-string
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - output buffer for
 *  NUL terminated string.
 *
 *  "size" [ OUT ] - return parameter giving size of string
 *  not including NUL byte. the size is set both upon success
 *  and insufficient buffer space error.
 */
LIB_EXPORT rc_t CC KMDataNodeReadCString ( const KMDataNode *self,
    char *buffer, size_t bsize, size_t *size )
{
    size_t remaining;
    rc_t rc = KMDataNodeRead ( self, 0, buffer, bsize - 1, size, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
        {
            * size += remaining;
            return RC ( rcDB, rcMetadata, rcReading, rcBuffer, rcInsufficient );
        }

        buffer [ * size ] = 0;
    }
    return rc;
}


/* Write ( formatted )
 *  writes integer or float value in metadata byte order
 *
 *  "bXX" [ IN ] - numeric value
 *
 * NB - we do not currently ( and maybe never ) support
 *  writing in non-native byte-order.
 */
LIB_EXPORT rc_t CC KMDataNodeWriteB8 ( KMDataNode *self, const void *b8 )
{
    return KMDataNodeWrite ( self, b8, 1 );
}

LIB_EXPORT rc_t CC KMDataNodeWriteB16 ( KMDataNode *self, const void *b16 )
{
    return KMDataNodeWrite ( self, b16, 2 );
}

LIB_EXPORT rc_t CC KMDataNodeWriteB32 ( KMDataNode *self, const void *b32 )
{
    return KMDataNodeWrite ( self, b32, 4 );
}

LIB_EXPORT rc_t CC KMDataNodeWriteB64 ( KMDataNode *self, const void *b64 )
{
    return KMDataNodeWrite ( self, b64, 8 );
}

LIB_EXPORT rc_t CC KMDataNodeWriteB128 ( KMDataNode *self, const void *b128 )
{
    return KMDataNodeWrite ( self, b128, 16 );
}


/* Write ( formatted )
 *  writes C-string
 *
 *  "str" [ IN ] - NUL terminated string.
 */
LIB_EXPORT rc_t CC KMDataNodeWriteCString ( KMDataNode *self, const char *str )
{
    if ( str != NULL )
        return KMDataNodeWrite ( self, str, strlen ( str ) );

    if ( self == NULL )
        return RC ( rcDB, rcNode, rcWriting, rcSelf, rcNull );

    return RC ( rcDB, rcNode, rcWriting, rcString, rcNull );
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
LIB_EXPORT rc_t CC KMDataNodeReadAttr ( const KMDataNode *self, const char *name,
    char *buffer, size_t bsize, size_t *size )
{
    rc_t rc;

    if ( size == NULL )
        rc = RC ( rcDB, rcMetadata, rcReading, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcMetadata, rcReading, rcSelf, rcNull );
        else if ( name == NULL )
            rc = RC ( rcDB, rcMetadata, rcReading, rcString, rcNull );
        else if ( name [ 0 ] == 0 )
            rc = RC ( rcDB, rcMetadata, rcReading, rcString, rcEmpty );
        else if ( buffer == NULL && bsize != 0 )
            rc = RC ( rcDB, rcMetadata, rcReading, rcBuffer, rcNull );
        else
        {
            const KMAttrNode *n = ( const KMAttrNode* )
                BSTreeFind ( & self -> attr, name, KMAttrNodeCmp );
            if ( n == NULL )
            {
                * size = 0;
                if ( bsize != 0 )
                    buffer [ 0 ] = 0;
                rc = RC ( rcDB, rcMetadata, rcReading, rcAttr, rcNotFound );
            }
            else
            {
                * size = n -> vsize;
                if ( n -> vsize < bsize )
                {
                    memmove ( buffer, n -> value, n -> vsize );
                    buffer [ n -> vsize ] = 0;
                    return 0;
                }

                return RC ( rcDB, rcMetadata, rcReading, rcBuffer, rcInsufficient );
            }
        }

        * size = 0;
    }

    return rc;
}


/* WriteAttr
 *  writes NUL-terminated string
 *
 *  "name" [ IN ] - NUL terminated attribute name
 *
 *  "value" [ IN ] - NUL terminated attribute value
 */
LIB_EXPORT rc_t CC KMDataNodeWriteAttr ( KMDataNode *self,
    const char *name, const char *value )
{
    rc_t rc;
    size_t size;
    KMAttrNode *n;

    if ( self == NULL )
        return RC ( rcDB, rcNode, rcWriting, rcSelf, rcNull );
    if ( self -> meta == NULL )
        return RC ( rcDB, rcNode, rcWriting, rcMetadata, rcNull );
    if ( name == NULL )
        return RC ( rcDB, rcNode, rcWriting, rcPath, rcNull );
    if ( name [ 0 ] == 0 )
        return RC ( rcDB, rcNode, rcWriting, rcPath, rcInvalid );

    /* don't allow update when open for read or root node */
    if ( self -> read_only || self -> par == NULL )
        return RC ( rcDB, rcNode, rcWriting, rcNode, rcReadonly );

    size = ( value == NULL ) ? 0 : strlen ( value );
    rc = KMAttrNodeMake ( & n, name, value, size );
    if ( rc == 0 )
    {
        BSTNode *x;
        if ( BSTreeInsertUnique ( & self -> attr,
             & n -> n, & x, KMAttrNodeSort ) != 0 )
        {
            BSTreeUnlink ( & self -> attr, x );
            KMAttrNodeWhack ( x, NULL );
            BSTreeInsert ( & self -> attr, & n -> n, KMAttrNodeSort );
        }

        self -> meta -> dirty = true;
    }
    return rc;
}


/* ReadAttrAs ( formatted )
 *  reads as integer or float value in native byte order
 *  casts smaller-sized values to desired size, e.g.
 *    uint32_t to uint64_t
 *
 *  "i" [ OUT ] - return parameter for signed integer
 *  "u" [ OUT ] - return parameter for unsigned integer
 *  "f" [ OUT ] - return parameter for double float
 */
LIB_EXPORT rc_t CC KMDataNodeReadAttrAsI16 ( const KMDataNode *self, const char *attr, int16_t *i )
{
    rc_t rc;
    if ( i == NULL )
        rc = RC ( rcDB, rcMetadata, rcReading, rcParam, rcNull );
    else
    {
        size_t size;
        char buffer [ 256 ];
        rc = KMDataNodeReadAttr ( self, attr, buffer, sizeof buffer, & size );
        if ( rc == 0 )
        {
            char *end;
            long val = strtol ( buffer, & end, 0 );
            if ( end [ 0 ] != 0 )
                rc = RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
            else if ( val < -32768 || val > 32767 )
                rc = RC ( rcDB, rcMetadata, rcReading, rcRange, rcExcessive );
            else
            {
                * i = ( int16_t ) val;
                return 0;
            }
        }

        * i = 0;
    }
    return rc;
}

LIB_EXPORT rc_t CC KMDataNodeReadAttrAsU16 ( const KMDataNode *self, const char *attr, uint16_t *u )
{
    rc_t rc;
    if ( u == NULL )
        rc = RC ( rcDB, rcMetadata, rcReading, rcParam, rcNull );
    else
    {
        size_t size;
        char buffer [ 256 ];
        rc = KMDataNodeReadAttr ( self, attr, buffer, sizeof buffer, & size );
        if ( rc == 0 )
        {
            char *end;
            unsigned long val = strtoul ( buffer, & end, 0 );
            if ( end [ 0 ] != 0 )
                rc = RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
            else if ( val > 0xFFFF )
                rc = RC ( rcDB, rcMetadata, rcReading, rcRange, rcExcessive );
            else
            {
                * u = ( uint16_t ) val;
                return 0;
            }
        }

        * u = 0;
    }
    return rc;
}

LIB_EXPORT rc_t CC KMDataNodeReadAttrAsI32 ( const KMDataNode *self, const char *attr, int32_t *i )
{
    rc_t rc;
    if ( i == NULL )
        rc = RC ( rcDB, rcMetadata, rcReading, rcParam, rcNull );
    else
    {
        size_t size;
        char buffer [ 256 ];
        rc = KMDataNodeReadAttr ( self, attr, buffer, sizeof buffer, & size );
        if ( rc == 0 )
        {
            char *end;
            long val = strtol ( buffer, & end, 0 );
            if ( end [ 0 ] != 0 )
                rc = RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
            else if ( val < (-2147483647 - 1) || val > 2147483647 )
                rc = RC ( rcDB, rcMetadata, rcReading, rcRange, rcExcessive );
            else
            {
                * i = ( int32_t ) val;
                return 0;
            }
        }

        * i = 0;
    }
    return rc;
}

LIB_EXPORT rc_t CC KMDataNodeReadAttrAsU32 ( const KMDataNode *self, const char *attr, uint32_t *u )
{
    rc_t rc;
    if ( u == NULL )
        rc = RC ( rcDB, rcMetadata, rcReading, rcParam, rcNull );
    else
    {
        size_t size;
        char buffer [ 256 ];
        rc = KMDataNodeReadAttr ( self, attr, buffer, sizeof buffer, & size );
        if ( rc == 0 )
        {
            char *end;
            unsigned long val = strtoul ( buffer, & end, 0 );
            if ( end [ 0 ] != 0 )
                rc = RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
            else if ( val > 0xFFFFFFFF )
                rc = RC ( rcDB, rcMetadata, rcReading, rcRange, rcExcessive );
            else
            {
                * u = ( uint32_t ) val;
                return 0;
            }
        }

        * u = 0;
    }
    return rc;
}

LIB_EXPORT rc_t CC KMDataNodeReadAttrAsI64 ( const KMDataNode *self, const char *attr, int64_t *i )
{
    rc_t rc;
    if ( i == NULL )
        rc = RC ( rcDB, rcMetadata, rcReading, rcParam, rcNull );
    else
    {
        size_t size;
        char buffer [ 256 ];
        rc = KMDataNodeReadAttr ( self, attr, buffer, sizeof buffer, & size );
        if ( rc == 0 )
        {
            char *end;
            int64_t val =  strtoi64 ( buffer, & end, 0 );
            if ( end [ 0 ] != 0 )
                rc = RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
            else
            {
                * i = val;
                return 0;
            }
        }

        * i = 0;
    }
    return rc;
}

LIB_EXPORT rc_t CC KMDataNodeReadAttrAsU64 ( const KMDataNode *self, const char *attr, uint64_t *u )
{
    rc_t rc;
    if ( u == NULL )
        rc = RC ( rcDB, rcMetadata, rcReading, rcParam, rcNull );
    else
    {
        size_t size;
        char buffer [ 256 ];
        rc = KMDataNodeReadAttr ( self, attr, buffer, sizeof buffer, & size );
        if ( rc == 0 )
        {
            char *end;
            uint64_t val =  strtou64 ( buffer, & end, 0 );
            if ( end [ 0 ] != 0 )
                rc = RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
            else
            {
                * u = val;
                return 0;
            }
        }

        * u = 0;
    }
    return rc;
}

LIB_EXPORT rc_t CC KMDataNodeReadAttrAsF64 ( const KMDataNode *self, const char *attr, double *f )
{
    rc_t rc;
    if ( f == NULL )
        rc = RC ( rcDB, rcMetadata, rcReading, rcParam, rcNull );
    else
    {
        size_t size;
        char buffer [ 256 ];
        rc = KMDataNodeReadAttr ( self, attr, buffer, sizeof buffer, & size );
        if ( rc == 0 )
        {
            char *end;
            double val = strtod ( buffer, & end );
            if ( end [ 0 ] != 0 )
                rc = RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
            else
            {
                * f = val;
                return 0;
            }
        }

        * f = 0.0;
    }
    return rc;
}


/* Drop
 * VDrop
 *  drop some or all node content
 */
LIB_EXPORT rc_t CC KMDataNodeDropAll ( KMDataNode *self )
{
    if ( self == NULL )
        return RC ( rcDB, rcNode, rcClearing, rcSelf, rcNull );
    if ( self -> meta == NULL )
        return RC ( rcDB, rcNode, rcClearing, rcMetadata, rcNull );
    if ( self -> read_only )
        return RC ( rcDB, rcNode, rcClearing, rcNode, rcReadonly );

    BSTreeWhack ( & self -> attr, KMAttrNodeWhack, NULL );
    BSTreeInit ( & self -> attr );

    BSTreeWhack ( & self -> child, KMDataNodeWhack, NULL );
    BSTreeInit ( & self -> child );

    free ( self -> value );
    self -> value = NULL;
    self -> vsize = 0;
    self -> meta -> dirty = true;

    return 0;
}

LIB_EXPORT rc_t CC KMDataNodeDropAttr ( KMDataNode *self, const char *attr )
{
    if ( self == NULL )
        return RC ( rcDB, rcNode, rcUpdating, rcSelf, rcNull );
    if ( self -> meta == NULL )
        return RC ( rcDB, rcNode, rcUpdating, rcMetadata, rcNull );
    if ( attr == NULL )
        return RC ( rcDB, rcNode, rcUpdating, rcPath, rcNull );
    if ( attr [ 0 ] == 0 )
        return RC ( rcDB, rcNode, rcUpdating, rcPath, rcInvalid );
    if ( self -> read_only )
        return RC ( rcDB, rcNode, rcUpdating, rcNode, rcReadonly );

    /* recognize wildcard */
    if ( attr [ 0 ] == '*' && attr [ 1 ] == 0 )
    {
        BSTreeWhack ( & self -> attr, KMAttrNodeWhack, NULL );
        BSTreeInit ( & self -> attr );
    }
    else
    {
        KMAttrNode *found = ( KMAttrNode* )
            BSTreeFind ( & self -> attr, attr, KMAttrNodeCmp );
        if ( found == NULL )
            return RC ( rcDB, rcNode, rcUpdating, rcAttr, rcNotFound );

        BSTreeUnlink ( & self -> attr, & found -> n );
        KMAttrNodeWhack ( & found -> n, NULL );
    }

    self -> meta -> dirty = true;

    return 0;
}

LIB_EXPORT rc_t CC KMDataNodeDropChild ( KMDataNode *self, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KMDataNodeVDropChild ( self, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KMDataNodeVDropChild ( KMDataNode *self, const char *path, va_list args )
{
    int len;
    rc_t rc;
    KMDataNode *found;
    char full [ 4096 ], *p = full;

    if ( self == NULL )
        return RC ( rcDB, rcNode, rcUpdating, rcSelf, rcNull );
    if ( self -> meta == NULL )
        return RC ( rcDB, rcNode, rcUpdating, rcMetadata, rcNull );
    if ( path == NULL )
        return RC ( rcDB, rcNode, rcUpdating, rcPath, rcNull );
    if ( path [ 0 ] == 0 )
        return RC ( rcDB, rcNode, rcUpdating, rcPath, rcInvalid );

    /* generate full path */
    if ( args == NULL )
        len = snprintf ( full, sizeof full, "%s", path );
    else
        len = vsnprintf ( full, sizeof full, path, args );
    if ( len < 0 || len >= sizeof full )
        return RC ( rcDB, rcNode, rcUpdating, rcPath, rcExcessive );

    /* don't allow update when open for read */
    if ( self -> read_only )
        return RC ( rcDB, rcNode, rcUpdating, rcNode, rcReadonly );

    rc = KMDataNodeFind ( self, & found, & p );
    if ( GetRCState ( rc ) == rcNotFound )
    {
        if ( p [ 0 ] != '*' || p [ 1 ] != 0 )
            return rc;

        BSTreeWhack ( & found -> child, KMDataNodeWhack, NULL );
        BSTreeInit ( & found -> child );
    }
    else
    {
        BSTreeUnlink ( & self -> child, & found -> n );
        KMDataNodeWhack ( & found -> n, NULL );
    }

    self -> meta -> dirty = true;

    return 0;
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
LIB_EXPORT rc_t CC KMDataNodeRenameAttr ( KMDataNode *self, const char *from, const char *to )
{
    rc_t rc;
    KMAttrNode *found, *renamed;

    if ( self == NULL )
        return RC ( rcDB, rcNode, rcRenaming, rcSelf, rcNull );
    if ( self -> meta == NULL )
        return RC ( rcDB, rcNode, rcRenaming, rcMetadata, rcNull );
    if ( from == NULL || to == NULL )
        return RC ( rcDB, rcNode, rcRenaming, rcPath, rcNull );
    if ( from [ 0 ] == 0 || to [ 0 ] == 0 )
        return RC ( rcDB, rcNode, rcRenaming, rcPath, rcInvalid );
    if ( self -> read_only )
        return RC ( rcDB, rcNode, rcRenaming, rcNode, rcReadonly );

    found = ( KMAttrNode* )
        BSTreeFind ( & self -> attr, from, KMAttrNodeCmp );
    if ( found == NULL )
        return RC ( rcDB, rcNode, rcRenaming, rcAttr, rcNotFound );

    rc = KMAttrNodeRename ( found, & renamed, to );
    if ( rc == 0 )
    {
        BSTNode *x;
        rc = BSTreeInsertUnique ( & self -> attr,
            & renamed -> n, & x, KMAttrNodeSort );
        if ( rc != 0 )
        {
            KMAttrNodeWhack ( & renamed -> n, NULL );
            if ( x == & found -> n )
                return 0;
            return RC ( rcDB, rcNode, rcRenaming, rcAttr, rcExists );
        }

        BSTreeUnlink ( & self -> attr, & found -> n );
        KMAttrNodeWhack ( & found -> n, NULL );
        self -> meta -> dirty = true;
    }

    return rc;
}

static
void CC KMDataNodeRelinkParent ( BSTNode *n, void *data )
{
    ( ( KMDataNode* ) n ) -> par = data;
}

LIB_EXPORT rc_t CC KMDataNodeRenameChild ( KMDataNode *self, const char *from, const char *to )
{
    int len;
    rc_t rc;
    KMDataNode *found;
    char full [ 4096 ], *p = full;

    if ( self == NULL )
        return RC ( rcDB, rcNode, rcRenaming, rcSelf, rcNull );
    if ( self -> meta == NULL )
        return RC ( rcDB, rcNode, rcRenaming, rcMetadata, rcNull );
    if ( from == NULL || to == NULL )
        return RC ( rcDB, rcNode, rcRenaming, rcPath, rcNull );
    if ( from [ 0 ] == 0 || to [ 0 ] == 0 )
        return RC ( rcDB, rcNode, rcRenaming, rcPath, rcInvalid );
    if ( self -> read_only )
        return RC ( rcDB, rcNode, rcRenaming, rcNode, rcReadonly );

    /* generate full path */
    len = snprintf ( full, sizeof full, "%s", from );
    if ( len < 0 || len >= sizeof full )
        return RC ( rcDB, rcNode, rcRenaming, rcPath, rcExcessive );

    rc = KMDataNodeFind ( self, & found, & p );
    if ( rc == 0 )
    {
        KMDataNode *renamed;

        if ( atomic32_read ( & found -> refcount ) != 0 )
            return RC ( rcDB, rcNode, rcRenaming, rcNode, rcBusy );

        len = snprintf ( p = full, sizeof full, "%s", to );
        if ( len < 0 || len >= sizeof full )
            return RC ( rcDB, rcNode, rcRenaming, rcPath, rcExcessive );

        rc = KMDataNodeFind ( self, & renamed, & p );
        if ( GetRCState ( rc ) == rcNotFound )
        {
            rc = KMDataNodeMake ( renamed, & renamed, p );
            if ( rc == 0 )
            {
                BSTreeUnlink ( & found -> par -> child, & found -> n );

                renamed -> attr = found -> attr;
                BSTreeInit ( & found -> attr );

                renamed -> child = found -> child;
                BSTreeInit ( & found -> child );
                BSTreeForEach ( & renamed -> child,
                    false, KMDataNodeRelinkParent, renamed );

                renamed -> value = found -> value;
                renamed -> vsize = found -> vsize;
                found -> value = NULL;
                found -> vsize = 0;

                KMDataNodeWhack ( & found -> n, NULL );
                self -> meta -> dirty = true;
            }
        }
        else if ( rc == 0 )
            rc = RC ( rcDB, rcNode, rcRenaming, rcNode, rcExists );
    }

    return rc;
}


/*--------------------------------------------------------------------------
 * KMetadata
 *  a versioned, hierarchical structure
 */

/* Flush
 */
typedef struct KMDFlushData KMDFlushData;
struct KMDFlushData
{
    uint64_t pos;
    KFile *f;
    KMD5File *fmd5;
    uint8_t *buffer;
    size_t bsize;
    size_t marker;
    rc_t rc;
};

static
rc_t CC KMDWriteFunc ( void *param, const void *buffer, size_t size, size_t *num_writ )
{
    KMDFlushData *pb = param;
    size_t total, to_write;

    for ( total = 0; total < size; total += to_write )
    {
        to_write = size - total;
        if ( pb -> marker + to_write > pb -> bsize )
            to_write = pb -> bsize - pb -> marker;

        if ( to_write > 0 )
        {
            memmove ( pb -> buffer + pb -> marker,
                ( const uint8_t* ) buffer + total, to_write );
            pb -> marker += to_write;
        }

        if ( pb -> marker == pb -> bsize )
        {
            size_t num_flushed;
            pb -> rc = KFileWrite ( pb -> f, pb -> pos,
                pb -> buffer, pb -> bsize, & num_flushed );
            if ( pb -> rc != 0 )
            {
                * num_writ = 0;
                return pb -> rc;
            }

            if ( num_flushed == 0 )
            {
                * num_writ = total + to_write;
                return pb -> rc = RC ( rcDB, rcMetadata, rcPersisting, rcTransfer, rcIncomplete );
            }

            pb -> marker -= num_flushed;
            pb -> pos += num_flushed;

            if ( pb -> marker != 0 )
                memmove ( pb -> buffer, pb -> buffer + num_flushed, pb -> marker );
        }
    }

    * num_writ = total;
    return 0;
}

static
rc_t CC KMAttrNodeAuxFunc ( void *param, const void *node, size_t *num_writ,
    PTWriteFunc write, void *write_param )
{
    const KMAttrNode *n = node;
    size_t nsize = strlen ( n -> name );

    if ( write != NULL )
        return ( * write ) ( write_param, n -> name, nsize + n -> vsize + 1, num_writ );

    * num_writ = nsize + n -> vsize + 1;
    return 0;
}

static
rc_t CC KMDataNodeAuxFunc ( void *param, const void *node, size_t *num_writ,
    PTWriteFunc write, void *write_param )
{
    rc_t rc;
    const KMDataNode *n = node;
    size_t nsize = strlen ( n -> name );
    size_t auxsize = 0;

    /* first write node name */
    if ( write != NULL )
    {
        uint8_t bits = ( uint8_t ) ( nsize - 1 ) << 2;
        if ( n -> attr . root != NULL )
            bits |= 1;
        if ( n -> child . root != NULL )
            bits |= 2;
        rc = ( * write ) ( write_param, & bits, 1, num_writ );
        if ( rc == 0 )
            rc = ( * write ) ( write_param, n -> name, nsize, num_writ );
        if ( rc != 0 )
            return rc;
    }

    /* if there are any attributes */
    if ( n -> attr . root != NULL )
    {
        rc = BSTreePersist ( & n -> attr, num_writ,
            write, write_param, KMAttrNodeAuxFunc, NULL );
        if ( rc != 0 )
            return rc;
        auxsize += * num_writ;
    }

    /* if there are any children */
    if ( n -> child . root != NULL )
    {
        rc = BSTreePersist ( & n -> child, num_writ,
            write, write_param, KMDataNodeAuxFunc, NULL );
        if ( rc != 0 )
            return rc;
        auxsize += * num_writ;
    }

    /* finally write value */
    if ( write == NULL )
    {
        * num_writ = nsize + 1 + auxsize + n -> vsize;
        return 0;
    }

    rc = ( * write ) ( write_param, n -> value, n -> vsize, num_writ );
    * num_writ += nsize + 1 + auxsize;
    return rc;
}

static
rc_t KMetadataFlush ( KMetadata *self )
{
    rc_t rc;
    KMDFlushData pb;
    memset ( & pb, 0, sizeof pb );

    /* allocate buffer */
    pb . buffer = malloc ( pb . bsize = 32 * 1024 );
    if ( pb . buffer == NULL )
        rc = RC ( rcDB, rcMetadata, rcPersisting, rcMemory, rcExhausted );
    else
    {
        /* open output file */
        rc = KColumnFileCreate ( & pb . f, & pb . fmd5, self -> dir, self -> md5,
                                 kcmInit | kcmParents, false, "md/cur.tmp" );
        /* ZZZZ do we need a "KMD5FileReset ( pb -> fmd5 )" ? I don't think so */
        if ( rc == 0 )
        {
            /* write header */
            KDBHdr *hdr = ( KDBHdr* ) pb . buffer;
            hdr -> endian = eByteOrderTag;
            hdr -> version = KMETADATAVERS;
            pb . marker = sizeof * hdr;

            /* persist root node */
            rc = BSTreePersist ( & self -> root -> child, NULL,
                KMDWriteFunc, & pb, KMDataNodeAuxFunc, NULL );
            if ( rc == 0 && pb . marker != 0 )
            {
                size_t num_flushed;
                rc = KFileWrite ( pb . f, pb . pos,
                                  pb . buffer, pb . marker, & num_flushed );
                if ( rc == 0 && num_flushed != pb . marker )
                    rc = RC ( rcDB, rcMetadata, rcPersisting, rcTransfer, rcIncomplete );
            }
            pb . rc = KFileRelease ( pb . f );
            if ( pb . rc  ==  0 )
                pb . fmd5 = NULL;
            if ( rc == 0 && ( rc = pb . rc ) == 0 )
            {
                /* rename file */
                rc = KDirectoryRename ( self -> dir, true, "md/cur.tmp", "md/cur" );

                if ( self->md5 != NULL && rc == 0 )
                    rc = KMD5SumFmtRename ( self -> md5, "md/cur.tmp", "md/cur" );

                if ( rc == 0 )
                    self -> dirty = false;
            }
        }

        free ( pb . buffer );
    }
    return rc;
}

/* Whack
 */
static
rc_t KMetadataWhack ( KMetadata *self )
{
    rc_t rc = 0;
    KSymbol * symb;
    KDBManager *mgr = self -> mgr;
    assert ( mgr != NULL );

    KRefcountWhack ( & self -> refcount, "KMetadata" );

    if ( self -> dirty )
    {
        /* if this was a version 1 file,
           first freeze it */
        if ( self -> vers == 1 )
        {
            rc = KMetadataFreeze ( self );
            if ( rc != 0 )
                return rc;
        }

        /* flush it */
        rc = KMetadataFlush ( self );
        if ( rc != 0 )
            return rc;
        self -> dirty = false;
    }

    if ( self -> db != NULL )
    {
        rc = KDatabaseSever ( self -> db );
        if ( rc != 0 )
            return rc;
        self -> db = NULL;
    }
    else if ( self -> tbl != NULL )
    {
        rc = KTableSever ( self -> tbl );
        if ( rc != 0 )
            return rc;
        self -> tbl = NULL;
    }
    else if ( self -> col != NULL )
    {
        rc = KColumnSever ( self -> col );
        if ( rc != 0 )
            return rc;
        self -> col = NULL;
    }

    if ( self -> md5 != NULL )
    {
	rc = KMD5SumFmtRelease ( self -> md5 );
	if ( rc != 0 )
	    return rc;
	self -> md5 = NULL;
    }

    /* remove from mgr */
    symb = KDBManagerOpenObjectFind (mgr, self->path);
    if (symb != NULL)
    {
        rc = KDBManagerOpenObjectDelete (mgr, symb);
        if (rc == 0)
        {
            /* release manager
               should never fail */
            rc = KDBManagerSever ( mgr );
            if ( rc != 0 )
                KDBManagerOpenObjectAdd (mgr, symb);
            else
            {
                /* complete */
                KDirectoryRelease ( self -> dir );
                atomic32_set ( & self -> refcount, 0 );
                KMDataNodeWhack ( & self -> root -> n, NULL );
                free ( self );
                return 0;
            }
        }
    }

    KRefcountInit ( & self -> refcount, 1, "KMetadata", "whack", "kmeta" );
    return rc;
}


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
LIB_EXPORT rc_t CC KMetadataAddRef ( const KMetadata *cself )
{
    KMetadata *self = ( KMetadata* ) cself;
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KMetadata" ) )
        {
        case krefLimit:
            return RC ( rcDB, rcMetadata, rcAttaching, rcRange, rcExcessive );
        }
        ++ self -> opencount;
    }
    return 0;
}

LIB_EXPORT rc_t CC KMetadataRelease ( const KMetadata *cself )
{
    KMetadata *self = ( KMetadata* ) cself;
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KMetadata" ) )
        {
        case krefWhack:
            return KMetadataWhack ( ( KMetadata* ) self );
        case krefLimit:
            return RC ( rcDB, rcMetadata, rcReleasing, rcRange, rcExcessive );
        }
        -- self -> opencount;
    }
    return 0;
}

static
KMetadata *KMetadataAttach ( const KMetadata *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAddDep ( & self -> refcount, "KMetadata" ) )
        {
        case krefLimit:
            return NULL;
        }
    }
    return ( KMetadata* ) self;
}

static
rc_t KMetadataSever ( const KMetadata *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDropDep ( & self -> refcount, "KMetadata" ) )
        {
        case krefWhack:
            return KMetadataWhack ( ( KMetadata* ) self );
        case krefLimit:
            return RC ( rcDB, rcMetadata, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

/* Make
 */

static
rc_t KMetadataPopulate ( KMetadata *self, const KDirectory *dir, const char *path, bool read_only )
{
    const KFile *f;
    rc_t rc = KDirectoryOpenFileRead ( dir, & f, "%s", path );
    if ( rc == 0 )
    {
        const KMMap *mm;
        rc = KMMapMakeRead ( & mm, f );
        if ( rc == 0 )
        {
            size_t size;
            const void *addr;
            rc = KMMapSize ( mm, & size );
            if ( rc == 0 )
                rc = KMMapAddrRead ( mm, & addr );

            if ( rc == 0 )
            {
                union
                {
                    KDBHdr v1;
                    KDBHdr v2;
                } hdrs;

                const KDBHdr *hdr = ( const KDBHdr* ) addr;
                const void *pbstree_src = hdr + 1;

                rc = KDBHdrValidate ( hdr, size, 1, KMETADATAVERS );
                if ( self -> read_only && GetRCState ( rc ) == rcIncorrect && GetRCObject ( rc ) == rcByteOrder )
                {
                    hdrs . v1 . endian = bswap_32 ( hdr -> endian );
                    hdrs . v1 . version = bswap_32 ( hdr -> version );
                    rc = KDBHdrValidate ( & hdrs . v1, size, 1, KMETADATAVERS );
                    if ( rc == 0 )
                    {
                        self -> byteswap = true;
                        switch ( hdrs . v1 . version )
                        {
                        case 1:
                            hdr = & hdrs . v1;
                            break;
                        case 2:
                            hdr = & hdrs . v2;
                            break;
                        }
                    }
                }
                if ( rc == 0 )
                {
                    PBSTree *bst;
                    rc = PBSTreeMake ( & bst, pbstree_src, size - sizeof * hdr, self -> byteswap );
                    if ( rc != 0 )
                        rc = RC ( rcDB, rcMetadata, rcConstructing, rcData, rcCorrupt );
                    else
                    {
                        KMDataNodeInflateData pb;

                        pb . meta = self;
                        pb . par = self -> root;
                        pb . bst = & self -> root -> child;
                        pb . node_size_limit = read_only ? NODE_SIZE_LIMIT : 0;
                        pb . node_child_limit = read_only ? NODE_CHILD_LIMIT : 0;
                        pb . rc = 0;
                        pb . byteswap = self -> byteswap;

                        if ( hdr -> version == 1 )
                            PBSTreeDoUntil ( bst, false, KMDataNodeInflate_v1, & pb );
                        else
                            PBSTreeDoUntil ( bst, false, KMDataNodeInflate, & pb );
                        rc = pb . rc;

                        self -> vers = hdr -> version;

                        PBSTreeWhack ( bst );
                    }
                }
            }

            KMMapRelease ( mm );
        }

        KFileRelease ( f );
    }
    return rc;
}


static
rc_t KDBManagerInsertMetadata ( KDBManager * self, KMetadata * meta )
{
    rc_t rc;
    rc = KDBManagerOpenObjectAdd (self, &meta->sym);
    if ( rc == 0 )
        meta -> mgr = KDBManagerAttach ( self );
    return rc;
}


static
rc_t KMetadataMake ( KMetadata **metap,
    KDirectory *dir, const char *path, uint32_t rev,
    bool populate, bool read_only )
{
    rc_t rc;
    KMetadata *meta = malloc ( sizeof * meta + strlen ( path ) );
    if ( meta == NULL )
        rc = RC ( rcDB, rcMetadata, rcConstructing, rcMemory, rcExhausted );
    else
    {
        memset ( meta, 0, sizeof * meta );
        meta -> root = calloc ( 1, sizeof * meta -> root );
        if ( meta -> root == NULL )
            rc = RC ( rcDB, rcMetadata, rcConstructing, rcMemory, rcExhausted );
        else
        {
            meta -> root -> meta = meta;
            meta -> dir = dir;
            KRefcountInit ( & meta -> refcount, 1, "KMetadata", "make-update", path );
            meta -> opencount = 1;
            meta -> rev = rev;
            meta -> read_only = read_only;

            strcpy ( meta -> path, path );

            meta->sym.u.obj = meta;
            StringInitCString (&meta->sym.name, meta->path);
            meta->sym.type = kptMetadata;

            KRefcountInit ( & meta -> root -> refcount, 0, "KMDataNode", "make-read", "/" );

            if ( ! populate )
            {
                meta -> vers = KMETADATAVERS;
                KDirectoryAddRef ( dir );
                * metap = meta;
                return 0;
            }
            
            rc = KMetadataPopulate ( meta, dir, path, read_only );
            if ( rc == 0 )
            {
                KDirectoryAddRef ( dir );
                * metap = meta;
                return 0;
            }

            free ( meta -> root );
        }

        free ( meta );
    }
    * metap = NULL;
    return rc;
}

#if 0
static
rc_t KMetadataMakeRead ( KMetadata **meta,
    const KDirectory *dir, const char *path, uint32_t rev )
{
    rc_t rc = KMetadataMakeUpdate ( meta,
        ( KDirectory* ) dir, path, rev, true );
    if ( rc == 0 )
        ( * meta ) -> read_only = true;
    return rc;
}
#endif


/* OpenMetadataRead
 *  opens metadata for read
 *
 *  "meta" [ OUT ] - return parameter for metadata
 */
static
rc_t KDBManagerOpenMetadataReadInt ( KDBManager *self,
    const KMetadata **metap, const KDirectory *wd, uint32_t rev, bool prerelease,bool *cached )
{
    char metapath [ 4096 ];
    rc_t rc = ( prerelease == 1 ) ?
        KDirectoryVResolvePath ( wd, true, metapath, sizeof metapath, "meta", NULL ):
        ( ( rev == 0 ) ?
          KDirectoryVResolvePath ( wd, true, metapath, sizeof metapath, "md/cur", NULL ):
          KDirectoryResolvePath ( wd, true, metapath, sizeof metapath, "md/r%.3u", rev ) );
    if(cached != NULL ) *cached = false;
    if ( rc == 0 )
    {
        KMetadata * meta;
        KSymbol * sym;

        /* if already open */
        sym = KDBManagerOpenObjectFind (self, metapath);
        if (sym != NULL)
        {
            const KMetadata * cmeta;
            rc_t obj;

	    if(cached != NULL ) *cached = true;
            switch (sym->type)
            {
            case kptMetadata:
                cmeta = (KMetadata*)sym->u.obj;
                /* if open for update, refuse */
                if ( cmeta -> read_only )
                {
                    /* attach a new reference and we're gone */
                    rc = KMetadataAddRef ( cmeta );
                    if ( rc == 0 )
                        * metap = cmeta;
                    return rc;
                }
                obj = rcMetadata;
                break;

            default:
                obj = rcPath;
                break;
            case kptTable:
                obj = rcTable;
                break;
            case kptColumn:
                obj = rcColumn;
                break;
            case kptIndex:
                obj = rcIndex;
                break;
            case kptDatabase:
                obj = rcDatabase;
                break;
            }
            return  RC (rcDB, rcMgr, rcOpening, obj, rcBusy);
	}


        switch ( KDirectoryPathType ( wd, "%s", metapath ) )
        {
        case kptNotFound:
            rc = RC ( rcDB, rcMgr, rcOpening, rcMetadata, rcNotFound );
            break;
        case kptBadPath:
            rc = RC ( rcDB, rcMgr, rcOpening, rcPath, rcInvalid );
            break;
        case kptFile:
        case kptFile | kptAlias:
            break;
        default:
            rc = RC ( rcDB, rcMgr, rcOpening, rcPath, rcIncorrect );
            break;
        }

        if ( rc == 0 )
        {
            rc = KMetadataMake ( & meta, ( KDirectory* ) wd, metapath, rev, true, true );
            
            if ( rc == 0 )
            {
                rc = KDBManagerInsertMetadata (self, meta );
                if ( rc == 0 )
                {
                    * metap = meta;
                    return 0;
                }

                KMetadataRelease ( meta );
            }

/*             rc = RC ( rcDB, rcMgr, rcOpening, rcMetadata, rcExists ); */
        }
    }
    
    return rc;
}

LIB_EXPORT rc_t CC KDatabaseOpenMetadataRead ( const KDatabase *self, const KMetadata **metap )
{
    rc_t rc;
    const KMetadata *meta;
    bool  meta_is_cached;

    if ( metap == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcParam, rcNull );

    * metap = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcSelf, rcNull );

    rc = KDBManagerOpenMetadataReadInt ( self -> mgr, & meta, self -> dir, 0, false, &meta_is_cached );
    if ( rc == 0 )
    {
        if(!meta_is_cached) ((KMetadata*)meta) -> db = KDatabaseAttach ( self );
        * metap = meta;
    }

    return rc;
}

LIB_EXPORT rc_t CC KTableOpenMetadataRead ( const KTable *self, const KMetadata **metap )
{
    rc_t rc;
    const KMetadata *meta;
    bool  meta_is_cached;

    if ( metap == NULL )
        return RC ( rcDB, rcTable, rcOpening, rcParam, rcNull );

    * metap = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcTable, rcOpening, rcSelf, rcNull );

    rc = KDBManagerOpenMetadataReadInt ( self -> mgr, & meta, self -> dir, 0, self -> prerelease, &meta_is_cached );
    if ( rc == 0 )
    {
        if(!meta_is_cached) ((KMetadata*)meta) -> tbl = KTableAttach ( self );
        * metap = meta;
    }

    return rc;
}

LIB_EXPORT rc_t CC KColumnOpenMetadataRead ( const KColumn *self, const KMetadata **metap )
{
    rc_t rc;
    const KMetadata *meta;
    bool  meta_is_cached;

    if ( metap == NULL )
        return RC ( rcDB, rcColumn, rcOpening, rcParam, rcNull );

    * metap = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcColumn, rcOpening, rcSelf, rcNull );

    rc = KDBManagerOpenMetadataReadInt ( self -> mgr, & meta, self -> dir, 0, false, &meta_is_cached );
    if ( rc == 0 )
    {
        if(!meta_is_cached) ((KMetadata*)meta) -> col = KColumnAttach ( self );
        * metap = meta;
    }

    return rc;
}

/* OpenMetadataUpdate
 *  open metadata for read/write
 *
 *  "meta" [ OUT ] - return parameter for metadata
 */
static
rc_t KDBManagerOpenMetadataUpdateInt ( KDBManager *self,
    KMetadata **metap, KDirectory *wd, KMD5SumFmt * md5 )
{
/* WAK
 * NEEDS MD5 UPDATE???
 */
    char metapath [ 4096 ];
    rc_t rc = KDirectoryResolvePath ( wd, true,
        metapath, sizeof metapath, "md/cur" );
    if ( rc == 0 )
    {
        KSymbol * sym;
        KMetadata *meta;
        bool populate = true;

        switch ( KDirectoryPathType ( wd, "%s", metapath ) )
        {
        case kptNotFound:
            populate = false;
            break;
        case kptBadPath:
            return RC ( rcDB, rcMgr, rcOpening, rcPath, rcInvalid );
        case kptFile:
        case kptFile | kptAlias:
            break;
        default:
            return RC ( rcDB, rcMgr, rcOpening, rcPath, rcIncorrect );
        }

        /* if already open */
        sym = KDBManagerOpenObjectFind (self, metapath);
        if (sym != NULL)
        {
            rc_t obj;
            switch (sym->type)
            {
            default:
                obj = rcPath;
                break;
            case kptDatabase:
                obj = rcDatabase;
                break;
            case kptTable:
                obj = rcTable;
                break;
            case kptColumn:
                obj = rcColumn;
                break;
            case kptIndex:
                obj = rcIndex;
                break;
            case kptMetadata:
                obj = rcMetadata;
                break;
            }
            return RC ( rcDB, rcMgr, rcOpening, obj, rcBusy );
        }

        rc = KMetadataMake ( & meta, wd, metapath, 0, populate, false );
        if ( rc == 0 )
        {
            rc = KDBManagerInsertMetadata (self, meta);
            if (rc == 0)
            {
                if ( md5 != NULL )
                {
                    meta -> md5 = md5;
                    rc = KMD5SumFmtAddRef ( md5 );
                }

                if ( rc == 0 )
                {
                    * metap = meta;
                    return 0;
                }
            }

            KMetadataRelease ( meta );
        }
    }
    
    return rc;
}

LIB_EXPORT rc_t CC KDatabaseOpenMetadataUpdate ( KDatabase *self, KMetadata **metap )
{
    rc_t rc;
    KMetadata *meta;

    if ( metap == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcParam, rcNull );

    * metap = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcSelf, rcNull );

    if ( self -> read_only )
        return RC ( rcDB, rcDatabase, rcOpening, rcDatabase, rcReadonly );

    rc = KDBManagerOpenMetadataUpdateInt ( self -> mgr, & meta, self -> dir, self -> md5 );
    if ( rc == 0 )
    {
        meta -> db = KDatabaseAttach ( self );
        * metap = meta;
    }

    return rc;
}

LIB_EXPORT rc_t CC KTableOpenMetadataUpdate ( KTable *self, KMetadata **metap )
{
    rc_t rc;
    KMetadata *meta;

    if ( metap == NULL )
        return RC ( rcDB, rcTable, rcOpening, rcParam, rcNull );

    * metap = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcTable, rcOpening, rcSelf, rcNull );

    if ( self -> read_only )
        return RC ( rcDB, rcTable, rcOpening, rcTable, rcReadonly );

    rc = KDBManagerOpenMetadataUpdateInt ( self -> mgr, & meta, self -> dir, self -> md5 );
    if ( rc == 0 )
    {
        meta -> tbl = KTableAttach ( self );
        * metap = meta;
    }

    return rc;
}

LIB_EXPORT rc_t CC KColumnOpenMetadataUpdate ( KColumn *self, KMetadata **metap )
{
    rc_t rc;
    KMetadata *meta;

    if ( metap == NULL )
        return RC ( rcDB, rcColumn, rcOpening, rcParam, rcNull );

    * metap = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcColumn, rcOpening, rcSelf, rcNull );

    if ( self -> read_only )
        return RC ( rcDB, rcColumn, rcOpening, rcColumn, rcReadonly );

    rc = KDBManagerOpenMetadataUpdateInt ( self -> mgr, & meta, self -> dir, self -> md5 );
    if ( rc == 0 )
    {
        meta -> col = KColumnAttach ( self );
        * metap = meta;
    }

    return rc;
}


/* Version
 *  returns the metadata format version
 */
LIB_EXPORT rc_t CC KMetadataVersion ( const KMetadata *self, uint32_t *version )
{
    if ( version == NULL )
        return RC ( rcDB, rcMetadata, rcAccessing, rcParam, rcNull );

    if ( self == NULL )
    {
        * version = 0;
        return RC ( rcDB, rcMetadata, rcAccessing, rcSelf, rcNull );
    }

    * version = self -> vers;
    return 0;
}


/* ByteOrder
 *  indicates whether original byte order is reversed
 *  under current architecture.
 *
 *  the byte order of the column is established by
 *  the host architecture when created.
 *
 *  "reversed" [ OUT ] - if true, the original byte
 *  order is reversed with regard to host native byte order.
 */
LIB_EXPORT rc_t CC KMetadataByteOrder ( const KMetadata *self, bool *reversed )
{
    if ( reversed == NULL )
        return RC ( rcDB, rcMetadata, rcAccessing, rcParam, rcNull );

    if ( self == NULL )
    {
        * reversed = false;
        return RC ( rcDB, rcMetadata, rcAccessing, rcSelf, rcNull );
    }

    * reversed = self -> byteswap;
    return 0;
}


/* Revision
 *  returns current revision number
 *  where 0 ( zero ) means tip
 */
LIB_EXPORT rc_t CC KMetadataRevision ( const KMetadata *self, uint32_t *revision )
{
    if ( revision == NULL )
        return RC ( rcDB, rcMetadata, rcAccessing, rcParam, rcNull );

    if ( self == NULL )
    {
        * revision = 0;
        return RC ( rcDB, rcMetadata, rcAccessing, rcSelf, rcNull );
    }

    * revision = self -> rev;
    return 0;
}


/* MaxRevision
 *  returns the maximum revision available
 */
LIB_EXPORT rc_t CC KMetadataMaxRevision ( const KMetadata *self, uint32_t *revision )
{
    rc_t rc;
    KNamelist *listing;

    if ( revision == NULL )
        return RC ( rcDB, rcMetadata, rcAccessing, rcParam, rcNull );

    * revision = 0;

    if ( self == NULL )
        return RC ( rcDB, rcMetadata, rcAccessing, rcSelf, rcNull );

    rc = KDirectoryList ( self -> dir,
        & listing, NULL, NULL, "md" );
    if ( rc == 0 )
    {
        uint32_t rev_max, count;
        rc = KNamelistCount ( listing, & count );
        if ( rc == 0 )
        {
            uint32_t idx;
            for ( rev_max = idx = 0; idx < count; ++ idx )
            {
                const char *name;
                    
                rc = KNamelistGet ( listing, idx, & name );
                if ( rc != 0 )
                    break;
                    
                if ( name [ 0 ] == 'r' )
                {
                    char *end;
                    uint32_t rev = strtou32 ( name + 1, & end, 10 );
                    if ( end [ 0 ] == 0 && rev > rev_max )
                        rev_max = rev;
                }
            }

            * revision = rev_max;
        }
                
        KNamelistRelease ( listing );
    }
    else if ( GetRCState ( rc ) == rcNotFound )
    {
        rc = 0;
    }

    return rc;
}


/* Commit
 *  ensure any changes are committed to disk
 */
LIB_EXPORT rc_t CC KMetadataCommit ( KMetadata *self )
{
    rc_t rc;

    if ( self == NULL )
        return RC ( rcDB, rcMetadata, rcCommitting, rcSelf, rcNull );

    /* only flush if dirty */
    if ( ! self -> dirty )
        return 0;

    /* if this was a version 1 file,
       first freeze it */
    if ( self -> vers == 1 )
    {
        rc = KMetadataFreeze ( self );
        if ( rc != 0 )
            return rc;
    }

    /* flush it */
    rc = KMetadataFlush ( self );
    if ( rc == 0 )
        self -> dirty = false;

    return rc;
}


/* Freeze
 *  freezes current metadata revision
 *  further modification will begin on a copy
 */
LIB_EXPORT rc_t CC KMetadataFreeze ( KMetadata *self )
{
    rc_t rc;
    uint32_t rev_max;

    if ( self == NULL )
        return RC ( rcDB, rcMetadata, rcFreezing, rcSelf, rcNull );

    if ( self -> read_only )
        return RC ( rcDB, rcMetadata, rcFreezing, rcMetadata, rcReadonly );

    switch ( KDirectoryVPathType ( self -> dir, "md/cur", NULL ) )
    {
    case kptFile:
        break;
    case kptFile | kptAlias:
        return 0;
    default:
        return RC ( rcDB, rcMetadata, rcFreezing, rcPath, rcInvalid );
    }

    /* find max revision */
    rc = KMetadataMaxRevision ( self, & rev_max );
    if ( rc == 0 )
    {
        int len;
        char revision [ 16 ];	/* nice even stack (could have been 8) */

        /* rename tip to new version */
        len = snprintf ( revision, sizeof revision, "md/r%.3u", rev_max + 1 );
        if ( len < 0 || len >= sizeof revision )
            return RC ( rcDB, rcMetadata, rcFreezing, rcPath, rcInvalid );

        rc = KDirectoryRename ( self -> dir, false, "md/cur", revision );
        if ( rc == 0 )
        {
            /* now create link to this guy */
            rc = KDirectoryCreateAlias ( self -> dir,
                0777, kcmCreate, revision, "md/cur" );

            if ( rc == 0 )
            {
                if ( self -> md5 != NULL )
                {
                    rc = KMD5SumFmtRename ( self -> md5, "md/cur", revision );
                    if ( rc != 0 )	/* attempt to recover */
                        KDirectoryRemove ( self -> dir, false, "md/cur" );
                }
            }
            if ( rc != 0 )                /* attempt to recover */
                KDirectoryRename ( self -> dir, false, revision, "md/cur" );
        }
    }
        
    return rc;
}


/* OpenRevision
 *  opens a read-only indexed revision of metadata
 */
LIB_EXPORT rc_t CC KMetadataOpenRevision ( const KMetadata *self,
    const KMetadata **metap, uint32_t revision )
{
    rc_t rc;
    const KMetadata *meta;
    bool  meta_is_cached;

    if ( metap == NULL )
        return RC ( rcDB, rcMetadata, rcOpening, rcParam, rcNull );

    * metap = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcMetadata, rcOpening, rcSelf, rcNull );

    rc = KDBManagerOpenMetadataReadInt ( self -> mgr,
        & meta, self -> dir, revision, false, &meta_is_cached  );
    if ( rc == 0 )
    {
        if(!meta_is_cached)
	{
	    if ( self -> db != NULL )
                ((KMetadata*)meta) -> db = KDatabaseAttach ( self -> db );
            else if ( self -> tbl != NULL )
                ((KMetadata*)meta) -> tbl = KTableAttach ( self -> tbl );
            else if ( self -> col != NULL )
                ((KMetadata*)meta) -> col = KColumnAttach ( self -> col );
	}

        * metap = meta;
    }

    return rc;
}


/* GetSequence
 * SetSequence
 * NextSequence
 *  access a named sequence
 *
 *  "seq" [ IN ] - NUL terminated sequence name
 *
 *  "val" [ OUT ] - return parameter for sequence value
 *  "val" [ IN ] - new sequence value
 */
LIB_EXPORT rc_t CC KMetadataGetSequence ( const KMetadata *self,
    const char *seq, int64_t *val )
{
    rc_t rc;
    const KMDataNode *found;

    if ( val == NULL )
        return RC ( rcDB, rcMetadata, rcAccessing, rcParam, rcNull );
    * val = 0;

    if ( self == NULL )
        return RC ( rcDB, rcMetadata, rcAccessing, rcSelf, rcNull );
    if ( seq == NULL )
        return RC ( rcDB, rcMetadata, rcAccessing, rcPath, rcNull );
    if ( seq [ 0 ] == 0 )
        return RC ( rcDB, rcMetadata, rcAccessing, rcPath, rcInvalid );

    rc = KMDataNodeOpenNodeRead ( self -> root, & found, ".seq/%s", seq );
    if ( rc == 0 )
    {
        rc = KMDataNodeReadB64 ( found, val );
        KMDataNodeRelease ( found );
    }
    
    return rc;
}

LIB_EXPORT rc_t CC KMetadataSetSequence ( KMetadata *self,
    const char *seq, int64_t val )
{
    rc_t rc;
    KMDataNode *found;

    if ( self == NULL )
        return RC ( rcDB, rcMetadata, rcUpdating, rcSelf, rcNull );
    if ( seq == NULL )
        return RC ( rcDB, rcMetadata, rcUpdating, rcPath, rcNull );
    if ( seq [ 0 ] == 0 )
        return RC ( rcDB, rcMetadata, rcUpdating, rcPath, rcInvalid );

    rc = KMDataNodeOpenNodeUpdate ( self -> root, & found, ".seq/%s", seq );
    if ( rc == 0 )
    {
        rc = KMDataNodeWriteB64 ( found, & val );
        KMDataNodeRelease ( found );
    }
        
    return rc;
}

LIB_EXPORT rc_t CC KMetadataNextSequence ( KMetadata *self,
    const char *seq, int64_t *val )
{
    rc_t rc;
    KMDataNode *found;

    if ( val == NULL )
        return RC ( rcDB, rcMetadata, rcUpdating, rcParam, rcNull );
    * val = 0;

    if ( self == NULL )
        return RC ( rcDB, rcMetadata, rcUpdating, rcSelf, rcNull );
    if ( seq == NULL )
        return RC ( rcDB, rcMetadata, rcUpdating, rcPath, rcNull );
    if ( seq [ 0 ] == 0 )
        return RC ( rcDB, rcMetadata, rcUpdating, rcPath, rcInvalid );

    rc = KMDataNodeOpenNodeUpdate ( self -> root, & found, ".seq/%s", seq );
    if ( rc == 0 )
    {
        rc = KMDataNodeReadB64 ( found, val );
        if ( rc == 0 || GetRCState ( rc ) == rcIncomplete )
        {
            * val += 1;
            rc = KMDataNodeWriteB64 ( found, val );
        }

        KMDataNodeRelease ( found );
    }

    return rc;
}



/*--------------------------------------------------------------------------
 * KNamelist
 */
typedef struct KMDataNodeNamelist KMDataNodeNamelist;
struct KMDataNodeNamelist
{
    KNamelist dad;
    const KMDataNode *node;
    size_t count;
    const char *namelist [ 1 ];
};

/* Whack
 */
static
rc_t CC KMDataNodeNamelistWhack ( KMDataNodeNamelist *self )
{
    rc_t rc = KMDataNodeRelease ( self -> node );
    if ( rc == 0 )
        free ( self );
    return rc;
}

/* Count
 */
static
rc_t CC KMDataNodeNamelistCount ( const KMDataNodeNamelist *self, uint32_t *count )
{
    * count = ( uint32_t ) self -> count;
    return 0;
}

/* Get
 */
static
rc_t CC KMDataNodeNamelistGet ( const KMDataNodeNamelist *self,
    uint32_t idx, const char **name )
{
    if ( ( size_t ) idx >= self -> count )
        return RC ( rcDB, rcNamelist, rcAccessing, rcParam, rcExcessive );
    * name = self -> namelist [ idx ];
    return 0;
}

/* Make
 */
static KNamelist_vt_v1 vtKMDataNodeNamelist =
{
    /* version 1.0 */
    1, 0,

    /* start minor version 0 methods */
    KMDataNodeNamelistWhack,
    KMDataNodeNamelistCount,
    KMDataNodeNamelistGet
    /* end minor version 0 methods */
};

static
rc_t KMDataNodeNamelistMake ( KNamelist **names, unsigned int count )
{
    rc_t rc;
    KMDataNodeNamelist *self = calloc ( 1, sizeof * self -
        sizeof self -> namelist + count * sizeof self -> namelist [ 0 ] );
    if ( self == NULL )
        rc = RC ( rcDB, rcNamelist, rcConstructing, rcMemory, rcExhausted );
    else
    {
        rc = KNamelistInit ( & self -> dad,
            ( const KNamelist_vt* ) & vtKMDataNodeNamelist );
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
void CC KMDataNodeListCount ( BSTNode *n, void *data )
{
    * ( unsigned int* ) data += 1;
}

static
void CC KMDataNodeGrabAttr ( BSTNode *n, void *data )
{
    KMDataNodeNamelist *list = data;
    list -> namelist [ list -> count ++ ] = ( ( const KMAttrNode* ) n ) -> name;
}

LIB_EXPORT rc_t CC KMDataNodeListAttr ( const KMDataNode *self, KNamelist **names )
{
    rc_t rc;
    unsigned int count;

    if ( names == NULL )
        return RC ( rcDB, rcNode, rcListing, rcParam, rcNull );
    * names = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcNode, rcListing, rcSelf, rcNull );

    count = 0;
    BSTreeForEach ( & self -> attr, 0, KMDataNodeListCount, & count );

    rc = KMDataNodeNamelistMake ( names, count );
    if ( rc == 0 )
        BSTreeForEach ( & self -> attr, false, KMDataNodeGrabAttr, * names );

    return rc;
}

static
void CC KMDataNodeGrabName ( BSTNode *n, void *data )
{
    KMDataNodeNamelist *list = data;
    list -> namelist [ list -> count ++ ] = ( ( const KMDataNode* ) n ) -> name;
}

LIB_EXPORT rc_t CC KMDataNodeListChildren ( const KMDataNode *self, KNamelist **names )
{
    rc_t rc;
    unsigned int count;

    if ( names == NULL )
        return RC ( rcDB, rcNode, rcListing, rcParam, rcNull );
    * names = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcNode, rcListing, rcSelf, rcNull );

    count = 0;
    BSTreeForEach ( & self -> child, 0, KMDataNodeListCount, & count );

    rc = KMDataNodeNamelistMake ( names, count );
    if ( rc == 0 )
        BSTreeForEach ( & self -> child, false, KMDataNodeGrabName, * names );

    return rc;
}
