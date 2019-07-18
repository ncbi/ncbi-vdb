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
    KMDataNode *node;
    void ( * byte_swap ) ( void *dst, const void *src, uint64_t count );
} self_t;

static void CC self_whack( void *vp ) {
    self_t *self = vp;
    
    KMDataNodeRelease( self->node );
    free( vp );
}

static
rc_t CC meta_write_func(
                     void *Self,
                     const VXformInfo *info,
                     int64_t row_id,
                     VRowResult *rslt,
                     uint32_t argc,
                     const VRowData argv[]
) {
    self_t *self = Self;

    /* pointer to input page */
    const uint8_t *sbuf = argv [ 0 ] . u . data . base;

    /* bit offset to first element in row */
    bitsz_t offset = (argv[0].u.data.first_elem * argv[0].u.data.elem_bits);

    /* the number of bits in this row */
    bitsz_t blen = argv[0].u.data.elem_count * argv[0].u.data.elem_bits;

    /* row length in bytes */
    size_t length = (blen + 7) >> 3;

    /* set output buffer size */
    rc_t rc = KDataBufferCast ( rslt -> data, rslt -> data, rslt -> elem_bits, true );
    if ( rc == 0 )
        rc = KDataBufferResize ( rslt -> data, (uint32_t)( blen / rslt -> elem_bits ) );
    if ( rc != 0 )
        return rc;

    /* copy data: if byte-swapping, then data are at least 16-bit aligned */
    if ( ( blen & 7 ) != 0 )
    {
        /* ensure trailing bits of buffer are zero */
        ( ( uint8_t* ) rslt -> data -> base ) [ length - 1 ] = 0;
        bitcpy ( rslt -> data -> base, 0, sbuf, offset, blen );
    }
    else if ( self -> byte_swap != NULL )
    {
        ( * self -> byte_swap ) ( rslt -> data -> base,
            & sbuf [ offset >> 3 ], argv[0].u.data.elem_count );
    }
    else
    {
        memmove ( rslt -> data -> base, & sbuf [ offset >> 3 ], length );
    }

    /* write row to metadata-node */
    rc = KMDataNodeWrite ( self -> node, rslt -> data -> base, length );
    if ( rc == 0 )
        rslt -> elem_count = rslt -> data -> elem_count;

    return rc;
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
