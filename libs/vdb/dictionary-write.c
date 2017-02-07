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

#include <vdb/extern.h>
#include <klib/defs.h>
#include <klib/rc.h>
#include <vdb/table.h>
#include <vdb/xform.h>
#include <vdb/schema.h>
#include <kdb/meta.h>
#include <kdb/column.h>
#include <klib/data-buffer.h>
#include <klib/btree.h>
#include <klib/ksort-macro.h>
#include <sysalloc.h>
#include "xform-priv.h"

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <zlib.h>

typedef struct key_s Key;
struct key_s {
    uint32_t offset;
    uint32_t count;
};

typedef struct self_t {
    KColumn *kcol;
    KDataBuffer data; /* values are stored here */
    KDataBuffer keys; /* keys (offsets into data) are stored here */
    Pager *pager;
    Pager_vt const *pager_vt;
    int64_t keysRowStart;
    int64_t keysRowLast;
    size_t totalValueSize;
    size_t totalBlobSize;
    size_t totalCompressedBlobSize;
    uint32_t btreeRoot;
    uint32_t blobLimit;
    uint32_t blobCount;
    uint32_t keysCarried;
} self_t;

static void resetState(self_t *const self)
{
    self->btreeRoot = 0;
    MallocPagerMake(&self->pager, &self->pager_vt);
    
    KDataBufferMake(&self->keys, 64, 0);
    KDataBufferMake(&self->data, 8, 0);
    self->keysRowStart = self->keysRowLast = 0;
}

static self_t *make_self()
{
    self_t *const self = calloc(1, sizeof(*self));
    if (self) {
        resetState(self);
        self->blobLimit = 64 * 1024;
    }
    return self;
}

static void const *const kEmptyValue = (void const *)"";

/* this function can only fail if out of memory */
static uint32_t insertDictionary(self_t *const self, int const len, uint8_t const *const value)
{
    size_t const valueLen = len < 0 ? strlen((char const *)value) : len;
    uint32_t valueId = (uint32_t)self->keys.elem_count;
    bool wasInserted = false;
    rc_t rc = BTreeEntry(&self->btreeRoot, self->pager, self->pager_vt, &valueId, &wasInserted, valueLen == 0 ? kEmptyValue : value, valueLen == 0 ? 1 : valueLen);

    assert(rc == 0);
    if (rc == 0 && wasInserted) {
        uint32_t const key = self->data.elem_count;
        
        rc = KDataBufferResize(&self->data, key + valueLen + 1);
        assert(rc == 0);
        if (rc == 0) {
            rc = KDataBufferResize(&self->keys, valueId + 1);
            assert(rc == 0);
            if (rc == 0) {
                memmove(&((uint8_t *)self->data.base)[key], value, valueLen);
                ((uint8_t *)self->data.base)[key + valueLen] = '\0';
                
                ((Key *)self->keys.base)[valueId].offset = key;
                ((Key *)self->keys.base)[valueId].count = 0;
            }
        }
    }
    if (rc == 0) {
        ((Key *)self->keys.base)[valueId].count += 1;
        return valueId;
    }
    return (uint32_t)self->keys.elem_count; /* an invalid value; one past the end */
}

static KDataBuffer compressToBuffer(int bytes, void *data)
{
    z_stream strm;
    int zrc = 0;
    KDataBuffer compressed;
    KDataBuffer result;
    rc_t rc = KDataBufferMake(&compressed, 8, bytes + 1);
    assert(rc == 0);

    memset(&strm, 0, sizeof(strm));
    zrc = deflateInit2(&strm, Z_BEST_COMPRESSION, Z_DEFLATED, -15, 9, Z_DEFAULT_STRATEGY);
    assert(zrc == Z_OK);
    
    strm.next_in = data;
    strm.avail_in = bytes;
    strm.next_out = compressed.base;
    strm.avail_out = compressed.elem_count - 1;
    
    *strm.next_out++ = 0x78; /* indicate compressed; looks like start of a normal zlib header but not followed by FLG byte */
    
    zrc = deflate(&strm, Z_FINISH);
    deflateEnd(&strm);
    if (zrc == Z_STREAM_END) {
        KDataBufferSub(&compressed, &result, 0, strm.total_out + 1);
    }
    else {
        /* unlikely that it didn't compress at all */
        uint8_t *out = compressed.base;
        
        *out++ = 0; /* indicate NOT compressed */
        memmove(out, data, bytes);
        KDataBufferSub(&compressed, &result, 0, bytes + 1);
    }
    
    KDataBufferWhack(&compressed);
    
    return result;
}

/* sort by use-count descending */
static void sort_stats(uint32_t const N, Key stats[])
{
    Key tmp;
#define SAVE(A) tmp = stats[(A)]
#define LOAD(A) ((void)(stats[(A)] = tmp))
#define COPY(DST, SRC) ((void)(stats[(DST)] = stats[(SRC)]))
#define LESS(A, B) (stats[(B)].count < stats[(A)].count)
    ASORT(N);
#undef SAVE
#undef LOAD
#undef COPY
#undef LESS
}

static void carryOver(self_t *const self, KDataBuffer keys, KDataBuffer const data)
{
    Key *stats = keys.base;
    uint32_t i;
    uint32_t N = keys.elem_count;
    uint32_t limit = 0;
    
    if (N == 0)
        return;
    
    sort_stats(N, stats);
    limit = stats[0].count / 2;
    if (limit < 2)
        limit = 2;

    for (i = 0; i < N; ++i) {
        uint32_t valueId;
        
        if (stats[i].count < limit)
            break;
        valueId = insertDictionary(self, -1, &((uint8_t const *)data.base)[stats[i].offset]);
        assert(valueId == i);
        ((Key *)self->keys.base)[valueId].count = 0; /* reset use count */
    }
    self->keysCarried += i;
}

static rc_t updateKColumnAndReset(self_t *const self, bool final)
{
    int64_t const startRow = self->keysRowStart;
    int64_t const count = self->keysRowLast + 1 - startRow;
    KColumnBlob *kblob = NULL;
    KDataBuffer keys = self->keys;
    KDataBuffer data = self->data;
    KDataBuffer compressed = compressToBuffer(data.elem_count, data.base);
    rc_t rc = 0;

    assert(count - ((uint32_t)count) == 0);

    /* reset state */
    MallocPagerWhack(self->pager);
    resetState(self);
    
    ++self->blobCount;
    self->totalBlobSize += data.elem_count;
    self->totalCompressedBlobSize += compressed.elem_count;

    /* write compressed blob */
    rc = KColumnCreateBlob(self->kcol, &kblob);
    assert(rc == 0);
    if (rc) return rc;
    
    rc = KColumnBlobAssignRange(kblob, startRow, (uint32_t)count);
    assert(rc == 0);
    if (rc) return rc;
    
    rc = KColumnBlobAppend(kblob, compressed.base, compressed.elem_count);
    if (rc == 0) {
        rc = KColumnBlobCommit(kblob);
    }
    
    KColumnBlobRelease(kblob);
    KDataBufferWhack(&compressed);

    if (!final) {
        /* add most-frequently-used-values to new dictionary */
        carryOver(self, keys, data);
    }
    KDataBufferWhack(&keys);
    KDataBufferWhack(&data);
    
    return rc;
}

static
rc_t CC dictionaryEncode(void *const Self,
                         VXformInfo const *const info,
                         int64_t const row_id,
                         VRowResult *const rslt,
                         uint32_t const argc,
                         VRowData const *const argv)
{
    assert(argv[0].u.data.elem_bits == 8);
    self_t *const self = Self;
    uint8_t const *const value = ((uint8_t const *)argv[0].u.data.base) + ((argv[0].u.data.first_elem * argv[0].u.data.elem_bits) / 8);
    size_t const valueLen = (argv[0].u.data.elem_count * argv[0].u.data.elem_bits) / 8;
    size_t const dictionarySize = self->data.elem_count;
    uint32_t valueId = 0;
    rc_t rc = 0;
    
    self->totalValueSize += valueLen;
    while (valueLen > self->blobLimit) {
        uint64_t const newLimit = ((uint64_t)self->blobLimit) << 1;
        if (newLimit > UINT32_MAX)
            return RC(rcVDB, rcFunction, rcExecuting, rcRow, rcTooBig);
        self->blobLimit = (uint32_t)newLimit;
    }
    assert(self->keysRowStart == 0 || self->keysRowStart < row_id);
    if (self->keysRowStart != 0 && row_id > self->keysRowStart + UINT32_MAX) {
        /* row range for a blob is 32 bits */
        updateKColumnAndReset(self, false);
    }
    valueId = insertDictionary(self, valueLen, value);
    if (self->data.elem_count > self->blobLimit) {
        /* back out the just-inserted value */
        self->data.elem_count = dictionarySize;
        --self->keys.elem_count;
        
        /* write the dictionary and reset it */
        updateKColumnAndReset(self, false);

        valueId = insertDictionary(self, valueLen, value);
    }
    if (valueId >= self->keys.elem_count)
        return RC(rcVDB, rcFunction, rcExecuting, rcMemory, rcExhausted);

    rc = KDataBufferResize(rslt->data, 1);
    assert(rc == 0);
    if (rc) return rc;
    rslt->elem_count = rslt->data->elem_count;
    {
        uint32_t *const y = rslt->data->base;
        Key const *const keys = self->keys.base;
        y[0] = keys[valueId].offset;
    }
    if (self->keysRowStart == 0)
        self->keysRowStart = row_id;
    self->keysRowLast = row_id;
    return 0;
}

static void CC self_whack(void *const vp)
{
    self_t *const self = vp;
    if (self) {
        if (self->keys.elem_count > 0)
            updateKColumnAndReset(self, true);
        
        MallocPagerWhack(self->pager);
        KDataBufferWhack(&self->keys);
        KDataBufferWhack(&self->data);
        
#ifndef UNIT_TEST
        free(self);
#endif
    }
}

#ifdef UNIT_TEST
static rc_t make_dict_text_insert(VFuncDesc *rslt, KColumn *kcol)
{
    self_t *const self = make_self();
    
    self->kcol = kcol;
    
    rslt->self = self;
    rslt->whack = self_whack;
    
    rslt->variant = vftNonDetRow;
    rslt->u.ndf = dictionaryEncode;
    
    return 0;
}
#else
/* U32 dict:text:insert(text8_set)
 */
VTRANSFACT_BUILTIN_IMPL(dict_text_insert, 1, 0, 0) (const void *Self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->self = make_self();
    rslt->whack = self_whack;
    
    rslt->variant = vftNonDetRow;
    rslt->u.ndf = dictionaryEncode;
    
    return 0;
}
#endif
