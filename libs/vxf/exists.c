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
#include <klib/data-buffer.h>
#include <vdb/xform.h>
#include <vdb/schema.h>
#include <sysalloc.h>
#include <bitstr.h>

#include <stdint.h>
#include <stdlib.h>

struct self_t {
    KDataBuffer val;
    bitsz_t csize;
    bitsz_t dsize;
    int count;
};

static void CC self_free( void *Self ) {
    struct self_t *self = Self;
    
    KDataBufferWhack(&self->val);
    free(self);
}

static void fill(struct self_t *self, int start) {
    int i;
    
    for (i = start; i != self->count; ++i)
        bitcpy(self->val.base, 0, self->val.base, i * self->csize, self->csize);
}

static rc_t grow_and_fill(struct self_t *self, int rlen) {
    bitsz_t rsize = rlen * self->dsize;
    rc_t rc;
    
    if (self->count * self->csize <= rsize) {
        int old_count = self->count;
        int new_count = old_count;
        
        while (new_count * self->csize <= rsize)
            new_count <<= 1;
        
        rc = KDataBufferResize(&self->val, new_count);
        if (rc)
            return rc;
        
        self->count = new_count;
        fill(self, old_count);
    }
    return 0;
}

/* exists
 function < type T > T exits #1.0 < T val > ( any predicate, any row_len );
 */
static
rc_t CC exists_func_1(
                 void *Self,
                 const VXformInfo *info,
                 int64_t row_id,
                 VRowResult *rslt,
                 uint32_t argc,
                 const VRowData argv[]
) {
    struct self_t *self = Self;
    rc_t rc = grow_and_fill(self, argv[1].u.data.elem_count);
    
    if (rc == 0) {
        KDataBufferWhack(rslt->data);
        rslt->elem_count = argv[1].u.data.elem_count;
        rc = KDataBufferSub(&self->val, rslt->data, 0, rslt->elem_count);
    }
    return rc;
}

/* exists
 function < type T > T exists #1.0 < T val > ( any predicate );
 */
static
rc_t CC exists_func_0(
                 void *Self,
                 const VXformInfo *info,
                 int64_t row_id,
                 VRowResult *rslt,
                 uint32_t argc,
                 const VRowData argv[]
) {
    struct self_t *self = Self;

    KDataBufferWhack(rslt->data);
    rslt->elem_count = (uint32_t)( self->csize / self->dsize );
    return KDataBufferSub(&self->val, rslt->data, 0, rslt->elem_count);
}

rc_t exists_row_0 ( const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp )
{
    rc_t rc;
    struct self_t *self = malloc ( sizeof *self );
    if ( self == NULL )
        return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);

    self->dsize = VTypedescSizeof ( & cp->argv[0].desc );
    self->csize = self->dsize * cp->argv[0].count;
    self->count = 1;

    rc = KDataBufferMake(&self->val, self->dsize, cp->argv[0].count);
    if (rc == 0) {
        bitcpy(self->val.base, 0, cp->argv[0].data.u8, 0, self->csize);
    
        rslt->self = self;
        rslt->whack = self_free;
        rslt->variant = vftRow;
        rslt->u.rf = exists_func_0;
        return 0;
    }
    free(self);
    return rc;
}

static
rc_t exists_row_1 ( const VXfactInfo *info, VFuncDesc *rslt,
    const VFactoryParams *cp, const VFunctionParams *dp )
{
    rc_t rc;
    struct self_t *self = malloc ( sizeof *self );
    if ( self == NULL )
        return RC ( rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted );

    self->dsize = VTypedescSizeof ( & cp->argv[0].desc );
    self->csize = self->dsize * cp->argv[0].count;
    self->count = 256;

    rc = KDataBufferMake(&self->val, self->dsize, cp->argv[0].count*self->count);
    if (rc == 0)
    {
        bitcpy(self->val.base, 0, cp->argv[0].data.u8, 0, self->csize);
        fill(self, 1);

        rslt->self = self;
        rslt->whack = self_free;
        rslt->variant = vftRow;
        rslt->u.rf = exists_func_1;
        return 0;
    }
    free(self);
    return rc;
}

/* exists
 *  returns constant or dynamic value if predicate input exists
 *
 *  "T" [ TYPE ] - type of data to return
 *
 *  "cval" [ CONST, OPTIONAL ] - a data constant. when present,
 *  the function will behave like "echo" ( see below )
 *
 *  "predicate" [ DATA ] - an input whose existence determines
 *  whether the function will operate or not.
 *
 *  "dval" [ DATA, OPTIONAL ] - data value, either passed through
 *  or used to determine a repeat count of "cval" ( see below )
 *
 * USAGE:
 *  when "cval" is omitted, "dval" must be present and will be
 *  passed through depending upon the existence of "predicate"
 *    U8 count = < U8 > exists ( col, count2 );
 *
 *  when "cval" is present, "dval" may be omitted, and "cval" will
 *  be passed through just like echo depending upon "predicate"
 *   U8 count = < U8 > exists < 2 > ( col2 ) | < U8 > echo < 1 > ();
 *
 *  when "cval" and "dval" are both present, the behavior is
 *  like echo, but gated with "predicate"
 *    ascii poly = < ascii > exists < 'a' > ( col, repeat );
 *
function < type T > T exists #1.0 < * T cval > ( any predicate, * T dval ) = vdb:exists;
*/
VTRANSFACT_IMPL ( vdb_exists, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    if ( dp -> argc == 1 )
        return exists_row_0 ( info, rslt, cp );
    return exists_row_1 ( info, rslt, cp, dp );
}
