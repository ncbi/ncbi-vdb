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
#include <klib/rc.h>
#include <klib/debug.h>
#include <sysalloc.h>

#include <bitstr.h>

#include "ref-tbl.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>


#ifdef _DEBUGGING
#define SUB_DEBUG(msg) DBGMSG(DBG_SRA,DBG_FLAG(DBG_SRA_SUB),msg)
#else
#define SUB_DEBUG(msg)
#endif


typedef struct RefTableSubSelect RefTableSubSelect;
struct RefTableSubSelect
{
    rc_t (*func)(struct RefTableSubSelect* self, int64_t ref_row_id,
                 INSDC_coord_zero offset, INSDC_coord_len ref_len,
                 uint32_t ref_ploidy, VRowResult* rslt);
    const VCursor *curs;
    uint32_t out_idx;
    union {
        struct {
            uint32_t circular_idx;
            uint32_t name_idx;
            uint32_t name_range_idx;
            uint32_t seq_len_idx;
            uint32_t max_seq_len_idx;
            uint32_t cmp_read_idx;
            /* set once upon 1st call */
            /* set once per each call, if changed from previous */
            char* name;
            int64_t start_id;
            int64_t stop_id;
            uint32_t name_len;
            uint32_t max_seq_len;
            INSDC_coord_len seq_len;
            bool circular;
            bool local;
        } ref;
        struct {
            uint32_t ref_id_idx;
            uint32_t ref_start_idx;
            uint32_t ref_len_idx;
            uint32_t read_start_idx;
            uint32_t read_len_idx;
            struct RefTableSubSelect* parent;
        } mod;
    } u;
};

/*
  ref_ploidy != 0 means that offset here is relative to ref_row_id, so it can be
    negative or positive and can extend between rows within same refseq
  ref_ploidy means that offset is normal REF_START
 */

static
rc_t CC REFERENCE_TABLE_sub_select( RefTableSubSelect* self, int64_t ref_row_id,
                                    INSDC_coord_zero offset, INSDC_coord_len ref_len,
                                    uint32_t ref_ploidy, VRowResult* rslt )
{
    rc_t rc = 0;
    INSDC_coord_len num_read;

    if ( ref_row_id < self->u.ref.start_id || ref_row_id > self->u.ref.stop_id )
    {
        /* update cached ref data if ref has changed */
        const char* n;
        uint32_t n_len;
        struct {
            int64_t start_id;
            int64_t stop_id;
        } *out;
    
        SUB_DEBUG( ( "SUB.Rd in 'ref-tbl-sub-select.c' (REF) at #%lu offset %lu\n", ref_row_id, offset ) );

        rc = VCursorCellDataDirect( self->curs, ref_row_id, self->u.ref.name_idx, NULL, ( const void** )&n, NULL, &n_len );
        if ( rc == 0 )
        {
            rc = VCursorParamsSet( ( const struct VCursorParams * )(self->curs), "QUERY_SEQ_NAME", "%.*s", n_len, n );
            if ( rc == 0 )
            {
                rc = VCursorCellDataDirect( self->curs, ref_row_id, self->u.ref.name_range_idx, NULL, (const void**)&out, NULL, NULL );
                if ( rc == 0 )
                {
                    if ( self->u.ref.name_len < n_len )
                    {
                        void* p = realloc( self->u.ref.name, n_len );
                        if ( p == NULL )
                            rc = RC( rcXF, rcFunction, rcSelecting, rcMemory, rcExhausted );
                        else
                            self->u.ref.name = ( char* )p;
                    }

                    if ( rc == 0 )
                    {
                        const bool* c;
                        INSDC_coord_len* sl;
                        uint32_t* m;
                        uint32_t cmp_read_len = 0;
                        int64_t row;
                        
                        memmove( self->u.ref.name, n, n_len );
                        self->u.ref.name_len = n_len;
                        self->u.ref.start_id = out->start_id;
                        self->u.ref.stop_id = out->stop_id;
                        for ( row = out->start_id; row <= out->stop_id && cmp_read_len == 0; ++row )
                        {
                            uint32_t tmp_len = 0;
                            void const *dummy = NULL;
                            
                            rc = VCursorCellDataDirect( self->curs, row, self->u.ref.cmp_read_idx, NULL, &dummy, NULL, &tmp_len );
                            if ( rc != 0 ) break;
                            cmp_read_len += tmp_len;
                        }

                        if ( rc == 0 )
                        {
                            rc = VCursorCellDataDirect( self->curs, self->u.ref.stop_id, self->u.ref.circular_idx, NULL, (const void**)&c, NULL, NULL );
                            if ( rc == 0 )
                            {
                                rc = VCursorCellDataDirect( self->curs, self->u.ref.stop_id, self->u.ref.seq_len_idx, NULL, (const void**)&sl, NULL, NULL );
                                if ( rc == 0 )
                                {
                                    rc = VCursorCellDataDirect( self->curs, self->u.ref.stop_id, self->u.ref.max_seq_len_idx, NULL, (const void**)&m, NULL, NULL);
                                    if ( rc == 0 )
                                    {
                                        self->u.ref.circular = c[ 0 ] || cmp_read_len != 0;
                                        self->u.ref.seq_len = m[ 0 ] * (INSDC_coord_len)( self->u.ref.stop_id - self->u.ref.start_id ) + sl[0];
                                        self->u.ref.max_seq_len = m[ 0 ];
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if ( rc == 0 && ref_ploidy != 0 )
    {
        /* convert offset to normal from start of ref relative to current row */
        offset += self->u.ref.max_seq_len * (INSDC_coord_len)( ref_row_id - self->u.ref.start_id );

        if ( self->u.ref.circular )
        {
            /* make offset positive starting from refseq actual start */
            if ( offset < 0 )
                offset = self->u.ref.seq_len - ( ( -offset ) % self->u.ref.seq_len );
            else
                offset %= self->u.ref.seq_len;
        }
        else if ( offset < 0 || offset >= (INSDC_coord_zero)self->u.ref.seq_len )
            rc = RC( rcXF, rcFunction, rcSelecting, rcData, rcCorrupt );

        ref_row_id = self->u.ref.start_id + offset / self->u.ref.max_seq_len;
        offset %= self->u.ref.max_seq_len;
    }

    /* read the data */
    for ( num_read = 0; rc == 0 && num_read < ref_len && ref_row_id <= self->u.ref.stop_id; offset = 0 )
    {
        uint32_t bits;
        const void* output;
        uint32_t boff;
        uint32_t row_len;
        
        rc = VCursorCellDataDirect( self->curs, ref_row_id, self->out_idx, &bits, &output, &boff, &row_len );
        if ( rc == 0 )
        {
            /* row_len MUST be > offset */
            if ( row_len <= (uint32_t)offset )
                rc = RC(rcXF, rcFunction, rcSelecting, rcData, rcCorrupt);
            else
            {
                row_len -= offset;
                if ( ref_len < row_len + num_read )
                    row_len = ref_len - num_read;

                /* copy data */
                bitcpy( rslt->data->base, rslt->elem_count * bits, output, offset * bits + boff, row_len * bits );
                rslt->elem_count += row_len;
                num_read += row_len;

                if ( ++ref_row_id > self->u.ref.stop_id && self->u.ref.circular )
                    ref_row_id = self->u.ref.start_id;
            }
        }
    }

    /* detect incomplete read */
    if ( rc == 0 && num_read == 0 )
        rc = RC( rcXF, rcFunction, rcSelecting, rcTransfer, rcIncomplete );

    return rc;
}

static
rc_t CC ALIGN_CMN_TABLE_sub_select(RefTableSubSelect* self, int64_t ref_row_id,
                                   INSDC_coord_zero offset, INSDC_coord_len ref_len,
                                   uint32_t ref_ploidy, VRowResult* rslt)
{
	rc_t rc=0;
    INSDC_coord_len num_read = 0;
    const int64_t* al_ref_id = NULL;
    int64_t al_ref_id_value;
    const INSDC_coord_zero* al_ref_start = NULL;
    
    SUB_DEBUG( ( "SUB.Rd in 'ref-tbl-sub-select.c' (ALIGN) at #%lu offset %lu\n", ref_row_id, offset ) );

    if ( offset < 0 )
    {
        rc = VCursorCellDataDirect( self->curs, ref_row_id, self->u.mod.ref_id_idx, NULL, (void const **)&al_ref_id, NULL, NULL );
        if ( rc == 0 )
        {
            rc = VCursorCellDataDirect( self->curs, ref_row_id, self->u.mod.ref_start_idx, NULL, (void const **)&al_ref_start, NULL, NULL);
            if ( rc == 0 )
            {
                memmove ( &al_ref_id_value, al_ref_id, sizeof *al_ref_id );
                if ( -offset > (INSDC_coord_zero)ref_len )
                {
                    /* requested chunk is to the left and is not using allele data */
                    rc = RC( rcXF, rcFunction, rcSelecting, rcData, rcCorrupt );
                }
                else
                {
                    rc = self->u.mod.parent->func( self->u.mod.parent, al_ref_id_value, offset + al_ref_start[0],
                                                    -offset, ref_ploidy, rslt );
                    if ( rc == 0 )
                    {
                        /* read left portion of underlying reference */
                        num_read += (INSDC_coord_len)rslt->elem_count;
                        offset = 0;
                    }
                }
            }
        }
    }

    if ( rc == 0 && num_read < ref_len )
    {
        /* copy self */
        void const* output;
        uint32_t bits, boff, rs_len, rl_len;
        const INSDC_coord_zero* rs;
        const INSDC_coord_len* rl;
        
        rc = VCursorCellDataDirect( self->curs, ref_row_id, self->out_idx, &bits, &output, &boff, NULL );
        if ( rc == 0 )
        {
            rc = VCursorCellDataDirect( self->curs, ref_row_id, self->u.mod.read_start_idx, NULL, (void const **)&rs, NULL, &rs_len );
            if ( rc == 0 )
            {
                rc = VCursorCellDataDirect( self->curs, ref_row_id, self->u.mod.read_len_idx, NULL, (void const **)&rl, NULL, &rl_len );
                if ( rc == 0 )
                {
                    assert( rs_len == rl_len );
                    assert( ref_ploidy > 0 && ref_ploidy <= rl_len );
                    if ( offset > (INSDC_coord_zero)rl[ ref_ploidy - 1 ] )
                    {
                        /* requested chunk starts beyond allele */
                        rc = RC( rcXF, rcFunction, rcSelecting, rcData, rcCorrupt );
                    }
                    else
                    {
                        INSDC_coord_len left = ref_len - num_read;
                        if ( ( rl[ ref_ploidy - 1 ] - offset ) < left )
                        {
                            left = rl[ ref_ploidy - 1 ] - offset;
                        }
                        bitcpy( rslt->data->base, rslt->elem_count * bits, output, ( rs[ref_ploidy - 1] + offset ) * bits + boff, left * bits );
                        num_read += left;
                        rslt->elem_count += left;
                    }
                }
            }
        }
    }

    if ( rc == 0 && num_read < ref_len )
    {
        const INSDC_coord_len* al_ref_len;
        /* copy right portion of underlying reference */
        if ( al_ref_id == NULL || al_ref_start == NULL )
        {
            rc = VCursorCellDataDirect( self->curs, ref_row_id, self->u.mod.ref_id_idx, NULL, (void const **)&al_ref_id, NULL, NULL );
            if ( rc == 0 )
                rc = VCursorCellDataDirect( self->curs, ref_row_id, self->u.mod.ref_start_idx, NULL, (void const **)&al_ref_start, NULL, NULL );
        }

        if ( rc == 0 )
            rc = VCursorCellDataDirect( self->curs, ref_row_id, self->u.mod.ref_len_idx, NULL, (void const **)&al_ref_len, NULL, NULL );

        memmove ( & al_ref_id_value, al_ref_id, sizeof *al_ref_id );
        if ( rc == 0 )
            rc = self->u.mod.parent->func( self->u.mod.parent, al_ref_id_value, al_ref_start[0] + al_ref_len[0],
                                            ref_len - num_read, ref_ploidy, rslt );

    }
    return rc;
}


static
void CC RefTableSubSelect_Whack ( void *obj )
{
    if ( obj != NULL )
    {
        RefTableSubSelect* self = ( RefTableSubSelect* )obj;
        VCursorRelease( self->curs );
        if ( self->func != REFERENCE_TABLE_sub_select )
        {
            RefTableSubSelect_Whack( self->u.mod.parent );
        }
        else
        {
            free( self->u.ref.name );
        }
        free( self );
    }
}

/* normal way to do it */
#define IS_ADDED(c, i, n) ((rc = VCursorAddColumn(c, i, "%s", n)) == 0 || \
                           (GetRCObject(rc) == (enum RCObject)rcColumn && GetRCState(rc) == rcExists))

static
rc_t RefTableSubSelect_Make( RefTableSubSelect **objp, const VTable *tbl, const VCursor *native_curs, const char* out_col_name )
{
    rc_t rc;

    /* create the object */
    RefTableSubSelect* obj = ( RefTableSubSelect* )calloc( 1, sizeof( *obj ) );
    if ( obj == NULL )
    {
        rc = RC( rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted );
    }
    else
    {
        const VTable *reftbl = NULL;

        SUB_DEBUG( ( "SUB.Make in 'ref-tbl-sub-select.c' col=%s\n", out_col_name ) );

        /* open the reference table cursor*/
        rc = AlignRefTableCursor( tbl, native_curs, &obj->curs, &reftbl );
        if ( rc == 0 )
        {
            /* add columns to cursor */
            if ( IS_ADDED( obj->curs, &obj->u.ref.circular_idx, "CIRCULAR" ) )
            {
                /* normal REFERENCE table */
                if ( IS_ADDED( obj->curs, &obj->u.ref.name_idx, "(utf8)NAME" ) )
                {
                    if ( IS_ADDED( obj->curs, &obj->u.ref.name_range_idx, "NAME_RANGE" ) )
                    {
                        if ( IS_ADDED( obj->curs, &obj->u.ref.seq_len_idx, "SEQ_LEN" ) )
                        {
                            if ( IS_ADDED( obj->curs, &obj->u.ref.max_seq_len_idx, "MAX_SEQ_LEN" ) )
                            {
                                if ( IS_ADDED( obj->curs, &obj->u.ref.cmp_read_idx, "CMP_READ" ) )
                                {
                                    obj->func = REFERENCE_TABLE_sub_select;
                                    rc = 0;
                                }
                            }
                        }
                    }
                }
            }
            else if ( GetRCObject( rc ) == ( enum RCObject )rcColumn && GetRCState( rc ) == rcNotFound )
            {
                /* try as align_cmn */
                rc = RefTableSubSelect_Make( &obj->u.mod.parent, reftbl, native_curs, out_col_name );
                if ( rc == 0 )
                {
                    if ( IS_ADDED( obj->curs, &obj->u.mod.ref_id_idx, "REF_ID" ) )
                    {
                        if ( IS_ADDED( obj->curs, &obj->u.mod.ref_start_idx, "REF_START" ) )
                        {
                            if ( IS_ADDED( obj->curs, &obj->u.mod.ref_len_idx, "REF_LEN" ) )
                            {
                                if ( IS_ADDED( obj->curs, &obj->u.mod.read_start_idx, "READ_START" ) )
                                {
                                    if ( IS_ADDED( obj->curs, &obj->u.mod.read_len_idx, "READ_LEN" ) )
                                    {
                                        obj->func = ALIGN_CMN_TABLE_sub_select;
                                        rc = 0;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if ( rc == 0 )
            {
                if ( IS_ADDED( obj->curs, &obj->out_idx,  out_col_name ) )
                    rc = 0;
            }

            if ( rc == 0 )
            {
                *objp = obj;
                VTableRelease( reftbl );
                return 0;
            }

            VCursorRelease( obj->curs );
        }
        VTableRelease( reftbl );
        free( obj );
    }
    return rc;
}

static
rc_t CC reftbl_sub_select ( void *data, const VXformInfo *info,
                            int64_t row_id, VRowResult *rslt, uint32_t argc, const VRowData argv[] )
{
    rc_t rc;
    RefTableSubSelect* self = ( RefTableSubSelect* )data;
    const int64_t* ref_id = ( const int64_t* )argv[ 0 ].u.data.base;
    const INSDC_coord_zero* ref_start = ( const INSDC_coord_zero* )argv[ 1 ].u.data.base;
    const INSDC_coord_len* ref_len = ( const INSDC_coord_len* )argv[ 2 ].u.data.base;
    const uint32_t* ref_ploidy = NULL;
    
    if ( argc > 3 )
    {
        ref_ploidy = ( const uint32_t* )argv[ 3 ].u.data.base;
        ref_ploidy += argv[3].u.data.first_elem;
        assert( argv[ 3 ].u.data.elem_bits == sizeof( *ref_ploidy ) * 8 );
    }
    assert( argv[ 0 ].u.data.elem_bits == sizeof( *ref_id ) * 8 );
    assert( argv[ 1 ].u.data.elem_bits == sizeof( *ref_start ) * 8 );
    assert( argv[ 2 ].u.data.elem_bits == sizeof( *ref_len ) * 8 );

    ref_id += argv[ 0 ].u.data.first_elem;
    ref_start += argv[ 1 ].u.data.first_elem;
    ref_len += argv[ 2 ].u.data.first_elem;

    /* get the memory for output row */
    rslt->data->elem_bits = rslt->elem_bits;
    rc = KDataBufferResize( rslt->data, ref_len[ 0 ] );
    if ( rc == 0 )
    {
        /* must set it to 0 here - functions above accumulate */
        rslt->elem_count = 0;
        if ( ref_len[ 0 ] > 0 )
        {
            int64_t ref_id_val;
            memmove( &ref_id_val, ref_id, sizeof ref_id_val );
            rc = self->func( self, ref_id_val, ref_start[ 0 ], ref_len[ 0 ], ref_ploidy ? ref_ploidy[ 0 ] : 0, rslt );
        }
    }

    return rc;
}


VTRANSFACT_IMPL ( ALIGN_ref_sub_select, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    RefTableSubSelect *fself;
    rc_t rc = RefTableSubSelect_Make( &fself, info -> tbl, ( const VCursor* )info->parms, "(INSDC:4na:bin)READ" );
    if ( rc == 0 )
    {
        rslt -> self = fself;
        rslt -> whack = RefTableSubSelect_Whack;
        rslt -> u . ndf = reftbl_sub_select;
        rslt -> variant = vftRow;
    }
    return rc;
}


VTRANSFACT_IMPL ( NCBI_align_ref_sub_select_preserve_qual, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
                                                   VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    RefTableSubSelect *fself;
    rc_t rc = RefTableSubSelect_Make( & fself, info -> tbl, (const VCursor*)info->parms , "(bool)PRESERVE_QUAL" );
    if ( rc == 0 )
    {
        rslt -> self = fself;
        rslt -> whack = RefTableSubSelect_Whack;
        rslt -> u . ndf = reftbl_sub_select;
        rslt -> variant = vftRow;
    }
    return rc;
}
