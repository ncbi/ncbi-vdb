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

#include <sysalloc.h>
#include <sra/sradb.h>
#include <vdb/xform.h>
#include <klib/data-buffer.h>
#include <klib/rc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>

static
void process_position ( uint16_t *dst, const uint16_t *src, uint32_t count )
{
    uint32_t i, cur, prev;

    for ( prev = i = 0; i < count; prev = cur, ++ i )
    {
        cur = src [ i ];
        if ( cur > 0x7FFF )
            cur = ( uint8_t ) cur;
        while ( prev > cur )
            cur += 256;
        dst [ i ] = cur;
    }
}

static
rc_t CC process_position1 ( void *self, const VXformInfo *info,
    int64_t row_id, const VFixedRowResult *rslt,
    uint32_t argc, const VRowData argv [] )
{
    uint16_t *dst = rslt -> base;
    const uint16_t *src = argv [ 0 ] . u . data . base;
    uint32_t count = argv [ 0 ] . u . data . elem_count;

    dst += rslt -> first_elem;
    src += argv [ 0 ] . u . data . first_elem;

    process_position ( dst, src, count );

    return 0;
}

static
rc_t CC process_position2 ( void *self, const VXformInfo *info,
    int64_t row_id, VRowResult *rslt,
    uint32_t argc, const VRowData argv [] )
{
    /* position row data */
    const uint16_t *src = argv [ 0 ] . u . data . base;
    uint32_t count = argv [ 0 ] . u . data . elem_count;

    /* filter row data */
    const SRAReadFilter *rd_filt = argv [ 1 ] . u . data . base;
    uint32_t i, nreads = argv [ 1 ] . u . data . elem_count;

    /* output buffer */
    uint16_t *dst;
    rc_t rc = KDataBufferCast ( rslt -> data, rslt -> data, 16, true );
    if ( rc == 0 )
        rc = KDataBufferResize ( rslt -> data, count );
    if ( rc != 0 )
        return rc;

    /* adjust all pointers to start of row */
    src += argv [ 0 ] . u . data . first_elem;
    rd_filt += argv [ 1 ] . u . data . first_elem;
    dst = rslt -> data -> base;

    /* set output size */
    rslt -> elem_count = count;
    rslt -> elem_bits = 16;

    /* walk input */
    for ( i = 0; i < nreads; ++ i )
    {
        if ( rd_filt [ i ] == SRA_READ_FILTER_REDACTED )
        {
            memset ( dst, 0, count * sizeof * dst );
            return 0;
        }
    }

    process_position ( dst, src, count );

    return 0;
}

static
rc_t CC process_position3 ( void *self, const VXformInfo *info,
    int64_t row_id, VRowResult *rslt,
    uint32_t argc, const VRowData argv [] )
{
    /* position row data */
    const uint16_t *src = argv [ 0 ] . u . data . base;
    uint32_t count = argv [ 0 ] . u . data . elem_count;

    /* filter row data */
    const SRAReadFilter *rd_filt = argv [ 1 ] . u . data . base;
    uint32_t i, nreads = argv [ 1 ] . u . data . elem_count;

    /* signal row data */
    const uint16_t *sig = argv [ 2 ] . u . data . base;

    /* output buffer */
    uint16_t *dst;
    rc_t rc = KDataBufferCast ( rslt -> data, rslt -> data, 16, true );
    if ( rc == 0 )
        rc = KDataBufferResize ( rslt -> data, count );
    if ( rc != 0 )
        return rc;

    /* adjust all pointers to start of row */
    src += argv [ 0 ] . u . data . first_elem;
    rd_filt += argv [ 1 ] . u . data . first_elem;
    sig += argv [ 2 ] . u . data . first_elem;
    dst = rslt -> data -> base;

    /* set output size */
    rslt -> elem_count = count;
    rslt -> elem_bits = 16;

    /* walk input */
    for ( i = 0; i < nreads; ++ i )
    {
        if ( rd_filt [ i ] == SRA_READ_FILTER_REDACTED )
        {
            uint32_t scount = argv [ 2 ] . u . data . elem_count;
            for ( i = 0; i < scount; ++ i )
            {
                if ( sig [ i ] != 0 )
                    break;
            }
            if ( i == scount )
            {
                memset ( dst, 0, count * sizeof * dst );
                return 0;
            }
            break;
        }
    }

    process_position ( dst, src, count );

    return 0;
}


/* process_position
 *  convert 8 bit integration into 16 bit
 */
VTRANSFACT_IMPL ( NCBI_SRA__454__process_position, 1, 0, 0 ) ( const void *fself, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    if ( dp -> argc == 3 )
    {
        rslt -> u . rf = process_position3;
        rslt -> variant = vftRow;
        return 0;
    }

    if ( dp -> argc == 2 )
    {
        rslt -> u . rf = process_position2;
        rslt -> variant = vftRow;
        return 0;
    }

    rslt -> u . pf = process_position1;
    rslt -> variant = vftFixedRow;
    return 0;
}
