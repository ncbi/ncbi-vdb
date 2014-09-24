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


#define DELTA_NAME( T )  delta_ ## T
#define DELTA( T )                                                       \
static                                                                   \
rc_t CC DELTA_NAME ( T ) ( void *data,                                   \
    const VXformInfo *info,  void *rslt, const void *input,              \
    uint64_t elem_count)                                                 \
{                                                                        \
    uint32_t i;                                                          \
    T *dst       = (T*) rslt;                                            \
    const T *src = (const T*) input;                                     \
    if(elem_count >0) {							 \
        dst [ 0 ] = src [ 0 ];						 \
	for ( i = 1; i < elem_count; ++ i ) {                            \
	    dst [ i ] = src [ i ] - src [ i - 1 ];                       \
	}                                                                \
    }									 \
    return 0;                                                            \
}

DELTA ( int8_t )
DELTA ( int16_t )
DELTA ( int32_t )
DELTA ( int64_t )

static VArrayFunc delta_func [] =
{
    DELTA_NAME ( int8_t  ),
    DELTA_NAME ( int16_t ),
    DELTA_NAME ( int32_t ),
    DELTA_NAME ( int64_t )
};
/* delta
 *  return the 1 derivative of inputs
 *
 *  "T" [ TYPE ] - input and output data type
 *  must be member of  signed integers
 *
 *  "a" [ DATA ] - operand
 *
 *
 * SYNOPSIS:
 *  return 1st derivative for the whole blob with 1st element unmodified
 *
 * USAGE:
 *    I32 A = < I32 > delta ( B );
 */
VTRANSFACT_IMPL ( vdb_delta, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    int size_idx;
    if ( info -> fdesc . desc . domain != vtdInt)
    {
        return RC ( rcXF, rcFunction, rcConstructing, rcType, rcIncorrect );
    }

    /* TBD - eventually support vector derivatives
       for today, check that dim of T is 1 */
    if ( dp -> argv [ 0 ] . desc . intrinsic_dim != 1 )
        return RC ( rcXF, rcFunction, rcConstructing, rcType, rcIncorrect );

    /* the only numeric types we support are between 8 and 64 bits */
    size_idx = uint32_lsbit ( dp -> argv [ 0 ] . desc . intrinsic_bits ) - 3;
    if ( size_idx < 0 || size_idx > 3 || ( ( dp -> argv [ 0 ] . desc . intrinsic_bits &
                                             ( dp -> argv [ 0 ] . desc . intrinsic_bits - 1 ) ) != 0 ) )
        return RC ( rcXF, rcFunction, rcConstructing, rcType, rcIncorrect );


    rslt -> u . af = delta_func [ size_idx ];
    rslt -> variant = vftArray;

    return 0;
}
