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
#include <kdb/index.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/md5.h>
#include <kfs/mmap.h>
#include <klib/hashtable.h>
#include <klib/text.h>
#include <klib/pack.h>
#include <klib/rc.h>
#include <os-native.h>
#include <sysalloc.h>
#include <byteswap.h>

#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "index-cmn.h"
#include "htidx-priv.h"

#define LOADING_FACTOR (0.618) /* PHI - 1 */

/** @brief: free a buffer
 **
 ** @return: the previous entry in the chain, or NULL
 **/
static BufferListEntry *freeBufferListEntry(BufferListEntry *const self)
{
    BufferListEntry *const chain = self->prv;
    
    free(self->buffer);
    free(self);
    
    return chain;
}

/** @brief: free the resources, but not self
 **/
rc_t KHTIndexWhack_v5(KHTIndex_v5 *const self)
{
    BufferListEntry *current = self->current;

    if (self->hashtable)
        KHashTableDispose(self->hashtable, NULL, NULL, NULL);
    free(self->entries);
    
    while (current != NULL)
        current = freeBufferListEntry(current);
    return 0;
}

/** @brief: create a hashtable and add the existing keys to it
 **/
static bool initializeRead(KHTIndex_v5 *const self, rc_t *rc)
{
    size_t i;
    
    *rc = KHashTableMake(&self->hashtable, sizeof(char *), sizeof(size_t), self->keyCount, LOADING_FACTOR, KHT_key_type_cstr);
    if (*rc) return false;

    for (i = 0; i < self->keyCount; ++i) {
        if (addEntryToHashTable(self, i, rc))
            continue;
        return false;
    }
    return true;
}

/** @brief: look up key in the index
 **/
rc_t KHTIndexFind_v5 ( KHTIndex_v5 const *const self
                     , char const *const key
                     , int64_t *const out_firstId
                     , uint32_t *const out_span
                     )
{
    rc_t rc = 0;
    size_t keylen;
    uint64_t fnd = 0;
    IdMapEntry const *entry = NULL;

    assert(self != NULL);
    assert(key != NULL);

    if (self->hashtable == NULL) {
        if (!initializeRead((KHTIndex_v5 *)self, &rc))
            return rc;
    }
    assert(self->hashtable != NULL);
    
    keylen = cstring_byte_count(key);
    if (!KHashTableFind(self->hashtable, key, KHash(key, keylen), &fnd))
        return RC(rcDB, rcIndex, rcSelecting, rcString, rcNotFound);
  
    assert(fnd < self->keyCount);
    entry = self->entries + fnd;
    if (out_firstId)
        *out_firstId = entry->firstId;
    if (out_span)
        *out_span = 1 + (entry->lastId - entry->firstId);

    return 0;
}

/** @brief: is the index 1:1 and is the ID space contiguous
 **/
static bool isOntoAndGapless(KHTIndex_v5 const *const self)
{
    return self->numId == self->keyCount
        && self->numId == 1 + (self->prvId - self->minId);
}

/** @brief: retrieve the key that was associated with an id
 **
 ** @note: O(log n) at worst, O(1) if index is 1:1 and the
 **        ID space is contiguous.
 **/
rc_t KHTIndexProject_v5 ( KHTIndex_v5 const *const self
                        , int64_t const id
                        , char const **const key
                        , int64_t *const first_id
                        , uint32_t *const span
                        )
{
    IdMapEntry const *fnd = NULL;

    assert(self != NULL);
    
    if (self->minId <= id && id <= self->prvId) {
        if (isOntoAndGapless(self)) {
            /* can do a linear lookup by id */
            size_t const i = id - self->minId;
            IdMapEntry const *const entry = self->entries + i;
            
            assert(i < self->keyCount);
            assert(id == entry->firstId);
            assert(entry->lastId == entry->firstId);

            fnd = entry;
            goto FOUND;
        }
        else {
            /* keys map to a range of IDs
             * or there are gaps in the ID range 
             * do a binary search
             */
            size_t f = 0;
            size_t e = self->keyCount;

            if (f < e) {
                /* make an initial guess based on the assumption that
                 * the number of IDs per key is roughly constant
                 */
                uint64_t const span = 1 + (self->prvId - self->minId);
                size_t const initial_guess = self->keyCount * ((id - self->minId) / (double)span);
                if (initial_guess < self->keyCount) {
                    IdMapEntry const *const entry = self->entries + initial_guess;

                    if (entry->lastId < id)
                        f = initial_guess + 1;
                    else if (id < entry->firstId)
                        e = initial_guess;
                    else {
                        /* LUCKY GUESS! */
                        fnd = entry;
                        goto FOUND;
                    }
                }
            }
            while (f < e && fnd == NULL) {
                size_t const m = f + ((e - f) >> 1);
                IdMapEntry const *const entry = self->entries + m;

                if (entry->lastId < id) {
                    f = m + 1;
                    continue;
                }
                if (id < entry->firstId) {
                    e = m;
                    continue;
                }
                fnd = entry;
                goto FOUND;
            }
        }
    }
    if (fnd == NULL)
        return RC(rcDB, rcIndex, rcProjecting, rcId, rcNotFound);
    else {
    FOUND:
        assert(fnd->firstId <= id && id <= fnd->lastId);
        
        if (key     ) *key      = fnd->name;
        if (first_id) *first_id = fnd->firstId;
        if (span    ) *span     = 1 + (fnd->lastId - fnd->firstId);

        return 0;
    }
}

static bool deserialize(KHTIndex_v5 *const self
                       , void const *buffer
                       , rc_t *rc
                       )
{
    IndexHeader const *const hdr = buffer;
    uint8_t const *const cmp_id = (void const *)(hdr + 1);
    uint8_t const *const cmp_span = cmp_id + hdr->cmp_idsize;
    uint8_t const *const cmp_keys = cmp_span + hdr->cmp_spansize;
    size_t const keysize = hdr->keysize;
    
    self->keyCount = hdr->entries;
    self->numId = hdr->numIds;
    self->minId = hdr->firstId;
    self->prvId = hdr->lastId;
    
    self->entries = malloc(self->keyCount * sizeof(self->entries[0]));
    if (self->entries == NULL) {
        *rc = RC(rcDB, rcIndex, rcReading, rcMemory, rcExhausted);
        return false;
    }
    self->maxKeys = self->keyCount;
    
    self->current = calloc(1, sizeof(*self->current));
    if (self->current == NULL) {
        *rc = RC(rcDB, rcIndex, rcReading, rcMemory, rcExhausted);
        return false;
    }
    self->current->buffer = malloc(keysize);
    if (self->current->buffer == NULL) {
        *rc = RC(rcDB, rcIndex, rcReading, rcMemory, rcExhausted);
        return false;
    }
    self->current->prv = NULL;
    self->current->used = keysize;
    if (hdr->cmp_keysize == keysize)
        memmove(self->current->buffer, cmp_keys, keysize);
    else {
        /* decompress */
        assert(!"implemented");
        abort();
    }
    if (hdr->cmp_idsize == 0 && hdr->cmp_spansize == 0) {
        size_t i;
        for (i = 0; i < self->keyCount; ++i) {
            self->entries[i].firstId = self->entries[i].lastId = self->minId + i;
        }
        assert(self->minId + i == self->prvId + 1);
    }
    else {
        if (   hdr->cmp_idsize != sizeof(uint64_t) * self->keyCount
            || hdr->cmp_spansize != sizeof(uint32_t) * self->keyCount)
        {
            /* decompress */
            assert(!"implemented");
            abort();
        }
        else {
            uint64_t const *const id = (void const *)cmp_id;
            uint32_t const *const span = (void const *)cmp_span;
            size_t i;
            for (i = 0; i < self->keyCount; ++i) {
                self->entries[i].lastId = span[i];
            }
            for (i = 0; i < self->keyCount; ++i) {
                int64_t const first = id[i] + self->minId;
                self->entries[i].firstId = first;
                self->entries[i].lastId += first;
            }
        }
    }
    {
        size_t i;
        char const *key = self->current->buffer;
        for (i = 0; i < self->keyCount; ++i) {
            self->entries[i].name = key;
            while (*key != '\0')
                ++key;
            ++key;
        }
    }
    return true;
}

rc_t KHTIndexOpen_v5( KHTIndex_v5 *const self
                    , KMMap const *map
                    , bool byteswap
                    )
{
    if (!byteswap) {
        rc_t rc = 0;
        
        if (map != NULL) {
            void const *mem = NULL;
            rc = KMMapAddrRead(map, &mem);
            if (rc == 0) deserialize(self, mem, &rc);
        }
        return rc;
    }
    return RC(rcDB, rcIndex, rcReading, rcByteOrder, rcInvalid);
}

rc_t KHTIndexCheckConsistency ( KHTIndex_v5 const *const self
                              , int64_t *const start_id
                              , uint64_t *const id_range
                              , uint64_t *const num_keys
                              , uint64_t *const num_rows
                              , uint64_t *const num_holes
                              , bool const key2id
                              , bool const id2key
                              , bool const id_all
                              )
{
    size_t i;

    if (start_id != NULL) start_id[0] = self->minId;
    if (id_range != NULL) id_range[0] = self->prvId - self->minId;
    if (num_keys != NULL) num_keys[0] = self->keyCount;
    if (num_rows != NULL) num_rows[0] = self->numId;

    if (num_holes != NULL || id2key || id_all) {
        size_t holes = 0;
        int64_t next = self->minId;
        
        for (i = 0; i < self->keyCount; ++i) {
            IdMapEntry const *const entry = self->entries + i;
            
            if (next != entry->firstId)
                holes += 1;
            next = entry->lastId + 1;
        }
        if (isOntoAndGapless(self) && holes != 0)
            goto BAD;
        if (num_holes != NULL) num_holes[0] = holes;
    }
    if (key2id || id_all) {
        for (i = 0; i < self->keyCount; ++i) {
            IdMapEntry const *const entry = self->entries + i;
            int64_t firstId = 0;
            uint32_t span = 0;
            rc_t rc = KHTIndexFind_v5(self, entry->name, &firstId, &span);
            
            if (rc != 0 || firstId != entry->firstId || firstId + (span - 1) != entry->lastId)
                goto BAD;
        }
    }
    return 0;
    
BAD:
    return RC ( rcDB, rcIndex, rcValidating, rcSelf, rcCorrupt );
}
