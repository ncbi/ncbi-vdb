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

#include <klib/extern.h>
#include <klib/btree.h>
#include <klib/ksort-macro.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

typedef struct SearchWindow {
    uint16_t	lower;
    uint16_t	upper;
} SearchWindow;


/* the leaf node works out naturally to be an odd count
 which is needed for splitting at the median. so we
 let a leaf fill, then split before further insert */
#define LNSIZE ( ( PGSIZE - 2 ) / 8 )
#define LNMEDIAN ( LNSIZE / 2 )
#define LNHALF ( LNSIZE / 2 )

typedef struct LeafEntry LeafEntry;
struct LeafEntry
{
    uint16_t key;
    uint16_t ksize;
};

typedef struct LeafNode LeafNode;
struct LeafNode
{
    uint16_t key_prefix;       /*** prefix offset into the blob ***/
    uint16_t key_prefix_len;   /*** length of the prefix ***/
    SearchWindow   win[256]; /*** search windows for first letter of the key ***/
    uint16_t count;	       /*** number of elements **/
    uint16_t key_bytes;        /*** bytes used for storing keys ***/
    LeafEntry ord [ ( PGSIZE - 8 - 256 * sizeof(SearchWindow)) / sizeof ( LeafEntry ) ];
};


/* the branch node works out to be an even key count
 which means that we also split before insert when
 full, but the split leaves the target insert side
 light, corrected immediately with an insert */
#define BRSIZE ( ( PGSIZE - 2 - 4 ) / 12 )

typedef struct BranchEntry BranchEntry;
struct BranchEntry
{
    uint16_t key;
    uint16_t ksize;
    uint32_t trans;
};

typedef struct BranchNode BranchNode;
struct BranchNode
{
    uint16_t key_prefix;       /*** offset into the blob ***/
    uint16_t key_prefix_len;   /*** length of the prefix ***/
    SearchWindow   win[256]; /*** search windows for first letter of the key ***/
    /* the exact structure here is important:
     "ltrans" will be accessed as node -> ord [ -1 ] . trans */
    uint16_t count;
    uint16_t key_bytes;
    
    uint32_t ltrans;
    BranchEntry ord [ ( PGSIZE - 12 - 256 * sizeof(SearchWindow) ) / sizeof ( BranchEntry ) ];
};



/* when keys are stored in pages, the max key size
 will be such that some number of keys are guaranteed to fit */
#define MIN_KEY_COUNT 2
#define MAX_KEY_SIZE \
(( PGSIZE - 12 - 256 * sizeof(SearchWindow)   - \
MIN_KEY_COUNT * ( sizeof ( BranchEntry ) + sizeof ( uint32_t ) ) \
) / MIN_KEY_COUNT )



/* Find
 *  searches for a match
 *
 *  "val" [ OUT ] - return parameter for value found
 *   accessed via KBTreeValueAccess* described above
 *   must be balanced with a call to KBTreeValueWhack.
 *
 *  "key" [ IN ] and "key_size" [ IN ] - describes an
 *   opaque key
 */

static int compare_keys(unsigned const qsize, uint8_t const query[/* qsize */], unsigned const ksize, uint8_t const key[/* ksize */])
{
    unsigned const csize = (qsize < ksize) ? qsize : ksize;
    int const diff = csize == 0 ? 0 : memcmp(query, key, csize);
    return diff == 0 ? (int)qsize - (int)ksize : diff;
}

static rc_t leaf_find(Pager *const pager, Pager_vt const *const vt, void const *page,
                      uint32_t *id, uint8_t const *const query, unsigned const qsize)
{
    const uint8_t *query_8 = query;
    size_t  qsize_8 = qsize;
    const LeafNode *cnode = vt->access(pager, page);
    assert(cnode != NULL);

    if(cnode->key_prefix_len > 0){
        const size_t key_prefix_len=cnode->key_prefix_len;
        /*** prefix must match ***/
        assert(key_prefix_len == 0 || compare_keys(key_prefix_len, query, key_prefix_len, ((uint8_t *)cnode )+cnode->key_prefix) == 0);
        /*************************/
        query_8 += key_prefix_len;
        qsize_8 -= key_prefix_len;
    }
    {
    /* perform search on branch node */
    unsigned const q = (qsize_8 > 0)?*query_8:0;
    unsigned lower = cnode->win[q].lower;
    unsigned upper = cnode->win[q].upper;

    while (lower < upper)
    {
        /* determine the slot to examine */
        unsigned const slot = ( lower + upper ) >> 1;

        /* perform comparison */
        const uint8_t *key = & ( ( const uint8_t* ) cnode ) [ cnode -> ord [ slot ] . key ];
        int const diff = compare_keys ( qsize_8, query_8, cnode -> ord [ slot ] . ksize, key );
        if ( diff == 0 )
        {
            memmove(id, key + cnode->ord[slot].ksize, 4);
            return 0;
        }
        if ( diff < 0 )
            upper = slot;
        else
            lower = slot + 1;
    }
    }
    return RC(rcDB, rcTree, rcSelecting, rcItem, rcNotFound);
}

static rc_t branch_find(Pager *const pager, Pager_vt const *const vt, void const *page,
                        uint32_t *id, uint8_t const *const query, unsigned const qsize)
{
    const uint8_t *query_8 = query;
    size_t  qsize_8 = qsize;
    rc_t rc = 0;
    const BranchNode *cnode = vt->access(pager, page);
    assert(cnode != NULL);

    if(cnode->key_prefix_len > 0){
        const size_t key_prefix_len=cnode->key_prefix_len;
        /*** prefix must match ***/
        assert(key_prefix_len == 0 || compare_keys(key_prefix_len, query, key_prefix_len, ((uint8_t *)cnode )+cnode->key_prefix) == 0);
        /*************************/
        query_8 += key_prefix_len;
        qsize_8 -= key_prefix_len;
    }
    {
        /* perform search on branch node */
        unsigned const q = (qsize_8 > 0)?*query_8:0;
        unsigned lower = cnode->win[q].lower;
        unsigned upper = cnode->win[q].upper;

        while (lower < upper)
        {
            /* determine the slot to examine */
            unsigned const slot = ( lower + upper ) >> 1;

            /* perform comparison */
            const uint8_t *key = & ( ( const uint8_t* ) cnode ) [ cnode -> ord [ slot ] . key ];
            int const diff = compare_keys ( qsize_8, query_8, cnode -> ord [ slot ] . ksize, key );
            if ( diff == 0 )
            {
                memmove(id, key + cnode->ord[slot].ksize, 4);
                return 0;
            }
            if ( diff < 0 )
                upper = slot;
            else
                lower = slot + 1;
        }
        /* should have the last slot tried ( < 0 ) or next slot to try ( > 0 ) */
        assert ( lower == upper );

        /* the node id is left-shifted by 1 and has the "branch-bit" indicator
           in the LSB. the remaining bits should NOT be zero */
        /* NB - if "upper" is 0 and type is signed,
           this will access entry -1, giving "ltrans" */
        {
            uint32_t const nid = (upper == 0) ? cnode->ltrans : cnode -> ord [ upper - 1 ] . trans;
            assert ( ( nid >> 1 ) != 0 );

            /* access child node */
            {
                void const *const child = vt->use(pager, nid >> 1);
                assert(child != NULL);
                rc = ( ( ( nid & 1 ) == 0 ) ? leaf_find : branch_find )
                    ( pager, vt, child, id, query, qsize );
                vt->unuse(pager, child);
            }
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC BTreeFind ( uint32_t const root, Pager *const pager, Pager_vt const *const vt, uint32_t *id,
                              const void *const key, size_t const key_size )
{
    assert (root != 0);
    assert (vt != NULL);
    assert (id != NULL);
    assert (key != NULL);
    assert (key_size != 0);
    {
        rc_t rc = 0;
        void const *const page = vt->use(pager, root >> 1);

        assert(page != NULL);
        rc = (((root & 1) == 0) ? leaf_find : branch_find)(pager, vt, page, id, key, key_size);
        vt->unuse(pager, page);
        return rc;
    }
}

/* Entry
 *  searches for a match or creates a new entry
 *
 *  "val" [ OUT ] - return parameter for value found
 *   accessed via KBTreeValueAccess* described above
 *   must be balanced with a call to KBTreeValueWhack.
 *
 *  "was_inserted" [ OUT ] - if true, the returned value was the result of an
 *   insertion and can be guaranteed to be all 0 bits. otherwise, the returned
 *   value will be whatever was there previously.
 *
 *  "alloc_size" [ IN ] - the number of value bytes to allocate upon insertion,
 *   i.e. if the key was not found. this value must agree with the limits
 *   specified in Make ( see above ).
 *
 *  "key" [ IN ] and "key_size" [ IN ] - describes an
 *   opaque key
 */
typedef struct Split Split;
struct Split
{
    void *key;
    uint32_t left;
    uint32_t right;
    uint16_t ksize;
    uint8_t buff [ 256 ];
};

void SplitInit(Split *const self)
{
    memset(self, 0, 32);
}

void SplitWhack(Split *const self)
{
    if (self->key && self->key != &self->buff[0]) {
        free(self->key);
    }
}

typedef struct EntryData EntryData;
struct EntryData
{
    Pager *pager;
    Pager_vt const *vt;
    uint32_t root;
    uint32_t *id;
    const void *key;
    size_t key_size;
    bool was_inserted;
};


static
rc_t make_entry ( EntryData *pb, void *hdrp, void *ordp )
{
    rc_t rc=0;
    /* use id provided from outside */
    {
        uint8_t *page = hdrp;
        LeafNode *hdr = hdrp;
        LeafEntry *ord = ordp;
        const uint8_t *key = pb -> key;
        uint16_t key_size = (uint16_t) ( pb -> key_size - hdr->key_prefix_len );
        
        
        assert(hdr->key_prefix_len == 0 || memcmp(key,page + hdr->key_prefix, hdr -> key_prefix_len)==0);/*** validate in debug mode **/
        key += hdr->key_prefix_len;
        
        /* grab memory for key and value-id */
        hdr -> key_bytes += key_size + sizeof ( uint32_t );
        assert ( hdr -> key_bytes < PGSIZE );
        
        /* location of key within page */
        ord -> key = PGSIZE - hdr -> key_bytes;
        ord -> ksize = ( uint16_t ) key_size;
        
        /* insert key */
        memmove ( page + ord -> key, key,  key_size );
        
        /* record value id */
        memmove ( & page [ ord -> key + key_size ], pb -> id, sizeof * pb -> id );
        
        /* mark inserted */
        pb -> was_inserted = true;
    }
    
    return rc;
}

static
bool leaf_node_full ( const LeafNode *node, size_t key_size )
{
    /*** add existing keys ***/
    size_t size = node -> key_bytes;
    /*** add new key ***/
    
    assert( key_size >= node -> key_prefix_len);
    size += key_size + sizeof ( uint32_t ) - node -> key_prefix_len;
    /*** add key index **/
    size += (uint8_t*)(node->ord + node -> count+1)  - (uint8_t*)node ;
    
    if ( size > PGSIZE )
    return true;
    return false;
}

static
rc_t leaf_insert ( EntryData *pb, LeafNode *node, uint32_t slot )
{
    rc_t rc;
    uint16_t q;
    
    /* check that key will fit */
    if ( leaf_node_full ( node, pb->key_size ) ) {
        return RC ( rcDB, rcTree, rcInserting, rcConstraint, rcViolated );
    }
    
    /* open hole */
    if ( slot != node -> count )
    memmove ( & node -> ord [ slot + 1 ], & node -> ord [ slot ], sizeof node -> ord [ 0 ] * ( node -> count - slot ) );
    
    /* enter into the leaf */
    rc = make_entry ( pb, node, & node -> ord [ slot ] );
    if ( rc == 0 )
    ++ node -> count;
    
    /* recover from error */
    else if ( slot != node -> count )
    memmove ( & node -> ord [ slot ], & node -> ord [ slot + 1 ], sizeof node -> ord [ 0 ] * ( node -> count - slot ) );
    
    /* correct search windows */
    q=(pb->key_size > node->key_prefix_len)?((uint8_t*)pb->key)[node->key_prefix_len]:0;
    /*** unused windows should have been maintained properly ****/
    assert((node->win[q].upper==node->win[q].lower)?(node->win[q].lower==slot):true);
    node->win[q].upper ++;
    for(q=q+1; q < 256; q++){
        node->win[q].lower++;
        node->win[q].upper++;
    }
    return rc;
}

static void LeafEntry_sort_desc_by_offset(uint16_t ord[], unsigned const count, LeafNode const *const node)
{
    uint16_t i;
    
    for (i = 0; i < (uint16_t)count; ++ i)
        ord[i] = i;
    
#define CMP(A, B) (node->ord[*(uint16_t *)B].key - node->ord[*(uint16_t *)A].key)
#define SWAP(A, B, C, D) do { i = *(uint16_t *)A; *(uint16_t *)A = *(uint16_t *)B; *(uint16_t *)B = i; } while(0)
    {
    KSORT(ord, count, sizeof(ord[0]), 0, 0);
    }
#undef SWAP
#undef CMP
}

static void BranchEntry_sort_desc_by_offset(uint16_t ord[], unsigned const count, BranchNode const *const node)
{
    uint16_t i;
    
    for (i = 0; i < (uint16_t)count; ++ i)
        ord[i] = i;
    
#define CMP(A, B) (node->ord[*(uint16_t *)B].key - node->ord[*(uint16_t *)A].key)
#define SWAP(A, B, C, D) do { i = *(uint16_t *)A; *(uint16_t *)A = *(uint16_t *)B; *(uint16_t *)B = i; } while(0)
    {
    KSORT(ord, count, sizeof(ord[0]), 0, 0);
    }
#undef SWAP
#undef CMP
}

static
rc_t split_leaf ( EntryData *pb,
                 LeafNode *left, LeafNode *right, uint32_t slot, Split *split)
{
    size_t off, ksize;
    bool hoist_existing;
    uint16_t ord [ ( sizeof left -> ord / sizeof left -> ord [ 0 ] + 1 ) / 2 ];
    
    /* calculate median */
    int32_t i, j, median = ( left -> count + 1 ) >> 1;
    
    /* pointers to pages */
    uint8_t * lpage = ( void* ) left;
    uint8_t * rpage = ( void* ) right;
    
    /* check to see if key being inserted would be hoisted */
    if ( slot == median && median > ( left -> count >> 1 ) )
    {
        hoist_existing = false;
        ksize = pb -> key_size;
    }
    else
    {
        /* TBD - choose median based upon count and slot */
        /* get median key size */
        hoist_existing = true;
        ksize = left -> ord [ median ] . ksize + left -> key_prefix_len;
    }
    
    /* allocate key space */
    split -> key = split -> buff;
    if ( ksize > ( sizeof split -> buff - sizeof ( uint32_t ) ) )
    {
        split -> key = malloc ( ksize + sizeof ( uint32_t ) );
        if ( split -> key == NULL )
        return RC ( rcDB, rcTree, rcInserting, rcMemory, rcExhausted );
    }
    
    /* copy out key plus value id */
    if ( hoist_existing ){
        split -> ksize = 0;
        if(left -> key_prefix_len > 0){
            memmove ( ((uint8_t*)split -> key) + split -> ksize, lpage + left -> key_prefix, left -> key_prefix_len );
            split -> ksize +=left -> key_prefix_len;
        }
        memmove(((uint8_t*)split -> key) + split -> ksize, lpage + left -> ord [ median ] . key , left -> ord [ median ] . ksize + sizeof ( uint32_t ) );
        split -> ksize += left -> ord [ median ] . ksize;
    }
    
    /* right page is empty */
    right -> key_bytes = 0;
    
    /* copy all of the keys to the right of median from left to right */
    for ( i = 0, j = median + hoist_existing; j < left -> count; ++ i, ++ j )
    {
        ksize = left -> ord [ j ] . ksize + sizeof ( uint32_t );
        right -> ord [ i ] . ksize = left -> ord [ j ] . ksize;
        right -> key_bytes += (uint16_t) ksize;
        right -> ord [ i ] . key = ( uint16_t ) ( PGSIZE - right -> key_bytes );
        memmove (rpage + right -> ord [ i ] . key, lpage + left -> ord [ j ] . key, ksize );
        if(i == 0 && left->key_prefix_len > 0){
            right -> key_prefix_len = left -> key_prefix_len;
            right -> key_bytes     += left -> key_prefix_len;
            right -> key_prefix     = PGSIZE - right -> key_bytes;
            memmove ( rpage + right -> key_prefix, lpage + left->key_prefix, left -> key_prefix_len );
        }
    }
    
    /* each node has half its former count */
    right -> count = ( uint16_t ) i;
    left -> count = median;
    
    /* compact the keys */
    LeafEntry_sort_desc_by_offset(ord, median, left);
    for ( left -> key_bytes = 0, i = 0; i < median; ++ i )
    {
        j = ord [ i ];
        ksize = left -> ord [ j ] . ksize + sizeof ( uint32_t );
        off = PGSIZE - ( left -> key_bytes += (uint16_t) ksize );
        if ( left -> ord [ j ] . key != off )
        {
            memmove ( & lpage [ off ], & lpage [ left -> ord [ j ] . key ], ksize );
            left -> ord [ j ] . key = ( uint16_t ) off;
        }
        if(i == 0 && left->key_prefix_len > 0){ /** need to restore prefix from the right ***/
            left -> key_bytes += left -> key_prefix_len;
            left -> key_prefix = PGSIZE - left -> key_bytes;
            memmove ( lpage + left -> key_prefix, rpage + right -> key_prefix, left -> key_prefix_len );
        }
    }
    /*** maintain search windows ****/
    /*** find the last window of left ***/
    for(i=0;left->win[i].upper < left->count; i++){
        assert(i<256);
        right->win[i].lower=0;
        right->win[i].upper=0;
    }
    j=right->ord[0].ksize?((uint8_t*)right)[right->ord[0].key]:0;
    assert(i<=j);
    for(;i<j;i++){
        right->win[i].lower=0;
        right->win[i].upper=0;
        left->win[i].upper=left->count;
        left->win[i+1].lower=left->count;
    }
    /*** set first window of right ***/
    right->win[i].lower = 0;
    assert( left->win[i].upper >= left->count + hoist_existing);
    right->win[i].upper = left->win[i].upper - left->count - hoist_existing;
    /*** set last window of left ***/
    left->win[i].upper = left->count;
    i++;
    /*** set right windows from left, reset left ***/
    for(;i<256;i++){
        assert( left->win[i].lower >= left->count + hoist_existing );
        assert( left->win[i].upper >= left->count + hoist_existing );
        right->win[i].lower = left->win[i].lower - left->count  - hoist_existing;
        right->win[i].upper = left->win[i].upper - left->count  - hoist_existing;
        left->win[i].lower=left->count;
        left->win[i].upper=left->count;
    }
    assert(right->win[255].upper == right -> count);
    /*** reset right window ***/
    for(;i<256;i++){
        right->win[i].lower=right->count;
        right->win[i].upper=right->count;
    }
    /*****************************************/
    
    /* if insert would be hoisted, do it directly */
    if ( ! hoist_existing )
    {
        memmove ( split -> key, pb -> key , pb -> key_size );
        memmove ( & ( ( uint8_t* ) split -> key ) [ pb -> key_size ], pb -> id, sizeof * pb -> id );
        split -> ksize =  (uint16_t) pb -> key_size;
        return 0;
    }
    
    /* TBD - with keys in pages, there is no guarantee that the following insert
     will succeed, unless there would be a provision for overflow. */
    
    /* decide where to insert entry */
    if ( slot <= (uint32_t) median )
    return leaf_insert ( pb, left, slot );
    return leaf_insert ( pb, right, slot - median - 1 );
}



static
rc_t leaf_entry ( EntryData *pb, void const *page, Split *split)
{
    int lower, upper;
    rc_t rc = 0;
    
    const uint8_t *query = pb -> key;
    size_t qsize = pb -> key_size;
    uint16_t q;
    
    /* access page for read */
    const LeafNode *cnode = pb->vt->access(pb->pager, page);
    assert(cnode != NULL);
    
    /* special case for root node initial insert */
    if ( cnode -> count == 0 )
    {
        LeafNode *node = pb->vt->update(pb->pager, page);
        assert(node != NULL);
        
        {
            rc = make_entry ( pb, node, & node -> ord [ 0 ] );
            if ( rc == 0 ){
                int i;
                q=(qsize >0)? *query:0;
                node -> count = 1;
                node -> win[q].lower = 0;
                node -> win[q].upper = 1;
                for(i=0;i<q;i++){
                    node -> win[i].lower = 0;
                    node -> win[i].upper = 0;
                }
                for(i=q+1;i<256;i++){
                    node -> win[i].lower = 1;
                    node -> win[i].upper = 1;
                }
            }
        }
        return rc;
    }
    
    /* perform search on branch node */
    /* start with prefix compare */
    if(cnode->key_prefix_len > 0){
        const size_t key_prefix_len=cnode->key_prefix_len;
        /*** prefix must match ***/
        assert(key_prefix_len == 0 || compare_keys(key_prefix_len, query, key_prefix_len, ((uint8_t *)cnode )+cnode->key_prefix) == 0);
        /*************************/
        query += cnode->key_prefix_len;
        qsize -= cnode->key_prefix_len;
        
    }
    q=(qsize >0)? *query:0;
    for ( lower = cnode->win[q].lower, upper = cnode -> win[q].upper; lower < upper; )
    {
        /* determine the slot to examine */
        int slot = ( lower + upper ) >> 1;
        
        /* perform comparison */
        const uint8_t *key = & ( ( const uint8_t* ) cnode ) [ cnode -> ord [ slot ] . key ];
        int diff = compare_keys(qsize, query, cnode -> ord [ slot ] . ksize, key);
        if ( diff == 0 )
        {
            uint32_t val_id;
            memmove ( & val_id, & key [ cnode -> ord [ slot ] . ksize ], sizeof val_id );
            * pb -> id = val_id;
            return 0;
        }
        if ( diff < 0 )
        upper = slot;
        else
        lower = slot + 1;
    }
    
    /* should have the last slot tried ( < 0 ) or next slot to try ( > 0 ) */
    assert ( lower == upper );
    {
    /* going to need to update the node */
    LeafNode *node = pb->vt->update(pb->pager, page);
    assert(node != NULL);
    
    /* unless an error occurs */
    pb -> was_inserted = true;
    
    /* test for split */
    if ( leaf_node_full ( node, pb->key_size ) )
    {
        /* create another leaf node to right */
        void const *dup = pb->vt->alloc(pb->pager, &split->right);
        if (dup == NULL)
        rc = RC ( rcDB, rcTree, rcInserting, rcMemory, rcExhausted );
        else {
            void *mem = pb->vt->update(pb->pager, dup);
            assert(mem != NULL);
            {
                rc = split_leaf ( pb, node, mem, upper, split);
                if ( rc == 0 )
                {
                    rc = RC ( rcDB, rcTree, rcInserting, rcId, rcInsufficient );
                    split -> right <<= 1;
                }
            }
            pb->vt->unuse(pb->pager, dup);
        }
    }
    else
    {
        /* simple insert */
        rc = leaf_insert ( pb, node, upper );
    }
    }
    return rc;
}

static
bool branch_node_full ( const BranchNode *node, size_t key_size )
{
    /*** add existing keys ***/
    size_t size = node -> key_bytes;
    /*** add new key ***/
    assert( key_size >= node -> key_prefix_len);
    size += key_size + sizeof ( uint32_t ) - node -> key_prefix_len;
    
    /*** add key index **/
    size += (uint8_t*)(node->ord + node -> count+1)  - (uint8_t*)node ;
    if ( size > PGSIZE )
    return true;
    return false;
}

static
rc_t branch_insert ( BranchNode *node, const Split *split, uint32_t slot )
{
    uint16_t ksize = split -> ksize;
    uint8_t *key   = split -> key;
    uint8_t *page = (uint8_t*) node;
    uint16_t q;
    
    /* remove prefix from the key */
    ksize -= node -> key_prefix_len;
    
    assert(node->key_prefix_len == 0 || memcmp(key,page + node->key_prefix, node -> key_prefix_len)==0);/*** validate in debug mode **/
    key   += node -> key_prefix_len;
    
    /* check that key will fit */
    if ( branch_node_full ( node, split -> ksize ) )
    return RC ( rcDB, rcTree, rcInserting, rcConstraint, rcViolated );
    
    /* open hole */
    if ( slot != node -> count )
    memmove ( & node -> ord [ slot + 1 ], & node -> ord [ slot ], sizeof node -> ord [ 0 ] * ( node -> count - slot ) );
    
    /* enter key value pair */
    node -> key_bytes += ksize + sizeof ( uint32_t );
    node -> ord [ slot ] . ksize = ksize;
    node -> ord [ slot ] . key = ( uint16_t ) ( PGSIZE - node -> key_bytes );
    memmove ( & ( ( uint8_t* ) node ) [ PGSIZE - node -> key_bytes ], key, ksize + sizeof ( uint32_t ) );
    
    /* enter the new transitions */
    assert ( node -> ord [ ( int ) slot - 1 ] . trans == split -> left );
    node -> ord [ ( int ) slot - 1 ] . trans = split -> left;
    node -> ord [ slot ] . trans = split -> right;
    
    ++ node -> count;
    
    /* correct search windows */
    q=(split->ksize > node->key_prefix_len)?((uint8_t*)split->key)[node->key_prefix_len]:0;
    /*** unused windows should have beed maintained properly ****/
    assert((node->win[q].upper==node->win[q].lower)?(node->win[q].lower==slot):true);
    node->win[q].upper ++;
    for(q=q+1; q < 256; q++){
        node->win[q].lower++;
        node->win[q].upper++;
    }
    return 0;
}

static rc_t split_branch ( BranchNode *left, BranchNode *right, const Split *val, Split *split, uint32_t slot)
{
    size_t off, ksize;
    bool hoist_existing;
    uint16_t ord [ ( sizeof left -> ord / sizeof left -> ord [ 0 ] + 1 ) / 2 ];
    
    /* calculate median */
    int32_t i, j, median = ( left -> count + 1 ) >> 1;
    
    /* pointers to pages */
    uint8_t * lpage = ( void* ) left;
    uint8_t * rpage = ( void* ) right;
    
    /* check to see if key being inserted would be hoisted */
    if ( slot == median && median > ( left -> count >> 1 ) )
    {
        hoist_existing = false;
        ksize = val -> ksize;
    }
    else
    {
        /* get median key size */
        hoist_existing = true;
        ksize = left -> ord [ median ] . ksize + left -> key_prefix_len;
    }
    
    /* allocate key space */
    split -> key = split -> buff;
    if ( ksize > ( sizeof split -> buff - sizeof ( uint32_t ) ) )
    {
        split -> key = malloc ( ksize + sizeof ( uint32_t ) );
        if ( split -> key == NULL )
        return RC ( rcDB, rcTree, rcInserting, rcMemory, rcExhausted );
    }
    
    /* copy out key plus value id */
    if ( hoist_existing ){
        split -> ksize = 0;
        if(left -> key_prefix_len > 0){
            memmove ( ((uint8_t*)split -> key) + split -> ksize, lpage + left -> key_prefix, left -> key_prefix_len );
            split -> ksize +=left -> key_prefix_len;
        }
        memmove(((uint8_t*)split -> key) + split -> ksize, lpage + left -> ord [ median ] . key , left -> ord [ median ] . ksize + sizeof ( uint32_t ) );
        split -> ksize +=  left -> ord [ median ] . ksize;
    }
    
    
    /* right page is empty */
    right -> key_bytes = 0;
    
    /* copy all of the keys to the right of the median from left to right */
    for ( i = 0, j = median + hoist_existing; j < left -> count; ++ i, ++ j )
    {
        ksize = left -> ord [ j ] . ksize + sizeof ( uint32_t );
        right -> ord [ i ] . ksize = left -> ord [ j ] . ksize;
        right -> key_bytes += (uint16_t) ksize;
        right -> ord [ i ] . key = ( uint16_t ) ( PGSIZE - right -> key_bytes );
        memmove ( & rpage [ PGSIZE - right -> key_bytes ], & lpage [ left -> ord [ j ] . key ], ksize );
        right -> ord [ i - 1 ] . trans = left -> ord [ j - 1 ] . trans;
        if(i == 0 && left->key_prefix_len > 0){
            off = PGSIZE - right -> key_bytes - left -> key_prefix_len;
            memmove ( & rpage [ off ], lpage + left -> key_prefix, left -> key_prefix_len );
            right -> key_bytes += left -> key_prefix_len;
            right -> key_prefix_len = left -> key_prefix_len;
            right -> key_prefix = (uint16_t) off;
        }
    }
    
    /* copy the last trans */
    right -> ord [ i - 1 ] . trans = left -> ord [ j - 1 ] . trans;
    
    /* each node has half its former count */
    right -> count = ( uint16_t ) i;
    left -> count = median;
    
    /* compact the keys */
    BranchEntry_sort_desc_by_offset(ord, median, left);
    for ( left -> key_bytes = 0, i = 0; i < median; ++ i )
    {
        j = ord [ i ];
        ksize = left -> ord [ j ] . ksize + sizeof ( uint32_t );
        off = PGSIZE - ( left -> key_bytes += (uint16_t) ksize );
        if ( left -> ord [ j ] . key != off )
        {
            memmove ( & lpage [ off ], & lpage [ left -> ord [ j ] . key ], ksize );
            left -> ord [ j ] . key = ( uint16_t ) off;
        }
        if(i == 0 && left->key_prefix_len > 0){ /** need to restore prefix from the right ***/
            off = PGSIZE - left -> key_bytes - left -> key_prefix_len;
            memmove ( & lpage [ off ], rpage + right -> key_prefix, left -> key_prefix_len );
            left -> key_bytes += left -> key_prefix_len;
            left -> key_prefix = (uint16_t) off;
        }
    }
    /*** maintain search windows ****/
    /*** find the last window of left ***/
    for(i=0;left->win[i].upper < left->count; i++){
        assert(i<256);
        right->win[i].lower=0;
        right->win[i].upper=0;
    }
    j=right->ord[0].ksize?((uint8_t*)right)[right->ord[0].key]:0;
    assert(i<=j);
    for(;i<j;i++){
        right->win[i].lower=0;
        right->win[i].upper=0;
        left->win[i].upper=left->count;
        left->win[i+1].lower=left->count;
    }
    /*** set first window of right ***/
    right->win[i].lower = 0;
    assert( left->win[i].upper >= left->count + hoist_existing);
    right->win[i].upper = left->win[i].upper - left->count - hoist_existing;
    /*** set last window of left ***/
    left->win[i].upper = left->count;
    i++;
    /*** set right windows from left, reset left ***/
    for(;i<256;i++){
        assert( left->win[i].lower >= left->count + hoist_existing );
        assert( left->win[i].upper >= left->count + hoist_existing );
        right->win[i].lower = left->win[i].lower - left->count  - hoist_existing;
        right->win[i].upper = left->win[i].upper - left->count  - hoist_existing;
        left->win[i].lower=left->count;
        left->win[i].upper=left->count;
    }
    assert(right->win[255].upper == right -> count);
    /*** reset right window ***/
    for(;i<256;i++){
        right->win[i].lower=right->count;
        right->win[i].upper=right->count;
    }
    /*****************************************/
    
    /* if insert would be hoisted, do it directly */
    if ( ! hoist_existing )
    {
        /* copy key and value */
        memmove ( split -> key, val -> key, val -> ksize + sizeof ( uint32_t ) );
        split ->  ksize = val -> ksize;
        
        /* set left and right transitions */
        assert ( left -> ord [ median - 1 ] . trans == val -> left );
        left -> ord [ median - 1 ] . trans = val -> left;
        right -> ltrans = val -> right;
        
        return 0;
    }
    
    /* decide where to insert entry */
    if ( slot <= (uint32_t) median )
        return branch_insert ( left, val, slot );
    return branch_insert ( right, val, slot - median - 1 );
}
#define MIN_PREFIX_TO_COMPACT 1 /*** minimum prefix delta to initiate compacting ****/
static
rc_t leaf_compact (EntryData *pb, void const *pg,uint16_t prefix_len)
{
    rc_t rc = 0;
    const LeafNode *cnode = pb->vt->access(pb->pager, pg);
    assert(cnode != NULL);
    
    if(cnode->key_prefix_len < prefix_len) {
        assert(cnode->count   > 0);
        if( cnode->count   > 0 && prefix_len > cnode->key_prefix_len ){
            prefix_len -= cnode->key_prefix_len; /*** need only to compact the delta **/
            assert(prefix_len == 0 || memcmp(((uint8_t*)cnode)+cnode->ord[0].key,
                          ((uint8_t*)cnode)+cnode->ord[cnode->count-1].key,
                          prefix_len )==0);/****** To be sure ****/
            if( prefix_len >= MIN_PREFIX_TO_COMPACT){ /*** good to compact ***/
                LeafNode *node = pb->vt->update(pb->pager, pg);
                assert(node != NULL);
                {
                    uint8_t  q,last;
                    uint16_t i;
                    uint16_t ord [ ( sizeof node -> ord / sizeof node -> ord [ 0 ] + 1 ) / 2 ];
                    
                    LeafEntry_sort_desc_by_offset(ord, node->count, node);
                    /*** deal with prefix and index 0 ***/
                    /*** prefix is cut from the last stored key on the page **/
                    if(node->key_prefix_len == 0){
                        node->key_prefix = node->ord[ord[0]].key;
                    }
                    node->key_prefix_len    += prefix_len;
                    node->ord[ord[0]].key   += prefix_len;
                    node->ord[ord[0]].ksize -= prefix_len;
                    /*** deal with index 1 (shift) ***/
                    if(node->count > 1){
                        node->ord[ord[1]].key   += prefix_len;
                        node->ord[ord[1]].ksize -= prefix_len;
                    }
                    /*** cut all other values ***/
                    for ( i = 2; i < node->count; ++ i ){
                        uint16_t j = ord[i];
                        uint8_t *src = ((uint8_t*)node) + node->ord[j].key + prefix_len;
                        uint8_t *dst = src + (i-1)*prefix_len; /*** proportional offset ***/
                        uint16_t ksize = node->ord[j].ksize + sizeof ( uint32_t ) - prefix_len;
                        memmove(dst,src,ksize);
                        node->ord[j].ksize -= prefix_len;
                        node->ord[j].key    = (uint16_t) ( dst -  ((uint8_t*)node) );
                    }
                    assert( node -> key_bytes > (node->count-1) * prefix_len);
                    node -> key_bytes -= (node->count-1) * prefix_len;
                    /*** rebuild search windows ***/
                    q=node->ord[0].ksize?((uint8_t*)node)[node->ord[0].key]:0;
                    for(i=0;i<q;i++){
                        node->win[i].lower=0;
                        node->win[i].upper=0;
                    }
                    node->win[q].lower=0;
                    for(i=0,last=q;i< node->count; ++ i ){
                        q=node->ord[i].ksize?((uint8_t*)node)[node->ord[i].key]:0;
                        assert(last<=q);
                        if(q!=last){
                            node->win[last++].upper = i;
                            for(;last<q;last++){/*** need to catch up ***/
                                node->win[last].lower=i;
                                node->win[last].upper=i;
                            }
                            node->win[q].lower=i;
                            last=q;
                        }
                    }
                    node->win[last].upper=node->count;
                    for(i=last+1;i<256;i++){
                        node->win[i].lower=node->count;
                        node->win[i].upper=node->count;
                    }
                }
            }
        }
    }
    return rc;
}

static
rc_t branch_compact (EntryData *pb, void const *pg,uint16_t prefix_len)
{
    rc_t rc = 0;
    const BranchNode *cnode = pb->vt->access(pb->pager, pg);
    assert(cnode != NULL);
    
    if(cnode->key_prefix_len < prefix_len) {
        assert(cnode->count   > 0);
        if( cnode->count   > 0 && prefix_len > cnode->key_prefix_len ){
            prefix_len -= cnode->key_prefix_len; /*** need only to compact the delta **/
            assert(prefix_len == 0 || memcmp(((uint8_t*)cnode)+cnode->ord[0].key,
                          ((uint8_t*)cnode)+cnode->ord[cnode->count-1].key,
                          prefix_len )==0);/****** To be sure ****/
            if( prefix_len >= MIN_PREFIX_TO_COMPACT){ /*** good to compact ***/
                BranchNode *node = pb->vt->update(pb->pager, pg);
                assert(node != NULL);
                {
                    uint8_t	 q,last;
                    uint16_t i;
                    uint16_t ord [ ( sizeof node -> ord / sizeof node -> ord [ 0 ] + 1 ) / 2 ];
                    
                    BranchEntry_sort_desc_by_offset(ord, node->count, node);
                    /*** deal with prefix and index 0 ***/
                    /*** prefix is cut from the last stored key on the page **/
                    if(node->key_prefix_len == 0){
                        node->key_prefix = node->ord[ord[0]].key;
                    }
                    node->key_prefix_len    += prefix_len;
                    node->ord[ord[0]].key   += prefix_len;
                    node->ord[ord[0]].ksize -= prefix_len;
                    /*** deal with index 1 (shift) ***/
                    if(node->count > 1){
                        node->ord[ord[1]].key   += prefix_len;
                        node->ord[ord[1]].ksize -= prefix_len;
                    }
                    /*** cut all other values ***/
                    for ( i = 2; i < node->count; ++ i ){
                        uint16_t j = ord[i];
                        uint8_t *src = ((uint8_t*)node) + node->ord[j].key + prefix_len;
                        uint8_t *dst = src + (i-1)*prefix_len; /*** proportional offset ***/
                        uint16_t ksize = node->ord[j].ksize + sizeof ( uint32_t ) - prefix_len;
                        memmove(dst,src,ksize);
                        node->ord[j].ksize -= prefix_len;
                        node->ord[j].key    = (uint16_t) ( dst -  ((uint8_t*)node) );
                    }
                    assert( node -> key_bytes > (node->count-1) * prefix_len);
                    node -> key_bytes -= (node->count-1) * prefix_len;
                    /*** rebuild search windows ***/
                    q=node->ord[0].ksize?((uint8_t*)node)[node->ord[0].key]:0;
                    for(i=0;i<q;i++){
                        node->win[i].lower=0;
                        node->win[i].upper=0;
                    }
                    node->win[q].lower=0;
                    for(i=0,last=q;i< node->count; ++ i ){
                        q=node->ord[i].ksize?((uint8_t*)node)[node->ord[i].key]:0;
                        assert(last<=q);
                        if(q!=last){
                            node->win[last++].upper = i;
                            for(;last<q;last++){/*** need to catch up ***/
                                node->win[last].lower=i;
                                node->win[last].upper=i;
                            }
                            node->win[q].lower=i;
                            last=q;
                        }
                    }
                    node->win[last].upper=node->count;
                    for(i=last+1;i<256;i++){
                        node->win[i].lower=node->count;
                        node->win[i].upper=node->count;
                    }
                }
            }
        }
    }
    return rc;
}

static
rc_t compact_page ( EntryData *pb, uint32_t nid, uint16_t prefix_len)
{
    void const *pg = pb->vt->use(pb->pager, nid >> 1);
    assert(pg != NULL);
    {
    rc_t const rc = (((nid & 1) == 0) ? leaf_compact : branch_compact)(pb, pg, prefix_len);
    pb->vt->unuse(pb->pager, pg);
    return rc;
    }
}


static
rc_t branch_entry ( EntryData *pb, void const *page, Split *rsplit)
{
    void const *child;
    uint32_t nid;
    int lower, upper;
    const uint8_t *query = pb -> key;
    size_t qsize = pb -> key_size;
    uint16_t q;
    rc_t rc = 0;

    /* look at node in read-only mode */
    const BranchNode *cnode = pb->vt->access(pb->pager, page);
    assert(cnode != NULL);
    
    /* perform search on branch node */
    /* start with prefix compare */
    if(cnode->key_prefix_len > 0){
        const size_t key_prefix_len=cnode->key_prefix_len;
        /*** prefix must match ***/
        assert(key_prefix_len == 0 || compare_keys(key_prefix_len, query,key_prefix_len, ((uint8_t *)cnode )+cnode->key_prefix) == 0);
        /*************************/
        query += cnode->key_prefix_len;
        qsize -= cnode->key_prefix_len;
    }
    q = (qsize>0)?*query:0;
    
    for ( lower = cnode->win[q].lower, upper = cnode -> win[q].upper; lower < upper; )
    {
        /* determine the slot to examine */
        int slot = ( lower + upper ) >> 1;
        
        /* perform comparison */
        const uint8_t *key = & ( ( const uint8_t* ) cnode ) [ cnode -> ord [ slot ] . key ];
        int diff = compare_keys(qsize, query, cnode -> ord [ slot ] . ksize, key);
        if ( diff == 0 )
        {
            memmove(pb->id, &key[cnode->ord[slot].ksize], 4);
            return 0;
        }
        if ( diff < 0 )
        upper = slot;
        else
        lower = slot + 1;
    }
    
    /* should have the last slot tried ( < 0 ) or next slot to try ( > 0 ) */
    assert ( lower == upper );
    
    /* the node id is left-shifted by 1 and has the "branch-bit" indicator
     in the LSB. the remaining bits should NOT be zero */
    nid = cnode -> ord [ upper - 1 ] . trans;
    assert ( ( nid >> 1 ) != 0 );
    
    /* access child node */
    child = pb->vt->use(pb->pager, nid >> 1);
    assert(child != NULL);
    {
        Split split;
        SplitInit ( & split );
        /* recursively continue search */
        rc = (((nid & 1) == 0) ? leaf_entry : branch_entry)(pb, child, &split);
        /* detect split */
        if ( GetRCState ( rc ) == rcInsufficient && GetRCObject ( rc ) == rcId )
        {
            /* splitting may replace value being inserted in to the branch ***/
            /* access current node */
            BranchNode *node = pb->vt->update(pb->pager, page);
            assert(node != NULL);
            rc = 0;
            {
                split . left = nid;
                
                /* if we are also full, we have to split */
                if ( branch_node_full ( node, split.ksize ) )
                {
                    /* create another branch node to right */
                    void const *dup = pb->vt->alloc(pb->pager, &rsplit->right);
                    if (dup == NULL)
                        rc = RC ( rcDB, rcTree, rcInserting, rcMemory, rcExhausted );
                    else {
                        void *mem = pb->vt->update(pb->pager, dup);
                        assert(mem != NULL);
                        {
                            split_branch( node, mem, & split, rsplit, upper);
                            rc = RC ( rcDB, rcTree, rcInserting, rcId, rcInsufficient );
                            rsplit -> right += rsplit -> right + 1;
                        }
                        pb->vt->unuse(pb->pager, dup);
                    }
                }
                else
                {
                    int pl;/**common prefix_len **/
                    /* hoist into location */
                    /* it will increase node->count and upper will start pointing into a new key ***/
                    if(rc==0 && upper > 0 ){ /*** left side compact is possible  upper is 1 based ***/
                        uint8_t   *a = (uint8_t*)split.key;
                        uint8_t   *b = (uint8_t*)node;
                        
                        a += node->key_prefix_len;
                        b += node->ord[upper-1].key;
                        assert ( node->ord[upper-1].trans == split.left );
                        for(pl=0;pl < split.ksize - node->key_prefix_len && pl < node->ord[upper-1].ksize && a[pl]==b[pl];pl++){}
                        pl+=node->key_prefix_len;
                        if(pl >= MIN_PREFIX_TO_COMPACT){
                            rc=compact_page(pb,split.left,pl);
                        }
                    }
                    if(rc==0 && upper < node->count ){ /*** right side compact is possible ***/
                        uint8_t   *a = (uint8_t*)split.key;
                        uint8_t   *b = (uint8_t*)node;
                        a += node->key_prefix_len;
                        b += node->ord[upper].key;
                        for(pl=0;pl < split.ksize - node->key_prefix_len && pl < node->ord[upper].ksize && a[pl]==b[pl];pl++){}
                        pl+=node->key_prefix_len;
                        if(pl >= MIN_PREFIX_TO_COMPACT){
                            rc=compact_page(pb,split.right,pl);
                        }
                    }
                    if(rc == 0 ){
                        rc=branch_insert ( node, & split, upper );
                        assert(rc==0);
                    }
                }
            }
        }
        
        SplitWhack ( & split );
        
        pb->vt->unuse(pb->pager, child);
    }
    
    return rc;
}

static rc_t tree_entry(EntryData *pb)
{
    void const *page;
    rc_t rc;
    Split split;
    
    if (pb->root == 0) {
        uint32_t new_id = 0;
        
        page = pb->vt->alloc(pb->pager, &new_id);
        if (page == NULL)
            return RC ( rcDB, rcTree, rcInserting, rcMemory, rcExhausted );
        pb->root = new_id << 1;
    }
    else {
        page = pb->vt->use(pb->pager, pb->root >> 1);
        assert(page != NULL);
    }
    SplitInit(&split);
    
    rc = (((pb->root & 1) == 0) ? leaf_entry : branch_entry)(pb, page, &split);
    /* detect split */
    if ( GetRCState ( rc ) == rcInsufficient && GetRCObject ( rc ) == rcId ) {
        void const *new_root;

        rc = 0;
        split.left = pb->root;
        new_root = pb->vt->alloc(pb->pager, &pb->root);
        if (new_root) {
            BranchNode *node = pb->vt->update(pb->pager, new_root);
            assert(node != NULL);
            {
                uint16_t	q,i;
                /* install the new root */
                node -> key_bytes = split . ksize + sizeof ( uint32_t );
                node -> key_prefix_len = 0;
                node -> key_prefix = 0;
                node -> ord [ 0 ] . ksize = split . ksize;
                node -> ord [ 0 ] . key = ( uint16_t ) ( PGSIZE - node -> key_bytes );
                memmove ( & ( ( uint8_t* ) node ) [ PGSIZE - node -> key_bytes ], split . key, node -> key_bytes );
                node -> ltrans = split . left;
                node -> ord [ 0 ] . trans = split . right;
                node -> count = 1;
                /* 				assert ( split . left < 10000 && split . right < 10000); */
                /*** init search windows ***/
                q=(split.ksize>0)? *(uint8_t*)split.key:0;
                node -> win[q].lower = 0;
                node -> win[q].upper = 1;
                for(i=0;i<q;i++){
                    node -> win[i].lower = 0;
                    node -> win[i].upper = 0;
                }
                for(i=q+1;i<256;i++){
                    node -> win[i].lower = 1;
                    node -> win[i].upper = 1;
                }
                pb->root = (pb->root << 1) | 1;
            }
            pb->vt->unuse(pb->pager, new_root);
        }
        else
            rc = RC ( rcDB, rcTree, rcInserting, rcMemory, rcExhausted );
    }
    SplitWhack(&split);
    pb->vt->unuse(pb->pager, page);
    return rc;
}

LIB_EXPORT rc_t CC BTreeEntry ( uint32_t *root, Pager *pager, Pager_vt const *vt, uint32_t *id,
                               bool *was_inserted, const void *key, size_t key_size )
{
    assert(root != NULL);
    assert(vt != NULL);
    assert(id != NULL);
    assert(was_inserted != NULL);
    assert(key != NULL);
    assert(key_size != 0);
    {
        EntryData pb;
        
        pb.pager = pager;
        pb.vt = vt;
        pb.root = *root;
        pb.id = id;
        pb.key = key;
        pb.key_size = key_size;
        pb.was_inserted = false;
        {
            rc_t const rc = tree_entry(&pb);
            
            *root = pb.root;
            *was_inserted = pb.was_inserted;
            return rc;
        }
    }
}


/* ForEach
 *  executes a function on each tree element
 *
 *  "reverse" [ IN ] - if true, iterate in reverse order
 *
 *  "f" [ IN ] and "data" [ IN, OPAQUE ] - callback function
 */
static void invoke_foreach_func ( void const *const cnode, void const *const ordp,
                                 void ( CC * f ) ( const void *key, size_t key_size, uint32_t id, void *data ), void *data )
{
    LeafEntry const *const ord = ordp;
    uint8_t const *const page = cnode;
    uint8_t const *const key = &page[ord->key];
    size_t const key_size = ord->ksize;
    uint32_t val_id;
    
    memmove(&val_id, &key[key_size], 4);
    f(key, key_size, val_id, data);
}

static void foreach_leaf_reverse(uint32_t nodeid, Pager *pager, Pager_vt const *vt,
                                 void ( CC * f ) ( const void *key, size_t key_size, uint32_t id, void *data ), void *data )
{
    void const *const page = vt->use(pager, nodeid);
    assert(page != NULL);
    {
        unsigned i;
    LeafNode const *const node = vt->access(pager, page);
    assert(node != NULL);
    
    for (i = node->count; i > 0; ) {
        invoke_foreach_func(node, &node->ord[--i], f, data);
    }
    }
    vt->unuse(pager, page);
}

static void foreach_branch_reverse(uint32_t nodeid, Pager *pager, Pager_vt const *vt,
                                   void ( CC * f ) ( const void *key, size_t key_size, uint32_t id, void *data ), void *data )
{
    void const *const page = vt->use(pager, nodeid);
    assert(page != NULL);
    {
        unsigned i;
    BranchNode const *const node = vt->access(pager, page);
    assert(node != NULL);
    
    for (i = node->count; i > 0; ) {
        uint32_t const child = node->ord[--i].trans;
        
        invoke_foreach_func(node, &node->ord[i], f, data);
        if (child & 1) {
            foreach_branch_reverse(child >> 1, pager, vt, f, data);
        }
        else {
            foreach_leaf_reverse(child >> 1, pager, vt, f, data);
        }
    }
    }
    vt->unuse(pager, page);
}

static void foreach_reverse(uint32_t root, Pager *pager, Pager_vt const *vt,
                            void ( CC * f ) ( const void *key, size_t key_size, uint32_t id, void *data ), void *data )
{
    if (root & 1) {
        foreach_branch_reverse(root >> 1, pager, vt, f, data);
    }
    else {
        foreach_leaf_reverse(root >> 1, pager, vt, f, data);
    }
}

static void foreach_leaf(uint32_t nodeid, Pager *pager, Pager_vt const *vt,
                         void ( CC * f ) ( const void *key, size_t key_size, uint32_t id, void *data ), void *data )
{
    void const *const page = vt->use(pager, nodeid);
    assert(page != NULL);
    {
        unsigned i;
    LeafNode const *const node = vt->access(pager, page);
    assert(node != NULL);
    
    for (i = 0; i < node->count; ++i) {
        invoke_foreach_func(node, &node->ord[i], f, data);
    }
    }
    vt->unuse(pager, page);
}

static void foreach_branch(uint32_t nodeid, Pager *pager, Pager_vt const *vt,
                           void ( CC * f ) ( const void *key, size_t key_size, uint32_t id, void *data ), void *data )
{
    void const *const page = vt->use(pager, nodeid);
    assert(page != NULL);
    {
        unsigned i;
    BranchNode const *const node = vt->access(pager, page);
    assert(node != NULL);
    
    for (i = 0; i < node->count; ++i) {
        uint32_t const child = node->ord[i].trans;
        
        invoke_foreach_func(node, &node->ord[i], f, data);
        if (child & 1) {
            foreach_branch(child >> 1, pager, vt, f, data);
        }
        else {
            foreach_leaf(child >> 1, pager, vt, f, data);
        }
    }
    }
    vt->unuse(pager, page);
}

static void foreach(uint32_t root, Pager *pager, Pager_vt const *vt,
                    void ( CC * f ) ( const void *key, size_t key_size, uint32_t id, void *data ), void *data )
{
    if (root & 1) {
        foreach_branch(root >> 1, pager, vt, f, data);
    }
    else {
        foreach_leaf(root >> 1, pager, vt, f, data);
    }
}

LIB_EXPORT rc_t CC BTreeForEach ( uint32_t root, Pager *pager, Pager_vt const *vt, bool reverse,
                                 void ( CC * f ) ( const void *key, size_t key_size, uint32_t id, void *data ), void *data )
{
    if (vt != NULL && root != 0 && f != NULL) {
        if (reverse) {
            foreach_reverse(root, pager, vt, f, data);
        }
        else {
            foreach(root, pager, vt, f, data);
        }
    }
    return 0;
}
