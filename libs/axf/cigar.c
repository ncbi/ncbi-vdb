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

#include <klib/defs.h>
#include <klib/rc.h>
#include <vdb/table.h>
#include <vdb/xform.h>
#include <vdb/schema.h>
#include <kdb/meta.h>
#include <klib/data-buffer.h>
#include <bitstr.h>
#include <sysalloc.h>
#include <klib/printf.h>

#include <klib/out.h>

#include <align/align.h>

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <os-native.h>

#include <insdc/insdc.h>

#define ARG_BASE(TYPE, N) (((TYPE const *)argv[(N)].u.data.base) + argv[(N)].u.data.first_elem)
#define ARG_ALIAS(TYPE, NAME, N) TYPE const *const NAME = ARG_BASE(TYPE, N)
#define ARG_ALIAS_COND(TYPE, NAME, N, ALT) TYPE const *const NAME = ((argc > (N)) ? ARG_BASE(TYPE, N) : ALT)

typedef struct {
    int version;
} self_t;

static
rc_t op2b( KDataBuffer *dst, unsigned const offset, unsigned *const count, int const opcode, unsigned oplen )
{
    unsigned digits = 1;
    unsigned scale = 10;
    
    if ( oplen == 0 )
    {
        *count = 0;
        return 0;
    }

    while ( scale < oplen )
    {
        scale *= 10;
        ++digits;
    }

    if ( scale == oplen ) /* oplen is whole power of 10 */
        ++digits;

    *count = digits + 1;
    
    if ( dst != NULL )
    {
        unsigned const need = offset + digits + 1;
        
        if ( need > dst->elem_count )
        {
            rc_t rc = KDataBufferResize( dst, need );
            if ( rc != 0 ) return rc;
        }

        {
            char *const base = &( ( char * )dst->base )[ offset ];
            
            base[ digits ] = opcode;
            do
            {
                unsigned const digit = oplen % 10;
                
                oplen /= 10;
                base[ --digits ] = digit + '0';
            } while ( digits );
        }
    }
    return 0;
}


static
rc_t cigar_string(KDataBuffer *dst, size_t boff,
                  INSDC_coord_len *const bsize, bool const full,
                  bool const has_mismatch[], bool const has_ref_offset[],
                  INSDC_coord_zero const read_start, INSDC_coord_zero const read_end,
                  int32_t const ref_offset[], unsigned const ro_len, unsigned *ro_offset)
{
    size_t bsz = 0;
    unsigned nwrit;
    uint32_t i, m, mm;
    rc_t rc;
    unsigned cur_off = ro_offset ? *ro_offset : 0;
    
#define BUF_WRITE(OP, LEN) if ((rc = op2b(dst, (unsigned const)(bsz + boff), &nwrit, (int const)(OP), (unsigned)(LEN))) != 0) return rc; bsz += nwrit;

#define MACRO_FLUSH_MATCH    { BUF_WRITE('=', m); m = 0; }
#define MACRO_FLUSH_MISMATCH { BUF_WRITE(i == read_end ? 'S' : 'X', mm); mm = 0; }

#define MACRO_FLUSH_BOTH \
if(m+mm > 0) { \
    if(i==read_end && has_ref_offset[i]) { \
        BUF_WRITE('M', m); BUF_WRITE('S', mm); \
    } else { \
        BUF_WRITE('M', m + mm) \
    } \
    m=mm=0; \
}

#define MACRO_FLUSH \
if(full){ \
    MACRO_FLUSH_MATCH; \
    MACRO_FLUSH_MISMATCH; \
} else { \
    MACRO_FLUSH_BOTH; \
}

    for( i = read_start, bsz = m = mm = 0; i < (uint32_t)read_end; i++ )
    {
        if( has_ref_offset[ i ] ) /*** No offset in the reference **/
        {
            int32_t offset;
            
            if ( cur_off >= ro_len ) /*** bad data ***/
                return RC( rcXF, rcFunction, rcExecuting, rcData, rcInvalid );
            
            offset = ref_offset[ cur_off++ ];
            
            if( offset > 0 ) /*** insert in the reference, delete in sequence ***/
            {
                if ( i == 0 ) /**** deletes in the beginning are disallowed, REF_START should have been moved and delete converted to insert **/
                    return RC( rcXF, rcFunction, rcExecuting, rcData, rcInvalid );
                MACRO_FLUSH;
                BUF_WRITE( 'D', offset );
            }
            else if ( offset < 0 ) /**** delete from the reference ***/
            {
                offset = -offset;
                if ( i + offset > (uint32_t)read_end )
                    return RC( rcXF, rcFunction, rcExecuting, rcData, rcInvalid );
                if ( i > 0 ) /** normally indels are before the current base ***/
                {
                    MACRO_FLUSH;
                    BUF_WRITE( 'I', offset );
                }
                else
                { /***  this  is a soft clip at the beginning ***/
                    BUF_WRITE( 'S', offset );
                }
                i += offset;
            }
            else
            { /*** Not possible ??? ***/
                return RC( rcXF, rcFunction, rcExecuting, rcData, rcInvalid );
            }
        }

        if ( i < (uint32_t)read_end )
        {
            if ( has_mismatch[ i ] )
            {
                if ( full )
                {
                    MACRO_FLUSH_MATCH;
                }
                mm++;
            }
            else
            {
                if ( full )
                {
                    MACRO_FLUSH_MISMATCH;
                }
                else
                {
                    m += mm;
                    mm = 0;
                }
                m++;
            }
        }
    }
    MACRO_FLUSH;
    *bsize = (INSDC_coord_len)bsz;
    if ( ro_offset )
        *ro_offset = cur_off;
    return 0;
    
#undef BUF_WRITE
#undef MACRO_FLUSH_MATCH
#undef MACRO_FLUSH_MISMATCH
#undef MACRO_FLUSH_BOTH
#undef MACRO_FLUSH
}

typedef struct {
    int opcode_M, opcode_X, opcode_S;
} cigar_opcode_options_t;

static
rc_t cigar_string_2_0(KDataBuffer *dst,
                      size_t boff,
                      INSDC_coord_len *const bsize,
                      bool const has_mismatch[],
                      bool const has_ref_offset[],
                      INSDC_coord_zero const read_start,
                      INSDC_coord_zero const read_end,
                      int32_t const ref_offset[],
                      unsigned const ro_len,
                      unsigned ro_offset[],
                      unsigned const reflen,
                      cigar_opcode_options_t const *ops)
{
    int ri;
    unsigned si;
    unsigned di;
    rc_t rc;
    unsigned nwrit;
    unsigned cur_off = ro_offset ? *ro_offset : 0;
    unsigned op_len;
    int opcode;
    
#define BUF_WRITE(OP, LEN) { if ((rc = op2b(dst, (unsigned const)(di + boff), &nwrit, (int const)(OP), (unsigned)(LEN))) != 0) return rc; di += nwrit; }
    si = read_start;
    if ( /* !use_S && */ read_start == read_end && reflen > 0 ) /** full delete as a last ploidy ends up written nowhere  **/
    {
        di=0;
        opcode = 'D';
        op_len = reflen;
    }
    else for ( op_len = di = 0, opcode = ri = 0; si < ( unsigned )read_end && ri <= ( int )reflen; )
    {
        if ( has_ref_offset[ si ] )
        {
            int offs;
            
            if ( op_len > 0 )
            {
                BUF_WRITE( opcode, op_len );
                op_len = 0;
            }
            if ( cur_off >= ro_len ) /*** bad data ***/
                return RC( rcXF, rcFunction, rcExecuting, rcData, rcInvalid );
            
            offs = ref_offset[ cur_off++ ];
            if ( offs < 0 )
            {
                unsigned j;
                for ( j = 1; j < (unsigned)-offs && ( si + j ) < ( unsigned )read_end; )
                {
                    if ( has_ref_offset[ si + j ] ) /*** structured insert **/
                    {
                        BUF_WRITE( si ? 'I' : ops->opcode_S, j );
                        offs += j;
                        si   += j;
                        j = 1;
                    }
                    else
                    {
                        j++;
                    }
                }
                if ( offs < 0 )
                {
                    BUF_WRITE( si ? 'I' : ops->opcode_S, -offs );
                    si -= offs;
                }
                continue;
            }
            else if ( offs > 0 )
            {
                BUF_WRITE( 'D', offs );
                ri += offs;
            }
            else
                return RC( rcXF, rcFunction, rcExecuting, rcData, rcInvalid );
        }
        
        if ( ri < ( int )reflen )
        {
            int const op_nxt = ( has_mismatch[ si ] ? ops->opcode_X : ops->opcode_M );
            
            if ( op_len == 0 || opcode == op_nxt )
                ++op_len;
            else
            {
                BUF_WRITE( opcode, op_len );
                op_len = 1;
            }
            opcode = op_nxt;
        }
        else
            break;
        ++si;
        ++ri;
    }
    
    BUF_WRITE( opcode, op_len );
    if ( si < (unsigned)read_end )
    {
        if ( cur_off + 1 < ro_len )
        {
            assert( read_end + ref_offset[ cur_off ] == si );
            cur_off++;
            BUF_WRITE( 'I', read_end - si );
        }
        else
        {
            BUF_WRITE( ops->opcode_S, read_end - si );
        }
    }
    *bsize = di;
    if ( ro_offset != NULL )
        *ro_offset = cur_off;
    
    return 0;
#undef BUF_WRITE
}

static
rc_t cigar_string_2(KDataBuffer *dst, size_t boff,
                    INSDC_coord_len *bsize, const int version,
                    bool const has_mismatch[], bool const has_ref_offset[],
                    INSDC_coord_zero const read_start, INSDC_coord_zero const read_end,
                    int32_t const ref_offset[], unsigned const ro_len, unsigned * ro_offset,
                    unsigned const reflen, bool use_S)
{
    cigar_opcode_options_t const ops = {
        version == 1 ? '=' : 'M',
        version == 1 ? 'X' : 'M',
        use_S ? 'S' : 'I'
    };
    return cigar_string_2_0(dst, boff, bsize, has_mismatch, has_ref_offset, read_start, read_end, ref_offset, ro_len, ro_offset, reflen, &ops);
}

static
rc_t cigar_string_2_1(KDataBuffer *dst, size_t boff,
                      INSDC_coord_len *bsize, const int version,
                      bool const has_mismatch[], bool const has_ref_offset[],
                      INSDC_coord_zero const read_start, INSDC_coord_zero const read_end,
                      int32_t const ref_offset[], unsigned const ro_len, unsigned ro_offset[],
                      uint8_t const ref_offset_type[],
                      unsigned const reflen)
{
    int ri;
    unsigned si = read_start;
    unsigned di;
    rc_t rc;
    unsigned nwrit;
    unsigned cur_off = ro_offset ? *ro_offset : 0;
    unsigned op_len;
    int opcode;
    int const opM = (version & 1) ? '=' : 'M';
    int const opX = (version & 1) ? 'X' : 'M';
    
#define BUF_WRITE(OP, LEN) { if ((rc = op2b(dst, (unsigned const)(di + boff), &nwrit, (int const)(OP), (unsigned)(LEN))) != 0) return rc; di += nwrit; }
    if (read_start == read_end && reflen > 0) {
        /** full delete as a last ploidy ends up written nowhere  **/
        di = 0;
        opcode = 'D';
        op_len = reflen;
    }
    else for (op_len = di = 0, opcode = ri = 0; si < (unsigned)read_end && ri <= (int)reflen; ) {
        if (has_ref_offset[si]) {
            int offs;
            int type;
            
            if (op_len > 0) {
                BUF_WRITE(opcode, op_len);
                op_len = 0;
            }
            if (cur_off >= ro_len) /*** bad data ***/
                return RC(rcXF, rcFunction, rcExecuting, rcData, rcInvalid);
            
            type = ref_offset_type[cur_off];
            offs = ref_offset[cur_off];
            ++cur_off;
            
            if (offs < 0) {
		unsigned const ins = -offs;
		if( (version & 1) /*CIGAR_LONG*/ && type == 5 /** complete genomics **/ && ri >= ins /** safety **/){
			BUF_WRITE('B', ins);
			ri -= ins;
		} else {
			int const opc = (type == 1) ? 'S' : 'I';
			BUF_WRITE(opc, ins);
			si += ins;
			continue;
		}
            } else if (offs > 0) {
                int const opc = type == 0 ? 'D' : 'N';
                
                BUF_WRITE(opc, offs);
                ri += offs;
            }
        }
        if (ri < (int)reflen) {
            int const op_nxt = (has_mismatch[si] ? opX : opM);
            
            if (op_len == 0 || opcode == op_nxt)
                ++op_len;
            else {
                BUF_WRITE(opcode, op_len);
                op_len = 1;
            }
            opcode = op_nxt;
        }
        else
            break;
        ++si;
        ++ri;
    }
    
    BUF_WRITE(opcode, op_len);
    *bsize = di;
    if (ro_offset != NULL)
        *ro_offset = cur_off;
    
    return 0;
#undef BUF_WRITE
}

static INSDC_coord_len right_soft_clip(unsigned seq_len, unsigned ref_len,
                                       unsigned noffsets,
                                       int32_t const ref_offset[])
{
    INSDC_coord_len a = ref_len;
    unsigned i;
    
    for ( i = 0; i < noffsets; ++i )
        a -= ref_offset[ i ];
    return a < seq_len ? seq_len - a : 0;
}

static
rc_t CC cigar_impl ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    self_t const *self = data;
    unsigned const rdln   = ( unsigned const )argv[ 0 ].u.data.elem_count;
    unsigned const ro_len = ( unsigned const )argv[ 2 ].u.data.elem_count;
    bool const *has_mismatch   = argv[ 0 ].u.data.base;
    bool const *has_ref_offset = argv[ 1 ].u.data.base;
    int32_t const *ref_offset  = argv[ 2 ].u.data.base;
    rc_t rc;
    
    assert( argv[ 0 ].u.data.elem_bits == 8 );
    assert( argv[ 1 ].u.data.elem_bits == 8 );
    assert( argv[ 2 ].u.data.elem_bits == 32 );

    assert( rdln == argv[ 1 ].u.data.elem_count );

    has_mismatch   += argv[ 0 ].u.data.first_elem;
    has_ref_offset += argv[ 1 ].u.data.first_elem;
    ref_offset     += argv[ 2 ].u.data.first_elem;

    rslt->data->elem_bits = 8;
    if ( argc == 3 )
    {
        INSDC_coord_len count;
        
        rc = cigar_string( rslt->data, 0, &count, self->version & 0x1,
                           has_mismatch, has_ref_offset,
                           0, rdln, ref_offset, ro_len, NULL);
        rslt->elem_count = count;
    }
    else
    {
        int32_t const *const rfln = argv[ 3 ].u.data.base;
        INSDC_coord_len count;
        
        rc = cigar_string_2( rslt->data, 0, &count, self->version & 0x1,
                            has_mismatch, has_ref_offset,
                            0, rdln, ref_offset, ro_len, NULL,
                            rfln[ argv[ 3 ].u.data.first_elem ], true );
        rslt->elem_count = count;
    }
    return rc;
}

static
rc_t CC cigar_impl_2 ( void *data, const VXformInfo *info, int64_t row_id,
                        VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    self_t const *self = data;
    bool const *has_mismatch        = argv[0].u.data.base;
    bool const *has_ref_offset      = argv[1].u.data.base;
    int32_t const *ref_offset       = argv[2].u.data.base;
    INSDC_coord_len const *read_len = argv[3].u.data.base;
    uint8_t const *const ref_offset_type = argc >= 6 ? ((uint8_t const *)argv[5].u.data.base) + argv[5].u.data.first_elem : NULL;
    uint32_t const nreads = (uint32_t const)argv[ 3 ].u.data.elem_count;
    uint32_t const ro_len = (uint32_t const)argv[ 2 ].u.data.elem_count;
    uint32_t n;
    uint32_t ro_offset = 0;
    rc_t rc = 0;
    INSDC_coord_zero start;
    INSDC_coord_len *cigar_len = NULL;
    KDataBuffer *buf = ( self->version & 0x04 ) ? NULL : rslt->data;
    
    assert( argv[ 0 ].u.data.elem_bits == 8 );
    assert( argv[ 1 ].u.data.elem_bits == 8 );
    assert( argv[ 2 ].u.data.elem_bits == 32 );
    assert( argv[ 3 ].u.data.elem_bits == 32 );
    
    has_mismatch   += argv[ 0 ].u.data.first_elem;
    has_ref_offset += argv[ 1 ].u.data.first_elem;
    ref_offset     += argv[ 2 ].u.data.first_elem;
    read_len       += argv[ 3 ].u.data.first_elem;

    if (self->version & 0x4) {
        rslt->data->elem_bits = sizeof(cigar_len[0]) * 8;
        rslt->elem_count = nreads;

        rc = KDataBufferResize(rslt->data, rslt->elem_count);
        if (rc != 0)
            return rc;

        cigar_len = rslt->data->base;
        if (argv[0].u.data.elem_count == 0 || argv[1].u.data.elem_count == 0) {
            memset(cigar_len, 0, sizeof(cigar_len[0]) * nreads);
            return 0;
        }
    }
    else {
        rslt->data->elem_bits = 8;
        rslt->elem_count = 0;
    }

    for (n = 0, start = 0, ro_offset = 0; n < nreads; start += read_len[n], ++n) {
        INSDC_coord_len cnt;
        INSDC_coord_len *const count = (self->version & 0x04) ? cigar_len + n : &cnt;
        
        if (argc == 4) {
            rc = cigar_string(buf, (size_t)rslt->elem_count, count, self->version & 0x1,
                              has_mismatch, has_ref_offset,
                              start, start + read_len[n],
                              ref_offset, ro_len, &ro_offset);
        }
        else if (argc == 5) {
            int32_t const *const reflen = argv[4].u.data.base;
            
            rc = cigar_string_2(buf, (size_t)rslt->elem_count, count, self->version & 0x1,
                                has_mismatch, has_ref_offset,
                                start, start + read_len[n],
                                ref_offset, ro_len, &ro_offset,
                                reflen[argv[4].u.data.first_elem], nreads == 1 ? true : false);
        }
        else { /* should be new function */
            int32_t const *const reflen = argv[4].u.data.base;
            
            rc = cigar_string_2_1(buf, (size_t)rslt->elem_count, count, self->version & 0x1,
                                  has_mismatch, has_ref_offset,
                                  start, start + read_len[n],
                                  ref_offset, ro_len, &ro_offset,
                                  ref_offset_type,
                                  reflen[argv[4].u.data.first_elem]);
        }
        if (rc != 0)
            return rc;
        if ((self->version & 0x04) == 0)
            rslt->elem_count += cnt;
    }
    return 0;
}

static
void CC self_whack( void *ptr )
{
    free( ptr );
}


/* 
 * function
 * ascii ALIGN:cigar #1 ( bool has_mismatch, bool has_ref_offset, I32 ref_offset);
 */
VTRANSFACT_IMPL ( ALIGN_cigar, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    self_t self;

    self.version =cp -> argv [ 0 ] . data . u8 [ 0 ];
    switch(self.version){
    case 0:
    case 1:
        break;
    default:
        return RC ( rcXF, rcFunction, rcConstructing, rcParam, rcIncorrect );
    }
    rslt->u.rf = cigar_impl;
    rslt->variant = vftRow;
    rslt -> self = malloc ( sizeof self );
    memmove(rslt -> self,&self,sizeof(self));
    rslt -> whack = self_whack;

    return 0;
}

VTRANSFACT_IMPL ( ALIGN_cigar_2, 2, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    self_t self;
    VTypedesc const return_type = info->fdesc.desc;
    int const version = cp->argv[0].data.u8[0];

    self.version = version;
    switch (version) {
    case 0:
    case 1:
        break;
    default:
        return RC( rcXF, rcFunction, rcConstructing, rcParam, rcIncorrect );
    }

    if (return_type.domain == vtdAscii && return_type.intrinsic_bits == 8)
    {
        self.version |= 0x2;
    }
    else if (return_type.domain == vtdUint && return_type.intrinsic_bits == sizeof(INSDC_coord_len) * 8)
    {
        self.version |= 0x4;
    }
    else
    {
        return RC( rcXF, rcFunction, rcConstructing, rcParam, rcIncorrect );
    }

    rslt->u.rf = cigar_impl_2;
    rslt->variant = vftRow;
    rslt->self = malloc(sizeof(self));
    if (rslt->self == NULL)
        return RC( rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted );

    memmove(rslt->self, &self, sizeof(self));
    rslt->whack = self_whack;
    return 0;
}

static
rc_t CC edit_distance_impl ( void *data, const VXformInfo *info, int64_t row_id,
                             VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    uint32_t i, roi, mrun;
    uint32_t len = ( uint32_t ) argv[ 0 ].u.data.elem_count;
    uint32_t *dst;
    
    uint8_t const *has_mismatch   = argv [ 0 ] . u . data . base;
    uint8_t const *has_ref_offset = argv [ 1 ] . u . data . base;
    int32_t const *ref_offset     = argv [ 2 ] . u . data . base;
    
    assert( argv[ 0 ].u.data.elem_bits == 8 );
    assert( argv[ 1 ].u.data.elem_bits == 8 );
    assert( argv[ 2 ].u.data.elem_bits == 32 );
    
    assert( len == argv[ 1 ].u.data.elem_count );
    
    has_mismatch   += argv [ 0 ] . u . data . first_elem;
    has_ref_offset += argv [ 1 ] . u . data . first_elem;
    ref_offset     += argv [ 2 ] . u . data . first_elem;
    
    /* resize output row for the total number of reads */    
    rslt->data->elem_bits = rslt->elem_bits;
    rc = KDataBufferResize( rslt -> data, 1 );
    if ( rc != 0 )
        return rc;
    
    rslt -> elem_count = 1;
    dst = rslt -> data -> base;
    dst[ 0 ] = 0;
    if( len == 0 )
        return 0; /** nothing to do **/
    
    if ( has_ref_offset[ 0 ] ) /** skip mismatches from the beginning == soft clip ***/
    {
        if ( ref_offset[ 0 ] > 0 )  /**** deletes in the beginning are disallowed, REF_START should have been moved and delete converted to insert **/
            return RC( rcXF, rcFunction, rcExecuting, rcData, rcInvalid  );

        i = -( ref_offset [ 0 ] );
        roi = 1;
        mrun = 0;
    }
    else
    {
        i = roi = 0;
    }

    for ( mrun = 0; i < len; i++ )
    {
        if ( has_mismatch[ i ] )
        {
            mrun++;
        }
        else /*** intentionally skipping last run of mismatches **/
        {
            dst[ 0 ] += mrun;
            mrun = 0;
        }
    }
    return 0;
}

/*
 * function
 * ascii NCBI:align:edit_distance #1 ( bool has_mismatch, bool has_ref_offset, I32 ref_offset);
 */
VTRANSFACT_IMPL ( NCBI_align_edit_distance, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = edit_distance_impl;
    rslt->variant = vftRow;
    return 0;
}


/*
 * edit distance = sum of lengths of inserts
 *               + sum of lengths of deletes
 *               + number of mismatches
 * excluding soft clips
 */

unsigned edit_distance(bool const has_ref_offset[],
                       bool const has_mismatch[],
                       unsigned const readlen,
                       unsigned const reflen,
                       int32_t const ref_offset[],
                       unsigned const offsets)
{
    if ( readlen == 0 )
    {
        /* full delete */
        return reflen;
    }
    else
    {
        INSDC_coord_len const rsc = right_soft_clip( readlen, reflen, offsets, ref_offset );
        unsigned indels = 0;
        unsigned misses = 0;
        unsigned i = 0;
        unsigned j = 0;
        
        if ( has_ref_offset[ 0 ] && ref_offset[ j ] < 0 )
            j = i = 1;
        
        /* sum of insert lengths + sum of delete lengths excluding soft clips */
        for ( ; i < readlen - rsc; ++i )
        {
            if ( has_ref_offset[ i ] )
            {
                int const offset = ref_offset[ j++ ];
                
                if ( offset < 0 )
                    indels += -offset;
                else
                    indels +=  offset;
            }
        }

        /* sum of mismatches not in inserts or soft clips */
        for ( j = i = 0; i < readlen - rsc; )
        {
            if ( has_ref_offset[ i ] )
            {
                int offset = ref_offset[ j++ ];
                
                if ( offset < 0 )
                {
                    i += -offset;
                    continue;
                }
            }
            misses += has_mismatch[ i ] ? 1 : 0;
            ++i;
        }
        return indels + misses;
    }
}


static
rc_t CC edit_distance_2_impl ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    unsigned const nreads = argc > 4 ? ( unsigned const )argv[ 4 ].u.data.elem_count : 1;
    unsigned const len = ( unsigned const )argv[ 0 ].u.data.elem_count;
    unsigned const noffsets = ( unsigned const )argv[ 2 ].u.data.elem_count;
    INSDC_coord_len const dummy_rl = len;

    ARG_ALIAS     ( bool           , has_mismatch  , 0 );
    ARG_ALIAS     ( bool           , has_ref_offset, 1 );
    ARG_ALIAS     ( int32_t        , ref_offset    , 2 );
    ARG_ALIAS     ( INSDC_coord_len, ref_len       , 3 );
    ARG_ALIAS_COND( INSDC_coord_len, readlen       , 4, &dummy_rl );
    
    assert( argv[ 0 ].u.data.elem_bits == sizeof( has_mismatch  [ 0 ] ) * 8 );
    assert( argv[ 1 ].u.data.elem_bits == sizeof( has_ref_offset[ 0 ] ) * 8 );
    assert( argv[ 2 ].u.data.elem_bits == sizeof( ref_offset    [ 0 ] ) * 8 );
    assert( argv[ 3 ].u.data.elem_bits == sizeof( ref_len       [ 0 ] ) * 8 );

    rslt->data->elem_bits = rslt->elem_bits;
    if ( len == 0 )
    {
        return KDataBufferResize( rslt->data, rslt->elem_count = 0 );
    }
    
    assert( len == argv[ 1 ].u.data.elem_count );

    rslt->elem_count = nreads;
    rc = KDataBufferResize( rslt->data, nreads );
    if ( rc == 0 )
    {
        unsigned i;
        unsigned start = 0;
        unsigned offset = 0;
        uint32_t *const dst = rslt->data->base;
        
        for ( i = 0; i < nreads; ++i )
        {
            unsigned const rlen = readlen[ i ];
            unsigned j;
            unsigned offsets = 0;
            
            for ( j = 0; j < rlen; ++j )
            {
                if ( has_ref_offset[ start + j ] )
                    ++offsets;
            }

            if ( offsets + offset > noffsets )
                return RC( rcXF, rcFunction, rcExecuting, rcData, rcInvalid );
            
            dst[ i ] = edit_distance( has_ref_offset + start,
                                   has_mismatch + start,
                                   rlen,
                                   ref_len[ 0 ],
                                   ref_offset + offset,
                                   offsets );
            start += rlen;
            offset += offsets;
        }
    }
    return rc;
}


/*
 * function
 * U32 NCBI:align:edit_distance #2 ( bool has_mismatch, bool has_ref_offset,
 *     I32 ref_offset, INSDC:coord:len ref_len );
 */
VTRANSFACT_IMPL ( NCBI_align_edit_distance_2, 2, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = edit_distance_2_impl;
    rslt->variant = vftRow;
    return 0;
}


/*
 * function
 * U32 NCBI:align:edit_distance #3 ( bool has_mismatch, bool has_ref_offset,
 *     I32 ref_offset, NCBI:align:ro_type ref_offset_type, INSDC:coord:len read_len );
 */
static
rc_t CC edit_distance_3_impl(void *const data, VXformInfo const info[],
                             int64_t const row_id,
                             VRowResult rslt[],
                             uint32_t const argc,
                             VRowData const argv[/* argc */])
{
    rc_t rc;
    unsigned const nreads = (unsigned const)argv[4].u.data.elem_count;
    ARG_ALIAS(              bool, has_mismatch   , 0);
    ARG_ALIAS(              bool, has_ref_offset , 1);
    ARG_ALIAS(           int32_t, ref_offset     , 2);
    ARG_ALIAS(NCBI_align_ro_type, ref_offset_type, 3);
    ARG_ALIAS(   INSDC_coord_len, read_len       , 4);
    
    rslt->data->elem_bits = rslt->elem_bits;
    rslt->elem_count = nreads;
    rc = KDataBufferResize(rslt->data, nreads);
    if (rc == 0) {
        uint32_t *const result = rslt->data->base;
        unsigned cur = 0;
        unsigned cur_ro = 0;
        unsigned n;
        
        for (n = 0; n < nreads; ++n) {
            unsigned const len = read_len[n];
            unsigned j;
            unsigned miss = 0;
            unsigned indel = 0;
            unsigned cur_ro2 = cur_ro;
            
            for (j = 0; j < len; ) {
                if (has_ref_offset[cur] != 0) {
                    int const offset = ref_offset[cur_ro];
                    
                    ++cur_ro;
                    if (offset < 0) {
                        unsigned const dist = -offset;
                        
                        cur += dist;
                        j += dist;
                        continue;
                    }
                }
                if (has_mismatch[cur])
                    ++miss;
                ++cur;
                ++j;
            }
            while (cur_ro2 < cur_ro) {
                int const type = ref_offset_type[cur_ro2];
                
                if (type == 0) {
                    int const offset = ref_offset[cur_ro2];
                    
                    if (offset < 0)
                        indel += -offset;
                    else
                        indel += +offset;
                }
                ++cur_ro2;
            }
            result[n] = miss + indel;
        }
    }
    return rc;
}

VTRANSFACT_IMPL ( NCBI_align_edit_distance_3, 3, 0, 0 ) ( const void *Self, const VXfactInfo *info,
     VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = edit_distance_3_impl;
    rslt->variant = vftRow;
    return 0;
}


/*
 * function bool ALIGN:generate_has_mismatch #1 (INSDC:4na:bin reference,
 *     INSDC:4na:bin subject, bool has_ref_offset, I32 ref_offset);
 */
static
rc_t CC generate_has_mismatch_impl ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    int32_t si, ri, roi;
    uint32_t ref_len = ( uint32_t )argv[ 0 ].u.data.elem_count;
    uint32_t sbj_len = ( uint32_t )argv[ 1 ].u.data.elem_count;
    uint32_t hro_len = ( uint32_t )argv[ 2 ].u.data.elem_count;
    uint32_t ro_len  = ( uint32_t )argv[ 3 ].u.data.elem_count;
    const uint8_t *ref  = argv [ 0 ] . u . data . base;
    const uint8_t *sbj  = argv [ 1 ] . u . data . base;
    const uint8_t *has_ref_offset     = argv [ 2 ] . u . data . base;
    const int32_t *ref_offset = argv [ 3 ] . u . data . base;

    uint8_t * dst;
    uint32_t  len = 0;

    rslt -> data -> elem_bits = 8;
    if ( sbj_len == 0 )
    {
        rc = KDataBufferResize ( rslt -> data, 0 );
        if ( rc != 0 )
            return rc;
        rslt -> elem_count = 0;
        return 0;
    }
    assert( sbj_len == hro_len );
    len = sbj_len;

    ref            += argv [ 0 ] . u . data . first_elem;
    sbj            += argv [ 1 ] . u . data . first_elem;
    has_ref_offset += argv [ 2 ] . u . data . first_elem;
    ref_offset     += argv [ 3 ] . u . data . first_elem;

    /* resize output row for the total number of reads */
    rslt -> data -> elem_bits = 8;
    rc = KDataBufferResize ( rslt -> data, len );
    if ( rc != 0 )
        return rc;
    rslt -> elem_count = len;
    dst = rslt -> data->base;
    for ( si = ri = roi = 0; si < ( int32_t )len; si++, ri++ )
    {
        if ( has_ref_offset[ si ] != 0 ) /*** need to offset the reference ***/
        {
            if ( roi >= ( int32_t )ro_len )
            {
                return RC( rcXF, rcFunction, rcExecuting, rcData, rcInvalid );
            }
            ri += ref_offset[ roi++ ];
        }

        if ( ri >= 0 && ri < ( int32_t )ref_len && sbj[ si ] == ref[ ri ] )
            dst[ si ]=0;
        else
            dst[ si ]=1;
    }
    return 0;
}


VTRANSFACT_IMPL ( ALIGN_generate_has_mismatch, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = generate_has_mismatch_impl;
    rslt->variant = vftRow;
    rslt -> self = NULL;
    rslt -> whack = NULL;
    return 0;
}

/*
 * function bool ALIGN:generate_mismatch #1 (INSDC:4na:bin reference,INSDC:4na:bin subject, bool has_ref_offset, I32 ref_offset);
 */
static
rc_t CC generate_mismatch_impl ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    int32_t si, ri, roi;
    uint32_t ref_len = ( uint32_t )argv[ 0 ].u.data.elem_count;
    uint32_t sbj_len = ( uint32_t )argv[ 1 ].u.data.elem_count;
    uint32_t hro_len = ( uint32_t )argv[ 2 ].u.data.elem_count;
    uint32_t ro_len  = ( uint32_t )argv[ 3 ].u.data.elem_count;
    const uint8_t *ref  = argv [ 0 ] . u . data . base;
    const uint8_t *sbj  = argv [ 1 ] . u . data . base;
    const uint8_t *has_ref_offset     = argv [ 2 ] . u . data . base;
    const int32_t *ref_offset = argv [ 3 ] . u . data . base;
    uint8_t	buf[ 5 * 1024 ];
    uint32_t  len;

    rslt -> data -> elem_bits = 8;
    if ( sbj_len == 0 )
    {
        return KDataBufferResize( rslt->data, rslt->elem_count = 0 );
    }
    assert( sbj_len == hro_len );

    ref            += argv [ 0 ] . u . data . first_elem;
    sbj            += argv [ 1 ] . u . data . first_elem;
    has_ref_offset += argv [ 2 ] . u . data . first_elem;
    ref_offset     += argv [ 3 ] . u . data . first_elem;

    for ( si = ri = roi = 0, len = 0; si < ( int32_t )sbj_len; si++, ri++ )
    {
        if ( has_ref_offset[ si ] != 0 )/*** need to offset the reference ***/
        {
            if ( roi >= ( int32_t )ro_len )
            {
                return RC( rcXF, rcFunction, rcExecuting, rcData, rcInvalid );
            }
            ri += ref_offset[ roi++ ];
        }

        if ( ri >=0 && ri < ( int32_t )ref_len && sbj[ si ] == ref[ ri ] )
        {
            /*noop*/
        }
        else
        {
            if ( len > sizeof( buf ) )
                return RC( rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient );

            buf[ len++ ] = sbj[ si ];
        }
    }

    /* resize output row for the total number of reads */
    rc = KDataBufferResize ( rslt -> data, len );
    if ( rc != 0 )
        return rc;
    rslt -> elem_count = len;
    memmove( rslt -> data->base, buf, len );
    return 0;
}


VTRANSFACT_IMPL ( ALIGN_generate_mismatch, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = generate_mismatch_impl;
    rslt->variant = vftRow;
    rslt -> self = NULL;
    rslt -> whack = NULL;
    return 0;
}



/*
 * function INSDC:quality:phred NCBI:align:generate_mismatch_qual #1 (INSDC:quality:phred qual,bool has_mismatch)
 */
static
rc_t CC generate_mismatch_qual_impl ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    const uint8_t *q    = argv[ 0 ].u.data.base; 
    const uint8_t *h_mm = argv[ 1 ].u.data.base;
    uint8_t	buf[ 5 * 1024 ];
    uint32_t mm_cnt, i;

    q    += argv[ 0 ].u.data.first_elem;
    h_mm += argv[ 1 ].u.data.first_elem;

    for ( mm_cnt = 0, i = 0; i < argv[ 0 ].u.data.elem_count; i++ )
    {
        if( h_mm[ i ] )
        {
            buf[ mm_cnt++ ] = q[ i ];
        }
    }

    /* resize output row for the total number of reads */
    rslt -> data -> elem_bits = 8;
    rc = KDataBufferResize ( rslt -> data, mm_cnt );
    if ( rc != 0 )
        return rc;
    rslt -> elem_count = mm_cnt;
    if ( mm_cnt > 0 )
        memmove( rslt -> data->base, buf, mm_cnt );

    return 0;
}


VTRANSFACT_IMPL ( ALIGN_generate_mismatch_qual, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = generate_mismatch_qual_impl;
    rslt->variant = vftRow;
    rslt -> self = NULL;
    rslt -> whack = NULL;
    return 0;
}



/*
 * function ascii NCBI:align:get_mismatch_read #1
 *    ( bool has_mismatch, INSDC:dna:text mismatch )
 */
static
rc_t CC get_mismatch_read_impl ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    char *result;
    unsigned const readlen = ( unsigned const )argv[ 0 ].u.data.elem_count;
    
    rslt->data->elem_bits = sizeof( result[ 0 ] ) * 8;
    rslt->elem_count = readlen;
    rc = KDataBufferResize( rslt->data, rslt->elem_count );
    if ( rc == 0 )
    {
        unsigned i;
        unsigned j;
        bool const *has_mismatch = argv[ 0 ].u.data.base;
        char const *mismatch = argv[ 1 ].u.data.base;
        
        assert( argv[ 0 ].u.data.elem_bits == sizeof( has_mismatch[ 0 ] ) * 8 );
        has_mismatch += argv[ 0 ].u.data.first_elem;
        
        assert( argv[ 1 ].u.data.elem_bits == sizeof( mismatch[ 0 ] ) * 8 );
        mismatch += argv[ 1 ].u.data.first_elem;
        
        result = rslt->data->base;
        for ( i = j = 0; i != readlen; ++i )
        {
            result[ i ] = has_mismatch[ i ] ? mismatch[ j++ ] : '=';
        }
    }

    return rc;
}


VTRANSFACT_IMPL ( NCBI_align_get_mismatch_read, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = get_mismatch_read_impl;
    rslt->variant = vftRow;
    rslt -> self = NULL;
    rslt -> whack = NULL;
    return 0;
}



/*
 * function INSDC:coord:len NCBI:align:get_left_soft_clip #1 (
 *     bool has_ref_offset, INSDC:coord:zero ref_offset )
 */
static
rc_t CC left_soft_clip_impl ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    INSDC_coord_len result;
    unsigned const n_offsets = ( unsigned const )argv[ 1 ].u.data.elem_count;
    
    result = 0;
    
    if ( n_offsets > 0 )
    {
        bool const *has_ref_offset = argv[ 0 ].u.data.base;
        int32_t const *ref_offset = argv[ 1 ].u.data.base;
        
        assert( argv[ 0 ].u.data.elem_bits == sizeof( has_ref_offset[ 0 ] ) * 8 );
        assert( argv[ 1 ].u.data.elem_bits == sizeof( ref_offset[ 0 ] ) * 8 );
        
        has_ref_offset += argv[ 0 ].u.data.first_elem;
        ref_offset += argv[ 1 ].u.data.first_elem;
        
        if ( has_ref_offset[ 0 ] && ref_offset[ 0 ] < 0 )
        {
            result = -ref_offset[ 0 ];
        }
    }

    rslt->data->elem_bits = sizeof( result ) * 8;
    rslt->elem_count = 1;
    rc = KDataBufferResize( rslt->data, 1 );
    if ( rc == 0 )
        memmove( rslt->data->base, &result, sizeof( result ) );

    return rc;
}


static
rc_t CC left_soft_clip_impl_2 ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    INSDC_coord_len* result;
    const bool* has_ref_offset = argv[ 0 ].u.data.base;
    const int32_t* ref_offset = argv[ 1 ].u.data.base;
    const uint64_t n_offsets = argv[ 1 ].u.data.elem_count;
    const INSDC_coord_len* read_len = argv[ 2 ].u.data.base;
    const uint32_t nreads = ( const uint32_t )argv[ 2 ].u.data.elem_count;
    uint32_t n, roi;
    INSDC_coord_zero start;

    assert( argv[ 0 ].u.data.elem_bits == sizeof( has_ref_offset[ 0 ] ) * 8 );
    assert( argv[ 1 ].u.data.elem_bits == sizeof( ref_offset[ 0 ] ) * 8 );
    assert( argv[ 2 ].u.data.elem_bits == sizeof( read_len[ 0 ] ) * 8 );
    
    has_ref_offset += argv[ 0 ].u.data.first_elem;
    ref_offset += argv[ 1 ].u.data.first_elem;
    read_len += argv[ 2 ].u.data.first_elem;

    rslt->data->elem_bits = sizeof( *result ) * 8;
    rslt->elem_count = nreads;
    rc = KDataBufferResize( rslt->data, rslt->elem_count );
    result = rslt->data->base;

    for ( n = roi = start = 0; rc == 0 && n < nreads; start += read_len[ n++ ] )
    {
        if ( has_ref_offset[ start ] && ref_offset[ roi ] < 0 )
        {
            if ( roi >= n_offsets )
            {
                rc = RC( rcXF, rcFunction, rcExecuting, rcData, rcInvalid );
                break;
            }
            result[ n ] = -ref_offset[ roi ];
        }
        else
        {
            result[ n ] = 0;
        }

        if ( n < nreads - 1 && roi < n_offsets )
        {
            /* scroll through has_ref_offset and consume ref_offsets for this read
               only if there is next read or more offsets */
            uint32_t k;
            for ( k = start; k < start + read_len[ n ]; k++ )
            {
                if ( has_ref_offset[ k ] )
                {
                    if ( roi >= n_offsets )
                    {
                        rc = RC( rcXF, rcFunction, rcExecuting, rcData, rcInvalid );
                        break;
                    }
                    roi++;
                }
            }
        }
    }

    return rc;
}


VTRANSFACT_IMPL ( NCBI_align_get_left_soft_clip, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = left_soft_clip_impl;
    rslt->variant = vftRow;
    rslt -> self = NULL;
    rslt -> whack = NULL;
    return 0;
}


VTRANSFACT_IMPL ( NCBI_align_get_left_soft_clip_2, 2, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = left_soft_clip_impl_2;
    rslt->variant = vftRow;
    rslt -> self = NULL;
    rslt -> whack = NULL;
    return 0;
}


/*
 * function INSDC:coord:len NCBI:align:get_right_soft_clip #1 ( 
 *     bool has_mismatch, INSDC:coord:len left_clip )
 */
static
rc_t CC right_soft_clip_impl ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    INSDC_coord_len result = 0;
    INSDC_coord_len left = 0;
    uint32_t right = ( uint32_t )argv[ 0 ].u.data.elem_count;
    bool const *has_mismatch = argv[ 0 ].u.data.base;
    bool const *has_ref_offset = NULL;
    int32_t    last_ref_offset = 0;
    
    assert( argv[ 0 ].u.data.elem_bits == sizeof( has_mismatch[ 0 ] ) * 8 );
    has_mismatch += argv[ 0 ].u.data.first_elem;
    
    if ( argc > 2 )
    {
        has_ref_offset = argv[ 2 ].u.data.base;
        has_ref_offset += argv[ 2 ].u.data.first_elem;
        if ( argc > 3 )
        {
            const int32_t *ro = argv[ 3 ].u.data.base;
            ro += argv[ 3 ].u.data.first_elem;
            if ( argv[ 3 ].u.data.elem_count > 0 )
            {
                last_ref_offset = ro[ argv[ 3 ].u.data.elem_count - 1 ];
            }
        }
    }
    
    assert( argv[ 1 ].u.data.elem_bits == sizeof( left ) * 8 );
    left = ( ( INSDC_coord_len const * )argv[ 1 ].u.data.base )[ argv[ 1 ].u.data.first_elem ];
    
    while ( right != left && has_mismatch[ right - 1 ] &&
            ( has_ref_offset == NULL || has_ref_offset[ right - 1 ] == 0 ) )
    {
        ++result;
        --right;
    }

    while ( right > 0 && last_ref_offset < 0 && has_ref_offset[ right - 1 ] == 0 ) /*** some mismatches from left needs to be recovered to cover for inserts **/
    {
        last_ref_offset++;
        right--;
    }

    if ( last_ref_offset < -1 )
    {
        last_ref_offset ++;
        if ( result < (INSDC_coord_len)-last_ref_offset )
            result=0;
        else
            result += last_ref_offset;
    }
    else if ( last_ref_offset > 0 )
    {
        result += last_ref_offset;
    }
    
    rslt->data->elem_bits = sizeof( result ) * 8;
    rslt->elem_count = 1;
    rc = KDataBufferResize( rslt->data, 1 );
    if ( rc == 0 )
        memmove( rslt->data->base, &result, sizeof( result ) );

    return rc;
}


VTRANSFACT_IMPL ( NCBI_align_get_right_soft_clip, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = right_soft_clip_impl;
    rslt->variant = vftRow;
    rslt -> self = NULL;
    rslt -> whack = NULL;
    return 0;
}


VTRANSFACT_IMPL ( NCBI_align_get_right_soft_clip_2, 2, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = right_soft_clip_impl;
    rslt->variant = vftRow;
    rslt -> self = NULL;
    rslt -> whack = NULL;
    return 0;
}


/*
 * function INSDC:coord:len NCBI:align:get_right_soft_clip #3 
 *     ( bool has_ref_offset, I32 ref_offset, INSDC:coord:len ref_len )
 */
static
rc_t CC right_soft_clip_3_impl ( void *data, const VXformInfo *info, int64_t row_id,
                              VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    INSDC_coord_len const ref_len = ( ( INSDC_coord_len const * )argv[ 2 ].u.data.base )[ argv[ 2 ].u.data.first_elem ];
    unsigned const seq_len = ( unsigned const )argv[ 0 ].u.data.elem_count;
    int32_t const *ref_offset  = &( ( int32_t const * )argv[ 1 ].u.data.base )[ argv[ 1 ].u.data.first_elem ];
    unsigned const n = ( unsigned const )argv[ 1 ].u.data.elem_count;
    
    assert( argv[ 1 ].u.data.elem_bits == sizeof( ref_offset[ 0 ] ) * 8 );
    assert( argv[ 2 ].u.data.elem_bits == sizeof( ref_len         ) * 8 );

    rslt->data->elem_bits = rslt->elem_bits;
    rslt->elem_count = 1;
    rc = KDataBufferResize( rslt->data, 1 );
    if ( rc == 0 )
    {
        ( ( INSDC_coord_len * )rslt->data->base )[ 0 ] = right_soft_clip( seq_len, ref_len, n, ref_offset );
    }

    return rc;
}


VTRANSFACT_IMPL ( NCBI_align_get_right_soft_clip_3, 3, 0, 0 )
    ( const void *Self, const VXfactInfo *info, VFuncDesc *rslt,
      const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = right_soft_clip_3_impl;
    rslt->variant = vftRow;
    rslt -> self = NULL;
    rslt -> whack = NULL;
    return 0;
}


static
rc_t CC right_soft_clip_4_impl ( void *data, const VXformInfo *info, int64_t row_id,
                                 VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    INSDC_coord_len* result;
    const bool* has_ref_offset = argv[ 0 ].u.data.base;
    const int32_t* ref_offset = argv[ 1 ].u.data.base;
#if 0
    const uint64_t n_offsets = argv[ 1 ].u.data.elem_count;
#endif
    const INSDC_coord_len* read_len = argv[ 2 ].u.data.base;
    const uint32_t nreads = ( const uint32_t )argv[ 2 ].u.data.elem_count;
    const INSDC_coord_len* ref_len = argv[ 3 ].u.data.base;
    uint32_t n, roi;
    INSDC_coord_zero start;

    assert( argv[ 0 ].u.data.elem_bits == sizeof( has_ref_offset[ 0 ] ) * 8 );
    assert( argv[ 1 ].u.data.elem_bits == sizeof( ref_offset[ 0 ] ) * 8 );
    assert( argv[ 2 ].u.data.elem_bits == sizeof( read_len[ 0 ] ) * 8 );
    assert( argv[ 3 ].u.data.elem_bits == sizeof( ref_len[ 0 ] ) * 8 );
    assert( argv[ 3 ].u.data.elem_count == 1);
    
    has_ref_offset += argv[ 0 ].u.data.first_elem;
    ref_offset += argv[ 1 ].u.data.first_elem;
    read_len += argv[ 2 ].u.data.first_elem;
    ref_len += argv[ 3 ].u.data.first_elem;

    rslt->data->elem_bits = sizeof( *result ) * 8;
    rslt->elem_count = nreads;
    rc = KDataBufferResize( rslt->data, rslt->elem_count );
    result = rslt->data->base;

    for( n = roi = start = 0; rc == 0 && n < nreads; ++n )
    {
        INSDC_coord_len const rlen = read_len[ n ];
        unsigned k;
        unsigned offset_count;
        
        for ( k = offset_count = 0; k < rlen; ++k )
        {
            if ( has_ref_offset[ k + start ] )
                ++offset_count;
        }

        result[ n ] = right_soft_clip( rlen, ref_len[ 0 ], offset_count, ref_offset );
        ref_offset += offset_count;
        start += rlen;
#if 0
        result[ n ] = read_len[ n ] - ref_len[ 0 ];
        for ( k = start; k < start + read_len[ n ]; k++ )
        {
            if ( has_ref_offset[ k ] )
            {
                if ( roi >= n_offsets )
                {
                    rc = RC( rcXF, rcFunction, rcExecuting, rcData, rcInvalid );
                    break;
                }
                result[ n ] += ref_offset[ roi++ ];
            }
        }
#endif
    }

    return rc;
}

VTRANSFACT_IMPL ( NCBI_align_get_right_soft_clip_4, 4, 0, 0 )
    ( const void *Self, const VXfactInfo *info, VFuncDesc *rslt,
      const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = right_soft_clip_4_impl;
    rslt->variant = vftRow;
    rslt -> self = NULL;
    rslt -> whack = NULL;
    return 0;
}

/* extern function INSDC:coord:len NCBI:align:get_right_soft_clip #5
 *     ( bool has_ref_offset, I32 ref_offset, NCBI:align:ro_type ref_offset_type,
 *		 INSDC:coord:len read_len )
 */
static
rc_t CC right_soft_clip_5_impl ( void *data, const VXformInfo *info, int64_t row_id,
                                VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    unsigned const nreads = argv[3].u.data.elem_count;
    ARG_ALIAS(              bool,  has_ref_offset, 0);
    ARG_ALIAS(           int32_t,      ref_offset, 1);
    ARG_ALIAS(NCBI_align_ro_type, ref_offset_type, 2);
    ARG_ALIAS(   INSDC_coord_len,        read_len, 3);
    
    rslt->data->elem_bits = sizeof(INSDC_coord_len) * 8;
    rslt->elem_count = nreads;
    rc = KDataBufferResize(rslt->data, nreads);
    if (rc == 0) {
        INSDC_coord_len *const result = rslt->data->base;
        unsigned cur = 0;
        unsigned cur_ro = 0;
        unsigned n;
        
        for (n = 0; n < nreads; ++n) {
            unsigned const len = read_len[n];
            unsigned clip = 0;
            unsigned j;
            
            for (j = 0; j < len; ++j, ++cur) {
                if (has_ref_offset[cur] != 0) {
                    int const offset = ref_offset[cur_ro];
                    int const type = ref_offset_type[cur_ro];
                    
                    ++cur_ro;
                    if (j > 0 && offset < 0 && type == 1) {
                        assert(clip == 0);
                        clip = -offset;
                    }
                }
            }
            result[n] = clip;
        }
    }
    
    return rc;
}

VTRANSFACT_IMPL(NCBI_align_get_right_soft_clip_5, 5, 0, 0 )
    (const void *Self, const VXfactInfo *info, VFuncDesc *rslt,
     const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = right_soft_clip_5_impl;
    rslt->variant = vftRow;
    rslt -> self = NULL;
    rslt -> whack = NULL;
    return 0;
}

/*
 * function ascii NCBI:align:get_clipped_cigar #1 ( ascii cigar )
 */
static
rc_t CC clipped_cigar_impl ( void *data, const VXformInfo *info, int64_t row_id,
                              VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    char const *cigar = argv[ 0 ].u.data.base;
    unsigned const ciglen = ( unsigned const )argv[ 0 ].u.data.elem_count;
    int n;
    unsigned start = 0;
    unsigned end = ciglen;
    
    assert( argv[ 0 ].u.data.elem_bits == sizeof( cigar[ 0 ] ) * 8 );
    cigar += argv[ 0 ].u.data.first_elem;
    
    for ( n = 0; n != ciglen; ++n )
    {
        if ( !isdigit( cigar[ n ] ) )
            break;
    }

    if ( cigar[ n ] == 'S' )
        start = n + 1;
    
    if ( cigar[ end - 1 ] == 'S' )
    {
        --end;
        while ( end > start && isdigit( cigar[ end - 1 ] ) )
            --end;
    }
    
    rslt->data->elem_bits = sizeof( cigar[ 0 ] ) * 8;
    rslt->elem_count = ( end > start ) ? end - start : 0;
    rc = KDataBufferResize( rslt->data, rslt->elem_count );
    if ( rc == 0 && rslt->elem_count > 0 )
        memmove( rslt->data->base, &cigar[ start ], ( size_t )rslt->elem_count );
    
    return rc;
}


static int remove_left_soft_clip(int const length, char const cigar[ /* length */ ])
{
    int i;
    for (i = 0; i < length; ++i) {
        int const ch = cigar[i];
        if (ch < '0' || ch > '9')
            break;
    }
    if (i < length && cigar[i] == 'S')
        return i + 1;
    else
        return 0;
}

static int remove_right_soft_clip(int const length, char const cigar[ /* length */ ])
{
    if (length > 0 && cigar[length - 1] == 'S') {
        int i = length - 1;
        while (i > 0) {
            int const ch = cigar[i - 1];
            if (ch < '0' || ch > '9')
                break;
            --i;
        }
        return i;
    }
    return length;
}

static
rc_t CC clipped_cigar_impl_v2 ( void *data, const VXformInfo *info, int64_t row_id,
                                VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    VRowData const *const argCigar = &argv[0];
    VRowData const *const argCigLen = &argv[1];
    char const* cigar = argCigar->u.data.base;
    INSDC_coord_len const *cigLen = argCigLen->u.data.base;
    int const N = (int)argCigLen->u.data.elem_count;
    rc_t rc = 0;

    assert(argCigar->u.data.elem_bits == sizeof(cigar[0]) * 8);
    assert(argCigLen->u.data.elem_bits == sizeof(cigLen[0]) * 8);

    if (argCigar->u.data.elem_count == 0) {
        rslt->elem_count = 0;
        return KDataBufferResize(rslt->data, 0);
    }

    cigar += argCigar->u.data.first_elem;
    cigLen += argCigLen->u.data.first_elem;

    if (data != NULL)
    { /* outputting the lengths */
        rslt->data->elem_bits = sizeof(cigLen[0]) * 8;
        rslt->elem_count = N;
        rc = KDataBufferResize(rslt->data, rslt->elem_count);
        if (rc == 0) {
            INSDC_coord_len *const out = rslt->data->base;
            int i;

            for (i = 0; i < N; ++i) {
                int const len = cigLen[i];
                int const rlsc = remove_left_soft_clip(len, cigar);
                int const rrsc = remove_right_soft_clip(len, cigar);

                out[i] = (INSDC_coord_len)(rrsc > rlsc ? (rrsc - rlsc) : 0);
                cigar += len;
            }
        }
    }
    else
    { /* outputting the strings */
        int i;
        
        rslt->data->elem_bits = sizeof(cigar[0]) * 8;
        rslt->elem_count = 0;

        for (i = 0; i < N; ++i) {
            int const len = cigLen[i];
            int const rlsc = remove_left_soft_clip(len, cigar);
            int const rrsc = remove_right_soft_clip(len, cigar);
            int const newLen = rrsc > rlsc ? (rrsc - rlsc) : 0;
            char *out;

            rc = KDataBufferResize(rslt->data, rslt->elem_count + newLen);
            if (rc) return rc;
            out = rslt->data->base;
            memmove(&out[rslt->elem_count], &cigar[rlsc], (size_t)newLen);
            rslt->elem_count += newLen;
            cigar += len;
        }
    }
    return rc;
}


VTRANSFACT_IMPL ( NCBI_align_get_clipped_cigar, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = clipped_cigar_impl;
    rslt->variant = vftRow;
    rslt -> self = NULL;
    rslt -> whack = NULL;
    return 0;
}


VTRANSFACT_IMPL ( NCBI_align_get_clipped_cigar_2, 2, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    const VTypedesc* tp = &info->fdesc.desc;

    if ( tp->domain == vtdAscii && tp->intrinsic_bits == 8 )
    {
        rslt->self = NULL;
    }
    else if ( tp->domain == vtdUint && tp->intrinsic_bits == sizeof( INSDC_coord_len ) * 8 )
    {
        rslt->self = rslt; /* something different from NULL :) */
    }
    else
    {
        return RC( rcXF, rcFunction, rcConstructing, rcParam, rcIncorrect );
    }
    rslt->u.rf = clipped_cigar_impl_v2;
    rslt->variant = vftRow;
    rslt->whack = NULL;
    return 0;
}



/*
 * function I32 NCBI:align:get_clipped_ref_offset #1 (
 *     bool has_ref_offset, I32 ref_offset )
 */
static
rc_t CC clipped_ref_offset_impl ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    unsigned start = 0;
    unsigned const n_offsets = ( unsigned const )argv[ 1 ].u.data.elem_count;
    int32_t const *ref_offset = argv[ 1 ].u.data.base;
    
    assert( argv[ 1 ].u.data.elem_bits == sizeof( ref_offset[ 0 ] ) * 8 );
    ref_offset += argv[ 1 ].u.data.first_elem;
    
    if ( n_offsets > 0 )
    {
        bool const *has_ref_offset = argv[ 0 ].u.data.base;
        
        assert( argv[ 0 ].u.data.elem_bits == sizeof( has_ref_offset[ 0 ] ) * 8 );
        has_ref_offset += argv[ 0 ].u.data.first_elem;
        
        if ( has_ref_offset[ 0 ] && ref_offset[ 0 ] < 0 )
        {
            start = 1;
        }
    }

    rslt->data->elem_bits = sizeof( ref_offset[ 0 ] ) * 8;
    rslt->elem_count = n_offsets - start;
    rc = KDataBufferResize( rslt->data, rslt->elem_count );
    if ( rc == 0 )
        memmove( rslt->data->base,
                &ref_offset[ start ],
                ( size_t )( sizeof( ref_offset[ 0 ] ) * rslt->elem_count ) );

    return rc;
}


VTRANSFACT_IMPL ( NCBI_align_get_clipped_ref_offset, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = clipped_ref_offset_impl;
    rslt->variant = vftRow;
    rslt -> self = NULL;
    rslt -> whack = NULL;
    return 0;
}


/*
 * function INSDC:coord:len NCBI:align:get_ref_len #1 (
 *     bool has_ref_offset, I32 ref_offset, INSDC:coord:len right_clip )
 */
static
rc_t CC get_ref_len_impl ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    INSDC_coord_len result,right=0;
    
    unsigned const read_len = ( unsigned const )argv[ 0 ].u.data.elem_count;
    unsigned const n_offsets = ( unsigned const )argv[ 1 ].u.data.elem_count;
    bool const *hro = argv[ 0 ].u.data.base;
    int32_t const *ref_offset = argv[ 1 ].u.data.base;
    
    assert( argv[ 1 ].u.data.elem_bits == sizeof( ref_offset[ 0 ] ) * 8 );
    
    hro += argv[ 0 ].u.data.first_elem;
    ref_offset += argv[ 1 ].u.data.first_elem;

    if ( argc > 2 )
    {
        right = ( ( INSDC_coord_len* )argv[ 2 ].u.data.base )[ argv[ 2 ].u.data.first_elem ];
        
        assert( argv[ 2 ].u.data.elem_bits == sizeof( right ) * 8 );
        assert( read_len >= right );
    }

    if ( n_offsets == 0 )
    {
        if ( read_len < right )
            return RC( rcXF, rcFunction, rcExecuting, rcData, rcInvalid );
        result = read_len - right;
    }
    else
    {
        int32_t  ires, rov;
        unsigned i;
        
        if ( argc > 2 ) /*** clipping ***/
        {
            for ( i = 0, ires = read_len - right; i < n_offsets; i++ )
            {
                memmove( &rov, ref_offset + i, sizeof rov );
                ires += rov;
            }
        }
        else
        {
            int32_t sum_pos, sum_neg;
            for ( i = 0, sum_pos = sum_neg = 0; i < n_offsets; i++ )
            {
                memmove( &rov, ref_offset + i, sizeof rov );
                if ( rov > 0 )
                    sum_pos += rov;
                else
                    sum_neg += rov;            
            }

            if ( sum_pos + sum_neg >= 0 ) /** all offsets may not over-shorten needed reference ***/
            {
                ires = read_len + sum_pos + sum_neg;
            }
            else /** inefficient case - exact reach into the reference is needed **/
            {
                unsigned j;
                for ( i = 0, j = 0, sum_pos = 0, ires = 0; j < read_len; j++ )
                {
                    if ( hro[ i ] )
                    {
                        if ( i >= n_offsets )
                            return RC( rcXF, rcFunction, rcExecuting, rcData, rcInvalid );
                        memmove( &rov, ref_offset + i, sizeof rov );
                        ires += rov;
                        i++;
                    }
                    sum_pos ++;
                    if ( sum_pos > ires )
                        ires=sum_pos;
                }
            }
        }

        if ( ires < 0 )
            return RC( rcXF, rcFunction, rcExecuting, rcData, rcInvalid );
        result = ires;
    }

    rslt->data->elem_bits = sizeof( result ) * 8;
    rslt->elem_count = 1;
    rc = KDataBufferResize( rslt->data, rslt->elem_count );
    if ( rc == 0 )
        memmove( rslt->data->base, &result, sizeof( result ) );

    return rc;
}


VTRANSFACT_IMPL ( NCBI_align_get_ref_len, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = get_ref_len_impl;
    rslt->variant = vftRow;
    rslt -> self = NULL;
    rslt -> whack = NULL;
    return 0;
}


/*
 * function INSDC:coord:len NCBI:align:get_ref_len #2 (
 *											bool has_ref_offset,
 *											I32 ref_offset)
 */
static
rc_t CC get_ref_len_2_impl ( void *data, const VXformInfo *info, int64_t row_id,
                          VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    ARG_ALIAS(int32_t, refOffset, 1);
    unsigned const n = (unsigned)argv[1].u.data.elem_count;
    int result = (unsigned)argv[0].u.data.elem_count; 
    unsigned i;

    for (i = 0; i < n; ++i)
        result += (int)refOffset[i];
    
    rslt->data->elem_bits = sizeof(INSDC_coord_len) * 8;
    rc = KDataBufferResize(rslt->data, rslt->elem_count = 1);
    if (rc == 0)
        ((INSDC_coord_len *)rslt->data->base)[0] = result;

    return rc;
}


VTRANSFACT_IMPL(NCBI_align_get_ref_len_2, 2, 0, 0)(const void *Self, const VXfactInfo *info,
                     VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp)
{
    rslt->u.rf = get_ref_len_2_impl;
    rslt->variant = vftRow;
    rslt -> self = NULL;
    rslt -> whack = NULL;
    return 0;
}


/*
 * function < type T > T NCBI:align:clip #1 ( T value,
 *     INSDC:coord:len left, INSDC:coord:len right )
 */
static
rc_t CC clip_impl ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    INSDC_coord_len left  = ( ( INSDC_coord_len const * )argv[ 1 ].u.data.base )[ argv[ 1 ].u.data.first_elem ];
    INSDC_coord_len right = ( ( INSDC_coord_len const * )argv[ 2 ].u.data.base )[ argv[ 2 ].u.data.first_elem ];

    rslt->data->elem_bits = argv[ 0 ].u.data.elem_bits;
    if ( argv[ 0 ].u.data.elem_count >= left + right )
    {
        rslt->elem_count = argv[ 0 ].u.data.elem_count - left - right;
    }
    else
    {
        rslt->elem_count = 0;
    }

    rc = KDataBufferResize( rslt->data, rslt->elem_count );
    if ( rc == 0 )
    {
        if ( ( rslt->data->elem_bits & 7 ) == 0 )
        {
            memmove( rslt->data->base,
                    &( ( char const * )argv[ 0 ].u.data.base )[ ( ( left + argv[ 0 ].u.data.first_elem ) * argv[ 0 ].u.data.elem_bits ) >> 3 ],
                    ( size_t )( ( rslt->elem_count * rslt->data->elem_bits ) >> 3 ) );
        }
        else
        {
            bitcpy( rslt->data->base, 
                    0,
                    argv[ 0 ].u.data.base,
                    ( left + argv[ 0 ].u.data.first_elem ) * argv[ 0 ].u.data.elem_bits,
                    rslt->elem_count * rslt->data->elem_bits );
        }
    }
    return rc;
}


VTRANSFACT_IMPL ( NCBI_align_clip, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = clip_impl;
    rslt->variant = vftRow;
    rslt -> self = NULL;
    rslt -> whack = NULL;
    return 0;
}


static
rc_t CC clip_impl_2 ( void *data, const VXformInfo *info, int64_t row_id,
                      VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    const INSDC_coord_len* read_len = argv[ 1 ].u.data.base;
    const uint32_t nreads = ( const uint32_t )argv[ 1 ].u.data.elem_count;
    const INSDC_coord_len* left =  argv[ 2 ].u.data.base;
    const INSDC_coord_len* right = argv[ 3 ].u.data.base;
    uint32_t n;
    INSDC_coord_zero start;

    assert( argv[ 1 ].u.data.elem_count == argv[ 2 ].u.data.elem_count );
    assert( argv[ 1 ].u.data.elem_count == argv[ 3 ].u.data.elem_count );
    assert( argv[ 1 ].u.data.elem_bits == sizeof( read_len[ 0 ] ) * 8 );
    assert( argv[ 2 ].u.data.elem_bits == sizeof( left[ 0 ] ) * 8 );
    assert( argv[ 2 ].u.data.elem_bits == sizeof( right[ 0 ] ) * 8 );

    read_len += argv[ 1 ].u.data.first_elem;
    left += argv[ 2 ].u.data.first_elem;
    right += argv[ 3 ].u.data.first_elem;

    rslt->data->elem_bits = argv[ 0 ].u.data.elem_bits;
    rslt->elem_count = 0;
    rc = KDataBufferResize( rslt->data, rslt->elem_count );
    if ( rc != 0 )
        return rc;
    if ( argv[ 0 ].u.data.elem_count == 0 )
        return 0;

    for ( n = start = 0; rc == 0 && n < nreads; start += read_len[ n++ ] )
    {
        uint64_t x = left[n] + right[n];

        if ( argv[ 0 ].u.data.elem_count < start ||
             argv[ 0 ].u.data.elem_count < start + read_len[ n ] )
        {
            rc = RC( rcXF, rcFunction, rcExecuting, rcData, rcInvalid );
        }
/*
        assert(argv[0].u.data.elem_count >= start);
        assert(argv[0].u.data.elem_count >= start + read_len[n]);
*/
        else if ( read_len[ n ] > x )
        {
            x = read_len[ n ] - x;
            if ( x > 0 )
            {
                rc = KDataBufferResize( rslt->data, rslt->elem_count + x );
                if ( rc == 0 )
                {
                    bitcpy( rslt->data->base,
                            rslt->elem_count * argv[ 0 ].u.data.elem_bits,
                            argv[ 0 ].u.data.base,
                            ( argv[ 0 ].u.data.first_elem + start + left[ n ] ) * argv[ 0 ].u.data.elem_bits,
                            x * rslt->data->elem_bits );
                    rslt->elem_count += x;
                }
            }
        }
    }

    return rc;
}


VTRANSFACT_IMPL ( NCBI_align_clip_2, 2, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = clip_impl_2;
    rslt->variant = vftRow;
    rslt -> self = NULL;
    rslt -> whack = NULL;
    return 0;
}


/*
 * function bool NCBI:align:get_ref_mismatch #1
 *     ( bool has_mismatch, bool has_ref_offset, I32 ref_offset,
 *       INSDC:coord:len ref_len )
 */
static
rc_t CC get_ref_mismatch_impl ( void *data, const VXformInfo *info, int64_t row_id,
                   VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    bool const *has_mismatch       = argv [ 0 ] . u . data . base;
    bool const *has_ref_offset     = argv [ 1 ] . u . data . base;
    int32_t const *ref_offset      = argv [ 2 ] . u . data . base;
    INSDC_coord_len const *ref_len = argv [ 3 ] . u . data . base; 
    
    assert( argv[ 0 ].u.data.elem_bits == sizeof( has_mismatch[ 0 ] ) * 8 );
    assert( argv[ 1 ].u.data.elem_bits == sizeof( has_ref_offset[ 0 ] ) * 8 );
    assert( argv[ 2 ].u.data.elem_bits == sizeof( ref_offset[ 0 ] ) * 8 );
    assert( argv[ 3 ].u.data.elem_bits == sizeof( ref_len[ 0 ] ) * 8 );
    
    has_mismatch   += argv[ 0 ].u.data.first_elem;
    has_ref_offset += argv[ 1 ].u.data.first_elem;
    ref_offset     += argv[ 2 ].u.data.first_elem;
    
    rslt->data->elem_bits = sizeof( bool ) * 8;
    rslt->elem_count = ref_len[ argv[ 3 ].u.data.first_elem ];
    rc = KDataBufferResize( rslt->data, rslt->elem_count );
    if ( rc == 0 )
    {
        bool *result = ( bool* )rslt->data->base;
        unsigned j;
        unsigned ri;
        unsigned si;
        
        memset( result, 0, ( size_t )( sizeof( result[ 0 ] ) * rslt->elem_count ) );
        for ( j = ri = si = 0; si < argv[ 0 ].u.data.elem_count; )
        {
            if ( has_ref_offset[ si ] )
            {
                int offset = ref_offset[ j++ ];
                
                if ( offset > 0 )
                    ri += offset;
                else
                {
                    si -= offset;
                    continue;
                }
            }
            if ( ri >= rslt->elem_count )
                break;
            if ( has_mismatch[ si ] )
            {
                result[ ri ] = 1;
            }
            ++si;
            ++ri;
        }
    }

    return rc;
}


VTRANSFACT_IMPL ( NCBI_align_get_ref_mismatch, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = get_ref_mismatch_impl;
    rslt->variant = vftRow;
    rslt -> self = NULL;
    rslt -> whack = NULL;
    return 0;
}



/*
 * function bool NCBI:align:get_ref_insert #1
 *     ( bool has_mismatch, bool has_ref_offset, I32 ref_offset,
 *       INSDC:coord:len ref_len )
 */
static
rc_t CC get_ref_insert_impl ( void *data, const VXformInfo *info, int64_t row_id,
                               VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    bool const *has_mismatch       = argv [ 0 ] . u . data . base;
    bool const *has_ref_offset     = argv [ 1 ] . u . data . base;
    int32_t const *ref_offset      = argv [ 2 ] . u . data . base;
    INSDC_coord_len const *ref_len = argv [ 3 ] . u . data . base; 
    
    assert( argv[ 0 ].u.data.elem_bits == sizeof( has_mismatch[ 0 ] ) * 8 );
    assert( argv[ 1 ].u.data.elem_bits == sizeof( has_ref_offset[ 0 ] ) * 8 );
    assert( argv[ 2 ].u.data.elem_bits == sizeof( ref_offset[ 0 ] ) * 8 );
    assert( argv[ 3 ].u.data.elem_bits == sizeof( ref_len[ 0 ] ) * 8 );
    
    has_mismatch   += argv[ 0 ].u.data.first_elem;
    has_ref_offset += argv[ 1 ].u.data.first_elem;
    ref_offset     += argv[ 2 ].u.data.first_elem;
    
    rslt->data->elem_bits = sizeof( bool ) * 8;
    rslt->elem_count = ref_len[ argv[ 3 ].u.data.first_elem ];
    rc = KDataBufferResize( rslt->data, rslt->elem_count );
    if (rc == 0 && rslt->elem_count > 0) {
        bool *result = ( bool* )rslt->data->base;
        unsigned j;
        unsigned ri;
        unsigned si;
        
        memset( result, 0, ( size_t )( sizeof( result[ 0 ] ) * rslt->elem_count ) );
        for ( j = ri = si = 0; si < argv[ 0 ].u.data.elem_count; )
        {
            if ( has_ref_offset[ si ] )
            {
                int offset = ref_offset[ j++ ];
                
                if ( offset > 0 )
                {
                    ri += offset;
                }
                else
                {
                    if ( si > 0 )
                    {
                        if ( ri >= 1 )
                            result[ ri - 1 ] = 1;
                        result[ ri ] = 1;
                    }
                    si -= offset;
                    continue;
                }
            }
            ++si;
            ++ri;
        }
    }
    return rc;
}


VTRANSFACT_IMPL ( NCBI_align_get_ref_insert, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = get_ref_insert_impl;
    rslt->variant = vftRow;
    rslt -> self = NULL;
    rslt -> whack = NULL;
    return 0;
}



/*
 * function bool NCBI:align:get_ref_delete #1
 *     ( bool has_mismatch, bool has_ref_offset, I32 ref_offset,
 *       INSDC:coord:len ref_len )
 */
static
rc_t CC get_ref_delete_impl ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    bool const *has_mismatch       = argv [ 0 ] . u . data . base;
    bool const *has_ref_offset     = argv [ 1 ] . u . data . base;
    int32_t const *ref_offset      = argv [ 2 ] . u . data . base;
    INSDC_coord_len const *ref_len = argv [ 3 ] . u . data . base; 
    
    assert( argv[ 0 ].u.data.elem_bits == sizeof( has_mismatch[ 0 ] ) * 8 );
    assert( argv[ 1 ].u.data.elem_bits == sizeof( has_ref_offset[ 0 ] ) * 8 );
    assert( argv[ 2 ].u.data.elem_bits == sizeof( ref_offset[ 0 ] ) * 8 );
    assert( argv[ 3 ].u.data.elem_bits == sizeof( ref_len[ 0 ] ) * 8 );
    
    has_mismatch   += argv[ 0 ].u.data.first_elem;
    has_ref_offset += argv[ 1 ].u.data.first_elem;
    ref_offset     += argv[ 2 ].u.data.first_elem;
    
    rslt->data->elem_bits = sizeof( bool ) * 8;
    rslt->elem_count = ref_len[ argv[ 3 ].u.data.first_elem ];
    rc = KDataBufferResize( rslt->data, rslt->elem_count );
    if ( rc == 0 )
    {
        bool *result = ( bool* )rslt->data->base;
        unsigned j;
        unsigned ri;
        unsigned si;
        
        memset( result, 0, ( size_t )( sizeof( result[ 0 ] ) * rslt->elem_count ) );
        for ( j = ri = si = 0; si < argv[ 0 ].u.data.elem_count; )
        {
            if ( has_ref_offset[ si ] )
            {
                int offset = ref_offset[ j++ ];
                
                if ( offset > 0 )
                {
                    memset( &result[ ri ], 1, offset );
                    ri += offset;
                }
                else
                {
                    si -= offset;
                    continue;
                }
            }
            ++si;
            ++ri;
        }
    }

    return rc;
}


VTRANSFACT_IMPL ( NCBI_align_get_ref_delete, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = get_ref_delete_impl;
    rslt->variant = vftRow;
    rslt -> self = NULL;
    rslt -> whack = NULL;
    return 0;
}



#define USE_BIGGER_PRESERVE_BORDER 1
/*
 * function bool NCBI:align:get_preserve_qual #1
 *     ( bool has_mismatch, bool has_ref_offset, I32 ref_offset,
 *       INSDC:coord:len ref_len )
 */
static
rc_t CC get_ref_preserve_qual_impl ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    bool const *has_mismatch       = argv [ 0 ] . u . data . base;
    bool const *has_ref_offset     = argv [ 1 ] . u . data . base;
    int32_t const *ref_offset      = argv [ 2 ] . u . data . base;
    INSDC_coord_len const *ref_len = argv [ 3 ] . u . data . base; 
    
    assert( argv[ 0 ].u.data.elem_bits == sizeof( has_mismatch[ 0 ] ) * 8 );
    assert( argv[ 1 ].u.data.elem_bits == sizeof( has_ref_offset[ 0 ] ) * 8 );
    assert( argv[ 2 ].u.data.elem_bits == sizeof( ref_offset[ 0 ] ) * 8 );
    assert( argv[ 3 ].u.data.elem_bits == sizeof( ref_len[ 0 ] ) * 8 );
    
    has_mismatch   += argv[ 0 ].u.data.first_elem;
    has_ref_offset += argv[ 1 ].u.data.first_elem;
    ref_offset     += argv[ 2 ].u.data.first_elem;
    
    rslt->data->elem_bits = sizeof( bool ) * 8;
    rslt->elem_count = ref_len[ argv[ 3 ].u.data.first_elem ];
    rc = KDataBufferResize( rslt->data, rslt->elem_count );
    if ( rc == 0 )
    {
        bool *result = ( bool* )rslt->data->base;
        unsigned j;
        unsigned ri;
        unsigned si;
        
        memset( result, 0, ( size_t )( sizeof( result[ 0 ] ) * rslt->elem_count ) );
        for ( j = ri = si = 0; si < argv[ 0 ].u.data.elem_count; )
        {
            if ( has_ref_offset[ si ] )
            {
                int offset = ref_offset[ j++ ];
                
                if ( offset > 0 )
                {
                    /* Preserve the qualities for deleted bases + plus the border */
#if USE_BIGGER_PRESERVE_BORDER
                    if ( ri >= 2 ) result[ ri - 2 ] = 1;
#endif
                    if ( ri >= 1 ) result[ ri - 1 ] = 1;
                    memset( &result[ ri ], 1, offset );
                    ri += offset;
                    result[ ri ] = 1;
#if USE_BIGGER_PRESERVE_BORDER
                    if ( ri + 1 < rslt->elem_count ) result[ ri + 1 ] = 1;
#endif
                }
                else
                {
                    if ( si > 0 )
                    {
                        /* Preserve the qualites for the bases bordering the insert */
#if USE_BIGGER_PRESERVE_BORDER
                        if ( ri >= 2 ) result[ ri - 2 ] = 1;
#endif
                        if ( ri >= 1 ) result[ ri - 1 ] = 1;
                        result[ ri ] = 1;
#if USE_BIGGER_PRESERVE_BORDER
                        if ( ri + 1 < rslt->elem_count ) result[ ri + 1 ] = 1;
#endif
                    }
                    si -= offset;
                    continue;
                }
            }
            if ( ri >= rslt->elem_count ) break;
            if ( has_mismatch[ si ] )
            {
#if USE_BIGGER_PRESERVE_BORDER
                if ( ri >= 1 ) result[ ri - 1 ] = 1;
#endif
                result[ ri ] = 1;
#if USE_BIGGER_PRESERVE_BORDER
                if ( ri + 1 < rslt->elem_count ) result[ ri + 1 ] = 1;
#endif
            }
            ++si;
            ++ri;
        }
    }
    return rc;
}


VTRANSFACT_IMPL ( NCBI_align_get_ref_preserve_qual, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = get_ref_preserve_qual_impl;
    rslt->variant = vftRow;
    rslt -> self = NULL;
    rslt -> whack = NULL;
    return 0;
}



/*
 * function bool NCBI:align:get_seq_preserve_qual #1
 *    ( bool ref_preserve_qual, bool has_ref_offset, I32 ref_offset );
 */
static
rc_t CC get_seq_preserve_qual_impl ( void *data, const VXformInfo *info, int64_t row_id,
                                    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    bool const *ref_pq             = argv [ 0 ] . u . data . base;
    bool const *has_ref_offset     = argv [ 1 ] . u . data . base;
    int32_t const *ref_offset      = argv [ 2 ] . u . data . base;
    
    assert( argv[ 0 ].u.data.elem_bits == sizeof( ref_pq[ 0 ] ) * 8 );
    assert( argv[ 1 ].u.data.elem_bits == sizeof( has_ref_offset[ 0 ] ) * 8 );
    assert( argv[ 2 ].u.data.elem_bits == sizeof( ref_offset[ 0 ] ) * 8 );
    
    ref_pq         += argv[ 0 ].u.data.first_elem;
    has_ref_offset += argv[ 1 ].u.data.first_elem;
    ref_offset     += argv[ 2 ].u.data.first_elem;
    
    rslt->data->elem_bits = sizeof( bool ) * 8;
    rslt->elem_count = argv[ 1 ].u.data.elem_count;
    rc = KDataBufferResize( rslt->data, rslt->elem_count );
    if ( rc == 0 )
    {
        bool *result = ( bool* )rslt->data->base;
        unsigned j;
        unsigned ri;
        unsigned si;
        
        memset( result, 1, ( size_t )( sizeof( result[ 0 ] ) * rslt->elem_count ) );
        for ( j = ri = si = 0; si < argv[ 1 ].u.data.elem_count; )
        {
            if ( has_ref_offset[ si ] )
            {
                int offset = ref_offset[ j++ ];
                
                if ( offset > 0 )
                {
                    ri += offset;
                }
                else
                {
                    si -= offset;
                    continue;
                }
            }
            if ( ri >= argv[ 0 ].u.data.elem_count ) break;
            result[ si ] = ref_pq[ ri ];
            ++si;
            ++ri;
        }
    }

    return rc;
}

VTRANSFACT_IMPL ( NCBI_align_get_seq_preserve_qual, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
                                                               VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = get_seq_preserve_qual_impl;
    rslt->variant = vftRow;
    rslt -> self = NULL;
    rslt -> whack = NULL;
    return 0;
}



/*
 * function ascii NCBI:align:rna_orientation #1 ( NCBI:align:ro_type *ref_offset_type );
 */
static
rc_t CC get_rna_orientation(void *data, const VXformInfo *info, int64_t row_id,
                        VRowResult *rslt, uint32_t argc, const VRowData argv [])
{
    rc_t rc;
    uint8_t const *const offset_type = argc == 1 ? ((uint8_t const *)argv[0].u.data.base) + argv[0].u.data.first_elem : NULL;
    unsigned const count = argc == 1 ? (unsigned)argv[0].u.data.elem_count : 0;

    assert(argv[ 0 ].u.data.elem_bits == sizeof(offset_type[0]) * 8);
    
    rslt->data->elem_bits = sizeof(char) * 8;
    rslt->elem_count = 1;
    rc = KDataBufferResize(rslt->data, 1);
    if ( rc == 0 )
    {
        unsigned p_count = 0;
        unsigned m_count = 0;
    	unsigned i;
        char *orient = rslt->data->base;
        
        for (i = 0; i < count; ++i) {
            if (offset_type[i] == NCBI_align_ro_intron_plus)
                ++p_count;
            else if (offset_type[i] == NCBI_align_ro_intron_minus)
                ++m_count;
        }
        if (p_count > 0 && m_count == 0)
            *orient = '+';
        else if (m_count > 0 && p_count == 0)
            *orient = '-';
        else
            rslt->elem_count = 0;
    }
    
    return rc;
}

VTRANSFACT_IMPL ( NCBI_align_rna_orientation, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = get_rna_orientation;
    rslt->variant = vftRow;
    rslt -> self = NULL;
    rslt -> whack = NULL;
    return 0;
}
