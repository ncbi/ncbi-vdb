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
#include <klib/log.h>

#include <string.h>
#include <assert.h>

#include "spot-check.c"


#define SPOT_FILTER ((self_t const *)self)->spot_filter

#define SAFE_BASE(ELEM, DTYPE) ((ELEM < argc && sizeof(DTYPE) * 8 == (size_t)argv[ELEM].u.data.elem_bits) ? (((DTYPE const *)argv[ELEM].u.data.base) + argv[ELEM].u.data.first_elem) : ((DTYPE const *)NULL))
#define BIND_COLUMN(ELEM, DTYPE, POINTER) DTYPE const *const POINTER = SAFE_BASE(ELEM, DTYPE)
#define SAFE_COUNT(ELEM) (ELEM < argc ? argv[ELEM].u.data.elem_count : 0)
#define SAME_COUNT(ELEM1, ELEM2) (SAFE_COUNT(ELEM1) == SAFE_COUNT(ELEM2))

static
rc_t CC make_spot_filter(void *const self
                         , const VXformInfo *const info
                         , int64_t const row_id
                         , VRowResult *const rslt
                         , uint32_t const argc
                         , const VRowData *const argv)
{
    enum COLUMNS {
        COL_READ,
        COL_QUALITY,
        COL_READ_START,
        COL_READ_LEN,
        COL_READ_TYPE,
        COL_SPOT_FILTER
    };
    unsigned const nreads = (unsigned)SAFE_COUNT(COL_READ_LEN);
    BIND_COLUMN(COL_READ       ,  uint8_t, read  ); ///< 4NA or x2na or 2na
    BIND_COLUMN(COL_QUALITY    ,  uint8_t, qual  ); ///< phred+0
    BIND_COLUMN(COL_READ_START ,  int32_t, start );
    BIND_COLUMN(COL_READ_LEN   , uint32_t, len   );
    BIND_COLUMN(COL_READ_TYPE  ,  uint8_t, type  );
    BIND_COLUMN(COL_SPOT_FILTER,  uint8_t, filter);
    rc_t rc = 0;

    assert(read != NULL);
    assert(qual != NULL);
    assert(start != NULL);
    assert(len != NULL);
    assert(type != NULL);
    assert(filter != NULL);
    assert(SAME_COUNT(COL_READ, COL_QUALITY));
    assert(SAME_COUNT(COL_READ_START, COL_READ_LEN));
    assert(SAME_COUNT(COL_READ_START, COL_READ_TYPE));
    assert(SAFE_COUNT(COL_SPOT_FILTER) == 1);
    
    rslt->data->elem_bits = 8;
    rslt->elem_bits = rslt->data->elem_bits;
    rslt->elem_count = 1;
    rc = KDataBufferResize( rslt->data, 1 );
    if (rc == 0) {
        INSDC_SRA_spot_filter result = filter[0];
        enum RejectCause cause = spotFilter;
        if (result == SRA_SPOT_FILTER_PASS) {
            cause = check_quality(self, nreads, start, len, type, qual);
 #if USE_BASE_FILTER
            if (cause == notRejected)
                cause = SPOT_FILTER(self, nreads, start, len, type, read);
 #endif
            result = cause == notRejected ? SRA_SPOT_FILTER_PASS : SRA_SPOT_FILTER_REJECT;
        }
        printRejectCause(cause, self);
        *((uint8_t *)rslt->data->base) = result;
    }
    return rc;
}

static bool isSameType(VSchema const *const schema, char const *const type_name, VFormatdecl const *const qry)
{
    VFormatdecl decl;
    rc_t const rc = VSchemaResolveFmtdecl(schema, &decl, type_name);
    if (rc != 0 || qry->td.type_id != decl.td.type_id)
        return false;
#if 0
    PLOGMSG(klogDebug, (klogDebug, "type matched $(name)", "name=%s", type_name));
#endif
    return true;
}

static spot_filter_func read_data_type_to_function(VSchema const *const schema, VFormatdecl const *const read_data_type)
{
    if (isSameType(schema, "INSDC:4na:bin", read_data_type))
        return check_4na;
    if (isSameType(schema, "INSDC:x2na:bin", read_data_type))
        return check_x2na;
    if (isSameType(schema, "INSDC:2na:bin", read_data_type))
        return check_2na;
    if (isSameType(schema, "INSDC:dna:text", read_data_type))
        return check_dna;
    LOGMSG(klogDebug, "matched no type");
    return NULL;
}

/*
  function NCBI:SRA:spot_filter < * U32 min_length, * U8 min_quality, * U8 no_quality >
  NCBI:SRA:make_spot_filter #1
       ( INSDC:dna:bin read, INSDC:quality:phred quality,
         INSDC:coord:zero read_start, U32 read_len,
         INSDC:SRA:read_type read_type, INSDC:SRA:spot_filter spot_filter )

 */
VTRANSFACT_IMPL( NCBI_SRA_make_spot_filter, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    spot_filter_func func = read_data_type_to_function(info->schema, &dp->argv[0].fd);
    if (func) {
        self_t *self = initialize_params(malloc(sizeof(*self)));
        assert(self != NULL);
        assert(cp->argc <= 3);
        if (cp->argc >= 1) {
            assert(cp->argv[0].count == 1);
            self->min_length = cp->argv[0].data.u32[0];
            if (cp->argc >= 2) {
                assert(cp->argv[1].count == 1);
                self->min_quality = cp->argv[1].data.u8[0];
                if (cp->argc >= 3) {
                    assert(cp->argv[2].count == 1);
                    self->no_quality = cp->argv[2].data.u8[0];
                }
            }
        }        
        self->spot_filter = func;
        rslt->self = self;
        rslt->whack = free;
        rslt -> u . rf = make_spot_filter;
        rslt -> variant = vftRow;
        return 0;
    }
    return RC(rcXF, rcFunction, rcConstructing, rcType, rcInvalid);
}

/*******************************************************************************
 * MARK: read filter <--> spot filter conversion routines
 */

/* read2spot_filter
 *  takes input from read filter bits
 *  produces new-style whole-spot filter bits
 *
 *  Rules are: (listed in order of precedence) ///< per KR email 2018-Sep-25
 *    1) REJECT, if any are REJECT
 *    2) REDACTED, if any are REDACTED
 *    3) CRITERIA, if any are CRITERIA
 *    4) else PASS
 *
 * function INSDC:SRA:spot_filter
 *     INSDC:SRA:read2spot_filter #1 ( INSDC:SRA:read_filter out_read_filter );
 */
static unsigned read_filter_array_to_bitset(unsigned const nreads, INSDC_read_filter const read_filter[])
{
    unsigned bits = 0;
    unsigned i;

    for (i = 0; i < nreads; ++i) {
        INSDC_read_filter const filter = read_filter[i];
        unsigned const bit = 1u << filter;
        bits |= bit;
    }
    assert(bits < 16);
    return bits;
}

#define DONT_USE_TABLE 0
#if DONT_USE_TABLE
static INSDC_SRA_spot_filter bitset_to_spot_filter(unsigned const bits)
{
    // *    1) REJECT, if any are REJECT
    if ((bits & (1u << SRA_READ_FILTER_REJECT)) != 0)
        return SRA_SPOT_FILTER_REJECT;
    
    // *    2) REDACTED, if any are REDACTED
    if ((bits & (1u << SRA_READ_FILTER_REDACTED)) != 0)
        return SRA_SPOT_FILTER_REDACTED;
    
    // *    3) CRITERIA, if any are CRITERIA
    if ((bits & (1u << SRA_READ_FILTER_CRITERIA)) != 0)
        return SRA_SPOT_FILTER_CRITERIA;
    
    return SRA_SPOT_FILTER_PASS;
}
#endif

static INSDC_SRA_spot_filter spot_filter_from_read_filter(unsigned const nreads, INSDC_read_filter const read_filter[])
{
    unsigned const bits = read_filter_array_to_bitset(nreads, read_filter);
#if DONT_USE_TABLE
    return bitset_to_spot_filter(bits);
#else
    // equivalent to expanding bitset_to_spot_filter for all
    // possibilities but with consecutive equal values removed
    static INSDC_SRA_spot_filter const results[] = {
        SRA_SPOT_FILTER_PASS,
        SRA_SPOT_FILTER_REJECT,
        SRA_SPOT_FILTER_CRITERIA,
        SRA_SPOT_FILTER_REJECT,
        SRA_SPOT_FILTER_REDACTED,
        SRA_SPOT_FILTER_REJECT,
        SRA_SPOT_FILTER_REDACTED,
        SRA_SPOT_FILTER_REJECT
    };
    return results[bits >> 1];
#endif
}

static
rc_t CC read2spot_filter(  void *const self
                         , const VXformInfo *const info
                         , int64_t const row_id
                         , VRowResult *const rslt
                         , uint32_t const argc
                         , const VRowData *const argv)
{
    enum COLUMNS {
        COL_READ_FILTER
    };
    unsigned const nfilt = (unsigned)SAFE_COUNT(COL_READ_FILTER);
    BIND_COLUMN(COL_READ_FILTER, INSDC_read_filter, filter);
    rc_t rc = 0;
    
    rslt->data->elem_bits = 8;
    rc = KDataBufferResize( rslt->data, 1 );
    if ( rc == 0 )
    {
        uint8_t *const dst = (uint8_t *)rslt->data->base;
        
        rslt->elem_bits = rslt->data->elem_bits;
        rslt->elem_count = 1;
        dst[0] = spot_filter_from_read_filter(nfilt, filter);
    }
    return rc;
}

VTRANSFACT_IMPL( INSDC_SRA_read2spot_filter, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
#if DONT_USE_TABLE
#else
    // this function assumes these values for read filter
    assert(    (1u << SRA_READ_FILTER_PASS) == 1);
    assert(  (1u << SRA_READ_FILTER_REJECT) == 2);
    assert((1u << SRA_READ_FILTER_CRITERIA) == 4);
    assert((1u << SRA_READ_FILTER_REDACTED) == 8);
#endif
    rslt -> u . rf = read2spot_filter;
    rslt -> variant = vftRow;
    return 0;
}

/* spot2read_filter
 *  takes input from whole-spot filter bits
 *  produces older-style array of per-read filter bits
 *  based upon dimension and possibly type of "out_read_type"
 *
 * function INSDC:SRA:read_filter
 *    INSDC:SRA:spot2read_filter #1 ( INSDC:SRA:spot_filter out_spot_filter, INSDC:SRA:xread_type out_read_type );
 */
static void fill_array(unsigned const count, INSDC_read_filter result[], INSDC_read_filter const value)
{
    unsigned i;
    
    for (i = 0; i < count; ++i) {
        result[i] = value;
    }
}

static
rc_t CC spot2read_filter(  void *const self
                         , const VXformInfo *const info
                         , int64_t const row_id
                         , VRowResult *const rslt
                         , uint32_t const argc
                         , VRowData const argv[])
{
    enum COLUMNS {
        COL_SPOT_FILTER,
        COL_READ_TYPE
    };
    unsigned const nfilt = (unsigned)SAFE_COUNT(COL_SPOT_FILTER);
    unsigned const nreads = (unsigned)SAFE_COUNT(COL_READ_TYPE);
    BIND_COLUMN(COL_SPOT_FILTER, INSDC_SRA_spot_filter, filter);
    INSDC_SRA_spot_filter const spot_value = (nfilt && filter) ? filter[0] : SRA_SPOT_FILTER_PASS;
    INSDC_SRA_read_filter const read_value = (INSDC_SRA_read_filter)spot_value;
    rc_t rc = 0;
    
    rslt->data->elem_bits = 8;
    rc = KDataBufferResize( rslt->data, nreads );
    if ( rc == 0 )
    {
        rslt->elem_bits = rslt->data->elem_bits;
        rslt->elem_count = nreads;
        fill_array(nreads, rslt->data->base, read_value);
    }
    return rc;
}

VTRANSFACT_IMPL( INSDC_SRA_spot2read_filter, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    // this function assumes read filter and spot filter values are equal
    assert((int)    SRA_READ_FILTER_PASS == (int)SRA_SPOT_FILTER_PASS    );
    assert((int)  SRA_READ_FILTER_REJECT == (int)SRA_SPOT_FILTER_REJECT  );
    assert((int)SRA_READ_FILTER_REDACTED == (int)SRA_SPOT_FILTER_REDACTED);
    assert((int)SRA_READ_FILTER_CRITERIA == (int)SRA_SPOT_FILTER_CRITERIA);
    rslt -> u . rf = spot2read_filter;
    rslt -> variant = vftRow;
    return 0;
}
