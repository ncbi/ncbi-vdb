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
#include <arch-impl.h>


#include <sysalloc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct trim_self_struct {
	union {
	    int8_t i8;
	    int16_t i16;
	    int32_t i32;
	    int64_t i64;

	    uint8_t u8;
	    uint16_t u16;
	    uint32_t u32;
	    uint64_t u64;

	    float f32;
	    double f64;
	} el;
	uint8_t side;
} self_t;


#define TRIM_NAME( T ) trim_ ## T

#define FUNC_TRIM( T, K )                                               \
static rc_t CC TRIM_NAME ( T ) ( void *data,                            \
    const VXformInfo *info, int64_t row_id, VRowResult *rslt,           \
    uint32_t argc, const VRowData argv [] )                             \
{                                                                       \
    rc_t rc = 0;                                                        \
    const self_t *self = data;                                          \
    uint32_t start = 0;                                                 \
    uint32_t stop = argv[0].u.data.elem_count;                          \
    const T *a = argv[0].u.data.base;                                   \
    a += argv[0].u.data.first_elem;                                     \
    if (self->side == 0) {                                              \
        while (start != stop && a[start] == self->el.K)                 \
            ++start;                                                    \
    }                                                                   \
    else {                                                              \
        while (start != stop && a[stop - 1]  == self->el.K)             \
            --stop;                                                     \
    }                                                                   \
    rslt->elem_count = stop - start;                                    \
    if (rslt->elem_count > 0) {                                         \
        rc = KDataBufferResize(rslt->data, rslt->elem_count);           \
        if (rc == 0)                                                    \
            memcpy(rslt->data->base, a + start, rslt->elem_count);      \
    }                                                                   \
    return rc;                                                          \
}
#if 1
FUNC_TRIM ( uint8_t, u8 )
#else
static									 
rc_t CC trim_uint8_t ( void *data,                                	 
    const VXformInfo *info, int64_t row_id, VRowResult *rslt,		 
    uint32_t argc, const VRowData argv [] )                              
{                                                                        
    rc_t rc = 0;
    const self_t *self = data;						 
    uint32_t start = 0;						
    uint32_t stop = argv[0].u.data.elem_count;			
    const uint8_t *a = argv[0].u.data.base;
    
    a += argv[0].u.data.first_elem;
    
    if (self->side == 0) { /*** left side trim ****/			 
        while (start != stop && a[start] == self->el.u8)
            ++start;
    }
    else {
        while (start != stop && a[stop - 1]  == self->el.u8)
            --stop;
    }
    rslt->elem_count = stop - start;
    if (rslt->elem_count > 0) {
        rc = KDataBufferResize(rslt->data, rslt->elem_count);
        if (rc == 0)
            memcpy(rslt->data->base, a + start, rslt->elem_count);
    }
    return rc;
}
#endif
FUNC_TRIM ( int8_t, i8 )
FUNC_TRIM ( int16_t, i16 )
FUNC_TRIM ( int32_t, i32 )
FUNC_TRIM ( int64_t, i64 )
FUNC_TRIM ( uint16_t, u16 )
FUNC_TRIM ( uint32_t, u32 )
FUNC_TRIM ( uint64_t, u64 )
FUNC_TRIM ( float, f32 )
FUNC_TRIM ( double, f64 )

static VRowFunc trim_func_set [] =
{
    TRIM_NAME ( uint8_t ),
    TRIM_NAME ( uint16_t ),
    TRIM_NAME ( uint32_t ),
    TRIM_NAME ( uint64_t ),
    TRIM_NAME ( int8_t ),
    TRIM_NAME ( int16_t ),
    TRIM_NAME ( int32_t ),
    TRIM_NAME ( int64_t ),
    NULL,
    NULL,
    TRIM_NAME ( float ),
    TRIM_NAME ( double )
};

static
void CC self_whack( void *ptr )
{
	free( ptr );
}

VTRANSFACT_IMPL ( vdb_trim, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    int32_t size_idx;
    self_t self;

    self.el.u64 = 0;

    self.side=cp -> argv [ 0 ] . data . u8 [ 0 ];

    /* "T" must be member of numeric_set */
    switch ( info -> fdesc . desc . domain )
    {
    case vtdUint:
    case vtdInt:
    case vtdFloat:
        break;
    default:
        return RC ( rcXF, rcFunction, rcConstructing, rcType, rcIncorrect );
    }

    if ( dp -> argv [ 0 ] . desc . intrinsic_dim != 1 )
        return RC ( rcXF, rcFunction, rcConstructing, rcType, rcIncorrect );

    /* the only numeric types we support are between 8 and 64 bits */
    size_idx = uint32_lsbit ( dp -> argv [ 0] . desc . intrinsic_bits ) - 3;
    if ( size_idx < 0 || size_idx > 3 || ( ( dp -> argv [ 0 ] . desc . intrinsic_bits &
                                             ( dp -> argv [ 0 ] . desc . intrinsic_bits - 1 ) ) != 0 ) )
        return RC ( rcXF, rcFunction, rcConstructing, rcType, rcIncorrect );


    switch ( cp -> argv [ 1 ] . desc . domain )
    {
    case vtdUint:
    case vtdInt:
        switch ( cp -> argv [ 1 ] . desc . intrinsic_bits )
        {
        case 8:
	    self . el . u8 = cp -> argv [ 1 ] . data . u8 [ 0 ];
            break;
        case 16:
	    self . el . u16 = cp -> argv [ 1 ] . data . u16 [ 0 ];
            break;
        case 32:
	    self . el . u32 = cp -> argv [ 1 ] . data . u32 [ 0 ];
            break;
        case 64:
            self . el . u64 = cp -> argv [ 1 ] . data . u64 [ 0 ];
            break;
	default: return RC ( rcXF, rcFunction, rcConstructing, rcType, rcIncorrect );
        }
        break;

    case vtdFloat:
        switch ( cp -> argv [ 1 ] . desc . intrinsic_bits )
        {
        case 32:
            self . el . f32 = cp -> argv [ 1 ] . data . f32 [ 0 ];
            break;
        case 64:
            self . el . f64 = cp -> argv [ 1 ] . data . f64 [ 0 ];
            break;
	default: return RC ( rcXF, rcFunction, rcConstructing, rcType, rcIncorrect );
        }
        break;
    default: return RC ( rcXF, rcFunction, rcConstructing, rcType, rcIncorrect );
    }
    rslt -> self = malloc ( sizeof self );
    memcpy(rslt -> self,&self,sizeof(self));
    rslt -> whack = self_whack;
    rslt -> u . rf = trim_func_set [ ( dp -> argv [ 0 ] . desc . domain - vtdUint ) * 4 + size_idx ];
    rslt -> variant = vftRow;

    return 0;
}
