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
#include <klib/data-buffer.h>
#include <klib/text.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

static
rc_t CC template_len_impl( void * data, const VXformInfo * info, int64_t row_id,
                           VRowResult *rslt, uint32_t argc, const VRowData argv [] ) 
{
    rc_t 	rc;
    int32_t	*dst;
    const INSDC_coord_zero *p1 = argv[ 0 ].u.data.base;     /* REF_POS */
    const INSDC_coord_zero *p2 = argv[ 1 ].u.data.base;     /* MATE_REF_POS */
    const INSDC_coord_len  *l1 = argv[ 2 ].u.data.base;     /* REF_LEN */
    const INSDC_coord_len  *l2 = argv[ 3 ].u.data.base;     /* MATE_REF_LEN */
    const char             *n1 = argv[ 4 ].u.data.base;     /* REF_NAME */
    const char             *n2 = argv[ 5 ].u.data.base;     /* MATE_REF_NAME */
    const INSDC_coord_one  *r1 = argv[ 6 ].u.data.base;     /* SEQ_READ_ID ( 1 or 2 ) */

    /** primary should have everything defines ***/
    assert( argv[ 0 ].u.data.elem_count == 1 );
    assert( argv[ 2 ].u.data.elem_count == 1 );
    assert( argv[ 4 ].u.data.elem_count > 0 );
    assert( argv[ 6 ].u.data.elem_count == 1 );

    rc = KDataBufferResize( rslt->data, 1 );
    if ( rc != 0 )
        return rc;
    rslt->elem_count = 1;
    dst = rslt->data->base;
    dst[ 0 ] = 0;

    /*** check if the mate is mapped **/
    if ( argv[ 1 ].u.data.elem_count > 0
         && argv[ 3 ].u.data.elem_count > 0
         && argv[ 4 ].u.data.elem_count == argv[ 5 ].u.data.elem_count )
    {
        n1 += argv[ 4 ].u.data.first_elem;
        n2 += argv[ 5 ].u.data.first_elem;
        r1 += argv[ 6 ].u.data.first_elem;
        
        if ( argv[ 4 ].u.data.elem_count == argv[ 5 ].u.data.elem_count
             && memcmp( n1, n2, ( size_t )( argv[ 4 ].u.data.elem_count ) ) == 0 ) /*** same reference name ***/
        {
#if 1
            unsigned const self_left  = p1[ argv[ 0 ].u.data.first_elem ];
            unsigned const mate_left  = p2[ argv[ 1 ].u.data.first_elem ];
            unsigned const self_right = self_left + l1[ argv[ 2 ].u.data.first_elem ];
            unsigned const mate_right = mate_left + l2[ argv[ 3 ].u.data.first_elem ];
            unsigned const  leftmost  = ( self_left  < mate_left ) ? self_left  : mate_left;
            unsigned const rightmost  = ( self_right > mate_right ) ? self_right : mate_right;
            unsigned const tlen = rightmost - leftmost;
            
            /* The standard says, "The leftmost segment has a plus sign and the rightmost has a minus sign." */
            if (   ( self_left <= mate_left && self_right >= mate_right )     /* mate fully contained within self or */
                || ( mate_left <= self_left && mate_right >= self_right ) )    /* self fully contained within mate; */
            {
                if ( self_left < mate_left || ( r1[ 0 ] == 1 && self_left == mate_left ) )
                    dst[ 0 ] = tlen;
                else
                    dst[ 0 ] = -( ( int32_t )tlen );
            }
            else if (   ( self_right == mate_right && mate_left == leftmost ) /* both are rightmost, but mate is leftmost */
                      || self_right == rightmost )
            {
                dst[ 0 ] = -( ( int32_t )tlen );
            }
            else
                dst[ 0 ] = tlen;
#else
            dst[ 0 ] = p2[ argv[ 1 ].u.data.first_elem ] -  p1[ argv[ 0 ].u.data.first_elem ];
            if( dst[ 0 ] < 0 )
            {
                dst[ 0 ] -= l1[ argv[ 2 ].u.data.first_elem ];
            }
            else if ( dst[ 0 ] > 0 )
            {
                dst[ 0 ] += l2[ argv[ 3 ].u.data.first_elem ];
            }
            else if ( r1[ 0 ] == 1 ) /*** bam wants to give positive TLEN ***/
            {
                dst[ 0 ] = l1[ argv[ 2 ].u.data.first_elem ];
            }
            else if ( r1[ 0 ] == 2 ) /*** bam wants to give negative TLEN ***/
            {
                dst[ 0 ] = -l2[ argv[ 3 ].u.data.first_elem ];
            } 
#endif
        }
    }
    return 0;	
}

VTRANSFACT_IMPL ( NCBI_align_template_len, 1, 0, 0 ) ( const void *self, const VXfactInfo *info, VFuncDesc *rslt,
                                                 const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->variant = vftRowFast;
    rslt->u.rf = template_len_impl;
    return 0;
}
