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
#include <vdb/schema.h>
#include <klib/data-buffer.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>


/* strtonum
 *  convert string to number
 *
 *  "radix" [ CONST, DEFAULT 10 ]
 *   if not specified, or if given as 0, the default will be 10
 *   unless the string begins with "0x" or "0X", in which case radix will be 16
 *   octal is NOT inferred ( i.e. leading "0" does not imply octal )
 *
 *  "str" [ DATA ] - text to be converted
 */

static
rc_t CC strtoint ( void *data, const VXformInfo *info,
    VRowResult *rslt, const char *str, size_t str_size )
{
    bool negate;
    size_t i = 0;
    uint64_t u, max_magnitude;
    uint32_t radix;
    rc_t rc;
    KDataBuffer *dst;

    if (str_size == 0) {
        rslt -> elem_count = 0;
        return 0;
    }

    /* determine radix */
    radix = ( uint32_t ) ( ( size_t ) data );
    if ( radix == 0 )
    {
        radix = 10;
        if ( str_size > 2 && str [ 0 ] == '0' ) switch ( tolower ( str [ 1 ] ) )
        {
        case 'x':
            radix = 16;
            i = 2;
            break;
        case 'b':
            radix = 2;
            i = 2;
            break;
        }
    }

    /* check for negative */
    negate = false;
    if ( radix == 10 && i < str_size && str [ i ] == '-' )
    {
        negate = true;
        ++ i;
    }

    /* convert numeral */
    for ( u = 0; i < str_size; ++ i )
    {
        int32_t d = toupper ( str [ i ] ) - '0';
        if ( d < 0 )
            return RC ( rcXF, rcFunction, rcExecuting, rcParam, rcInvalid );
        if ( d >= 10 )
        {
            d -= 'A' - '0' - 10;
            if ( d < 10 )
                return RC ( rcXF, rcFunction, rcExecuting, rcParam, rcInvalid );
        }
        if ( d > radix )
            return RC ( rcXF, rcFunction, rcExecuting, rcParam, rcInvalid );
        u = u * radix + d;
    }

    /* determine maximum magnitude */
    if ( info -> fdesc . desc . intrinsic_bits == 64 )
        max_magnitude = ( int64_t ) -1;
    else
        max_magnitude = ( ( ( uint64_t ) 1 ) << ( info -> fdesc . desc . intrinsic_bits ) ) - 1;
    if ( info -> fdesc . desc . domain == vtdInt || negate )
    {
        max_magnitude >>= 1;
        max_magnitude += negate;
    }

    if ( u > max_magnitude )
        return RC ( rcXF, rcFunction, rcExecuting, rcParam, rcExcessive );

    /* set output buffer size */
    dst = rslt -> data;
    rc = KDataBufferCast ( dst, dst, info -> fdesc . desc . intrinsic_bits, true );
    if ( rc == 0 )
        rc = KDataBufferResize ( dst, 1 );
    if ( rc == 0 )
    {
        if ( negate )
            u = - ( int64_t ) u;

        rslt -> elem_count = 1;

        switch ( info -> fdesc . desc . intrinsic_bits )
        {
        case 8:
            * ( uint8_t* ) dst -> base = ( uint8_t ) u;
            break;
        case 16:
            * ( uint16_t* ) dst -> base = ( uint16_t ) u;
            break;
        case 32:
            * ( uint32_t* ) dst -> base = ( uint32_t ) u;
            break;
        case 64:
            * ( uint64_t* ) dst -> base = u;
            break;
        default:
            rc = RC ( rcXF, rcFunction, rcExecuting, rcParam, rcInvalid );
        }
    }

    return rc;
}

static
rc_t CC strtoflt ( void *data, const VXformInfo *info,
    VRowResult *rslt, char *str, size_t str_size )
{
    /* set output buffer size */
    rc_t rc;
    KDataBuffer *dst = rslt -> data;

    double d;
    char *end;

    str [ str_size ] = 0;
    d = strtod ( str, & end );
    if ( end != & str [ str_size ] )
        return RC ( rcXF, rcFunction, rcExecuting, rcParam, rcInvalid );

    rc = KDataBufferCast ( dst, dst, info -> fdesc . desc . intrinsic_bits, true );
    if ( rc == 0 )
        rc = KDataBufferResize ( dst, 1 );
    if ( rc == 0 )
    {
        rslt -> elem_count = 1;

        switch ( info -> fdesc . desc . intrinsic_bits )
        {
        case 32:
            * ( float* ) dst -> base = ( float ) d;
            break;
        case 64:
            * ( double* ) dst -> base = d;
            break;
        default:
            rc = RC ( rcXF, rcFunction, rcExecuting, rcParam, rcInvalid );
        }
    }

    return rc;
}

static
rc_t CC strtoint_8 ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    const char *str = argv [ 0 ] . u . data . base;
    str += argv [ 0 ] . u . data . first_elem;
    return strtoint ( data, info, rslt, str, argv [ 0 ] . u . data . elem_count );
}

static
rc_t CC strtoflt_8 ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    char buffer [ 128 ];
    uint32_t count = argv [ 0 ] . u . data . elem_count;
    const char *str = argv [ 0 ] . u . data . base;
    str += argv [ 0 ] . u . data . first_elem;

    if ( count >= sizeof buffer )
        return RC ( rcXF, rcFunction, rcExecuting, rcParam, rcExcessive );

    memcpy ( buffer, str, count );

    return strtoflt ( data, info, rslt, buffer, count );
}

static
rc_t CC strtoint_16 ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    char buffer [ 64 ];
    uint32_t i, count = argv [ 0 ] . u . data . elem_count;
    const uint16_t *str = argv [ 0 ] . u . data . base;
    str += argv [ 0 ] . u . data . first_elem;

    if ( count > sizeof buffer )
        return RC ( rcXF, rcFunction, rcExecuting, rcParam, rcExcessive );

    for ( i = 0; i < count; ++ i )
    {
        if ( str [ i ] > 128 )
            return RC ( rcXF, rcFunction, rcExecuting, rcParam, rcInvalid );
        buffer [ i ] = ( char ) str [ i ];
    }

    return strtoint ( data, info, rslt, buffer, count );
}

static
rc_t CC strtoflt_16 ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    char buffer [ 128 ];
    uint32_t i, count = argv [ 0 ] . u . data . elem_count;
    const uint16_t *str = argv [ 0 ] . u . data . base;
    str += argv [ 0 ] . u . data . first_elem;

    if ( count >= sizeof buffer )
        return RC ( rcXF, rcFunction, rcExecuting, rcParam, rcExcessive );

    for ( i = 0; i < count; ++ i )
    {
        if ( str [ i ] > 128 )
            return RC ( rcXF, rcFunction, rcExecuting, rcParam, rcInvalid );
        buffer [ i ] = ( char ) str [ i ];
    }

    return strtoflt ( data, info, rslt, buffer, count );
}

static
rc_t CC strtoint_32 ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    char buffer [ 64 ];
    uint32_t i, count = argv [ 0 ] . u . data . elem_count;
    const uint32_t *str = argv [ 0 ] . u . data . base;
    str += argv [ 0 ] . u . data . first_elem;

    if ( count > sizeof buffer )
        return RC ( rcXF, rcFunction, rcExecuting, rcParam, rcExcessive );

    for ( i = 0; i < count; ++ i )
    {
        if ( str [ i ] > 128 )
            return RC ( rcXF, rcFunction, rcExecuting, rcParam, rcInvalid );
        buffer [ i ] = ( char ) str [ i ];
    }

    return strtoint ( data, info, rslt, buffer, count );
}

static
rc_t CC strtoflt_32 ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    char buffer [ 128 ];
    uint32_t i, count = argv [ 0 ] . u . data . elem_count;
    const uint32_t *str = argv [ 0 ] . u . data . base;
    str += argv [ 0 ] . u . data . first_elem;

    if ( count >= sizeof buffer )
        return RC ( rcXF, rcFunction, rcExecuting, rcParam, rcExcessive );

    for ( i = 0; i < count; ++ i )
    {
        if ( str [ i ] > 128 )
            return RC ( rcXF, rcFunction, rcExecuting, rcParam, rcInvalid );
        buffer [ i ] = ( char ) str [ i ];
    }

    return strtoflt ( data, info, rslt, buffer, count );
}

VTRANSFACT_IMPL ( vdb_strtonum, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    uint32_t radix = 0;

    if ( cp -> argc == 1 )
    {
        radix = cp -> argv [ 0 ] . data . u32 [ 0 ];
        if ( radix > 36 )
            return RC ( rcXF, rcFunction, rcConstructing, rcParam, rcInvalid );
    }

    if ( info -> fdesc . desc . domain == vtdFloat ) switch ( radix )
    {
    case 0:
    case 10:
        break;
    default:
        return RC ( rcXF, rcFunction, rcConstructing, rcParam, rcInvalid );
    }

    if ( dp -> argv [ 0 ] . desc . intrinsic_dim != 1 )
        return RC ( rcXF, rcFunction, rcConstructing, rcParam, rcInvalid );

    switch ( dp -> argv [ 0 ] . desc . intrinsic_bits )
    {
    case 8:
        rslt -> u . rf = ( info -> fdesc . desc . domain == vtdFloat ) ? strtoflt_8 : strtoint_8;
        break;
    case 16:
        rslt -> u . rf = ( info -> fdesc . desc . domain == vtdFloat ) ? strtoflt_16 : strtoint_16;
        break;
    case 32:
        rslt -> u . rf = ( info -> fdesc . desc . domain == vtdFloat ) ? strtoflt_32 : strtoint_32;
        break;
    default:
        return RC ( rcXF, rcFunction, rcConstructing, rcParam, rcInvalid );
    }

    rslt -> self = ( void* ) ( size_t ) radix;
    rslt -> variant = vftRow;

    return 0;
}
