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

#include <insdc/sra.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>


/* "properly aligned" fragments according to the aligner
   may mean minimally that the fragments are on the same
   chromosome. */
#define PROPER_ALIGNED_MEANS_SAME_CHROMOSOME 1


static
rc_t CC get_sam_flags_impl(void *data, const VXformInfo *info,
                           int64_t row_id, VRowResult *rslt,
                           uint32_t nreads,
                           uint32_t argc, const VRowData argv [] ) 
{
    rc_t 	rc;
    int32_t	*dst;
    const INSDC_coord_one  *rid = argv[ 1 ].u.data.base; /* SEQ_READ_ID */
    const int32_t         *tlen = argv[ 2 ].u.data.base; /* TEMPLATE_LEN */
    const bool	           *ro1 = argv[ 3 ].u.data.base; /* REF_ORIENTATION */
    const bool             *ro2 = argv[ 4 ].u.data.base; /* MATE_REF_ORIENTATION */
    const bool             *sec = argv[ 5 ].u.data.base; 
    const bool             mate_present = ( argv[ 4 ].u.data.elem_count > 0 );
    const SRAReadFilter    *flt = argc > 6 ? argv[ 6 ].u.data.base : NULL;

    assert( argv[ 1 ].u.data.elem_count == 1 );
    assert( argv[ 2 ].u.data.elem_count == 1 );
    assert( argv[ 3 ].u.data.elem_count == 1 );
    assert( argv[ 5 ].u.data.elem_count == 1 );
    
    rc = KDataBufferResize( rslt->data, 1 );
    if( rc != 0 )
        return rc;
    rslt->elem_count=1;
    dst = rslt->data->base;
    dst[ 0 ] = 0;
    if( nreads == 0 )
        return 0;
    
    rid  += argv[ 1 ].u.data.first_elem;
    tlen += argv[ 2 ].u.data.first_elem;
    ro1  += argv[ 3 ].u.data.first_elem;
    ro2  += argv[ 4 ].u.data.first_elem;
    sec  += argv[ 5 ].u.data.first_elem;
    if ( flt != NULL )
        flt += argv[ 6 ].u.data.first_elem;
    
    /***************** SAM FLAGS************
      Bit  Description
     0x001 template having multiple fragments in sequencing
     0x002 each fragment properly aligned according to the aligner
     0x004 fragment unmapped
     0x008 next fragment in the template unmapped
     0x010 SEQ being reverse complemented
     0x020 SEQ of the next fragment in the template being reversed
     0x040 the first fragment in the template
     0x080 the last fragment in the template
     0x100 secondary alignment
     0x200 not passing quality controls
     0x400 PCR or optical duplicate
    ****************************/
    if ( ro1[ 0 ] )
        dst[ 0 ] |= 0x10;

    if ( sec[ 0 ] )
        dst[ 0 ] |= 0x100;

    if ( nreads > 1 )
    {
        if ( rid[ 0 ] == 1 )
            dst[ 0 ] |= 0x40;

        if ( rid[ 0 ] == nreads )
            dst[ 0 ] |= 0x80;

        dst[ 0 ] |= 0x1;

        if( mate_present )
        {
#if PROPER_ALIGNED_MEANS_SAME_CHROMOSOME
           if ( tlen[ 0 ] != 0 )
#endif
                dst[ 0 ] |= 0x2;
           if ( ro2 [ 0 ] )
                dst[ 0 ] |= 0x20;
        }
        else
        {
            dst[ 0 ] |= 0x8;
        }
    }

    if ( flt != NULL )
    {
        if ( flt[ 0 ] == SRA_READ_FILTER_REJECT )
        {
            dst[ 0 ] |= 0x200;
        }
        else if ( flt[ 0 ] == SRA_READ_FILTER_CRITERIA )
        {
            dst[ 0 ] |= 0x400;
        }
    }
    return rc;	
}


static
rc_t CC get_sam_flags_impl_v1(  void  *data, const VXformInfo * info, int64_t row_id,
                                VRowResult * rslt, uint32_t argc, const VRowData argv [] ) 
{
	uint32_t nreads = 0;
    INSDC_coord_len const *read_len = argv[ 0 ].u.data.base;
    unsigned i;
    
    assert( argv[ 0 ].u.data.elem_bits == sizeof( read_len[ 0 ] ) * 8 );
    read_len += argv[ 0 ].u.data.first_elem;
   
#if 1 
    for ( i = 0, nreads = 0; i != argv[ 0 ].u.data.elem_count; ++i )
    {
        if ( read_len[ i ] != 0 )
            ++nreads;
    }
#else
    nreads = argv[ 0 ].u.data.elem_count;
#endif

    return get_sam_flags_impl( data, info, row_id, rslt, nreads, argc, argv );
}


static
rc_t CC get_sam_flags_impl_v2( void * data,const VXformInfo * info, int64_t row_id,
                               VRowResult * rslt, uint32_t argc, const VRowData argv [] ) 
{
	uint32_t nreads = 1;
    int64_t const *mate_id = argv[ 0 ].u.data.base;
    unsigned i;
    
    assert( argv[ 0 ].u.data.elem_bits == sizeof( mate_id[ 0 ] ) * 8 );
    mate_id += argv[ 0 ].u.data.first_elem;
    
    for ( i = 0; i != argv[ 0 ].u.data.elem_count; ++i )
    {
        if ( mate_id[ i ] != 0 )
            ++nreads;
    }

    return get_sam_flags_impl( data, info, row_id, rslt, nreads, argc, argv );
}


/*
 * extern function U32 NCBI:align:get_sam_flags #1 (INSDC:coord:len read_len,
 *                                                  INSDC:coord:one read_id,
 *                                                  I32 template_len,
 *                                                  bool strand,
 *                                                  bool mate_strand,
 *                                                  bool is_secondary);
 *
 */
VTRANSFACT_IMPL ( NCBI_align_get_sam_flags, 1, 0, 0 ) ( const void *self, const VXfactInfo *info, VFuncDesc *rslt,
                                                 const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->variant = vftRow;
    rslt->u.rf = get_sam_flags_impl_v1;
    return 0;
}


/*
 * extern function U32 NCBI:align:get_sam_flags #2 (I64 mate_id,
 *                                                  INSDC:coord:one read_id,
 *                                                  I32 template_len,
 *                                                  bool strand,
 *                                                  bool mate_strand,
 *                                                  bool is_secondary);
 *
 */
VTRANSFACT_IMPL ( NCBI_align_get_sam_flags_2, 2, 0, 0 ) ( const void *self, const VXfactInfo *info, VFuncDesc *rslt,
                                                       const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->variant = vftRow;
    rslt->u.rf = get_sam_flags_impl_v2;
    return 0;
}
