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
#include <vdb/table.h>
#include <vdb/vdb-priv.h>
#include <klib/rc.h>
#include <klib/data-buffer.h>
#include <klib/log.h>
#include <sysalloc.h>
#include <atomic32.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <assert.h>

typedef struct tag_self_t {
    uint8_t     align;
} self_t;

static void CC self_whack( void *Self )
{
    free( Self );
}
static
rc_t CC bit_or_impl(
                 void *Self,
                 const VXformInfo *info,
                 int64_t row_id,
                 VRowResult *rslt,
                 uint32_t argc,
                 const VRowData argv[]
                          )
{                                                                        
    rc_t rc;
    uint32_t a_size,b_size;
    const self_t *self = Self;
    const uint8_t *a = argv[0].u.data.base;
    const uint8_t *b = argv[1].u.data.base;
    uint8_t *dst;

    a += argv[0].u.data.first_elem;
    b += argv[1].u.data.first_elem;

    assert((argv[0].u.data.elem_bits&7)== 0);
    assert(argv[1].u.data.elem_bits==argv[0].u.data.elem_bits);

    rslt->elem_bits = argv[0].u.data.elem_bits;
    
    a_size = argv[0].u.data.elem_count * (argv[0].u.data.elem_bits>>3);
    b_size = argv[1].u.data.elem_count * (argv[1].u.data.elem_bits>>3);
    if(a_size >= b_size){ /**** leaving a > b *****/
	rslt->elem_count = argv[0].u.data.elem_count;
    } else { /*** switching so  a > b ***/
	const uint8_t *t;
	uint32_t       t_size;
	rslt->elem_count = argv[1].u.data.elem_count;
	t=a;a=b;b=t;
	t_size=a_size;a_size=b_size;b_size=t_size;
    } 
    /*** everything a is now the largest ***/
    rc = KDataBufferResize ( rslt->data, a_size );
    if(rc==0){
	dst = rslt -> data -> base;

	if(b_size == 0){/*** trivial shortcut ***/
		memcpy(dst,a,a_size);
		return 0;
	}
	if(self -> align == 1){ /*** left side align ***/
		while(a_size > b_size){
			*dst++ = *a++;
			a_size--;
		}
	}
	while(b_size > 0){
		*dst++ = *a++ | *b++;
		a_size--;
		b_size--;
		
	}
	while(a_size > 0){
		*dst++ = *a++;
                a_size--;
	}
    }
    return 0;
}

VTRANSFACT_IMPL ( vdb_bit_or, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rc_t rc;
    self_t *self;
            
    self = malloc(sizeof(*self));
    if (self) {
	self->align = cp->argv[0].data.u8[0];
	rslt->self = self;
	rslt->whack = self_whack;
	rslt->variant = vftRow;
	rslt->u.rf = bit_or_impl;
	return 0;
    }
    rc = RC(rcVDB, rcFunction, rcConstructing, rcMemory, rcExhausted);
    return rc;
}
