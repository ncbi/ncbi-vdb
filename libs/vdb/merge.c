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

#include "blob.h"

#include "blob-headers.h"
#include "page-map.h"
#include "blob-priv.h"
#include "xform-priv.h"

#include <vdb/xform.h>
#include <vdb/schema.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <atomic32.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static unsigned uniq_list( unsigned dst[], void **list, unsigned N ) {
    unsigned i;
    unsigned j;
    unsigned n;
    void *on_stack[16];
    void **uniq = on_stack;
    void **on_heap = 0;
    
    if (N > sizeof(on_stack) / sizeof(on_stack[0])) {
        on_heap = malloc(N * sizeof(on_heap[0]));
        uniq = on_heap;
    }
    for (i = 0, n = 0; i != N; ++i) {
        for (j = 0; j != n; ++j) {
            if (uniq[j] == list[i])
                break;
        }
        dst[i] = j;
        if (j == n) {
            uniq[j] = list[i];
            ++n;
        }
    }
    memcpy(list, uniq, n * sizeof(list[0]));
    if (on_heap)
        free(on_heap);
    return n;
}

static
rc_t CC VBlobCreateMerged( VBlob **lhs, uint32_t num_inputs, const VBlob *input[] ) {
    unsigned i;
    KDataBuffer buffer;
    BlobHeaders *headers;
    VBlobHeader *hdr;
    rc_t rc;
    uint64_t sz = 0;
    uint32_t bsize;
    
/*
     headers:
         args:
             num_inputs
             num_row_maps
             row_map_number[num_inputs]
             length(row_map)[num_row_maps]
             length(header)[num_inputs]
             length(data_blob)[num_inputs]
     data:
         row_map[num_row_maps]
         header[num_inputs]
         data_blob[num_inputs]
 */
    
    for (i = 1; i != num_inputs; ++i) {
        if (input[0]->start_id != input[i]->start_id || input[0]->stop_id != input[i]->stop_id)
            return RC(rcVDB, rcBlob, rcConstructing, rcParam, rcInvalid);
    }
    
    rc = BlobHeadersCreate(&headers);
    if (rc)
        return rc;
    
    hdr = BlobHeadersGetHdrWrite(headers);
    if (hdr == NULL) {
        BlobHeadersRelease(headers);
        return RC(rcVDB, rcFunction, rcExecuting, rcMemory, rcExhausted);
    }
    rc = KDataBufferMakeBytes(&buffer, bsize = 0);
    if (rc) {
        VBlobHeaderRelease(hdr);
        BlobHeadersRelease(headers);
        return rc;
    }
    VBlobHeaderSetFormat(hdr, 0); /* TBD: set format id to correct value for merged or do it in caller */
    VBlobHeaderSetVersion(hdr, 1);
    
    VBlobHeaderArgPushTail(hdr, num_inputs);
    
    {
        unsigned *v;
        PageMap **pm;
        unsigned n;
		
        v = malloc(num_inputs * sizeof(*v));
        pm = malloc(num_inputs * sizeof(*pm));
        for (i = 0; i != num_inputs; ++i) {
            pm[i] = input[i]->pm;
        }
        n = uniq_list(v, (void **)pm, num_inputs);
        VBlobHeaderArgPushTail(hdr, n);
        for (i = 0; i != num_inputs; ++i)
            VBlobHeaderArgPushTail(hdr, v[i]);
        free(v);
        for (i = 0; rc == 0 && i != n; ++i) {
            rc = PageMapSerialize(pm[i], &buffer, bsize, &sz);
            VBlobHeaderArgPushTail(hdr, sz);
            bsize += sz;
        }
        free(pm);
    }
    
    for (i = 0; rc == 0 && i != num_inputs; ++i) {
        if (input[i]->headers) {
            rc = BlobHeadersSerialize(input[i]->headers, &buffer, bsize, &sz);
            VBlobHeaderArgPushTail(hdr, sz);
            bsize += sz;
        }
        else
            VBlobHeaderArgPushTail(hdr, 0);
    }
    
    for (i = 0; rc == 0 && i != num_inputs; ++i) {
        const VBlob *o = input[i];
        
        sz = (uint32_t)BlobBufferBits( o );
        rc = VBlobHeaderArgPushTail( hdr, sz );
        sz = (sz + 7) >> 3;
        if (rc == 0) {
            rc = KDataBufferResize(&buffer, bsize + sz);
            if (rc == 0) {
                memcpy((uint8_t *)buffer.base + bsize, o->data.base, sz);
                bsize += sz;
            }
        }
    }
    VBlobHeaderRelease(hdr);
    if (rc == 0) {
        VBlob *y;
        
        rc = VBlobNew(&y, input[0]->start_id, input[0]->stop_id, "merge");
        if (rc == 0) {
            y->headers = headers;
            KDataBufferSub(&buffer, &y->data, 0, UINT64_MAX);
            
#if __BYTE_ORDER == __LITTLE_ENDIAN
            y->byte_order = vboLittleEndian;
#else
            y->byte_order = vboBigEndian;
#endif
            *lhs = y;
	    KDataBufferWhack(&buffer);
            return 0;
        }
        rc = RC(rcVDB, rcBlob, rcConstructing, rcMemory, rcExhausted);
    }
    KDataBufferWhack(&buffer);
    BlobHeadersRelease(headers);
    
    return rc;
}

static
rc_t CC vdb_merge_impl(
                    void *self,
                    const VXformInfo *info,
                    int64_t row_id,
                    VBlob **rslt,
                    uint32_t argc, const VBlob *argv[]
) {
	return VBlobCreateMerged(rslt, argc, argv);
}

VTRANSFACT_BUILTIN_IMPL(vdb_merge, 1, 0, 0) (const void *self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
	rslt->variant = vftBlobN;
    VFUNCDESC_INTERNAL_FUNCS(rslt)->bfN = vdb_merge_impl;
	return 0;
}
