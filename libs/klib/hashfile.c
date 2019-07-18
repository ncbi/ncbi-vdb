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
#include <arch-impl.h>
#include <assert.h>
#include <atomic.h>
#include <kfs/mmap.h>
#include <klib/hashfile.h>
#include <klib/rc.h>
#include <klib/vector.h>
#include <kproc/lock.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if LINUX
#include <sys/mman.h>
#endif

#undef memcpy

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char u8;
static u8 *           BUCKET_INVALID   = (u8 *)0; /* Must be 0, calloc fills */
static u8 *           BUCKET_INVISIBLE = (u8 *)1;
#define NUM_SEGMENTS 2048

/*#define MIN(a, b) ((a) > (b) ? (b) : (a)) */
#define MAX( a, b ) ( ( a ) < ( b ) ? ( b ) : ( a ) )

#if defined( __GNUC__ )
#define COMPILER_BARRIER __asm__ __volatile__( "" ::: "memory" )
#elif defined( _MSC_VER )
#define COMPILER_BARRIER _ReadWriteBarrier()
#else
/* C11's stdatomic */
#define COMPILER_BARRIER atomic_thread_fence( memory_order_release )
#endif

typedef struct Hashtable
{
    u8 **  table;
    size_t table_sz;
} Hashtable;

typedef struct Segment
{
    Hashtable * hashtable; /* Will be switched atomically so readers can be
                              lock-free */
    size_t  load;          /* Including invisible entries */
    size_t  max_load;      /* When to rehash */
    KLock * seglock;
    u8 *    alloc_base;
    size_t  alloc_remain;
} Segment;

struct KHashFile
{
    KFile * file;
#if _ARCH_BITS == 32
    atomic32_t count;
#else
    atomic64_t count;
#endif
    size_t  iter_seg;
    size_t  iter_ent;
    Segment segments[NUM_SEGMENTS];
    KLock * alloc_lock; /* protects below */
    u8 *    alloc_base;
    size_t  alloc_remain;
    size_t  alloc_chunk;
    Vector  allocs;
};

/* Hash/Key/Values are encoded as:
 * 8 bytes hash - 6 enough for a 256 trillion entry set requiring 5PB.
 * 1 byte key length, if key length >255 store 255 and actual size below
 * 1 byte value length, if value length >255 store 255 and actual size below
 * [ 8 bytes ] if key length >255, store length here
 * [ 8 bytes ] if value length >255, store length here
 * key
 * value
 *
 * So storage overhead of keys/values is usually 10 (for small hkv) +
 * ~4.5*8 (current and previous bucket pointers)=46 bytes per entry.
 */
typedef struct HKV
{
    uint64_t     hash;
    size_t       key_size;
    size_t       value_size;
    const void * key;
    const void * value;
} HKV;

static size_t which_segment( uint64_t hash )
{
    hash = hash >> 8;
    hash *= 11812119942205477693ULL; /* Random prime */
    hash = uint64_ror( hash, 53 );
    return hash % NUM_SEGMENTS;
}

static size_t hkv_space( const HKV * in )
{
    size_t out = 0;
    out += sizeof( uint64_t ); /* hash */
    out += 2;                  /* key & value_size */
    if ( in->key_size > 254 )
        out += sizeof( in->key_size ); /* optional key_size */
    if ( in->value_size > 254 )
        out += sizeof( in->value_size ); /* optional value_size */
    out += in->key_size;                 /* key */
    out += in->value_size;               /* value */
    return out;
}

static void hkv_decode( const u8 * in, HKV * out )
{
    const u8 * p = in;
    memcpy( &out->hash, p, sizeof( out->hash ) );
    p += sizeof( out->hash );
    out->key_size = *p;
    p += 1;
    out->value_size = *p;
    p += 1;
    if ( out->key_size == 255 )
    {
        memcpy( &out->key_size, p, sizeof( out->key_size ) );
        p += sizeof( out->key_size );
    }
    if ( out->value_size == 255 )
    {
        memcpy( &out->value_size, p, sizeof( out->value_size ) );
        p += sizeof( out->value_size );
    }
    out->key = (const void *)p;
    p += out->key_size;
    out->value = (const void *)p;
    p += out->value_size;
}

static void hkv_encode( const HKV * in, u8 * out )
{
    u8 * p = out;
    memcpy( p, &in->hash, sizeof( in->hash ) );
    p += sizeof( in->hash );

    if ( in->key_size <= 254 )
        *p = (u8)in->key_size;
    else
        *p = 255;
    ++p;

    if ( in->value_size <= 254 )
        *p = (u8)in->value_size;
    else
        *p = 255;
    ++p;

    if ( in->key_size >= 255 )
    {
        memcpy( p, &in->key_size, sizeof( in->key_size ) );
        p += sizeof( in->key_size );
    }

    if ( in->value_size >= 255 )
    {
        memcpy( p, &in->value_size, sizeof( in->value_size ) );
        p += sizeof( in->value_size );
    }

    switch ( in->key_size ) /* Optimize common sizes */
    {
        case 4:
            memcpy( p, in->key, 4 );
            break;
        case 8:
            memcpy( p, in->key, 8 );
            break;
        default:
            memcpy( p, in->key, in->key_size );
    }

    p += in->key_size;

    switch ( in->value_size )
    {
        case 4:
            memcpy( p, in->value, 4 );
            break;
        case 8:
            memcpy( p, in->value, 8 );
            break;
        default:
            memcpy( p, in->value, in->value_size );
    }

    p += in->value_size;
#if _DEBUGGING
    HKV test;
    hkv_decode( out, &test );
    if ( test.hash != in->hash )
    {
        /* USE THE DEBUGGING FACILITIES IN KLIB RATHER THAN fprintf(stderr).
           they handle arguments in a platform and architecture neutral fashion
           and are way cooler besides. */
        fprintf( stderr, "hash mismatch %llx %llx\n", ( long long unsigned ) test.hash, ( long long unsigned ) in->hash );
        abort();
    }
    if ( test.key_size != in->key_size )
    {
        fprintf( stderr, "key_size mismatch %ld %ld\n", test.key_size,
                 in->key_size );
        abort();
    }
    if ( test.value_size != in->value_size )
    {
        fprintf( stderr, "value_size mismatch\n" );
        abort();
    }
    if ( memcmp( in->key, test.key, in->key_size ) )
    {
        fprintf( stderr, "key mismatch\n" );
        abort();
    }
    if ( memcmp( in->value, test.value, in->value_size ) )
    {
        fprintf( stderr, "value mismatch\n" );
        abort();
    }
#endif
}

#if 0
static void dump(const KHashFile * self)
{
    assert(self != NULL);
    if (self->num_buckets > 40) return;
    fprintf(stderr, "-- table has %ld/%ld\n", self->count, self->load);
    for (size_t bucket = 0; bucket != self->num_buckets; bucket++) {
        const char* bucketptr = (char*)self->buckets;
        const char* hashptr = bucketptr;
        const char* keyptr = bucketptr + 8;
        uint64_t buckethash=0;
        fprintf(stderr, "   bucket %03ld hash %lx", bucket, buckethash);
        if (buckethash & BUCKET_VALID) fprintf(stderr, " val");
        if (buckethash & BUCKET_VISIBLE) fprintf(stderr, " vis");
        /*fprintf(stderr, " key=%ld", key); */
        fprintf(stderr, "\n");
    }
}
#endif

/* Single, locked allocator shared between all segments. */
static void * map_calloc( KHashFile * self, size_t size )
{
    rc_t   rc;
    void * block = NULL;

    if ( self == NULL || size == 0 )
    {
        return NULL;
    }

    rc = KLockAcquire( self->alloc_lock );
    if ( rc )
    {
        return NULL;
    }

    if ( size > self->alloc_remain )
    {
        size_t req = size;

        /* /proc/sys/vm/max_map_count limits us to about 64K mmaps per
         * process.
         * Since we don't know how large output file is going to be, we need
         * to
         * get creative. 1MB * 1.25^N will allow 1TB when N=62, 1PB at N=93.
         */
        if ( self->alloc_chunk == 0 )
            self->alloc_chunk = 1048576;
        else
        {
            self->alloc_chunk *= 5;
            self->alloc_chunk /= 4;
        }

        req = MAX( req, self->alloc_chunk );
        /* Round up to 4K page size */
        req = ( ( req + 4095 ) / 4096 ) * 4096;
        if ( self->file != NULL )
        {
            uint64_t filesize;
            rc = KFileSize( self->file, &filesize );
            if ( rc )
            {
                KLockUnlock( self->alloc_lock );
                return NULL;
            }

            rc = KFileSetSize( self->file, filesize + req );
            if ( rc )
            {
                KLockUnlock( self->alloc_lock );
                return NULL;
            }

            KMMap * mm = NULL;
            rc         = KMMapMakeRgnUpdate( &mm, self->file, filesize, req );
            if ( rc )
            {
                KLockUnlock( self->alloc_lock );
                return NULL;
            }

            rc = KMMapAddrUpdate( mm, (void *)&self->alloc_base );
            if ( rc )
            {
                KLockUnlock( self->alloc_lock );
                return NULL;
            }
#if LINUX
            /* Not sure this helps */
            madvise( (void *)self->alloc_base, req, MADV_RANDOM );
#endif

            self->alloc_remain = req;
            VectorAppend( &self->allocs, NULL, (void *)mm );
        }
        else
        {
            req                = MAX( size, 1048576 );
            self->alloc_base   = calloc( 1, req );
            self->alloc_remain = req;
            VectorAppend( &self->allocs, NULL, self->alloc_base );
        }
    }

    block = self->alloc_base;
    self->alloc_base += size;
    self->alloc_remain -= size;

    KLockUnlock( self->alloc_lock );

    return block;
}

/* Per segment allocator */
static void * seg_alloc( KHashFile * self, size_t segment, size_t size )
{
    assert( self != NULL );
    assert( segment < NUM_SEGMENTS );
    assert( size > 0 );

    if ( size % 8 != 0 )
        size += 8 - ( size % 8 ); /* Round up for alignment */

    Segment * seg = &self->segments[segment];
    if ( size > seg->alloc_remain )
    {
        size_t req      = MAX( size, 4096 );
        seg->alloc_base = map_calloc( self, req );
        if ( !seg->alloc_base )
        {
            return NULL;
        }
        seg->alloc_remain = req;
    }

    void * r = seg->alloc_base;
    seg->alloc_base += size;
    seg->alloc_remain -= size;

    return r;
}

static rc_t rehash_segment( KHashFile * self, size_t segment, size_t capacity )
{
    assert( self != NULL );
    assert( segment < NUM_SEGMENTS );

    /* Requires segment locked by caller */

    Segment * seg = &self->segments[segment];

    Hashtable * old_hashtable = seg->hashtable;

    /* Don't allow shrinking */
    if ( old_hashtable )
        capacity = MAX( capacity, old_hashtable->table_sz );

    uint64_t lg2 = (uint64_t)uint64_msbit( capacity | 1 );
    capacity     = 1ULL << lg2;
    /* Table must be 64-bit aligned for atomic updates, seg_alloc does */
    Hashtable * new_hashtable
        = (Hashtable *)seg_alloc( self, segment, sizeof( Hashtable ) );
    if ( !new_hashtable )
    {
        return RC( rcCont, rcTrie, rcInserting, rcMemory, rcExhausted );
    }

    new_hashtable->table_sz = capacity;
    new_hashtable->table
        = seg_alloc( self, segment, capacity * sizeof( u8 * ) );
    if ( !new_hashtable->table )
    {
        return RC( rcCont, rcTrie, rcInserting, rcMemory, rcExhausted );
    }

    if ( old_hashtable )
    {
        seg->load                   = 0;
        u8 **          old_table    = old_hashtable->table;
        u8 **          new_table    = new_hashtable->table;
        size_t         old_table_sz = old_hashtable->table_sz;
        size_t         new_table_sz = new_hashtable->table_sz;
        const uint64_t new_mask     = new_table_sz - 1;
        for ( size_t old_bucket = 0; old_bucket != old_table_sz; ++old_bucket )
        {
            u8 * okv = old_table[old_bucket];
            if ( okv != BUCKET_INVALID && okv != BUCKET_INVISIBLE )
            {
                size_t triangle = 0;
                HKV    obkv;
                hkv_decode( okv, &obkv );
                uint64_t new_bucket = obkv.hash;
                while ( 1 )
                {
                    new_bucket &= new_mask;
                    u8 * nkv = new_table[new_bucket];
                    if ( nkv == BUCKET_INVALID )
                    {
                        new_table[new_bucket] = okv;
                        ++seg->load;
                        break;
                    }
                    ++triangle;
                    new_bucket += ( triangle * ( triangle + 1 ) / 2 );
                }
            }
        }
    }
    COMPILER_BARRIER;
    seg->hashtable = new_hashtable;
    /* New max_load should be 0.5..0.7X table_sz */
    long int ratio = 50 + segment % 20;
    seg->max_load  = seg->hashtable->table_sz * ratio / 100;

    return 0;
}

LIB_EXPORT rc_t KHashFileMake( KHashFile ** self, KFile * hashfile )
{
    if ( self == NULL )
        return RC( rcCont, rcTrie, rcConstructing, rcParam, rcInvalid );

    rc_t rc;
    *self = NULL;

    KHashFile * kht = (KHashFile *)malloc( sizeof( KHashFile ) );
    if ( kht == NULL )
        return RC( rcCont, rcTrie, rcConstructing, rcMemory, rcExhausted );

    kht->file = hashfile;
#if _ARCH_BITS == 32
    atomic32_set( &kht->count, 0 );
#else
    atomic64_set( &kht->count, 0 );
#endif
    VectorInit( &kht->allocs, 0, 0 );
    kht->iter_seg     = NUM_SEGMENTS;
    kht->alloc_base   = NULL;
    kht->alloc_remain = 0;
    kht->alloc_chunk  = 0;
    rc                = KLockMake( &kht->alloc_lock );
    if ( rc )
        return rc;
    for ( size_t i = 0; i != NUM_SEGMENTS; ++i )
    {
        kht->segments[i].hashtable = NULL;
        kht->segments[i].load      = 0;
        KLockMake( &kht->segments[i].seglock );
        if ( rc )
            return rc;
        kht->segments[i].alloc_base   = NULL;
        kht->segments[i].alloc_remain = 0;
        rc                            = rehash_segment( kht, i, 64 );

        if ( rc )
        {
            free( kht );
            return rc;
        }
    }

    *self = kht;

    return 0;
}

LIB_EXPORT void KHashFileDispose( KHashFile * self )
{
    if ( self == NULL )
        return;

#if _ARCH_BITS == 32
    atomic32_set( &self->count, 0 );
#else
    atomic64_set( &self->count, 0 );
#endif

    for ( size_t i = 0; i != NUM_SEGMENTS; ++i )
    {
        self->segments[i].hashtable = NULL;
        KLockRelease( self->segments[i].seglock );
        self->segments[i].alloc_base   = NULL;
        self->segments[i].alloc_remain = 0;
    }

    for ( uint32_t i = 0; i != VectorLength( &self->allocs ); ++i )
    {
        void * alloc = VectorGet( &self->allocs, i );
        if ( self->file == NULL )
        {
            free( alloc );
        }
        else
        {
            KMMapRelease( (KMMap *)alloc );
        }
    }
    self->iter_seg     = NUM_SEGMENTS;
    self->alloc_base   = NULL;
    self->alloc_remain = 0;
    self->alloc_chunk  = 0;
    VectorWhack( &self->allocs, NULL, NULL );
    KLockRelease( self->alloc_lock );

    memset( self, 0, sizeof( KHashFile ) );
    free( self );
}

LIB_EXPORT size_t KHashFileCount( const KHashFile * self )
{
    if ( self != NULL )
#if _ARCH_BITS == 32
        return (size_t)atomic32_read( &self->count );
#else
        return (size_t)atomic64_read( &self->count );
#endif
    else
        return 0;
}

LIB_EXPORT bool KHashFileFind( const KHashFile * self, const void * key,
                               const size_t key_size, const uint64_t keyhash,
                               void * value, size_t * value_size )
{
    if ( self == NULL )
        return RC( rcCont, rcTrie, rcInserting, rcParam, rcInvalid );

    size_t            triangle  = 0;
    uint64_t          bucket    = keyhash;
    size_t            segment   = which_segment( bucket );
    const Segment *   seg       = &self->segments[segment];
    const Hashtable * hashtable = seg->hashtable;
    /* Not sure barrier required, clang 3.8/gcc 4.9 don't resolve
     * hashtable->table or hashtable->sz before seg->hashtable and I'm
     * not sure why any future optimizer would. Doesn't affect assembly, so
     * leaving it in for now.
     */
    COMPILER_BARRIER;
    u8 **          table = hashtable->table;
    const uint64_t mask  = hashtable->table_sz - 1;

    while ( 1 )
    {
        bucket &= mask;
        const u8 * kv = table[bucket];
        if ( kv == BUCKET_INVALID )
            return false;

        if ( kv != BUCKET_INVISIBLE )
        {
            HKV bkv;
            hkv_decode( kv, &bkv );
            bool found = false;
            if ( bkv.hash == keyhash ) /* hash hit */
                if ( bkv.key_size == key_size )
                    switch ( key_size ) /* Optimize common sizes */
                    {
                        case 4:
                            found = ( memcmp( key, bkv.key, 4 ) == 0 );
                            break;
                        case 8:
                            found = ( memcmp( key, bkv.key, 8 ) == 0 );
                            break;
                        default:
                            found = ( memcmp( key, bkv.key, key_size ) == 0 );
                    }

            if ( found )
            {
                if ( value )
                    memcpy( value, bkv.value, bkv.value_size );
                if ( value_size )
                    *value_size = bkv.value_size;
                return true;
            }
        }

        /* To improve lookups when hash function has poor
         * distribution, we use
         * quadratic probing with a triangle sequence: 0,1,3,6,10...
         * This will allow complete coverage on a % 2^N hash table.
         */
        ++triangle;
        bucket += ( triangle * ( triangle + 1 ) / 2 );
    }
}

LIB_EXPORT rc_t KHashFileAdd( KHashFile * self, const void * key,
                              const size_t key_size, const uint64_t keyhash,
                              const void * value, const size_t value_size )
{
    if ( self == NULL )
        return RC( rcCont, rcTrie, rcInserting, rcParam, rcInvalid );

    if ( key == NULL || key_size == 0 )
        return RC( rcCont, rcTrie, rcInserting, rcParam, rcInvalid );
    size_t    triangle = 0;
    uint64_t  bucket   = keyhash;
    size_t    segment  = which_segment( bucket );
    Segment * seg      = &self->segments[segment];
    rc_t      rc       = 0;

    KLockAcquire( seg->seglock );

    Hashtable * hashtable = seg->hashtable;

    u8 **          table = hashtable->table;
    const uint64_t mask  = hashtable->table_sz - 1;
    HKV            hkv;
    hkv.hash       = keyhash;
    hkv.key_size   = key_size;
    hkv.value_size = value_size;
    hkv.key        = (const void *)key;
    hkv.value      = (const void *)value;
    size_t kvsize  = hkv_space( &hkv );

    while ( 1 )
    {
        bucket &= mask;
        u8 * kv = table[bucket];
        if ( kv == BUCKET_INVALID )
        {
            void * buf = seg_alloc( self, segment, kvsize );
            if ( !buf )
                return RC( rcCont, rcTrie, rcInserting, rcMemory,
                           rcExhausted );
            hkv_encode( &hkv, buf );

            /* Intel 64 and IA-32 Architectures Software Developers
             * Manual
             * Volume 3 (System Programming Guide)
             * 8.2.2 Memory Ordering in P6 and More Recent Processor
             * Families:
             *  * Writes to memory are not reordered with other
             * writes, ...
             *
             * So as long as we update buckets last, lock-free readers
             * (aka the HashFileFinds will never see a partial
             * key/value.
             *
             * We do need to ensure the _compiler_ doesn't reorder the
             * stores however, which is what the compiler barrier is for.
             */
            COMPILER_BARRIER;
            table[bucket] = buf;
            ++seg->load;
            rc = 0;
            if ( seg->load > seg->max_load )
                rc = rehash_segment( self, segment, hashtable->table_sz * 2 );
#if _ARCH_BITS == 32
            atomic32_inc( &self->count );
#else
            atomic64_inc( &self->count );
#endif

            KLockUnlock( seg->seglock );

            return rc;
        }

        if ( kv != BUCKET_INVISIBLE )
        {
            HKV bkv;
            hkv_decode( kv, &bkv );
            bool found = false;

            if ( bkv.hash == keyhash ) /* hash hit */
                if ( bkv.key_size == key_size )
                    switch ( key_size ) /* Optimize common sizes */
                    {
                        case 4:
                            found = ( memcmp( key, bkv.key, 4 ) == 0 );
                            break;
                        case 8:
                            found = ( memcmp( key, bkv.key, 8 ) == 0 );
                            break;
                        default:
                            found = ( memcmp( key, bkv.key, key_size ) == 0 );
                    }
            if ( found )
            {
                /* replacement */
                if ( value_size )
                {
                    if ( bkv.value_size == value_size
                         && ( memcmp( bkv.value, value, value_size ) == 0 ) )
                    {
                        /* Identical */

                        KLockUnlock( seg->seglock );
                        return 0;
                    }

                    void * buf = seg_alloc( self, segment, kvsize );
                    if ( !buf )
                        return RC( rcCont, rcTrie, rcInserting, rcMemory,
                                   rcExhausted );
                    hkv_encode( &hkv, buf );

                    COMPILER_BARRIER;
                    table[bucket] = buf;
                }
                KLockUnlock( seg->seglock );
                return 0;
            }
        }

        ++triangle;
        bucket += ( triangle * ( triangle + 1 ) / 2 );
    }
}

LIB_EXPORT bool KHashFileDelete( KHashFile * self, const void * key,
                                 const size_t key_size, uint64_t keyhash )
{
    if ( self == NULL )
        return RC( rcCont, rcTrie, rcInserting, rcParam, rcInvalid );

    size_t    triangle = 0;
    uint64_t  bucket   = keyhash;
    size_t    segment  = which_segment( bucket );
    Segment * seg      = &self->segments[segment];

    KLockAcquire( seg->seglock );

    Hashtable * hashtable = seg->hashtable;
    COMPILER_BARRIER;
    u8 **          table = hashtable->table;
    const uint64_t mask  = hashtable->table_sz - 1;

    while ( 1 )
    {
        bucket &= mask;
        u8 * kv = table[bucket];
        if ( kv == BUCKET_INVALID )
        {
            KLockUnlock( seg->seglock );

            return false;
        }

        if ( kv != BUCKET_INVISIBLE )
        {
            HKV bkv;
            hkv_decode( kv, &bkv );

            if ( bkv.hash == keyhash && /* hash hit */
                 bkv.key_size == key_size
                 && ( memcmp( key, bkv.key, key_size ) == 0 ) )
            {
                table[bucket] = BUCKET_INVISIBLE;
#if _ARCH_BITS == 32
                atomic32_dec( &self->count );
#else
                atomic64_dec( &self->count );
#endif
                KLockUnlock( seg->seglock );
                return true;
            }
        }

        ++triangle;
        bucket += ( triangle * ( triangle + 1 ) / 2 );
    }
}

LIB_EXPORT rc_t KHashFileReserve( KHashFile * self, size_t capacity )
{
    uint64_t lg2       = (uint64_t)uint64_msbit( capacity | 1 );
    capacity           = 1ULL << ( lg2 + 1 );
    size_t per_segment = 2 * capacity / NUM_SEGMENTS;

    rc_t rc;
    for ( size_t i = 0; i != NUM_SEGMENTS; ++i )
    {
        Segment * seg = &self->segments[i];
        KLockAcquire( seg->seglock );
        rc = rehash_segment( self, i, per_segment );
        KLockUnlock( seg->seglock );
        if ( rc )
            return rc;
    }

    return 0;
}

LIB_EXPORT void KHashFileIteratorMake( KHashFile * self )
{
    if ( self == NULL )
        return;

    self->iter_seg = 0;
    self->iter_ent = 0;
}

LIB_EXPORT bool KHashFileIteratorNext( KHashFile * self, void * key,
                                       size_t * key_size, void * value,
                                       size_t * value_size )
{
    if ( self == NULL )
        return false;

    while ( 1 )
    {
        if ( self->iter_seg >= NUM_SEGMENTS )
            return false;

        const Segment *   seg       = &self->segments[self->iter_seg];
        const Hashtable * hashtable = seg->hashtable;
        u8 **             table     = hashtable->table;
        const u8 *        kv        = table[self->iter_ent];

        ++self->iter_ent;
        if ( self->iter_ent >= hashtable->table_sz )
        {
            self->iter_ent = 0;
            ++self->iter_seg;
        }

        if ( kv != BUCKET_INVALID && kv != BUCKET_INVISIBLE )
        {
            HKV bkv;
            hkv_decode( kv, &bkv );
            if ( key )
                memcpy( key, bkv.key, bkv.key_size );
            if ( key_size )
                *key_size = bkv.key_size;
            if ( value )
                memcpy( value, bkv.value, bkv.value_size );
            if ( value_size )
                *value_size = bkv.value_size;
            return true;
        }
    }
}

#ifdef __cplusplus
}
#endif
