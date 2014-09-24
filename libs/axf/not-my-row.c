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
rc_t CC not_my_row_impl( void * data, const VXformInfo * info, int64_t row_id,
                         VRowResult * rslt, uint32_t argc, const VRowData argv [] ) 
{
	rc_t 	rc;
	const int64_t* src = argv[ 0 ].u.data.base;
	uint32_t len = ( uint32_t )argv[ 0 ].u.data.elem_count;
	
	src += argv[ 0 ].u.data.first_elem;
	assert( argv[ 0 ].u.data.elem_bits == 64 );
	rc = KDataBufferResize( rslt->data, len );
	if( rc == 0 )
    {
		int64_t *dst;
		int i, j;
		dst = rslt->data->base;
		for( i = j = 0; i < ( int )len; i++ )
        {
			if ( src[ i ] != row_id && src[i] > 0 ) /**we used alignment is as 0 to indicate no alignmnent**/
            {
				dst[ j++ ] = src[ i ];
			}
		}
		rslt->elem_count = j;
	}
	return rc;	
}

VTRANSFACT_IMPL ( NCBI_align_not_my_row, 1, 0, 0 ) ( const void *self, const VXfactInfo *info, VFuncDesc *rslt,
                                                 const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->variant = vftIdDepRow; /*** row_id influences behavior of the function ***/ 
    rslt->u.ndf = not_my_row_impl;
    return 0;
}
