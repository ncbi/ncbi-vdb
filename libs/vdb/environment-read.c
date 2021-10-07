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
#include <vdb/vdb-priv.h>
#include <sysalloc.h>

#include "xform-priv.h"

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

static
rc_t CC environment_read_func(void *Self,
                              const VXformInfo *info,
                              int64_t row_id,
                              VRowResult *rslt,
                              uint32_t argc,
                              const VRowData argv[]
                              )
{
    KDataBuffer const *const value = Self;

    // Self, refcount == 1
    KDataBufferSub(value, rslt->data, 0, value->elem_count);
    // rslt->data = Self, refcount == 2
    // prod-cmn.c:538 blob->data = rslt->data, refcount == 3
    // prod-cmn.c:540 rslt->data, refcount == 2
    // blob.c:130 blob->data, refcount == 1
    return 0;
}

static rc_t CC getEnvToDataBuffer(KDataBuffer *const rslt, size_t const name_len, const char *const name)
{
    // probably need to add a NUL to the name before calling getenv
    char *x = NULL;
    char const *env_val = NULL;
    rc_t rc = KDataBufferMakeBytes(rslt, name_len + 1);

    if (rc) return rc;

    x = rslt->base;
    memmove(x, name, name_len);
    x[name_len] = '\0';

    env_val = getenv(name);
    if (env_val) {
        size_t const len = strlen(x);
        if (rc == 0)
            rc = KDataBufferResize(rslt, len);
        if (rc == 0)
            memmove(rslt->base, x, len);
    }
    else
        rc = RC(rcVDB, rcFunction, rcConstructing, rcName, rcNotFound);
    if (rc)
        KDataBufferWhack(rslt);

    return rc;
}

/* 
 function utf8 environment:read #1.0 < ascii name > ();
 */
VTRANSFACT_BUILTIN_IMPL(environment_read, 1, 0, 0)
    (const void *Self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp
) {
    rc_t rc;
    KDataBuffer *value = calloc(1, sizeof(*value));

    if (value != NULL) {
        rc = getEnvToDataBuffer(value, cp->argv[0].count, cp->argv[0].data.ascii);
        if (value->elem_count > (uint64_t)(UINT32_MAX))
            rc = RC(rcVDB, rcFunction, rcConstructing, rcItem, rcTooLong);
        else if (rc == 0) {
            rslt->self = value;
            rslt->whack = (void (*)(void *))KDataBufferWhack;

            rslt->variant = vftRow;
            rslt->u.rf = environment_read_func;
        }
    }
    else
        rc = RC(rcXF, rcSelf, rcAllocating, rcMemory, rcExhausted);

	return rc;
}
