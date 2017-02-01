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
#include <sysalloc.h>
#include "xform-priv.h"

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct self_t {
    KColumn *kcol;
    KDataBuffer data; /* values are stored here */
    KDataBuffer keys; /* keys (offsets into data) are stored here */
    Pager *pager;
    Pager_vt const *pager_vt;
    int64_t keysRowStart;
    int64_t keysRowLast;
    uint32_t btreeRoot;
} self_t;

static self_t *make_self()
{
    self_t *const self = calloc(1, sizeof(*self));
    KDataBufferMake(&self->data, 8, 0);
    KDataBufferMake(&self->keys, 32, 0);
    MallocPagerMake(&self->pager, &self->pager_vt);
}

static void CC self_whack(void *const vp)
{
    self_t *const self = vp;
    
    MallocPagerWhack(self->pager);
    KDataBufferWhack(&self->keys);
    KDataBufferWhack(&self->data);
    
    free(self);
}

static rc_t updateKColumn(self_t *self)
{
    int64_t count = self->keysRowLast + 1 - self->keysRowStart;
    KColumnBlob *kblob = NULL;

    rc_t rc = KColumnCreateBlob(self->kcol, &kblob);
    assert(rc == 0);
    if (rc) return rc;
    
    assert(count < 0x100000000ll);

    rc = KColumnBlobAssignRange(kblob, self->keysRowStart, (uint32_t)count);
    assert(rc == 0);
    if (rc) return rc;
    
    rc = KColumnBlobAppend(kblob, self->data.base, self->data.elem_count);
    if (rc == 0) {
        rc = KColumnBlobCommit(kblob);
    }
    
    KDataBufferWhack(&self->data);
    KDataBufferWhack(&self->keys);
    self->keysRowStart = self->keysRowLast = 0;
    
    KColumnBlobRelease(kblob);
    
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
    assert(argv[0].u.data.elem_bits == 8);
    self_t *const self = Self;
    uint8_t const *const value = ((uint8_t const *)argv[0].u.data.base) + ((argv[0].u.data.first_elem * argv[0].u.data.elem_bits) / 8);
    size_t const valueLen = (argv[0].u.data.elem_count * argv[0].u.data.elem_bits) / 8;
    uint32_t valueId = 0;
    bool wasInserted = false;
    rc_t rc = 0
    
    assert(self->keysRowStart == 0 || self->keysRowStart < row_id);
    if (self->keysRowStart != 0 && row_id >= self->keysRowStart + 0x100000000ll) {
        updateKColumn(self);
        resetDictionary(self);
    }
AGAIN:
    rc = BTreeEntry(&self->btreeRoot, self->pager, self->pager_vt, &valueId, &wasInserted, value, valueLen);
    assert(rc == 0);
    if (rc) return rc;
    
    if (wasInserted) {
        size_t const last = self->data.elem_count;
        size_t const newSize = last + valueLen + 1;

#if 0
        if (newSize >= limit) {
            KColumnWriteBlob ...
            ResetDictionaryState ...
            goto AGAIN;
        }
#endif
        rc = KDataBufferResize(&self->keys, valueId + 1);
        assert(rc == 0);
        if (rc) return rc;
        ((uint32_t *)self->keys.base)[valueId] = last;

        rc = KDataBufferResize(&self->data, newSize);
        assert(rc == 0);
        if (rc) return rc;
        memmove(&((uint8_t *)self->data.base)[last], value, valueLen);
        ((uint8_t *)self->data.base)[last + valueLen] = '\0';
    }
    rc = KDataBufferResize(rslt->data, 1);
    assert(rc == 0);
    if (rc) return rc;
    rslt->elem_count = rslt->data->elem_count;
    {
        uint32_t *const rslt = rslt->data->base;
        uint32_t const *const keys = self->keys.base;
        rslt[0] = keys[valueId];
    }
    if (self->keysRowStart == 0)
        self->keysRowStart = row_id;
    self->keysRowLast = row_id;
    return 0;
}

/* meta:write
 */
VTRANSFACT_BUILTIN_IMPL(meta_write, 1, 0, 0) (const void *Self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rc_t rc;
    self_t *self;
    bool need_byte_swap;
    
    SDatatype *sdt = VSchemaFindTypeid(info->schema, dp->argv[0].fd.td.type_id);
    assert(sdt != NULL);
    
	self = calloc(1, sizeof(self_t));
	if (self == NULL)
		rc = RC(rcVDB, rcFunction, rcConstructing, rcMemory, rcExhausted);
    else
    {
        KMetadata *meta;

        /* even if we cast away const on the table,
           kdb will still prevent us from doing so if
           it's actually read-only */
        rc = VTableOpenMetadataUpdate((VTable *)info->tbl, &meta);
        if (rc == 0) {
            rc = KMetadataOpenNodeUpdate(meta, &self->node, "%s", cp->argv[0].data.ascii);
            KMetadataRelease(meta);
            if (rc == 0) {
                KMDataNodeByteOrder(self->node, &need_byte_swap);
                if (need_byte_swap)
                    self->byte_swap = sdt->byte_swap;
                
                rslt->self = self;
                rslt->whack = self_whack;
                
                rslt->variant = vftNonDetRow;
                rslt->u.ndf = meta_write_func;
                
                return 0;
            }
        }
        self_whack(self);
	}

	return rc;
}
