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

typedef struct self_t self_t;
struct self_t
{
    VTypedesc sdesc;
    VTypedesc ddesc;
    uint32_t sbits;
};

static
rc_t CC unpack_func( void *Self, const VXformInfo *info,
    void *dst, const void *src, uint64_t elem_count )
{
    size_t usize;
    const self_t *self = ( const void* ) Self;
    bitsz_t ssize = elem_count * VTypedescSizeof ( & self -> sdesc );
    size_t dsize = (elem_count * VTypedescSizeof ( & self -> ddesc ) + 7) >> 3;

    return Unpack( self -> sbits, self -> ddesc.intrinsic_bits,
        src, 0, ssize, NULL,
        dst, dsize, &usize );
}

static
void CC vxf_unpack_wrapper( void *ptr )
{
    free( ptr );
}

/* 
 */
VTRANSFACT_IMPL ( vdb_unpack, 1, 0, 0 ) ( const void *ignore, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    self_t *self = malloc ( sizeof * self );
    if ( self == NULL )
        return RC ( rcVDB, rcFunction, rcConstructing, rcMemory, rcExhausted );

    assert (dp->argc == 1);
    self->sdesc = dp->argv[0].desc;
    self->ddesc = info->fdesc.desc;
    self->sbits = VTypedescSizeof ( & dp->argv[0].desc );
        
    rslt->self = self;
    rslt->whack = vxf_unpack_wrapper;
        
    rslt->variant = vftArray;
    rslt->u.af = unpack_func;
        
    return 0;
}

VTRANSFACT_IMPL ( NCBI_unpack, 1, 0, 0 ) ( const void *ignore, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    self_t *self = malloc ( sizeof * self );
    if ( self == NULL )
        return RC ( rcVDB, rcFunction, rcConstructing, rcMemory, rcExhausted );

    assert ( dp -> argc == 1 );

    assert ( cp -> argc == 2 );
    assert ( cp -> argv [ 0 ] . desc . intrinsic_bits == 32 );
    assert ( cp -> argv [ 0 ] . desc . domain == vtdUint );

    self->sdesc = dp->argv[0].desc;
    self -> ddesc . intrinsic_bits = cp -> argv [ 1 ] . data . u32 [ 0 ];
    self -> ddesc . intrinsic_dim = 1;
    self -> ddesc . domain = 0;
    self -> sbits = cp -> argv [ 0 ] . data . u32 [ 0 ];

    rslt->self = self;
    rslt->whack = vxf_unpack_wrapper;
        
    rslt->variant = vftArray;
    rslt->u.af = unpack_func;
        
    return 0;
}
