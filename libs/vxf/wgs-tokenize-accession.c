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
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

enum
{
    NCBI_WGS_acc_token_unrecognized = 1,
    NCBI_WGS_acc_token_prefix,
    NCBI_WGS_acc_token_contig
};

enum
{
    TTextToken_id,
    TTextToken_start,
    TTextToken_len
};


static
uint32_t ParseWgsAccession ( uint16_t *tok, const char *acc, size_t acc_len )
{
    /* WGS accessions have form:
     * [NZ_]<4-letter prefix><2-digit version><6-7 digit contig>
     */
    size_t i;

    /* scan non-digits */
    for ( i = 0; i < acc_len; ++ i )
    {
        if ( isdigit ( acc [ i ] ) )
            break;
    }

    /* require at least 4 characters... */
    if ( i >= 4 )
    {
        /* require at least 8 digits */
        if ( acc_len - i >= 8 )
        {
            /* 2-digit version */
            if ( isdigit ( acc [ ++ i ] ) )
            {
                /* record first token as prefix */
                tok [ 0 + TTextToken_id ] = NCBI_WGS_acc_token_prefix;
                tok [ 0 + TTextToken_start ] = 0;
                tok [ 0 + TTextToken_len ] = ( uint16_t ) ++ i;

                /* the remainder should be contig */
                tok [ 3 + TTextToken_id ] = NCBI_WGS_acc_token_contig;
                tok [ 3 + TTextToken_start ] = ( uint16_t ) i;
                tok [ 3 + TTextToken_len ] = ( uint16_t ) ( acc_len - i );

                /* verify that it is numeric */
                for ( ; i < acc_len; ++ i )
                {
                    if ( ! isdigit ( acc [ i ] ) )
                        break;
                }

                /* great success */
                if ( i == acc_len )
                    return 2;
            }
        }
    }

    tok [ 0 + TTextToken_id ] = NCBI_WGS_acc_token_unrecognized;
    tok [ 0 + TTextToken_start ] = 0;
    tok [ 0 + TTextToken_len ] = ( uint16_t ) acc_len;

    return 1;
}

static
rc_t CC tokenize_nuc_accession ( void *obj,
     const VXformInfo *info, int64_t row_id, VRowResult *rslt,
     uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    const char *acc;
    
    assert ( rslt -> elem_bits == 16 * 3 );

    rslt -> data -> elem_bits = rslt -> elem_bits;
    rc = KDataBufferResize ( rslt -> data, 2 );
    if ( rc != 0 )
        return rc;

    acc = argv [ 0 ] . u . data . base;
    acc += argv [ 0 ] . u . data . first_elem;

    rslt -> elem_count = ParseWgsAccession ( rslt -> data -> base, acc, argv [ 0 ] . u . data . elem_count );
    if ( rslt -> elem_count != 2 )
        rc = KDataBufferResize ( rslt -> data, rslt -> elem_count );

    return rc;
}

static
uint32_t ParseProtAccession ( uint16_t *tok, const char *acc, size_t acc_len )
{
    /* Protein accessions generally have pattern 
     * <Cap-letter prefix><digital suffix>
     * If this doesn't hold, use full accession as prefix, and make suffix empty.
     */
    size_t i;

    for ( i = 0; i < acc_len; ++ i )
    {
        if ( isdigit ( acc [ i ] ) )
            break;
    }

    tok [ 0 + TTextToken_id ] = NCBI_WGS_acc_token_prefix;
    tok [ 0 + TTextToken_start ] = 0;
    tok [ 0 + TTextToken_len ] = ( uint16_t ) i;

    for ( ; i < acc_len; ++ i )
    {
        if ( ! isdigit ( acc [ i ] ) )
            break;
    }

    if ( i == acc_len )
    {
        tok [ 3 + TTextToken_id ] = NCBI_WGS_acc_token_contig;
        tok [ 3 + TTextToken_start ] = tok [ 0 + TTextToken_len ];
        tok [ 3 + TTextToken_len ] = ( uint16_t ) ( i - tok [ 0 + TTextToken_len ] );
        return 2;
    }

    tok [ 0 + TTextToken_len ] = ( uint16_t ) acc_len;
    return 1;
}

static
rc_t CC tokenize_prot_accession ( void *obj,
     const VXformInfo *info, int64_t row_id, VRowResult *rslt,
     uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    const char *acc;
    
    assert ( rslt -> elem_bits == 16 * 3 );

    rslt -> data -> elem_bits = rslt -> elem_bits;
    rc = KDataBufferResize ( rslt -> data, 2 );
    if ( rc != 0 )
        return rc;

    acc = argv [ 0 ] . u . data . base;
    acc += argv [ 0 ] . u . data . first_elem;

    rslt -> elem_count = ParseProtAccession ( rslt -> data -> base, acc, argv [ 0 ] . u . data . elem_count );
    if ( rslt -> elem_count != 2 )
        rc = KDataBufferResize ( rslt -> data, rslt -> elem_count );

    return rc;
}

VTRANSFACT_IMPL ( NCBI_WGS_tokenize_nuc_accession, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt -> u . rf = tokenize_nuc_accession;
    rslt -> variant = vftRow;
    return 0;
}

VTRANSFACT_IMPL ( NCBI_WGS_tokenize_prot_accession, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt -> u . rf = tokenize_prot_accession;
    rslt -> variant = vftRow;
    return 0;
}
