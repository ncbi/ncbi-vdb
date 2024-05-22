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

#include <kdb/kdb-priv.h>

#include <klib/refcount.h>
#include <klib/debug.h>
#include <klib/log.h>
#include <klib/rc.h>

struct KRMDataNodeNamelist;
#define KNAMELIST_IMPL struct KRMDataNodeNamelist
#include <klib/impl.h>

#include <limits.h>
#include <stdio.h>
#include <byteswap.h>
#include <strtol.h>

/*--------------------------------------------------------------------------
 * KRMAttrNode
 */

static
int64_t CC KRMAttrNodeCmp ( const void *item, const BSTNode *n )
{
#define a ( ( const char* ) item )
#define b ( ( const KRMAttrNode* ) n )

    return strcmp ( a, b -> name );

#undef a
#undef b
}

static
int64_t CC KRMAttrNodeSort ( const BSTNode *item, const BSTNode *n )
{
#define a ( ( const KRMAttrNode* ) item )
#define b ( ( const KRMAttrNode* ) n )

    return strcmp ( a -> name, b -> name );

#undef a
#undef b
}

static
void CC KRMAttrNodeWhack ( BSTNode *n, void *data )
{
    free ( n );
}

static
bool CC KRMAttrNodeInflate ( PBSTNode *n, void *data )
{
    KRMAttrNode *b;
    KRMDataNodeInflateData *pb = data;

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
    BSTreeInsert ( pb -> bst, & b -> n, KRMAttrNodeSort );
    return false;
}


/*--------------------------------------------------------------------------
 * KRMDataNode
 *  a node with an optional value,
 *  optional attributes, and optional children
 *
 *  nodes are identified by path, relative to a starting node,
 *  where "/" serves as a path separator.
 */

static rc_t CC KRMDataNodeWhack ( KMDataNode *cself );
static rc_t CC KRMDataNodeByteOrder ( const KMDataNode *self, bool *reversed );
static rc_t CC KRMDataNodeRead ( const KMDataNode *self, size_t offset, void *buffer, size_t bsize, size_t *num_read, size_t *remaining );
static rc_t CC KRMDataNodeVOpenNodeRead ( const KMDataNode *self, const KMDataNode **node, const char *path, va_list args );
static rc_t CC KRMDataNodeReadAttr ( const KMDataNode *self, const char *name, char *buffer, size_t bsize, size_t *size );
static rc_t CC KRMDataNodeCompare( const KMDataNode *self, KMDataNode const *other, bool *equal );
static rc_t CC KRMDataNodeAddr ( const KMDataNode *self, const void **addr, size_t *size );
static rc_t CC KRMDataNodeListAttr ( const KMDataNode *self, KNamelist **names );
static rc_t CC KRMDataNodeListChildren ( const KMDataNode *self, KNamelist **names );

static KMDataNode_vt KRMDataNode_vt =
{
    KRMDataNodeWhack,
    KMDataNodeBaseAddRef,
    KMDataNodeBaseRelease,
    KRMDataNodeByteOrder,
    KRMDataNodeRead,
    KRMDataNodeVOpenNodeRead,
    KRMDataNodeReadAttr,
    KRMDataNodeCompare,
    KRMDataNodeAddr,
    KRMDataNodeListAttr,
    KRMDataNodeListChildren
};

#define CAST() assert( bself->vt == &KRMDataNode_vt ); KRMDataNode * self = (KRMDataNode *)bself

rc_t
KRMDataNodeMakeRoot( KRMDataNode ** node, KRMetadata *meta )
{
    assert( node != NULL );
    KRMDataNode * ret = calloc ( 1, sizeof *ret );
    if ( ret == NULL )
        return RC ( rcDB, rcMetadata, rcConstructing, rcMemory, rcExhausted );
    ret -> dad . vt = & KRMDataNode_vt;
    KRefcountInit ( & ret -> dad . refcount, 1, "KRMDataNode", "make-read", "/" );
    ret -> meta = meta;
    *node = ret;

    return 0;
}

static
int64_t CC KRMDataNodeCmp ( const void *item, const BSTNode *n )
{
#define a ( ( const char* ) item )
#define b ( ( const KRMDataNode* ) n )

    return strcmp ( a, b -> name );

#undef a
#undef b
}

static
int64_t CC KRMDataNodeSort ( const BSTNode *item, const BSTNode *n )
{
#define a ( ( const KRMDataNode* ) item )
#define b ( ( const KRMDataNode* ) n )

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
KRMDataNodeWhack ( KMDataNode *bself )
{
    CAST();

    BSTreeWhack ( & self -> attr, KRMAttrNodeWhack, NULL );
    BSTreeWhack ( & self -> child, DataNodeWhack, NULL );
    free ( self -> value );
    return KMDataNodeBaseWhack ( & self -> dad );
}

/* Inflate
 */
bool CC KRMDataNodeInflate_v1 ( PBSTNode *n, void *data )
{
    void *value;
    KRMDataNode *b;
    KRMDataNodeInflateData *pb = data;

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
    KRefcountInit ( & b -> dad . refcount, 1, "KRMDataNode", "inflate", name );
    strcpy ( b -> name, name );

    /* a name with no associated value */
    if ( b -> vsize == 0 )
    {
        b -> value = NULL;
        BSTreeInsert ( pb -> bst, & b -> dad . n, KRMDataNodeSort );
        return false;
    }

    /* allocate value because v2+ code expects this */
    value = malloc ( b -> vsize );
    if ( value != NULL )
    {
        memmove ( value, b -> value, b -> vsize );
        b -> value = value;
        BSTreeInsert ( pb -> bst, & b -> dad . n, KRMDataNodeSort );
        return false;
    }

    pb -> rc = RC ( rcDB, rcMetadata, rcConstructing, rcMemory, rcExhausted );
    free ( b );
    return true;
}

static
rc_t KMDataNodeInflateAttr ( KRMDataNode *n, bool byteswap )
{
    PBSTree *bst;
    rc_t rc = PBSTreeMake ( & bst, n -> value, n -> vsize, byteswap );
    if ( rc != 0 )
        rc = RC ( rcDB, rcMetadata, rcConstructing, rcData, rcCorrupt );
    else
    {
        KRMDataNodeInflateData pb;
        size_t bst_size = PBSTreeSize ( bst );

        pb . meta = n -> meta;
        pb . par = n;
        pb . bst = & n -> attr;
        pb . node_size_limit = NODE_SIZE_LIMIT;
        pb . node_child_limit = NODE_CHILD_LIMIT;
        pb . rc = 0;
        pb . byteswap = byteswap;
        PBSTreeDoUntil ( bst, 0, KRMAttrNodeInflate, & pb );
        rc = pb . rc;

        PBSTreeWhack ( bst );

        n -> value = ( char* ) n -> value + bst_size;
        n -> vsize -= bst_size;
    }
    return rc;
}

static
rc_t KMDataNodeInflateChild ( KRMDataNode *n,
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
            KRMDataNodeInflateData pb;
            pb . meta = n -> meta;
            pb . par = n;
            pb . bst = & n -> child;
            pb . node_size_limit = node_size_limit;
            pb . node_child_limit = node_child_limit;
            pb . rc = 0;
            pb . byteswap = byteswap;
            PBSTreeDoUntil ( bst, 0, KRMDataNodeInflate, & pb );
            rc = pb . rc;
        }

        PBSTreeWhack ( bst );

        n -> value = ( char* ) n -> value + bst_size;
        n -> vsize -= bst_size;
    }
    return rc;
}

bool CC KRMDataNodeInflate ( PBSTNode *n, void *data )
{
    KRMDataNode *b;
    KRMDataNodeInflateData *pb = data;

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
    KRefcountInit ( & b -> dad . refcount, 1, "KRMDataNode", "inflate", b -> name );

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
                BSTreeInsert ( pb -> bst, & b -> dad . n, KRMDataNodeSort );
                return false;
            }

            value = malloc ( b -> vsize );
            if ( value != NULL )
            {
                memmove ( value, b -> value, b -> vsize );
                b -> value = value;
                BSTreeInsert ( pb -> bst, & b -> dad . n, KRMDataNodeSort );
                return false;
            }
            pb -> rc = RC ( rcDB, rcMetadata, rcConstructing, rcMemory, rcExhausted );

            BSTreeWhack ( & b -> child, DataNodeWhack, NULL );
        }

        BSTreeWhack ( & b -> attr, KRMAttrNodeWhack, NULL );
    }

    free ( b );
    return true;
}


/* Find
 */
static
rc_t KRMDataNodeFind ( const KRMDataNode *self, const KRMDataNode **np, char **path )
{
    const KRMDataNode *found;

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
        found = ( const KRMDataNode* )
            BSTreeFind ( & self -> child, name, KRMDataNodeCmp );
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
KRMDataNodeVOpenNodeRead ( const KMDataNode *bself, const KMDataNode **node, const char *path, va_list args )
{
    CAST();

    rc_t rc;
    const KRMDataNode *found;
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

    rc = KRMDataNodeFind ( self, & found, & p );
    if ( rc == 0 )
    {
        KMDataNodeAddRef ( & found -> dad );
        * node = & found -> dad;
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
KRMDataNodeByteOrder ( const KMDataNode *bself, bool *reversed )
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
KRMDataNodeRead ( const KMDataNode *bself,
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
KRMDataNodeAddr ( const KMDataNode *bself, const void **addr, size_t *size )
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
KRMDataNodeReadAttr ( const KMDataNode *bself, const char *name, char *buffer, size_t bsize, size_t *size )
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
            const KRMAttrNode *n = ( const KRMAttrNode* )
                BSTreeFind ( & self -> attr, name, KRMAttrNodeCmp );
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

/*--------------------------------------------------------------------------
 * KNamelist
 */
typedef struct KRMDataNodeNamelist KRMDataNodeNamelist;
struct KRMDataNodeNamelist
{
    KNamelist dad;
    const KRMDataNode *node;
    size_t count;
    const char *namelist [ 1 ];
};

/* Whack
 */
static
rc_t CC KMDataNodeNamelistWhack ( KRMDataNodeNamelist *self )
{
    rc_t rc = KMDataNodeRelease ( & self -> node -> dad );
    if ( rc == 0 )
        free ( self );
    return rc;
}

/* Count
 */
static
rc_t CC KMDataNodeNamelistCount ( const KRMDataNodeNamelist *self, uint32_t *count )
{
    * count = ( uint32_t ) self -> count;
    return 0;
}

/* Get
 */
static
rc_t CC KMDataNodeNamelistGet ( const KRMDataNodeNamelist *self,
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
    KRMDataNodeNamelist *self = calloc ( 1, sizeof * self -
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
void CC KRMDataNodeListCount ( BSTNode *n, void *data )
{
    * ( uint32_t* ) data += 1;
}

static
void CC KRMDataNodeGrabAttr ( BSTNode *n, void *data )
{
    KRMDataNodeNamelist *list = data;
    list -> namelist [ list -> count ++ ] = ( ( const KRMAttrNode* ) n ) -> name;
}

static
rc_t CC
KRMDataNodeListAttr ( const KMDataNode *bself, KNamelist **names )
{
    CAST();

    if ( names == NULL )
        return RC ( rcDB, rcMetadata, rcListing, rcParam, rcNull );

    * names = NULL;

    rc_t rc;

    uint32_t count = 0;
    BSTreeForEach ( & self -> attr, 0, KRMDataNodeListCount, & count );

    rc = KMDataNodeNamelistMake ( names, count );
    if ( rc == 0 )
        BSTreeForEach ( & self -> attr, 0, KRMDataNodeGrabAttr, * names );

    return rc;
}

static
void CC KRMDataNodeGrabName ( BSTNode *n, void *data )
{
    KRMDataNodeNamelist *list = data;
    list -> namelist [ list -> count ++ ] = ( ( const KRMDataNode* ) n ) -> name;
}

static
rc_t CC
KRMDataNodeListChildren ( const KMDataNode *bself, KNamelist **names )
{
    CAST();

    if ( names == NULL )
        return RC ( rcDB, rcMetadata, rcListing, rcParam, rcNull );

    * names = NULL;

    rc_t rc;

    uint32_t count = 0;
    BSTreeForEach ( & self -> child, 0, KRMDataNodeListCount, & count );

    rc = KMDataNodeNamelistMake ( names, count );
    if ( rc == 0 )
        BSTreeForEach ( & self -> child, 0, KRMDataNodeGrabName, * names );

    return rc;
}

/*--------------------------------------------------------------------------
 * deep comparison of 2 Metadata-nodes
 */

/* >>>>>> !!! any changes here have to be duplicated in wmeta.c !!! <<<<<< */
static rc_t KRMDataNodeCompare_int( const KRMDataNode *self, const KRMDataNode *other, bool *equal );

/* >>>>>> !!! any changes here have to be duplicated in wmeta.c !!! <<<<<< */
static rc_t KRMDataNodeCompareValue( const KRMDataNode *self, const KRMDataNode *other, bool *equal ) {
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

/* >>>>>> !!! any changes here have to be duplicated in wmeta.c !!! <<<<<< */
struct CompareContext {
    KRMDataNode const *other;
    bool *equal;
    rc_t rc;
};

/* >>>>>> !!! any changes here have to be duplicated in wmeta.c !!! <<<<<< */
static void CC KRMDataNodeCompareChildren_cb( BSTNode *n, void *data ) {
    struct CompareContext *const ctx = data;
    if ( 0 == ctx -> rc && *( ctx -> equal ) ) {
        const KRMDataNode *child = ( const KRMDataNode * )n;
        const char * childName = child -> name;
        // find the corresponding child-node in ctx -> other
        const KRMDataNode *other_child;
        ctx -> rc = KMDataNodeOpenNodeRead( & ctx -> other -> dad, (const KMDataNode**)& other_child, childName );
        if ( 0 == ctx -> rc ) {
            ctx -> rc = KRMDataNodeCompare_int( child, other_child, ctx -> equal ); /* recursion here! */
        } else {
            *( ctx -> equal ) = false;
        }
        KMDataNodeRelease( & other_child -> dad );
    }
}

/* >>>>>> !!! any changes here have to be duplicated in wmeta.c !!! <<<<<< */
static void CC KRMDataNodeCompareAttr_cb( BSTNode *n, void *data ) {
    struct CompareContext *const ctx = data;
    if ( 0 == ctx -> rc && *( ctx -> equal ) ) {
        const KRMAttrNode *attr = ( const KRMAttrNode * )n;
        if ( NULL != attr ) {
            const char * attrName = attr -> name;
            const KRMAttrNode *other_attr = ( const KRMAttrNode* )BSTreeFind( &( ctx -> other -> attr ),
                                                                            attrName, KRMAttrNodeCmp );
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
static rc_t KRMDataNodeCompare_int( const KRMDataNode *self, const KRMDataNode *other, bool *equal ) {
    /* first we compare the value of these 2 nodes */
    rc_t rc = KRMDataNodeCompareValue( self, other, equal );
    if ( 0 == rc && *equal ) {
        /* if they are equal, we compare all of the children of self with the children of other */
        struct CompareContext ctx = { other, equal, 0 };
        BSTreeForEach( &( self -> child ), false, KRMDataNodeCompareChildren_cb, &ctx );
        rc = ctx . rc;
        if ( 0 == rc && *equal ) {
            /* if they are equal we compare all attributes... */
            BSTreeForEach( &( self -> attr ), false, KRMDataNodeCompareAttr_cb, &ctx );
            rc = ctx .rc;
        }
    }
    return rc;
}

/* >>>>>> !!! any changes here have to be duplicated in wmeta.c !!! <<<<<< */
static
rc_t CC
KRMDataNodeCompare( const KMDataNode *bself, KMDataNode const *bother, bool *equal )
{
    CAST();
    assert( bother->vt == &KRMDataNode_vt );
    KRMDataNode * other = (KRMDataNode *)bother;

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
        rc =  KRMDataNodeCompare_int( self, other, equal );
        if ( 0 == rc && *equal ) {
            rc =  KRMDataNodeCompare_int( other, self, equal );
        }
    }
    return rc;
}
