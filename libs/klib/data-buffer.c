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

#include <klib/extern.h>
#include <klib/data-buffer.h>
#include <klib/rc.h>
#include <atomic32.h>
#include <bitstr.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#if _DEBUGGING
#define DEBUG_MALLOC_FREE 1
#include <stdio.h>
#endif

/*--------------------------------------------------------------------------
 * KDataBufferImpl
 */
typedef struct buffer_impl_t buffer_impl_t;
struct buffer_impl_t {
    size_t allocated;
    atomic32_t refcount;
#if DEBUG_MALLOC_FREE
    uint32_t foo;
#endif
};

static size_t roundup(size_t value, unsigned bits)
{
    size_t const mask = (((size_t)1u) << bits) - 1;
    return (value + mask) & (~mask);
}

static
rc_t allocate(buffer_impl_t **target, size_t capacity) {
    buffer_impl_t *y = malloc(capacity + sizeof(*y));

    if (y == NULL)
        return RC(rcRuntime, rcBuffer, rcAllocating, rcMemory, rcExhausted);

    y->allocated = capacity;
    atomic32_set(&y->refcount, 1);
    
#if DEBUG_MALLOC_FREE
    y->foo = 0;
#endif

    *target = y;
    return 0;
}

static buffer_impl_t *add_ref(buffer_impl_t *self) {
    atomic32_inc(&self->refcount);
    return self;
}

static buffer_impl_t *test_add_ref(buffer_impl_t *self) {
    if (self) add_ref(self);
    return self;
}

static void release(buffer_impl_t *self) {
    int32_t refcount = atomic32_read_and_add(&self->refcount, -1);
    
    if (refcount == 1) {
#if DEBUG_MALLOC_FREE
        if (self->foo != 0) {
            fprintf(stderr, "DIE DIE\n");
        }
        self->foo = 55;
#endif
        free(self);
    }
#if DEBUG_MALLOC_FREE
    else if (refcount < 1) {
        fprintf(stderr, "ALREADY DEAD\n");
    }
#endif
}

/* always returns object (new or original) with refcount == 1 */
static rc_t reallocate(buffer_impl_t **target, size_t capacity) {
    buffer_impl_t *temp;
    buffer_impl_t *self = *target;

    if (capacity <= self->allocated)
        return 0;

    /* check reference count for copies */
    if (atomic32_read(&self->refcount) <= 1)
    {
        temp = realloc(self, capacity + sizeof(*temp));
        if (temp == NULL)
            return RC(rcRuntime, rcBuffer, rcResizing, rcMemory, rcExhausted);
    }
    else
    {
        temp = malloc(capacity + sizeof(*temp));
        if (temp == NULL)
            return RC(rcRuntime, rcBuffer, rcResizing, rcMemory, rcExhausted);
        memcpy(temp, self, self->allocated + sizeof(*temp));
        release(self);
    }
    self = temp;
    self->allocated = capacity;
    atomic32_set(&self->refcount, 1);
    *target = self;

    return 0;
}

static rc_t shrink(buffer_impl_t **target, size_t capacity)
{
    buffer_impl_t *self = *target;
    
    if (capacity < self->allocated && atomic32_read(&self->refcount) == 1) {
        buffer_impl_t *temp = realloc(self, capacity + sizeof(*temp));
        
        if (temp == NULL)
            return RC(rcRuntime, rcBuffer, rcResizing, rcMemory, rcExhausted);

        temp->allocated = capacity;
        *target = temp;
    }
    return 0;
}

/*
 either returns original with refcount == 2
 or returns new copy with refcount == 1
 */
static buffer_impl_t* make_copy(buffer_impl_t *self) {
    if (atomic32_read_and_add_eq(&self->refcount, 1, 1)==1)
        return self;
    else {
        buffer_impl_t *copy = malloc(self->allocated + sizeof(*self));
        if (copy) {
            memcpy(copy, self, self->allocated + sizeof(*copy));
            atomic32_set(&copy->refcount, 1);
        }
        return copy;
    }
}

static void const *get_data(buffer_impl_t const *self)
{
    return &self[1];
}

static void const *get_data_endp(buffer_impl_t const *self)
{
    return (uint8_t const *)get_data(self) + self->allocated;
}

static
rc_t buffer_impl_check_integrity (buffer_impl_t const *self, uint8_t const *base, size_t offset, size_t size)
{
    void const *const endp = &base[size+offset];

    if (self == NULL)
        return 0;

    /* refcount is of major importance */
    if (atomic32_read (& self->refcount) <= 0)
        return RC (rcRuntime, rcBuffer, rcValidating, rcBuffer, rcDestroyed);

#if DEBUG_MALLOC_FREE
    if (self->foo != 0)
        return RC (rcRuntime, rcBuffer, rcValidating, rcBuffer, rcDestroyed);
#endif

    if ((void const *)base < get_data(self) || get_data_endp(self) < endp)
        return RC (rcRuntime, rcBuffer, rcValidating, rcData, rcCorrupt);

    return 0;
}

/* Make
 *  create a new empty buffer
 *
 *  "buffer" [ OUT ] - pointer to structure to initialize
 *
 *  "capacity" [ IN ] - the number of bytes to be allocated
 */
LIB_EXPORT rc_t CC KDataBufferMake(KDataBuffer *target, uint64_t elem_bits, uint64_t elem_count) {
    rc_t rc;
    size_t bytes;
    buffer_impl_t **impp;
    
    if (target == NULL)
    	return RC(rcRuntime, rcBuffer, rcConstructing, rcParam, rcNull);

    impp = (buffer_impl_t **)&target->ignore;
    
    bytes = roundup((elem_bits * elem_count + 7) / 8, 12);
    if (8 * (uint64_t)bytes < elem_bits * elem_count)
    	return RC(rcRuntime, rcBuffer, rcConstructing, rcParam, rcTooBig);
    
    memset (target, 0, sizeof(*target));

    rc = allocate(impp, bytes);
    if (rc == 0) {
        target->base = (void *)get_data(*impp);
        target->elem_bits = elem_bits;
        target->elem_count = elem_count;
    }
    return rc;
}

LIB_EXPORT rc_t CC KDataBufferResize(KDataBuffer *self, uint64_t new_count) {
    rc_t rc;
    buffer_impl_t *imp;
    buffer_impl_t *new_imp;
    size_t new_size;
    uint64_t bits;
    const uint8_t *new_end;
    const uint8_t *cur_end;
    
    if (self == NULL)
    	return RC(rcRuntime, rcBuffer, rcResizing, rcParam, rcNull);

    if (new_count == 0) {
        self->elem_count = 0;
        return 0; /*** no change for empty data ***/
    }

    bits = self->elem_bits * new_count;
    if (((bits + 7) >> 35) != 0)
    	return RC(rcRuntime, rcBuffer, rcConstructing, rcParam, rcTooBig);
    
    imp = (buffer_impl_t *)self->ignore;
    if (imp == NULL) {
        /* new buffer */
        rc = allocate(&imp, roundup((bits + 7) / 8, 12));
        if (rc == 0) {
            self->ignore = imp;
            self->base = (void *)get_data(imp);
            self->elem_count = new_count;
        }
        return rc;
    }

    cur_end = &((const uint8_t *)imp)[imp->allocated + sizeof *imp];
    new_end = &((const uint8_t *)self->base)[(bits + self->bit_offset + 7) >> 3];
    if (cur_end >= new_end) {
        /* requested end-of-buffer is within current allocation; realloc not required */
        self->elem_count = new_count;
        return 0;
    }

    if (!KDataBufferWritable(self))
        return RC(rcRuntime, rcBuffer, rcResizing, rcSelf, rcReadonly);
    
    new_size = roundup((bits + 7) / 8, 12);
    if (self->base == get_data(imp) && self->bit_offset == 0) {
        rc = reallocate(&imp, new_size);
        if (rc == 0) {
            self->ignore = imp;
            self->base = (void *)get_data(imp);
            self->elem_count = new_count;
        }
        return rc;
    }

    /* is sub-buffer but is sole reference */
    rc = allocate(&new_imp, roundup(new_size, 12));
    if (rc == 0) {
        memcpy((void *)get_data(new_imp), self->base, new_size);
        release(imp);
        self->base = (void *)get_data(new_imp);
        self->ignore = new_imp;
        self->elem_count = new_count;
    }
    return rc;
}

LIB_EXPORT rc_t CC KDataBufferSub (const KDataBuffer *self,
    KDataBuffer *target, uint64_t start, uint64_t count)
{
    if (self == NULL || target == NULL)
    	return RC(rcRuntime, rcBuffer, rcConstructing, rcParam, rcNull);
    
    if (self->ignore == NULL) {
        if (start > 0 || count < UINT64_MAX)
            return RC(rcRuntime, rcBuffer, rcConstructing, rcParam, rcNull);
        *target = *self;
        return 0;
    }
    if ((const KDataBuffer *)target != self) {
        *target = *self;
        target->ignore = add_ref((buffer_impl_t *)self->ignore);
    }
    {
        buffer_impl_t *imp = (buffer_impl_t *)self->ignore;
        bitsz_t offset = self->bit_offset + (((bitsz_t)((uint8_t const *)self->base - (uint8_t const *)get_data(imp))) << 3);
        
        if (start > self->elem_count){
            start = self->elem_count;
            count = 0;
        }
        else if (count > self->elem_count) {
            count = self->elem_count;
        }
        if (start + count > self->elem_count) {
            count = self->elem_count - start;
        }
        target->elem_count = count;
        offset += (bitsz_t)start * self->elem_bits;
        target->base = &((uint8_t *)get_data(imp))[offset >> 3];
        target->bit_offset = (uint8_t)offset & 0x07;
    }
    return 0;
}

LIB_EXPORT rc_t CC KDataBufferCast(const KDataBuffer *self, KDataBuffer *target, uint64_t new_elem_bits, bool can_shrink) {
    bitsz_t bits;
    size_t new_elem_count;
    bitsz_t new_bits;

    if (self == NULL)
        return RC (rcRuntime, rcBuffer, rcCasting, rcSelf, rcNull);
    if (target == NULL)
    	return RC (rcRuntime, rcBuffer, rcCasting, rcParam, rcNull);
    if (new_elem_bits == 0)
    	return RC (rcRuntime, rcBuffer, rcCasting, rcParam, rcInvalid);
    
    bits = KDataBufferBits(self);
    new_elem_count = bits / new_elem_bits;
    new_bits = new_elem_bits * new_elem_count;
    
    if (new_bits == bits || (can_shrink && new_bits < bits)) {
        if ((const KDataBuffer *)target != self) {
            *target = *self;
            target->ignore = test_add_ref((buffer_impl_t *)self->ignore);
        }
        target->elem_count = new_elem_count;
        target->elem_bits = new_elem_bits;
        return 0;
    }
    return RC(rcRuntime, rcBuffer, rcCasting, rcParam, rcInvalid);
}

LIB_EXPORT rc_t CC KDataBufferMakeWritable (const KDataBuffer *cself, KDataBuffer *target)
{
    if (cself == NULL)
    	return RC(rcRuntime, rcBuffer, rcConstructing, rcParam, rcNull);
    
    if (target == NULL)
    	return RC(rcRuntime, rcBuffer, rcConstructing, rcParam, rcNull);

    if ((KDataBuffer const *)target != cself)
        memset(target, 0, sizeof(*target));
    
    if (cself->ignore == NULL)
        return KDataBufferMake(target, cself->elem_bits, cself->elem_count);
    else {
        buffer_impl_t *self = (buffer_impl_t *)cself->ignore;
        buffer_impl_t *copy;
        
        if (cself->base == get_data(self) && cself->bit_offset == 0) {
            /* not a sub-buffer */
            copy = make_copy(self);
            if (copy) {
                if ((KDataBuffer const *)target == cself)
                    release(self);
                else 
                    *target = *cself;
                target->ignore = copy;
                target->base = (uint8_t *)get_data(copy);
                
                return 0;
            }
            return RC(rcRuntime, rcBuffer, rcAllocating, rcMemory, rcExhausted);
        }
        else if (atomic32_read(&self->refcount) == 1) {
            /* sub-buffer but is only reference so let it be */
            if ((KDataBuffer const *)target != cself) {
                *target = *cself;
                atomic32_set(&self->refcount, 2);
            }
            return 0;
        }
        else {
            /* sub-buffer so make new and copy */
            rc_t rc;
            
            rc = allocate(&copy, roundup(KDataBufferBytes(cself), 12));
            if (rc == 0) {
                if (cself->bit_offset == 0)
                    memcpy((void *)get_data(copy), cself->base, KDataBufferBytes(cself));
                else
                    bitcpy((void *)get_data(copy), 0, cself->base, cself->bit_offset, KDataBufferBits(cself));

                if ((const KDataBuffer *)target == cself)
                    release(self);
                else 
                    *target = *cself;
                target->ignore = copy;
                target->base = (uint8_t *)get_data(copy);
                target->bit_offset = 0;
                return 0;
            }
            return rc;
        }
    }
}

LIB_EXPORT rc_t CC KDataBufferWhack (KDataBuffer *self)
{
    if (self)
    {
        if (self->ignore)
            release((buffer_impl_t *)self->ignore);

        memset(self, 0, sizeof(*self));
    }
    return 0;
}

LIB_EXPORT bool CC KDataBufferWritable(const KDataBuffer *cself)
{
    return (cself != NULL && cself->ignore != NULL &&
            atomic32_read(&((buffer_impl_t *)cself->ignore)->refcount) == 1) ? true : false;
}

LIB_EXPORT rc_t CC KDataBufferShrink(KDataBuffer *self)
{
    if (self && self->ignore) {
        return shrink((buffer_impl_t **)&self->ignore,
            (self->elem_bits * self->elem_count + self->bit_offset + 7) / 8);
    }
    return 0;
}

LIB_EXPORT size_t CC KDataBufferMemorySize(KDataBuffer const *self)
{
    if (self && self->ignore) {
        return ((buffer_impl_t const *)self->ignore)->allocated + sizeof(buffer_impl_t);
    }
    return 0;
}

/* CheckIntegrity
 *  performs some level of integrity checking on buffer structure
 */
LIB_EXPORT rc_t CC KDataBufferCheckIntegrity (const KDataBuffer *self)
{
    /* whether NULL is allowed is determined by caller */
    if (self == NULL)
        return RC (rcRuntime, rcBuffer, rcValidating, rcSelf, rcNull);

    /* elem-bits must not be 0, unless there is no allocation */
    if (self->elem_bits == 0)
    {
        if (self->elem_count == 0 && self->bit_offset == 0 && self->ignore == NULL) {
            /* no allocation */
            return 0;
        }
	    return RC (rcRuntime, rcBuffer, rcValidating, rcSelf, rcCorrupt);
    }

    /* if there is no allocation, check self */
    if (self->ignore == NULL)
    {
        if (self->elem_count != 0 || self->bit_offset != 0)
	    return RC (rcRuntime, rcBuffer, rcValidating, rcSelf, rcCorrupt);
    }

    /* test self against allocated buffer */
    return buffer_impl_check_integrity (self->ignore, (const void*) self->base,
        self->bit_offset / self->elem_bits, KDataBufferBytes (self));
}

/* 0x101e9b000 */
