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
#include <klib/text.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>


#define SLX_COORD_LEN 10

static
int non_braindead_atoi ( const char *start, const char *end )
{
    int val = start [ 0 ] - '0';
    uint32_t i, count = (uint32_t)( end - start );
    for ( i = 1; i < count; ++ i )
    {
        val *= 10;
        val += start [ i ] - '0';
    }
    return val;
}

static
int scan_hex ( const char *str, int count )
{
    int i, val = str [ 0 ] - '0';
    if ( val > 9 )
        val = toupper ( str [ 0 ] ) - 'A' + 10;
    for ( i = 1; i < count; ++ i )
    {
        int d = str [ i ] - '0';
        if ( d > 9 )
            d = toupper ( str [ i ] ) - 'A' + 10;
        val = val << 4 | d;
    }
    return val;
}

static
void common_extract_coordinates ( const char *skey, uint32_t slen, uint32_t coords [ 4 ] )
{
    unsigned int a, b, c, d;
    uint32_t i, j, count = slen;

    /* scan hex digits */
    for ( i = count; i > 0; )
    {
        if ( ! isxdigit ( skey [ -- i ] ) )
            break;
    }

    /* detect newer format */
    if ( count - i < ( SLX_COORD_LEN - 1 ) )
    {
        const char *end = skey + count;

        /* new format */
        for ( d = 0, j = 0, i = count; i > 0; )
        {
            if ( ! isdigit ( skey [ -- i ] ) )
            {
                j = i + 1;
                break;
            }
        }
        d = non_braindead_atoi ( & skey [ j ], end );
        for ( c = 0, j = 0; i > 0; )
        {
            if ( ! isdigit ( skey [ -- i ] ) )
            {
                j = i + 1;
                break;
            }
        }
        c = atoi ( & skey [ j ] );
        for ( b = 0, j = 0; i > 0; )
        {
            if ( ! isdigit ( skey [ -- i ] ) )
            {
                j = i + 1;
                break;
            }
        }
        b = atoi ( & skey [ j ] );
        for ( a = 0, j = 0; i > 0; )
        {
            if ( ! isdigit ( skey [ -- i ] ) )
            {
                j = i + 1;
                break;
            }
        }
        a = atoi ( & skey [ j ] );
        if ( j > 0 )
        {
            if ( i > 0 )
                -- i;
            while ( isalpha ( skey [ i ] ) )
                ++ i;
        }
    }
    else
    {
        /* original format */
        a = scan_hex ( skey, 1 );
        b = scan_hex ( & skey [ 1 ], 3 );
        c = scan_hex ( & skey [ 4 ], 3 );
        d = scan_hex ( & skey [ 7 ], 3 );
        if ( count > SLX_COORD_LEN )
        {
            i = count - SLX_COORD_LEN;
        }
    }

    coords [ 3 ] = a;
    coords [ 2 ] = b;
    coords [ 1 ] = c;
    coords [ 0 ] = d;
}

static
rc_t CC illumina_extract_coordinates ( void *self, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    uint32_t *out;
    KDataBuffer *dst = rslt -> data;

    uint32_t coords [ 4 ];

    const char *skey = argv [ 0 ] . u . data . base;
    uint32_t i, count = argv [ 0 ] . u . data . elem_count;
    skey += argv [ 0 ] . u . data . first_elem;

    common_extract_coordinates ( skey, count, coords );

    /* count the number of output coordinates */
    count = ( uint32_t ) ( size_t ) self;

    /* produce however many elements are requested */
    dst -> elem_bits = 32 * count;
    rc = KDataBufferResize ( dst, 1 );
    if ( rc != 0 )
        return rc;

    out = dst -> base;
    switch ( count )
    {
    default:
        for ( i = 4; i < count; ++ i )
            out [ i ] = 0;
    case 4:
        out [ 3 ] = coords [ 3 ];
    case 3:
        out [ 2 ] = coords [ 2 ];
    case 2:
        out [ 1 ] = coords [ 1 ];
    case 1:
        out [ 0 ] = coords [ 0 ];
    case 0:
        break;
    }

    rslt -> elem_count = 1;

    return 0;
}

static
rc_t CC abi_extract_coordinates ( void *self, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    uint32_t *out;
    KDataBuffer *dst = rslt -> data;

    uint32_t coords [ 4 ];

    const char *skey = argv [ 0 ] . u . data . base;
    uint32_t i, count = argv [ 0 ] . u . data . elem_count;
    skey += argv [ 0 ] . u . data . first_elem;

    common_extract_coordinates ( skey, count, coords );

    /* count the number of output coordinates */
    count = ( uint32_t ) ( size_t ) self;

    /* produce however many elements are requested */
    dst -> elem_bits = 32 * count;
    rc = KDataBufferResize ( dst, 1 );
    if ( rc != 0 )
        return rc;

    out = dst -> base;
    switch ( count )
    {
    default:
        for ( i = 3; i < count; ++ i )
            out [ i ] = 0;
    case 3:
        out [ 2 ] = coords [ 2 ];
    case 2:
        out [ 1 ] = coords [ 1 ];
    case 1:
        out [ 0 ] = coords [ 0 ];
    case 0:
        break;
    }

    rslt -> elem_count = 1;

    return 0;
}

static
rc_t CC _454_extract_coordinates ( void *self, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    uint32_t *out;
    unsigned int a, b, c;
    KDataBuffer *dst = rslt -> data;

    const char *skey = argv [ 0 ] . u . data . base;
    uint32_t i, count = argv [ 0 ] . u . data . elem_count;
    skey += argv [ 0 ] . u . data . first_elem;

    /* advance skey to coordinates */
    if ( count < 7 )
        return RC ( rcSRA, rcName, rcParsing, rcString, rcInsufficient );
    skey += count - 7;

    /* get region */
    if ( ! isdigit ( skey [ 0 ] ) || ! isdigit ( skey [ 1 ] ) )
        return RC ( rcSRA, rcName, rcParsing, rcString, rcCorrupt );
    a = ( skey [ 0 ] - '0' ) * 10 + ( skey [ 1 ] - '0' );

    /* get 'Q' - base-36 encoded x/y */
    for ( c = 0, i = 2; i < 7; ++ i )
    {
        c *= 36;
        if ( isdigit ( skey [ i ] ) )
            c += skey [ i ] - '0' + 26;
        else if ( ! isalpha ( skey [ i ] ) )
            return RC ( rcSRA, rcName, rcParsing, rcString, rcCorrupt );
        else
        {
            /* yes, the alphabetic symbols come BEFORE the numeric */
            c += toupper ( skey [ i ] ) - 'A';
        }
    }

    /* convert 'Q' to x and y */
    b = c >> 12;
    c &= 0xFFF;

    /* count the number of output coordinates */
    count = ( uint32_t ) ( size_t ) self;

    /* produce however many elements are requested */
    dst -> elem_bits = 32 * count;
    rc = KDataBufferResize ( dst, 1 );
    if ( rc != 0 )
        return rc;

    out = dst -> base;
    switch ( count )
    {
    default:
        for ( i = 3; i < count; ++ i )
            out [ i ] = 0;
    case 3:
        out [ 2 ] = a;
    case 2:
        out [ 1 ] = b;
    case 1:
        out [ 0 ] = c;
    case 0:
        break;
    }

    rslt -> elem_count = 1;

    return 0;
}

/* extract coordinates
 *  given a spotname with encoded coordinates
 *  extract them in reverse order into a vector
 *
 *  "platform" [ CONST ] - which platform rules to use
 *
 *  "skey" [ DATA ] - skey string for row
 */
VTRANSFACT_IMPL ( NCBI_SRA_extract_coordinates, 1, 0, 0 ) ( const void *self,
    const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    const uint8_t *platform = cp -> argv [ 0 ] . data . u8;
    switch ( platform [ 0 ] )
    {
    case SRA_PLATFORM_454:
        rslt -> u . rf = _454_extract_coordinates;
        break;
    case SRA_PLATFORM_ILLUMINA:
    case SRA_PLATFORM_HELICOS:
        rslt -> u . rf = illumina_extract_coordinates;
        break;
    case SRA_PLATFORM_ABSOLID:
        rslt -> u . rf = abi_extract_coordinates;
        break;
    default:
        return RC ( rcSRA, rcFunction, rcConstructing, rcType, rcUnsupported );
    }

    assert ( info -> fdesc . desc . intrinsic_bits == 32 );
    rslt -> self = ( void* ) ( size_t ) info -> fdesc . desc . intrinsic_dim;
    rslt -> variant = vftRow;
    return 0;
}
