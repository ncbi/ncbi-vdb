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


static
rc_t CC seq_construct_read ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    uint8_t *dst;

    /* base pointers to input data */
    const uint8_t *aligned = argv [ 0 ] . u . data . base;
    const INSDC_coord_len *aligned_len = argv [ 1 ] . u . data . base;
    const uint8_t *unaligned = argv [ 2 ] . u . data . base;
    const INSDC_coord_len *unaligned_len = argv [ 3 ] . u . data . base;

    /* the number of reads */
    uint32_t i, seqlen, aligned_seqlen, unaligned_seqlen;
    uint32_t nreads = (uint32_t)argv [ 1 ] . u . data . elem_count;

    /* require the same dimension for both inputs */
    if ( argv [ 1 ] . u . data . elem_count != argv [ 3 ] . u . data . elem_count )
        return RC ( rcXF, rcFunction, rcExecuting, rcData, rcInconsistent );

    /* offset base pointers to start of row */
    aligned += argv [ 0 ] . u . data . first_elem;
    aligned_len += argv [ 1 ] . u . data . first_elem;
    unaligned += argv [ 2 ] . u . data . first_elem;
    unaligned_len += argv [ 3 ] . u . data . first_elem;

    /* calculate new sequence length */
    for ( i = seqlen = 0; i < nreads; ++ i )
        seqlen += unaligned_len [ i ] != 0 ? unaligned_len [ i ] : aligned_len [ i ];

    /* set output buffer size */
    rslt -> data -> elem_bits = 8;
    rc = KDataBufferResize ( rslt -> data, seqlen );
    if ( rc != 0 )
        return rc;

    /* produce final sequence */
    for ( dst = rslt -> data -> base, i = seqlen = aligned_seqlen = unaligned_seqlen = 0; i < nreads; ++ i )
    {
        /* give preference to unaligned */
        if ( unaligned_len [ i ] != 0 )
        {
            memmove ( & dst [ seqlen ], & unaligned [ unaligned_seqlen ], unaligned_len [ i ] );
            seqlen += unaligned_len [ i ];
        }
        else
        {
            memmove ( & dst [ seqlen ], & aligned [ aligned_seqlen ], aligned_len [ i ] );
            seqlen += aligned_len [ i ];
        }

        aligned_seqlen += aligned_len [ i ];
        unaligned_seqlen += unaligned_len [ i ];
    }

    rslt -> elem_count = seqlen;

    return 0;
}

/* 
 * function < type T >
 * T NCBI:align:seq_construct_read #1 (T     read, INSDC:coord:len     read_len,
 *                                     T cmp_read, INSDC:coord:len cmp_read_len);
 */
VTRANSFACT_IMPL ( NCBI_align_seq_construct_read, 1, 0, 0 )
    ( const void *Self, const VXfactInfo *info,
      VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    if (info->fdesc.desc.intrinsic_bits * info->fdesc.desc.intrinsic_dim != 8)
        return RC(rcAlign, rcFunction, rcConstructing, rcType, rcIncorrect);
    rslt->u.ndf = seq_construct_read;
    rslt->variant = vftRow;
    return 0;
}
