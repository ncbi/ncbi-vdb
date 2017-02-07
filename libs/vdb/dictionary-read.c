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
#include <kdb/column.h>
#include <klib/data-buffer.h>
#include <bitstr.h>
#include <sysalloc.h>

#include "schema-priv.h"
#include "xform-priv.h"

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <zlib.h>

typedef struct self_t {
    KColumn const *kcol;
    KDataBuffer data; /* values are stored here */
    int64_t dataRowStart;
    int64_t dataRowEnd;
} self_t;

static void CC self_whack(void *const vp) {
    self_t *const self = vp;
    
    KColumnRelease(self->kcol);
    KDataBufferWhack(&self->data);
    free(self);
}

#ifndef INFLATE_BACK_INPUT_BUFFER_SIZE
#define INFLATE_BACK_INPUT_BUFFER_SIZE (32u * 1024u) /* should be enough to read the whole blob in one shot */
#endif

struct readBlobContext {
    KColumnBlob const *source;
    size_t cur;
    rc_t rc;
    uint8_t input[INFLATE_BACK_INPUT_BUFFER_SIZE];
};

static unsigned readBlob(void *const Ctx, uint8_t **const input)
{
    struct readBlobContext *ctx = Ctx;
    size_t nread = 0;
    size_t dummy = 0;
    
    ctx->rc = KColumnBlobRead(ctx->source, ctx->cur, ctx->input, sizeof(ctx->input), &nread, &dummy);
    if (nread == 0)
        return 0;
    ctx->cur += nread;
    
    *input = ctx->input;
    return (unsigned)nread;
}

static int resizeAndCopy(void *const Ctx, uint8_t *const data, unsigned const length)
{
    KDataBuffer *const outputBuffer = Ctx;
    unsigned const at = (unsigned)outputBuffer->elem_count;
    rc_t rc = KDataBufferResize(outputBuffer, at + length);
    assert(rc == 0);
    if (rc == 0) {
        uint8_t *const out = outputBuffer->base;
        memmove(out + at, data, length);
        return 0;
    }
    return -1;
}
static rc_t decompress(struct readBlobContext *const ctx, KDataBuffer *const rslt)
{
    uint8_t window[1u << MAX_WBITS];
    int zrc = 0;
    z_stream strm;
    
    memset(&strm, 0, sizeof(strm));
    zrc = inflateBackInit(&strm, MAX_WBITS, window);
    assert(zrc == Z_OK);
    
    strm.next_in = ctx->input + 1;
    strm.avail_in = ctx->cur - 1;
    
    zrc = inflateBack(&strm, readBlob, ctx, resizeAndCopy, rslt);
    {
        rc_t rc = 0;
        switch (zrc) {
        case Z_STREAM_END:
            break;
        case Z_DATA_ERROR:
            /* programmer error or corrupt data */
            rc = RC(rcVDB, rcFunction, rcExecuting, rcData, rcCorrupt);
            assert(rc == 0); /* let's assume it's programmer error */
            break;
        case Z_BUF_ERROR:
            if (strm.next_in == Z_NULL) {
                /* the error came from readBlob or blob was truncated */
                rc = ctx->rc;
                assert(rc != 0); /* let's assume truncation is programmer error */
                if (rc == 0)
                    rc = RC(rcVDB, rcFunction, rcExecuting, rcBlob, rcTooShort);
            }
            else {
                /* the error came from resizeAndCopy */
                rc = RC(rcVDB, rcFunction, rcExecuting, rcMemory, rcExhausted);
            }
        default:
            /* programmer error */
            abort();
            break;
        }
        inflateBackEnd(&strm);
        return rc;
    }
}

static rc_t loadBlob(KColumnBlob const *blob, KDataBuffer *rslt)
{
    struct readBlobContext ctx;
    size_t nread = 0;
    size_t remain = 0;
    
    ctx.rc = KColumnBlobRead(blob, 0, ctx.input, sizeof(ctx.input), &nread, &remain);
    assert(ctx.rc == 0);
    if (ctx.rc) return ctx.rc;

    if (ctx.input[0] == 0x78) {
        ctx.rc = KDataBufferMake(rslt, 8, (nread + remain - 1) * 4);
        assert(ctx.rc == 0);
        if (ctx.rc != 0)
            return ctx.rc;
        rslt->elem_count = 0;

        ctx.source = blob;
        ctx.cur = nread;
        
        return decompress(&ctx, rslt);
    }
    else {
        /* not compressed */
        ctx.rc = KDataBufferMake(rslt, 8, nread + remain - 1);
        assert(ctx.rc == 0);
        if (ctx.rc) return ctx.rc;

        memmove(rslt->base, ctx.input + 1, nread - 1);
        if (remain > 0) {
            size_t dummy = 0;
            uint8_t *dst = rslt->base;
            dst += nread - 1;
            return KColumnBlobRead(blob, nread, dst, remain, &nread, &dummy);
        }
        return 0;
    }
}

static
rc_t CC dictionaryDecode(void *const Self,
                         VXformInfo const *const info,
                         int64_t const row_id,
                         VRowResult *const rslt,
                         uint32_t const argc,
                         VRowData const *const argv)
{
    self_t *const self = Self;
    uint32_t const *const key = (void const *)(((uint8_t const *)argv[0].u.data.base) + ((argv[0].u.data.first_elem * argv[0].u.data.elem_bits) / 8));
    size_t const count = argv[0].u.data.elem_count;
    rc_t rc = 0;

    assert(count == 1);
    assert(argv[0].u.data.elem_bits == 32);
    
    if (row_id >= self->dataRowEnd || row_id < self->dataRowStart) {
        KColumnBlob const *kblob = NULL;
        uint32_t rowCount = 0;
        int64_t startId = 0;
        
        KDataBufferWhack(&self->data);
        self->dataRowStart = self->dataRowEnd = 0;
        
        rc = KColumnOpenBlobRead(self->kcol, &kblob, row_id);
        assert(rc == 0);
        if (rc) return rc;
        
        rc = KColumnBlobIdRange(kblob, &startId, &rowCount);
        assert(rc == 0);
        if (rc) return rc;

        rc = loadBlob(kblob, &self->data);
        KColumnBlobRelease(kblob);
        assert(rc == 0);
        if (rc) return rc;
        
        self->dataRowStart = startId;
        self->dataRowEnd = startId + rowCount;
    }
    {
        uint8_t const *const endp = ((uint8_t const *)self->data.base) + self->data.elem_count;
        uint8_t const *const data = ((uint8_t const *)self->data.base) + key[0];
        size_t len;

        for (len = 0; ; ++len) {
            if (data + len < endp) {
                int const ch = data[len];
                if (ch == '\0')
                    break;
            }
            else
                return RC(rcVDB, rcFunction, rcExecuting, rcData, rcInvalid);
        }
        rc = KDataBufferResize(rslt->data, len);
        if (rc == 0) {
            memmove(rslt->data->base, data, len);
            rslt->elem_count = len;
        }
    }
    return rc;
}

#ifdef UNIT_TEST
static rc_t make_dict_text_lookup(VFuncDesc *rslt, KColumn const *kcol)
{
    self_t *const self = calloc(1, sizeof(self_t));
    
    self->kcol = kcol;
    
    rslt->self = self;
    rslt->whack = self_whack;
    rslt->variant = vftRow;
    rslt->u.ndf = dictionaryDecode;
    
    return 0;
}
#else
/* 
 function
 text8_set dict:text:lookup #1.0 ( vdb:dict:text:key key );
 */
VTRANSFACT_BUILTIN_IMPL ( dict_text_lookup, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->self = calloc(1, sizeof(self_t));
    rslt->whack = self_whack;
    rslt->variant = vftRow;
    rslt->u.ndf = dictionaryDecode;

    return 0;
}
#endif
