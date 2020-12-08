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

#if _ARCH_BITS == 32
#define BASE_PTR_ALIGNMENT 8
#else
#define BASE_PTR_ALIGNMENT 16
#endif

#define DEBUG_ALIGNMENT 0

#define MAX_BUFFER_BITS 35
#define MIN_ELEM_BITS 1
#define MAX_ELEM_COUNT_BITS ( MAX_BUFFER_BITS / MIN_ELEM_BITS )
#define MAX_ELEM_BITS_BITS ( 64 - MAX_ELEM_COUNT_BITS )

#if _DEBUGGING
#define DEBUG_MALLOC_FREE 1
#include <stdio.h>
#endif

#if DEBUG_ALIGNMENT
#if ! _DEBUGGING
#include <stdio.h>
#endif

static
void cc_impl ( const KDataBuffer * self, const char * func, uint32_t lineno )
{
    /* if not byte-aligned, pointer is always a byte pointer */
    if ( ( self -> elem_bits & 7 ) == 0 )
    {
        /* convert from bits to bytes */
        uint64_t elem_bytes = self -> elem_bits >> 3;

        /* if the size is an integral power of 2 */
        if ( ( ( elem_bytes - 1 ) & elem_bytes ) == 0 )
        {
            /* never should be called with < 2 bytes ( see "cc" macro ) */
            assert ( elem_bytes >= 2 );

#if _ARCH_BITS == 32
            /* test pointer alignment against 1 ( 16-bit ), 3 ( 32-bit ) */
            if ( ( ( size_t ) self -> base & ( elem_bytes - 1 ) & 3 ) != 0 )
            {
                /* this buffer has bad pointer alignment */
                fprintf ( stderr, "%s:%u: %s - WARNING: bad pointer alignment: 0x%08lx -> %lu bytes\n",
                          __FILE__, lineno, func, ( size_t ) self -> base, ( size_t ) elem_bytes );
            }
#else
            /* test pointer alignment against 1 ( 16-bit ), 3 ( 32-bit ), 7 ( 64-bit ), 15 ( 128-bit ) */
            if ( ( ( size_t ) self -> base & ( elem_bytes - 1 ) & 15 ) != 0 )
            {
                /* this buffer has bad pointer alignment */
                fprintf ( stderr, "%s:%u: %s - WARNING: bad pointer alignment: 0x%016lx -> %lu bytes\n",
                          __FILE__, lineno, func, ( size_t ) self -> base, ( size_t ) elem_bytes );
            }
#endif
        }
    }
}

#define cc( self )                                          \
    if ( ( self ) != NULL && ( self ) -> elem_bits > 16 )   \
        cc_impl ( self, __func__, __LINE__ )
#else
#define cc( self ) ( ( void ) 0 )
#endif

/*--------------------------------------------------------------------------
 * KDataBufferImpl
 */
typedef struct buffer_impl_t buffer_impl_t;
struct buffer_impl_t {
    size_t allocated;
    atomic32_t refcount;
    uint32_t foo;
#if _ARCH_BITS == 32
    uint32_t foo2;
#endif
};

static size_t roundup(size_t value, unsigned bits)
{
    size_t const mask = (((size_t)1u) << bits) - 1;
    return (value + mask) & (~mask);
}

static
void buffer_impl_wipe ( buffer_impl_t * self )
{
    if ( self != NULL )
        memset ( self + 1, 0, self -> allocated );
}

static
rc_t allocate ( buffer_impl_t ** target, size_t capacity, bool clear )
{
    size_t bytes;
    buffer_impl_t * y;
    
    bytes = capacity + sizeof * y;
    y = clear ? calloc ( bytes, 1 ) : malloc ( bytes );
    if ( y == NULL )
        return RC ( rcRuntime, rcBuffer, rcAllocating, rcMemory, rcExhausted );

    y -> allocated = capacity;
    atomic32_set ( & y -> refcount, 1 );
    
#if DEBUG_MALLOC_FREE
    y -> foo = 0;
#endif

    * target = y;
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

static void release ( buffer_impl_t * self, bool wipe )
{
    int32_t refcount = atomic32_read_and_add ( & self -> refcount, -1 );
    
    if ( refcount == 1 )
    {
#if DEBUG_MALLOC_FREE
        if (self->foo != 0) {
            fprintf(stderr, "DIE DIE\n");
        }
        self->foo = 55;
#endif
        if ( wipe )
            memset ( self, 0, sizeof * self + self -> allocated );
        
        free ( self );
    }
#if DEBUG_MALLOC_FREE
    else if (refcount < 1) {
        fprintf(stderr, "ALREADY DEAD\n");
    }
#endif
}

/* always returns object (new or original) with refcount == 1 */
static rc_t reallocate ( buffer_impl_t ** target, size_t capacity, bool clear, bool wipe )
{
    buffer_impl_t * temp, * self = * target;

    /* if down-sizing, the original behavior was just to ignore */
    if ( capacity <= self -> allocated )
    {
        /* but there may be some wiping to do */
        if ( wipe )
            memset ( ( char * ) ( self + 1 ) + capacity, 0, self -> allocated - capacity );

        /* also, this behavior is reasonable only when the new capacity
           is close to the existing and/or under some minimum.
           but if dropping from 4G to 256 bytes, it's unreasonable
           to leave it alone.

           I won't change the behavior just yet, but it needs to be
           examined and tested for consequences.
        */
        return 0;
    }

    /* check reference count for copies.
       a buffer that is only referenced once is only referenced
       by a single thread ( when properly used ).
       as soon as we see a refcount < 2, we assume no possibility
       of contention.

       but if wiping is in effect, then we can't rely upon realloc()
       unless we can exactly predict when it will create a new
       allocation. It's easily detected upon return, but by then
       is too late to wipe the old allocation.
     */
    if ( ! wipe && atomic32_read ( & self -> refcount ) <= 1 )
    {
        /* realloc() can be more efficient than malloc + copy + free */
        temp = realloc ( self, capacity + sizeof * temp );
        if ( temp == NULL )
            return RC ( rcRuntime, rcBuffer, rcResizing, rcMemory, rcExhausted );
    }
    else
    {
        /* if wiping or sharing the allocation, generate another copy */
        temp = malloc ( capacity + sizeof * temp );
        if ( temp == NULL )
            return RC ( rcRuntime, rcBuffer, rcResizing, rcMemory, rcExhausted );

        /* copy from original */
        if ( self -> allocated <= capacity )
            memmove ( temp, self, self -> allocated + sizeof * temp );
        else
            memmove ( temp, self, capacity + sizeof * temp );

        /* done with original */
        release ( self, wipe );
    }

    /* check to see if clearing is in effect
       at the moment this is being written, we know that the size
       was increased. still testing for it here in case that logic
       is removed.
    */
    if ( clear && temp -> allocated < capacity )
        memset ( ( char * ) ( temp + 1 ) + temp -> allocated, 0, capacity - temp -> allocated );

    self = temp;
    self -> allocated = capacity;
    atomic32_set ( & self->refcount, 1 );
    
    * target = self;

    return 0;
}

static rc_t shrink ( buffer_impl_t ** target, size_t capacity, bool wipe )
{
    buffer_impl_t * self = * target;

    /* ignore unless downsizing and the buffer is writable */
    if ( capacity < self -> allocated && atomic32_read ( & self -> refcount ) == 1 )
    {
        buffer_impl_t * temp;

        if ( ! wipe )
            temp = realloc ( self, capacity + sizeof * temp );
        else
        {
            temp = malloc ( capacity + sizeof * temp );
            if ( temp != NULL )
            {
                memmove ( temp, self, capacity + sizeof * temp );
                memset ( self, 0, self -> allocated + sizeof * self );
                free ( self );
            }
        }
        
        if ( temp == NULL )
            return RC ( rcRuntime, rcBuffer, rcResizing, rcMemory, rcExhausted );

        temp -> allocated = capacity;
        * target = temp;
    }
    
    return 0;
}

/*
 either returns original with refcount == 2
 or returns new copy with refcount == 1
 */
static buffer_impl_t * make_copy ( buffer_impl_t * self )
{
    buffer_impl_t * copy = self;

    /* the following line checks for a unique reference with count == 1
       and increments it to a second reference if so. */
    if ( atomic32_read_and_add_eq ( & self -> refcount, 1, 1 ) != 1 )
    {
        /* falling into here, the reference count was NOT 1
           so it's not a unique reference and memory should be cloned */
        copy = malloc ( self -> allocated + sizeof * copy );
        if ( copy != NULL )
        {
            memmove ( copy, self, self -> allocated + sizeof * copy );
            atomic32_set ( & copy -> refcount, 1 );
        }
    }
    return copy;
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
    rc_t rc = 0;
    size_t bytes;
    buffer_impl_t **impp;
    
    if (target == NULL)
    	return RC(rcRuntime, rcBuffer, rcConstructing, rcParam, rcNull);

    impp = (buffer_impl_t **)&target->ignore;
    
    bytes = roundup((elem_bits * elem_count + 7) / 8, 12);
    if (8 * (uint64_t)bytes < elem_bits * elem_count)
    	return RC(rcRuntime, rcBuffer, rcConstructing, rcParam, rcTooBig);
    
    memset (target, 0, sizeof(*target));
    target->elem_bits = elem_bits;

    if ( bytes > 0 )
    {
        rc = allocate(impp, bytes, false);
        if (rc == 0) {
            target->base = (void *)get_data(*impp);
            target->elem_count = elem_count;
        }
    }

    cc ( target );

    return rc;
}

static rc_t KDataBufferResizeInt ( KDataBuffer * self,
    uint64_t new_count, bool clear, bool wipe )
{
    rc_t rc;
    buffer_impl_t *imp;
    buffer_impl_t *new_imp;
    size_t new_size;
    uint64_t bits;
    const uint8_t *new_end;
    const uint8_t *cur_end;
    const KDataBuffer * cself = self;
    
    if ( self == NULL )
    	return RC ( rcRuntime, rcBuffer, rcResizing, rcParam, rcNull );

    /* early test for deleting buffer that should not fail
       even if existing buffer was partially initialized */
    if ( new_count == 0 )
    {
        if ( ! KDataBufferWritable ( cself ) )
            return RC ( rcRuntime, rcBuffer, rcResizing, rcSelf, rcReadonly );

        self -> elem_count = 0;
        return 0; /*** no change for empty data ***/
    }

    /* there is an unwritten law that the content size of a KDataBuffer
       is 4G, i.e. 32 bits worth of bytes or 35 bits worth of bits */
    if ( ( ( new_count + 7 ) >> MAX_ELEM_COUNT_BITS ) != 0 )
        return RC ( rcRuntime, rcBuffer, rcConstructing, rcParam, rcTooBig );

    /* detect partially initialized object that was zeroed but not fully initialized */
    if ( cself -> elem_bits == 0 )
	    return RC ( rcRuntime, rcBuffer, rcResizing, rcSelf, rcCorrupt );

    /* we should never have a huge element size.
       but since it's an open structure, check it now. */
    if ( ( cself -> elem_bits + 7 ) >> MAX_ELEM_BITS_BITS != 0 )
	    return RC ( rcRuntime, rcBuffer, rcResizing, rcSelf, rcCorrupt );

    /* object has been lightly examined.
       at this point there can be no change unless the new count differs */
    if ( cself -> elem_count == new_count )
         return 0;

    /* calculate a total bit count, which must fit into a 64-bit unsigned integer.
       since new_count can be represented in MAX_ELEM_COUNT_BITS or less, and
       since self->elem_bits can be represented in MAX_ELEM_BITS_BITS or less, and
       since MAX_ELEM_COUNT_BITS + MAX_ELEM_BITS_BITS == 64, we should be okay. */
    bits = cself -> elem_bits * new_count;
    if ( ( ( bits + 7 ) >> MAX_BUFFER_BITS ) != 0 )
    	return RC ( rcRuntime, rcBuffer, rcConstructing, rcParam, rcTooBig );
    
    /* at this point, we intend to modify */
    if ( ! KDataBufferWritable ( cself ) )
        return RC ( rcRuntime, rcBuffer, rcResizing, rcSelf, rcReadonly );

    imp = (buffer_impl_t *) self -> ignore;
    if ( imp == NULL )
    {
        /* new buffer - writable by definition */
        rc = allocate(&imp, roundup((bits + 7) / 8, 12), clear);
        if (rc == 0)
        {
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
    
    new_size = roundup((bits + 7) / 8, 12);
    if (self->base == get_data(imp) && self->bit_offset == 0) {
        rc = reallocate(&imp, new_size, clear, wipe);
        if (rc == 0) {
            self->ignore = imp;
            self->base = (void *)get_data(imp);
            self->elem_count = new_count;
        }
        return rc;
    }

    /* is sub-buffer but is sole reference */
    rc = allocate(&new_imp, roundup(new_size, 12), clear);
    if (rc == 0) {
        memmove((void *)get_data(new_imp), self->base, new_size);
        release(imp, wipe);
        self->base = (void *)get_data(new_imp);
        self->ignore = new_imp;
        self->elem_count = new_count;
    }
    return rc;
}

LIB_EXPORT rc_t CC KDataBufferResize(KDataBuffer *self, uint64_t new_count)
{
    rc_t rc = KDataBufferResizeInt ( self, new_count, false, false );
    cc ( self );
    return rc;
}

static rc_t KDataBufferSubInt (const KDataBuffer *self,
    KDataBuffer *target, uint64_t start, uint64_t count)
{
    if (self == NULL || target == NULL)
    	return RC(rcRuntime, rcBuffer, rcConstructing, rcParam, rcNull);
    
    if (self->ignore == NULL) {
        if (start > 0 || ( count != 0 && count < UINT64_MAX) )
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

LIB_EXPORT rc_t CC KDataBufferSub (const KDataBuffer *self,
    KDataBuffer *target, uint64_t start, uint64_t count)
{
    rc_t rc = KDataBufferSubInt ( self, target, start, count );
    cc ( self );
    return rc;
}

static
rc_t KDataBufferCastInt(const KDataBuffer *self, KDataBuffer *target, uint64_t new_elem_bits, bool can_shrink)
{
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
    
    if (new_bits != bits && ! (can_shrink && new_bits < bits))
        return RC(rcRuntime, rcBuffer, rcCasting, rcParam, rcInvalid);

    /* check alignment - if new element size is integral power of 2 and >= 16 bits */
    if ( ( ( new_elem_bits - 1 ) & new_elem_bits ) == 0 && new_elem_bits >= 16 )
    {
        uint64_t elem_bytes = new_elem_bits >> 3;
        if ( ( ( size_t ) self -> base & ( elem_bytes - 1 ) ) != 0 )
        {
            buffer_impl_t * buffer = ( buffer_impl_t * ) self -> ignore;
            size_t total_bytes = KDataBufferBytes ( self );

#if DEBUG_ALIGNMENT
            fprintf ( stderr, "NOTICE: %s - adjusting pointer alignment of %zu byte buffer: ", __func__, total_bytes );
#endif

            /* need to realign data */
            if ( ( const KDataBuffer * ) target == self && atomic32_read ( & buffer -> refcount ) == 1 )
            {
#if DEBUG_ALIGNMENT
                fprintf ( stderr, "using memmove within buffer\n" );
#endif
                /* can simply memmove */
                memmove ( buffer + 1, target -> base, total_bytes );
                target -> base = buffer + 1;
                assert ( ( ( size_t ) target -> base & ( BASE_PTR_ALIGNMENT - 1 ) ) == 0 );

                /* perform cast */
                target-> elem_count = new_elem_count;
                target-> elem_bits = new_elem_bits;
            }
            else
            {
                /* must realloc */
                KDataBuffer tmp;
                rc_t rc = KDataBufferMakeBytes ( & tmp, total_bytes );
                if ( rc != 0 )
                    return rc;
                assert ( ( ( size_t ) tmp . base & ( BASE_PTR_ALIGNMENT - 1 ) ) == 0 );

#if DEBUG_ALIGNMENT
                fprintf ( stderr, "reallocating and copying buffer\n" );
#endif
                /* copy */
                memmove ( tmp . base, self -> base, total_bytes );

                /* if assigning target would overwrite self, whack original */
                if ( ( const KDataBuffer * ) target == self )
                    KDataBufferWhack ( target );

                /* new buffer is now output */
                * target = tmp;
            }

            return 0;
        }
    }

    if ((const KDataBuffer *)target != self)
    {
        *target = *self;
        target->ignore = test_add_ref((buffer_impl_t *)self->ignore);
    }

    target->elem_count = new_elem_count;
    target->elem_bits = new_elem_bits;

    return 0;

}

LIB_EXPORT rc_t CC KDataBufferCast(const KDataBuffer *self, KDataBuffer *target, uint64_t new_elem_bits, bool can_shrink)
{
    rc_t rc = KDataBufferCastInt ( self, target, new_elem_bits, can_shrink );
    cc ( self );
    return rc;
}

static rc_t KDataBufferMakeWritableInt (const KDataBuffer *cself, KDataBuffer *target)
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
                    release(self, false);
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
            
            rc = allocate(&copy, roundup(KDataBufferBytes(cself), 12), false);
            if (rc == 0) {
                if (cself->bit_offset == 0)
                    memmove((void *)get_data(copy), cself->base, KDataBufferBytes(cself));
                else
                    bitcpy((void *)get_data(copy), 0, cself->base, cself->bit_offset, KDataBufferBits(cself));

                if ((const KDataBuffer *)target == cself)
                    release(self, false);
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

LIB_EXPORT rc_t CC KDataBufferMakeWritable (const KDataBuffer *cself, KDataBuffer *target)
{
    rc_t rc = KDataBufferMakeWritableInt ( cself, target );
    cc ( cself );
    return rc;
}

LIB_EXPORT rc_t CC KDataBufferWhack (KDataBuffer *self)
{
    cc ( self );
    if (self)
    {
        if (self->ignore)
            release((buffer_impl_t *)self->ignore, false);

        memset(self, 0, sizeof(*self));
    }
    return 0;
}

LIB_EXPORT bool CC KDataBufferWritable(const KDataBuffer *cself)
{
    cc ( cself );
    return (cself != NULL && 
            ( cself->ignore == NULL ||
              atomic32_read(&((buffer_impl_t *)cself->ignore)->refcount) == 1 ) )
        ? true : false;
}

LIB_EXPORT rc_t CC KDataBufferShrink(KDataBuffer *self)
{
    rc_t rc = 0;
    if (self && self->ignore) {
        rc = shrink((buffer_impl_t **)&self->ignore,
            (self->elem_bits * self->elem_count + self->bit_offset + 7) / 8, false);
        cc ( self );
    }
    return rc;
}

LIB_EXPORT size_t CC KDataBufferMemorySize(KDataBuffer const *self)
{
    cc ( self );
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


/* Wipe
 *  overwrite allocated memory
 */
LIB_EXPORT rc_t CC KDataBufferWipe ( KDataBuffer * self )
{
    cc ( self );
    
    if ( self == NULL )
        return RC ( rcRuntime, rcBuffer, rcValidating, rcSelf, rcNull );

    if ( self -> ignore != NULL )
        buffer_impl_wipe ( ( buffer_impl_t * ) self -> ignore );

    cc ( self );
    
    return 0;
}


/* WipeNWhack
 *  wipes and releases memory associated with a buffer.
 */
LIB_EXPORT rc_t CC KDataBufferWipeNWhack ( KDataBuffer * self )
{
    cc ( self );
    
    if ( self != NULL )
    {
        if ( self -> ignore )
            release ( ( buffer_impl_t * ) self -> ignore, true );

        memset ( self, 0, sizeof * self );
    }
    return 0;
}


/* WipeResize
 *  make a buffer bigger or smaller.
 *  can fail if not enough memory.
 *  can fail if not writable.
 *  wipes memory before release or reallocation
 *
 *  "new_count" [ IN ] - new number of elements
 */
LIB_EXPORT rc_t CC KDataBufferWipeResize ( KDataBuffer * self, uint64_t new_count )
{
    rc_t rc = KDataBufferResizeInt ( self, new_count, true, true );
    cc ( self );
    return rc;
}
