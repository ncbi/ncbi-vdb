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

#include <klib/rc.h>
#include <vdb/xform.h>
#include <vdb/schema.h>
#include <insdc/insdc.h>
#include <klib/data-buffer.h>
#include <sysalloc.h>

#include <assert.h>

/*
    This is a schema-function to synthesize quality values.
    Its input are the read-len and the read-filter column.
    It sums up the values in read-len to determin the length of the produced column
    It reads the read-filter values and puts 'good' or 'bad' values into the output
    The literals for 'good' and 'bad' are not hardcoded but passed into the function
    from the schema.
*/

typedef struct syn_qual_params
{
    INSDC_quality_phred good;
    INSDC_quality_phred bad;
} syn_qual_params;

static rc_t CC syn_quality_impl ( void * self,
                                  const VXformInfo * info,
                                  int64_t row_id,
                                  VRowResult * rslt,
                                  uint32_t argc,
                                  const VRowData argv [] )
{
    rc_t rc = 0;

    const syn_qual_params * params = self;
    const INSDC_coord_len * read_lens = NULL;
    uint32_t num_read_lens = 0;
    
    const INSDC_read_filter * read_filters = NULL;
    uint32_t num_read_filters = 0;
    
    INSDC_coord_len total_read_len = 0;
    uint32_t i;
    
    if ( argc > 0 )
    {
        read_lens = argv[ 0 ] . u . data . base;
        read_lens += argv[ 0 ] . u . data . first_elem;
        num_read_lens = ( uint32_t )argv[ 0 ] . u . data . elem_count;    
    }
    
    for ( i = 0; i < num_read_lens; ++i )
        total_read_len += read_lens[ i ];

    if ( argc > 1 )
    {
        read_filters = argv[ 1 ].u.data.base;
        if ( argv[ 1 ] . u . data . elem_bits == ( ( sizeof read_filters[ 0 ] ) * 8 ) )
        {
            num_read_filters = ( uint32_t )argv[ 1 ] . u . data . elem_count;
            read_filters += argv[ 1 ] . u . data . first_elem;
        }
    }
    
    rslt -> data -> elem_bits = 8;
    rslt -> elem_count = total_read_len;
    if ( total_read_len > 0 )
    {
        rc = KDataBufferResize ( rslt -> data, total_read_len );
        if ( rc == 0 )
        {
            INSDC_quality_phred * dst = rslt -> data -> base;
            if ( num_read_filters == 0 )
            {
                memset( dst, params -> good, total_read_len );
            }
            else
            {
                for ( i = 0; i < num_read_lens; ++i )
                {
                    INSDC_coord_len len = read_lens[ i ];
                    INSDC_quality_phred q = params -> good;

                    if ( i < num_read_filters )
                    {
                        if ( read_filters[ i ] == READ_FILTER_REJECT )
                            q = params -> bad;
                    }
                    
                    memset( dst, q, len );
                    dst += len;
                }
            }
        }
    }
    return rc;
}

/* 
 * function INSDC:quality:phred NCBI:syn_quality #1
 *      < * INSDC:quality:phred good_quality, INSDC:quality:phred bad_quality >
 *      ( INSDC:coord:len read_len, INSDC:SRA:read_filter read_filter );
 */
VTRANSFACT_IMPL ( NCBI_SRA_syn_quality, 1, 0, 0 ) ( const void * Self,
                                           const VXfactInfo * info,
                                           VFuncDesc * rslt,
                                           const VFactoryParams * cp,
                                           const VFunctionParams * dp )
{
    rc_t rc = 0;
    INSDC_quality_phred q_good = 30;
    INSDC_quality_phred q_bad  = 3;

    if ( cp -> argc > 0 )
    {
        if ( cp -> argv[ 0 ].desc.domain == vtdUint &&
             cp -> argv[ 0 ].count > 0 )
        {
            q_good = cp -> argv[ 0 ] . data . u8[ 0 ];
        }

        if ( cp -> argc > 1 )
        {
            if ( cp -> argv[ 1 ].desc.domain == vtdUint &&
                 cp -> argv[ 1 ].count > 0 )
            {
                q_bad = cp -> argv[ 1 ] . data . u8[ 0 ];
            }
        }
    }
    
    syn_qual_params * params = malloc( sizeof * params );
    if ( params == NULL )
        rc = RC( rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted );
    else
    {
        params -> good = q_good;
        params -> bad  = q_bad;
        
        rslt -> self = params;
        rslt -> whack = free;
        rslt -> u . rf  = syn_quality_impl;
        rslt -> variant = vftRow;
    }
    return rc;
}
