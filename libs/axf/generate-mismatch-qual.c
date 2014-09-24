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
#include <vdb/vdb-priv.h>
#include <insdc/insdc.h>
#include <klib/data-buffer.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <bitstr.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

enum args {
    QUAL,
    HAS_MISMATCH,
    LEFT_CLIP,
    RIGHT_CLIP
};

static
rc_t CC generate_mismatch_qual ( void *data, const VXformInfo *info,
    int64_t row_id, VRowResult *rslt, uint32_t argc, const VRowData argv[] )
{
    INSDC_quality_phred const *qual = argv[ QUAL ].u.data.base;
    bool const *has_mismatch = argv[ HAS_MISMATCH ].u.data.base;
    INSDC_coord_len const *left  = argv[ LEFT_CLIP ].u.data.base;
    INSDC_coord_len const *right = argv[ RIGHT_CLIP ].u.data.base;
    unsigned const readlen = ( unsigned const )argv[ QUAL ].u.data.elem_count;
    INSDC_quality_phred *dst;
    rc_t rc;
    unsigned i;
    unsigned j;
    
    assert( argv[ QUAL ].u.data.elem_bits == sizeof( qual[ 0 ] ) * 8 );
    assert( argv[ HAS_MISMATCH ].u.data.elem_bits == sizeof( has_mismatch[ 0 ] ) * 8 );
    assert( argv[ LEFT_CLIP ].u.data.elem_bits == sizeof( left[ 0 ] ) * 8 );
    assert( argv[ RIGHT_CLIP ].u.data.elem_bits == sizeof( right[ 0 ] ) * 8 );
    assert( argv[ HAS_MISMATCH ].u.data.elem_count == readlen );
    
    qual += argv[ QUAL ].u.data.first_elem;
    has_mismatch += argv[ HAS_MISMATCH ].u.data.first_elem;
    left  += argv[ LEFT_CLIP  ].u.data.first_elem;
    right += argv[ RIGHT_CLIP ].u.data.first_elem;
    
    rslt->data->elem_bits = argv[ QUAL ].u.data.elem_bits;
    rc = KDataBufferResize( rslt->data, argv[ QUAL ].u.data.elem_count );
    if ( rc != 0 )
        return rc;
    dst = rslt->data->base;
    
    for ( j = 0, i = left[ 0 ]; i != readlen - right[ 0 ]; ++i )
    {
        if ( has_mismatch[ i ] )
            dst[ j++ ] = qual[ i ];
    }
    rslt->elem_count = j;
    return 0;
}


/*
 * function INSDC:quality:phred NCBI:align:generate_mismatch_qual #2 (
 *    INSDC:quality:phred qual, bool has_mismatch,
 *    INSDC:coord:len left_clip, INSDC:coord:len right_clip );
 */
VTRANSFACT_IMPL ( NCBI_align_generate_mismatch_qual_2, 2, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt -> u . rf = generate_mismatch_qual;
    rslt -> variant = vftRow;
    
    return 0;
}

static
rc_t CC compress_quality_impl ( void *data, const VXformInfo *info,
    int64_t row_id, VRowResult *rslt, uint32_t argc, const VRowData argv[] )
{
    INSDC_quality_phred const *qual = argv[ 0 ].u.data.base;
    bool const *preserve = argv[ 1 ].u.data.base;
    unsigned const readlen = ( unsigned const )argv[ 0 ].u.data.elem_count;
    INSDC_quality_phred *dst;
    rc_t rc;
    unsigned i;
    unsigned j;
    
    assert( argv[ 0 ].u.data.elem_bits == sizeof( qual[ 0 ] ) * 8 );
    assert( argv[ 1 ].u.data.elem_bits == sizeof( preserve[ 0 ] ) * 8 );
    assert( argv[ 1 ].u.data.elem_count == readlen );
    
    qual += argv[ 0 ].u.data.first_elem;
    preserve += argv[ 1 ].u.data.first_elem;
    
    rslt->data->elem_bits = argv[ 0 ].u.data.elem_bits;
    rc = KDataBufferResize( rslt->data, argv[ 0 ].u.data.elem_count );
    if ( rc != 0 )
        return rc;
    dst = rslt->data->base;
    
    for ( j = 0, i = 0; i != readlen; ++i )
    {
        if ( preserve[ i ] )
            dst[ j++ ] = qual[ i ];
    }
    rslt->elem_count = j;
    return 0;
}


/*
 * function INSDC:quality:phred NCBI:align:compress_quality #1
 *    ( INSDC:quality:phred quality, bool preserved );
 */
VTRANSFACT_IMPL ( NCBI_align_compress_quality, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt -> u . rf = compress_quality_impl;
    rslt -> variant = vftRow;
    
    return 0;
}
