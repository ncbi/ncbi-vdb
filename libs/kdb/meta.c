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

#include <kdb/extern.h>

struct KMDataNodeNamelist;
#define KNAMELIST_IMPL struct KMDataNodeNamelist

#define KONST const
#include "kdb-priv.h"
#include "dbmgr-priv.h"
#include "database-priv.h"
#include "table-priv.h"
#include "column-priv.h"
#include "kdbfmt-priv.h"
#undef KONST

#include <kdb/meta.h>
#include <kdb/namelist.h>
#include <klib/container.h>
#include <klib/pbstree.h>
#include <klib/namelist.h>
#include <klib/impl.h>
#include <kfs/file.h>
#include <kfs/mmap.h>
#include <klib/refcount.h>
#include <klib/debug.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <strtol.h>
#include <os-native.h>
#include <sysalloc.h>
#include <strtol.h>

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
    const KMetadata *meta;
    const KMDataNode *par;
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
    const KDirectory *dir;
    const KDBManager *mgr;

    /* owner */
    const KDatabase *db;
    const KTable *tbl;
    const KColumn *col;

    /* root node */
    KMDataNode *root;

    KRefcount refcount;
    uint32_t vers;
    uint32_t rev;
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
    const KMDataNode *par;
    const KMetadata *meta;
    void *value;
    size_t vsize;
    BSTree attr;
    BSTree child;
    KRefcount refcount;
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
                return KMetadataSever ( self -> meta );

            KRefcountWhack ( & self -> refcount, "KMDataNode" );

            BSTreeWhack ( & self -> attr, KMAttrNodeWhack, NULL );
            BSTreeWhack ( & self -> child, KMDataNodeWhack, NULL );
            free ( self -> value );
            free ( self );
            break;

        case krefNegative:
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
        pb . node_size_limit = NODE_SIZE_LIMIT;
        pb . node_child_limit = NODE_CHILD_LIMIT;
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
        if ( bst_count > node_child_limit )
        {
            PLOGMSG ( klogWarn, ( klogWarn,
                                  "refusing to inflate metadata node '$(node)' within file '$(path)': "
                                  "number of children ($(num_children)) exceeds limit ($(limit))."
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
                                  "refusing to inflate metadata node '$(node)' within file '$(path)': "
                                 "node size ($(node_size)) exceeds limit ($(limit))."
                                 , "node=%s,path=%s,node_size=%zu,limit=%zu"
                                 , n -> name
                                 , n -> meta -> path
                                 , bst_size
                                 , node_size_limit )
                );
        }
        else
        {
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
rc_t KMDataNodeFind ( const KMDataNode *self, const KMDataNode **np, char **path )
{
    const KMDataNode *found;

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
                        return RC ( rcDB, rcMetadata, rcSelecting, rcPath, rcInvalid );
                    continue;
                }
                break;
            }
            break;
        }

        /* find actual path */
        found = ( const KMDataNode* )
            BSTreeFind ( & self -> child, name, KMDataNodeCmp );
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

    rc = KMDataNodeFind ( self, ( const KMDataNode** ) & found, & p );
    if ( rc == 0 )
    {
        KMetadataAttach ( found -> meta );
        KMDataNodeAddRef ( found );
        * node = found;
    }

    DBGMSG(DBG_KDB, DBG_FLAG(DBG_KDB_KDB),
                ("KMDataNodeVOpenNodeRead(%s) = %d\n", full, rc));

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
                rc = SILENT_RC ( rcDB, rcMetadata, rcReading, rcAttr, rcNotFound );
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
            int64_t val = strtoi64 ( buffer, & end, 0 );
            if ( end [ 0 ] != 0 )
                rc = RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
            else if ( val < SHRT_MIN || val > SHRT_MAX )
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
            uint64_t val = strtou64 ( buffer, & end, 0 );
            if ( end [ 0 ] != 0 )
                rc = RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
            else if ( val > USHRT_MAX )
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
            int64_t val = strtoi64 ( buffer, & end, 0 );
            if ( end [ 0 ] != 0 )
                rc = RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
            else if ( val < INT_MIN || val > INT_MAX )
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
            uint64_t val = strtou64 ( buffer, & end, 0 );
            if ( end [ 0 ] != 0 )
                rc = RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
            else if ( val > UINT_MAX )
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


/*--------------------------------------------------------------------------
 * KMetadata
 *  a versioned, hierarchical structure
 */

/* Whack
 */
static
rc_t KMetadataWhack ( KMetadata *self )
{
    rc_t rc = 0;

    KRefcountWhack ( & self -> refcount, "KMetadata" );

    if ( self -> db != NULL )
    {
        rc = KDatabaseSever ( self -> db );
        if ( rc == 0 )
            self -> db = NULL;
    }
    else if ( self -> tbl != NULL )
    {
        rc = KTableSever ( self -> tbl );
        if ( rc == 0 )
            self -> tbl = NULL;
    }
    else if ( self -> col != NULL )
    {
        rc = KColumnSever ( self -> col );
        if ( rc == 0 )
            self -> col = NULL;
    }

    if ( rc == 0 )
        rc = KDBManagerSever ( self -> mgr );

    if ( rc == 0 )
    {
        KDirectoryRelease ( self -> dir );
        KMDataNodeWhack ( ( BSTNode* ) & self -> root -> n, NULL );
        free ( self );
        return 0;
    }

    KRefcountInit ( & self -> refcount, 1, "KMetadata", "whack", "kmeta" );
    return rc;
}


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
LIB_EXPORT rc_t CC KMetadataAddRef ( const KMetadata *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KMetadata" ) )
        {
        case krefLimit:
            return RC ( rcDB, rcMetadata, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC KMetadataRelease ( const KMetadata *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KMetadata" ) )
        {
        case krefWhack:
            return KMetadataWhack ( ( KMetadata* ) self );
        case krefNegative:
            return RC ( rcDB, rcMetadata, rcReleasing, rcRange, rcExcessive );
        }
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
        case krefNegative:
            return RC ( rcDB, rcMetadata, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

/* Make
 */

static
rc_t KMetadataPopulate ( KMetadata *self, const KDirectory *dir, const char *path )
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
                if ( GetRCState ( rc ) == rcIncorrect && GetRCObject ( rc ) == rcByteOrder )
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
                        pb . node_size_limit = NODE_SIZE_LIMIT;
                        pb . node_child_limit = NODE_CHILD_LIMIT;
                        pb . rc = 0;
                        pb . byteswap = self -> byteswap;

                        if ( hdr -> version == 1 )
                            PBSTreeDoUntil ( bst, 0, KMDataNodeInflate_v1, & pb );
                        else
                            PBSTreeDoUntil ( bst, 0, KMDataNodeInflate, & pb );
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
rc_t KMetadataMakeRead ( KMetadata **metap,
    const KDirectory *dir, const char *path, uint32_t rev )
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
            KRefcountInit ( & meta -> refcount, 1, "KMetadata", "make-read", path );
            meta -> rev = rev;
            meta -> byteswap = false;
            strcpy ( meta -> path, path );

            KRefcountInit ( & meta -> root -> refcount, 0, "KMDataNode", "make-read", "/" );

            rc = KMetadataPopulate ( meta, dir, path );
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


/* OpenMetadataRead
 *  opens metadata for read
 *
 *  "meta" [ OUT ] - return parameter for metadata
 */
static
rc_t KDBManagerOpenMetadataReadInt ( const KDBManager *self,
    KMetadata **metap, const KDirectory *wd, uint32_t rev, bool prerelease )
{
    char metapath [ 4096 ];
    rc_t rc = ( prerelease == 1 ) ?
        KDirectoryVResolvePath ( wd, true, metapath, sizeof metapath, "meta", NULL ):
        ( ( rev == 0 ) ?
          KDirectoryVResolvePath ( wd, true, metapath, sizeof metapath, "md/cur", NULL ):
          KDirectoryResolvePath ( wd, true, metapath, sizeof metapath, "md/r%.3u", rev ) );
    if ( rc == 0 )
    {
        KMetadata *meta;

        switch ( KDirectoryPathType ( wd, "%s", metapath ) )
        {
        case kptNotFound:
            return RC ( rcDB, rcMgr, rcOpening, rcMetadata, rcNotFound );
        case kptBadPath:
            return RC ( rcDB, rcMgr, rcOpening, rcPath, rcInvalid );
        case kptFile:
        case kptFile | kptAlias:
            break;
        default:
            return RC ( rcDB, rcMgr, rcOpening, rcPath, rcIncorrect );
        }

        rc = KMetadataMakeRead ( & meta, wd, metapath, rev );
        if ( rc == 0 )
        {
            meta -> mgr = KDBManagerAttach ( self );
            * metap = meta;
            return 0;
        }
    }
    
    return rc;
}

LIB_EXPORT rc_t CC KDatabaseOpenMetadataRead ( const KDatabase *self, const KMetadata **metap )
{
    rc_t rc;
    KMetadata *meta;

    if ( metap == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcParam, rcNull );

    * metap = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcSelf, rcNull );

    rc = KDBManagerOpenMetadataReadInt ( self -> mgr, & meta, self -> dir, 0, false );
    if ( rc == 0 )
    {
        meta -> db = KDatabaseAttach ( self );
        * metap = meta;
    }

    return rc;
}

LIB_EXPORT rc_t CC KTableOpenMetadataRead ( const KTable *self, const KMetadata **metap )
{
    rc_t rc;
    KMetadata *meta;

    if ( metap == NULL )
        return RC ( rcDB, rcTable, rcOpening, rcParam, rcNull );

    * metap = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcTable, rcOpening, rcSelf, rcNull );

    rc = KDBManagerOpenMetadataReadInt ( self -> mgr, & meta,
        self -> dir, 0, self -> prerelease );
    if ( rc == 0 )
    {
        meta -> tbl = KTableAttach ( self );
        * metap = meta;
    }

    return rc;
}

LIB_EXPORT rc_t CC KColumnOpenMetadataRead ( const KColumn *self, const KMetadata **metap )
{
    rc_t rc;
    KMetadata *meta;

    if ( metap == NULL )
        return RC ( rcDB, rcColumn, rcOpening, rcParam, rcNull );

    * metap = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcColumn, rcOpening, rcSelf, rcNull );

    rc = KDBManagerOpenMetadataReadInt ( self -> mgr, & meta, self -> dir, 0, false );
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
    if ( revision == NULL )
        return RC ( rcDB, rcMetadata, rcAccessing, rcParam, rcNull );

    * revision = 0;

    if ( self != NULL )
    {
        KNamelist *listing;
        rc_t rc = KDirectoryList ( self -> dir,
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

    return RC ( rcDB, rcMetadata, rcAccessing, rcSelf, rcNull );
}


/* OpenRevision
 *  opens a read-only indexed revision of metadata
 */
LIB_EXPORT rc_t CC KMetadataOpenRevision ( const KMetadata *self,
    const KMetadata **metap, uint32_t revision )
{
    rc_t rc;
    KMetadata *meta;

    if ( metap == NULL )
        return RC ( rcDB, rcMetadata, rcOpening, rcParam, rcNull );

    * metap = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcMetadata, rcOpening, rcSelf, rcNull );

    rc = KDBManagerOpenMetadataReadInt ( self -> mgr,
        & meta, self -> dir, revision, false );
    if ( rc == 0 )
    {
        if ( self -> db != NULL )
            meta -> db = KDatabaseAttach ( self -> db );
        else if ( self -> tbl != NULL )
            meta -> tbl = KTableAttach ( self -> tbl );
        else if ( self -> col != NULL )
            meta -> col = KColumnAttach ( self -> col );

        * metap = meta;
    }

    return rc;
}


/* GetSequence
 *  access a named sequence
 *
 *  "seq" [ IN ] - NUL terminated sequence name
 *
 *  "val" [ OUT ] - return parameter for sequence value
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
        return RC ( rcDB, rcMetadata, rcAccessing, rcString, rcNull );
    if ( seq [ 0 ] == 0 )
        return RC ( rcDB, rcMetadata, rcAccessing, rcString, rcInvalid );

    rc = KMDataNodeOpenNodeRead
        ( self -> root, & found, ".seq/%s", seq );
    if ( rc == 0 )
    {
        size_t num_read, remaining;
        rc = KMDataNodeRead ( found, 0, val, sizeof * val, & num_read, & remaining );
        assert ( rc != 0 || ( num_read == sizeof * val && remaining == 0 ) );
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
        return RC ( rcDB, rcNamelist, rcAccessing, rcParam, rcInvalid );
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
rc_t KMDataNodeNamelistMake ( KNamelist **names, uint32_t count )
{
    rc_t rc;
    KMDataNodeNamelist *self = calloc ( 1, sizeof * self -
        sizeof self -> namelist + count * sizeof self -> namelist [ 0 ] );
    if ( self == NULL )
        rc = RC ( rcDB, rcMetadata, rcListing, rcMemory, rcExhausted );
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
    * ( uint32_t* ) data += 1;
}

static
void CC KMDataNodeGrabAttr ( BSTNode *n, void *data )
{
    KMDataNodeNamelist *list = data;
    list -> namelist [ list -> count ++ ] = ( ( const KMAttrNode* ) n ) -> name;
}

LIB_EXPORT rc_t CC KMDataNodeListAttr ( const KMDataNode *self, KNamelist **names )
{
    if ( names == NULL )
        return RC ( rcDB, rcMetadata, rcListing, rcParam, rcNull );

    * names = NULL;

    if ( self != NULL )
    {
        rc_t rc;

        uint32_t count = 0;
        BSTreeForEach ( & self -> attr, 0, KMDataNodeListCount, & count );

        rc = KMDataNodeNamelistMake ( names, count );
        if ( rc == 0 )
            BSTreeForEach ( & self -> attr, 0, KMDataNodeGrabAttr, * names );

        return rc;
    }

    return RC ( rcDB, rcMetadata, rcListing, rcSelf, rcNull );
}

static
void CC KMDataNodeGrabName ( BSTNode *n, void *data )
{
    KMDataNodeNamelist *list = data;
    list -> namelist [ list -> count ++ ] = ( ( const KMDataNode* ) n ) -> name;
}

LIB_EXPORT rc_t CC KMDataNodeListChildren ( const KMDataNode *self, KNamelist **names )
{
    if ( names == NULL )
        return RC ( rcDB, rcMetadata, rcListing, rcParam, rcNull );

    * names = NULL;

    if ( self != NULL )
    {
        rc_t rc;

        uint32_t count = 0;
        BSTreeForEach ( & self -> child, 0, KMDataNodeListCount, & count );

        rc = KMDataNodeNamelistMake ( names, count );
        if ( rc == 0 )
            BSTreeForEach ( & self -> child, 0, KMDataNodeGrabName, * names );

        return rc;
    }

    return RC ( rcDB, rcMetadata, rcListing, rcSelf, rcNull );
}
