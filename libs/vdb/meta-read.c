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

#include "schema-priv.h"
#include "xform-priv.h"

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct self_t {
    const KMDataNode *node;
    void ( * byte_swap ) ( void *dst, const void *src, uint64_t count );
} self_t;

static void CC self_whack( void *vp ) {
    self_t *self = vp;
    
    KMDataNodeRelease( self->node );
    free( vp );
}

static
rc_t CC meta_read_func(
                     void *Self,
                     const VXformInfo *info,
                     int64_t row_id,
                     VRowResult *rslt,
                     uint32_t argc,
                     const VRowData argv[]
) {
    self_t *self = Self;
    size_t length;
    size_t read;
    rc_t rc = 0;
    
    rc = KMDataNodeRead(self->node, 0, 0, 0, &read, &length);
    if (rc == 0) {
        rslt->data->elem_bits = 8;
        rslt->data->elem_count = 0; /* no usable data */
        rc = KDataBufferResize( rslt->data, length );
        if (rc)
            return rc;
        
        rc = KMDataNodeRead( self->node, 0, rslt->data->base, length, &length, 0 );
        if (rc == 0) {
            rc = KDataBufferCast(rslt->data, rslt->data, rslt->elem_bits, true);
            if (rc == 0) {
                if (self->byte_swap)
                    self->byte_swap(rslt->data->base, rslt->data->base, rslt->data->elem_count);
                rslt->elem_count = rslt->data->elem_count;
            }
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
