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
#include <insdc/insdc.h>
#include <klib/defs.h>
#include <klib/rc.h>
#include <vdb/table.h>
#include <vdb/xform.h>
#include <vdb/schema.h>
#include <kdb/meta.h>
#include <klib/data-buffer.h>
#include <bitstr.h>
#include <sysalloc.h>

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>


static INSDC_4na_bin  map[]={
/*0  0000 - 0000*/ 0,
/*1  0001 - 1000*/ 8,
/*2  0010 - 0100*/ 4,
/*3  0011 - 1100*/ 12,
/*4  0100 - 0010*/ 2,
/*5  0101 - 1010*/ 10,
/*6  0110 - 0110*/ 6,
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
rc_t CC raw_restore_read_impl ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    int i, j;
    const INSDC_4na_bin	*read 	= argv[ 0 ].u.data.base;
    const uint32_t	read_len 	= (uint32_t)argv[ 0 ].u.data.elem_count;
    const uint8_t	*strand		= argv[ 1 ].u.data.base;
    const uint32_t	strand_len 	= (uint32_t)argv[ 1 ].u.data.elem_count;
    
    INSDC_4na_bin *dst;
    
    assert( argv[ 0 ].u.data.elem_bits == 8 );
    assert( argv[ 1 ].u.data.elem_bits == 8 );
    assert( strand_len == 1 );
    
    read   += argv[ 0 ].u.data.first_elem;
    strand += argv[ 1 ].u.data.first_elem;
    
    /* resize output row for the total number of reads */    
    rslt -> data -> elem_bits = 8;
    rc = KDataBufferResize ( rslt -> data, read_len );
    if ( rc != 0 )
        return rc;
    rslt -> elem_count = read_len;
    dst = rslt -> data -> base;

    /**** MAIN RESTORATION LOOP ***/
    if ( strand[ 0 ] == false ) /*** nothing to do **/
    {
        memmove( dst, read, read_len );
    }
    else for ( i = 0, j = read_len - 1; i < (int)read_len; i++, j-- )
    {
        dst[ i ] = map[ read[ j ]&15 ];
    }
    return 0;
}


/* 
 * function
 * INSDC:4na:bin ALIGN:raw_restore_read #1( INSDC:4na:bin ref_read, bool ref_orientation);
 */
VTRANSFACT_IMPL ( ALIGN_raw_restore_read, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{

    rslt->u.rf = raw_restore_read_impl;
    rslt->variant = vftRow;
    rslt -> whack = NULL;
    return 0;
}
