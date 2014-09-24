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
#include <klib/vector.h>
#include <klib/sort.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * Vector
 *  simple vector of void*
 */


/* Init
 *  initialize vector
 *
 *  "block" [ IN ] - selects the number of elements in
 *  a vector block; used for allocating and extending
 */
LIB_EXPORT void CC VectorInit ( Vector *self, uint32_t start, uint32_t block )
{
    if ( self != NULL )
    {
        self -> v = NULL;
        self -> start = start;
        self -> len = 0;

        if ( block == 0 )
            block = 64;

        if ( ( ( block - 1 ) & block ) != 0 )
        {
            uint32_t blk;
            for ( blk = 1; blk < block; blk <<= 1 )
                ( void ) 0;
            block = blk;
        }

        /* force a minimum of two */
        self -> mask = ( block - 1 ) | 1;
    }
}


/* Copy
 *  initialize a new vector from an existing one
 *  performs a shallow copy
 */
LIB_EXPORT rc_t CC VectorCopy ( const Vector *self, Vector *copy )
{
    uint32_t cap;

    if ( copy == NULL )
        return RC ( rcCont, rcVector, rcCopying, rcParam, rcNull );
    if ( self == NULL )
        return RC ( rcCont, rcVector, rcCopying, rcSelf, rcNull );

    VectorInit ( copy, self -> start, self -> mask + 1 );
    cap = ( self -> len + self -> mask ) & ~ self -> mask;
    copy -> v = malloc ( sizeof copy -> v [ 0 ] * cap );
    if ( copy -> v == NULL )
        return RC ( rcCont, rcVector, rcCopying, rcMemory, rcExhausted );

    memcpy ( copy -> v, self -> v, self -> len * sizeof copy -> v [ 0 ] );
    copy -> len = self -> len;

    return 0;
}


/* Get
 *  retrieve an indexed element
 */
LIB_EXPORT  void* CC VectorGet ( const Vector *self, uint32_t idx )
{
    if ( self != NULL && idx >= self -> start )
    {
        idx -= self -> start;
        if ( idx < self -> len )
            return self -> v [ idx ];
    }
    return NULL;
}


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
LIB_EXPORT rc_t CC VectorSet ( Vector *self, uint32_t idx, const void *item )
{
    uint32_t curcap, newcap;

    if ( self == NULL )
        return RC ( rcCont, rcVector, rcInserting, rcSelf, rcNull );
    if ( idx < self -> start )
        return RC ( rcCont, rcVector, rcInserting, rcParam, rcInvalid );
    if ( ( idx -= self -> start ) < self -> len )
    {
        if ( self -> v [ idx ] != NULL )
            return RC ( rcCont, rcVector, rcInserting, rcItem, rcExists );
        self -> v [ idx ] = ( void* ) item;
        return 0;
    }

    curcap = ( self -> len + self -> mask ) & ~ self -> mask;
    newcap = ( idx + self -> mask + 1 ) & ~ self -> mask;
    if ( newcap > curcap )
    {
        void **v = realloc ( self -> v, newcap * sizeof * v );
        if ( v == NULL )
            return RC ( rcCont, rcVector, rcInserting, rcMemory, rcExhausted );
        self -> v = v;
    }

    if ( idx > self -> len )
        memset ( & self -> v [ self -> len ], 0, ( idx - self -> len ) * sizeof self -> v [ 0 ] );

    self -> v [ idx ] = ( void* ) item;
    self -> len = idx + 1;

    return 0;
}


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
LIB_EXPORT rc_t CC VectorSwap ( Vector *self, uint32_t idx,
    const void *item, void **prior )
{
    rc_t rc;

    if ( prior == NULL )
        rc = RC ( rcCont, rcVector, rcInserting, rcParam, rcNull );
    else
    {
        rc = VectorSet ( self, idx, item );
        if ( GetRCState ( rc ) == rcExists )
        {
            idx -= self -> start;
            * prior = self -> v [ idx ];

            /* if swapping NULL for the last item
               just drop the count */
            if ( item == NULL && ( idx + 1 ) == self -> len )
                -- self -> len;
            else
                self -> v [ idx ] = ( void* ) item;

            return 0;
        }

        * prior = NULL;
    }
    return rc;
}


/* Append
 *  appends item to end of vector
 *
 *  "idx" [ OUT, NULL OKAY ] - optional return parameter for item index
 *
 *  "item" [ IN, OPAQUE ] - item to be appended to vector
 */
LIB_EXPORT rc_t CC VectorAppend ( Vector *self, uint32_t *idx, const void *item )
{
    if ( self == NULL )
        return RC ( rcCont, rcVector, rcInserting, rcSelf, rcNull );

    if ( ( self -> len & self -> mask ) == 0 )
    {
        void **v = realloc ( self -> v, ( self -> len + self -> mask + 1 ) * sizeof * v );
        if ( v == NULL )
            return RC ( rcCont, rcVector, rcInserting, rcMemory, rcExhausted );
        self -> v = v;
    }

    self -> v [ self -> len ] = ( void* ) item;

    if ( idx != NULL )
        * idx = self -> len + self -> start;

    ++ self -> len;

    return 0;
}


/* First
 * Last
 *  get item at either end of vector
 */
LIB_EXPORT void* CC VectorFirst ( const Vector *self )
{
    if ( self != NULL && self -> len != 0 )
        return self -> v [ 0 ];
    return NULL;
}

LIB_EXPORT  void* CC VectorLast ( const Vector *self )
{
    if ( self != NULL && self -> len != 0 )
        return self -> v [ self -> len - 1 ];
    return NULL;
}


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
LIB_EXPORT void* CC VectorFind ( const Vector *self, const void *key, uint32_t *idx,
    int ( CC * cmp ) ( const void *key, const void *n ) )
{
    if ( self != NULL && cmp != NULL )
    {
        uint32_t left, right;

        for ( left = 0, right = self -> len; left < right; )
        {
            uint32_t i, mid = ( left + right ) >> 1;
            void *n = self -> v [ i = mid ];
            while ( n == NULL && i > left )
                n = self -> v [ -- i ];
            if ( n == NULL )
                left = mid + 1;
            else
            {
                int diff = ( * cmp ) ( key, n );
                if ( diff == 0 )
                {
                    if ( idx != NULL )
                        * idx = i + self -> start;
                    return n;
                }
                if ( diff < 0 )
                    right = mid;
                else
                    left = mid + 1;
            }
        }
    }

    return NULL;
}


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
LIB_EXPORT rc_t CC VectorInsert ( Vector *self, const void *item, uint32_t *idx,
    int ( CC * sort ) ( const void *item, const void *n ) )
{
    rc_t rc;

    if ( self == NULL )
        return RC ( rcCont, rcVector, rcInserting, rcSelf, rcNull );
    if ( sort == NULL )
        return RC ( rcCont, rcVector, rcInserting, rcFunction, rcNull );

    /* let append handle vector expansion */
    rc = VectorAppend ( self, NULL, item );
    if ( rc == 0 )
    {
        uint32_t left, right;

        for ( left = 0, right = ( int ) self -> len - 1; left < right; )
        {
            uint32_t i, mid = ( left + right ) >> 1;
            void *n = self -> v [ i = mid ];
            while ( n == NULL && i > left )
                n = self -> v [ -- i ];
            if ( n == NULL )
                left = mid + 1;
            else
            {
                int diff = ( * sort ) ( item, n );
                if ( diff <= 0 )
                    right = mid;
                else
                    left = mid + 1;
            }
        }

        if ( self -> len > ++ right )
        {
            memmove ( & self -> v [ right ], & self -> v [ left ],
                ( self -> len - right ) * sizeof self -> v [ 0 ] );
            self -> v [ left ] = ( void* ) item;
        }

        if ( idx != NULL )
            * idx = left + self -> start;
    }

    return rc;
}


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
LIB_EXPORT rc_t CC VectorInsertUnique ( Vector *self, const void *item, uint32_t *idx,
    int ( CC * sort ) ( const void *item, const void *n ) )
{
    rc_t rc;

    if ( self == NULL )
        return RC ( rcCont, rcVector, rcInserting, rcSelf, rcNull );
    if ( sort == NULL )
        return RC ( rcCont, rcVector, rcInserting, rcFunction, rcNull );

    rc = VectorAppend ( self, NULL, item );
    if ( rc == 0 )
    {
        uint32_t left, right;

        for ( left = 0, right = ( int ) self -> len - 1; left < right; )
        {
            uint32_t i, mid = ( left + right ) >> 1;
            void *n = self -> v [ i = mid ];
            while ( n == NULL && i > left )
                n = self -> v [ -- i ];
            if ( n == NULL )
                left = mid + 1;
            else
            {
                int diff = ( * sort ) ( item, n );
                if ( diff == 0 )
                {
                    -- self -> len;
                    if ( idx != NULL )
                        * idx = i + self -> start;
                    return RC ( rcCont, rcVector, rcInserting, rcNode, rcExists );
                }
                if ( diff < 0 )
                    right = mid;
                else
                    left = mid + 1;
            }
        }

        if ( self -> len > ++ right )
        {
            memmove ( & self -> v [ right ], & self -> v [ left ],
                ( self -> len - right ) * sizeof self -> v [ 0 ] );
            self -> v [ left ] = ( void* ) item;
        }

        if ( idx != NULL )
            * idx = left + self -> start;
    }

    return rc;
}


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
LIB_EXPORT rc_t CC VectorMerge ( Vector *self, bool unique, const Vector *v,
    int ( CC * sort ) ( const void *item, const void *n ) )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcCont, rcVector, rcInserting, rcSelf, rcNull );
    else if ( v == NULL )
        rc = RC ( rcCont, rcVector, rcInserting, rcParam, rcNull );
    else if ( sort == NULL )
        rc = RC ( rcCont, rcVector, rcInserting, rcFunction, rcNull );
    else
    {
        void **out = malloc ( ( self -> len + v -> len ) * sizeof out [ 0 ] );
        if ( out == NULL )
            rc = RC ( rcCont, rcVector, rcInserting, rcMemory, rcExhausted );
        else
        {
            uint32_t i, a, b;
            for ( rc = 0, i = a = b = 0; a < self -> len && b < v -> len; ++ i )
            {
                int diff = ( * sort ) ( self -> v [ a ], v -> v [ b ] );
                if ( diff == 0 )
                {
                    out [ i ] = self -> v [ a ++ ];
                    if ( ! unique )
                        out [ ++ i ] = v -> v [ b ];
                    ++ b;
                }
                else if ( diff < 0 )
                    out [ i ] = self -> v [ a ++ ];
                else
                    out [ i ] = v -> v [ b ++ ];
            }
            for ( ; a < self -> len; ++ i, ++ a )
                out [ i ] = self -> v [ a ];
            for ( ; b < v -> len; ++ i, ++ b )
                out [ i ] = v -> v [ b ];

            self -> len = i;
            free ( self -> v );
            self -> v = out;
        }
    }

    return rc;
}


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
LIB_EXPORT rc_t CC VectorRemove ( Vector *self, uint32_t idx, void **removed )
{
    rc_t rc;
    if ( removed == NULL )
        rc = RC ( rcCont, rcVector, rcRemoving, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcCont, rcVector, rcRemoving, rcSelf, rcNull );
        else if ( idx < self -> start )
            rc = RC ( rcCont, rcVector, rcRemoving, rcParam, rcInvalid );
        else if ( ( idx -= self -> start ) >= self -> len )
            rc = RC ( rcCont, rcVector, rcRemoving, rcParam, rcInvalid );
        else
        {
            * removed = self -> v [ idx ];
            if ( -- self -> len > idx )
            {
                memmove ( & self -> v [ idx ], & self -> v [ idx + 1 ],
                    ( self -> len - idx ) * sizeof self -> v [ 0 ] );
            }
            return 0;
        }

        * removed = NULL;
    }
    return rc;
}


/* Reorder
 *  execute qsort on vector with provided function
 *  not particularly well formed ( i.e. doesn't have a function param ),
 *  but works within a limited context
 */
LIB_EXPORT void CC VectorReorder ( Vector *self,
    int ( CC * f ) ( const void**, const void**, void* ), void *data )
{
    if ( self != NULL && f != NULL && self -> len != 0 )
    {
        assert ( self -> v != NULL );
        ksort ( self -> v, self -> len, sizeof self -> v [ 0 ],
            ( int ( CC * ) ( const void*, const void*, void* ) ) f, data );
    }
}


/* ForEach
 *  executes a function on each vector element
 */
LIB_EXPORT void CC VectorForEach ( const Vector *self, bool reverse,
    void ( CC * f ) ( void *item, void *data ), void *data )
{
    if ( self != NULL && f != NULL )
    {
        uint32_t i;

        assert ( self -> len == 0 || self -> v != NULL );

        if ( reverse ) for ( i = self -> len; i > 0; )
        {
            ( * f ) ( self -> v [ -- i ], data );
        }
        else for ( i = 0; i < self -> len; ++ i )
        {
            ( * f ) ( self -> v [ i ], data );
        }
    }
}


/* DoUntil
 *  executes a function on each element
 *  until the function returns true
 *
 *  return values:
 *    false unless the function returns true
 */
LIB_EXPORT bool CC VectorDoUntil ( const Vector *self, bool reverse,
    bool ( CC * f ) ( void *item, void *data ), void *data )
{
    if ( self != NULL && f != NULL )
    {
        uint32_t i;

        assert ( self -> len == 0 || self -> v != NULL );

        if ( reverse ) for ( i = self -> len; i > 0; )
        {
            if ( ( * f ) ( self -> v [ -- i ], data ) )
                return true;
        }
        else for ( i = 0; i < self -> len; ++ i )
        {
            if ( ( * f ) ( self -> v [ i ], data ) )
                return true;
        }
    }

    return false;
}


/* Whack
 *  removes entries from vector and
 *  executes a user provided destructor
 */
LIB_EXPORT void CC VectorWhack ( Vector *self, 
    void ( CC * whack ) ( void *item, void *data ), void *data )
{
    if ( self != NULL )
    {
        uint32_t i;

        assert ( self -> len == 0 || self -> v != NULL );

        if ( whack != NULL ) for ( i = 0; i < self -> len; ++ i )
            ( * whack ) ( self -> v [ i ], data );

        free ( self -> v );

        self -> v = NULL;
        self -> len = 0;
    }
}
