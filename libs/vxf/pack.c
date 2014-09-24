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
#include <vdb/xform.h>
#include <vdb/schema.h>
#include <sysalloc.h>

#include <klib/pack.h>

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>


typedef struct self_t {
    VTypedesc sdesc;
    uint32_t dbits;
} self_t;

static
rc_t CC pack_func(
               void *Self,
               const VXformInfo *info,
               void *dst,
               const void *src,
               uint64_t elem_count
) {
    const self_t *self = Self;
    bitsz_t psize;
    
    return Pack(
                self->sdesc.intrinsic_bits, self->dbits,
                src, (elem_count * self->sdesc.intrinsic_dim * self->sdesc.intrinsic_bits) >> 3, 0,
                dst, 0, elem_count * self->sdesc.intrinsic_dim * self->dbits, &psize
               );
}

static
void CC vxf_pack_wrapper( void *ptr )
{
	free( ptr );
}

/* 
 */
VTRANSFACT_IMPL(vdb_pack, 1, 0, 0) (const void *Self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rc_t rc = 0;
    self_t *self;

    self = malloc(sizeof(self_t));
    if (self) {
        self->dbits = info->fdesc.desc.intrinsic_dim;

        assert (dp->argc == 1);
        self->sdesc = dp->argv[0].desc;
        
        rslt->self = self;
        rslt->whack = vxf_pack_wrapper;

        rslt->variant = vftArray;
        rslt->u.af = pack_func;
    }
    else
        rc = RC(rcVDB, rcFunction, rcConstructing, rcMemory, rcExhausted);
    return rc;
}
