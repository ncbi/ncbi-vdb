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
#include <vdb/xform.h>
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vdb/schema.h>


#include <klib/defs.h>
#include <klib/rc.h>
#include <kdb/meta.h>
#include <klib/data-buffer.h>
#include <insdc/insdc.h>
#include <align/refseq-mgr.h>
#include <bitstr.h>
#include <sysalloc.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <insdc/sra.h>

static void copy( void * dst, size_t doff, void const * src, size_t soff, size_t bits )
{
    if ( ( doff & 7 ) == 0 && ( soff & 7 ) == 0 && ( bits & 7 ) == 0 )
    {
        memmove( &( ( char * )dst )[ doff >> 3 ], &( ( char const * )src )[ soff >> 3 ], bits >> 3 );
    }
    else
    {
        bitcpy( dst, doff, src, soff, bits );
    }
}


static
rc_t CC make_cmp_read_desc_inv_impl( void *data, const VXformInfo *info, int64_t row_id,
    VFixedRowResult const *rslt, uint32_t argc, VRowData const argv [] )
{
    int64_t const *const alignId = &( ( int64_t const * )argv[ 1 ].u.data.base )[ argv[ 1 ].u.data.first_elem ];
    void const *const ZERO = data;
    void const *const base = argv[ 0 ].u.data.base;
    unsigned src = ( unsigned )( argv[ 0 ].u.data.first_elem * rslt->elem_bits );
    unsigned dst = ( unsigned )(rslt->first_elem * rslt->elem_bits );
    unsigned i;
    
    for ( i = 0; i != rslt->elem_count; ++i )
    {
        bool const aligned = alignId[ i ] != 0;

        copy( rslt->base,
              dst,
              !aligned ? base : ZERO,
              !aligned ? src : 0,
              ( size_t )rslt->elem_bits );

        src += ( unsigned )rslt->elem_bits;
        dst += ( unsigned )rslt->elem_bits;
    }
    return 0;
}


static
rc_t CC make_cmp_read_desc_impl( void *data, const VXformInfo *info, int64_t row_id,
    VFixedRowResult const *rslt, uint32_t argc, VRowData const argv [] )
{
    int64_t const *const alignId = &( ( int64_t const * )argv[ 1 ].u.data.base )[ argv[ 1 ].u.data.first_elem ];
    void const *const ZERO = data;
    void const *const base = argv[ 0 ].u.data.base;
    unsigned src = ( unsigned )( argv[ 0 ].u.data.first_elem * rslt->elem_bits );
    unsigned dst = ( unsigned )( rslt->first_elem * rslt->elem_bits );
    unsigned i;
    
    for ( i = 0; i != rslt->elem_count; ++i )
    {
        bool const aligned = alignId[ i ] != 0;

        copy( rslt->base,
              dst,
              aligned ? base : ZERO,
              aligned ? src : 0,
              ( size_t )rslt->elem_bits );

        src += ( unsigned )rslt->elem_bits;
        dst += ( unsigned )rslt->elem_bits;
    }
    return 0;
}


static
rc_t CC make_read_start_impl( void *data, const VXformInfo *info, int64_t row_id,
                              VFixedRowResult const *rslt, uint32_t argc, VRowData const argv [] )
{
    INSDC_coord_zero *const dst = &( ( INSDC_coord_zero * )rslt->base )[ rslt->first_elem ];
    INSDC_coord_len const *const readlen = &( ( INSDC_coord_len const * )argv[ 0 ].u.data.base )[ argv[ 0 ].u.data.first_elem ];
    INSDC_coord_zero start;
    unsigned i;
    
    for ( start = 0, i = 0; i != rslt->elem_count; ++i )
    {
        INSDC_coord_len const rlen = readlen[ i ];

        dst[ i ] = start;
        start += rlen;
    }
    return 0;
}


static rc_t make_cmp_read_desc_factory( const void *Self,
                                        const VXfactInfo *info,
                                        VFuncDesc *rslt,
                                        const VFactoryParams *cp,
                                        const VFunctionParams *dp )
{
    rslt->self = calloc( 1, ( info->fdesc.desc.intrinsic_bits * info->fdesc.desc.intrinsic_bits + 7 ) >> 3 );
    if ( rslt->self == NULL )
        return RC( rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted );
    rslt->whack = free;
    rslt->u.pf = cp->argv[ 0 ].data.b[ 0 ] ? make_cmp_read_desc_inv_impl : make_cmp_read_desc_impl;
    rslt->variant = vftFixedRow;
    return 0;
}


/* 
 * function < type T >
 * T NCBI:align:make_cmp_read_desc #1 <bool invert>(T operand, I64 align_id);
 */
VTRANSFACT_IMPL ( NCBI_align_make_cmp_read_desc, 1, 0, 0 )
    ( const void *Self, const VXfactInfo *info, VFuncDesc *rslt,
      const VFactoryParams *cp, const VFunctionParams *dp )
{
    return make_cmp_read_desc_factory( Self, info, rslt, cp, dp );
}

/*
 * extern function INSDC:coord:zero NCBI:align:make_read_start #1
 *     (INSDC:coord:len read_len, I64 align_id);
 */
VTRANSFACT_IMPL ( NCBI_align_make_read_start, 1, 0, 0 )
    ( const void *Self, const VXfactInfo *info, VFuncDesc *rslt,
      const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.pf = make_read_start_impl;
    rslt->variant = vftFixedRow;
    return 0;
}
