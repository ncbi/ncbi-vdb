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

#include <kdb/extern.h>

#define TRACK_REFERENCES 0

struct KWMDataNodeNamelist;
#define KNAMELIST_IMPL struct KWMDataNodeNamelist

#include "wmeta.h"
#include "wmetadatanode.h"

#include "wkdb.h"
#include "wdbmgr.h"
#include "database-cmn.h"
#include "wtable.h"
#include "wcolumn.h"
#include "kdbfmt.h"

#include <kdb/meta.h>
#include <kdb/namelist.h>
#include <kdb/kdb-priv.h>

#include <klib/container.h>
#include <klib/pbstree.h>
#include <klib/namelist.h>
#include <klib/impl.h>
#include <klib/symbol.h>
#include <klib/refcount.h>
#include <klib/debug.h>
#include <klib/log.h>
#include <klib/rc.h>

#include <kfs/file.h>
#include <kfs/mmap.h>
#include <kfs/md5.h>

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

/*--------------------------------------------------------------------------
 * KWMAttrNode
 */

int64_t CC KWMAttrNodeCmp ( const void *item, const BSTNode *n )
{
#define a ( ( const char* ) item )
#define b ( ( const KWMAttrNode* ) n )

    return strcmp ( a, b -> name );

#undef a
#undef b
}

int64_t CC KWMAttrNodeSort ( const BSTNode *item, const BSTNode *n )
{
#define a ( ( const KWMAttrNode* ) item )
#define b ( ( const KWMAttrNode* ) n )

    return strcmp ( a -> name, b -> name );

#undef a
#undef b
}

void CC KWMAttrNodeWhack ( BSTNode *n, void *data )
{
    free ( n );
}

static
bool CC KWMAttrNodeInflate ( PBSTNode *n, void *data )
{
    KWMAttrNode *b;
    KWMDataNodeInflateData *pb = data;

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
    BSTreeInsert ( pb -> bst, & b -> n, KWMAttrNodeSort );
    return false;
}

rc_t KWMAttrNodeRename ( const KWMAttrNode *self,
    KWMAttrNode **renamed, const char *name )
{
    size_t size = strlen ( name );
    KWMAttrNode *b = malloc ( sizeof * b + self -> vsize + size );
    if ( b == NULL )
        return RC ( rcDB, rcNode, rcRenaming, rcMemory, rcExhausted );

    b -> value = & b -> name [ 1 + size ];
    b -> vsize = self -> vsize;
    strcpy ( b -> name, name );
    memmove ( b -> value, self -> value, self -> vsize );
    * renamed = b;
    return 0;
}

rc_t KWMAttrNodeMake ( KWMAttrNode **np,
    const char *name, const void *value, size_t vsize )
{
    size_t size = strlen ( name );
    KWMAttrNode *n = malloc ( sizeof * n + vsize + size );
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

static rc_t KWMAttrNodeMakeCopy(KWMAttrNode **node, KWMAttrNode const *source)
{
    size_t const size = &((uint8_t const *)source->value)[source->vsize] - ((uint8_t const *)source);
    KWMAttrNode *result = malloc(size);
    if (result == NULL)
        return RC ( rcDB, rcNode, rcConstructing, rcMemory, rcExhausted );

    memmove(result, source, size);
    memset(&result->n, 0, sizeof(result->n));
    *node = result;
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

static rc_t CC KWMDataNodeWhack ( KMDataNode *cself );
static rc_t CC KWMDataNodeByteOrder ( const KMDataNode *self, bool *reversed );
static rc_t CC KWMDataNodeRead ( const KMDataNode *self, size_t offset, void *buffer, size_t bsize, size_t *num_read, size_t *remaining );
static rc_t CC KWMDataNodeVOpenNodeRead ( const KMDataNode *self, const KMDataNode **node, const char *path, va_list args );
static rc_t CC KWMDataNodeReadAttr ( const KMDataNode *self, const char *name, char *buffer, size_t bsize, size_t *size );
static rc_t CC KWMDataNodeCompare( const KMDataNode *self, KMDataNode const *other, bool *equal );
static rc_t CC KWMDataNodeAddr ( const KMDataNode *self, const void **addr, size_t *size );
static rc_t CC KWMDataNodeListAttr ( const KMDataNode *self, KNamelist **names );
static rc_t CC KWMDataNodeListChildren ( const KMDataNode *self, KNamelist **names );

static KMDataNode_vt KWMDataNode_vt =
{
    KWMDataNodeWhack,
    KMDataNodeBaseAddRef,
    KMDataNodeBaseRelease,
    KWMDataNodeByteOrder,
    KWMDataNodeRead,
    KWMDataNodeVOpenNodeRead,
    KWMDataNodeReadAttr,
    KWMDataNodeCompare,
    KWMDataNodeAddr,
    KWMDataNodeListAttr,
    KWMDataNodeListChildren
};

#define CAST() assert( bself->vt == &KWMDataNode_vt ); KWMDataNode * self = (KWMDataNode *)bself

rc_t
KWMDataNodeMakeRoot( KWMDataNode ** node, KWMetadata *meta )
{
    assert( node != NULL );
    KWMDataNode * ret = calloc ( 1, sizeof *ret );
    if ( ret == NULL )
        return RC ( rcDB, rcMetadata, rcConstructing, rcMemory, rcExhausted );
    ret -> dad . vt = & KWMDataNode_vt;
    KRefcountInit ( & ret -> dad . refcount, 1, "KWMDataNode", "make-read", "/" );
    ret -> meta = meta;
    *node = ret;

    return 0;
}

static
int64_t CC KWMDataNodeCmp ( const void *item, const BSTNode *n )
{
#define a ( ( const char* ) item )
#define b ( ( const KWMDataNode* ) n )

    return strcmp ( a, b -> name );

#undef a
#undef b
}

static
int64_t CC KWMDataNodeSort ( const BSTNode *item, const BSTNode *n )
{
#define a ( ( const KWMDataNode* ) item )
#define b ( ( const KWMDataNode* ) n )

    return strcmp ( a -> name, b -> name );

#undef a
#undef b
}

static
void
DataNodeWhack ( BSTNode *n, void *data )
{
    KMDataNodeRelease( (KMDataNode *)n );
}

static
rc_t CC
KWMDataNodeWhack ( KMDataNode *bself )
{
    CAST();

    BSTreeWhack ( & self -> attr, KWMAttrNodeWhack, NULL );
    BSTreeWhack ( & self -> child, DataNodeWhack, NULL );
    free ( self -> value );
    return KMDataNodeBaseWhack ( bself );
}

/* Inflate
 */
bool CC KWMDataNodeInflate_v1 ( PBSTNode *n, void *data )
{
    void *value;
    KWMDataNode *b;
    KWMDataNodeInflateData *pb = data;

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
    b -> dad . vt = & KWMDataNode_vt;
    b -> par = pb -> par;
    b -> meta = pb -> meta;
    b -> value = ( void* ) ( name + size + 1 );
    b -> vsize = n -> data . size - size - 1;
    BSTreeInit ( & b -> attr );
    BSTreeInit ( & b -> child );
    KRefcountInit ( & b -> dad . refcount, 1, "KMDataNode", "inflate", name );

    b -> read_only = 0;
    strcpy ( b -> name, name );

    /* a name with no associated value */
    if ( b -> vsize == 0 )
    {
        b -> value = NULL;
        BSTreeInsert ( pb -> bst, & b -> dad . n, KWMDataNodeSort );
        return false;
    }

    /* allocate value because v2+ code expects this */
    value = malloc ( b -> vsize );
    if ( value != NULL )
    {
        memmove ( value, b -> value, b -> vsize );
        b -> value = value;
        BSTreeInsert ( pb -> bst, & b -> dad . n, KWMDataNodeSort );
        return false;
    }

    pb -> rc = RC ( rcDB, rcMetadata, rcConstructing, rcMemory, rcExhausted );
    free ( b );
    return true;
}

static
rc_t KWMDataNodeInflateAttr ( KWMDataNode *n, bool byteswap )
{
    PBSTree *bst;
    rc_t rc = PBSTreeMake ( & bst, n -> value, n -> vsize, byteswap );
    if ( rc != 0 )
        rc = RC ( rcDB, rcMetadata, rcConstructing, rcData, rcCorrupt );
    else
    {
        KWMDataNodeInflateData pb;
        size_t bst_size = PBSTreeSize ( bst );

        pb . meta = n -> meta;
        pb . par = n;
        pb . bst = & n -> attr;
        pb . node_size_limit = 0;
        pb . node_child_limit = 0;
        pb . rc = 0;
        pb . byteswap = byteswap;
        PBSTreeDoUntil ( bst, 0, KWMAttrNodeInflate, & pb );
        rc = pb . rc;

        PBSTreeWhack ( bst );

        n -> value = ( char* ) n -> value + bst_size;
        n -> vsize -= bst_size;
    }
    return rc;
}

static
rc_t KWMDataNodeInflateChild ( KWMDataNode *n,
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
            KWMDataNodeInflateData pb;

            pb . meta = n -> meta;
            pb . par = n;
            pb . bst = & n -> child;
            pb . node_size_limit = node_size_limit;
            pb . node_child_limit = node_child_limit;
            pb . rc = 0;
            pb . byteswap = byteswap;
            PBSTreeDoUntil ( bst, 0, KWMDataNodeInflate, & pb );
            rc = pb . rc;
        }

        PBSTreeWhack ( bst );

        n -> value = ( char* ) n -> value + bst_size;
        n -> vsize -= bst_size;
    }
    return rc;
}

bool CC
KWMDataNodeInflate ( PBSTNode *n, void *data )
{
    KWMDataNode *b;
    KWMDataNodeInflateData *pb = data;

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

    b -> dad . vt = & KWMDataNode_vt;
    b -> par = pb -> par;
    b -> meta = pb -> meta;
    b -> value = ( void* ) ( name + size );
    b -> vsize = n -> data . size - size - 1;
    BSTreeInit ( & b -> attr );
    BSTreeInit ( & b -> child );
    memmove ( b -> name, name, size );
    b -> name [ size ] = 0;
    KRefcountInit ( & b -> dad . refcount, 1, "KMDataNode", "inflate", b -> name );
    b -> read_only = 0;

    pb -> rc = ( bits & 1 ) != 0 ? KWMDataNodeInflateAttr ( b, pb -> byteswap ) : 0;
    if ( pb -> rc == 0 )
    {
        pb -> rc = ( bits & 2 ) != 0 ?
            KWMDataNodeInflateChild ( b, pb -> node_size_limit, pb -> node_child_limit, pb -> byteswap ) : 0;
        if ( pb -> rc == 0 )
        {
            void *value;

            if ( b -> vsize == 0 )
            {
                b -> value = NULL;
                BSTreeInsert ( pb -> bst, & b -> dad . n, KWMDataNodeSort );
                return false;
            }

            value = malloc ( b -> vsize );
            if ( value != NULL )
            {
                memmove ( value, b -> value, b -> vsize );
                b -> value = value;
                BSTreeInsert ( pb -> bst, & b -> dad . n, KWMDataNodeSort );
                return false;
            }
            pb -> rc = RC ( rcDB, rcMetadata, rcConstructing, rcMemory, rcExhausted );

            BSTreeWhack ( & b -> child, DataNodeWhack, NULL );
        }

        BSTreeWhack ( & b -> attr, KWMAttrNodeWhack, NULL );
    }

    free ( b );
    return true;
}


/* Find
 */
rc_t KWMDataNodeFind ( const KWMDataNode *cself, KWMDataNode **np, char **path )
{
    KWMDataNode *found, *self= ( KWMDataNode* ) cself;

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
        found = ( KWMDataNode* ) BSTreeFind ( & self -> child, name, KWMDataNodeCmp );
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
rc_t KWMDataNodeMake ( KWMDataNode *self, KWMDataNode **np, char *name )
{
    rc_t rc;
    char *end;

    /* create nodes */
    for ( ; name != NULL; name = end )
    {
        BSTNode *x;
        size_t size;
        KWMDataNode *n;

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
        n -> dad . vt = & KWMDataNode_vt;
        n -> par = self;
        n -> meta = self -> meta;
        n -> value = NULL;
        n -> vsize = 0;
        BSTreeInit ( & n -> attr );
        BSTreeInit ( & n -> child );
        strcpy ( n -> name, name );
        KRefcountInit ( & n -> dad . refcount, 1, "KWMDataNode", "make", n -> name );
        n -> read_only = 0;

        /* insert */
        rc = BSTreeInsertUnique ( & self -> child,
            & n -> dad . n, & x, KWMDataNodeSort );
        if ( rc != 0 )
        {
            free ( n );
            n = ( KWMDataNode* ) x;
        }

        /* transition */
        self = n;
    }

    /* the current self is our guy */
    * np = self;
    return 0;
}

static
rc_t CC
KWMDataNodeVOpenNodeRead ( const KMDataNode *bself, const KMDataNode **node, const char *path, va_list args )
{
    CAST();

    rc_t rc;
    KWMDataNode *found;
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
        /* VDB-4386: cannot treat va_list as a pointer! */
        /*if ( args == NULL )
            len = snprintf ( full, sizeof full, "%s", path );
        else*/
            len = vsnprintf ( full, sizeof full, path, args );
        if ( len < 0 || len >= sizeof full )
            return RC ( rcDB, rcNode, rcOpening, rcPath, rcExcessive );
    }

    rc = KWMDataNodeFind ( self, & found, & p );
    if ( rc == 0 )
    {
        /* check if the node is not open */
        if ( atomic32_read ( & found -> dad . refcount ) == 1 )
        {
            /* mark as read-only, since we're the first to open */
            found -> read_only = 1;
        }
        /* disallow open if already open for write */
        else if ( ! found -> read_only )
        {
            return RC ( rcDB, rcNode, rcOpening, rcNode, rcBusy );
        }

        KMDataNodeAddRef ( & found -> dad );
        * node = & found -> dad;
    }

    DBGMSG(DBG_KDB, DBG_FLAG(DBG_KDB_KDB),
            ("KMDataNodeVOpenNodeRead(%s) = %d\n", full, rc));

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

LIB_EXPORT
rc_t CC
KMDataNodeVOpenNodeUpdate ( KMDataNode *bself,
    KMDataNode **node, const char *path, va_list args )
{
    CAST();

    rc_t rc = 0;
    KWMDataNode *found;
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
        /* VDB-4386: cannot treat va_list as a pointer! */
        /*if ( args == NULL )
            len = snprintf ( full, sizeof full, "%s", path );
        else*/
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
    rc = KWMDataNodeFind ( self, & found, & p );
    if ( rc == 0 )
    {
        if ( atomic32_read ( & found -> dad . refcount ) != 1 )
            return RC ( rcDB, rcNode, rcOpening, rcNode, rcBusy );
    }
    else if ( GetRCState ( rc ) == rcNotFound )
    {
        rc = KWMDataNodeMake ( found, & found, p );
        if ( rc != 0 )
            return rc;
    }
    else
    {
        return rc;
    }

    KMDataNodeAddRef ( & found -> dad );
    found -> read_only = false;
    * node = & found -> dad;

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
static
rc_t CC
KWMDataNodeByteOrder ( const KMDataNode *bself, bool *reversed )
{
    CAST();
    return KMetadataByteOrder ( & self -> meta -> dad, reversed );
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
static
rc_t CC
KWMDataNodeRead ( const KMDataNode *bself,
    size_t offset, void *buffer, size_t bsize,
    size_t *num_read, size_t *remaining )
{
    CAST();

    rc_t rc;

    size_t dummy;
    if ( remaining == NULL )
        remaining = & dummy;

    if ( num_read == NULL )
        rc = RC ( rcDB, rcNode, rcReading, rcParam, rcNull );
    else
    {
        if ( buffer == NULL && bsize != 0 )
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
static
rc_t CC
KWMDataNodeAddr ( const KMDataNode *bself, const void **addr, size_t *size )
{
    CAST();

    rc_t rc;

    size_t dummy;
    if ( size == NULL )
        size = & dummy;

    if ( addr == NULL )
        rc = RC ( rcDB, rcMetadata, rcReading, rcParam, rcNull );
    else
    {
        * addr = self -> value;
        * size = self -> vsize;

        return 0;
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
LIB_EXPORT
rc_t CC
KMDataNodeWrite ( KMDataNode *bself, const void *buffer, size_t size )
{
    CAST();

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
LIB_EXPORT
rc_t CC
KMDataNodeAppend ( KMDataNode *bself, const void *buffer, size_t size )
{
    CAST();

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
static
rc_t CC
KWMDataNodeReadAttr ( const KMDataNode *bself, const char *name, char *buffer, size_t bsize, size_t *size )
{
    CAST();

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
            const KWMAttrNode *n = ( const KWMAttrNode* )
                BSTreeFind ( & self -> attr, name, KWMAttrNodeCmp );
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
LIB_EXPORT
rc_t CC
KMDataNodeWriteAttr ( KMDataNode *bself, const char *name, const char *value )
{
    CAST();

    rc_t rc;
    size_t size;
    KWMAttrNode *n;

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
    rc = KWMAttrNodeMake ( & n, name, value, size );
    if ( rc == 0 )
    {
        BSTNode *x;
        if ( BSTreeInsertUnique ( & self -> attr,
             & n -> n, & x, KWMAttrNodeSort ) != 0 )
        {
            BSTreeUnlink ( & self -> attr, x );
            KWMAttrNodeWhack ( x, NULL );
            BSTreeInsert ( & self -> attr, & n -> n, KWMAttrNodeSort );
        }

        self -> meta -> dirty = true;
    }
    return rc;
}

/* Drop
 * VDrop
 *  drop some or all node content
 */
rc_t KWMDataNodeDropAll_int ( KWMDataNode *self )
{
    BSTreeWhack ( & self -> attr, KWMAttrNodeWhack, NULL );
    BSTreeInit ( & self -> attr );

    BSTreeWhack ( & self -> child, DataNodeWhack, NULL );
    BSTreeInit ( & self -> child );

    free ( self -> value );
    self -> value = NULL;
    self -> vsize = 0;
    self -> meta -> dirty = true;

    return 0;
}

LIB_EXPORT
rc_t CC
KMDataNodeDropAll ( KMDataNode *bself )
{
    CAST();

    if ( self == NULL )
        return RC ( rcDB, rcNode, rcClearing, rcSelf, rcNull );
    if ( self -> meta == NULL )
        return RC ( rcDB, rcNode, rcClearing, rcMetadata, rcNull );
    if ( self -> read_only )
        return RC ( rcDB, rcNode, rcClearing, rcNode, rcReadonly );

    return KWMDataNodeDropAll_int(self);
}

LIB_EXPORT
rc_t CC
KMDataNodeDropAttr ( KMDataNode *bself, const char *attr )
{
    CAST();

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
        BSTreeWhack ( & self -> attr, KWMAttrNodeWhack, NULL );
        BSTreeInit ( & self -> attr );
    }
    else
    {
        KWMAttrNode *found = ( KWMAttrNode* )
            BSTreeFind ( & self -> attr, attr, KWMAttrNodeCmp );
        if ( found == NULL )
            return RC ( rcDB, rcNode, rcUpdating, rcAttr, rcNotFound );

        BSTreeUnlink ( & self -> attr, & found -> n );
        KWMAttrNodeWhack ( & found -> n, NULL );
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

LIB_EXPORT
rc_t CC
KMDataNodeVDropChild ( KMDataNode *bself, const char *path, va_list args )
{
    CAST();

    int len = 0;
    rc_t rc;
    KWMDataNode *found;
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
    /* VDB-4386: cannot treat va_list as a pointer! */
    /*if ( args == NULL )
        len = snprintf ( full, sizeof full, "%s", path );
    else*/
    if ( path != NULL )
        len = vsnprintf ( full, sizeof full, path, args );
    if ( len < 0 || len >= sizeof full )
        return RC ( rcDB, rcNode, rcUpdating, rcPath, rcExcessive );

    /* don't allow update when open for read */
    if ( self -> read_only )
        return RC ( rcDB, rcNode, rcUpdating, rcNode, rcReadonly );

    rc = KWMDataNodeFind ( self, & found, & p );
    if ( GetRCState ( rc ) == rcNotFound )
    {
        if ( p [ 0 ] != '*' || p [ 1 ] != 0 )
            return rc;

        BSTreeWhack ( & found -> child, DataNodeWhack, NULL );
        BSTreeInit ( & found -> child );
    }
    else
    {
        BSTreeUnlink ( & self -> child, & found -> dad . n );
        KMDataNodeRelease ( & found -> dad );
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
LIB_EXPORT
rc_t CC
KMDataNodeRenameAttr ( KMDataNode *bself, const char *from, const char *to )
{
    CAST();

    rc_t rc;
    KWMAttrNode *found, *renamed;

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

    found = ( KWMAttrNode* )
        BSTreeFind ( & self -> attr, from, KWMAttrNodeCmp );
    if ( found == NULL )
        return RC ( rcDB, rcNode, rcRenaming, rcAttr, rcNotFound );

    rc = KWMAttrNodeRename ( found, & renamed, to );
    if ( rc == 0 )
    {
        BSTNode *x;
        rc = BSTreeInsertUnique ( & self -> attr,
            & renamed -> n, & x, KWMAttrNodeSort );
        if ( rc != 0 )
        {
            KWMAttrNodeWhack ( & renamed -> n, NULL );
            if ( x == & found -> n )
                return 0;
            return RC ( rcDB, rcNode, rcRenaming, rcAttr, rcExists );
        }

        BSTreeUnlink ( & self -> attr, & found -> n );
        KWMAttrNodeWhack ( & found -> n, NULL );
        self -> meta -> dirty = true;
    }

    return rc;
}

static
void CC KWMDataNodeRelinkParent ( BSTNode *n, void *data )
{
    ( ( KWMDataNode* ) n ) -> par = data;
}

LIB_EXPORT
rc_t CC
KMDataNodeRenameChild ( KMDataNode *bself, const char *from, const char *to )
{
    CAST();

    int len;
    rc_t rc;
    KWMDataNode *found;
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

    rc = KWMDataNodeFind ( self, & found, & p );
    if ( rc == 0 )
    {
        KWMDataNode *renamed;

        if ( atomic32_read ( & found -> dad . refcount ) != 1 )
            return RC ( rcDB, rcNode, rcRenaming, rcNode, rcBusy );

        len = snprintf ( p = full, sizeof full, "%s", to );
        if ( len < 0 || len >= sizeof full )
            return RC ( rcDB, rcNode, rcRenaming, rcPath, rcExcessive );

        rc = KWMDataNodeFind ( self, & renamed, & p );
        if ( GetRCState ( rc ) == rcNotFound )
        {
            rc = KWMDataNodeMake ( renamed, & renamed, p );
            if ( rc == 0 )
            {
                BSTreeUnlink ( & found -> par -> child, & found -> dad . n );

                renamed -> attr = found -> attr;
                BSTreeInit ( & found -> attr );

                renamed -> child = found -> child;
                BSTreeInit ( & found -> child );
                BSTreeForEach ( & renamed -> child,
                    false, KWMDataNodeRelinkParent, renamed );

                renamed -> value = found -> value;
                renamed -> vsize = found -> vsize;
                found -> value = NULL;
                found -> vsize = 0;

                KMDataNodeRelease ( & found -> dad );
                self -> meta -> dirty = true;
            }
        }
        else if ( rc == 0 )
            rc = RC ( rcDB, rcNode, rcRenaming, rcNode, rcExists );
    }

    return rc;
}

/*--------------------------------------------------------------------------
 * deep copy of a Metadata-node
 */

struct CopyContext {
    KWMDataNode *dest;
    KWMDataNode const *source;
    rc_t rc;
};
static void KWMDataNodeCopy_int(struct CopyContext *ctx);

static void KWMDataNodeCopyValue(struct CopyContext *ctx)
{
    if (ctx->rc == 0) {
        void const *data = NULL;
        size_t size = 0;

        ctx->rc = KMDataNodeAddr( &ctx->source->dad, &data, &size );
        if (ctx->rc == 0)
            ctx->rc = KMDataNodeWrite( &ctx->dest->dad, data, size);
    }
}

static void CC KWMDataNodeCopyAttribute_cb(BSTNode *n, void *data)
{
    struct CopyContext *const ctx = data;
    if (ctx->rc == 0) {
        KWMAttrNode *newAttr = NULL;
        ctx->rc = KWMAttrNodeMakeCopy(&newAttr, (KWMAttrNode const *)n);
        if (ctx->rc == 0)
            BSTreeInsert(&ctx->dest->attr, &newAttr->n, KWMAttrNodeSort);
    }
}

static void CC KWMDataNodeCopy_cb(BSTNode *n, void *data)
{
    struct CopyContext *const ctx = data;
    if (ctx->rc == 0) {
        char const *const nodeName = ((KWMDataNode const *)n)->name;
        KWMDataNode *const dest = ctx->dest;
        KWMDataNode const *const source = ctx->source;

        ctx->rc = KMDataNodeOpenNodeRead( & source -> dad, (const KMDataNode**)&ctx->source, nodeName);
        assert(ctx->rc == 0);
        if (ctx->rc == 0) {
            ctx->rc = KMDataNodeOpenNodeUpdate( & dest -> dad, (KMDataNode**)&ctx->dest, nodeName);
            if (ctx->rc == 0) {
                KWMDataNodeCopy_int(ctx);
                KMDataNodeRelease( & ctx->dest->dad );
            }
            KMDataNodeRelease( & ctx->source->dad );
        }
        ctx->source = source;
        ctx->dest = dest;
    }
}

static void KWMDataNodeCopy_int(struct CopyContext *ctx)
{
    KWMDataNodeCopyValue(ctx);
    BSTreeForEach(&ctx->source->attr, false, KWMDataNodeCopyAttribute_cb, ctx);
    BSTreeForEach(&ctx->source->child, false, KWMDataNodeCopy_cb, ctx);
    ctx->dest->meta->dirty = (ctx->rc == 0);
}

LIB_EXPORT rc_t CC KMDataNodeCopy(  KMDataNode *bself
                                  , KMDataNode const *bsource)
{
    CAST();
    const KWMDataNode *source = (const KWMDataNode *)bsource;

    struct CopyContext ctx;

    if (self == NULL)
        return RC(rcDB, rcNode, rcCopying, rcSelf, rcNull);
    if (self->read_only)
        return RC(rcDB, rcNode, rcCopying, rcNode, rcReadonly);
    if (source == NULL)
        return RC(rcDB, rcNode, rcCopying, rcParam, rcNull);
    if (self->meta == NULL || source->meta == NULL)
        return RC(rcDB, rcNode, rcCopying, rcMetadata, rcNull);

    ctx.rc = KWMDataNodeDropAll_int(self);
    ctx.dest = self;
    ctx.source = source;

    KWMDataNodeCopy_int(&ctx);

    return ctx.rc;
}

/*--------------------------------------------------------------------------
 * deep comparison of 2 Metadata-nodes
 */

/* >>>>>> !!! any changes here have to be duplicated in meta.c !!! <<<<<< */
static rc_t KWMDataNodeCompare_int( const KWMDataNode *self, const KWMDataNode *other, bool *equal );

/* >>>>>> !!! any changes here have to be duplicated in meta.c !!! <<<<<< */
static rc_t KWMDataNodeCompareValue( const KWMDataNode *self, const KWMDataNode *other, bool *equal ) {
    void const *self_data = NULL;
    size_t self_size = 0;
    rc_t rc = KMDataNodeAddr( & self -> dad, &self_data, &self_size );
    if ( 0 == rc ) {
        void const *other_data = NULL;
        size_t other_size = 0;
        rc = KMDataNodeAddr( & other -> dad, &other_data, &other_size );
        if ( 0 == rc ) {
            if ( self_size != other_size ) {
                *equal = false; /* if the sizes are different - they can't be equal... */
            } else if ( 0 == self_size ) {
                *equal = true; /* if both nodes have no data... */
            } else {
                int cmp = memcmp( self_data, other_data, self_size );
                *equal = ( 0 == cmp );
            }
        }
    }
    return rc;
}

/* >>>>>> !!! any changes here have to be duplicated in meta.c !!! <<<<<< */
struct CompareContext {
    KWMDataNode const *other;
    bool *equal;
    rc_t rc;
};

/* >>>>>> !!! any changes here have to be duplicated in meta.c !!! <<<<<< */
static void CC KWMDataNodeCompareChildren_cb( BSTNode *n, void *data ) {
    struct CompareContext *const ctx = data;
    if ( 0 == ctx -> rc && *( ctx -> equal ) ) {
        const KWMDataNode *child = ( const KWMDataNode * )n;
        const char * childName = child -> name;
        // find the corresponding child-node in ctx -> other
        const KWMDataNode *other_child;
        ctx -> rc = KMDataNodeOpenNodeRead( & ctx -> other -> dad, (const KMDataNode**)&other_child, childName );
        if ( 0 == ctx -> rc ) {
            ctx -> rc = KWMDataNodeCompare_int( child, other_child, ctx -> equal ); /* recursion here! */
            KMDataNodeRelease( & other_child -> dad );
        } else {
            *( ctx -> equal ) = false;
        }
    }
}

/* >>>>>> !!! any changes here have to be duplicated in meta.c !!! <<<<<< */
static void CC KWMDataNodeCompareAttr_cb( BSTNode *n, void *data ) {
    struct CompareContext *const ctx = data;
    if ( 0 == ctx -> rc && *( ctx -> equal ) ) {
        const KWMAttrNode *attr = ( const KWMAttrNode * )n;
        if ( NULL != attr ) {
            const char * attrName = attr -> name;
            const KWMAttrNode *other_attr = ( const KWMAttrNode* )BSTreeFind( &( ctx -> other -> attr ),
                                                                            attrName, KWMAttrNodeCmp );
            if ( NULL == other_attr ) {
                *( ctx -> equal ) = false; /* not found in other... */
            } else if ( attr -> vsize != other_attr -> vsize ) {
                *( ctx -> equal ) = false; /* if the sizes are different - they can't be equal... */
            } else if ( 0 == attr -> vsize ) {
                *( ctx -> equal ) = true; /* both attributes have no data... */
            } else {
                int cmp = memcmp( attr -> value, other_attr -> value, attr -> vsize );
                *( ctx -> equal ) = ( 0 == cmp );
            }
        }
    }
}

/* >>>>>> !!! any changes here have to be duplicated in meta.c !!! <<<<<< */
static rc_t KWMDataNodeCompare_int( const KWMDataNode *self, const KWMDataNode *other, bool *equal ) {
    /* first we compare the value of these 2 nodes */
    rc_t rc = KWMDataNodeCompareValue( self, other, equal );
    if ( 0 == rc && *equal ) {
        /* if they are equal, we compare all of the children of self with the children of other */
        struct CompareContext ctx = { other, equal, 0 };
        BSTreeForEach( &( self -> child ), false, KWMDataNodeCompareChildren_cb, &ctx );
        rc = ctx . rc;
        if ( 0 == rc && *equal ) {
            /* if they are equal we compare all attributes... */
            BSTreeForEach( &( self -> attr ), false, KWMDataNodeCompareAttr_cb, &ctx );
            rc = ctx .rc;
        }
    }
    return rc;
}

/* >>>>>> !!! any changes here have to be duplicated in meta.c !!! <<<<<< */
static
rc_t CC
KWMDataNodeCompare( const KMDataNode *bself, KMDataNode const *bother, bool *equal )
{
    CAST();
    assert( bother->vt == &KWMDataNode_vt );
    const KWMDataNode * other = (const KWMDataNode *)bother;

    rc_t rc = 0;
    if ( other == NULL || equal == NULL ) {
        rc = RC( rcDB, rcNode, rcComparing, rcParam, rcNull );
    } else if ( self -> meta == NULL && other -> meta == NULL ) {
        *equal = true;
    } else if ( self -> meta == NULL || other -> meta == NULL ) {
        *equal = false;
        rc = RC( rcDB, rcNode, rcComparing, rcMetadata, rcNull );
    } else {
        /* 2 way comparison, to make sure that both nodes have the same children... */
        rc =  KWMDataNodeCompare_int( self, other, equal );
        if ( 0 == rc && *equal ) {
            rc =  KWMDataNodeCompare_int( other, self, equal );
        }
    }
    return rc;
}


/*--------------------------------------------------------------------------
 * KNamelist
 */
typedef struct KWMDataNodeNamelist KWMDataNodeNamelist;
struct KWMDataNodeNamelist
{
    KNamelist dad;
    const KWMDataNode *node;
    size_t count;
    const char *namelist [ 1 ];
};

/* Whack
 */
static
rc_t CC KMDataNodeNamelistWhack ( KWMDataNodeNamelist *self )
{
    rc_t rc = KMDataNodeRelease ( & self -> node -> dad );
    if ( rc == 0 )
        free ( self );
    return rc;
}

/* Count
 */
static
rc_t CC KMDataNodeNamelistCount ( const KWMDataNodeNamelist *self, uint32_t *count )
{
    * count = ( uint32_t ) self -> count;
    return 0;
}

/* Get
 */
static
rc_t CC KMDataNodeNamelistGet ( const KWMDataNodeNamelist *self,
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
    KWMDataNodeNamelist *self = calloc ( 1, sizeof * self -
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
    KWMDataNodeNamelist *list = data;
    list -> namelist [ list -> count ++ ] = ( ( const KWMAttrNode* ) n ) -> name;
}

static
rc_t CC
KWMDataNodeListAttr ( const KMDataNode *bself, KNamelist **names )
{
    CAST();

    rc_t rc;
    unsigned int count;

    if ( names == NULL )
        return RC ( rcDB, rcNode, rcListing, rcParam, rcNull );
    * names = NULL;

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
    KWMDataNodeNamelist *list = data;
    list -> namelist [ list -> count ++ ] = ( ( const KWMDataNode* ) n ) -> name;
}

static
rc_t CC
KWMDataNodeListChildren ( const KMDataNode *bself, KNamelist **names )
{
    CAST();

    rc_t rc;
    unsigned int count;

    if ( names == NULL )
        return RC ( rcDB, rcNode, rcListing, rcParam, rcNull );
    * names = NULL;

    count = 0;
    BSTreeForEach ( & self -> child, 0, KMDataNodeListCount, & count );

    rc = KMDataNodeNamelistMake ( names, count );
    if ( rc == 0 )
        BSTreeForEach ( & self -> child, false, KMDataNodeGrabName, * names );

    return rc;
}
