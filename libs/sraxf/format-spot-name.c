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
#include <klib/data-buffer.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/rc.h>
#include <sysalloc.h>
#include "name-tokenizer.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

/* format_spot_name ( ascii name_fmt, I32 X, I32 Y * ascii spot_name );
 *  given a name format string, X, and Y
 *  produce a reconstructed spot name string
 *
 *  "name_fmt" [ DATA ] - name format string ( see format explanation below )
 *
 *  "X" [ DATA ] - X coordinate for spot
 *
 *  "Y" [ DATA ] - Y coordinate for spot
 *
 *  "spot_name" [ DATA, OPTIONAL ] - potential source of unformatted names
 *
 * SYNOPSIS:
 *  "name_fmt" may have any ASCII characters
 *  the special character '$' is an escape symbol
 *  when followed by a recognized format character,
 *  both the '$' and its format character will be
 *  replaced with a numeral generated from X and/or Y.
 *
 *  when "spot_name" is present and the "name_fmt" row is empty,
 *  output is taken verbatim from "spot_name"
 *
 * FORMAT:
 *  'X' ( or 'x' ) = substitute with a decimal representation for X
 *  'Y' ( or 'y' ) = substitute with a decimal representation for Y
 *  'Q' ( or 'q' ) = calculate 454-style base-36 representation
 *                   for both X and Y
 */
static
rc_t CC format_spot_name ( void *self,
    const VXformInfo *info, int64_t row_id, VRowResult *rslt,
    uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    char *name;
    uint32_t elem_count;
    KDataBuffer *dst = rslt -> data;

    if ( dst -> elem_bits != 8 )
    {
        rc = KDataBufferCast ( dst, dst, 8, true );
        if ( rc != 0 )
            return rc;
    }

    /* check for NAME_FMT */
    if ( argv [ 0 ] . u. data . elem_count != 0 )
    {
        size_t num_writ;
        char sname[1024]; /** name on stack **/
        const char *name_fmt = ((char*)argv[0].u.data.base) + argv[0].u.data.first_elem;
        uint32_t i, j, x, y;
        const uint32_t fmt_size = argv [ 0 ] . u . data . elem_count;

        /* the coordinates to substitute */
        x = ( ( const int32_t* ) argv [ 1 ] . u . data . base )
            [ argv [ 1 ] . u . data . first_elem ];
        y = ( ( const int32_t* ) argv [ 2 ] . u . data . base )
            [ argv [ 2 ] . u . data . first_elem ];

        for ( i=j=0; i < fmt_size -1;){
            if( name_fmt [ i ] == '$' ){
                switch( name_fmt [ i+1 ]){
                case 'x': case 'X':
                    if( j > sizeof(sname) - 11){
    					return RC ( rcXF, rcFunction, rcDecoding, rcBuffer, rcInsufficient );
                    } else {
                        i+=2;
                        if( i < fmt_size -1 && name_fmt [ i ] == '%' && isdigit(name_fmt [ i+1 ])) {
                            x += 24*1024*(name_fmt [ i+1 ]-'0');
                            i+=2;
                        }
                        j+=sprintf(sname+j,"%d",x);
                    }
                    break;
                case 'y': case 'Y':
                    if( j > sizeof(sname) - 11){
    					return RC ( rcXF, rcFunction, rcDecoding, rcBuffer, rcInsufficient );
                    } else {
                        i+=2;
                        if( i < fmt_size -1 && name_fmt [ i ] == '%' && isdigit(name_fmt [ i+1 ])) {
                            y += 24*1024*(name_fmt [ i+1 ]-'0');
                            i+=2;
                        }
                        j+=sprintf(sname+j,"%d",y);
                    }
                    break;
                case 'q': case 'Q':
                    if( j > sizeof(sname) - 5) {
    					return RC ( rcXF, rcFunction, rcDecoding, rcBuffer, rcInsufficient );
                    } else if( y > 0xFFF) {
                        return RC ( rcXF, rcFunction, rcDecoding, rcRange, rcTooBig);
                    } else {
                        uint32_t q= x << 12 | y;
                        sname[j+4]=Q_ALPHABET[q % 36]; q /= 36;
                        sname[j+3]=Q_ALPHABET[q % 36]; q /= 36;
                        sname[j+2]=Q_ALPHABET[q % 36]; q /= 36;
                        sname[j+1]=Q_ALPHABET[q % 36]; q /= 36;
                        sname[j]  =Q_ALPHABET[q];
                        j+=5;
                        i+=2;
                    }
                    break;
		 case 'r': case 'R':
            rc = string_printf ( & sname [ j ], sizeof sname - j, & num_writ, "%ld", row_id );
            assert ( rc == 0 );
            j += ( uint32_t ) num_writ;
		    i+=2;
		    break;
                default:
                    sname[j++]=name_fmt[i++];
                    break;
                }
            } else {
                if(j < sizeof(sname)){
                    sname[j++]=name_fmt[i++];
                } else {
                    return RC ( rcXF, rcFunction, rcDecoding, rcBuffer, rcInsufficient );
                }
            }
            if( j > sizeof(sname)-1){	
                return RC ( rcXF, rcFunction, rcDecoding, rcBuffer, rcInsufficient );
            }
        }
        if(i==fmt_size -1) {
            if(j < sizeof(sname)){
                sname[j++]=name_fmt[i++];
            } else {
                return RC ( rcXF, rcFunction, rcDecoding, rcBuffer, rcInsufficient );
            }
        }
        
        /* j is our new element count **/
        elem_count = j;

        /* resize output */
        if ( dst -> elem_count <= elem_count )
        {
            rc = KDataBufferResize ( dst, elem_count);
            if ( rc != 0 )
                return rc;
        }
        
        /* the output name */
        name = dst -> base;
        
        memcpy ( name, sname, elem_count );
        rslt -> elem_count = elem_count;
        return 0;
    }

    /* check for NAME */
    if( argc == 4 && argv[3].u.data.elem_count != 0 ) {
        const char *sname = ((char*)argv[3].u.data.base) + argv[3].u.data.first_elem;
        /* output size */
        elem_count = argv[3].u.data.elem_count;

        /* resize output */
        if( dst -> elem_count <= elem_count ) {
            rc = KDataBufferResize( dst, elem_count + 1 );
            if( rc != 0 )
                return rc;
        }
        name = dst->base;
        memcpy(dst->base, sname, elem_count);
        rslt->elem_count = elem_count;
        name[elem_count] = 0;
        return 0;
    }

    /* spot has no name */
    return RC ( rcSRA, rcColumn, rcReading, rcRow, rcNull );
}
static
rc_t CC format_spot_name_no_coord ( void *self,
    const VXformInfo *info, int64_t row_id, VRowResult *rslt,
    uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    char *name;
    uint32_t elem_count;
    KDataBuffer *dst = rslt -> data;

    if ( dst -> elem_bits != 8 )
    {
        rc = KDataBufferCast ( dst, dst, 8, true );
        if ( rc != 0 )
            return rc;
    }

    /* check for NAME_FMT */
    if ( argv [ 0 ] . u. data . elem_count != 0 )
    {
        size_t num_writ;
        char sname[1024]; /** name on stack **/
        const char *name_fmt = ((char*)argv[0].u.data.base) + argv[0].u.data.first_elem;
        uint32_t i, j;
        const uint32_t fmt_size = argv [ 0 ] . u . data . elem_count;

        for ( i=j=0; i < fmt_size -1;){
            if( name_fmt [ i ] == '$' ){
                switch( name_fmt [ i+1 ]){
		 case 'r': case 'R':
            rc = string_printf ( & sname [ j ], sizeof sname - j, & num_writ, "%ld", row_id );
            assert ( rc == 0 );
		    j += ( uint32_t ) num_writ;
		    i+=2;
		    break;
                default:
                    sname[j++]=name_fmt[i++];
                    break;
                }
            } else {
                if(j < sizeof(sname)){
                    sname[j++]=name_fmt[i++];
                } else {
                    return RC ( rcXF, rcFunction, rcDecoding, rcBuffer, rcInsufficient );
                }
            }
            if( j > sizeof(sname)-1){	
                return RC ( rcXF, rcFunction, rcDecoding, rcBuffer, rcInsufficient );
            }
        }
        if(i==fmt_size -1) {
            if(j < sizeof(sname)){
                sname[j++]=name_fmt[i++];
            } else {
                return RC ( rcXF, rcFunction, rcDecoding, rcBuffer, rcInsufficient );
            }
        }
        
        /* j is our new element count **/
        elem_count = j;

        /* resize output */
        if ( dst -> elem_count <= elem_count )
        {
            rc = KDataBufferResize ( dst, elem_count);
            if ( rc != 0 )
                return rc;
        }
        
        /* the output name */
        name = dst -> base;
        
        memcpy ( name, sname, elem_count );
        rslt -> elem_count = elem_count;
        return 0;
    }

    /* check for NAME */
    if( argc == 2 && argv[1].u.data.elem_count != 0 ) {
        const char *sname = ((char*)argv[1].u.data.base) + argv[1].u.data.first_elem;
        /* output size */
        elem_count = argv[1].u.data.elem_count;

        /* resize output */
        if( dst -> elem_count <= elem_count ) {
            rc = KDataBufferResize( dst, elem_count + 1 );
            if( rc != 0 )
                return rc;
        }
        name = dst->base;
        memcpy(dst->base, sname, elem_count);
        rslt->elem_count = elem_count;
        name[elem_count] = 0;
        return 0;
    }

    /* spot has no name */
    return RC ( rcSRA, rcColumn, rcReading, rcRow, rcNull );
}
/*
 * extern function ascii INSDC:SRA:format_spot_name #1 ( ascii name_fmt, I32 X, I32 Y * ascii spot_name );
 */
VTRANSFACT_IMPL ( INSDC_SRA_format_spot_name, 1, 0, 0 ) ( const void *self,
    const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt -> u . rf = format_spot_name;
    rslt -> variant = vftRow;
    return 0;
}


VTRANSFACT_IMPL ( INSDC_SRA_format_spot_name_no_coord, 1, 0, 0 ) ( const void *self,
    const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt -> u . rf = format_spot_name_no_coord;
    rslt -> variant = vftRow;
    return 0;
}

