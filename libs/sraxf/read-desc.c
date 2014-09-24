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

#include <sra/sradb.h>
#include <vdb/xform.h>
#include <klib/data-buffer.h>
#include <klib/rc.h>
#include <klib/text.h>
#include <sysalloc.h>

#include <assert.h>
#include <string.h>

static
rc_t CC make_read_desc ( void *self, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    SRAReadDesc *dst;

    const uint8_t *num_reads              = argv[0].u.data.base;
    const INSDC_coord_zero *read_start    = argv[1].u.data.base;
    const INSDC_coord_len *read_len       = argv[2].u.data.base;
    const INSDC_SRA_xread_type *read_type = argv[3].u.data.base;
    const INSDC_SRA_read_filter *read_flt = argv[4].u.data.base;
    const INSDC_dna_text *cs_key          = argv[5].u.data.base;
    const INSDC_coord_zero *lbl_start     = argv[6].u.data.base;
    const INSDC_coord_len *lbl_len        = argv[7].u.data.base;
    const char *label                     = argv[8].u.data.base;
    uint32_t label_max                    = argv[8].u.data.elem_count;

    num_reads += argv[0].u.data.first_elem; /* valid for argv[0].u.data.elem_count */
    assert( argv[0].u.data.elem_bits == (sizeof( *num_reads ) * 8 ) );

    read_start += argv[1].u.data.first_elem; /* valid for argv[1].u.data.elem_count */
    assert( argv[1].u.data.elem_bits == (sizeof( *read_start ) * 8 ) );
    assert( argv[1].u.data.elem_count >= *num_reads ); /*** some old bug overspecified metadata **/

    read_len += argv[2].u.data.first_elem; /* valid for argv[2].u.data.elem_count */
    assert( argv[2].u.data.elem_bits == (sizeof( *read_len ) * 8 ) );
    assert( argv[2].u.data.elem_count >= *num_reads );  /*** some old bug overspecified metadata **/

    read_type += argv[3].u.data.first_elem; /* valid for argv[3].u.data.elem_count */
    assert( argv[3].u.data.elem_bits == (sizeof( *read_type ) * 8 ) );
    assert( argv[3].u.data.elem_count >= *num_reads );  /*** some old bug overspecified metadata **/

    read_flt += argv[4].u.data.first_elem; /* valid for argv[4].u.data.elem_count */
    assert( argv[4].u.data.elem_bits == (sizeof( *read_flt ) * 8 ) );
    assert( argv[4].u.data.elem_count >= *num_reads );

    cs_key += argv[5].u.data.first_elem; /* valid for argv[5].u.data.elem_count */
    assert( argv[5].u.data.elem_bits == (sizeof( *cs_key ) * 8 ) );
    assert( argv[5].u.data.elem_count >= *num_reads );

    lbl_start += argv[6].u.data.first_elem; /* valid for argv[6].u.data.elem_count */
    assert( argv[6].u.data.elem_bits == (sizeof( *lbl_start ) * 8 ) );
    assert( argv[6].u.data.elem_count >= *num_reads );

    lbl_len += argv[7].u.data.first_elem; /* valid for argv[7].u.data.elem_count */
    assert( argv[7].u.data.elem_bits == (sizeof( *lbl_len ) * 8 ) );
    assert( argv[7].u.data.elem_count >= *num_reads );

    label += argv[8].u.data.first_elem; /* valid for argv[8].u.data.elem_count */
    assert( argv[8].u.data.elem_bits >= (sizeof( *label ) * 8 ) );

    rslt->data->elem_bits = sizeof(*dst) * 8;

    rc = KDataBufferResize( rslt->data, *num_reads );
    if ( rc == 0 )
    {
        uint32_t idx;
        dst = rslt->data->base;
        for ( idx = 0; idx < *num_reads; ++idx )
        {
            uint32_t label_start = lbl_start [ idx ];
            uint32_t label_len = lbl_len [ idx ];
            size_t   label_used;
            if ( label_start + label_len > label_max )
            {
                if ( label_start > label_max )
                    label_start = label_len = 0;
                else
                    label_len = label_max - label_start;
            }

            dst[idx].seg.start = (uint16_t)read_start[idx];
            dst[idx].seg.len = (uint16_t)read_len[idx];
            dst[idx].type = read_type[idx] & SRA_READ_TYPE_BIOLOGICAL;
            dst[idx].cs_key = cs_key[idx];

            label_used = string_copy( dst[idx].label, sizeof(dst[idx].label), 
                                      & label [ label_start ], label_len );
            /* Pad with NULs so that bitcmp sees fully initialized data */
            memset( dst[idx].label + label_used, '\0',
                    sizeof(dst[idx].label) - label_used );        
        }

        rslt->elem_bits = sizeof(*dst) * 8;
        rslt->elem_count = *num_reads;

    }

    return rc;
}

/*
 * function NCBI:SRA:ReadDesc NCBI:SRA:make_read_desc #1.0 ( U8 num_reads,
    INSDC:coord:zero read_start, U32 read_len, INSDC:SRA:read_type read_type,
    INSDC:SRA:read_filter read_filt, INSDC:dna:text cs_key,
    INSDC:coord:zero label_start, U32 label_len, ascii label );

 */
VTRANSFACT_IMPL( NCBI_SRA_make_read_desc, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt -> u . rf = make_read_desc;
    rslt -> variant = vftRow;
    return 0;
}
