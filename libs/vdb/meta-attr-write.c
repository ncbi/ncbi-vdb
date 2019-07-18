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
#include <klib/printf.h>
#include <klib/rc.h>
#include <vdb/table.h>
#include <vdb/xform.h>
#include <vdb/schema.h>
#include <kdb/meta.h>
#include <klib/data-buffer.h>
#include <sysalloc.h>
#include "xform-priv.h"

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

typedef struct self_t {
    KMDataNode *node;
    char name[1];
} self_t;

static void CC self_whack( void *vp ) {
    self_t *self = vp;
    
    KMDataNodeRelease( self->node );
    free( vp );
}

static rc_t meta_attr_write_fmt( self_t *self, const char *fmt, ... )
{
    rc_t rc;
    size_t n;
    char sbuf[4096];

    va_list va;
    va_start(va, fmt);
    rc = string_vprintf(sbuf, sizeof(sbuf), & n, fmt, va);
    va_end(va);
    
    if ( rc == 0 )
        rc = KMDataNodeWriteAttr(self->node, self->name, sbuf);

    return rc;
}

static
rc_t CC meta_attr_write_ascii(
                             void *Self,
                             const VXformInfo *info,
                             int64_t row_id,
                             VRowResult *rslt,
                             uint32_t argc,
                             const VRowData argv[]
) {
    self_t *self = Self;

    char *value;
    size_t length = argv[0].u.data.elem_count;

    const char *src = argv[0].u.data.base;
    
    rc_t rc = KDataBufferResize( rslt->data, (uint32_t)( length + 1 ) );
    if (rc)
        return rc;
    
    value = rslt->data->base;
    memmove(value, & src [ argv[0].u.data.first_elem ], length);
    value[length] = '\0';

    rc = KMDataNodeWriteAttr(self->node, self->name, value);
    if (rc)
        return rc;

    rslt->elem_count = (uint32_t)length;
	return 0;
}

static
rc_t CC meta_attr_write_bool(
                          void *Self,
                          const VXformInfo *info,
                          int64_t row_id,
                          VRowResult *rslt,
                          uint32_t argc,
                          const VRowData argv[]                          
) {
    rc_t rc;
    self_t *self = Self;
    uint8_t value = ((const uint8_t *)argv[0].u.data.base)[argv[0].u.data.first_elem];

    /* TBD -this is not dependent upon C code, but upon schema.
       we probably want a warning here rather than an assert */
    assert(argv[0].u.data.elem_count == 1);

    rc = KDataBufferResize(rslt -> data, 1);
    if (rc)
        return rc;

    *(uint8_t *)rslt->data->base = value ? true : false;
    rc = meta_attr_write_fmt(self, "%s", value ? "true" : "false");
    if (rc)
        return rc;

    rslt->elem_count = 1;
	return 0;
}

static
rc_t CC meta_attr_write_I8(
                        void *Self,
                        const VXformInfo *info,
                        int64_t row_id,
                        VRowResult *rslt,
                        uint32_t argc,
                        const VRowData argv[]
) {
    self_t *self = Self;
    int8_t value = ((const int8_t *)argv[0].u.data.base)[argv[0].u.data.first_elem];
    rc_t rc = 0;
    
    assert(argv[0].u.data.elem_count == 1);
    
    rc = KDataBufferResize(rslt->data, 1);
    if (rc)
        return rc;
    
    *(int8_t *)rslt->data ->base = value;
    rc = meta_attr_write_fmt(self, "%d", (int)value);
    if (rc)
        return rc;
    
    rslt->elem_count = 1;
    
	return 0;
}

static
rc_t CC meta_attr_write_U8(
                        void *Self,
                        const VXformInfo *info,
                        int64_t row_id,
                        VRowResult *rslt,
                        uint32_t argc,
                        const VRowData argv[]
) {
    self_t *self = Self;
    uint8_t value = ((const uint8_t *)argv[0].u.data.base)[argv[0].u.data.first_elem];
    rc_t rc = 0;
    
    assert(argv[0].u.data.elem_count == 1);
    
    rc = KDataBufferResize(rslt->data, 1);
    if (rc)
        return rc;
    
    *(uint8_t *)rslt->data->base = value;
    rc = meta_attr_write_fmt(self, "%u", (int)value);
    if (rc)
        return rc;
    
    rslt->elem_count = 1;
    
	return 0;
}

static
rc_t CC meta_attr_write_I16(
                         void *Self,
                         const VXformInfo *info,
                         int64_t row_id,
                         VRowResult *rslt,
                         uint32_t argc,
                         const VRowData argv[]
) {
    self_t *self = Self;
    int16_t value = ((const int16_t *)argv[0].u.data.base)[argv[0].u.data.first_elem];
    rc_t rc = 0;
    
    assert(argv[0].u.data.elem_count == 1);
    
    rc = KDataBufferResize(rslt->data, 1);
    if (rc)
        return rc;
    
    *(int16_t *)rslt->data->base = value;
    rc = meta_attr_write_fmt(self, "%d", (int)value);
    if (rc)
        return rc;
    
    rslt->elem_count = 1;
    
	return 0;
}

static
rc_t CC meta_attr_write_U16(
                         void *Self,
                         const VXformInfo *info,
                         int64_t row_id,
                         VRowResult *rslt,
                         uint32_t argc,
                         const VRowData argv[]
) {
    self_t *self = Self;
    uint16_t value = ((const uint16_t *)argv[0].u.data.base)[argv[0].u.data.first_elem];
    rc_t rc = 0;
    
    assert(argv[0].u.data.elem_count == 1);
    
    rc = KDataBufferResize(rslt->data, 1);
    if (rc)
        return rc;
    
    *(uint16_t *)rslt->data->base = value;
    rc = meta_attr_write_fmt(self, "%u", (int)value);
    if (rc)
        return rc;
    
    rslt->elem_count = 1;
    
	return 0;
}

static
rc_t CC meta_attr_write_I32(
                         void *Self,
                         const VXformInfo *info,
                         int64_t row_id,
                         VRowResult *rslt,
                         uint32_t argc,
                         const VRowData argv[]
) {
    self_t *self = Self;
    int32_t value = ((const int32_t *)argv[0].u.data.base)[argv[0].u.data.first_elem];
    rc_t rc = 0;
    
    assert(argv[0].u.data.elem_count == 1);
    
    rc = KDataBufferResize(rslt->data, 1);
    if (rc)
        return rc;
    
    *(int32_t *)rslt->data->base = value;
    rc = meta_attr_write_fmt(self, "%d", value);
    if (rc)
        return rc;
    
    rslt->elem_count = 1;
    
	return 0;
}

static
rc_t CC meta_attr_write_U32(
                         void *Self,
                         const VXformInfo *info,
                         int64_t row_id,
                         VRowResult *rslt,
                         uint32_t argc,
                         const VRowData argv[]
) {
    self_t *self = Self;
    uint32_t value = ((const uint32_t *)argv[0].u.data.base)[argv[0].u.data.first_elem];
    rc_t rc = 0;
    
    assert(argv[0].u.data.elem_count == 1);
    
    rc = KDataBufferResize(rslt->data, 1);
    if (rc)
        return rc;
    
    *(uint32_t *)rslt->data->base = value;
    rc = meta_attr_write_fmt(self, "%u", value);
    if (rc)
        return rc;
    
    rslt->elem_count = 1;
    
	return 0;
}

static
rc_t CC meta_attr_write_I64(
                         void *Self,
                         const VXformInfo *info,
                         int64_t row_id,
                         VRowResult *rslt,
                         uint32_t argc,
                         const VRowData argv[]
) {
    self_t *self = Self;
    int64_t value = ((const int64_t *)argv[0].u.data.base)[argv[0].u.data.first_elem];
    rc_t rc = 0;
    
    assert(argv[0].u.data.elem_count == 1);
    
    rc = KDataBufferResize(rslt->data, 1);
    if (rc)
        return rc;
    
    *(int64_t *)rslt->data->base = value;
    rc = meta_attr_write_fmt(self, "%ld", value);
    if (rc)
        return rc;
    
    rslt->elem_count = 1;
    
	return 0;
}

static
rc_t CC meta_attr_write_U64(
                         void *Self,
                         const VXformInfo *info,
                         int64_t row_id,
                         VRowResult *rslt,
                         uint32_t argc,
                         const VRowData argv[]
) {
    self_t *self = Self;
    uint64_t value = ((const uint64_t *)argv[0].u.data.base)[argv[0].u.data.first_elem];
    rc_t rc = 0;
    
    assert(argv[0].u.data.elem_count == 1);
    
    rc = KDataBufferResize(rslt->data, 1);
    if (rc)
        return rc;
    
    *(uint64_t *)rslt->data->base = value;
    rc = meta_attr_write_fmt(self, "%lu", value);
    if (rc)
        return rc;
    
    rslt->elem_count = 1;
    
	return 0;
}

static
rc_t CC meta_attr_write_F32(
                         void *Self,
                         const VXformInfo *info,
                         int64_t row_id,
                         VRowResult *rslt,
                         uint32_t argc,
                         const VRowData argv[]
) {
    self_t *self = Self;
    float value = ((const float *)argv[0].u.data.base)[argv[0].u.data.first_elem];
    rc_t rc = 0;
    
    assert(argv[0].u.data.elem_count == 1);
    
    rc = KDataBufferResize(rslt->data, 1);
    if (rc)
        return rc;
    
    *(float *)rslt->data->base = value;
    rc = meta_attr_write_fmt(self, "%.8e", value);
    if (rc)
        return rc;
    
    rslt->elem_count = 1;
    
	return 0;
}

static
rc_t CC CC meta_attr_write_F64(
                         void *Self,
                         const VXformInfo *info,
                         int64_t row_id,
                         VRowResult *rslt,
                         uint32_t argc,
                         const VRowData argv[]
) {
    self_t *self = Self;
    double value = ((const double *)argv[0].u.data.base)[argv[0].u.data.first_elem];
    rc_t rc = 0;
    
    assert(argv[0].u.data.elem_count == 1);
    
    rc = KDataBufferResize(rslt->data, 1);
    if (rc)
        return rc;
    
    *(double *)rslt->data->base = value;
    rc = meta_attr_write_fmt(self, "%.16e", value);
    if (rc)
        return rc;
    
    rslt->elem_count = 1;
    
	return 0;
}

/* 
 */
VTRANSFACT_BUILTIN_IMPL(meta_attr_write, 1, 0, 0) (const void *Self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rc_t rc = 0;
    self_t *self;
    /* bool variable = false; */
    rc_t ( CC * var_row_func )(
                         void *Self,
                         const VXformInfo *info,
                         int64_t row_id,
                         VRowResult *rslt,
                         uint32_t argc,
                         const VRowData argv[]
                         );
    
    switch (dp->argv[0].desc.domain) {
    case vtdBool:
        var_row_func = meta_attr_write_bool;
        break;
    case vtdInt:
        switch (info->fdesc.desc.intrinsic_bits) {
        case 8:
            var_row_func = meta_attr_write_I8;
            break;
        case 16:
            var_row_func = meta_attr_write_I16;
            break;
        case 32:
            var_row_func = meta_attr_write_I32;
            break;
        case 64:
            var_row_func = meta_attr_write_I64;
            break;
        default:
            return RC(rcVDB, rcFunction, rcConstructing, rcType, rcInvalid);
        }
        break;
    case vtdUint:
        switch (info->fdesc.desc.intrinsic_bits) {
        case 8:
            var_row_func = meta_attr_write_U8;
            break;
        case 16:
            var_row_func = meta_attr_write_U16;
            break;
        case 32:
            var_row_func = meta_attr_write_U32;
            break;
        case 64:
            var_row_func = meta_attr_write_U64;
            break;
        default:
            return RC(rcVDB, rcFunction, rcConstructing, rcType, rcInvalid);
        }
        break;
    case vtdFloat:
        switch (info->fdesc.desc.intrinsic_bits) {
        case 32:
            var_row_func = meta_attr_write_F32;
            break;
        case 64:
            var_row_func = meta_attr_write_F64;
            break;
        default:
            return RC(rcVDB, rcFunction, rcConstructing, rcType, rcInvalid);
        }
        break;
    case vtdAscii:
        /* variable = true; */
        var_row_func = meta_attr_write_ascii;
        break;
    default:
        return RC(rcVDB, rcFunction, rcConstructing, rcType, rcInvalid);
    }
    
	self = calloc(1, sizeof *self + cp->argv[1].count);
	if (self == NULL)
		rc = RC(rcVDB, rcFunction, rcConstructing, rcMemory, rcExhausted);
    else
    {
        KMetadata *meta;
        strcpy(self->name, cp->argv[1].data.ascii);
        rc = VTableOpenMetadataUpdate((VTable *)info->tbl, &meta);
        if (rc == 0) {
            rc = KMetadataOpenNodeUpdate(meta, &self->node, "%s", cp->argv[0].data.ascii);
            KMetadataRelease(meta);
            if (rc == 0) {
                rslt->self = self;
                rslt->whack = self_whack;
                        
                rslt->variant = vftNonDetRow;
                rslt->u.ndf = var_row_func;
                        
                return 0;
            }
        }
        self_whack(self);
	}

	return rc;
}
