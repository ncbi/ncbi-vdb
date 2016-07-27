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
#include <arch-impl.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


#define INTEGRAL_NAME( T )  integral_ ## T
#define INTEGRAL( T )                                                    \
static                                                                   \
rc_t CC INTEGRAL_NAME ( T ) ( void *data,                                \
    const VXformInfo *info, int64_t row_id, const VFixedRowResult *rslt, \
    uint32_t argc, const VRowData argv [] )                              \
{                                                                        \
    uint32_t i;                                                          \
    T prior, * dst = rslt -> base;                                       \
    const T * src = argv [ 0 ] . u . data . base;                        \
    dst += rslt -> first_elem;						                     \
    src += argv [ 0 ] . u . data . first_elem;				             \
    for ( prior = 0, i = 0; i < rslt -> elem_count; ++ i )               \
    {                                                                    \
        prior += src [ i ];                                              \
        dst [ i ] = prior;                                               \
    }                                                                    \
    return 0;                                                            \
}

INTEGRAL ( int8_t )
INTEGRAL ( int16_t )
INTEGRAL ( int32_t )
INTEGRAL ( int64_t )

static VFixedRowFunc integral_func [] =
{
    INTEGRAL_NAME ( int8_t  ),
    INTEGRAL_NAME ( int16_t ),
    INTEGRAL_NAME ( int32_t ),
    INTEGRAL_NAME ( int64_t )
};
/* integral
 *  return the 1 integral of input
 *
 *  "T" [ TYPE ] - input and output data type
 *  must be member of  signed integers
 *
 *  "a" [ DATA ] - operand
 *
 *
 * SYNOPSIS:
 *  return 1st integral for every row
 *
 * USAGE:
 *    I32 position = < I32 > integral ( pos_1st_d );
 */
VTRANSFACT_IMPL ( vdb_integral, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    int size_idx;
    switch ( info -> fdesc . desc . domain )
    {
    case vtdUint:
    case vtdInt:
        break;
    default:
        return RC ( rcXF, rcFunction, rcConstructing, rcType, rcIncorrect );
    }

    /* TBD - eventually support vector integrals
       for today, check that dim of T is 1 */
    if ( dp -> argv [ 0 ] . desc . intrinsic_dim != 1 )
        return RC ( rcXF, rcFunction, rcConstructing, rcType, rcIncorrect );

    /* the only numeric types we support are between 8 and 64 bits */
    size_idx = uint32_lsbit ( dp -> argv [ 0 ] . desc . intrinsic_bits ) - 3;
    if ( size_idx < 0 || size_idx > 3 || ( ( dp -> argv [ 0 ] . desc . intrinsic_bits &
                                             ( dp -> argv [ 0 ] . desc . intrinsic_bits - 1 ) ) != 0 ) )
        return RC ( rcXF, rcFunction, rcConstructing, rcType, rcIncorrect );


    rslt -> u . pf = integral_func [ size_idx ];
    rslt -> variant = vftFixedRow;

    return 0;
}
