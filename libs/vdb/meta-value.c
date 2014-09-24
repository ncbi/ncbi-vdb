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
#include <klib/data-buffer.h>
#include <bitstr.h>
#include <sysalloc.h>
#include "xform-priv.h"

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct self_t {
    const KMDataNode *node;
    KDataBuffer value;
} self_t;

static void CC self_whack( void *vp ) {
    self_t *self = vp;
    
    KMDataNodeRelease( self->node );
    KDataBufferWhack( &self->value );
    free( vp );
}

static
rc_t CC meta_value_U8(
                   void *Self,
                   const VXformInfo *info,
                   int64_t row_id,
                   VRowResult *rslt,
                   uint32_t argc,
                   const VRowData argv[]
) {
    self_t *self = Self;
    uint64_t value;
    rc_t rc = 0;
    
    rc = KMDataNodeReadAsU64(self->node, &value);
    if (rc == 0) {
        ((uint8_t *)self->value.base)[0] = (uint8_t)value;
        rslt->elem_bits = 8;
        rslt->elem_count = 1;
        KDataBufferWhack(rslt->data);
        rc = KDataBufferSub(&self->value, rslt->data, 0, UINT64_MAX);
    }
    return rc;
}

static
rc_t CC meta_value_U16(
                   void *Self,
                   const VXformInfo *info,
                   int64_t row_id,
                   VRowResult *rslt,
                   uint32_t argc,
                   const VRowData argv[]
) {
    self_t *self = Self;
    uint64_t value;
    rc_t rc = 0;
    
    rc = KMDataNodeReadAsU64(self->node, &value);
    if (rc == 0) {
        ((uint16_t *)self->value.base)[0] = (uint16_t)value;
        rslt->elem_bits = 16;
        rslt->elem_count = 1;
        KDataBufferWhack(rslt->data);
        rc = KDataBufferSub(&self->value, rslt->data, 0, UINT64_MAX);
    }
    return rc;
}

static
rc_t CC meta_value_U32(
                   void *Self,
                   const VXformInfo *info,
                   int64_t row_id,
                   VRowResult *rslt,
                   uint32_t argc,
                   const VRowData argv[]
) {
    self_t *self = Self;
    uint64_t value;
    rc_t rc = 0;
    
    rc = KMDataNodeReadAsU64(self->node, &value);
    if (rc == 0) {
        ((uint32_t *)self->value.base)[0] = (uint32_t)value;
        rslt->elem_bits = 32;
        rslt->elem_count = 1;
        KDataBufferWhack(rslt->data);
        rc = KDataBufferSub(&self->value, rslt->data, 0, UINT64_MAX);
    }
    return rc;
}

static
rc_t CC meta_value_U64(
                   void *Self,
                   const VXformInfo *info,
                   int64_t row_id,
                   VRowResult *rslt,
                   uint32_t argc,
                   const VRowData argv[]
) {
    self_t *self = Self;
    rc_t rc = 0;
    
    rc = KMDataNodeReadAsU64(self->node, (uint64_t *)self->value.base);
    if (rc == 0) {
        rslt->elem_bits = 64;
        rslt->elem_count = 1;
        KDataBufferWhack(rslt->data);
        rc = KDataBufferSub(&self->value, rslt->data, 0, UINT64_MAX);
    }
    return rc;
}


static
rc_t CC meta_value_I8(
                   void *Self,
                   const VXformInfo *info,
                   int64_t row_id,
                   VRowResult *rslt,
                   uint32_t argc,
                   const VRowData argv[]
) {
    self_t *self = Self;
    int64_t value;
    rc_t rc = 0;
    
    rc = KMDataNodeReadAsI64(self->node, &value);
    if (rc == 0) {
        ((int8_t *)self->value.base)[0] = (int8_t)value;
        rslt->elem_bits = 8;
        rslt->elem_count = 1;
        KDataBufferWhack(rslt->data);
        rc = KDataBufferSub(&self->value, rslt->data, 0, UINT64_MAX);
    }
    return rc;
}

static
rc_t CC meta_value_I16(
                    void *Self,
                    const VXformInfo *info,
                    int64_t row_id,
                    VRowResult *rslt,
                    uint32_t argc,
                    const VRowData argv[]
) {
    self_t *self = Self;
    int64_t value;
    rc_t rc = 0;
    
    rc = KMDataNodeReadAsI64(self->node, &value);
    if (rc == 0) {
        ((int16_t *)self->value.base)[0] = (int16_t)value;
        rslt->elem_bits = 16;
        rslt->elem_count = 1;
        KDataBufferWhack(rslt->data);
        rc = KDataBufferSub(&self->value, rslt->data, 0, UINT64_MAX);
    }
    return rc;
}

static
rc_t CC meta_value_I32(
                    void *Self,
                    const VXformInfo *info,
                    int64_t row_id,
                    VRowResult *rslt,
                    uint32_t argc,
                    const VRowData argv[]
) {
    self_t *self = Self;
    int64_t value;
    rc_t rc = 0;
    
    rc = KMDataNodeReadAsI64(self->node, &value);
    if (rc == 0) {
        ((int32_t *)self->value.base)[0] = (int32_t)value;
        rslt->elem_bits = 32;
        rslt->elem_count = 1;
        KDataBufferWhack(rslt->data);
        rc = KDataBufferSub(&self->value, rslt->data, 0, UINT64_MAX);
    }
    return rc;
}

static
rc_t CC meta_value_I64(
                    void *Self,
                    const VXformInfo *info,
                    int64_t row_id,
                    VRowResult *rslt,
                    uint32_t argc,
                    const VRowData argv[]
) {
    self_t *self = Self;
    rc_t rc = 0;
    
    rc = KMDataNodeReadAsI64(self->node, (int64_t *)self->value.base);
    if (rc == 0) {
        rslt->elem_bits = 64;
        rslt->elem_count = 1;
        KDataBufferWhack(rslt->data);
        rc = KDataBufferSub(&self->value, rslt->data, 0, UINT64_MAX);
    }
    return rc;
}

static
rc_t CC meta_value_F32(
                    void *Self,
                    const VXformInfo *info,
                    int64_t row_id,
                    VRowResult *rslt,
                    uint32_t argc,
                    const VRowData argv[]
) {
    self_t *self = Self;
    double value;
    rc_t rc = 0;
    
    rc = KMDataNodeReadAsF64(self->node, &value);
    if (rc == 0) {
        ((float *)self->value.base)[0] = (float)value;
        rslt->elem_bits = 32;
        rslt->elem_count = 1;
        KDataBufferWhack(rslt->data);
        rc = KDataBufferSub(&self->value, rslt->data, 0, UINT64_MAX);
    }
    return rc;
}

static
rc_t CC meta_value_F64(
                    void *Self,
                    const VXformInfo *info,
                    int64_t row_id,
                    VRowResult *rslt,
                    uint32_t argc,
                    const VRowData argv[]
) {
    self_t *self = Self;
    rc_t rc = 0;
    
    rc = KMDataNodeReadAsF64(self->node, (double *)self->value.base);
    if (rc == 0) {
        rslt->elem_bits = 64;
        rslt->elem_count = 1;
        KDataBufferWhack(rslt->data);
        rc = KDataBufferSub(&self->value, rslt->data, 0, UINT64_MAX);
    }
    return rc;
}

/* 
 function < type T > T meta:value #1.0 < ascii node, * bool deterministic > ();
 */
VTRANSFACT_BUILTIN_IMPL(meta_value, 1, 0, 0) (const void *Self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rc_t rc = 0;
    self_t *self;
    bool deterministic = true;
    
    if (cp->argc > 1)
        deterministic = cp->argv[1].data.b[0];

    self = calloc(1, sizeof(self_t));
	if (self) {
        rc = KDataBufferMake(&self->value, info->fdesc.desc.intrinsic_bits, 1);
        if (rc == 0) {
            const KMetadata *meta;
            
            rc = VTableOpenMetadataRead(info->tbl, &meta);
            if (rc == 0) {
                rc = KMetadataOpenNodeRead(meta, &self->node, "%.*s", cp->argv[0].count, cp->argv[0].data.ascii);
                KMetadataRelease(meta);
                if (rc == 0) {
                    rslt->self = self;
                    rslt->whack = self_whack;
                    
                    rslt->variant = deterministic ? vftRow : vftNonDetRow;
                    switch (info->fdesc.desc.domain) {
                    case vtdFloat:
                        switch (info->fdesc.desc.intrinsic_bits) {
                        case 32:
                            rslt->u.ndf = meta_value_F32;
                            return 0;
                        case 64:
                            rslt->u.ndf = meta_value_F64;
                            return 0;
                        default:
                            break;
                        }
                        break;
                    case vtdInt:
                        switch (info->fdesc.desc.intrinsic_bits) {
                        case 8:
                            rslt->u.ndf = meta_value_I8;
                            return 0;
                        case 16:
                            rslt->u.ndf = meta_value_I16;
                            return 0;
                        case 32:
                            rslt->u.ndf = meta_value_I32;
                            return 0;
                        case 64:
                            rslt->u.ndf = meta_value_I64;
                            return 0;
                        default:
                            break;
                        }
                        break;
                    case vtdUint:
                        switch (info->fdesc.desc.intrinsic_bits) {
                        case 8:
                            rslt->u.ndf = meta_value_U8;
                            return 0;
                        case 16:
                            rslt->u.ndf = meta_value_U16;
                            return 0;
                        case 32:
                            rslt->u.ndf = meta_value_U32;
                            return 0;
                        case 64:
                            rslt->u.ndf = meta_value_U64;
                            return 0;
                        default:
                            break;
                        }
                        break;
                    default:
                        break;
                    }
                    rc = RC(rcVDB, rcFunction, rcConstructing, rcType, rcInvalid);
                }
            }
        }
        self_whack(self);
	}
	else
		rc = RC(rcVDB, rcFunction, rcConstructing, rcMemory, rcExhausted);
    return rc;
}
