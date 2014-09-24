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
#include <insdc/insdc.h>
#include <klib/defs.h>
#include <klib/rc.h>
#include <vdb/table.h>
#include <vdb/xform.h>
#include <vdb/schema.h>
#include <kdb/meta.h>
#include <klib/data-buffer.h>
#include <bitstr.h>
#include <sysalloc.h>

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static
rc_t CC mismatch_restore_qual_impl ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    unsigned i, j;
    INSDC_quality_phred const *qual	= argv[ 0 ].u.data.base;
    unsigned const nqual = ( unsigned const )argv[ 0 ].u.data.elem_count;
    bool const *mismatch = argv[ 1 ].u.data.base;
    unsigned const nmis = ( unsigned const )argv[ 1 ].u.data.elem_count;
    INSDC_quality_phred const rqual = ( INSDC_quality_phred const )( intptr_t )data;
    INSDC_quality_phred *dst;
    
    qual += argv[ 0 ].u.data.first_elem;
    mismatch += argv[ 1 ].u.data.first_elem;
    
    rslt->data->elem_bits = argv[ 0 ].u.data.elem_bits;
    rc = KDataBufferResize( rslt -> data, nmis );
    if ( rc != 0 )
        return rc;
    rslt -> elem_count = nmis;
    dst = rslt -> data -> base;
    
    for ( j = i = 0; i != nmis; ++i )
    {
        dst[ i ] = mismatch[ i ] ? qual[ j++ ] : rqual;
    }
    assert( j == nqual );
    return 0;
}


/* 
 * function INSDC:quality:phred NCBI:align:mismatch_restore_qual #1 < INSDC:quality:phred matched_qual >
 *    ( INSDC:quality:phred mismatch_qual, bool has_mismatch );
 */
VTRANSFACT_IMPL ( NCBI_align_mismatch_restore_qual, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->self = ( void * )( intptr_t )cp->argv[ 0 ].data.u8[ 0 ];
    rslt->u.rf = mismatch_restore_qual_impl;
    rslt->variant = vftRow;
    rslt -> whack = NULL;
    return 0;
}

static
rc_t CC decompress_quality_impl ( void *data, const VXformInfo *info, int64_t row_id,
                                  VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    unsigned i, j;
    INSDC_quality_phred const *qual	= argv[ 0 ].u.data.base;
    unsigned const nqual = ( unsigned const )argv[ 0 ].u.data.elem_count;
    bool const *preserved = argv[ 1 ].u.data.base;
    unsigned const readlen = ( unsigned const )argv[ 1 ].u.data.elem_count;
    INSDC_quality_phred const rqual = ( INSDC_quality_phred const )( intptr_t )data;
    INSDC_quality_phred *dst;
    
    qual += argv[ 0 ].u.data.first_elem;
    preserved += argv[ 1 ].u.data.first_elem;
    
    rslt->data->elem_bits = argv[ 0 ].u.data.elem_bits;
    rc = KDataBufferResize( rslt -> data, readlen );
    if ( rc != 0 )
        return rc;
    rslt -> elem_count = readlen;
    dst = rslt -> data -> base;
    
    for ( j = i = 0; i != readlen; ++i )
    {
        dst[ i ] = preserved[ i ] ? qual[ j++ ] : rqual;
    }
    assert( j == nqual );
    return 0;
}


/* 
 * function INSDC:quality:phred NCBI:align:decompress_quality #1
 *    < INSDC:quality:phred restored_qual_value >
 *    ( INSDC:quality:phred cmp_quality, bool preserved );
 */
VTRANSFACT_IMPL ( NCBI_align_decompress_quality, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
                                                              VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->self = ( void * )( intptr_t )cp->argv[ 0 ].data.u8[ 0 ];
    rslt->u.rf = decompress_quality_impl;
    rslt->variant = vftRow;
    rslt -> whack = NULL;
    return 0;
}
