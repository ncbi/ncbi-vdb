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
#include <klib/rc.h>

#include "xform-priv.h"
#include "blob-priv.h"
#include "blob.h"
#include "page-map.h"

#include <assert.h>


/* Create
 *
 * Create a new blob.  A new data buffer is sized to
 * 'element_count' * sizeof datatype; its contents are
 * uninitialized.
 */
static
rc_t VBlobCreate ( VBlob **blobp, const VBlob *in )
{
    rc_t rc = VBlobNew ( blobp, in -> start_id, in -> stop_id, "transpose" );
    if ( rc == 0 )
    {
        VBlob *blob = * blobp;
        rc = KDataBufferMake ( & blob -> data,
            in -> data . elem_bits, in -> data . elem_count );
        if ( rc != 0 )
        {
            VBlobRelease ( blob );
            * blobp = NULL;
        }
        else
        {
            blob -> byte_order = in -> byte_order;
        }
    }
    return rc;
}


/* detranspose
 *  pardoning the awful name, apply a transposition on the result
 *  of "transpose" to produce the original blob. "transpose"
 *  itself cannot be reused because of its signature.
 *
 *  see "transpose" below for an explanation
 */
static
rc_t CC transpose_formatted8 ( void *self, const VXformInfo *info, int64_t row_id,
    VBlob **rslt, uint32_t argc, const VBlob *argv [] )
{
    /* input blob - schema assures us that it is ready to go */
    const VBlob *in = argv [ 0 ];

    /* allocate output blob of same size */
    rc_t rc = VBlobCreate ( rslt, in );
    if ( rc != 0 )
        rc = ResetRCContext ( rc, rcVDB, rcFunction, rcExecuting );
    else
    {
        VBlob *out = * rslt;

        /* page-map iterator */
        PageMapIterator it;
        rc = PageMapNewIterator ( in -> pm, & it, 0, -1 );

        /* create a new reference to the page map */
        if ( rc == 0 )
            rc = PageMapAddRef ( in -> pm );

        /* last chance for failure */
        if ( rc != 0 )
        {
            rc = ResetRCContext ( rc, rcVDB, rcFunction, rcExecuting );
            VBlobRelease ( out );
            * rslt = NULL;
        }
        else
        {
            uint32_t j, k;
            elem_count_t i, max_row_len, row_len, rpt_cnt;

            /* get source and destination pointers */
            uint8_t *dst = out -> data . base;
            const uint8_t *src = in -> data . base;

            /* set page-map on out */
            out -> pm = in -> pm;

            /* perform the transformation
               the first pass will determine maximum row length */
            for ( max_row_len = 0, j = k = 0; ;  k += row_len )
            {
                row_len = PageMapIteratorDataLength ( & it );
                if ( row_len > max_row_len )
                    max_row_len = row_len;

                if ( row_len != 0 )
                {
                    dst [ k ] = src [ j ];
                    ++ j;
                }

                rpt_cnt = PageMapIteratorRepeatCount ( & it );
                if ( ! PageMapIteratorAdvance ( & it, rpt_cnt ) )
                    break;
            }

            /* perform transformation on the remainder */
            for ( i = 1; i < max_row_len; ++ i )
            {
                it . cur_row = 0;
                for ( k = i; ; k += row_len )
                {
                    row_len = PageMapIteratorDataLength ( & it );
                    if ( row_len > i )
                    {
                        dst [ k ] = src [ j ];
                        ++ j;
                    }

                    rpt_cnt = PageMapIteratorRepeatCount ( & it );
                    if ( ! PageMapIteratorAdvance ( & it, rpt_cnt ) )
                        break;
                }
            }
        }
    }

    return rc;
}

static
rc_t CC transpose_formatted16 ( void *self, const VXformInfo *info, int64_t row_id,
    VBlob **rslt, uint32_t argc, const VBlob *argv [] )
{
    const VBlob *in = argv [ 0 ];
    rc_t rc = VBlobCreate ( rslt, in );
    if ( rc != 0 )
        rc = ResetRCContext ( rc, rcVDB, rcFunction, rcExecuting );
    else
    {
        VBlob *out = * rslt;
        PageMapIterator it;
        rc = PageMapNewIterator ( in -> pm, & it, 0, -1 );
        if ( rc == 0 )
            rc = PageMapAddRef ( in -> pm );
        if ( rc != 0 )
        {
            rc = ResetRCContext ( rc, rcVDB, rcFunction, rcExecuting );
            VBlobRelease ( out );
            * rslt = NULL;
        }
        else
        {
            uint32_t j, k;
            elem_count_t i, max_row_len, row_len, rpt_cnt;
            uint16_t *dst = out -> data . base;
            const uint16_t *src = in -> data . base;
            out -> pm = in -> pm;
            for ( max_row_len = 0, j = k = 0; ;  k += row_len )
            {
                row_len = PageMapIteratorDataLength ( & it );
                if ( row_len > max_row_len )
                    max_row_len = row_len;
                if ( row_len != 0 )
                {
                    dst [ k ] = src [ j ];
                    ++ j;
                }
                rpt_cnt = PageMapIteratorRepeatCount ( & it );
                if ( ! PageMapIteratorAdvance ( & it, rpt_cnt ) )
                    break;
            }
            for ( i = 1; i < max_row_len; ++ i )
            {
                it . cur_row = 0;
                for ( k = i; ; k += row_len )
                {
                    row_len = PageMapIteratorDataLength ( & it );
                    if ( row_len > i )
                    {
                        dst [ k ] = src [ j ];
                        ++ j;
                    }
                    rpt_cnt = PageMapIteratorRepeatCount ( & it );
                    if ( ! PageMapIteratorAdvance ( & it, rpt_cnt ) )
                        break;
                }
            }
        }
    }

    return rc;
}

static
rc_t CC transpose_formatted32 ( void *self, const VXformInfo *info, int64_t row_id,
    VBlob **rslt, uint32_t argc, const VBlob *argv [] )
{
    const VBlob *in = argv [ 0 ];
    rc_t rc = VBlobCreate ( rslt, in );
    if ( rc != 0 )
        rc = ResetRCContext ( rc, rcVDB, rcFunction, rcExecuting );
    else
    {
        VBlob *out = * rslt;
        PageMapIterator it;
        rc = PageMapNewIterator ( in -> pm, & it, 0, -1 );
        if ( rc == 0 )
            rc = PageMapAddRef ( in -> pm );
        if ( rc != 0 )
        {
            rc = ResetRCContext ( rc, rcVDB, rcFunction, rcExecuting );
            VBlobRelease ( out );
            * rslt = NULL;
        }
        else
        {
            uint32_t j, k;
            elem_count_t i, max_row_len, row_len, rpt_cnt;
            uint32_t *dst = out -> data . base;
            const uint32_t *src = in -> data . base;
            out -> pm = in -> pm;
            for ( max_row_len = 0, j = k = 0; ;  k += row_len )
            {
                row_len = PageMapIteratorDataLength ( & it );
                if ( row_len > max_row_len )
                    max_row_len = row_len;
                if ( row_len != 0 )
                {
                    dst [ k ] = src [ j ];
                    ++ j;
                }
                rpt_cnt = PageMapIteratorRepeatCount ( & it );
                if ( ! PageMapIteratorAdvance ( & it, rpt_cnt ) )
                    break;
            }
            for ( i = 1; i < max_row_len; ++ i )
            {
                it . cur_row = 0;
                for ( k = i; ; k += row_len )
                {
                    row_len = PageMapIteratorDataLength ( & it );
                    if ( row_len > i )
                    {
                        dst [ k ] = src [ j ];
                        ++ j;
                    }
                    rpt_cnt = PageMapIteratorRepeatCount ( & it );
                    if ( ! PageMapIteratorAdvance ( & it, rpt_cnt ) )
                        break;
                }
            }
        }
    }

    return rc;
}

static
rc_t CC transpose_formatted64 ( void *self, const VXformInfo *info, int64_t row_id,
    VBlob **rslt, uint32_t argc, const VBlob *argv [] )
{
    const VBlob *in = argv [ 0 ];
    rc_t rc = VBlobCreate ( rslt, in );
    if ( rc != 0 )
        rc = ResetRCContext ( rc, rcVDB, rcFunction, rcExecuting );
    else
    {
        VBlob *out = * rslt;
        PageMapIterator it;
        rc = PageMapNewIterator ( in -> pm, & it, 0, -1 );
        if ( rc == 0 )
            rc = PageMapAddRef ( in -> pm );
        if ( rc != 0 )
        {
            rc = ResetRCContext ( rc, rcVDB, rcFunction, rcExecuting );
            VBlobRelease ( out );
            * rslt = NULL;
        }
        else
        {
            uint32_t j, k;
            elem_count_t i, max_row_len, row_len, rpt_cnt;
            uint64_t *dst = out -> data . base;
            const uint64_t *src = in -> data . base;
            out -> pm = in -> pm;
            for ( max_row_len = 0, j = k = 0; ;  k += row_len )
            {
                row_len = PageMapIteratorDataLength ( & it );
                if ( row_len > max_row_len )
                    max_row_len = row_len;
                if ( row_len != 0 )
                {
                    dst [ k ] = src [ j ];
                    ++ j;
                }
                rpt_cnt = PageMapIteratorRepeatCount ( & it );
                if ( ! PageMapIteratorAdvance ( & it, rpt_cnt ) )
                    break;
            }
            for ( i = 1; i < max_row_len; ++ i )
            {
                it . cur_row = 0;
                for ( k = i; ; k += row_len )
                {
                    row_len = PageMapIteratorDataLength ( & it );
                    if ( row_len > i )
                    {
                        dst [ k ] = src [ j ];
                        ++ j;
                    }
                    rpt_cnt = PageMapIteratorRepeatCount ( & it );
                    if ( ! PageMapIteratorAdvance ( & it, rpt_cnt ) )
                        break;
                }
            }
        }
    }

    return rc;
}

VTRANSFACT_BUILTIN_IMPL ( vdb_detranspose, 1, 0, 0 )
    ( const void *self, const VXfactInfo *info, VFuncDesc *rslt,
      const VFactoryParams *cp, const VFunctionParams *dp )
{
    switch ( VTypedescSizeof ( & info -> fdesc . desc ) )
    {
    case 8:
        VFUNCDESC_INTERNAL_FUNCS ( rslt ) -> bfN = transpose_formatted8;
        break;
    case 16:
        VFUNCDESC_INTERNAL_FUNCS ( rslt ) -> bfN = transpose_formatted16;
        break;
    case 32:
        VFUNCDESC_INTERNAL_FUNCS ( rslt ) -> bfN = transpose_formatted32;
        break;
    case 64:
        VFUNCDESC_INTERNAL_FUNCS ( rslt ) -> bfN = transpose_formatted64;
        break;
    }

    rslt -> variant = vftBlobN;
    return 0;
}

/* transpose
 *  transpose a page of unformatted data
 *
 *  for example - convert a simple page of values,
 *  where vertical scale is row id and horizontal element index:
 *
 *        1   2   3
 *      +---+---+---+
 *    1 | a | b | c |
 *      +---+---+---+
 *    2 | d | e |
 *      +---+---+
 *    3 | f |
 *      +---+---+---+
 *    4 | g | h | i |
 *      +---+---+---+
 *
 *  into:
 *
 *        1   2   3   4
 *      +---+---+---+---+
 *    1 | a | d | f | g |
 *      +---+---+---+---+
 *    2 | b | e |   | h |
 *      +---+---+   +---+
 *    3 | c |       | i |
 *      +---+       +---+
 *
 *  or:
 *
 *      +---+---+---+---+---+---+---+---+---+
 *      | a | d | f | g | b | e | h | c | i |
 *      +---+---+---+---+---+---+---+---+---+
 *
 *  variable row-lengths are supported. The output blob is
 *  formatted, meaning that the result can no longer be addressed
 *  as a matrix, but the transposition has be applied to data.
 *
 *  "in" [ DATA ] - unformatted data to be transposed
 */
static
rc_t CC transpose_unformatted8 ( void *self, const VXformInfo *info, int64_t row_id,
    VBlob **rslt, uint32_t argc, const VBlob *argv [] )
{
    /* input blob - schema assures us that it is ready to go */
    const VBlob *in = argv [ 0 ];

    /* allocate output blob of same size */
    rc_t rc = VBlobCreate ( rslt, in );
    if ( rc != 0 )
        rc = ResetRCContext ( rc, rcVDB, rcFunction, rcExecuting );
    else
    {
        VBlob *out = * rslt;

        /* page-map iterator */
        PageMapIterator it;
        rc = PageMapNewIterator ( in -> pm, & it, 0, -1 );

        /* create a new reference to the page map */
        if ( rc == 0 )
            rc = PageMapAddRef ( in -> pm );

        /* last chance for failure */
        if ( rc != 0 )
        {
            rc = ResetRCContext ( rc, rcVDB, rcFunction, rcExecuting );
            VBlobRelease ( out );
            * rslt = NULL;
        }
        else
        {
            uint32_t j, k;
            elem_count_t i, max_row_len, row_len, rpt_cnt;

            /* get source and destination pointers */
            uint8_t *dst = out -> data . base;
            const uint8_t *src = in -> data . base;

            /* set page-map on out */
            out -> pm = in -> pm;

            /* perform the transformation
               the first pass will determine maximum row length */
            for ( max_row_len = 0, j = k = 0; ;  k += row_len )
            {
                row_len = PageMapIteratorDataLength ( & it );
                if ( row_len > max_row_len )
                    max_row_len = row_len;

                if ( row_len != 0 )
                {
                    dst [ j ] = src [ k ];
                    ++ j;
                }

                rpt_cnt = PageMapIteratorRepeatCount ( & it );
                if ( ! PageMapIteratorAdvance ( & it, rpt_cnt ) )
                    break;
            }

            /* perform transformation on the remainder */
            for ( i = 1; i < max_row_len; ++ i )
            {
                it . cur_row = 0;
                for ( k = i; ; k += row_len )
                {
                    row_len = PageMapIteratorDataLength ( & it );
                    if ( row_len > i )
                    {
                        dst [ j ] = src [ k ];
                        ++ j;
                    }

                    rpt_cnt = PageMapIteratorRepeatCount ( & it );
                    if ( ! PageMapIteratorAdvance ( & it, rpt_cnt ) )
                        break;
                }
            }
        }
    }

    return rc;
}

static
rc_t CC transpose_unformatted16 ( void *self, const VXformInfo *info, int64_t row_id,
    VBlob **rslt, uint32_t argc, const VBlob *argv [] )
{
    const VBlob *in = argv [ 0 ];
    rc_t rc = VBlobCreate ( rslt, in );
    if ( rc != 0 )
        rc = ResetRCContext ( rc, rcVDB, rcFunction, rcExecuting );
    else
    {
        VBlob *out = * rslt;
        PageMapIterator it;
        rc = PageMapNewIterator ( in -> pm, & it, 0, -1 );
        if ( rc == 0 )
            rc = PageMapAddRef ( in -> pm );
        if ( rc != 0 )
        {
            rc = ResetRCContext ( rc, rcVDB, rcFunction, rcExecuting );
            VBlobRelease ( out );
            * rslt = NULL;
        }
        else
        {
            uint32_t j, k;
            elem_count_t i, max_row_len, row_len, rpt_cnt;
            uint16_t *dst = out -> data . base;
            const uint16_t *src = in -> data . base;
            out -> pm = in -> pm;
            for ( max_row_len = 0, j = k = 0; ;  k += row_len )
            {
                row_len = PageMapIteratorDataLength ( & it );
                if ( row_len > max_row_len )
                    max_row_len = row_len;
                if ( row_len != 0 )
                {
                    dst [ j ] = src [ k ];
                    ++ j;
                }
                rpt_cnt = PageMapIteratorRepeatCount ( & it );
                if ( ! PageMapIteratorAdvance ( & it, rpt_cnt ) )
                    break;
            }
            for ( i = 1; i < max_row_len; ++ i )
            {
                it . cur_row = 0;
                for ( k = i; ; k += row_len )
                {
                    row_len = PageMapIteratorDataLength ( & it );
                    if ( row_len > i )
                    {
                        dst [ j ] = src [ k ];
                        ++ j;
                    }
                    rpt_cnt = PageMapIteratorRepeatCount ( & it );
                    if ( ! PageMapIteratorAdvance ( & it, rpt_cnt ) )
                        break;
                }
            }
        }
    }

    return rc;
}

static
rc_t CC transpose_unformatted32 ( void *self, const VXformInfo *info, int64_t row_id,
    VBlob **rslt, uint32_t argc, const VBlob *argv [] )
{
    const VBlob *in = argv [ 0 ];
    rc_t rc = VBlobCreate ( rslt, in );
    if ( rc != 0 )
        rc = ResetRCContext ( rc, rcVDB, rcFunction, rcExecuting );
    else
    {
        VBlob *out = * rslt;
        PageMapIterator it;
        rc = PageMapNewIterator ( in -> pm, & it, 0, -1 );
        if ( rc == 0 )
            rc = PageMapAddRef ( in -> pm );
        if ( rc != 0 )
        {
            rc = ResetRCContext ( rc, rcVDB, rcFunction, rcExecuting );
            VBlobRelease ( out );
            * rslt = NULL;
        }
        else
        {
            uint32_t j, k;
            elem_count_t i, max_row_len, row_len, rpt_cnt;
            uint32_t *dst = out -> data . base;
            const uint32_t *src = in -> data . base;
            out -> pm = in -> pm;
            for ( max_row_len = 0, j = k = 0; ;  k += row_len )
            {
                row_len = PageMapIteratorDataLength ( & it );
                if ( row_len > max_row_len )
                    max_row_len = row_len;
                if ( row_len != 0 )
                {
                    dst [ j ] = src [ k ];
                    ++ j;
                }
                rpt_cnt = PageMapIteratorRepeatCount ( & it );
                if ( ! PageMapIteratorAdvance ( & it, rpt_cnt ) )
                    break;
            }
            for ( i = 1; i < max_row_len; ++ i )
            {
                it . cur_row = 0;
                for ( k = i; ; k += row_len )
                {
                    row_len = PageMapIteratorDataLength ( & it );
                    if ( row_len > i )
                    {
                        dst [ j ] = src [ k ];
                        ++ j;
                    }
                    rpt_cnt = PageMapIteratorRepeatCount ( & it );
                    if ( ! PageMapIteratorAdvance ( & it, rpt_cnt ) )
                        break;
                }
            }
        }
    }

    return rc;
}

static
rc_t CC transpose_unformatted64 ( void *self, const VXformInfo *info, int64_t row_id,
    VBlob **rslt, uint32_t argc, const VBlob *argv [] )
{
    const VBlob *in = argv [ 0 ];
    rc_t rc = VBlobCreate ( rslt, in );
    if ( rc != 0 )
        rc = ResetRCContext ( rc, rcVDB, rcFunction, rcExecuting );
    else
    {
        VBlob *out = * rslt;
        PageMapIterator it;
        rc = PageMapNewIterator ( in -> pm, & it, 0, -1 );
        if ( rc == 0 )
            rc = PageMapAddRef ( in -> pm );
        if ( rc != 0 )
        {
            rc = ResetRCContext ( rc, rcVDB, rcFunction, rcExecuting );
            VBlobRelease ( out );
            * rslt = NULL;
        }
        else
        {
            uint32_t j, k;
            elem_count_t i, max_row_len, row_len, rpt_cnt;
            uint64_t *dst = out -> data . base;
            const uint64_t *src = in -> data . base;
            out -> pm = in -> pm;
            for ( max_row_len = 0, j = k = 0; ;  k += row_len )
            {
                row_len = PageMapIteratorDataLength ( & it );
                if ( row_len > max_row_len )
                    max_row_len = row_len;
                if ( row_len != 0 )
                {
                    dst [ j ] = src [ k ];
                    ++ j;
                }
                rpt_cnt = PageMapIteratorRepeatCount ( & it );
                if ( ! PageMapIteratorAdvance ( & it, rpt_cnt ) )
                    break;
            }
            for ( i = 1; i < max_row_len; ++ i )
            {
                it . cur_row = 0;
                for ( k = i; ; k += row_len )
                {
                    row_len = PageMapIteratorDataLength ( & it );
                    if ( row_len > i )
                    {
                        dst [ j ] = src [ k ];
                        ++ j;
                    }
                    rpt_cnt = PageMapIteratorRepeatCount ( & it );
                    if ( ! PageMapIteratorAdvance ( & it, rpt_cnt ) )
                        break;
                }
            }
        }
    }

    return rc;
}


VTRANSFACT_BUILTIN_IMPL ( vdb_transpose, 1, 0, 0 )
    ( const void *self, const VXfactInfo *info, VFuncDesc *rslt,
      const VFactoryParams *cp, const VFunctionParams *dp )
{
    /* the TYPE of input has been validated by schema, but not the dimension */
    switch ( VTypedescSizeof ( & dp -> argv [ 0 ] . desc ) )
    {
    case 8:
        VFUNCDESC_INTERNAL_FUNCS ( rslt ) -> bfN = transpose_unformatted8;
        break;
    case 16:
        VFUNCDESC_INTERNAL_FUNCS ( rslt ) -> bfN = transpose_unformatted16;
        break;
    case 32:
        VFUNCDESC_INTERNAL_FUNCS ( rslt ) -> bfN = transpose_unformatted32;
        break;
    case 64:
        VFUNCDESC_INTERNAL_FUNCS ( rslt ) -> bfN = transpose_unformatted64;
        break;
    default:
        return RC ( rcVDB, rcFunction, rcConstructing, rcType, rcUnsupported );
    }

    rslt -> variant = vftBlobN;
    return 0;
}
