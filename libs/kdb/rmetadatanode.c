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


#include "rmeta.h"
#include "rmetadatanode.h"

#include <klib/refcount.h>
#include <klib/debug.h>
#include <klib/log.h>
#include <klib/rc.h>

struct KMDataNodeNamelist;
#define KNAMELIST_IMPL struct KMDataNodeNamelist
#include <klib/impl.h>

#include <limits.h>
#include <stdio.h>
#include <byteswap.h>
#include <strtol.h>

/*--------------------------------------------------------------------------
 * KMAttrNode
 */

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

static rc_t CC KRMDataNodeWhack ( KMDataNode *cself );
static rc_t CC KRMDataNodeRelease ( const KMDataNode *cself );
static rc_t CC KRMDataNodeByteOrder ( const KMDataNode *self, bool *reversed );
static rc_t CC KRMDataNodeRead ( const KMDataNode *self, size_t offset, void *buffer, size_t bsize, size_t *num_read, size_t *remaining );
static rc_t CC KRMDataNodeVOpenNodeRead ( const KMDataNode *self, const KMDataNode **node, const char *path, va_list args );
static rc_t CC KRMDataNodeReadB8 ( const KMDataNode *self, void *b8 );
static rc_t CC KRMDataNodeReadB16 ( const KMDataNode *self, void *b16 );
static rc_t CC KRMDataNodeReadB32 ( const KMDataNode *self, void *b32 );
static rc_t CC KRMDataNodeReadB64 ( const KMDataNode *self, void *b64 );
static rc_t CC KRMDataNodeReadB128 ( const KMDataNode *self, void *b128 );
static rc_t CC KRMDataNodeReadAsI16 ( const KMDataNode *self, int16_t *i );
static rc_t CC KRMDataNodeReadAsU16 ( const KMDataNode *self, uint16_t *u );
static rc_t CC KRMDataNodeReadAsI32 ( const KMDataNode *self, int32_t *i );
static rc_t CC KRMDataNodeReadAsU32 ( const KMDataNode *self, uint32_t *u );
static rc_t CC KRMDataNodeReadAsI64 ( const KMDataNode *self, int64_t *i );
static rc_t CC KRMDataNodeReadAsU64 ( const KMDataNode *self, uint64_t *u );
static rc_t CC KRMDataNodeReadAsF64 ( const KMDataNode *self, double *f );
static rc_t CC KRMDataNodeReadCString ( const KMDataNode *self, char *buffer, size_t bsize, size_t *size );
static rc_t CC KRMDataNodeReadAttr ( const KMDataNode *self, const char *name, char *buffer, size_t bsize, size_t *size );
static rc_t CC KRMDataNodeReadAttrAsI16 ( const KMDataNode *self, const char *attr, int16_t *i );
static rc_t CC KRMDataNodeReadAttrAsU16 ( const KMDataNode *self, const char *attr, uint16_t *u );
static rc_t CC KRMDataNodeReadAttrAsI32 ( const KMDataNode *self, const char *attr, int32_t *i );
static rc_t CC KRMDataNodeReadAttrAsU32 ( const KMDataNode *self, const char *attr, uint32_t *u );
static rc_t CC KRMDataNodeReadAttrAsI64 ( const KMDataNode *self, const char *attr, int64_t *i );
static rc_t CC KRMDataNodeReadAttrAsU64 ( const KMDataNode *self, const char *attr, uint64_t *u );
static rc_t CC KRMDataNodeReadAttrAsF64 ( const KMDataNode *self, const char *attr, double *f );

static KMDataNode_vt KRMDataNode_vt =
{
    KRMDataNodeWhack,
    KMDataNodeBaseAddRef,
    KRMDataNodeRelease,
    KRMDataNodeByteOrder,
    KRMDataNodeRead,
    KRMDataNodeVOpenNodeRead,
    KRMDataNodeReadB8,
    KRMDataNodeReadB16,
    KRMDataNodeReadB32,
    KRMDataNodeReadB64,
    KRMDataNodeReadB128,
    KRMDataNodeReadAsI16,
    KRMDataNodeReadAsU16,
    KRMDataNodeReadAsI32,
    KRMDataNodeReadAsU32,
    KRMDataNodeReadAsI64,
    KRMDataNodeReadAsU64,
    KRMDataNodeReadAsF64,
    KRMDataNodeReadCString,
    KRMDataNodeReadAttr,
    KRMDataNodeReadAttrAsI16,
    KRMDataNodeReadAttrAsU16,
    KRMDataNodeReadAttrAsI32,
    KRMDataNodeReadAttrAsU32,
    KRMDataNodeReadAttrAsI64,
    KRMDataNodeReadAttrAsU64,
    KRMDataNodeReadAttrAsF64
};

rc_t
KMDataNodeMakeRoot( KMDataNode ** node, KMetadata *meta )
{
    assert( node != NULL );
    KMDataNode * ret = calloc ( 1, sizeof *ret );
    if ( ret == NULL )
        return RC ( rcDB, rcMetadata, rcConstructing, rcMemory, rcExhausted );
    ret -> dad . vt = & KRMDataNode_vt;
    KRefcountInit ( & ret -> dad . refcount, 1, "KMDataNode", "make-read", "/" );
    ret -> meta = meta;
    *node = ret;

    return 0;
}

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
void
DataNodeWhack ( BSTNode *n, void *data )
{
    KMDataNodeRelease( (KMDataNode *)n );
}

static
rc_t CC
KRMDataNodeWhack ( KMDataNode *self )
{
    BSTreeWhack ( & self -> attr, KMAttrNodeWhack, NULL );
    BSTreeWhack ( & self -> child, DataNodeWhack, NULL );
    free ( self -> value );
    return KMDataNodeBaseWhack ( self );
}

static
rc_t CC
KRMDataNodeRelease ( const KMDataNode *cself )
{
    KMDataNode *self = ( KMDataNode* ) cself;

    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> dad . refcount, "KMDataNode" ) )
        {
        case krefOkay:
            break;
        case krefWhack:
            KMDataNodeWhack( self );
            break;
        case krefNegative:
            return RC ( rcDB, rcMetadata, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

/* Inflate
 */
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

    b -> dad . vt = & KRMDataNode_vt;
    b -> par = pb -> par;
    b -> meta = pb -> meta;
    b -> value = ( void* ) ( name + size + 1 );
    b -> vsize = n -> data . size - size - 1;
    BSTreeInit ( & b -> attr );
    BSTreeInit ( & b -> child );
    KRefcountInit ( & b -> dad . refcount, 1, "KMDataNode", "inflate", name );
    strcpy ( b -> name, name );

    /* a name with no associated value */
    if ( b -> vsize == 0 )
    {
        b -> value = NULL;
        BSTreeInsert ( pb -> bst, & b -> dad . n, KMDataNodeSort );
        return false;
    }

    /* allocate value because v2+ code expects this */
    value = malloc ( b -> vsize );
    if ( value != NULL )
    {
        memmove ( value, b -> value, b -> vsize );
        b -> value = value;
        BSTreeInsert ( pb -> bst, & b -> dad . n, KMDataNodeSort );
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

    b -> dad . vt = & KRMDataNode_vt;
    b -> par = pb -> par;
    b -> meta = pb -> meta;
    b -> value = ( void* ) ( name + size );
    b -> vsize = n -> data . size - size - 1;
    BSTreeInit ( & b -> attr );
    BSTreeInit ( & b -> child );
    memmove ( b -> name, name, size );
    b -> name [ size ] = 0;
    KRefcountInit ( & b -> dad . refcount, 1, "KMDataNode", "inflate", b -> name );

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
                BSTreeInsert ( pb -> bst, & b -> dad . n, KMDataNodeSort );
                return false;
            }

            value = malloc ( b -> vsize );
            if ( value != NULL )
            {
                memmove ( value, b -> value, b -> vsize );
                b -> value = value;
                BSTreeInsert ( pb -> bst, & b -> dad . n, KMDataNodeSort );
                return false;
            }
            pb -> rc = RC ( rcDB, rcMetadata, rcConstructing, rcMemory, rcExhausted );

            BSTreeWhack ( & b -> child, DataNodeWhack, NULL );
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
static
rc_t CC
KRMDataNodeVOpenNodeRead ( const KMDataNode *self, const KMDataNode **node, const char *path, va_list args )
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
        /*VDB-4386: cannot treat va_list as a pointer!*/
        /*if ( args == NULL )
            len = snprintf ( full, sizeof full, "%s", path );
        else*/
            len = vsnprintf ( full, sizeof full, path, args );
        if ( len < 0 || len >= sizeof full )
            return RC ( rcDB, rcNode, rcOpening, rcPath, rcExcessive );
    }

    rc = KMDataNodeFind ( self, ( const KMDataNode** ) & found, & p );
    if ( rc == 0 )
    {
        KMDataNodeAddRef ( found );
        * node = found;
    }

    DBGMSG(DBG_KDB, DBG_FLAG(DBG_KDB_KDB),
                ("KMDataNodeVOpenNodeRead(%s) = %d\n", full, rc));

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
KRMDataNodeByteOrder ( const KMDataNode *self, bool *reversed )
{
    return KMetadataByteOrder ( self -> meta, reversed );
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
KRMDataNodeRead ( const KMDataNode *self,
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
static
rc_t CC
KRMDataNodeReadB8 ( const KMDataNode *self, void *b8 )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, b8, 1, & num_read, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
            return RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
        if ( num_read < 1 )
            return RC ( rcDB, rcMetadata, rcReading, rcTransfer, rcIncomplete );
    }
    return rc;
}

static
rc_t CC
KRMDataNodeReadB16 ( const KMDataNode *self, void *b16 )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, b16, 2, & num_read, & remaining );
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

static
rc_t CC
KRMDataNodeReadB32 ( const KMDataNode *self, void *b32 )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, b32, 4, & num_read, & remaining );
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

static
rc_t CC
KRMDataNodeReadB64 ( const KMDataNode *self, void *b64 )
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

static
rc_t CC
KRMDataNodeReadB128 ( const KMDataNode *self, void *b128 )
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
static
rc_t CC
KRMDataNodeReadAsI16 ( const KMDataNode *self, int16_t *i )
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

static
rc_t CC
KRMDataNodeReadAsU16 ( const KMDataNode *self, uint16_t *u )
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

static
rc_t CC
KRMDataNodeReadAsI32 ( const KMDataNode *self, int32_t *i )
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

static
rc_t CC
KRMDataNodeReadAsU32 ( const KMDataNode *self, uint32_t *u )
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

static
rc_t CC
KRMDataNodeReadAsI64 ( const KMDataNode *self, int64_t *i )
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

static
rc_t CC
KRMDataNodeReadAsU64 ( const KMDataNode *self, uint64_t *u )
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

static
rc_t CC
KRMDataNodeReadAsF64 ( const KMDataNode *self, double *f )
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
static
rc_t CC
KRMDataNodeReadCString ( const KMDataNode *self, char *buffer, size_t bsize, size_t *size )
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
static
rc_t CC
KRMDataNodeReadAttr ( const KMDataNode *self, const char *name, char *buffer, size_t bsize, size_t *size )
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
static
rc_t CC
KRMDataNodeReadAttrAsI16 ( const KMDataNode *self, const char *attr, int16_t *i )
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

static
rc_t CC
KRMDataNodeReadAttrAsU16 ( const KMDataNode *self, const char *attr, uint16_t *u )
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

static
rc_t CC
KRMDataNodeReadAttrAsI32 ( const KMDataNode *self, const char *attr, int32_t *i )
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

static
rc_t CC
KRMDataNodeReadAttrAsU32 ( const KMDataNode *self, const char *attr, uint32_t *u )
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

static
rc_t CC
KRMDataNodeReadAttrAsI64 ( const KMDataNode *self, const char *attr, int64_t *i )
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

static
rc_t CC
KRMDataNodeReadAttrAsU64 ( const KMDataNode *self, const char *attr, uint64_t *u )
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

static
rc_t CC
KRMDataNodeReadAttrAsF64 ( const KMDataNode *self, const char *attr, double *f )
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

/*--------------------------------------------------------------------------
 * deep comparison of 2 Metadata-nodes
 */

/* >>>>>> !!! any changes here have to be duplicated in wmeta.c !!! <<<<<< */
static rc_t KMDataNodeCompare_int( const KMDataNode *self, const KMDataNode *other, bool *equal );

/* >>>>>> !!! any changes here have to be duplicated in wmeta.c !!! <<<<<< */
static rc_t KMDataNodeCompareValue( const KMDataNode *self, const KMDataNode *other, bool *equal ) {
    void const *self_data = NULL;
    size_t self_size = 0;
    rc_t rc = KMDataNodeAddr( self, &self_data, &self_size );
    if ( 0 == rc ) {
        void const *other_data = NULL;
        size_t other_size = 0;
        rc = KMDataNodeAddr( other, &other_data, &other_size );
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

/* >>>>>> !!! any changes here have to be duplicated in wmeta.c !!! <<<<<< */
struct CompareContext {
    KMDataNode const *other;
    bool *equal;
    rc_t rc;
};

/* >>>>>> !!! any changes here have to be duplicated in wmeta.c !!! <<<<<< */
static void CC KMDataNodeCompareChildren_cb( BSTNode *n, void *data ) {
    struct CompareContext *const ctx = data;
    if ( 0 == ctx -> rc && *( ctx -> equal ) ) {
        const KMDataNode *child = ( const KMDataNode * )n;
        const char * childName = child -> name;
        // find the corresponding child-node in ctx -> other
        const KMDataNode *other_child;
        ctx -> rc = KMDataNodeOpenNodeRead( ctx -> other, &other_child, childName );
        if ( 0 == ctx -> rc ) {
            ctx -> rc = KMDataNodeCompare_int( child, other_child, ctx -> equal ); /* recursion here! */
        } else {
            *( ctx -> equal ) = false;
        }
        KMDataNodeRelease( other_child );
    }
}

/* >>>>>> !!! any changes here have to be duplicated in wmeta.c !!! <<<<<< */
static void CC KMDataNodeCompareAttr_cb( BSTNode *n, void *data ) {
    struct CompareContext *const ctx = data;
    if ( 0 == ctx -> rc && *( ctx -> equal ) ) {
        const KMAttrNode *attr = ( const KMAttrNode * )n;
        if ( NULL != attr ) {
            const char * attrName = attr -> name;
            const KMAttrNode *other_attr = ( const KMAttrNode* )BSTreeFind( &( ctx -> other -> attr ),
                                                                            attrName, KMAttrNodeCmp );
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

/* >>>>>> !!! any changes here have to be duplicated in wmeta.c !!! <<<<<< */
static rc_t KMDataNodeCompare_int( const KMDataNode *self, const KMDataNode *other, bool *equal ) {
    /* first we compare the value of these 2 nodes */
    rc_t rc = KMDataNodeCompareValue( self, other, equal );
    if ( 0 == rc && *equal ) {
        /* if they are equal, we compare all of the children of self with the children of other */
        struct CompareContext ctx = { other, equal, 0 };
        BSTreeForEach( &( self -> child ), false, KMDataNodeCompareChildren_cb, &ctx );
        rc = ctx . rc;
        if ( 0 == rc && *equal ) {
            /* if they are equal we compare all attributes... */
            BSTreeForEach( &( self -> attr ), false, KMDataNodeCompareAttr_cb, &ctx );
            rc = ctx .rc;
        }
    }
    return rc;
}

/* >>>>>> !!! any changes here have to be duplicated in wmeta.c !!! <<<<<< */
LIB_EXPORT rc_t CC KMDataNodeCompare( const KMDataNode *self, KMDataNode const *other, bool *equal ) {
    rc_t rc = 0;
    if ( self == NULL ) {
        rc = RC( rcDB, rcNode, rcComparing, rcSelf, rcNull );
    } else if ( other == NULL || equal == NULL ) {
        rc = RC( rcDB, rcNode, rcComparing, rcParam, rcNull );
    } else if ( self -> meta == NULL && other -> meta == NULL ) {
        *equal = true;
    } else if ( self -> meta == NULL || other -> meta == NULL ) {
        *equal = false;
        rc = RC( rcDB, rcNode, rcComparing, rcMetadata, rcNull );
    } else {
        /* 2 way comparison, to make sure that both nodes have the same children... */
        rc =  KMDataNodeCompare_int( self, other, equal );
        if ( 0 == rc && *equal ) {
            rc =  KMDataNodeCompare_int( other, self, equal );
        }
    }
    return rc;
}
