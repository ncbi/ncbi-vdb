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
#include "xform-priv.h"

#include <klib/defs.h>
#include <klib/rc.h>
#include <vdb/table.h>
#include <vdb/xform.h>
#include <vdb/schema.h>
#include <kdb/meta.h>
#include <klib/data-buffer.h>
#include <vdb/vdb-priv.h>
#include <sysalloc.h>

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

static
rc_t CC parameter_read_func(
                         void *Self,
                         const VXformInfo *info,
                         int64_t row_id,
                         VRowResult *rslt,
                         uint32_t argc,
                         const VRowData argv[]
) {
    const KDataBuffer *value = Self;
    rc_t rc = 0;
    
    rslt->data->elem_bits = value->elem_bits;
    rslt->data->elem_count = 0;
    rc = KDataBufferResize(rslt->data, value->elem_count);
    if (rc == 0) {
        memcpy(rslt->data->base, value->base, KDataBufferBytes(value));
        rc = KDataBufferCast(rslt->data, rslt->data, rslt->elem_bits, true);
        if (rc == 0)
            rslt->elem_count = rslt->data->elem_count;
    }
	return rc;
}

static rc_t get_databuffer( KDataBuffer **rslt, struct VCursorParams const *params, const char *Name, size_t len ) {
    char name[4096];
    
    if (len >= sizeof(name))
        return RC(rcVDB, rcFunction, rcConstructing, rcName, rcTooLong);
    
    memcpy(name, Name, len);
    name[len] = '\0';
    
    return VCursorParamsGet(params, name, rslt);
}

/* 
 function utf8 parameter:read #1.0 < ascii name, * bool deterministic > ();
 */
VTRANSFACT_BUILTIN_IMPL(parameter_read, 1, 0, 0)
    (const void *Self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp
) {
    rc_t rc;
    KDataBuffer *value;
    bool deterministic = true;
    
    if (cp->argc > 1)
        deterministic = cp->argv[1].data.b[0];

    rc = get_databuffer(&value, info->parms, cp->argv[0].data.ascii, cp->argv[0].count);
    if (rc == 0) {
        rslt->self = value;
        
        rslt->variant = deterministic ? vftRow : vftNonDetRow;
        rslt->u.ndf = parameter_read_func;
    }

	return rc;
}
