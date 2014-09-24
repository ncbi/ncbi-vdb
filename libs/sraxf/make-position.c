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

#include <stdlib.h>
#include <assert.h>

/* make_position
 *  return a synthesized position row with 1-1 correspondence
 *
 *  "T" [ TYPE ] - position type being generated
 *
 *  "start" [ CONST ] - either 0 or 1, depending upon the coordinate system
 *
 *  "read" [ DATA ] - the actual row of bases. the output row
 *  will be the same length, but with synthesized data
 */
static
rc_t CC make_position ( void *self, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt,  uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    KDataBuffer *dst = self;

    if ( argv [ 0 ] . u . data . elem_count > dst -> elem_count )
    {
        uint32_t old = dst -> elem_count;
        rc = KDataBufferResize ( dst, argv [ 0 ] . u . data . elem_count );
        if ( rc != 0 )
            return rc;
        if ( dst -> elem_bits == 16 )
        {
            uint16_t i, *p = dst -> base;
            uint16_t offset = p [ 0 ];
            for ( i = old; i < dst -> elem_count; ++ i )
                p [ i ] = i + offset;
        }
        else
        {
            int32_t i, *p = dst -> base;
            int32_t offset = p [ 0 ];
            for ( i = old; i < (int32_t)dst -> elem_count; ++ i )
                p [ i ] = i + offset;
        }
    }

    KDataBufferWhack ( rslt -> data );
    rc = KDataBufferSub ( dst, rslt -> data, 0, argv [ 0 ] . u . data . elem_count );
    if ( rc == 0 )
        rslt -> elem_count = argv [ 0 ] . u . data . elem_count;

    return rc;
}

static
void CC whack_data_buffer ( void *data )
{
    KDataBufferWhack ( data );
    free ( data );
}

VTRANSFACT_IMPL( NCBI_SRA_make_position, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rc_t rc;
    KDataBuffer *b = malloc ( sizeof * b );
    if ( b == NULL )
        return RC ( rcSRA, rcFunction, rcConstructing, rcMemory, rcExhausted );

    rc = KDataBufferMake ( b, info -> fdesc . desc . intrinsic_bits, 4096 );
    if ( rc != 0 )
    {
        free ( b );
        return rc;
    }

    if ( info -> fdesc . desc . intrinsic_bits == 16 )
    {
        uint16_t i, *dst = b -> base;
        uint16_t offset = cp -> argv [ 0 ] . data . u16 [ 0 ];
        for ( i = 0; i < 4096; ++ i )
            dst [ i ] = i + offset;
    }
    else if ( info -> fdesc . desc . intrinsic_bits == 32 )
    {
        int32_t i, *dst = b -> base;
        int32_t offset = cp -> argv [ 0 ] . data . i32 [ 0 ];
        for ( i = 0; i < 4096; ++ i )
            dst [ i ] = i + offset;
    }
    else
    {
        /* this shouldn't really be needed */
        whack_data_buffer ( b );
        return RC ( rcSRA, rcFunction, rcConstructing, rcType, rcUnsupported );
    }

    rslt -> self = b;
    rslt -> whack = whack_data_buffer;
    rslt -> u . rf = make_position;
    rslt -> variant = vftRow;

    return 0;
}
