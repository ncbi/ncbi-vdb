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
#include <vdb/schema.h>


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
    uint32_t read_idx;
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
rc_t RestoreReadMake ( RestoreRead **objp, const VTable *tbl )
{
    rc_t rc;

    /* create the object */
    RestoreRead *obj = malloc ( sizeof * obj );
    if ( obj == NULL )
    {
        rc = RC ( rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted );
    }
    else
    {
        /* get at the parent database */
        const VDatabase *db;
        rc = VTableOpenParentRead ( tbl, & db );
        if ( rc == 0 )
        {
            /* open the primary alignment table */
            const VTable *patbl;
            rc = VDatabaseOpenTableRead ( db, & patbl, "PRIMARY_ALIGNMENT" );
            VDatabaseRelease ( db );
            if ( rc == 0 )
            {
                /* create a cursor */
                rc = VTableCreateCachedCursorRead( patbl, &obj->curs, 32*1024*1024UL );
                VTableRelease ( patbl );
                if ( rc == 0 )
                {
                    /* add columns to cursor */
                    rc = VCursorAddColumn ( obj -> curs, & obj -> read_idx, "( INSDC:4na:bin ) READ" );
                    if ( rc == 0 )
                    {
                        rc = VCursorOpen ( obj -> curs );
                        if ( rc == 0 )
                        {

                            SUB_DEBUG( ( "SUB.Make in 'seq-restore-read.c'\n" ) );

                            * objp = obj;
                        return 0;
                        }
                    }
                    VCursorRelease ( obj -> curs );
                }
            }
        }
        free ( obj );
    }

    return rc;
}

static INSDC_4na_bin  map[]={
/*0  0000 - 0000*/ 0,
/*1  0001 - 1000*/ 8,
/*2  0010 - 0100*/ 4,
/*3  0011 - 1100*/ 12,
/*4  0100 - 0010*/ 2,
/*5  0101 - 1010*/ 10,
/*  0110 - 0110*/ 6,
/*7  0111 - 1110*/ 14,
/*8  1000 - 0001*/ 1,
/*9  1001 - 1001*/ 9,
/*10  1010 - 0101*/ 5,
/*11  1011 - 1101*/ 13,
/*12  1100 - 0011*/ 3,
/*13  1101 - 1011*/ 11,
/*14  1110 - 0111*/ 7,
/*15  1111 - 1111*/ 15
};

static
rc_t CC seq_restore_read_impl ( void *data, const VXformInfo *info, int64_t row_id,
                                VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    int i;
    RestoreRead		*self = data;
    INSDC_4na_bin	*dst;
    INSDC_coord_len 	len;
    uint32_t		src_len		= (uint32_t)argv[ 0 ].u.data.elem_count;
    const INSDC_4na_bin	*src	= argv[ 0 ].u.data.base;
    const uint32_t 	num_reads	= (uint32_t)argv[ 1 ].u.data.elem_count;
    const int64_t	*align_id	= argv[ 1 ].u.data.base;
    const INSDC_coord_len *read_len = argv[ 2 ].u.data.base;
    const uint8_t	*read_type	= argv[ 3 ].u.data.base;
    
    assert( argv[ 0 ].u.data.elem_bits == 8 );
    assert( argv[ 1 ].u.data.elem_bits == 64 );
    assert( argv[ 2 ].u.data.elem_bits == sizeof( INSDC_coord_len ) * 8 );
    assert( argv[ 2 ].u.data.elem_count == num_reads );
    assert( argv[ 3 ].u.data.elem_count == num_reads );
    
    
    src   += argv [ 0 ] . u . data . first_elem;
    align_id  += argv [ 1 ] . u . data . first_elem;
    read_len  += argv [ 2 ] . u . data . first_elem;
    read_type += argv [ 3 ] . u . data . first_elem;
    
    for ( i = 0, len = 0; i < (int)num_reads; i++ )
    {
        len += read_len[ i ];
    }

    /* resize output row */    
    rslt->data->elem_bits = 8;
    rc = KDataBufferResize( rslt->data, len );
    rslt->elem_count = len;
    dst = rslt->data->base;
    if ( rc == 0 && len > 0 )
    {
        if ( len == src_len ) /*** shortcut - all data is local ***/
        {
            memcpy( dst, src, len );
        }
        else for( i = 0; i < (int)num_reads && rc == 0; i++ ) /*** checking read by read ***/
        {
            if ( align_id[ i ] > 0 )
            {
                const INSDC_4na_bin *r_src;
                uint32_t             r_src_len;

                SUB_DEBUG( ( "SUB.Rd in 'seq-restore-read.c' at #%lu\n", align_id[ i ] ) );

                rc = VCursorCellDataDirect( self -> curs, align_id[ i ], self -> read_idx,
                                            NULL, ( const void** ) &r_src, NULL, &r_src_len );
                if ( rc == 0 )
                {
                    if ( r_src_len == read_len[ i ] )
                    {
                        if ( read_type[ i ] & SRA_READ_TYPE_FORWARD )
                        {
                            memcpy( dst, r_src, read_len[ i ] );
                        }
                        else if ( read_type[ i ] & SRA_READ_TYPE_REVERSE )
                        {
                            int j, k;
                            for( j = 0, k = read_len[ i ] - 1; j < (int)read_len[ i ]; j++, k-- )
                            {
                                dst[ j ] = map [ r_src[ k ] & 15 ];
                            }
                        }
                        else
                        {
                            rc = RC( rcXF, rcFunction, rcExecuting, rcData, rcInconsistent );
                        }
                    }
                    else
                    {
                        rc = RC( rcXF, rcFunction, rcExecuting, rcData, rcInconsistent );
                    }
                }
            }
            else /*** data is in READ column **/
            {
                if ( src_len >= read_len[ i ] )
                {
                    memcpy( dst, src, read_len[ i ] );
                    src_len -= read_len[ i ];
                    src     += read_len[ i ];
                }
                else
                {
                    return RC( rcXF, rcFunction, rcExecuting, rcData, rcInconsistent );
                }
            }
            dst += read_len[ i ];
        }
    }

    return rc;
}

/* 
 * function
 * INSDC:4na:bin ALIGN:seq_restore_read #1 (  INSDC:4na:bin rd , I64 align_id , INSDC:coord:len read_len);
 */
VTRANSFACT_IMPL ( ALIGN_seq_restore_read, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
                                                     VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    RestoreRead *fself;
    rc_t rc = RestoreReadMake ( & fself, info -> tbl );
    if ( rc == 0 )
    {
        rslt->self = fself;
        rslt->u.ndf = seq_restore_read_impl;
        rslt->variant = vftRow;
        rslt -> whack = RestoreReadWhack;
    }
    return rc;
}
