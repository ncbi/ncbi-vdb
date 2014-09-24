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
#include <sysalloc.h>

#include <string.h>
#include <assert.h>

static
rc_t CC make_spot_desc ( void *self, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    const uint32_t *spot_len   = argv[0].u.data.base;
    const uint32_t *fixed_len  = argv[1].u.data.base;
    const uint32_t *sig_len    = argv[2].u.data.base;
    const int32_t *trim_start  = argv[3].u.data.base;
    const uint32_t *trim_len   = argv[4].u.data.base;
    const uint8_t *num_reads   = argv[5].u.data.base;

    SRASpotDesc *dst;

    num_reads += argv[5].u.data.first_elem; /* valid for argv[0].u.data.elem_count */
    assert( argv[5].u.data.elem_bits == (sizeof( *num_reads ) * 8 ) );

    spot_len += argv[0].u.data.first_elem;
    assert( argv[0].u.data.elem_bits == (sizeof( *spot_len ) * 8 ) );

    fixed_len += argv[1].u.data.first_elem;
    assert( argv[1].u.data.elem_bits == (sizeof( *fixed_len ) * 8 ) );

    sig_len += argv[2].u.data.first_elem;
    assert( argv[2].u.data.elem_bits == (sizeof( *sig_len ) * 8 ) );

    trim_start += argv[3].u.data.first_elem;
    assert( argv[3].u.data.elem_bits == (sizeof( *trim_start ) * 8 ) );

    trim_len += argv[4].u.data.first_elem;
    assert( argv[4].u.data.elem_bits == (sizeof( *trim_len ) * 8 ) );

    rslt->data->elem_bits = sizeof(*dst) * 8;

    rc = KDataBufferResize( rslt->data, 1 );
    if ( rc == 0 )
    {
        dst = rslt->data->base;
        dst->spot_len = (uint16_t)spot_len[0];
        dst->fixed_len = (uint16_t)fixed_len[0];
        dst->signal_len = (uint16_t)sig_len[0];
        dst->clip_qual_right = (uint16_t)( trim_start[0]+trim_len[0] );
        dst->num_reads = num_reads[0];

        memset( dst->align, 0, sizeof( dst->align ) );
        
        rslt->elem_bits = sizeof(*dst) * 8;
        rslt->elem_count = 1;
    }

    return rc;
}

/*
  function NCBI:SRA:SpotDesc NCBI:SRA:make_spot_desc
       ( U32 spot_len, U32 fixed_len,  U32 sig_len,
        INSDC:coord:zero trim_start, U32 trim_len, U8 num_reads )

 */
VTRANSFACT_IMPL( NCBI_SRA_make_spot_desc, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt -> u . rf = make_spot_desc;
    rslt -> variant = vftRow;
    return 0;
}
