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

#ifdef _DEBUGGING
#define SUB_DEBUG(msg) DBGMSG(DBG_SRA,DBG_FLAG(DBG_SRA_SUB),msg)
#else
#define SUB_DEBUG(msg)
#endif

typedef struct RefPos RefPos;

struct RefPos
{
    const VCursor *curs;
    uint32_t name_idx;
    uint32_t name_range_idx;
    uint32_t max_seq_len;
/***** cache ****/
    BSTree   tr_range; /** region tree ***/
};

typedef struct {
    int64_t start_id;
    int64_t stop_id;
}RowRange;

typedef struct {
    BSTNode  n;
    RowRange rr;
}BSTRowRange;


static
int64_t CC row_range_cmp ( const void *a, const BSTNode *b )
{
	const int64_t  *key=a;
	const BSTRowRange *node = ( const BSTRowRange* ) b;
	if ( key[ 0 ] < node->rr.start_id ) return -1;
	if ( key[ 0 ] > node->rr.stop_id ) return +1;
	return 0;
}

static
int64_t CC row_range_sort( const BSTNode *a, const BSTNode *b )
{
    const BSTRowRange * item = ( const BSTRowRange* ) a;
    const BSTRowRange * node = ( const BSTRowRange* ) b;
    return item->rr.start_id < node->rr.start_id ?
        -1 : item->rr.start_id > node->rr.start_id;
}    


static
void CC bst_range_free ( BSTNode *n, void *ignore )
{
    free( n );
}


static
void CC RefPosWhack ( void *obj )
{
    RefPos * self = obj;
    if ( self != NULL )
    {
    	BSTreeWhack( &self->tr_range, bst_range_free, NULL );
        VCursorRelease ( self -> curs );
        free ( self );
    }
}

static
rc_t RefPosMake ( RefPos **objp, const VTable *tbl, const VCursor *native_curs )
{
    rc_t rc;

    /* create the object */
    RefPos *obj = malloc ( sizeof * obj );
    if ( obj == NULL )
    {
        rc = RC( rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted );
    }
    else
    {
        obj->curs=NULL;
        BSTreeInit( &obj->tr_range );
        /* open the reference table cursor*/

        SUB_DEBUG( ( "SUB.Make in 'align-ref-pos.c'\n" ) );
	  
        rc = AlignRefTableCursor( tbl, native_curs, &obj->curs, NULL );
        if ( rc == 0 )
        {
            uint32_t itmp;
            rc = VCursorAddColumn( obj->curs, &itmp, "(U32)MAX_SEQ_LEN" );
            if ( ( rc == 0 ) || GetRCState( rc ) == rcExists )
            {
                const void * base;
                uint32_t row_len;
                rc = VCursorCellDataDirect( obj->curs, 1, itmp, NULL, &base, NULL, &row_len );
                if ( rc == 0 )
                {
                    assert( row_len == 1 );
                    memmove( &obj->max_seq_len, base, 4 );
                }
            }

            if ( GetRCObject( rc ) == ( enum RCObject )rcColumn && GetRCState( rc ) == rcNotFound )
            {
                /*** no MAX_SEQ_LEN means that REF_POS==REF_START **/
                VCursorRelease( obj->curs );
                obj->curs = NULL;
                obj->max_seq_len = 0;
                obj->name_range_idx = 0;
                obj->name_idx = 0;
                rc = 0;
            }
            else if ( rc == 0 )
            {
                /* add columns to cursor */
                rc = VCursorAddColumn( obj->curs, &obj->name_idx, "(utf8)NAME" );
                if ( rc == 0 || GetRCState( rc ) == rcExists )
                {
                    rc = VCursorAddColumn( obj->curs, &obj->name_range_idx, "NAME_RANGE" );
                }
                if ( GetRCState( rc ) == rcExists )
                {
                    rc = 0;
                }
            }
        }

        if ( rc == 0 )
        {
            *objp = obj;
        }
        else
        {
            VCursorRelease( obj->curs );
            free( obj );
        }
    }

    return rc;
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
static
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

    ref_start = argv[ REF_START ].u.data.base;
    ref_start += argv[ REF_START ].u.data.first_elem;

    if ( self->curs != NULL )
    {
        char const *name = NULL;
        uint32_t name_len;
        BSTRowRange *brr;

        ref_id = argv[ REF_ID ].u.data.base;
        ref_id += argv[ REF_ID ].u.data.first_elem;

        brr = ( BSTRowRange * )BSTreeFind( &self->tr_range, &ref_id[ 0 ], row_range_cmp );
        if ( brr == NULL )
        {
            RowRange *new_rr;

            SUB_DEBUG( ( "SUB.Rd in 'align-ref-pos.c' at #%lu\n", ref_id[ 0 ] ) );

            rc = VCursorCellDataDirect( self->curs, ref_id[ 0 ], self->name_idx, NULL, (void const **)&name, NULL, &name_len );
            if ( rc != 0 )
                return rc;

            rc = VCursorParamsSet( ( struct VCursorParams const * )self->curs, "QUERY_SEQ_NAME", "%.*s", name_len, name );
            if ( rc != 0 )
                return rc;

            rc = VCursorCellDataDirect( self->curs, ref_id[ 0 ], self->name_range_idx, NULL, (void const **)&new_rr, NULL, NULL );
            if ( rc != 0 )
                return rc;

            brr = malloc( sizeof( *brr ) );
            if ( brr == NULL )
            {
                return RC( rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted );
            }
            else
            {
                memmove( &brr->rr, new_rr, sizeof( *new_rr ) );
                BSTreeInsert( ( BSTree* )&self->tr_range, ( BSTNode* )brr, row_range_sort );
            }
        }
        ref_row_id = brr->rr.start_id;
    }

    rc = KDataBufferResize( rslt->data, ploidy );
    if ( rc != 0 )
        return rc;
    
    ref_pos = rslt->data->base;
    for ( i = 0; i != ploidy; ++i )
    {
        ref_pos[ i ] = ref_start[ i ];
        if ( self->curs != NULL )
        {
            ref_pos[ i ] += ( INSDC_coord_zero )( ( ref_id[ 0 ] - ref_row_id ) * self->max_seq_len );
        }
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
        rslt -> whack = RefPosWhack;
        rslt -> u . rf = align_ref_pos;
        rslt -> variant = vftRowFast;
    }

    return rc;
}
