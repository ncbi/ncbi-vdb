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
#include <bitstr.h>
#include <sysalloc.h>

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

static
void dna_from_color(uint8_t dnabin[], const uint8_t csbin[], size_t n, uint8_t primer, const uint8_t conversion_matrix[]) {
    uint8_t st;
    size_t i;
	
    switch (primer) {
    case 'A':
        st = 0;
        break;
    case 'C':
        st = 1;
        break;
    case 'G':
        st = 2;
        break;
    case 'T':
        st = 3;
        break;
    default:
        st = 4;
        break;
    }
	
    for (i = 0; i != n && st !=4; ++i) { /*** interrupt translation after the first ambiguity in color space ***/
        uint8_t tr = csbin[i];
		
        st = dnabin[i] = conversion_matrix[st * 5 + tr];
    }
    if(i<n) memset(dnabin+i,4,n-i); /*** translation was interrupted - the remainder is ambiguity in base space ***/
}

static
rc_t CC dna_from_color_drvr ( void *self, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    uint8_t *dnabin;

    uint32_t i, row_len = argv [ 0 ] . u . data . elem_count;
    uint32_t out_len, nreads = argv [ 1 ] . u . data . elem_count;

    const uint8_t *csbin = argv [ 0 ] . u . data . base;
    const uint32_t *read_start = argv [ 1 ] . u . data . base;
    const uint32_t *read_len = argv [ 2 ] . u . data . base;
    const uint8_t *cskey = argv [ 3 ] . u . data . base;
    const uint8_t *cmatrx = argv [ 4 ] . u . data . base;
    
    assert(argv[0].u.data.elem_bits == 8);
    assert(argv[1].u.data.elem_bits == 32);
    assert(argv[2].u.data.elem_bits == 32);
    assert(argv[3].u.data.elem_bits == 8);
    assert(argv[4].u.data.elem_bits == 8);

    assert(argv[2].u.data.elem_count == nreads);
    assert(argv[3].u.data.elem_count == nreads);

    csbin += argv [ 0 ] . u . data . first_elem;
    read_start += argv [ 1 ] . u . data . first_elem;
    read_len += argv [ 2 ] . u . data . first_elem;
    cskey += argv [ 3 ] . u . data . first_elem;
    cmatrx += argv [ 4 ] . u . data . first_elem;

    /* safety check that read structure is within bounds */
    for ( i = out_len = 0; i < nreads; ++i )
    {
        if ( read_start [ i ] + read_len [ i ] > row_len )
            return RC ( rcXF, rcFunction, rcExecuting, rcData, rcInvalid );
        out_len += read_len [ i ];
    }
    
    /* resize output row for the total number of reads */    
    rslt -> data -> elem_bits = 8;
    rc = KDataBufferResize ( rslt -> data, out_len );
    if ( rc != 0 )
        return rc;

    /* huh? */
    rslt -> data -> bit_offset = 0;

    rslt -> elem_count = out_len;
    dnabin = rslt -> data -> base;

    for ( i = out_len = 0; i < nreads; ++i )
    {
        dna_from_color ( & dnabin [ out_len ],
            & csbin [ read_start [ i ] ], read_len [ i ],
            cskey [ i ], cmatrx );

        out_len += read_len [ i ];
    }
    
    return 0;
}

/* 
 * function
 * INSDC:color:bin NCBI:dna_from_color #1 ( INSDC:dna:bin dna_bin,
 *     INSDC:coord:zero read_start, U32 read_len,
 *     INSDC:dna:text cs_key, U8 color_matrix )
 */
VTRANSFACT_IMPL ( NCBI_dna_from_color, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = dna_from_color_drvr;
    rslt->variant = vftRow;
    
    return 0;
}
