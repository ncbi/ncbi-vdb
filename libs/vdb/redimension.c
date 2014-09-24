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

#include <assert.h>

typedef struct self_t {
    uint32_t elem_bits;
} self_t;

static
rc_t CC redimension_drvr(
                      void *Self,
                      const VXformInfo *info,
                      int64_t row_id,
                      VBlob **rslt,
                      uint32_t argc, const VBlob *argv[]
) {
    rc_t rc;
    const self_t *self = (const self_t *)Self;
    const VBlob *src = argv[0];
    VBlob *y;
    
    rc = VBlobNew(&y, src->start_id, src->stop_id, "redimension");
    if (rc)
        return rc;
    
    rc = KDataBufferCast(&src->data, &y->data, self->elem_bits, false);
    if (rc == 0) {
        y->byte_order = src->byte_order;
        PageMapAddRef(y->pm = src->pm);
	y->pm->optimized = eBlobPageMapOptimizedFailed; /** pagemap is no longer valid; prevent optimization ***/
        BlobHeadersAddRef(y->headers = src->headers);
        
        *rslt = y;
        return 0;
    }
	return rc;
}
static
void CC vfunc_free ( void * self )
{
    free ( self );
}

#include <stdio.h>

VTRANSFACT_BUILTIN_IMPL(vdb_redimension, 1, 0, 0) (
                                           const void *Self,
                                           const VXfactInfo *info,
                                           VFuncDesc *rslt,
                                           const VFactoryParams *cp,
                                           const VFunctionParams *dp
) {
    self_t *self;
    
    self = malloc(sizeof(*self));
    if (self) {
        self->elem_bits = VTypedescSizeof(&info->fdesc.desc);
        
        rslt->self = self;
        rslt->whack = vfunc_free;
        rslt->variant = vftBlobN;
        VFUNCDESC_INTERNAL_FUNCS(rslt)->bfN = redimension_drvr;
        return 0;
    }
    return RC(rcVDB, rcFunction, rcConstructing, rcMemory, rcExhausted);
}
