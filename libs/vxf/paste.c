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
#include <bitstr.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct pastie pastie;
struct pastie
{
    struct
    {
        union
        {
            void *u1;
            uint8_t *u8;
            uint16_t *u16;
            uint32_t *u32;
            uint64_t *u64;
        } u;
        bitsz_t off;

    } dst;
    struct
    {
        union
        {
            const void *u1;
            const uint8_t *u8;
            const uint16_t *u16;
            const uint32_t *u32;
            const uint64_t *u64;
        } u;
        bitsz_t off, len;

    } src [ 16 ];
};

static
rc_t CC paste_func1 ( void *data, const VXformInfo *info, int64_t row_id,
    const VFixedRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc = 0;
    pastie *self = data;
    uint32_t i, j, row_len = (uint32_t)rslt -> elem_count;

    assert(rslt -> elem_count >> 32 == 0);
    /* populate "self" */
    self -> dst . u . u1 = rslt -> base;
    self -> dst . off = ((uint64_t)rslt -> first_elem) * rslt -> elem_bits;
    for ( j = 0; j < argc; ++ j )
    {
        self -> src [ j ] . u . u1 = argv [ j ] . u . data .base;
        self -> src [ j ] . off = ((uint64_t)argv [ j ] . u . data .first_elem) * argv [ j ] . u . data .elem_bits;
        self -> src [ j ] . len = argv [ j ] . u . data .elem_bits;
    }

    /* create output row */
    for ( i = 0; i < row_len; ++ i )
    {
        for ( j = 0; j < argc; ++ j )
        {
            uint32_t k = (uint32_t)self -> src [ j ] . len;
            assert(self -> src [ j ] . len >> 32 == 0);
            bitcpy ( self -> dst . u . u1, self -> dst . off,
                     self -> src [ j ] . u . u1, self -> src [ j ] . off, k );
            self -> dst . off += k;
            self -> src [ j ] . off += k;
        }
    }
    return rc;
}

static
rc_t CC paste_func8 ( void *data, const VXformInfo *info, int64_t row_id,
    const VFixedRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc = 0;
    pastie *self = data;
    uint32_t i, j, row_len = (uint32_t)rslt -> elem_count;
    
    assert(rslt -> elem_count >> 32 == 0);
    /* populate "self" */
    self -> dst . u . u8 = rslt -> base;
    self -> dst . off = ((uint64_t)rslt -> first_elem) * (rslt -> elem_bits>>3);
    for ( j = 0; j < argc; ++ j )
    {
        self -> src [ j ] . u . u8 = argv [ j ] . u . data .base;
        self -> src [ j ] . off = ((uint64_t)argv [ j ] . u . data .first_elem) * (argv [ j ] . u . data .elem_bits >> 3);
        self -> src [ j ] . len = argv [ j ] . u . data .elem_bits >> 3;
    }

    /* create output row */
    for ( i = 0; i < row_len; ++ i )
    {
        for ( j = 0; j < argc; ++ j )
        {
            uint32_t k;
            for ( k = 0; k < self -> src [ j ] . len; ++ k )
            {
                self -> dst . u . u8 [ self -> dst . off + k ] =
                    self -> src [ j ] . u . u8 [ self -> src [ j ] . off + k ];
            }

            self -> dst . off += k;
            self -> src [ j ] . off += k;
        }
    }
    return rc;
}

static
rc_t CC paste_func16 ( void *data, const VXformInfo *info, int64_t row_id,
    const VFixedRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc = 0;
    pastie *self = data;
    uint32_t i, j, row_len = (uint32_t)rslt -> elem_count;
    
    assert(rslt -> elem_count >> 32 == 0);
    /* populate "self" */
    self -> dst . u . u16 = rslt -> base;
    self -> dst . off = ((uint64_t)rslt -> first_elem) * (rslt -> elem_bits>>4);
    for ( j = 0; j < argc; ++ j )
    {
        self -> src [ j ] . u . u16 = argv [ j ] . u . data .base;
        self -> src [ j ] . off = ((uint64_t)argv [ j ] . u . data .first_elem) * (argv [ j ] . u . data .elem_bits >> 4);
        self -> src [ j ] . len = argv [ j ] . u . data .elem_bits >> 4;
    }

    /* create output row */
    for ( i = 0; i < row_len; ++ i )
    {
        for ( j = 0; j < argc; ++ j )
        {
            uint32_t k;
            for ( k = 0; k < self -> src [ j ] . len; ++ k )
            {
                self -> dst . u . u16 [ self -> dst . off + k ] =
                    self -> src [ j ] . u . u16 [ self -> src [ j ] . off + k ];
            }

            self -> dst . off += k;
            self -> src [ j ] . off += k;
        }
    }
    return rc;
}

static
rc_t CC paste_func32 ( void *data, const VXformInfo *info, int64_t row_id,
    const VFixedRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc = 0;
    pastie *self = data;
    uint32_t i, j, row_len = (uint32_t)rslt -> elem_count;
    
    assert(rslt -> elem_count >> 32 == 0);
    /* populate "self" */
    self -> dst . u . u32 = rslt -> base;
    self -> dst . off = ((uint64_t)rslt -> first_elem) * (rslt -> elem_bits>>5);
    for ( j = 0; j < argc; ++ j )
    {
        self -> src [ j ] . u . u32 = argv [ j ] . u . data .base;
        self -> src [ j ] . off = ((uint64_t)argv [ j ] . u . data .first_elem) * (argv [ j ] . u . data .elem_bits >> 5);
        self -> src [ j ] . len = argv [ j ] . u . data .elem_bits >> 5;
    }

    /* create output row */
    for ( i = 0; i < row_len; ++ i )
    {
        for ( j = 0; j < argc; ++ j )
        {
            uint32_t k;
            for ( k = 0; k < self -> src [ j ] . len; ++ k )
            {
                self -> dst . u . u32 [ self -> dst . off + k ] =
                    self -> src [ j ] . u . u32 [ self -> src [ j ] . off + k ];
            }

            self -> dst . off += k;
            self -> src [ j ] . off += k;
        }
    }
    return rc;
}

static
rc_t CC paste_func64 ( void *data, const VXformInfo *info, int64_t row_id,
    const VFixedRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc = 0;
    pastie *self = data;
    uint32_t i, j, row_len = (uint32_t)rslt -> elem_count;
    
    assert(rslt -> elem_count >> 32 == 0);
    /* populate "self" */
    self -> dst . u . u64 = rslt -> base;
    self -> dst . off = ((uint64_t)rslt -> first_elem) * (rslt -> elem_bits>>6);
    for ( j = 0; j < argc; ++ j )
    {
        self -> src [ j ] . u . u64 = argv [ j ] . u . data .base;
        self -> src [ j ] . off = ((uint64_t)argv [ j ] . u . data .first_elem) * (argv [ j ] . u . data .elem_bits >> 6);
        self -> src [ j ] . len = argv [ j ] . u . data .elem_bits >> 6;
    }

    /* create output row */
    for ( i = 0; i < row_len; ++ i )
    {
        for ( j = 0; j < argc; ++ j )
        {
            uint32_t k;
            for ( k = 0; k < self -> src [ j ] . len; ++ k )
            {
                self -> dst . u . u64 [ self -> dst . off + k ] =
                    self -> src [ j ] . u . u64 [ self -> src [ j ] . off + k ];
            }

            self -> dst . off += k;
            self -> src [ j ] . off += k;
        }
    }
    return rc;
}

static
void CC vxf_paste_wrapper( void *ptr )
{
	free( ptr );
}

#include <stdio.h>

/* paste
 * function < type T > T [ * ] paste #1.0 ( T [ * ] in, ... );
 * function any NCBI:paste #1.0 ( any in, ... );
 */
VTRANSFACT_IMPL ( vdb_paste, 1, 0, 0 ) ( const void *self,
    const VXfactInfo *info, VFuncDesc *rslt,
    const VFactoryParams *cp, const VFunctionParams *dp )
{
    pastie *fself;
    uint32_t rtn_bits, min_gcd;

    /* in the case of the modern "paste", the compiler is able to
       enforce only the function signature, but not the essential
       requirement that the output dimension equal the sum of all
       input dimensions. in the case of "NCBI:paste", the compiler
       cannot enforce anything, so we check for inputs that have
       the same domain and intrinsic type-size, already guaranteed
       by the type-checked version. */
    uint32_t i, sum;
    for ( i = sum = 0; i < dp -> argc; ++ i )
    {
        assert ( dp -> argv [ i ] . fd . td . dim != 0 );

        /* handle type-checking for NCBI:paste */
        if ( dp -> argv [ i ] . desc . domain != info -> fdesc . desc . domain ||
             dp -> argv [ i ] . desc . intrinsic_bits != info -> fdesc . desc . intrinsic_bits )
        {
            return RC ( rcXF, rcFunction, rcConstructing, rcType, rcIncorrect );
        }

        sum += dp -> argv [ i ] . desc . intrinsic_dim;
    }

    assert ( info -> fdesc . fd . td . dim != 0 );
    if ( sum > info -> fdesc . desc . intrinsic_dim )
        return RC ( rcXF, rcFunction, rcConstructing, rcArgv, rcExcessive );

    if ( sum < info -> fdesc . desc . intrinsic_dim )
        return RC ( rcXF, rcFunction, rcConstructing, rcArgv, rcInsufficient );

    /* allocate self */
    fself = malloc ( sizeof * fself - sizeof fself -> src + sizeof fself -> src [ 0 ] * dp -> argc );
    if ( fself == NULL )
        return RC ( rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted );

    /* we can create the function */
    rslt -> self = fself;
    rslt -> whack = vxf_paste_wrapper;
    rslt -> variant = vftFixedRow;

    /* the output dimension == sum ( dimension ) for all inputs
       each input dimension will have a GCD of sizeof ( element ) * N
       for N = 1 .. output dimension. the minimum of these will determine
       the common copy word size */
    rtn_bits = VTypedescSizeof ( & info -> fdesc . desc );
    for ( min_gcd = rtn_bits, i = 0; i < dp -> argc; ++ i )
    {
        uint32_t gcd, a, b = VTypedescSizeof ( & dp -> argv [ i ] . desc );
        for ( a = rtn_bits; ; )
        {
            if ( b == 0 )
            {
                gcd = a;
                break;
            }

            a %= b;
            if ( a == 0 )
            {
                gcd = b;
                break;
            }

            b %= a;
        }

        if ( gcd < min_gcd )
            min_gcd = gcd;
    }

    /* select the copy word size */
    if ( ( min_gcd & 63 ) == 0 )
        rslt -> u . pf = paste_func64;
    else if ( ( min_gcd & 31 ) == 0 )
        rslt -> u . pf = paste_func32;
    else if ( ( min_gcd & 15 ) == 0 )
        rslt -> u . pf = paste_func16;
    else if ( ( min_gcd & 7 ) == 0 )
        rslt -> u . pf = paste_func8;
    else
        rslt -> u . pf = paste_func1;

    return 0;
}
