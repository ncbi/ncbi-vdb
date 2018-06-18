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
rc_t CC make_spot_filter ( void *self, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    uint8_t const *read = ((uint8_t const *)argv[0].u.data.base) + argv[0].u.data.first_elem;
    uint8_t const *qual = ((uint8_t const *)argv[1].u.data.base) + argv[1].u.data.first_elem;
    int32_t const *start = ((int32_t const *)argv[2].u.data.base) + argv[2].u.data.first_elem;
    unsigned const nreads = argv[2].u.data.elem_count;
    uint32_t const *len = ((uint32_t const *)argv[3].u.data.base) + argv[3].u.data.first_elem;
    uint8_t const *type = ((uint8_t const *)argv[4].u.data.base) + argv[4].u.data.first_elem;
    uint8_t const *filt = ((uint8_t const *)argv[5].u.data.base) + argv[5].u.data.first_elem;

    bool pass = true;
    U8 *dst;
    unsigned i;
    
    assert(argv[3].u.data.elem_count == nreads);
    assert(argv[4].u.data.elem_count == nreads);
    assert(argv[5].u.data.elem_count == nreads);
    
    assert(argv[0].u.data.elem_bits == sizeof(*read) * 8);
    assert(argv[1].u.data.elem_bits == sizeof(*qual) * 8);
    assert(argv[2].u.data.elem_bits == sizeof(*start) * 8);
    assert(argv[3].u.data.elem_bits == sizeof(*len) * 8);
    assert(argv[4].u.data.elem_bits == sizeof(*type) * 8);
    assert(argv[5].u.data.elem_bits == sizeof(*filt) * 8);

    for (i = 0; i < nreads; ++i) {
        unsigned A = 0, C = 0, G = 0, T = 0, N = 0;
        unsigned const readLen = len[i];
        bool const rev = (type[i] & READ_TYPE_REVERSE) == READ_TYPE_REVERSE;
        unsigned j = 0;

        if ((type[i] & READ_TYPE_BIOLOGICAL) != READ_TYPE_BIOLOGICAL) continue;
        if (readLen < M) goto FAIL;
        if (filt[i] != READ_FILTER_PASS) goto FAIL;

        for ( ; j < M; ++j) {
            unsigned const k = start[i] + (rev ? (readLen - j - 1) : j);
            int const base = read[k];

            switch (base) {
            case 0:
                goto FAIL;
            case 1:
                ++A;
                break;
            case 2:
                ++C;
                break;
            case 4:
                ++G;
                break;
            case 8:
                ++T;
                break;
            default:
                ++N;
                break;
            }
            if (qual[k] < 3) goto FAIL;
        }
        if (N != 0 || ((A == 0 ? 0 : 1) + (C == 0 ? 0 : 1) + (G == 0 ? 0 : 1) + (T == 0 ? 0 : 1)) < 2)
            goto FAIL;
        
        for ( ; j < readLen; ++j) {
            unsigned const k = start[i] + (rev ? (readLen - j - 1) : j);
            int const base = read[k];

            switch (base) {
            case 0:
                goto FAIL;
            case 1:
                ++A;
                break;
            case 2:
                ++C;
                break;
            case 4:
                ++G;
                break;
            case 8:
                ++T;
                break;
            default:
                ++N;
                break;
            }
        }

        if (N >= A + C + G + T) {
    FAIL:
            pass = false;
            break;
        }
    }
    
    rslt->data->elem_bits = sizeof(*dst) * 8;
    rc = KDataBufferResize( rslt->data, 1 );
    if ( rc == 0 )
    {
        dst = rslt->data->base;
        dst[0] = pass ? READ_FILTER_PASS : READ_FILTER_REJECT;
        rslt->elem_bits = sizeof(*dst) * 8;
        rslt->elem_count = 1;
    }

    return rc;
}

/*
  function NCBI:SRA:spot_filter NCBI:SRA:make_spot_filter #1
       ( INSDC:dna:bin read, INSDC:quality:phred_33 quality,
         INSDC:coord:zero read_start, U32 read_len,
         INSDC:SRA:read_type read_type, INSDC:SRA:read_filter read_filter )

 */
VTRANSFACT_IMPL( NCBI_SRA_make_spot_filter, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt -> u . rf = make_spot_filter;
    rslt -> variant = vftRow;
    return 0;
}
