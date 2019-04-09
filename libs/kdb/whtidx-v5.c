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

#include "windex-priv.h"

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

typedef struct BufferListEntry BufferListEntry;
struct BufferListEntry {
    BufferListEntry *prv;
    char *buffer;
    size_t used;
};

#define KEY_BUFFER_SIZE ((size_t)(16ull * 1024ull *1024ull))

static BufferListEntry *makeBufferListEntry(BufferListEntry *prv, rc_t *rc)
{
    BufferListEntry *rslt = malloc(sizeof(*rslt));

    if (rslt) {
        rslt->prv = prv;
        rslt->used = 0;
        rslt->buffer = malloc(KEY_BUFFER_SIZE); /* pointers must stay valid so no realloc */
        if (rslt->buffer != NULL)
            return rslt;        
        free(rslt);
    }
    *rc = RC(rcDB, rcIndex, rcAllocating, rcMemory, rcExhausted);
    return NULL;
}

struct IdMapEntry {
    char const *name;
    int64_t firstId, lastId;
};

#define LOADING_FACTOR (0.618) /* PHI - 1 */
#define INITIAL_ENTRY_COUNT (4096u)

static bool initializeWrite(KHTIndex_v5 *const self, rc_t *rc)
{
    self->entries = malloc((self->maxKeys = INITIAL_ENTRY_COUNT) * sizeof(self->entries[0]));
    if (self->entries == NULL) {
        *rc = RC(rcDB, rcIndex, rcAllocating, rcMemory, rcExhausted);
        return false;
    }

    *rc = KHashTableMake(&self->hashtable, sizeof(char *), sizeof(size_t), INITIAL_ENTRY_COUNT, LOADING_FACTOR, KHT_key_type_cstr);
    if (*rc) return false;

    self->current = makeBufferListEntry(NULL, rc);
    if (self->current == NULL) return false;

    return true;
}

static size_t cstring_byte_count(char const *const cstring)
{
    size_t n = 0;
    while (cstring[n] != '\0')
        ++n;
    return n;
}

static char *cstring_copy(size_t const n, char *const dst, char const *const src)
{
    size_t i;
    for (i = 0; i < n; ++i) {
        dst[i] = src[i];
    }
    assert(src[n] == '\0');
    dst[n] = '\0';
    return dst;
}

static bool addEntryToHashTable(KHTIndex_v5 *const self, size_t const i, rc_t *rc)
{
    IDMapEntry const *const entry = self->entries + i;
    *rc = KHashTableAdd(self->hashtable, entry->name, KHashCStr(entry->name), &i);
    return *rc == 0;
}

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

static bool growKeys(KHTIndex_v5 *const self, size_t const keylen, rc_t *rc)
{
    if (self->current->used + keylen + 1 < KEY_BUFFER_SIZE)
        return true;
    self->current = makeBufferListEntry(self->current, rc);
    return self->current != NULL;
}

static bool growEntries(KHTIndex_v5 *const self, rc_t *rc)
{
    if (self->keyCount < self->maxKeys)
        return true;
    else {
        void *tmp = realloc(self->entries, (self->maxKeys * 2) * sizeof(self->entries[0]));
        if (tmp) {
            self->maxKeys *= 2;
            self->entries = tmp;
            return true;
        }
    }
    *rc = RC(rcDB, rcIndex, rcInserting, rcMemory, rcExhausted);
    return false;
}

static bool insertNewEntry( KHTIndex_v5 *const self
                          , size_t const keylen
                          , char const *const key
                          , int64_t const id
                          , rc_t *rc)
{
    size_t const i = self->keyCount;

    if (growEntries(self, rc) && growKeys(self, keylen, rc)) {
        IdMapEntry *const entry = self->entries + i;
        char const *const copy = self->current->buffer + self->current->used;

        entry->name = cstring_copy(keylen, copy, key);
        entry->firstId = entry->lastId = id;

        self->keyCount = i + 1;
        self->current->used += keylen + 1;

        return addEntryToHashTable(self, i, rc);
    }
    return false;
}

static bool updateEntry(KHTIndex_v5 *const self, uint64_t const i, int64_t const id, rc_t *rc)
{
    IdMapEntry *const entry = self->entries + i;

    if (entry->lastId + 1 == id) {
        entry->lastId = id;
        return true;
    }
    *rc = RC(rcDB, rcIndex, rcInserting, rcConstraint, rcViolated);
    return false;
}

rc_t KHTIndexInsert( KHTIndex_v5 *const self
                   , char const *const key
                   , int64_t const id )
{
    rc_t rc = 0;
    size_t keylen;

    assert(self != NULL)
    assert(key != NULL);

    keylen = cstring_byte_count(key);
    assert(keylen != 0); /* is this true */
    
    if (self->keyCount > 0) {
        if (id <= self->prvId)
            return RC(rcDB, rcIndex, rcInserting, rcConstraint, rcViolated);
        
        /* check if key exists */
        uint64_t fnd = 0;
        if (!KHashTableFind(self->hashtable, key, keylen, KHash(keylen, key), &fnd)) {
            if (!insertNewEntry(self, keylen, key, id, &rc))
                return rc;
        }
        else {
            assert(fnd < self->keyCount);
            if (!updateEntry(self, fnd, id, &rc))
                return rc;
        }        
        self->prvId = id;
        self->numId += 1;
    }
    else {
        if (!initializeWrite(self, &rc))
            return rc;
        if (!insertNewEntry(self, key, id, &rc))
            return rc;
        self->prvId = self->minId = id;
        self->numId = 1;
    }
    return 0;
}

rc_t KHTIndexFind( KHTIndex_v5 const *const self
                 , char const *const key
                 , int64_t *const out_firstId
                 , uint32_t *const out_span )
{
    rc_t rc = 0;
    size_t keylen;
    uint64_t fnd = 0;
    IDMapEntry const *entry = NULL;

    assert(self != NULL);
    assert(key != NULL);

    if (self->hashtable == NULL) {
        if (!initializeRead((KHTIndex_v5 *)self, &rc))
            return rc;
    }
    assert(self->hashtable != NULL);
    
    keylen = cstring_byte_count(key);
    if (!KHashTableFind(self->hashtable, key, keylen, KHash(keylen, key), &fnd))
        return RC(rcDB, rcIndex, rcSelecting, rcString, rcNotFound);
  
    assert(fnd < self->keyCount);
    entry = self->entries + fnd;
    if (out_firstId)
        *out_firstId = entry->firstId;
    if (out_span)
        *out_span = 1 + (entry->lastId - entry->firstId);

    return 0;
}

static bool isOntoAndGapless(KHTIndex_v5 const *const self)
{
    return self->numId == self->keyCount
        && self->numId == 1 + (self->prvId - self->minId);
}

rc_t KHTIndexProject_v5 ( KHTIndex_v5 const *const self
                        , int64_t const id
                        , char const **const key
                        , int64_t *const first_id
                        , uint32_t *const span
                        )
{
    IDMapEntry const *fnd = NULL;

    assert(self != NULL);
    
    if (self->minId <= id && id <= self->prvId) {
        if (isOntoAndGapless(self)) {
            /* can do a linear lookup by id */
            size_t const i = id - self->minId;
            IDMapEntry const *const entry = self->entries + i;
            
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
                size_t const initial_guess = self->keyCount * ((id - minId) / (double)span);
                if (initial_guess < self->keyCount) {
                    IDMapEntry const *const entry = self->entries + initial_guess;

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
                IDMapEntry const *const entry = self->entries + m;

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
    if (fnd) {
    FOUND:
        assert(fnd->firstId <= id && id <= fnd->lastId);
        
        if (key     ) *key      = fnd->name;
        if (first_id) *first_id = fnd->firstId;
        if (span    ) *span     = 1 + (fnd->lastId - fnd->firstId);

        return 0;
    }
    else
        return RC(rcDB, rcIndex, rcProjecting, rcId, rcNotFound);
}
