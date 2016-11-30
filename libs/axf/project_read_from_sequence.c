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

#include <klib/defs.h>
#include <klib/rc.h>
#include <klib/debug.h>
#include <kdb/meta.h>
#include <klib/data-buffer.h>
#include <insdc/insdc.h>
#include <align/refseq-mgr.h>
#include <bitstr.h>
#include <sysalloc.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <insdc/sra.h>

#ifdef _DEBUGGING
#define SUB_DEBUG(msg) DBGMSG(DBG_SRA,DBG_FLAG(DBG_SRA_SUB),msg)
#else
#define SUB_DEBUG(msg)
#endif

typedef struct RestoreRead RestoreRead;
struct RestoreRead
{
    const VCursor *curs;
    uint32_t col_idx;
    uint32_t read_len_idx;
    uint32_t read_start_idx;
};

static
void CC RestoreReadWhack ( void *obj )
{
    RestoreRead * self = obj;
    if ( self != NULL )
    {
        VCursorRelease ( self -> curs );
        free ( self );
    }
}

static
rc_t RestoreReadMake ( RestoreRead **objp, const VXfactInfo *info, const VFactoryParams *cp,
                       const VCursor *native_curs )
{
    rc_t rc;
    RestoreRead * obj;

    SUB_DEBUG( ( "SUB.Make in 'project_read_from_sequence.c'\n" ) );

    /* create the object */
    obj = malloc ( sizeof ( * obj ) );
    if ( obj == NULL )
    {
        rc = RC ( rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted );
    }
    else
    {
        rc = VCursorLinkedCursorGet( native_curs, "SEQUENCE", &obj->curs );
        if ( rc != 0 )
        {
            const VDatabase * db;
            const VTable * tbl;
            uint64_t cache_size = 32*1024*1024;
            uint64_t native_cursor_cache_size = VCursorGetCacheCapacity(native_curs);

            /* get at the parent database */
            rc = VTableOpenParentRead ( info -> tbl, & db );
            if ( rc != 0 )
                return rc;

            /* open the table */
            rc = VDatabaseOpenTableRead ( db, &tbl, "SEQUENCE" );
            VDatabaseRelease ( db );
            if ( rc != 0 )
                return rc;

            if(native_cursor_cache_size/4 > cache_size){
                /* share cursor size with native cursor **/
                cache_size = native_cursor_cache_size/4;
                native_cursor_cache_size -= cache_size;
                VCursorSetCacheCapacity((VCursor*)native_curs,native_cursor_cache_size);
            }
            /* create a cursor */
            rc = VTableCreateCachedCursorRead( tbl, &obj->curs, cache_size );
            VTableRelease( tbl );
            if ( rc != 0 )
                return rc;

            rc = VCursorPermitPostOpenAdd( obj->curs );
            if ( rc != 0 )
                return rc;
            rc = VCursorOpen( obj->curs );
            if ( rc != 0 )
                return rc;
            rc = VCursorLinkedCursorSet( native_curs, "SEQUENCE", obj->curs );
            if ( rc != 0 )
                return rc;
        }

        if ( rc == 0 )
        {
            /* add columns to cursor */
            assert ( cp -> argc == 1 );
            rc = VCursorAddColumn ( obj -> curs, & obj -> col_idx, "%.*s",
                                    cp -> argv [ 0 ] . count, cp -> argv [ 0 ] . data . ascii );
            if ( rc == 0 || GetRCState( rc ) == rcExists )
                rc = VCursorAddColumn ( obj -> curs, & obj -> read_len_idx, "(INSDC:coord:len)READ_LEN" );

            if ( rc == 0 || GetRCState( rc ) == rcExists )
                rc = VCursorAddColumn ( obj -> curs, & obj -> read_start_idx, "(INSDC:coord:zero)READ_START" );

            if ( rc == 0  || GetRCState( rc ) == rcExists)
            {
                VTypedesc src;
                rc = VCursorDatatype ( obj -> curs, obj -> col_idx, NULL, & src );
                if ( rc == 0 )
                {
                    /* selected column should have same characteristics */
                    if ( src . domain != info -> fdesc . desc . domain                 ||
                         src . intrinsic_bits != info -> fdesc . desc . intrinsic_bits ||
                         src . intrinsic_dim != info -> fdesc . desc. intrinsic_dim )
                    {
                        rc = RC ( rcXF, rcFunction, rcConstructing, rcType, rcInconsistent );
                    }
                    else if ( ( src . intrinsic_bits & 7 ) != 0 )
                    {
                        rc = RC ( rcXF, rcFunction, rcConstructing, rcType, rcUnsupported );
                    }
                    else
                    {
                        * objp = obj;
                        return 0;
                    }
                }
            }
        }
        free ( obj );
    }
    return rc;
}


static
rc_t CC project_from_sequence_impl ( void *data, const VXformInfo *info,
    int64_t row_id, VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    RestoreRead *self =  data;

    rc_t rc;
    INSDC_coord_zero read_id;
    const int64_t *spot_id = argv [ 0 ] . u . data . base;
    const INSDC_coord_one *read_id_in = argv [ 1 ] . u . data . base;
    const INSDC_coord_one *read_start;
    const INSDC_coord_len *read_len;
    const void *src;
    uint32_t src_sz;
    uint32_t src_bits;
    uint32_t nreads;
    uint32_t nreads_2;
    
    spot_id += argv [ 0 ] . u . data . first_elem;
    read_id_in += argv [ 1 ] . u . data . first_elem;

    assert( argv[ 0 ].u.data.elem_bits == 64 );
    assert( argv[ 0 ].u.data.elem_count == 1 );

    assert( argv[ 1 ].u.data.elem_bits == 32 );
    assert( argv[ 1 ].u.data.elem_count == 1 );

    if ( spot_id[ 0 ] == 0 ) /*** valid case , the projection should be empty ***/
    {
        rslt->elem_count = 0;
        return 0;
    }
    assert( read_id_in[ 0 ] > 0 );
    read_id = read_id_in[ 0 ] - 1; /** make zero - based **/

    SUB_DEBUG( ( "SUB.Rd in 'project_read_from_sequence.c' at #%lu\n", spot_id[ 0 ] ) );

    rc = VCursorCellDataDirect( self->curs, spot_id[ 0 ], self->read_len_idx,
                                NULL, ( void const ** )&read_len, NULL, &nreads );
    if ( rc != 0 )
        return rc;
    
    rc = VCursorCellDataDirect( self->curs, spot_id[ 0 ], self->read_start_idx,
                                NULL, ( void const ** )&read_start, NULL, &nreads_2 );
    if ( rc != 0 )
        return rc;
    
    if ( nreads != nreads_2 || read_id >= (INSDC_coord_zero)nreads )
    {
        return RC( rcXF, rcFunction, rcExecuting, rcData, rcInvalid );
    }
    
    rc = VCursorCellDataDirect( self->curs, spot_id[ 0 ], self->col_idx,
                                &src_bits, &src, NULL, &src_sz );
    if ( rc != 0 )
        return rc;
    
    if ( src_sz == nreads )
    {
        rslt->elem_count = 1;
        rslt->data->elem_bits = src_bits;
        rc = KDataBufferResize( rslt->data, 1 );
        if ( rc == 0 )
        {
            memmove( rslt->data->base,
                    &( ( char const * )src )[ ( read_id * src_bits ) >> 3 ],
                    src_bits >> 3 );
        }
    }
    else if ( src_sz == read_start[ nreads - 1 ] + read_len[ nreads - 1 ] )
    {
        /* like READ or QUALITY */
        rslt->elem_count = read_len[ read_id ];
        rslt->data->elem_bits = src_bits;
        rc = KDataBufferResize( rslt->data, rslt->elem_count );
        if ( rc == 0 )
        {
            memmove( rslt->data->base,
                    &( ( char const * )src )[ ( read_start[ read_id ] * src_bits ) >> 3 ],
                    ( size_t )( ( src_bits * rslt->elem_count ) >> 3 ) );
        }
    }
    else
    {
        /* don't know how to break up the read or should use simple_sub_select */
        return RC( rcXF, rcFunction, rcExecuting, rcConstraint, rcViolated );
    }
    return 0;
}

/* 
 * function
 * INSDC:4na:bin NCBI:align:project_from_sequence #1 < ascii col > ( I64 seq_spot_id, INSDC:coord:one seq_read_id )
 *     = ALIGN:project_from_sequence;
 */
VTRANSFACT_IMPL ( ALIGN_project_from_sequence, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    RestoreRead *fself;
    rc_t rc = RestoreReadMake ( & fself, info, cp, (const VCursor*)info->parms  );
    if ( rc == 0 )
    {
        rslt->self = fself;
        rslt->u.ndf = project_from_sequence_impl;
        rslt->variant = vftRow;
        rslt -> whack = RestoreReadWhack;
    }
    return rc;
}
