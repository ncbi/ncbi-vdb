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
#include <klib/text.h>
#include <sysalloc.h>
#include "xform-priv.h"

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct self_t {
    const KMDataNode *node;
    char *name;
    KDataBuffer value;
} self_t;

static void CC self_whack( void *vp ) {
    self_t *self = vp;
    
    KDataBufferWhack( &self->value );
    free( self->name );
    KMDataNodeRelease( self->node );
    free( vp );
}

static
rc_t CC meta_attr_read_cstring(
                            void *Self,
                            const VXformInfo *info,
                            int64_t row_id,
                            VRowResult *rslt,
                            uint32_t argc,
                            const VRowData argv[]
) {
    self_t *self = Self;
    size_t length;
    rc_t rc = 0;
    
    rc = KMDataNodeReadAttr(self->node, self->name, 0, 0, &length);
    if (rc)
        return rc;
    rc = KDataBufferResize( &self->value, (uint32_t)( length + 1 ) );
    if (rc)
        return rc;
    rc = KMDataNodeReadAttr( self->node, self->name, self->value.base, self->value.elem_count, &length );
    if (rc)
        return rc;
    KDataBufferWhack( rslt->data );
    KDataBufferSub( &self->value, rslt->data, 0, (uint32_t)length );
    rslt->elem_count = (uint32_t)length;
	return 0;
}

static
rc_t CC meta_attr_read_bool(
                       void *Self,
                       const VXformInfo *info,
                       int64_t row_id,
                       VRowResult *rslt,
                       uint32_t argc,
                       const VRowData argv[]
) {
    const self_t *self = Self;
    int16_t value;
    rc_t rc = 0;
    
    rc = KMDataNodeReadAttrAsI16(self->node, self->name, &value);
    if (rc)
        return rc;
    *(bool *)self->value.base = value == 0 ? false : true;
    KDataBufferWhack(rslt->data);
    KDataBufferSub(&self->value, rslt->data, 0, UINT64_MAX);
    rslt->elem_count = 1;
	return 0;
}

static
rc_t CC meta_attr_read_I8(
                        void *Self,
                        const VXformInfo *info,
                        int64_t row_id,
                        VRowResult *rslt,
                        uint32_t argc,
                        const VRowData argv[]
) {
    const self_t *self = Self;
    int16_t value;
    rc_t rc = 0;
    
    rc = KMDataNodeReadAttrAsI16(self->node, self->name, &value);
    if (rc)
        return rc;
    *(int8_t *)self->value.base = (int8_t)value;
    KDataBufferWhack(rslt->data);
    KDataBufferSub(&self->value, rslt->data, 0, UINT64_MAX);
    rslt->elem_count = 1;
	return 0;
}

static
rc_t CC meta_attr_read_U8(
                        void *Self,
                        const VXformInfo *info,
                        int64_t row_id,
                        VRowResult *rslt,
                        uint32_t argc,
                        const VRowData argv[]
) {
    const self_t *self = Self;
    uint16_t value;
    rc_t rc = 0;
    
    rc = KMDataNodeReadAttrAsU16(self->node, self->name, &value);
    if (rc)
        return rc;
    *(uint8_t *)self->value.base = (int8_t)value;
    KDataBufferWhack(rslt->data);
    KDataBufferSub(&self->value, rslt->data, 0, UINT64_MAX);
    rslt->elem_count = 1;
	return 0;
}

static
rc_t CC meta_attr_read_I16(
                        void *Self,
                        const VXformInfo *info,
                        int64_t row_id,
                        VRowResult *rslt,
                        uint32_t argc,
                        const VRowData argv[]
) {
    const self_t *self = Self;
    int16_t *value = self->value.base;
    rc_t rc = 0;
    
    rc = KMDataNodeReadAttrAsI16(self->node, self->name, value);
    if (rc)
        return rc;
    KDataBufferWhack(rslt->data);
    KDataBufferSub(&self->value, rslt->data, 0, UINT64_MAX);
    rslt->elem_count = 1;
	return 0;
}

static
rc_t CC meta_attr_read_U16(
                        void *Self,
                        const VXformInfo *info,
                        int64_t row_id,
                        VRowResult *rslt,
                        uint32_t argc,
                        const VRowData argv[]
) {
    const self_t *self = Self;
    uint16_t *value = self->value.base;
    rc_t rc = 0;
    
    rc = KMDataNodeReadAttrAsU16(self->node, self->name, value);
    if (rc)
        return rc;
    KDataBufferWhack(rslt->data);
    KDataBufferSub(&self->value, rslt->data, 0, UINT64_MAX);
    rslt->elem_count = 1;
	return 0;
}

static
rc_t CC meta_attr_read_I32(
                        void *Self,
                        const VXformInfo *info,
                        int64_t row_id,
                        VRowResult *rslt,
                        uint32_t argc,
                        const VRowData argv[]
) {
    const self_t *self = Self;
    int32_t *value = self->value.base;
    rc_t rc = 0;
    
    rc = KMDataNodeReadAttrAsI32(self->node, self->name, value);
    if (rc)
        return rc;
    KDataBufferWhack(rslt->data);
    KDataBufferSub(&self->value, rslt->data, 0, UINT64_MAX);
    rslt->elem_count = 1;
	return 0;
}

static
rc_t CC meta_attr_read_U32(
                        void *Self,
                        const VXformInfo *info,
                        int64_t row_id,
                        VRowResult *rslt,
                        uint32_t argc,
                        const VRowData argv[]
) {
    const self_t *self = Self;
    uint32_t *value = self->value.base;
    rc_t rc = 0;
    
    rc = KMDataNodeReadAttrAsU32(self->node, self->name, value);
    if (rc)
        return rc;
    KDataBufferWhack(rslt->data);
    KDataBufferSub(&self->value, rslt->data, 0, UINT64_MAX);
    rslt->elem_count = 1;
	return 0;
}

static
rc_t CC meta_attr_read_I64(
                        void *Self,
                        const VXformInfo *info,
                        int64_t row_id,
                        VRowResult *rslt,
                        uint32_t argc,
                        const VRowData argv[]
) {
    const self_t *self = Self;
    int64_t *value = self->value.base;
    rc_t rc = 0;
    
    rc = KMDataNodeReadAttrAsI64(self->node, self->name, value);
    if (rc)
        return rc;
    KDataBufferWhack(rslt->data);
    KDataBufferSub(&self->value, rslt->data, 0, UINT64_MAX);
    rslt->elem_count = 1;
	return 0;
}

static
rc_t CC meta_attr_read_U64(
                        void *Self,
                        const VXformInfo *info,
                        int64_t row_id,
                        VRowResult *rslt,
                        uint32_t argc,
                        const VRowData argv[]
) {
    const self_t *self = Self;
    uint64_t *value = self->value.base;
    rc_t rc = 0;
    
    rc = KMDataNodeReadAttrAsU64(self->node, self->name, value);
    if (rc)
        return rc;
    KDataBufferWhack(rslt->data);
    KDataBufferSub(&self->value, rslt->data, 0, UINT64_MAX);
    rslt->elem_count = 1;
	return 0;
}

static
rc_t CC meta_attr_read_F32(
                        void *Self,
                        const VXformInfo *info,
                        int64_t row_id,
                        VRowResult *rslt,
                        uint32_t argc,
                        const VRowData argv[]
) {
    const self_t *self = Self;
    double value;
    rc_t rc = 0;
    
    rc = KMDataNodeReadAttrAsF64(self->node, self->name, &value);
    if (rc)
        return rc;
    *(float *)self->value.base = (float)value;
    KDataBufferWhack(rslt->data);
    KDataBufferSub(&self->value, rslt->data, 0, UINT64_MAX);
    rslt->elem_count = 1;
	return 0;
}

static
rc_t CC meta_attr_read_F64(
                        void *Self,
                        const VXformInfo *info,
                        int64_t row_id,
                        VRowResult *rslt,
                        uint32_t argc,
                        const VRowData argv[]
) {
    const self_t *self = Self;
    double *value = self->value.base;
    rc_t rc = 0;
    
    rc = KMDataNodeReadAttrAsF64(self->node, self->name, value);
    if (rc)
        return rc;
    KDataBufferWhack(rslt->data);
    KDataBufferSub(&self->value, rslt->data, 0, UINT64_MAX);
    rslt->elem_count = 1;
	return 0;
}

/* 
 function ascii meta:attr:read #1.0 < ascii node, ascii attr, * bool deterministic > ();
 */
VTRANSFACT_BUILTIN_IMPL(meta_attr_read, 1, 0, 0) (const void *Self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rc_t rc = 0;
    self_t *self;
    bool variable = false;
    rc_t ( CC * var_row_func )(
                         void *Self,
                         const VXformInfo *info,
                         int64_t row_id,
                         VRowResult *rslt,
                         uint32_t argc,
                         const VRowData argv[]
                         );
    bool deterministic = true;
    
    if (cp->argc > 2)
        deterministic = cp->argv[2].data.b[0];
    
    switch (info->fdesc.desc.domain) {
    case vtdBool:
        var_row_func = meta_attr_read_bool;
        break;
    case vtdInt:
        switch (info->fdesc.desc.intrinsic_bits) {
        case 8:
            var_row_func = meta_attr_read_I8;
            break;
        case 16:
            var_row_func = meta_attr_read_I16;
            break;
        case 32:
            var_row_func = meta_attr_read_I32;
            break;
        case 64:
            var_row_func = meta_attr_read_I64;
            break;
        default:
            return RC(rcVDB, rcFunction, rcConstructing, rcType, rcInvalid);
        }
        break;
    case vtdUint:
        switch (info->fdesc.desc.intrinsic_bits) {
        case 8:
            var_row_func = meta_attr_read_U8;
            break;
        case 16:
            var_row_func = meta_attr_read_U16;
            break;
        case 32:
            var_row_func = meta_attr_read_U32;
            break;
        case 64:
            var_row_func = meta_attr_read_U64;
            break;
        default:
            return RC(rcVDB, rcFunction, rcConstructing, rcType, rcInvalid);
        }
        break;
    case vtdFloat:
        switch (info->fdesc.desc.intrinsic_bits) {
        case 32:
            var_row_func = meta_attr_read_F32;
            break;
        case 64:
            var_row_func = meta_attr_read_F64;
            break;
        default:
            return RC(rcVDB, rcFunction, rcConstructing, rcType, rcInvalid);
        }
        break;
    case vtdAscii:
        variable = true;
        var_row_func = meta_attr_read_cstring;
        break;
    default:
        return RC(rcVDB, rcFunction, rcConstructing, rcType, rcInvalid);
    }
    
	self = calloc(1, sizeof(self_t));
	if (self) {
        self->name = malloc(cp->argv[1].count + 1);
        if (self->name) {
            string_copy(self->name, cp->argv[1].count + 1, cp->argv[1].data.ascii,cp->argv[1].count);
            if (!variable)
                rc = KDataBufferMake(&self->value, info->fdesc.desc.intrinsic_bits, 1);
            else
                self->value.elem_bits = 8;
            if (rc == 0) {
                const KMetadata *meta;
                
                rc = VTableOpenMetadataRead(info->tbl, &meta);
                if (rc == 0) {
                    rc = KMetadataOpenNodeRead(meta, &self->node, "*.*s", cp->argv[0].count, cp->argv[0].data.ascii);
                    KMetadataRelease(meta);
                    if (rc == 0) {
                        rslt->self = self;
                        rslt->whack = self_whack;
                        
                        rslt->variant = deterministic ? vftRow : vftNonDetRow;
                        rslt->u.ndf = var_row_func;
                        
                        return 0;
                    }
                }
            }
        }
        self_whack(self);
	}
	else
		rc = RC(rcVDB, rcFunction, rcConstructing, rcMemory, rcExhausted);
	return rc;
}
