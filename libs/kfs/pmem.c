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

#include <kfs/extern.h>
#include <kfs/file.h>
#include <kfs/pmem.h>
#include <klib/refcount.h>
#include <klib/rc.h>
#include <klib/log.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define NUM_HOT_ENTRIES ((1024u * 1024u))

typedef uint64_t BlockID;
typedef union HotEntry HotEntry;
typedef uint64_t Offset;
typedef int64_t Size;

struct HotEntryInUse {
    int64_t size;  /* > 0 */
    BlockID id;
    void *storage;
};

struct HotEntryFree {
    int64_t size; /* == 0 */
    BlockID id;
    unsigned next;
};

union HotEntry {
    struct HotEntryInUse inuse;
    struct HotEntryFree free;
};

typedef struct ColdPageLeaf ColdPageLeaf;
struct ColdPageLeaf {
    Offset offset;
    Size size;
};

typedef union ColdPageNode ColdPageNode;
union ColdPageNode {
    ColdPageNode *next;
    ColdPageLeaf *leaf;
};

/*--------------------------------------------------------------------------
 * KMemBank
 *  Uses a generational allocator.
 *  A fixed number of blocks are managed by malloc/free.
 *  Old blocks are written to a file and free'd.
 *
 *  The two envisioned usage patterns are:
 *    1) blocks which are written, read, and freed 'quickly'.
 *    2) blocks which are written, not used for 'a while', and then accessed all
 *       at once, in the order they were created.
 *
 *  No attempt is made to reclaim or reuse the space in the file.
 */
struct KMemBank
{
    KFile *pf;

    ColdPageNode coldStorage[64u * 1024u];

    KRefcount refcount;

    BlockID next;
    uint64_t fileNext;
    unsigned nextFree;
    HotEntry hot[NUM_HOT_ENTRIES];
};

static KMemBank *newMemBank(void)
{
    KMemBank *self = calloc(1, sizeof(*self));

    if (self) {
        unsigned i;

        for (i = 0; i < NUM_HOT_ENTRIES; ++i) {
            self->hot[i].free.size = (int64_t)(-1);
            self->hot[i].free.id = i + 1;
            self->hot[i].free.next = i + 1;
        }
        self->next = NUM_HOT_ENTRIES;

        KRefcountInit(&self->refcount, 1, "KMemBank", "new", "bank");
        return self;
    }
    return NULL;
}

static void freeColdStorage(ColdPageNode *const node, int const depth);

/* Whack
 */
static
void KMemBankWhack ( KMemBank *self )
{
    freeColdStorage(self->coldStorage, 0);
    KFileRelease(self->pf);
    free(self);
}

/* Make
 *  make a memory bank
 *
 *  "block_size" [ IN ] - ignored
 *
 *  "limit" [ IN, DFLT ZERO ] - ignored
 *
 *  "backing" [ IN ] - required.
 */
LIB_EXPORT rc_t CC KMemBankMake ( KMemBank **bankp,
    size_t block_size, uint64_t limit, KFile *backing )
{
    assert(backing != NULL);
    assert(bankp != NULL);
    if (bankp == NULL || backing == NULL) {
        PLOGMSG(klogFatal, (klogFatal, "PROGRAMMER ERROR - " __FILE__ ":$(line)", "line=%i", __LINE__));
        abort();
        return RC(rcFS, rcMemory, rcConstructing, rcParam, rcNull);
    }
    else {
        KMemBank *const self = newMemBank();
        if (self == NULL) {
            PLOGMSG(klogFatal, (klogFatal, "OUT OF MEMORY - " __FILE__ ":$(line)", "line=%i", __LINE__));
            abort(); 
            return RC(rcFS, rcMemory, rcConstructing, rcMemory, rcExhausted);
        }
        if (backing) {
            self->pf = backing;
            KFileAddRef(self->pf);
        }
        *bankp = self;
        return 0;
    }
}


/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC KMemBankAddRef(const KMemBank *cself)
{
    KMemBank *const self = (KMemBank *)cself;
    if (self != NULL) {
        int const rslt = KRefcountAdd(&self->refcount, "KMemBank");
        assert(rslt == krefOkay);
        if (rslt != krefOkay) {
            PLOGMSG(klogFatal, (klogFatal, "LOGIC ERROR - " __FILE__ ":$(line)", "line=%i", __LINE__));
            abort();
            return RC(rcFS, rcMemory, rcAttaching, rcConstraint, rcViolated);
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC KMemBankRelease(const KMemBank *cself)
{
    KMemBank *const self = (KMemBank *)cself;
    if (self != NULL) {
        int const rslt = KRefcountDrop(&self->refcount, "KMemBank");
        switch(rslt) {
        case krefWhack:
            KMemBankWhack(self);
            break;
        case krefOkay:
            break;
        default:
            PLOGMSG(klogFatal, (klogFatal, "LOGIC ERROR - " __FILE__ ":$(line)", "line=%i", __LINE__));
            abort();
            return RC(rcFS, rcMemory, rcAttaching, rcConstraint, rcViolated);
        }
    }
    return 0;
}

static HotEntry *findBlock(KMemBank const *const self, BlockID const id)
{
    unsigned const slot = (id - 1) % NUM_HOT_ENTRIES;
    HotEntry *const entry = (HotEntry *)&self->hot[slot];

    return entry->inuse.id == id ? entry : NULL;
}

struct BlockID_ext {
    int node[3];
    int leaf;
};

static struct BlockID_ext extendBlockID(BlockID const id)
{
    struct BlockID_ext rslt;
    rslt.node[0] = ((id - 1) >> 48) & 0xFFFF; /* -> node[65536] */
    rslt.node[1] = ((id - 1) >> 32) & 0xFFFF; /* -> node[65536] */
    rslt.node[2] = ((id - 1) >> 16) & 0xFFFF; /* -> leaf[65536] */
    rslt.leaf    = ((id - 1) >>  0) & 0xFFFF; /* -> leaf      */
    return rslt;
}

static void freeColdStorage(ColdPageNode *const node, int const depth)
{
    int i;
    for (i = 0; i < 0x10000; ++i) {
        if (node[i].next != NULL && depth < 2) {
            freeColdStorage(node[i].next, depth + 1);
        }
        free(node[i].next);
    }
}

static ColdPageLeaf *findColdBlock(KMemBank const *const self, BlockID const id)
{
    ColdPageNode const *node = self->coldStorage;
    struct BlockID_ext const ext = extendBlockID(id);
    int i;

    for (i = 0; i < 2; ++i) {
        int const j = ext.node[i];
        if (node[j].next != NULL)
            node = node[j].next;
        else
            return NULL;
    }
    {
        ColdPageLeaf *const leaf = node[ext.node[2]].leaf;
        return leaf == NULL ? NULL : &leaf[ext.leaf];
    }
}

static rc_t transferToColdStorage(KMemBank *const self, BlockID const id, Offset const offset, Size const size)
{
    ColdPageNode *node = self->coldStorage;
    struct BlockID_ext const ext = extendBlockID(id);
    int i;

    for (i = 0; i < 2; ++i) {
        int const j = ext.node[i];
        if (node[j].next != NULL)
            node = node[j].next;
        else {
            node[j].next = calloc(64u * 1024u, sizeof(ColdPageNode));
            if (node[j].next != NULL)
                node = node[j].next;
            else
                return RC(rcFS, rcMemory, rcAllocating, rcMemory, rcExhausted);
        }
    }

    if (node[ext.node[2]].leaf == NULL) {
        node[ext.node[2]].leaf = calloc(64u * 1024u, sizeof(ColdPageLeaf));
        if (node[ext.node[2]].leaf == NULL) {
            return RC(rcFS, rcMemory, rcAllocating, rcMemory, rcExhausted);
        }
    }
    {
        ColdPageLeaf *const leaf = node[ext.node[2]].leaf;
        leaf[ext.leaf].offset = offset;
        leaf[ext.leaf].size = size;
    }
    return 0;
}

static rc_t freeColdBlock(KMemBank *const self, BlockID const id)
{
    ColdPageLeaf *const leaf = findColdBlock(self, id);

    if (leaf && leaf->size > 0) {
        leaf->size = -leaf->size;
        return 0;
    }
    if (leaf == NULL || leaf->size == 0) {
        PLOGMSG(klogFatal, (klogFatal, "PROGRAMMER ERROR - BLOCK WAS NEVER ALLOCATED - " __FILE__ ":$(line)", "line=%i", __LINE__));
        abort();
        return RC(rcFS, rcMemory, rcDestroying, rcId, rcNotFound);
    }
    else {
        PLOGMSG(klogFatal, (klogFatal, "PROGRAMMER ERROR - DOUBLE FREE - " __FILE__ ":$(line)", "line=%i", __LINE__));
        abort();
        return RC(rcFS, rcMemory, rcDestroying, rcId, rcInvalid);
    }
}

static rc_t freeBlock(KMemBank *const self, BlockID const id)
{
    HotEntry *const entry = findBlock(self, id);
    if (entry) {
        struct HotEntryInUse const inuse = entry->inuse;
        struct HotEntryFree *const pfree = &entry->free;

        if (inuse.size == 0) {
            PLOGMSG(klogFatal, (klogFatal, "PROGRAMMER ERROR - DOUBLE FREE - " __FILE__ ":$(line)", "line=%i", __LINE__));
            abort();
            return RC(rcFS, rcMemory, rcDestroying, rcId, rcInvalid);
        }
        free(inuse.storage);
        memset(pfree, 0, sizeof(*pfree));
        pfree->size = (int64_t)(-1);
        pfree->id = id;
        pfree->next = self->nextFree;
        self->nextFree = entry - self->hot;
        return 0;
    }
    else
        return freeColdBlock(self, id);
}

static struct HotEntryInUse *newBlock(KMemBank *const self, rc_t *const rc)
{
    struct HotEntryInUse *inuse = NULL;
    
    if (self->nextFree < NUM_HOT_ENTRIES) {
        HotEntry *const entry = &self->hot[self->nextFree];
        struct HotEntryFree const free = entry->free;

        assert(free.size < 0); /* it really is free */

        self->nextFree = free.next;
        inuse = &entry->inuse;
        memset(inuse, 0, sizeof(*inuse));
        inuse->id = free.id;
    }
    else {
        BlockID const id = ++self->next;
        unsigned const slot = (id - 1) % NUM_HOT_ENTRIES;
        size_t written = 0;

        inuse = &self->hot[slot].inuse;
        assert(inuse->size >= 0); /* it really is inuse */

        while (written < inuse->size) {
            size_t const n = inuse->size - written;
            size_t w = 0;
            *rc = KFileWrite(self->pf, self->fileNext + written, ((char const *)inuse->storage) + written, n, &w);
            if (*rc != 0) return NULL;
            written += w;
        }

        *rc = transferToColdStorage(self, inuse->id, self->fileNext, inuse->size);
        if (*rc != 0) return NULL;

        self->fileNext += inuse->size;
        free(inuse->storage);
        memset(inuse, 0, sizeof(*inuse));
        inuse->id = id;
    }
    return inuse;
}

/* Alloc
 *  allocate memory
 *
 *  "id" [ OUT ] - return parameter for memory allocation id
 *
 *  "bytes" [ IN ] - the number of bytes to allocate
 *
 *  "clear" [ IN ] - ignored, memory is zero'ed
 */
LIB_EXPORT rc_t CC KMemBankAlloc ( KMemBank *self,
    uint64_t *id, uint64_t bytes, bool clear )
{
    rc_t rc = 0;
    struct HotEntryInUse *new_block = NULL;

    assert(self != NULL);
    assert(id != NULL);
    if (self == NULL) {
        PLOGMSG(klogFatal, (klogFatal, "PROGRAMMER ERROR - " __FILE__ ":$(line)", "line=%i", __LINE__));
        abort();
        return RC(rcFS, rcMemory, rcAllocating, rcSelf, rcNull);
    }
    if (id == NULL) {
        PLOGMSG(klogFatal, (klogFatal, "PROGRAMMER ERROR - " __FILE__ ":$(line)", "line=%i", __LINE__));
        abort();
        return RC(rcFS, rcMemory, rcAllocating, rcParam, rcNull);
    }
    if (bytes > INT64_MAX)
        return RC(rcFS, rcMemory, rcResizing, rcSize, rcExcessive);
    if (bytes == 0)
        return RC(rcFS, rcMemory, rcResizing, rcSize, rcInvalid);

    new_block = newBlock(self, &rc); if (rc) return rc;
    new_block->storage = calloc(1, bytes);
    if (new_block->storage == NULL) {
        PLOGMSG(klogFatal, (klogFatal, "OUT OF MEMORY - " __FILE__ ":$(line)", "line=%i", __LINE__));
        abort();
        return RC(rcFS, rcMemory, rcAllocating, rcMemory, rcExhausted);
    }
    new_block->size = bytes;
    *id = new_block->id;
    return 0;
}


/* Free
 *  free memory
 *
 *  "id" [ IN, ZERO OKAY ] - id of the allocation to be freed
 */
LIB_EXPORT rc_t CC KMemBankFree ( KMemBank *self, uint64_t id )
{
    if ( id == 0 )
        return 0;

    assert(self != NULL);
    if (self == NULL) {
        PLOGMSG(klogFatal, (klogFatal, "PROGRAMMER ERROR - " __FILE__ ":$(line)", "line=%i", __LINE__));
        abort();
        return RC(rcFS, rcMemory, rcAllocating, rcSelf, rcNull);
    }
    return freeBlock(self, id);
}


/* Size
 *  returns the allocated capacity of the memory object
 *
 *  "id" [ IN ] - id of the allocation
 *
 *  "size" [ OUT ] - return parameter
 */
LIB_EXPORT rc_t CC KMemBankSize ( const KMemBank *self, uint64_t id, uint64_t *size )
{
    assert(self != NULL);
    assert(size != NULL);
    if (self == NULL) {
        PLOGMSG(klogFatal, (klogFatal, "PROGRAMMER ERROR - " __FILE__ ":$(line)", "line=%i", __LINE__));
        abort();
        return RC(rcFS, rcMemory, rcAccessing, rcSelf, rcNull);
    }
    if (size == NULL) {
        PLOGMSG(klogFatal, (klogFatal, "PROGRAMMER ERROR - " __FILE__ ":$(line)", "line=%i", __LINE__));
        abort();
        return RC(rcFS, rcMemory, rcAccessing, rcParam, rcNull);
    }
    else {
        HotEntry const *const h = findBlock(self, id);
        if (h) {
            *size = h->inuse.size;
            return 0;
        }
        else {
            ColdPageLeaf const *const l = findColdBlock(self, id);
            *size = (l && l->size > 0) ? l->size : 0;
            return 0;
        }
    }
}


/* SetSize
 *  performs a reallocation of memory object
 *
 *  "id" [ IN ] - id of the allocation being resized
 *
 *  "bytes" [ IN ] - new capacity
 */
LIB_EXPORT rc_t CC KMemBankSetSize ( KMemBank *self, uint64_t id, uint64_t bytes )
{
    assert(self != NULL);
    assert(id != 0);
    if (self == NULL) {
        PLOGMSG(klogFatal, (klogFatal, "PROGRAMMER ERROR - " __FILE__ ":$(line)", "line=%i", __LINE__));
        abort();
        return RC(rcFS, rcMemory, rcResizing, rcSelf, rcNull);
    }
    if (id == 0) {
        PLOGMSG(klogFatal, (klogFatal, "PROGRAMMER ERROR - " __FILE__ ":$(line)", "line=%i", __LINE__));
        abort();
        return RC(rcFS, rcMemory, rcResizing, rcId, rcInvalid);
    }
    if (bytes > INT64_MAX)
        return RC(rcFS, rcMemory, rcResizing, rcSize, rcExcessive);
    if (bytes == 0)
        return RC(rcFS, rcMemory, rcResizing, rcSize, rcInvalid);
    else {
        HotEntry *const h = findBlock(self, id);
        if (h && h->inuse.size > 0) {
            void *const tmp = realloc(h->inuse.storage, bytes);
            if (tmp) {
                h->inuse.storage = tmp;
                h->inuse.size = bytes;
                return 0;
            }
            return RC(rcFS, rcMemory, rcResizing, rcMemory, rcExhausted);
        }
        else
            return RC(rcFS, rcMemory, rcResizing, rcMode, rcUnsupported);
    }
}


/* Read
 *  linearly access memory
 *
 *  "id" [ IN ] - id of the allocation being read
 *
 *  "pos" [ IN ] - starting offset into allocation
 *
 *  "buffer" [ IN ] and "bsize" [ IN ] - buffer for returned data
 *
 *  "num_read" [ OUT ] - return parameter for the number of bytes read
 */
LIB_EXPORT rc_t CC KMemBankRead ( const KMemBank *self, uint64_t id,
   uint64_t pos, void *buffer, size_t bsize, size_t *num_read )
{
    assert(self != NULL);
    assert(num_read != NULL);
    assert(id != 0);
    if (self == NULL) {
        PLOGMSG(klogFatal, (klogFatal, "PROGRAMMER ERROR - " __FILE__ ":$(line)", "line=%i", __LINE__));
        abort();
        return RC(rcFS, rcMemory, rcReading, rcSelf, rcNull);
    }
    if (id == 0) {
        PLOGMSG(klogFatal, (klogFatal, "PROGRAMMER ERROR - " __FILE__ ":$(line)", "line=%i", __LINE__));
        abort();
        return RC(rcFS, rcMemory, rcReading, rcId, rcInvalid);
    }
    if (num_read == NULL) {
        PLOGMSG(klogFatal, (klogFatal, "PROGRAMMER ERROR - " __FILE__ ":$(line)", "line=%i", __LINE__));
        abort();
        return RC(rcFS, rcMemory, rcReading, rcParam, rcNull);
    }
    else {
        HotEntry const *const h = findBlock(self, id);
        if (h) {
            char const *in = (char *)h->inuse.storage + pos;
            char const *const endp = (char *)h->inuse.storage + h->inuse.size;

            if (endp > in) {
                size_t const n = (endp - in) < bsize ? (endp - in) : bsize;
                memmove(buffer, in, n);
                *num_read = n;
            }
            return 0;
        }
        else {
            ColdPageLeaf const *const l = findColdBlock(self, id);

            if (l == NULL || l->size <= 0)
                return RC(rcFS, rcMemory, rcReading, rcId, rcNotFound);

            return KFileRead(self->pf, l->offset + pos, buffer, bsize, num_read);
        }
    }
}


/* Write
 *  linearly update memory
 *  will resize as required
 *
 *  "id" [ IN ] - id of the allocation being writ
 *
 *  "pos" [ IN ] - starting offset into allocation
 *
 *  "buffer" [ IN ] and "size" [ IN ] - data to write
 *
 *  "num_writ" [ OUT ] - return parameter for the number of bytes written
 */
LIB_EXPORT rc_t CC KMemBankWrite ( KMemBank *self, uint64_t id,
    uint64_t pos, const void *buffer, size_t size, size_t *num_writ )
{
    assert(self != NULL);
    assert(num_writ != NULL);
    assert(id != 0);
    if (self == NULL) {
        PLOGMSG(klogFatal, (klogFatal, "PROGRAMMER ERROR - " __FILE__ ":$(line)", "line=%i", __LINE__));
        abort();
        return RC(rcFS, rcMemory, rcReading, rcSelf, rcNull);
    }
    if (id == 0) {
        PLOGMSG(klogFatal, (klogFatal, "PROGRAMMER ERROR - " __FILE__ ":$(line)", "line=%i", __LINE__));
        abort();
        return RC(rcFS, rcMemory, rcReading, rcId, rcInvalid);
    }
    if (num_writ == NULL) {
        PLOGMSG(klogFatal, (klogFatal, "PROGRAMMER ERROR - " __FILE__ ":$(line)", "line=%i", __LINE__));
        abort();
        return RC(rcFS, rcMemory, rcReading, rcParam, rcNull);
    }
    else {
        HotEntry *const h = findBlock(self, id);
        if (h) {
            if (pos + size > h->inuse.size) {
                void *const tmp = realloc(h->inuse.storage, pos + size);
                if (tmp) {
                    h->inuse.storage = tmp;
                    h->inuse.size = pos + size;
                }
                else
                    return RC(rcFS, rcMemory, rcResizing, rcMemory, rcExhausted);
            }
            memmove(((char *)h->inuse.storage) + pos, buffer, size);
            *num_writ = size;
            return 0;
        }
        else
            return RC(rcFS, rcMemory, rcWriting, rcMode, rcUnsupported);
    }
}
