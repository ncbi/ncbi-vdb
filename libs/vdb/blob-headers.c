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
#include <klib/rc.h>
#include <klib/vlen-encode.h>
#include <klib/data-buffer.h>
#include "blob-headers.h"
#include <vdb/xform.h>
#include <atomic32.h>
#include <sysalloc.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>

struct VBlobHeaderData {
    int64_t *args;
    uint8_t *ops;
    atomic32_t refcount;
    uint32_t op_count;
    uint32_t arg_count;
    uint64_t osize;
    uint8_t flags;
    uint8_t version;
    uint32_t fmt;
    bool read_only;
    bool args_alloc;
    bool ops_alloc;
};

struct VBlobHeaderVT {
    void ( * destroy )( struct VBlobHeader * );
};

struct VBlobHeader {
    const struct VBlobHeaderVT *vt;
    const struct BlobHeaders *parent;
    const struct VBlobHeaderData *owned;
    atomic32_t refcount;
    int32_t op_head;
    int32_t op_tail;
    int32_t op_cap;
    int32_t arg_head;
    int32_t arg_tail;
    int32_t arg_cap;
};

struct BlobHeaders {
    struct VBlobHeaderData *data;
    const BlobHeaders *link;
    atomic32_t refcount;
};

struct BlobHeaderDummy {
    struct VBlobHeader hdr;
    struct BlobHeaders dummy;
    struct VBlobHeaderData data;
};

static void VBlobHeaderDestroy( struct VBlobHeader * );
static void VBlobHeaderDummyDestroy( struct VBlobHeader * );
static const struct VBlobHeaderVT VBlobHeader_vt[] = { { VBlobHeaderDestroy }, { VBlobHeaderDummyDestroy } };

static void VBlobHeaderDataWhack( struct VBlobHeaderData *that )
{
    if (that)
    {
        if (that->args_alloc)
            free(that->args);
        if (that->ops_alloc)
            free(that->ops);
        free(that);
    }
}

static rc_t VBlobHeaderDataDestroy( struct VBlobHeaderData *that ) {
    VBlobHeaderDataWhack(that);
    return 0;
}

static rc_t VBlobHeaderDataRelease( const struct VBlobHeaderData *self ) {
    if (self && atomic32_dec_and_test(&(((struct VBlobHeaderData *)self)->refcount)))
        return VBlobHeaderDataDestroy((struct VBlobHeaderData *)self);
    return 0;
}

#if 0
static rc_t VBlobHeaderDataAddRef ( const struct VBlobHeaderData *self ) {
    if (self)
        atomic32_inc(&(((struct VBlobHeaderData *)self)->refcount));
    return 0;
}
#endif

static int VBlobHeaderDataGetExclusiveOwnership( struct VBlobHeaderData *self ) {
    if (atomic32_read_and_add(&self->refcount, 1) == 1)
        return 1;
    return 0;    
}

static void BlobHeadersWhack( BlobHeaders *that ) {
    free( that );
}

static rc_t BlobHeadersDestroy( BlobHeaders *that ) {
    const BlobHeaders *link = that->link;
    
    VBlobHeaderDataRelease(that->data);
    BlobHeadersWhack(that);
    BlobHeadersRelease(link);
    return 0;
}

rc_t BlobHeadersAddRef ( const BlobHeaders *self ) {
    if (self)
        atomic32_inc(&(((BlobHeaders *)self)->refcount));
    return 0;
}

rc_t BlobHeadersRelease ( const BlobHeaders *self ) {
    if (self && atomic32_dec_and_test(&(((BlobHeaders *)self)->refcount)))
        return BlobHeadersDestroy((BlobHeaders *)self);
    return 0;
}

#if 0
static int BlobHeadersGetExclusiveOwnership( BlobHeaders *self ) {
    if (atomic32_read_and_add(&self->refcount, 1) == 1)
        return 1;
    return 0;    
}
#endif

static rc_t BlobHeadersCreateInternal ( BlobHeaders **lhs, unsigned ops, unsigned args ) {
    BlobHeaders *y = calloc(1, sizeof *y);

    *lhs = NULL;
    if (y) {
        atomic32_set(&y->refcount, 1);
        y->data = calloc(1, sizeof(*y->data) + args * sizeof(y->data->args[0]) + ops);
        if (y->data) {
            atomic32_set(&y->data->refcount, 1);
            if (args) {
                y->data->args = (void *)(&y->data[1]);
                if (ops) {
                    y->data->ops = (void *)(&y->data->args[args]);
                    y->data->op_count = ops;
                }
                y->data->arg_count = args;
            }
            else if (ops) {
                y->data->ops = (void *)(&y->data[1]);
                y->data->op_count = ops;
            }
            *lhs = y;
            
            return 0;
        }
        free(y);
    }
    return RC(rcVDB, rcHeader, rcConstructing, rcMemory, rcExhausted);
}

rc_t BlobHeadersCreate( BlobHeaders **lhs )
{
    return BlobHeadersCreateInternal( lhs, 0, 0 );
}

uint32_t BlobHeadersGetMaxBufferSize ( const BlobHeaders *self )
{
    uint32_t y = 1;
    const BlobHeaders *x = self;
    const uint32_t per_header = 25;
    
    while (x) {
        y += per_header + x->data ? (x->data->op_count + x->data->arg_count * 10) : 0;
        x = x->link;
    }
    return y;
}

static rc_t deserialize_v0(BlobHeaders **dst, const uint8_t *src, uint64_t ssize);

rc_t BlobHeadersCreateFromData ( BlobHeaders **lhs, const uint8_t *src, uint64_t ssize )
{
    rc_t rc;
    BlobHeaders *y;
    
    if (src == NULL)
        return RC(rcVDB, rcHeader, rcConstructing, rcParam, rcNull);
    if (ssize < 1)
        return RC(rcVDB, rcHeader, rcConstructing, rcData, rcInsufficient);
    
    switch (*src) {
    case 0:
        rc = deserialize_v0(&y, src + 1, ssize - 1);
        if (rc == 0)
            *lhs = y;
        break;
    default:
        rc = RC(rcVDB, rcHeader, rcConstructing, rcData, rcBadVersion);
        break;
    }
    return rc;
}

static rc_t serialize1( const BlobHeaders *self, KDataBuffer *buffer, uint64_t offset, uint64_t *size ) {
    uint8_t flags = 0;
    uint8_t version = 0;
    uint32_t fmt = 0;
    uint64_t osize = 0;
    uint32_t op_count = 0;
    uint32_t arg_count = 0;
    rc_t rc;
    uint64_t sz;
    uint8_t *dst;
    uint64_t dsize;

    if (self->data) {
        version = self->data->version;
        flags = self->data->flags;
        fmt = self->data->fmt;
        osize = self->data->osize;
        op_count = self->data->op_count;
        arg_count = self->data->arg_count;
    }
    sz = 2 + 4 * 5 + op_count + arg_count * 10;
    rc = KDataBufferResize(buffer, sz + offset);
    if (rc)
        return rc;

    dst = buffer->base;
    dst += offset;
    dsize = buffer->elem_count - offset;
    
    *dst++ = flags; --dsize;
    *dst++ = version; --dsize;
    
    rc = vlen_encode1(dst, dsize, &sz, fmt);
    if (rc)
        return rc;
    dst += sz; dsize -= sz;
    
    rc = vlen_encode1(dst, dsize, &sz, osize);
    if (rc)
        return rc;
    dst += sz; dsize -= sz;
    
    rc = vlen_encode1(dst, dsize, &sz, op_count);
    if (rc)
        return rc;
    dst += sz; dsize -= sz;
    
    rc = vlen_encode1(dst, dsize, &sz, arg_count);
    if (rc)
        return rc;
    dst += sz; dsize -= sz;
    
    if (op_count > 0) {
        memcpy(dst, self->data->ops, op_count);
        dst += op_count;
        dsize -= op_count;
    }
    if (arg_count > 0) {
        rc = vlen_encode(dst, dsize, &sz, self->data->args, arg_count);
        if (rc)
            return rc;
        dst += sz; /* dsize -= sz */;
    }
    *size = (uint64_t)( dst - (uint8_t *)buffer->base );
    return 0;
}

static rc_t deserialize_v0( BlobHeaders **dst, const uint8_t *src, uint64_t ssize ) {
    uint8_t flags;
    uint8_t version;
    uint32_t fmt;
    uint64_t osize;
    uint32_t op_count;
    uint32_t arg_count;
    int64_t x;
    uint64_t sz;
    rc_t rc;
    BlobHeaders *y;

    *dst = NULL;
    
    if (ssize < 2)
        return RC(rcVDB, rcHeader, rcConstructing, rcData, rcInsufficient);

    flags = *src++; --ssize;
    version = *src++; --ssize;
    
    rc = vlen_decode1(&x, src, ssize, &sz);
    if (rc)
        return ResetRCContext(rc, rcVDB, rcHeader, rcConstructing);
    src += sz; 
	ssize -= sz;
    fmt = (uint32_t)x;
    
    rc = vlen_decode1(&x, src, ssize, &sz);
    if (rc)
        return ResetRCContext(rc, rcVDB, rcHeader, rcConstructing);
    src += sz; 
	ssize -= sz;
    osize = (uint32_t)x;
    
    rc = vlen_decode1(&x, src, ssize, &sz);
    if (rc)
        return ResetRCContext(rc, rcVDB, rcHeader, rcConstructing);
    src += sz; 
	ssize -= sz;
    op_count = (uint32_t)x;
    
    rc = vlen_decode1(&x, src, ssize, &sz);
    if (rc)
        return ResetRCContext(rc, rcVDB, rcHeader, rcConstructing);
    src += sz; 
	ssize -= sz;
    arg_count = (uint32_t)x;

    rc = BlobHeadersCreateInternal(&y, op_count, arg_count);
    if (rc)
        return rc;
    
    y->data->flags = flags;
    y->data->version = version;
    y->data->fmt = fmt;
    y->data->osize = osize;
    y->data->read_only = true;

    if (op_count > 0) {
        if (ssize < op_count) {
            BlobHeadersRelease(y);
            return RC(rcVDB, rcHeader, rcConstructing, rcData, rcInsufficient);
        }
        memcpy(y->data->ops, src, op_count);
        src += op_count; ssize -= op_count;
    }        
    if (arg_count > 0) {
        rc = vlen_decode(y->data->args, arg_count, src, ssize, &sz);
        if (rc) {
            BlobHeadersRelease(y);
            return ResetRCContext(rc, rcVDB, rcHeader, rcConstructing);
        }
        src += sz; ssize -= sz;
    }
    if (ssize) {
        BlobHeaders *link;
        
        rc = deserialize_v0(&link, src, ssize);
        if (rc) {
            BlobHeadersRelease(y);
            return rc;
        }
        y->link = link;
    }
    *dst = y;
    return 0;
}

static rc_t serialize( const BlobHeaders *self, KDataBuffer *buffer, uint64_t offset, uint64_t *sz ) {
    rc_t rc;

    rc = serialize1(self, buffer, offset, sz);
    if (rc == 0 && self->link)
        rc = serialize(self->link, buffer, *sz, sz);
    return rc;
}

rc_t BlobHeadersSerialize ( BlobHeaders *self, KDataBuffer *buffer,
        uint64_t offset, uint64_t *psz )
{
    rc_t rc;
    uint64_t sz = 0;
    
    assert(buffer->elem_bits == 8);
    assert(buffer->bit_offset == 0);
    
    rc = KDataBufferResize(buffer, 1);
    if (rc)
        return rc;
    ((uint8_t *)(buffer->base))[offset] = 0;
    rc = serialize(self, buffer, offset + 1, &sz);
    *psz = sz - offset;
    return rc;
}

rc_t BlobHeadersCreateChild ( BlobHeaders *self, BlobHeaders **new_hdr ) 
{
    BlobHeaders *y;
    rc_t rc;

    *new_hdr = NULL;
    rc = BlobHeadersCreate(&y);
    if (rc == 0) {
        y->link = self;
        BlobHeadersAddRef(y->link);
        *new_hdr = y;
        return 0;
    }
    return rc;
}

VBlobHeader *BlobHeadersCreateDummyHeader ( uint8_t version, uint32_t fmt, uint8_t flags, uint64_t size )
{
    struct BlobHeaderDummy *y;
    
    y = calloc(1, sizeof(*y));
    if (y) {
        y->data.fmt = fmt;
        y->data.version = version;
        y->data.osize = size;
        y->data.flags = flags;
        atomic32_set(&y->data.refcount, 1);

        y->dummy.data = &y->data;
        atomic32_set(&y->dummy.refcount, 1);
        
        y->hdr.vt = &VBlobHeader_vt[1];
        y->hdr.parent = &y->dummy;
        atomic32_set(&y->hdr.refcount, 1);
        
        return &y->hdr;
    }
    return 0;
}

const BlobHeaders *BlobHeadersGetNextFrame( const BlobHeaders *cself ) {
    return cself ? cself->link : 0;
}

const BlobHeaders *BlobHeadersPop( const BlobHeaders *cself ) {
    const BlobHeaders *y = BlobHeadersGetNextFrame(cself);
    BlobHeadersAddRef(y);
    BlobHeadersRelease(cself);
    return y;
}

VBlobHeader *BlobHeadersGetHeader ( const BlobHeaders *self )
{
    VBlobHeader *y = calloc(1, sizeof(*y));
    
    if (y) {
        y->vt = &VBlobHeader_vt[0];
        atomic32_set(&y->refcount, 1);
        y->parent = self;
        BlobHeadersAddRef(y->parent);
        if (self->data) {
            y->op_tail = self->data->op_count;
            y->arg_tail = self->data->arg_count;
        }
    }
    return y;
}

VBlobHeader *BlobHeadersGetHdrWrite ( BlobHeaders *self ) {
    VBlobHeader *y = BlobHeadersGetHeader(self);
    
    if (y) {
        if (self->data && !self->data->read_only) {
            if (VBlobHeaderDataGetExclusiveOwnership(self->data)) {
                y->owned = self->data;
                return y;
            }
        }
        VBlobHeaderRelease(y);
    }
    return NULL;
}

static void VBlobHeaderDummyDestroy( VBlobHeader *that ) {
    free(that);
}

static void VBlobHeaderDestroy( VBlobHeader *that ) {
    VBlobHeaderDataRelease(that->owned);
    BlobHeadersRelease(that->parent);
    free(that);
}

LIB_EXPORT rc_t CC VBlobHeaderRelease ( const VBlobHeader *cself ) {
    VBlobHeader *self = (VBlobHeader *)cself;
    
    if (self && atomic32_dec_and_test(&self->refcount))
        self->vt->destroy(self);
    return 0;
}

rc_t VBlobHeaderAddRef ( const VBlobHeader *self ) {
    if (self)
        atomic32_inc(&(((VBlobHeader *)self)->refcount));
    return 0;
}

LIB_EXPORT rc_t CC VBlobHeaderCreateFork ( const VBlobHeader *self, VBlobHeader **rslt ) {
    if (self == NULL)
        return RC(rcVDB, rcHeader, rcCopying, rcSelf, rcNull);
    
    if (self->parent == NULL)
        return RC(rcVDB, rcHeader, rcCopying, rcSelf, rcInvalid);
    
    if (rslt == NULL)
        return RC(rcVDB, rcHeader, rcCopying, rcParam, rcNull);
    
    *rslt = BlobHeadersCreateDummyHeader(self->parent->data->version, self->parent->data->fmt, 0, self->parent->data->osize);
    if (*rslt)
        return 0;
    return RC(rcVDB, rcHeader, rcCopying, rcMemory, rcInsufficient);
}

rc_t VBlobHeaderReplace( VBlobHeader *targ, const VBlobHeader *src ) {
    BlobHeaders *parent;
    struct VBlobHeaderData *data;
    atomic32_t temp;
    
    if (src == NULL || targ == NULL)
        return RC(rcVDB, rcHeader, rcCopying, rcParam, rcNull);

    if (src->vt != &VBlobHeader_vt[1] || targ->vt != &VBlobHeader_vt[0])
        return RC(rcVDB, rcHeader, rcCopying, rcParam, rcInvalid);
    
    if (atomic32_read(&targ->refcount) != 1)
        return RC(rcVDB, rcHeader, rcCopying, rcParam, rcInvalid);
    
    data = calloc(1, sizeof(*data));
    if (data == NULL)
        return RC(rcVDB, rcHeader, rcCopying, rcMemory, rcExhausted);

    *data = *src->parent->data;
    atomic32_set(&data->refcount, 1);
    
    parent = (BlobHeaders *)targ->parent;
    VBlobHeaderDataRelease(parent->data);
    parent->data = data;
    
    atomic32_set(&temp, atomic32_read(&src->refcount));
    *targ = *src;
    atomic32_set(&targ->refcount, atomic32_read(&temp));
    targ->parent = parent;
    targ->vt = &VBlobHeader_vt[0];
    
    return 0;
}

rc_t VBlobHeaderOpPushHead ( VBlobHeader *self, uint8_t op )
{
    return RC(rcVDB, rcHeader, rcWriting, rcInterface, rcUnsupported);
}

rc_t VBlobHeaderArgPushHead ( VBlobHeader *self, int64_t arg )
{
    return RC(rcVDB, rcHeader, rcWriting, rcInterface, rcUnsupported);
}

#define MIN_ALLOC 16

LIB_EXPORT rc_t CC VBlobHeaderOpPushTail ( VBlobHeader *self, uint8_t op )
{
    struct VBlobHeaderData *data;
    
    assert(self);
    assert(self->parent);
    data = self->parent->data;
    assert(data);
    assert(!data->read_only);
    
    if (data->ops == NULL) {
        data->ops = malloc(MIN_ALLOC);
        if (data->ops == NULL)
            return RC(rcVDB, rcHeader, rcWriting, rcMemory, rcExhausted);
        self->op_cap = MIN_ALLOC;
    }
    
    assert(self->op_tail <= self->op_cap);
    
    if (self->op_tail == self->op_cap) {
        uint32_t size = self->op_cap;
        void *temp = realloc(data->ops, size += size);
        
        if (temp == NULL)
            return RC(rcVDB, rcHeader, rcWriting, rcMemory, rcExhausted);
        data->ops = temp;
        self->op_cap = size;
    }
    data->ops[self->op_tail++] = op;
    ++data->op_count;
    data->ops_alloc = true;
    return 0;
}

LIB_EXPORT rc_t CC VBlobHeaderArgPushTail ( VBlobHeader *self, int64_t arg )
{
    struct VBlobHeaderData *data;
    
    assert(self);
    assert(self->parent);
    data = self->parent->data;
    assert(data);
    assert(!data->read_only);
    
    if (data->args == NULL) {
        data->args = malloc(MIN_ALLOC * sizeof(data->args[0]));
        if (data->args == NULL)
            return RC(rcVDB, rcHeader, rcWriting, rcMemory, rcExhausted);
        self->arg_cap = MIN_ALLOC;
    }
    
    assert(self->arg_tail <= self->arg_cap);
    
    if (self->arg_tail == self->arg_cap) {
        uint32_t size = self->arg_cap;
        void *temp = realloc(data->args, (size += size) * sizeof(data->args[0]));
        
        if (temp == NULL)
            return RC(rcVDB, rcHeader, rcWriting, rcMemory, rcExhausted);
        data->args = temp;
        self->arg_cap = size;
    }
    data->args[self->arg_tail++] = arg;
    ++data->arg_count;
    data->args_alloc = true;
    return 0;
}

LIB_EXPORT rc_t CC VBlobHeaderOpPopHead ( const VBlobHeader *self, uint8_t *op )
{
    if (self->op_head >= self->op_tail || self->op_head >= (int32_t)self->parent->data->op_count)
        return RC(rcVDB, rcHeader, rcReading, rcData, rcExhausted);
    *op = self->parent->data->ops[self->op_head];
    ++((VBlobHeader *)self)->op_head;
    return 0;
}

LIB_EXPORT rc_t CC VBlobHeaderOpPopTail ( const VBlobHeader *self, uint8_t *op )
{
    if (self->op_tail < self->op_head || self->op_tail < 0)
        return RC(rcVDB, rcHeader, rcReading, rcData, rcExhausted);
    *op = self->parent->data->ops[self->op_tail];
    --((VBlobHeader *)self)->op_tail;
    return 0;
}

LIB_EXPORT rc_t CC VBlobHeaderArgPopHead ( const VBlobHeader *self, int64_t *arg )
{
    if (self->arg_head >= self->arg_tail || self->arg_head >= (int32_t)self->parent->data->arg_count || self->parent->data->args == NULL)
        return RC(rcVDB, rcHeader, rcReading, rcData, rcExhausted);
    *arg = self->parent->data->args[self->arg_head];
    ++((VBlobHeader *)self)->arg_head;
    return 0;
}

LIB_EXPORT rc_t CC VBlobHeaderArgPopTail ( const VBlobHeader *self, int64_t *arg )
{
    if (self->arg_tail < self->arg_head || self->arg_tail < 0 || self->parent->data->args == NULL)
        return RC(rcVDB, rcHeader, rcReading, rcData, rcExhausted);
    *(uint64_t *)arg = self->parent->data->args[self->arg_tail];
    --((VBlobHeader *)self)->arg_tail;
    return 0;
}

LIB_EXPORT uint8_t CC VBlobHeaderVersion ( const VBlobHeader *self )
{
    return self->parent->data->version;
}

LIB_EXPORT void CC VBlobHeaderSetVersion ( VBlobHeader *self, uint8_t version )
{
    self->parent->data->version = version;
}

uint8_t VBlobHeaderFlags ( const VBlobHeader *self )
{
    return self->parent->data->flags & 0x7F;
}

void VBlobHeaderSetFlags ( VBlobHeader *self, uint8_t flags )
{
    self->parent->data->flags = (self->parent->data->flags & 0x80) | (flags & 0x7F);
}

uint32_t VBlobHeaderFormat ( const VBlobHeader *self )
{
    return self->parent->data->fmt;
}

void VBlobHeaderSetFormat ( VBlobHeader *self, uint32_t fmt )
{
    self->parent->data->fmt = fmt;
}

uint64_t VBlobHeaderSourceSize ( const VBlobHeader *self )
{
    return self->parent->data->osize;
}

void VBlobHeaderSetSourceSize ( VBlobHeader *self, uint64_t ssize )
{
    self->parent->data->osize = ssize;
}
