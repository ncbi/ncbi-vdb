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
#include <bitstr.h>
#include <sysalloc.h>

#include "schema-priv.h"
#include "xform-priv.h"

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

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

static size_t KColumnBlobSize(KColumnBlob const *self)
{
    size_t dummy = 0;
    size_t remain = 0;
    rc_t const rc = KColumnBlobRead(self, 0, NULL, 0, &dummy, &remain);

    assert(rc == 0); /* this should be infallible */
    return remain;
}

static rc_t KColumnBlobReadBuffer(KColumnBlob const *self, KDataBuffer *buffer)
{
    size_t const size = KColumnBlobSize(self);
    rc_t rc = KDataBufferMake(buffer, 8, size);
    size_t cur = 0;
    uint8_t *base = buffer->base;

    assert(rc == 0);
    if (rc) return rc;
    while (cur < size) {
        size_t nread = 0;
        size_t dummy = 0;
        
        rc = KColumnBlobRead(self, cur, base + cur, size - cur, &nread, &dummy);
        if (rc) break;
        assert(nread > 0);
        cur += nread;
    }
    return rc;
}

static
rc_t CC row_func(void *const Self,
                 VXformInfo const *const info,
                 int64_t const row_id,
                 VRowResult *const rslt,
                 uint32_t const argc,
                 VRowData const *const argv)
{
    self_t *const self = Self;
    uint32_t const *const key = ((uint8_t const *)argv[0].u.data.base) + ((argv[0].u.data.first_elem * argv[0].u.data.elem_bits) / 8);
    size_t const count = (argv[0].u.data.elem_count * argv[0].u.data.elem_bits) / 8;
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
        if (rc) return rc;
        rc = KColumnBlobIdRange(kblob, &startId, &rowCount);
        assert(rc == 0);
        if (rc) return rc;
        rc = KColumnBlobReadBuffer(kblob, &self->data);
        assert(rc == 0);
        if (rc) return rc;
        KColumnBlobRelease(kblob);
        self->dataRowStart = startId;
        self->dataRowLast = startId + rowCount;
    }
    {
        uint8_t const *const data = ((uint8_t const *)self->data.base) + key[0];
        size_t len;
        for (len = 0; ; ++len) {
            int const ch = data[len];
            if (ch == '\0')
                break;
        }
        rc = KDataBufferResize(rslt->data, len);
        if (rc == 0) {
            memmove(rslt->data->base, data, len);
            rslt->elem_count = len;
        }
    }
    return rc;
}

/* 
 function < type T > T meta:read #1.0 < ascii node, * bool deterministic > ();
 */
VTRANSFACT_BUILTIN_IMPL ( meta_read, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rc_t rc = 0;
    self_t *self;
    SDatatype *sdt;
    bool need_byte_swap;
    bool deterministic = true;
    
    if (cp->argc > 1)
        deterministic = cp->argv[1].data.b[0];
        
    sdt = VSchemaFindTypeid(info->schema, info->fdesc.fd.td.type_id);
    assert(sdt != NULL);
    
	self = calloc(1, sizeof(self_t));
	if (self == NULL)
		rc = RC(rcVDB, rcFunction, rcConstructing, rcMemory, rcExhausted);
    else
    {
        const KMetadata *meta;
        
        rc = VTableOpenMetadataRead(info->tbl, &meta);
        if (rc == 0) {
            rc = KMetadataOpenNodeRead(meta, &self->node, "%.*s", cp->argv[0].count, cp->argv[0].data.ascii);
            KMetadataRelease(meta);
            if (rc == 0) {
                KMDataNodeByteOrder(self->node, &need_byte_swap);
                if (need_byte_swap)
                    self->byte_swap = sdt->byte_swap;
                
                rslt->self = self;
                rslt->whack = self_whack;
                
                rslt->variant = deterministic ? vftRow : vftNonDetRow;
                rslt->u.ndf = meta_read_func;
                
                return 0;
            }
        }
        self_whack(self);
	}
    return rc;
}
