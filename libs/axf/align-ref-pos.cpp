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
#include <vdb/vdb-priv.h>
#include <insdc/insdc.h>
#include <klib/data-buffer.h>
#include <klib/container.h>
#include <klib/rc.h>
#include <klib/debug.h>
#include <sysalloc.h>

#include <bitstr.h>

#include "ref-tbl.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <vector>
#include <map>
#include <algorithm>

#ifdef _DEBUGGING
#define SUB_DEBUG(msg) DBGMSG(DBG_SRA,DBG_FLAG(DBG_SRA_SUB),msg)
#else
#define SUB_DEBUG(msg)
#endif

typedef struct RefPos RefPos;
typedef struct RowRange RowRange;

struct RowRange {
    int64_t start_id;
    int64_t stop_id;

    RowRange(int64_t p_start_id, int64_t p_stop_id)
    : start_id(p_start_id)
    , stop_id(p_stop_id)
    {}

    RowRange(int64_t row)
    : start_id(row)
    , stop_id(row)
    {}

    void extendTo(int64_t row) {
        stop_id = stop_id < row ? row : stop_id;
    }

    bool operator <(int64_t rhs) const {
        return start_id < rhs;
    }
    bool operator <(RowRange const &rhs) const {
        return *this < rhs.start_id;
    }
};

static extern "C" void loadNamedRanges(void *ctx, int64_t row, char const *name, uint32_t length)
{
    auto &rangeNamed = *reinterpret_cast<std::map<std::string, RowRange> *>(ctx);
    auto const key = std::string(name, length);
    auto const iter = rangeNamed.find(key);

    if (iter == rangeNamed.end())
        rangeNamed.insert(iter, std::pair(key, RowRange(row)));
    else
        iter->second.extendTo(row);
}

struct RowRangeList : public std::vector<RowRange> {
    rc_t loadFrom(VCursor const *refCurs) {
        std::map<std::string, RowRange> rangeNamed;
        auto rc = AlignRefTableCursorForEachName(refCurs, loadNamedRanges, &rangeNamed);
        if (rc) return rc;

        for (auto &[k, v] : rangeNamed) {
            insert(v);
        }
        std::sort(begin(), end());

        return 0;
    }
    int64_t lookup(int64_t row) const {
        auto const fnd = std::lower_bound(begin(), end(), row);
        return (fnd != end() && row <= fnd->stop_id) ? fnd->start_id : 0;
    }
}

struct RefPos {
    RowRangeList array;
    uint32_t max_seq_len;

    RefPos(VTable const *const tbl, VCursor const *const native_curs) {
        VCursor const *curs = NULL;
        rc_t rc = AlignRefTableCursor(tbl, native_curs, &curs, NULL);
        if (rc) throw rc;

        rc = AlignRefTableCursorGetMaxSeqLen(curs, &max_seq_len);
        if (rc) throw rc;

        rc = array.loadFrom(curs);
        if (rc) throw rc;

        VCursorRelease(curs);
    }
};

static extern "C" void refPosWhack(void *obj) {
    auto self = reinterpret_cast<RefPos *>(obj);
    delete self;
}

enum align_ref_pos_args {
    REF_ID,
    REF_START,
    REF_PLOIDY,
    PLOIDY
};

/*
function INSDC:coord:zero NCBI:align:ref_pos ( I64 ref_id, INSDC:coord:zero ref_start );
*/
static extern "C"
rc_t CC align_ref_pos ( void *data, const VXformInfo *info,
    int64_t row_id, VRowResult *rslt, uint32_t argc, const VRowData argv[] )
{
    rc_t rc = 0;
    RefPos const *self = ( void const * )data;
    int64_t ref_row_id = 0;
    INSDC_coord_zero *ref_pos;
    unsigned const ploidy = ( unsigned const )argv[ REF_START ].u.data.elem_count;
    unsigned i;

    /* get start and length of reference segment */
    int64_t const *ref_id = 0;
    INSDC_coord_zero const *ref_start;

    assert( argv[ REF_ID ].u.data.elem_bits == sizeof( *ref_id ) * 8 );
    assert( argv[ REF_START ].u.data.elem_bits == sizeof( *ref_start ) * 8 );

    ref_id = argv[ REF_ID ].u.data.base;
    ref_id += argv[ REF_ID ].u.data.first_elem;

    ref_start = argv[ REF_START ].u.data.base;
    ref_start += argv[ REF_START ].u.data.first_elem;

    if (self->max_seq_len > 0) {
        ref_row_id = self->array.lookup(ref_id[0]);
        if (ref_row_id == 0)
            return RC( rcXF, rcTable, rcAccessing, rcRow, rcNotFound );
    }

    rc = KDataBufferResize( rslt->data, ploidy );
    if ( rc != 0 )
        return rc;
    
    ref_pos = rslt->data->base;
    for ( i = 0; i != ploidy; ++i )
    {
        ref_pos[ i ] = ref_start[ i ]
                     + ( INSDC_coord_zero )( ( ref_id[ 0 ] - ref_row_id ) * self->max_seq_len );
    }
    rslt->elem_count = ploidy;
    rslt->elem_bits = sizeof( ref_pos[ 0 ] ) * 8;

    return rc;
}


VTRANSFACT_IMPL ( NCBI_align_ref_pos, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    RefPos *fself;
    rc_t rc = RefPosMake( &fself, info -> tbl, ( const VCursor* )info->parms );
    if ( rc == 0 )
    {
        rslt -> self = fself;
        rslt -> whack = refPosWhack;
        rslt -> u . rf = align_ref_pos;
        rslt -> variant = vftRowFast;
    }

    return rc;
}
