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
#include "name-tokenizer.h"
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>


/* extract_name_coord
 *  generates inputs to .X and .Y and possibly other columns
 *
 *  if no tokens match "coord" constant, produces an empty row
 *  otherwise, produces binary coordinate value
 *  if multiple tokens match criteria, all values must be equivalent
 *  because only a single value will be output per row
 *
 *  "coord" [ CONST ] - either NCBI:SRA:name_token:X or NCBI:SRA:name_token:Y
 *  both of these values also match the token NCBI:SRA:name_token:Q and extract
 *  contents appropriately.
 *
 *  "name" [ DATA ] - raw spot names from NAME column
 *
 *  "tok" [ DATA ] - delimiting tokens produced by sub-table
function INSDC:coord:zero
    NCBI:SRA:extract_name_coord #1 < U16 coord > ( ascii name, NCBI:SRA:spot_name_token tok );
*/

typedef struct self_t {
    uint16_t coordinate;
    int position;
} self_t;

/* return 0...failed, 1...success */

MOD_EXPORT uint32_t CC parse_decimal ( int32_t* base, const char* src, const uint32_t length )
{
    /* coordinates are normally unsigned */
    bool negate = false;

    /* fetch initial character */
    int32_t val = src [ 0 ];

    /* assume loop starts with following character */
    uint32_t i = 1;

    /* detect explicit sign */
    switch ( val )
    {
    case '-':
        negate = true;
    case '+':
        val = src [ 1 ];
        i = 2;
        break;
    }

    if ( ! isdigit ( val ) )
        return 0;

    for ( val -= '0'; i < length; ++ i )
    {
        val *= 10;
        if ( ! isdigit ( src [ i ] ) )
            return 0;
        val += src [ i ] - '0';
    }

    * base = negate ? - val : val;

    return 1;
}

MOD_EXPORT uint32_t CC parse_Q ( uint32_t* base, const char* src,
    const uint32_t length, const uint16_t what )
{
    uint32_t xy, i;
    
    if( length != 5 ) {
        return 0;
    }
    for( xy = i = 0; i < length; ++ i )
    {
        char n = src[i];
        xy *= 36;
        if( isdigit ( n ) )
        {
            xy += n - '0' + 26;
        }
        else if ( isalpha ( n ) )
        {
            xy += toupper ( n ) - 'A';
        } 
        else
        {
            return 0;
        }
    }
    if ( what == nt_X )
    {
        *base = xy >> 12;
        return 1;
    }
    if ( what == nt_Y )
    {
        *base = xy & 0xFFF;
        return 1;
    }

    return 0;
}
 
static
rc_t CC extract_name_coordinate ( void *Self, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    uint32_t i, j;
    self_t* self = Self;

    const char *name;
    const spot_name_token_t* token;
    uint32_t name_len, num_tokens;

    assert( argc == 2 );

    /* name string */
    name = argv [ 0 ] . u . data . base;
    name_len = argv [ 0 ] . u . data . elem_count;
    name += argv [ 0 ] . u . data . first_elem;

    /* tokens */
    token = argv [ 1 ] . u . data . base;
    num_tokens = argv [ 1 ] . u . data . elem_count;
    token += argv [ 1 ] . u . data . first_elem;

    for ( i = j = 0; i < num_tokens; ++ i )
    {
        if ( token [ i ] . s . token_type != self -> coordinate )
        {
            if ( token [ i ] . s . token_type != nt_Q )
                continue;
            switch ( self -> coordinate )
            {
            case nt_X:
            case nt_Y:
                break;
            default:
                continue;
            }
        }

        if ( ++ j == self->position )
        {
            /* check bounds */
            if ( (uint32_t)( token [ i ] . s . position + token [ i ] . s . length ) > name_len )
                return 0;

            /* convert token */
            rc = KDataBufferResize( rslt->data, 1 );
            if ( rc )
                return rc;

            switch ( token [ i ] . s . token_type )
            {
            case nt_X:
            case nt_Y:
            case nt_T:
            case nt_L:
                rslt->elem_count = parse_decimal ( rslt -> data -> base,
                    name + token [ i ] . s . position, token [ i ] . s . length );
                return 0;
            case nt_Q : rslt->elem_count = parse_Q ( rslt -> data -> base,
                    name + token [ i ] . s . position, token [ i ] . s . length,
                    self -> coordinate );
                return 0;
            }
        }
    }

    rslt->elem_count = 0;
    return 0;
}

static
void CC sraxf_extract_name_coord_wrapper( void *ptr )
{
	free( ptr );
}

VTRANSFACT_IMPL ( NCBI_SRA_extract_name_coord, 1, 0, 0 ) ( const void *self,
    const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    self_t* Self;

    assert( cp->argc >= 1 );

    Self = malloc( sizeof( self_t ) );
    if ( Self == NULL ) return RC( rcSRA, rcFunction, rcConstructing, rcMemory, rcExhausted );
    Self->coordinate = cp->argv[0].data.u16[0];
    Self->position = 1;
    rslt->self = Self;
    rslt->whack = sraxf_extract_name_coord_wrapper;

    rslt->variant = vftRow;
    rslt->u.rf = extract_name_coordinate;

    return 0;
}
