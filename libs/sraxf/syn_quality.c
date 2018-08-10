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
#include <insdc/insdc.h>
#include <klib/data-buffer.h>
#include <bitstr.h>
#include <sysalloc.h>

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

static rc_t CC syn_quality_impl ( void *data,
                                  const VXformInfo *info,
                                  int64_t row_id,
                                  VRowResult *rslt,
                                  uint32_t argc,
                                  const VRowData argv [] )
{
    rc_t rc = 0;

    const INSDC_coord_len * read_lens = argv[ 0 ].u.data.base;
    const uint32_t num_read_lens = ( uint32_t )argv[ 0 ].u.data.elem_count;
    
    const INSDC_read_filter * read_filters = NULL;
    uint32_t num_read_filters = 0;
    
    uint32_t total_read_len = 0;
    uint32_t i;
    
    assert( argv[ 0 ].u.data.elem_bits == sizeof read_lens[ 0 ] );
    assert( argc > 0 );

    read_lens += argv[ 0 ].u.data.first_elem;
    
    for ( i = 0; i < num_read_filters; ++i )
        total_read_len += read_lens[ i ];
        
    if ( argc > 1 )
    {
        read_filters = argv[ 1 ].u.data.base;
        assert( argv[ 1 ].u.data.elem_bits == sizeof read_filters[ 0 ] );
        num_read_filters = ( uint32_t )argv[ 1 ].u.data.elem_count;
        read_filters += argv[ 0 ].u.data.first_elem;
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
                memset( dst, 30, total_read_len );
            }
            else
            {
                assert( num_read_lens == num_read_filters );
                for ( i = 0; i < num_read_filters; ++i )
                {
                    INSDC_coord_len len = read_lens[ i ];
                    INSDC_read_filter filter = read_filters[ i ];

                    INSDC_quality_phred value = 30;
                    if ( ( filter & READ_FILTER_REJECT ) > 0 )
                        value = 3;

                    memset( dst, value, len );
                    dst += len;
                }
            }
        }
    }

    return rc;
}

/* 
 * function INSDC:quality:phred NCBI:syn_quality #1
 *      < INSDC:quality:phred good_quality, INSDC:quality:phred bad_quality >
 *      ( INSDC:coord:len read_len,
 *        INSDC:SRA:read_filter read_filter );
 */
VTRANSFACT_IMPL ( syn_quality, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt -> u . rf  = syn_quality_impl;
    rslt -> variant = vftRow;
    return 0;
}
