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
#include <insdc/insdc.h>
#include <klib/data-buffer.h>
#include <sysalloc.h>
#include <assert.h>

#ifndef UNIT_TEST_FUNCTION
#include <vdb/xform.h>
#include <vdb/schema.h>
#endif

/*
    This is a schema-function to synthesize quality values.
    Its input are the read-len and the spot-filter column.
    It sums up the values in read-len to determin the length of the produced column
    It reads the spot-filter values and puts 'good' or 'bad' values into the output
    The literals for 'good' and 'bad' are not hardcoded but passed into the function
    from the schema.
*/

typedef struct syn_qual_params
{
    INSDC_quality_phred good;
    INSDC_quality_phred bad;
} syn_qual_params;

static INSDC_coord_len sum_read_len(size_t const count, INSDC_coord_len const *const lengths)
{
    INSDC_coord_len result = 0;
    size_t i;
    
    for (i = 0; i < count; ++i) {
        result += lengths[i];
    }
    return result;
}

static bool is_good(size_t const count, INSDC_SRA_spot_filter const *const filters)
{
    return (count == 0 || filters[0] == SRA_SPOT_FILTER_PASS) ? true : false;
}

static rc_t syn_quality_impl(syn_qual_params const *const params,
                             size_t numreads, INSDC_coord_len const *const lengths,
                             size_t numfilts, INSDC_SRA_spot_filter const *const filters,
                             KDataBuffer *rslt)
{
    rc_t rc = 0;
    INSDC_coord_len const total_read_len = sum_read_len(numreads, lengths);
    INSDC_quality_phred const q = is_good(numfilts, filters) ? params->good : params->bad;

    rslt->elem_bits = 8;
    rc = KDataBufferResize(rslt, total_read_len);
    if ( rc == 0 && total_read_len > 0 )
        memset(rslt->base, q, total_read_len);
    return rc;
}

#ifndef UNIT_TEST_FUNCTION

#define SAFE_BASE(ELEM, DTYPE) ((ELEM < argc && sizeof(DTYPE) * 8 == (size_t)argv[ELEM].u.data.elem_bits) ? (((DTYPE const *)argv[ELEM].u.data.base) + argv[ELEM].u.data.first_elem) : ((DTYPE const *)NULL))
#define BIND_COLUMN(ELEM, DTYPE, POINTER) DTYPE const *const POINTER = SAFE_BASE(ELEM, DTYPE)
#define SAFE_COUNT(ELEM) (ELEM < argc ? argv[ELEM].u.data.elem_count : 0)

static rc_t CC syn_quality_drvr ( void * self,
                                  const VXformInfo * info,
                                  int64_t row_id,
                                  VRowResult * rslt,
                                  uint32_t argc,
                                  const VRowData argv [] )
{
    enum {
        COL_READ_LEN,
        COL_SPOT_FILTER,
    };
    rc_t rc;
    assert(argc == 2);
    rc = syn_quality_impl
        (self,
         SAFE_COUNT(COL_READ_LEN), SAFE_BASE(COL_READ_LEN, INSDC_coord_len),
         SAFE_COUNT(COL_SPOT_FILTER), SAFE_BASE(COL_SPOT_FILTER, INSDC_SRA_spot_filter),
         rslt->data);
    rslt->elem_count = rslt->data->elem_count;
    return rc;
}

static void make_params(syn_qual_params *const params, VFactoryParams const *const fp)
{
    params->good = 30;
    params->bad = 3;
    if (fp->argc > 0) {
        assert(fp->argv[0].desc.domain == vtdUint && fp->argv[0].count == 1);
        params->good = fp->argv[0].data.u8[0];

        if (fp->argc > 1) {
            assert(fp->argv[1].desc.domain == vtdUint && fp->argv[1].count == 1);
            params->bad = fp->argv[1].data.u8[0];
        }
    }
}

static rc_t NCBI_SRA_syn_quality_factory(
    VFuncDesc * rslt,
    const VFactoryParams * cp,
    const VFunctionParams * dp )
{
    /* expecting 2 data arguments and 0, 1, or 2 factory arguments */
    assert(dp->argc == 2 && 0 <= cp->argc && cp->argc <= 2);

    rslt->whack = free;
    rslt->u.rf = syn_quality_drvr;
    rslt->variant = vftRow;
    rslt->self = malloc(sizeof(syn_qual_params));
    if (rslt->self) {
        make_params(rslt->self, cp);
        return 0;
    }
    return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);
}

/* 
 * function INSDC:quality:phred NCBI:syn_quality #1
 *      < * INSDC:quality:phred good_quality, INSDC:quality:phred bad_quality >
 *      ( INSDC:coord:len read_len, INSDC:SRA:spot_filter spot_filter );
 */
VTRANSFACT_IMPL ( NCBI_SRA_syn_quality, 1, 0, 0 ) ( const void * Self,
                                           const VXfactInfo * info,
                                           VFuncDesc * rslt,
                                           const VFactoryParams * cp,
                                           const VFunctionParams * dp )
{
    return NCBI_SRA_syn_quality_factory(rslt, cp, dp);
}

#else /* ifndef UNIT_TEST_FUNCTION */

#define ASSERT(X) do { if (!(X)) return -1; } while(0)

static int UnitTest_0Read_NoFilter(void)
{
    syn_qual_params p; p.good = 30; p.bad = 3;
    KDataBuffer buffer; memset(&buffer, 0, sizeof(buffer));
    INSDC_coord_len length[] = { 20, 40 };
    INSDC_SRA_spot_filter filter[] = { SRA_SPOT_FILTER_PASS, SRA_SPOT_FILTER_REJECT };
    {
        rc_t rc = syn_quality_impl(&p, 0, NULL, 0, NULL, &buffer);
        ASSERT(rc == 0);
        ASSERT(buffer.elem_bits == 8);
        ASSERT(buffer.elem_count == 0);
        (void)(length[0]);
        (void)(filter[0]);
    }
    KDataBufferWhack(&buffer);
    return 0;
}

static int UnitTest_1Read_NoFilter(void)
{
    syn_qual_params p; p.good = 30; p.bad = 3;
    KDataBuffer buffer; memset(&buffer, 0, sizeof(buffer));
    INSDC_coord_len length[] = { 20, 40 };
    INSDC_SRA_spot_filter filter[] = { SRA_SPOT_FILTER_PASS, SRA_SPOT_FILTER_REJECT };
    {
        unsigned i;
        rc_t rc = syn_quality_impl(&p, 1, length, 0, NULL, &buffer);
        ASSERT(rc == 0);
        ASSERT(buffer.elem_bits == 8);
        ASSERT(buffer.elem_count == length[0]);
        for (i = 0; i < buffer.elem_count; ++i) {
            INSDC_quality_phred const qv = ((INSDC_quality_phred *)buffer.base)[i];
            ASSERT(qv == p.good);
        }
        (void)(length[0]);
        (void)(filter[0]);
    }
    KDataBufferWhack(&buffer);
    return 0;
}

static int UnitTest_2Read_NoFilter(void)
{
    syn_qual_params p; p.good = 30; p.bad = 3;
    KDataBuffer buffer; memset(&buffer, 0, sizeof(buffer));
    INSDC_coord_len length[] = { 20, 40 };
    INSDC_SRA_spot_filter filter[] = { SRA_SPOT_FILTER_PASS, SRA_SPOT_FILTER_REJECT };
    {
        unsigned i;
        rc_t rc = syn_quality_impl(&p, 2, length, 0, NULL, &buffer);
        ASSERT(rc == 0);
        ASSERT(buffer.elem_bits == 8);
        ASSERT(buffer.elem_count == length[0] + length[1]);
        for (i = 0; i < buffer.elem_count; ++i) {
            INSDC_quality_phred const qv = ((INSDC_quality_phred *)buffer.base)[i];
            ASSERT(qv == p.good);
        }
        (void)(length[0]);
        (void)(filter[0]);
    }
    KDataBufferWhack(&buffer);
    return 0;
}

static int UnitTest_1Read_Pass(void)
{
    syn_qual_params p; p.good = 30; p.bad = 3;
    KDataBuffer buffer; memset(&buffer, 0, sizeof(buffer));
    INSDC_coord_len length[] = { 20, 40 };
    INSDC_SRA_spot_filter filter[] = { SRA_SPOT_FILTER_PASS, SRA_SPOT_FILTER_REJECT };
    {
        unsigned i;
        rc_t rc = syn_quality_impl(&p, 1, length, 1, filter, &buffer);
        ASSERT(rc == 0);
        ASSERT(buffer.elem_bits == 8);
        ASSERT(buffer.elem_count == length[0]);
        for (i = 0; i < buffer.elem_count; ++i) {
            INSDC_quality_phred const qv = ((INSDC_quality_phred *)buffer.base)[i];
            ASSERT(qv == p.good);
        }
        (void)(length[0]);
        (void)(filter[0]);
    }
    KDataBufferWhack(&buffer);
    return 0;
}

static int UnitTest_1Read_Fail(void)
{
    syn_qual_params p; p.good = 30; p.bad = 3;
    KDataBuffer buffer; memset(&buffer, 0, sizeof(buffer));
    INSDC_coord_len length[] = { 20, 40 };
    INSDC_SRA_spot_filter filter[] = { SRA_SPOT_FILTER_PASS, SRA_SPOT_FILTER_REJECT };
    {
        unsigned i;
        rc_t rc = syn_quality_impl(&p, 1, length, 1, filter + 1, &buffer);
        ASSERT(rc == 0);
        ASSERT(buffer.elem_bits == 8);
        ASSERT(buffer.elem_count == length[0]);
        for (i = 0; i < buffer.elem_count; ++i) {
            INSDC_quality_phred const qv = ((INSDC_quality_phred *)buffer.base)[i];
            ASSERT(qv == p.bad);
        }
        (void)(length[0]);
        (void)(filter[0]);
    }
    KDataBufferWhack(&buffer);
    return 0;
}

static int UnitTest_2Read_Pass(void)
{
    syn_qual_params p; p.good = 30; p.bad = 3;
    KDataBuffer buffer; memset(&buffer, 0, sizeof(buffer));
    INSDC_coord_len length[] = { 20, 40 };
    INSDC_SRA_spot_filter filter[] = { SRA_SPOT_FILTER_PASS, SRA_SPOT_FILTER_REJECT };
    {
        unsigned i;
        rc_t rc = syn_quality_impl(&p, 2, length, 1, filter, &buffer);
        ASSERT(rc == 0);
        ASSERT(buffer.elem_bits == 8);
        ASSERT(buffer.elem_count == length[0] + length[1]);
        for (i = 0; i < buffer.elem_count; ++i) {
            INSDC_quality_phred const qv = ((INSDC_quality_phred *)buffer.base)[i];
            ASSERT(qv == p.good);
        }
        (void)(length[0]);
        (void)(filter[0]);
    }
    KDataBufferWhack(&buffer);
    return 0;
}

static int UnitTest_2Read_Fail(void)
{
    syn_qual_params p; p.good = 30; p.bad = 3;
    KDataBuffer buffer; memset(&buffer, 0, sizeof(buffer));
    INSDC_coord_len length[] = { 20, 40 };
    INSDC_SRA_spot_filter filter[] = { SRA_SPOT_FILTER_PASS, SRA_SPOT_FILTER_REJECT };
    {
        unsigned i;
        rc_t rc = syn_quality_impl(&p, 2, length, 1, filter + 1, &buffer);
        ASSERT(rc == 0);
        ASSERT(buffer.elem_bits == 8);
        ASSERT(buffer.elem_count == length[0] + length[1]);
        for (i = 0; i < buffer.elem_count; ++i) {
            INSDC_quality_phred const qv = ((INSDC_quality_phred *)buffer.base)[i];
            ASSERT(qv == p.bad);
        }
        (void)(length[0]);
        (void)(filter[0]);
    }
    KDataBufferWhack(&buffer);
    return 0;
}
#endif
