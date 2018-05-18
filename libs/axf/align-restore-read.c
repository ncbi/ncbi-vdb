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
rc_t CC align_restore_read_impl ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    int mmi, roi, rri, di, bi;
    const INSDC_4na_bin	*ref_read 	= argv[ 0 ].u.data.base;
    const uint32_t	ref_read_len 	= (uint32_t)argv[ 0 ].u.data.elem_count;
    const uint8_t	*has_mismatch	= argv[ 1 ].u.data.base;
    const uint32_t	has_mismatch_len= (uint32_t)argv[ 1 ].u.data.elem_count;
    const INSDC_4na_bin *mismatch	= argv[ 2 ].u.data.base;
    const uint32_t	mismatch_len	= (uint32_t)argv[ 2 ].u.data.elem_count;
    const uint8_t	*has_ref_offset	   = argv[ 3 ].u.data.base;
    const uint32_t	has_ref_offset_len = (uint32_t)argv[ 3 ].u.data.elem_count;
    const int32_t 	*ref_offset	= argv[ 4 ].u.data.base;
    const uint32_t	ref_offset_len  = (uint32_t)argv[ 4 ].u.data.elem_count;
    uint32_t ploidy = 1, rl;
    const INSDC_coord_len* read_len = &has_mismatch_len;

    INSDC_4na_bin *dst;
    uint32_t	dst_len;
    
    assert( argv[ 0 ].u.data.elem_bits == 8 );
    assert( argv[ 1 ].u.data.elem_bits == 8 );
    assert( argv[ 2 ].u.data.elem_bits == 8 );
    assert( argv[ 3 ].u.data.elem_bits == 8 );
    assert( argv[ 4 ].u.data.elem_bits == 32 );

    ref_read	   += argv [ 0 ] . u . data . first_elem;
    has_mismatch   += argv [ 1 ] . u . data . first_elem;
    mismatch	   += argv [ 2 ] . u . data . first_elem;
    has_ref_offset += argv [ 3 ] . u . data . first_elem;
    ref_offset     += argv [ 4 ] . u . data . first_elem;

    if ( has_mismatch_len != has_ref_offset_len )
		return RC( rcXF, rcFunction, rcExecuting, rcData, rcInconsistent );

    if ( argc > 5 )
    {
        assert( argv[ 5 ].u.data.elem_bits == 32 );
        ploidy = (uint32_t)argv[ 5 ].u.data.elem_count;
        read_len = argv[ 5 ].u.data.base;
        read_len += argv[ 5 ].u.data.first_elem;
    }
    dst_len = has_mismatch_len;

    /* resize output row for the total number of reads */    
    rslt -> data -> elem_bits = 8;
    rc = KDataBufferResize ( rslt -> data, dst_len );
    if ( rc != 0 ) return rc;
    rslt -> elem_count = dst_len;
    dst = rslt -> data -> base;

    /**** MAIN RESTORATION LOOP ***/
    for ( mmi = roi = rri = di = bi = 0, rl = 1; di < (int)dst_len; di++, rri++, rl++, bi++ )
    {
        if ( has_ref_offset[ di ] && bi >= 0 ) /** bi can only become negative on Bs; skip has_ref_offset if Bs are not exhausted ***/
        {
            if ( roi >= (int)ref_offset_len )
                return RC( rcXF, rcFunction, rcExecuting, rcData, rcInconsistent );
            memmove ( & bi, ref_offset + roi, sizeof bi );
            rri += bi; /** can lead to negative rri ***/                
            roi++;
        }

        if ( has_mismatch[ di ] )
        {
            if ( mmi >= (int)mismatch_len )
                return RC( rcXF, rcFunction, rcExecuting, rcData, rcInconsistent );
            dst[ di ] = mismatch[ mmi ];
            mmi++;
        }
        else
        {
            if ( rri < 0 || rri >= (int)ref_read_len ) /*** can not have match after rolling back off the right edge ***/
            {
                return RC( rcXF, rcFunction, rcExecuting, rcData, rcInconsistent );
            }
            dst[ di ] = ref_read[ rri ];
        }

        if ( ploidy > 1 && rl == read_len[ 0 ] )
        {
            rri = -1;
            rl = 0;
            ploidy--;
            read_len++;
        }
    }
    return 0;
}


/* 
 * function
 * INSDC:4na:bin ALIGN:align_restore_read #1( INSDC:4na:bin ref_read, bool has_mismatch, INSDC:4na:bin mismatch, bool has_ref_offset, I32 ref_offset);
 */
VTRANSFACT_IMPL ( ALIGN_align_restore_read, 1, 0, 1 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{

    rslt->u.rf = align_restore_read_impl;
    rslt->variant = vftRow;
    rslt -> whack = NULL;
    return 0;
}
