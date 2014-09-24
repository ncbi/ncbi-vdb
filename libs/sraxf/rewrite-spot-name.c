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
#include <vdb/table.h>
#include <kdb/meta.h>
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
rc_t CC illumina_rewrite_spot_name ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    char buffer [ 64];
    uint32_t coord_len;
    uint32_t prefix_len;
    unsigned int a, b, c, d;
    KDataBuffer *dst = rslt -> data;

    const char *prefix;
    const char *skey = argv [ 0 ] . u . data . base;
    uint64_t i, j, count = argv [ 0 ] . u . data . elem_count;
    
    skey += argv [ 0 ] . u . data . first_elem;

    /* find last hex portion */
    for ( i = count; i > 0; )
    {
        if ( ! isxdigit ( skey [ -- i ] ) )
            break;
    }

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
        a = scan_hex ( skey, 1 );
        b = scan_hex ( & skey [ 1 ], 3 );
        c = scan_hex ( & skey [ 4 ], 3 );
        d = scan_hex ( & skey [ 7 ], 3 );
        if ( count > SLX_COORD_LEN )
        {
            i = count - SLX_COORD_LEN;
        }
    }

    /* generate coordinates */
    coord_len = sprintf ( buffer, ":%d:%d:%d:%d", a, b, c, d );

    /* get size of prefix */
    if ( argc == 1 )
    {
        prefix = "";
        prefix_len = 0;
    }
    else
    {
        prefix = argv [ 1 ] . u . data . base;
        assert(argv [ 1 ] . u . data . elem_count >> 32 == 0);
        prefix_len = (uint32_t)argv [ 1 ] . u . data . elem_count;
        prefix += argv [ 1 ] . u . data . first_elem;
    }

    /* resize output buffer for prefix, name stuff, coordinates */
    if ( dst -> elem_bits != 8 )
    {
        rc = KDataBufferCast ( dst, dst, 8, true );
        if ( rc != 0 )
            return rc;
    }
    rc = KDataBufferResize ( dst, prefix_len + i + coord_len + 1 );
    if ( rc != 0 )
        return rc;

    /* copy in prefix, name prefix, coordinates */
    rslt -> elem_count = sprintf ( dst -> base, "%.*s%.*s%s"
        , ( int ) prefix_len, prefix
        , ( int ) i, skey
        , buffer );

    return 0;
}

/* rewrite_spot_name
 *  given an old spotname directly from skey
 *  write according to platform rules, optionally with a prefix
 *
 *  "platform" [ CONST ] - which platform rules to use
 *
 *  "skey" [ DATA ] - skey string for row
 *
 *  "prefix" [ DATA, OPTIONAL ] - prefix for spot
 */
VTRANSFACT_IMPL ( NCBI_SRA_rewrite_spot_name, 1, 0, 0 ) ( const void *self,
    const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    const uint8_t *platform = cp -> argv [ 0 ] . data . u8;
    switch ( platform [ 0 ] )
    {
    case SRA_PLATFORM_ILLUMINA:
        break;
    default:
        return RC ( rcSRA, rcFunction, rcConstructing, rcType, rcUnsupported );
    }

    rslt -> u . ndf = illumina_rewrite_spot_name;
    rslt -> variant = vftNonDetRow;
    return 0;
}
