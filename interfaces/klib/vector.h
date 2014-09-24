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

#ifndef _h_klib_vector_
#define _h_klib_vector_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * KVector
 *  encapsulated vector interface
 *
 *  a vector obeys a particular set of messages
 *  it may be implemented as an array, but not necessarily.
 *
 *  the principal property of a vector is that it acts as a map
 *  between an integer key and a value of some type. in this case,
 *  we only accept unsigned integers as keys.
 */
typedef struct KVector KVector;


/* Make
 *  create an empty vector
 */
KLIB_EXTERN rc_t CC KVectorMake ( KVector **v );


/* AddRef
 * Release
 */
KLIB_EXTERN rc_t CC KVectorAddRef ( const KVector *self );
KLIB_EXTERN rc_t CC KVectorRelease ( const KVector *self );


/* Get
 *  get an untyped value
 *  returns rc_t state of rcNull if index is not set
 *
 *  "key" [ IN ] - vector index
 *
 *  "value_buffer" [ OUT ] and "bsize" [ IN ] - return buffer for value
 *
 *  "bytes" [ OUT ] - return parameter for bytes in value
 *
 * NB - if rc_t state is rcInsufficient, "bytes" will contain
 *  the number of bytes required to access the indexed value
 */
KLIB_EXTERN rc_t CC KVectorGet ( const KVector *self, uint64_t key,
    void *value_buffer, size_t bsize, size_t *bytes );

/* Get
 *  get typed values
 *  returns rc_t state of rcNull if index is not set
 *
 *  "key" [ IN ] - vector index
 *
 *  "value" [ OUT ] - return parameter for value
 */
KLIB_EXTERN rc_t CC KVectorGetBool ( const KVector *self, uint64_t key, bool *value );

KLIB_EXTERN rc_t CC KVectorGetI8 ( const KVector *self, uint64_t key, int8_t *value );
KLIB_EXTERN rc_t CC KVectorGetI16 ( const KVector *self, uint64_t key, int16_t *value );
KLIB_EXTERN rc_t CC KVectorGetI32 ( const KVector *self, uint64_t key, int32_t *value );
KLIB_EXTERN rc_t CC KVectorGetI64 ( const KVector *self, uint64_t key, int64_t *value );

KLIB_EXTERN rc_t CC KVectorGetU8 ( const KVector *self, uint64_t key, uint8_t *value );
KLIB_EXTERN rc_t CC KVectorGetU16 ( const KVector *self, uint64_t key, uint16_t *value );
KLIB_EXTERN rc_t CC KVectorGetU32 ( const KVector *self, uint64_t key, uint32_t *value );
KLIB_EXTERN rc_t CC KVectorGetU64 ( const KVector *self, uint64_t key, uint64_t *value );

KLIB_EXTERN rc_t CC KVectorGetF32 ( const KVector *self, uint64_t key, float *value );
KLIB_EXTERN rc_t CC KVectorGetF64 ( const KVector *self, uint64_t key, double *value );

KLIB_EXTERN rc_t CC KVectorGetPtr ( const KVector *self, uint64_t key, void **value );


/* Set
 *  set an untyped value
 *
 *  "key" [ IN ] - vector index
 *
 *  "value" [ IN ] and "bytes" [ IN ] - value buffer
 */
KLIB_EXTERN rc_t CC KVectorSet ( KVector *self, uint64_t key,
    const void *value, size_t bytes );

/* Set
 *  set typed values
 *
 *  "key" [ IN ] - vector index
 *
 *  "data" [ IN ] - value
 */
KLIB_EXTERN rc_t CC KVectorSetBool ( KVector *self, uint64_t key, bool value );

KLIB_EXTERN rc_t CC KVectorSetI8 ( KVector *self, uint64_t key, int8_t value );
KLIB_EXTERN rc_t CC KVectorSetI16 ( KVector *self, uint64_t key, int16_t value );
KLIB_EXTERN rc_t CC KVectorSetI32 ( KVector *self, uint64_t key, int32_t value );
KLIB_EXTERN rc_t CC KVectorSetI64 ( KVector *self, uint64_t key, int64_t value );

KLIB_EXTERN rc_t CC KVectorSetU8 ( KVector *self, uint64_t key, uint8_t value );
KLIB_EXTERN rc_t CC KVectorSetU16 ( KVector *self, uint64_t key, uint16_t value );
KLIB_EXTERN rc_t CC KVectorSetU32 ( KVector *self, uint64_t key, uint32_t value );
KLIB_EXTERN rc_t CC KVectorSetU64 ( KVector *self, uint64_t key, uint64_t value );

KLIB_EXTERN rc_t CC KVectorSetF32 ( KVector *self, uint64_t key, float value );
KLIB_EXTERN rc_t CC KVectorSetF64 ( KVector *self, uint64_t key, double value );

KLIB_EXTERN rc_t CC KVectorSetPtr ( KVector *self, uint64_t key, const void *value );


/* Unset
 *  unset an entry
 *  returns rc_t state rcNull if index is not set
 *
 *  "key" [ IN ] - vector index
 */
KLIB_EXTERN rc_t CC KVectorUnset ( KVector *self, uint64_t key );


/* Visit
 *  executes a function on each key/value pair
 *  returns early if "f" returns non-zero rc_t
 *
 *  "reverse" [ IN ] - if true, execute in reverse order
 *
 *  "f" [ IN ] and "user_data" [ IN, OPAQUE ] - function to be
 *  executed on each vector element or until the function
 *  returns true.
 */
KLIB_EXTERN rc_t CC KVectorVisit ( const KVector *self, bool reverse,
    rc_t ( CC * f ) ( uint64_t key, const void *value, size_t bytes, void *user_data ),
    void *user_data );

/* Visit
 *  visit typed values
 *  returns early if "f" returns non-zero rc_t
 *
 *  "reverse" [ IN ] - if true, execute in reverse order
 *
 *  "f" [ IN ] and "user_data" [ IN, OPAQUE ] - function to be
 *  executed on each vector element or until the function
 *  returns true.
 */
KLIB_EXTERN rc_t CC KVectorVisitBool ( const KVector *self, bool reverse,
    rc_t ( CC * f ) ( uint64_t key, bool value, void *user_data ),
    void *user_data );
KLIB_EXTERN rc_t CC KVectorVisitI64 ( const KVector *self, bool reverse,
    rc_t ( CC * f ) ( uint64_t key, int64_t value, void *user_data ),
    void *user_data );
KLIB_EXTERN rc_t CC KVectorVisitU64 ( const KVector *self, bool reverse,
    rc_t ( CC * f ) ( uint64_t key, uint64_t value, void *user_data ),
    void *user_data );
KLIB_EXTERN rc_t CC KVectorVisitF64 ( const KVector *self, bool reverse,
    rc_t ( CC * f ) ( uint64_t key, double value, void *user_data ),
    void *user_data );
KLIB_EXTERN rc_t CC KVectorVisitPtr ( const KVector *self, bool reverse,
    rc_t ( CC * f ) ( uint64_t key, const void *value, void *user_data ),
    void *user_data );



/*--------------------------------------------------------------------------
 * Vector
 *  simple vector of void*
 */
typedef struct Vector Vector;
struct Vector
{
    void **v;
    uint32_t start;
    uint32_t len;
    uint32_t mask;
};

/* Init
 *  initialize vector
 *
 *  "start" [ IN ] - initial index to vector, normally 0
 *
 *  "block" [ IN ] - selects the number of elements in
 *  a vector block; used for allocating and extending
 */
KLIB_EXTERN void CC VectorInit ( Vector *self, uint32_t start, uint32_t block );


/* Copy
 *  initialize a new vector from an existing one
 *  performs a shallow copy
 */
KLIB_EXTERN rc_t CC VectorCopy ( const Vector *self, Vector *copy );


/* Start
 *  returns the starting index
 */
#define VectorStart( self ) \
    ( * ( const uint32_t* ) & ( ( self ) -> start ) )


/* Length
 *  return the vector length
 */
#define VectorLength( self ) \
    ( * ( const uint32_t* ) & ( ( self ) -> len ) )


/* Block
 *  return the vector expansion block size
 */
#define VectorBlock( self ) \
    ( * ( const uint32_t* ) & ( ( self ) -> mask ) + 1 )


/* Get
 *  retrieve an indexed element
 */
KLIB_EXTERN void* CC VectorGet ( const Vector *self, uint32_t idx );


/* Set
 *  sets an indexed element
 *  extends vector as required
 *
 *  "idx" [ IN ] - item index
 *
 *  "item" [ IN, OPAQUE ] - item to be appended to vector
 *
 *  returns rcExists if element is not NULL
 */
KLIB_EXTERN rc_t CC VectorSet ( Vector *self, uint32_t idx, const void *item );


/* Swap
 *  swaps value of an indexed element
 *  behaves like Set except that it allows overwrite
 *
 *  "idx" [ IN ] - item index
 *
 *  "item" [ IN, OPAQUE ] - new element value
 *
 *  "prior" [ OUT ] - prior element value
 */
KLIB_EXTERN rc_t CC VectorSwap ( Vector *self, uint32_t idx,
    const void *item, void **prior );


/* Append
 *  appends item to end of vector
 *
 *  "idx" [ OUT, NULL OKAY ] - optional return parameter for item index
 *
 *  "item" [ IN, OPAQUE ] - item to be appended to vector
 */
KLIB_EXTERN rc_t CC VectorAppend ( Vector *self, uint32_t *idx, const void *item );


/* First
 * Last
 *  get item at either end of vector
 */
KLIB_EXTERN void* CC VectorFirst ( const Vector *self );
KLIB_EXTERN void* CC VectorLast ( const Vector *self );


/* Find
 *  find an object within ordered vector
 *
 *  "key" [ IN ] - find criteria
 *
 *  "idx" [ OUT, NULL OKAY ] - optional return parameter for item index
 *
 *  "cmp" [ IN ] - comparison function that returns
 *  equivalent of "key" - "n"
 */
KLIB_EXTERN void* CC VectorFind ( const Vector *self, const void *key, uint32_t *idx,
    int ( CC * cmp ) ( const void *key, const void *n ) );


/* Insert
 *  insert an object to vector, even if duplicate
 *
 *  "item" [ IN ] - object to insert
 *
 *  "idx" [ OUT, NULL OKAY ] - optional return parameter for item index
 *
 *  "sort" [ IN ] - comparison function that returns
 *  equivalent of "item" - "n"
 *
 *  the treatment of order for items reported as identical
 *  i.e. sort function returns zero when they are compared,
 *  is undefined.
 *
 *  the current implementation treats '<=' as '<' such
 *  that all inserts are converted to a '<' or '>' comparison,
 *  but this should not be relied upon.
 */
KLIB_EXTERN rc_t CC VectorInsert ( Vector *self, const void *item, uint32_t *idx,
    int ( CC * sort ) ( const void *item, const void *n ) );


/* InsertUnique
 *  insert an object to vector, but only if unique.
 *
 *  "item" [ IN ] - object to insert
 *
 *  "idx" [ OUT, NULL OKAY ] - optional return parameter for
 *  item index. when return code state is rcExists, this value
 *  will be that of the existing item.
 *
 *  "sort" [ IN ] - comparison function that returns
 *  equivalent of "item" - "n"
 */
KLIB_EXTERN rc_t CC VectorInsertUnique ( Vector *self, const void *item, uint32_t *idx,
    int ( CC * sort ) ( const void *item, const void *n ) );


/* Merge
 *  merge a sorted vector into an existing, sorted vector
 *
 *  "unique" [ IN ] - eliminate duplicates if true
 *
 *  "v" [ IN ] - sorted vector of elements to merge
 *
 *  "sort" [ IN ] - comparison function that returns
 *  equivalent of "item" - "n"
 */
KLIB_EXTERN rc_t CC VectorMerge ( Vector *self, bool unique, const Vector *v,
    int ( CC * sort ) ( const void *item, const void *n ) );


/* Remove
 *  retrieves value at given index
 *  shifts all elements to right of index toward start,
 *  preserving order and decrements length by 1
 *  return removed item
 *
 *  "idx" [ IN ] - item index
 *
 *  "removed" [ OUT ] - removed element value
 */
KLIB_EXTERN rc_t CC VectorRemove ( Vector *self, uint32_t idx, void **removed );


/* Reorder
 *  execute ksort on vector with provided function
 */
KLIB_EXTERN void CC VectorReorder ( Vector *self,
    int ( CC * cmp ) ( const void**, const void**, void *data ), void *data );


/* ForEach
 *  executes a function on each vector element
 *
 *  "reverse" [ IN ] - if true, execute in reverse order
 *
 *  "f" [ IN ] and "data" [ IN, OPAQUE ] - function to be
 *  executed on each vector element
 */
KLIB_EXTERN void CC VectorForEach ( const Vector *self, bool reverse,
    void ( CC * f ) ( void *item, void *data ), void *data );


/* DoUntil
 *  executes a function on each element
 *  until the function returns true
 *
 *  "reverse" [ IN ] - if true, execute in reverse order
 *
 *  "f" [ IN ] and "data" [ IN, OPAQUE ] - function to be
 *  executed on each vector element or until the function
 *  returns true.
 *
 *  return values:
 *    false unless "f" returns true
 */
KLIB_EXTERN bool CC VectorDoUntil ( const Vector *self, bool reverse,
    bool ( CC * f ) ( void *item, void *data ), void *data );


/* Whack
 *  removes entries from vector and
 *  executes a user provided destructor
 */
KLIB_EXTERN void CC VectorWhack ( Vector *self,
    void ( CC * whack ) ( void *item, void *data ), void *data );


#ifdef __cplusplus
}
#endif

#endif /* _h_klib_vector_ */
