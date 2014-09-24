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
#include <klib/data-buffer.h>
#include <vdb/xform.h>
#include <vdb/schema.h>
#include <sysalloc.h>

#include "prod-priv.h"
#include "xform-priv.h"

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

static
rc_t CC fixed_row_len_func ( void *self, const VXformInfo *info,
    int64_t row_id, VRowResult *rslt, uint32_t argc, const VRowData in[] )
{
    assert ( rslt -> data -> elem_bits == 32 );
    if ( rslt -> data -> elem_count == 0 )
    {
        rc_t rc = KDataBufferResize ( rslt -> data, 1 );
        if ( rc != 0 )
            return rc;
    }

    * ( uint32_t* ) rslt -> data -> base = VProductionFixedRowLength ( info -> prod, row_id, true );
    rslt -> elem_count = 1;

    return 0;
}

VTRANSFACT_BUILTIN_IMPL ( vdb_fixed_row_len, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt -> variant = vftRow;
    rslt -> u . rf = fixed_row_len_func;
    return 0;
}
