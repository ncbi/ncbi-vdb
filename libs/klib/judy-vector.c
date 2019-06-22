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
#include <klib/refcount.h>
#include <klib/rc.h>
#include <sysalloc.h>

#define JUST_DEFINE_YOUR_STUFF 1
#include "Judy.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>


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
struct KVector
{
    void *nancy;
    KRefcount refcount;
    uint32_t fixed_size;
    bool nancy_bool;
};


/* Make
 *  create an empty vector
 */
LIB_EXPORT rc_t CC KVectorMake ( KVector **vp )
{
    rc_t rc;

    if ( vp == NULL )
        rc = RC ( rcCont, rcVector, rcConstructing, rcParam, rcNull );
    else
    {
        KVector *v = calloc ( 1, sizeof * v );
        if ( v == NULL )
            rc = RC ( rcCont, rcVector, rcConstructing, rcMemory, rcExhausted );
        else
        {
            KRefcountInit ( & v -> refcount, 1, "KVector", "make", "v" );
            * vp = v;
            return 0;
        }

        * vp = NULL;
    }

    return rc;
}


/* Whack
 */
static
rc_t KVectorWhack ( KVector *self )
{
    if ( self -> nancy != NULL )
    {
        JError_t err;
        Word_t whee =
            ( self -> nancy_bool ? Judy1FreeArray : JudyLFreeArray )
            ( & self -> nancy, & err );
        ( void ) whee;
    }

    free ( self );
    return 0;
}


/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC KVectorAddRef ( const KVector *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KVector" ) )
        {
        case krefOkay:
            break;
        case krefLimit:
            return RC ( rcCont, rcVector, rcAttaching, rcRange, rcExcessive );
        default:
            return RC ( rcCont, rcVector, rcAttaching, rcRefcount, rcDestroyed );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC KVectorRelease ( const KVector *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KVector" ) )
        {
        case krefOkay:
            break;
        case krefWhack:
        case krefZero:
            return KVectorWhack ( ( KVector* ) self );
        default:
            return RC ( rcCont, rcVector, rcReleasing, rcRefcount, rcDestroyed );
        }
    }
    return 0;
}


/* NancyError
 *  translate a JError_t to an rc_t
 */
static
rc_t NancyError ( const JError_t *err, enum RCContext ctx )
{
    switch ( JU_ERRNO ( err ) )
    {
    case JU_ERRNO_NONE:
        break;
    case JU_ERRNO_FULL:
        return RC ( rcCont, rcVector, ctx, rcRange, rcExcessive );
    case JU_ERRNO_NOMEM:
        return RC ( rcCont, rcVector, ctx, rcMemory, rcExhausted );
    case JU_ERRNO_NULLPPARRAY:
    case JU_ERRNO_NULLPINDEX:
        return RC ( rcCont, rcVector, ctx, rcParam, rcNull );
    case JU_ERRNO_NOTJUDY1:
    case JU_ERRNO_NOTJUDYL:
    case JU_ERRNO_NOTJUDYSL:
        return RC ( rcCont, rcVector, ctx, rcType, rcIncorrect );
    case JU_ERRNO_OVERRUN:
    case JU_ERRNO_CORRUPT:
        return RC ( rcCont, rcVector, ctx, rcVector, rcCorrupt );
    case JU_ERRNO_NONNULLPARRAY:
    case JU_ERRNO_NULLPVALUE:
    case JU_ERRNO_UNSORTED:
        return RC ( rcCont, rcVector, ctx, rcParam, rcInvalid );
    }

    return 0;
}


/* Get
 *  get an untyped value
 *  returns rc_t state of rcNull if index is not set
 *
 *  "key" [ IN ] - vector index
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for value
 *
 *  "bytes" [ OUT ] - return parameter for bytes in value
 *
 * NB - if rc_t state is rcInsufficient, "bytes" will contain
 *  the number of bytes required to access the indexed value
 */
static
rc_t Nancy1Test ( const void *nancy, uint64_t idx, bool *value )
{
    JError_t err;
    int data = Judy1Test ( nancy, ( Word_t ) idx, & err );
    if ( data == JERR )
        return NancyError ( & err, rcAccessing );
    * value = data != 0;
    return 0;
}

static
rc_t NancyLGet ( const void *nancy, uint64_t idx, Word_t *value )
{
    JError_t err;
    PPvoid_t datap = JudyLGet ( nancy, ( Word_t ) idx, & err );
    if ( datap == NULL )
        return RC ( rcCont, rcVector, rcAccessing, rcItem, rcNotFound );
    if ( datap == PPJERR )
        return NancyError ( & err, rcAccessing );

    * value = * ( const Word_t* ) datap;
    return 0;
}

LIB_EXPORT rc_t CC KVectorGet ( const KVector *self, uint64_t key,
    void *buffer, size_t bsize, size_t *bytes )
{
    rc_t rc;

    if ( bytes == NULL )
        rc = RC ( rcCont, rcVector, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcCont, rcVector, rcAccessing, rcSelf, rcNull );
        else if ( buffer == NULL && bsize != 0 )
            rc = RC ( rcCont, rcVector, rcAccessing, rcParam, rcNull );
        else if ( sizeof key > sizeof ( Word_t ) && ( key >> 32 ) != 0 )
            rc = RC ( rcCont, rcVector, rcAccessing, rcRange, rcExcessive );
        else
        {
            if ( self -> nancy_bool )
            {
                bool data;
                rc = Nancy1Test ( self -> nancy, key, & data );
                if ( rc == 0 )
                {
                    * bytes = sizeof data;
                    if ( bsize < sizeof data )
                        rc = RC ( rcCont, rcVector, rcAccessing, rcBuffer, rcInsufficient );
                    else
                        * ( bool* ) buffer = data;
                    return rc;
                }
            }
            else
            {
                Word_t data;
                rc = NancyLGet ( self -> nancy, key, & data );
                if ( rc == 0 )
                {
                    if ( self -> fixed_size == 0 )
                        rc = RC ( rcCont, rcVector, rcAccessing, rcFunction, rcUnsupported );
                    else
                    {
                        * bytes = self -> fixed_size;
                        if ( bsize < self -> fixed_size )
                            rc = RC ( rcCont, rcVector, rcAccessing, rcBuffer, rcInsufficient );
                        else switch ( self -> fixed_size )
                        {
                        case 1:
                            if ( data >= 0x100 )
                                rc = RC ( rcCont, rcVector, rcAccessing, rcData, rcCorrupt );
                            * ( uint8_t* ) buffer = ( uint8_t ) data;
                            break;
                        case 2:
                            if ( data >= 0x10000 )
                                rc = RC ( rcCont, rcVector, rcAccessing, rcData, rcCorrupt );
                            * ( uint16_t* ) buffer = ( uint16_t ) data;
                            break;
                        case 4:
                            if ( sizeof data > 4 && ( ( uint64_t ) data >> 32 ) != 0 )
                                rc = RC ( rcCont, rcVector, rcAccessing, rcData, rcCorrupt );
                            * ( uint32_t* ) buffer = ( uint32_t ) data;
                            break;
                        case 8:
                            * ( uint64_t* ) buffer = ( uint64_t ) data;
                            break;
                        default:
                            rc = RC ( rcCont, rcVector, rcAccessing, rcType, rcUnsupported );
                        }
                        return rc;
                    }
                }
            }
        }

        * bytes = 0;
    }

    return rc;
}

/* Get
 *  get typed values
 *  returns rc_t state of rcNull if index is not set
 *
 *  "key" [ IN ] - vector index
 *
 *  "data" [ OUT ] - return parameter for value
 */

#if _ARCH_BITS == 64

typedef uint64_t bstored_bits_t;

#define KVectorBoolGetStoredBits KVectorGetU64
#define KVectorBoolSetStoredBits KVectorSetU64
#define KVectorBoolVisitStoredBits KVectorVisitU64
#define KVectorBoolGetFirstStoredBits KVectorGetFirstU64
#define KVectorBoolGetPrevStoredBits KVectorGetPrevU64
#define KVectorBoolGetNextStoredBits KVectorGetNextU64

#elif _ARCH_BITS == 32

typedef uint32_t bstored_bits_t;

#define KVectorBoolGetStoredBits KVectorGetU32
#define KVectorBoolSetStoredBits KVectorSetU32
#define KVectorBoolVisitStoredBits KVectorVisitU32
#define KVectorBoolGetFirstStoredBits KVectorGetFirstU32
#define KVectorBoolGetPrevStoredBits KVectorGetPrevU32
#define KVectorBoolGetNextStoredBits KVectorGetNextU32

#endif


#define BOOL_VECT_RECORD_SIZE_IN_BITS (size_t)2
#define BOOL_VECT_BIT_SET_MASK        (size_t)0x2
#define BOOL_VECT_BIT_VALUE_MASK      (size_t)0x1
#define BOOL_VECT_BIT_RECORD_MASK     (size_t)(BOOL_VECT_BIT_SET_MASK | BOOL_VECT_BIT_VALUE_MASK)


LIB_EXPORT rc_t CC KVectorGetBoolOld ( const KVector *self, uint64_t key, bool *data )
{
    size_t bytes;
    return KVectorGet ( self, key, data, sizeof * data, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetBool ( const KVector *self, uint64_t key, bool *data )
{
    rc_t rc = 0;
    bstored_bits_t stored_bits;
    uint64_t key_qword = key / (sizeof(stored_bits) * 8 / BOOL_VECT_RECORD_SIZE_IN_BITS);
    size_t bit_offset_in_qword = (key % (sizeof(stored_bits) * 8 / BOOL_VECT_RECORD_SIZE_IN_BITS)) * BOOL_VECT_RECORD_SIZE_IN_BITS;

    size_t record;

    if ( data == NULL )
        return RC ( rcCont, rcVector, rcAccessing, rcParam, rcNull );

    rc = KVectorBoolGetStoredBits ( self, key_qword, & stored_bits );
    if ( rc )
        return rc;

    record = stored_bits >> bit_offset_in_qword;
    if ( ( record & BOOL_VECT_BIT_SET_MASK ) == 0 )
    {
        * data = false;
        return RC ( rcCont, rcVector, rcAccessing, rcItem, rcNotFound );
    }
    * data = (bool) (record & BOOL_VECT_BIT_VALUE_MASK);
    return 0;
}

LIB_EXPORT rc_t CC KVectorGetI8 ( const KVector *self, uint64_t key, int8_t *data )
{
    size_t bytes;
    return KVectorGet ( self, key, data, sizeof * data, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetI16 ( const KVector *self, uint64_t key, int16_t *data )
{
    size_t bytes;
    return KVectorGet ( self, key, data, sizeof * data, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetI32 ( const KVector *self, uint64_t key, int32_t *data )
{
    size_t bytes;
    return KVectorGet ( self, key, data, sizeof * data, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetI64 ( const KVector *self, uint64_t key, int64_t *data )
{
    size_t bytes;
    return KVectorGet ( self, key, data, sizeof * data, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetU8 ( const KVector *self, uint64_t key, uint8_t *data )
{
    size_t bytes;
    return KVectorGet ( self, key, data, sizeof * data, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetU16 ( const KVector *self, uint64_t key, uint16_t *data )
{
    size_t bytes;
    return KVectorGet ( self, key, data, sizeof * data, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetU32 ( const KVector *self, uint64_t key, uint32_t *data )
{
    size_t bytes;
    return KVectorGet ( self, key, data, sizeof * data, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetU64 ( const KVector *self, uint64_t key, uint64_t *data )
{
    size_t bytes;
    return KVectorGet ( self, key, data, sizeof * data, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetF32 ( const KVector *self, uint64_t key, float *data )
{
    size_t bytes;
    rc_t rc = KVectorGet ( self, key, data, sizeof * data, & bytes );
    if ( rc == 0 && bytes != sizeof * data )
        rc = RC ( rcCont, rcVector, rcAccessing, rcType, rcUnsupported );
    return rc;
}

LIB_EXPORT rc_t CC KVectorGetF64 ( const KVector *self, uint64_t key, double *data )
{
    size_t bytes;
    rc_t rc = KVectorGet ( self, key, data, sizeof * data, & bytes );
    if ( rc == 0 && bytes != sizeof * data )
        rc = RC ( rcCont, rcVector, rcAccessing, rcType, rcUnsupported );
    return rc;
}

LIB_EXPORT rc_t CC KVectorGetPtr ( const KVector *self, uint64_t key, void **data )
{
    size_t bytes;
    rc_t rc = KVectorGet ( self, key, data, sizeof * data, & bytes );
    if ( rc == 0 && bytes != sizeof * data )
        rc = RC ( rcCont, rcVector, rcAccessing, rcType, rcUnsupported );
    return rc;
}


/* GetFirst
 *  get first non-null element
 *  returns key of found element in "first"
 */

static
rc_t NancyLGetFirst ( const void *nancy, uint64_t * first, Word_t * value )
{
    JError_t err;
    Word_t idx = 0;
    PPvoid_t datap = JudyLFirst ( nancy, & idx, & err );
    if ( datap == NULL )
        return RC ( rcCont, rcVector, rcAccessing, rcItem, rcNotFound );
    if ( datap == PPJERR )
        return NancyError ( & err, rcAccessing );

    * first = idx;
    * value = * ( const Word_t* ) datap;

    return 0;
}

LIB_EXPORT rc_t CC KVectorGetFirst ( const KVector *self, uint64_t *first,
    void * buffer, size_t bsize, size_t * bytes )
{
    rc_t rc;

    if ( bytes == NULL )
        rc = RC ( rcCont, rcVector, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcCont, rcVector, rcAccessing, rcSelf, rcNull );
        else if ( buffer == NULL && bsize != 0 )
            rc = RC ( rcCont, rcVector, rcAccessing, rcParam, rcNull );
        else if ( self -> nancy_bool )
            rc = RC ( rcCont, rcVector, rcAccessing, rcType, rcUnsupported );
        else
        {
            Word_t data;
            rc = NancyLGetFirst ( self -> nancy, first, & data );
            if ( rc == 0 )
            {
                if ( self -> fixed_size == 0 )
                    rc = RC ( rcCont, rcVector, rcAccessing, rcFunction, rcUnsupported );
                else
                {
                    * bytes = self -> fixed_size;
                    if ( bsize < self -> fixed_size )
                        rc = RC ( rcCont, rcVector, rcAccessing, rcBuffer, rcInsufficient );
                    else switch ( self -> fixed_size )
                    {
                    case 1:
                        if ( data >= 0x100 )
                            rc = RC ( rcCont, rcVector, rcAccessing, rcData, rcCorrupt );
                        * ( uint8_t* ) buffer = ( uint8_t ) data;
                        break;
                    case 2:
                        if ( data >= 0x10000 )
                            rc = RC ( rcCont, rcVector, rcAccessing, rcData, rcCorrupt );
                        * ( uint16_t* ) buffer = ( uint16_t ) data;
                        break;
                    case 4:
                        if ( sizeof data > 4 && ( ( uint64_t ) data >> 32 ) != 0 )
                            rc = RC ( rcCont, rcVector, rcAccessing, rcData, rcCorrupt );
                        * ( uint32_t* ) buffer = ( uint32_t ) data;
                        break;
                    case 8:
                        * ( uint64_t* ) buffer = ( uint64_t ) data;
                        break;
                    default:
                        rc = RC ( rcCont, rcVector, rcAccessing, rcType, rcUnsupported );
                    }
                    return rc;
                }
            }
        }

        * bytes = 0;
    }

    return rc;
}

LIB_EXPORT rc_t CC KVectorGetFirstBool ( const KVector *self, uint64_t *first, bool *data )
{
    rc_t rc;
    uint64_t i;
    bstored_bits_t stored_bits;
    const uint64_t width = sizeof stored_bits * 8 / BOOL_VECT_RECORD_SIZE_IN_BITS;

    if ( data == NULL )
        return RC ( rcCont, rcVector, rcAccessing, rcParam, rcNull );

    rc = KVectorBoolGetFirstStoredBits ( self, first, & stored_bits );
    while ( rc == 0 && stored_bits == 0 )
        rc = KVectorBoolGetNextStoredBits ( self, first, * first, & stored_bits );
    if ( rc != 0 )
        return rc;

    * first *= width;

    for ( i = 0; i < width; ++ i )
    {
        if ( ( stored_bits & ( ( size_t ) BOOL_VECT_BIT_SET_MASK << i * BOOL_VECT_RECORD_SIZE_IN_BITS ) ) != 0 )
            break;
    }

    * first += i;
    * data = ( bool ) ( ( stored_bits >> i * BOOL_VECT_RECORD_SIZE_IN_BITS ) & BOOL_VECT_BIT_VALUE_MASK );

    return 0;
}

LIB_EXPORT rc_t CC KVectorGetFirstI8 ( const KVector *self, uint64_t *first, int8_t *value )
{
    size_t bytes;
    return KVectorGetFirst ( self, first, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetFirstI16 ( const KVector *self, uint64_t *first, int16_t *value )
{
    size_t bytes;
    return KVectorGetFirst ( self, first, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetFirstI32 ( const KVector *self, uint64_t *first, int32_t *value )
{
    size_t bytes;
    return KVectorGetFirst ( self, first, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetFirstI64 ( const KVector *self, uint64_t *first, int64_t *value )
{
    size_t bytes;
    return KVectorGetFirst ( self, first, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetFirstU8 ( const KVector *self, uint64_t *first, uint8_t *value )
{
    size_t bytes;
    return KVectorGetFirst ( self, first, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetFirstU16 ( const KVector *self, uint64_t *first, uint16_t *value )
{
    size_t bytes;
    return KVectorGetFirst ( self, first, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetFirstU32 ( const KVector *self, uint64_t *first, uint32_t *value )
{
    size_t bytes;
    return KVectorGetFirst ( self, first, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetFirstU64 ( const KVector *self, uint64_t *first, uint64_t *value )
{
    size_t bytes;
    return KVectorGetFirst ( self, first, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetFirstF32 ( const KVector *self, uint64_t *first, float *value )
{
    size_t bytes;
    rc_t rc = KVectorGetFirst ( self, first, value, sizeof * value, & bytes );
    if ( rc == 0 && bytes != sizeof * value )
        rc = RC ( rcCont, rcVector, rcAccessing, rcType, rcUnsupported );
    return rc;
}

LIB_EXPORT rc_t CC KVectorGetFirstF64 ( const KVector *self, uint64_t *first, double *value )
{
    size_t bytes;
    rc_t rc = KVectorGetFirst ( self, first, value, sizeof * value, & bytes );
    if ( rc == 0 && bytes != sizeof * value )
        rc = RC ( rcCont, rcVector, rcAccessing, rcType, rcUnsupported );
    return rc;
}

LIB_EXPORT rc_t CC KVectorGetFirstPtr ( const KVector *self, uint64_t *first, void **value )
{
    size_t bytes;
    rc_t rc = KVectorGetFirst ( self, first, value, sizeof * value, & bytes );
    if ( rc == 0 && bytes != sizeof * value )
        rc = RC ( rcCont, rcVector, rcAccessing, rcType, rcUnsupported );
    return rc;
}

/* GetPrev
 *  given a starting key, get first previous non-null element
 *  returns key of found element in "prev"
 *
 *  "prev" [ OUT ] - pointer to vector index of the returned value
 *
 *  "key" [ IN ] - vector index
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for value
 *
 *  "bytes" [ OUT ] - return parameter for bytes in value
 *
 * NB - if rc_t state is rcInsufficient, "bytes" will contain
 *  the number of bytes required to access the indexed value
*/

static
rc_t Nancy1TestPrev ( const void *nancy, uint64_t* prev, uint64_t idx, bool *value )
{
    JError_t err;
    int data;
    *prev = idx;
    data = Judy1Prev ( nancy, ( Word_t* ) prev, & err );
    if ( data == JERR )
        return NancyError ( & err, rcAccessing );
    * value = data != 0;
    return 0;
}

static
rc_t NancyLGetPrev ( const void *nancy, uint64_t* prev, uint64_t idx, Word_t *value )
{
    JError_t err;
    PPvoid_t datap;
    *prev = idx;
    datap = JudyLPrev ( nancy, ( Word_t* ) prev, & err );
    if ( datap == NULL )
        return RC ( rcCont, rcVector, rcAccessing, rcItem, rcNotFound );
    if ( datap == PPJERR )
        return NancyError ( & err, rcAccessing );

    * value = * ( const Word_t* ) datap;
    return 0;
}

KLIB_EXTERN rc_t CC KVectorGetPrev ( const KVector *self, uint64_t *prev,
    uint64_t key, void *value_buffer, size_t bsize, size_t *bytes )
{
    rc_t rc;

    if ( bytes == NULL )
        rc = RC ( rcCont, rcVector, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcCont, rcVector, rcAccessing, rcSelf, rcNull );
        else if ( ( value_buffer == NULL && bsize != 0 ) || prev == NULL )
            rc = RC ( rcCont, rcVector, rcAccessing, rcParam, rcNull );
        else if ( sizeof key > sizeof ( Word_t ) && ( key >> 32 ) != 0 )
            rc = RC ( rcCont, rcVector, rcAccessing, rcRange, rcExcessive );
        else
        {
            if ( self -> nancy_bool )
            {
                bool data;
                rc = Nancy1TestPrev ( self -> nancy, prev, key, & data );
                if ( rc == 0 )
                {
                    * bytes = sizeof data;
                    if ( bsize < sizeof data )
                        rc = RC ( rcCont, rcVector, rcAccessing, rcBuffer, rcInsufficient );
                    else
                        * ( bool* ) value_buffer = data;
                    return rc;
                }
            }
            else
            {
                Word_t data;
                rc = NancyLGetPrev ( self -> nancy, prev, key, & data );
                if ( rc == 0 )
                {
                    if ( self -> fixed_size == 0 )
                        rc = RC ( rcCont, rcVector, rcAccessing, rcFunction, rcUnsupported );
                    else
                    {
                        * bytes = self -> fixed_size;
                        if ( bsize < self -> fixed_size )
                            rc = RC ( rcCont, rcVector, rcAccessing, rcBuffer, rcInsufficient );
                        else switch ( self -> fixed_size )
                        {
                        case 1:
                            if ( data >= 0x100 )
                                rc = RC ( rcCont, rcVector, rcAccessing, rcData, rcCorrupt );
                            * ( uint8_t* ) value_buffer = ( uint8_t ) data;
                            break;
                        case 2:
                            if ( data >= 0x10000 )
                                rc = RC ( rcCont, rcVector, rcAccessing, rcData, rcCorrupt );
                            * ( uint16_t* ) value_buffer = ( uint16_t ) data;
                            break;
                        case 4:
                            if ( sizeof data > 4 && ( ( uint64_t ) data >> 32 ) != 0 )
                                rc = RC ( rcCont, rcVector, rcAccessing, rcData, rcCorrupt );
                            * ( uint32_t* ) value_buffer = ( uint32_t ) data;
                            break;
                        case 8:
                            * ( uint64_t* ) value_buffer = ( uint64_t ) data;
                            break;
                        default:
                            rc = RC ( rcCont, rcVector, rcAccessing, rcType, rcUnsupported );
                        }
                        return rc;
                    }
                }
            }
        }

        * bytes = 0;
    }

    return rc;
}

/* GetPrev
 *  get prev typed values
 *  returns rc_t state of rcNull if index is not set
 *
 *  "prev" [ OUT ] - pointer to vector index of the returned value
 *
 *  "key" [ IN ] - vector index
 *
 *  "data" [ OUT ] - return parameter for value
 */

LIB_EXPORT rc_t CC KVectorGetPrevBoolOld ( const KVector *self,
    uint64_t *prev, uint64_t key, bool *value )
{
    size_t bytes;
    return KVectorGetPrev ( self, prev, key, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetPrevBool ( const KVector *self,
    uint64_t *prev, uint64_t key, bool *value )
{
    rc_t rc = 0;
    bstored_bits_t stored_bits = 0;
    uint64_t key_qword = key / (sizeof(stored_bits) * 8 / BOOL_VECT_RECORD_SIZE_IN_BITS);
    size_t bit_offset_in_qword = (key % (sizeof(stored_bits) * 8 / BOOL_VECT_RECORD_SIZE_IN_BITS)) * BOOL_VECT_RECORD_SIZE_IN_BITS;

    if ( bit_offset_in_qword )
    {
        rc = KVectorBoolGetStoredBits ( self, key_qword, & stored_bits );
        if ( rc && rc != RC ( rcCont, rcVector, rcAccessing, rcItem, rcNotFound ))
            return rc;
    }

    for (;;)
    {
        size_t record;

        for (; bit_offset_in_qword; )
        {
            bit_offset_in_qword -= BOOL_VECT_RECORD_SIZE_IN_BITS;

            record = stored_bits >> bit_offset_in_qword & BOOL_VECT_BIT_RECORD_MASK;
            if ( record & BOOL_VECT_BIT_SET_MASK )
            {
                *value = (bool) (record & BOOL_VECT_BIT_VALUE_MASK);
                *prev = key_qword * (sizeof(stored_bits) * 8 / BOOL_VECT_RECORD_SIZE_IN_BITS) |
                        (uint64_t)(bit_offset_in_qword / BOOL_VECT_RECORD_SIZE_IN_BITS);
                goto EXIT;
            }
        }

        rc = KVectorBoolGetPrevStoredBits ( self, & key_qword, key_qword, & stored_bits );
        if (rc)
            break;
        bit_offset_in_qword = sizeof(stored_bits) * 8;
    }

    EXIT:
    return rc;
}

LIB_EXPORT rc_t CC KVectorGetPrevI8 ( const KVector *self,
    uint64_t *prev, uint64_t key, int8_t *value )
{
    size_t bytes;
    return KVectorGetPrev ( self, prev, key, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetPrevI16 ( const KVector *self,
    uint64_t *prev, uint64_t key, int16_t *value )
{
    size_t bytes;
    return KVectorGetPrev ( self, prev, key, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetPrevI32 ( const KVector *self,
    uint64_t *prev, uint64_t key, int32_t *value )
{
    size_t bytes;
    return KVectorGetPrev ( self, prev, key, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetPrevI64 ( const KVector *self,
    uint64_t *prev, uint64_t key, int64_t *value )
{
    size_t bytes;
    return KVectorGetPrev ( self, prev, key, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetPrevU8 ( const KVector *self,
    uint64_t *prev, uint64_t key, uint8_t *value )
{
    size_t bytes;
    return KVectorGetPrev ( self, prev, key, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetPrevU16 ( const KVector *self,
    uint64_t *prev, uint64_t key, uint16_t *value )
{
    size_t bytes;
    return KVectorGetPrev ( self, prev, key, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetPrevU32 ( const KVector *self,
    uint64_t *prev, uint64_t key, uint32_t *value )
{
    size_t bytes;
    return KVectorGetPrev ( self, prev, key, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetPrevU64 ( const KVector *self,
    uint64_t *prev, uint64_t key, uint64_t *value )
{
    size_t bytes;
    return KVectorGetPrev ( self, prev, key, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetPrevF32 ( const KVector *self,
    uint64_t *prev, uint64_t key, float *value )
{
    size_t bytes;
    rc_t rc = KVectorGetPrev ( self, prev, key, value, sizeof * value, & bytes );
    if ( rc == 0 && bytes != sizeof * value )
        rc = RC ( rcCont, rcVector, rcAccessing, rcType, rcUnsupported );
    return rc;
}

LIB_EXPORT rc_t CC KVectorGetPrevF64 ( const KVector *self,
    uint64_t *prev, uint64_t key, double *value )
{
    size_t bytes;
    rc_t rc = KVectorGetPrev ( self, prev, key, value, sizeof * value, & bytes );
    if ( rc == 0 && bytes != sizeof * value )
        rc = RC ( rcCont, rcVector, rcAccessing, rcType, rcUnsupported );
    return rc;
}

LIB_EXPORT rc_t CC KVectorGetPrevPtr ( const KVector *self,
    uint64_t *prev, uint64_t key, void **value )
{
    size_t bytes;
    rc_t rc = KVectorGetPrev ( self, prev, key, value, sizeof * value, & bytes );
    if ( rc == 0 && bytes != sizeof * value )
        rc = RC ( rcCont, rcVector, rcAccessing, rcType, rcUnsupported );
    return rc;
}

/* GetNext
 *  given a starting key, get first following non-null element
 *  returns key of found element in "next"
 *
 *  "next" [ OUT ] - pointer to vector index of the returned value
 *
 *  "key" [ IN ] - vector index
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for value
 *
 *  "bytes" [ OUT ] - return parameter for bytes in value
 *
 * NB - if rc_t state is rcInsufficient, "bytes" will contain
 *  the number of bytes required to access the indexed value
*/

static
rc_t Nancy1TestNext ( const void *nancy, uint64_t* next, uint64_t idx, bool *value )
{
    JError_t err;
    int data;
    *next = idx;
    data = Judy1Next ( nancy, ( Word_t* ) next, & err );
    if ( data == JERR )
        return NancyError ( & err, rcAccessing );
    * value = data != 0;
    return 0;
}

static
rc_t NancyLGetNext ( const void *nancy, uint64_t* next, uint64_t idx, Word_t *value )
{
    JError_t err;
    PPvoid_t datap;
    *next = idx;
    datap = JudyLNext ( nancy, ( Word_t* ) next, & err );
    if ( datap == NULL )
        return RC ( rcCont, rcVector, rcAccessing, rcItem, rcNotFound );
    if ( datap == PPJERR )
        return NancyError ( & err, rcAccessing );

    * value = * ( const Word_t* ) datap;
    return 0;
}

KLIB_EXTERN rc_t CC KVectorGetNext ( const KVector *self, uint64_t *next,
    uint64_t key, void *value_buffer, size_t bsize, size_t *bytes )
{
    rc_t rc;

    if ( bytes == NULL )
        rc = RC ( rcCont, rcVector, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcCont, rcVector, rcAccessing, rcSelf, rcNull );
        else if ( ( value_buffer == NULL && bsize != 0 ) || next == NULL )
            rc = RC ( rcCont, rcVector, rcAccessing, rcParam, rcNull );
        else if ( sizeof key > sizeof ( Word_t ) && ( key >> 32 ) != 0 )
            rc = RC ( rcCont, rcVector, rcAccessing, rcRange, rcExcessive );
        else
        {
            if ( self -> nancy_bool )
            {
                bool data;
                rc = Nancy1TestNext ( self -> nancy, next, key, & data );
                if ( rc == 0 )
                {
                    * bytes = sizeof data;
                    if ( bsize < sizeof data )
                        rc = RC ( rcCont, rcVector, rcAccessing, rcBuffer, rcInsufficient );
                    else
                        * ( bool* ) value_buffer = data;
                    return rc;
                }
            }
            else
            {
                Word_t data;
                rc = NancyLGetNext ( self -> nancy, next, key, & data );
                if ( rc == 0 )
                {
                    if ( self -> fixed_size == 0 )
                        rc = RC ( rcCont, rcVector, rcAccessing, rcFunction, rcUnsupported );
                    else
                    {
                        * bytes = self -> fixed_size;
                        if ( bsize < self -> fixed_size )
                            rc = RC ( rcCont, rcVector, rcAccessing, rcBuffer, rcInsufficient );
                        else switch ( self -> fixed_size )
                        {
                        case 1:
                            if ( data >= 0x100 )
                                rc = RC ( rcCont, rcVector, rcAccessing, rcData, rcCorrupt );
                            * ( uint8_t* ) value_buffer = ( uint8_t ) data;
                            break;
                        case 2:
                            if ( data >= 0x10000 )
                                rc = RC ( rcCont, rcVector, rcAccessing, rcData, rcCorrupt );
                            * ( uint16_t* ) value_buffer = ( uint16_t ) data;
                            break;
                        case 4:
                            if ( sizeof data > 4 && ( ( uint64_t ) data >> 32 ) != 0 )
                                rc = RC ( rcCont, rcVector, rcAccessing, rcData, rcCorrupt );
                            * ( uint32_t* ) value_buffer = ( uint32_t ) data;
                            break;
                        case 8:
                            * ( uint64_t* ) value_buffer = ( uint64_t ) data;
                            break;
                        default:
                            rc = RC ( rcCont, rcVector, rcAccessing, rcType, rcUnsupported );
                        }
                        return rc;
                    }
                }
            }
        }

        * bytes = 0;
    }

    return rc;
}

/* GetNext
 *  get next typed values
 *  returns rc_t state of rcNull if index is not set
 *
 *  "next" [ OUT ] - pointer to vector index of the returned value
 *
 *  "key" [ IN ] - vector index
 *
 *  "data" [ OUT ] - return parameter for value
 */

LIB_EXPORT rc_t CC KVectorGetNextBoolOld ( const KVector *self,
    uint64_t *next, uint64_t key, bool *value )
{
    size_t bytes;
    return KVectorGetNext ( self, next, key, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetNextBool ( const KVector *self,
    uint64_t *next, uint64_t key, bool *value )
{
    rc_t rc = 0;
    bstored_bits_t stored_bits = 0;
    uint64_t key_qword = key / (sizeof(stored_bits) * 8 / BOOL_VECT_RECORD_SIZE_IN_BITS);
    size_t bit_offset_in_qword = (key % (sizeof(stored_bits) * 8 / BOOL_VECT_RECORD_SIZE_IN_BITS)) * BOOL_VECT_RECORD_SIZE_IN_BITS;

    size_t const MAX_BIT_OFFSET = sizeof(stored_bits) * 8 - BOOL_VECT_RECORD_SIZE_IN_BITS;

    /* unless the "previous" value is the last entry in a word,
       try to get the word and look for its next value */
    if ( bit_offset_in_qword != MAX_BIT_OFFSET )
    {
        /* if the initial value does not exist, this may fail */
        rc = KVectorBoolGetStoredBits ( self, key_qword, & stored_bits );
        if ( rc != 0 )
        {
            /* any failure other than "not found" is considered hard */
            if ( GetRCState ( rc ) != rcNotFound )
                return rc;

            /* the "key" passed in may be fake. */
            rc = KVectorBoolGetNextStoredBits ( self, & key_qword, key_qword, & stored_bits );
        }
    }

    for (;;)
    {
        size_t record;

        for (; bit_offset_in_qword != MAX_BIT_OFFSET; )
        {
            bit_offset_in_qword += BOOL_VECT_RECORD_SIZE_IN_BITS;

            record = stored_bits >> bit_offset_in_qword & BOOL_VECT_BIT_RECORD_MASK;
            if ( record & BOOL_VECT_BIT_SET_MASK )
            {
                *value = (bool) (record & BOOL_VECT_BIT_VALUE_MASK);
                *next = key_qword * (sizeof(stored_bits) * 8 / BOOL_VECT_RECORD_SIZE_IN_BITS) |
                        (uint64_t)(bit_offset_in_qword / BOOL_VECT_RECORD_SIZE_IN_BITS);
                goto EXIT;
            }
        }

        rc = KVectorBoolGetNextStoredBits ( self, & key_qword, key_qword, & stored_bits );
        if (rc)
            break;
        bit_offset_in_qword = 0 - BOOL_VECT_RECORD_SIZE_IN_BITS;
    }

    EXIT:
    return rc;
}

LIB_EXPORT rc_t CC KVectorGetNextI8 ( const KVector *self,
    uint64_t *next, uint64_t key, int8_t *value )
{
    size_t bytes;
    return KVectorGetNext ( self, next, key, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetNextI16 ( const KVector *self,
    uint64_t *next, uint64_t key, int16_t *value )
{
    size_t bytes;
    return KVectorGetNext ( self, next, key, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetNextI32 ( const KVector *self,
    uint64_t *next, uint64_t key, int32_t *value )
{
    size_t bytes;
    return KVectorGetNext ( self, next, key, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetNextI64 ( const KVector *self,
    uint64_t *next, uint64_t key, int64_t *value )
{
    size_t bytes;
    return KVectorGetNext ( self, next, key, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetNextU8 ( const KVector *self,
    uint64_t *next, uint64_t key, uint8_t *value )
{
    size_t bytes;
    return KVectorGetNext ( self, next, key, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetNextU16 ( const KVector *self,
    uint64_t *next, uint64_t key, uint16_t *value )
{
    size_t bytes;
    return KVectorGetNext ( self, next, key, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetNextU32 ( const KVector *self,
    uint64_t *next, uint64_t key, uint32_t *value )
{
    size_t bytes;
    return KVectorGetNext ( self, next, key, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetNextU64 ( const KVector *self,
    uint64_t *next, uint64_t key, uint64_t *value )
{
    size_t bytes;
    return KVectorGetNext ( self, next, key, value, sizeof * value, & bytes );
}

LIB_EXPORT rc_t CC KVectorGetNextF32 ( const KVector *self,
    uint64_t *next, uint64_t key, float *value )
{
    size_t bytes;
    rc_t rc = KVectorGetNext ( self, next, key, value, sizeof * value, & bytes );
    if ( rc == 0 && bytes != sizeof * value )
        rc = RC ( rcCont, rcVector, rcAccessing, rcType, rcUnsupported );
    return rc;
}

LIB_EXPORT rc_t CC KVectorGetNextF64 ( const KVector *self,
    uint64_t *next, uint64_t key, double *value )
{
    size_t bytes;
    rc_t rc = KVectorGetNext ( self, next, key, value, sizeof * value, & bytes );
    if ( rc == 0 && bytes != sizeof * value )
        rc = RC ( rcCont, rcVector, rcAccessing, rcType, rcUnsupported );
    return rc;
}

LIB_EXPORT rc_t CC KVectorGetNextPtr ( const KVector *self,
    uint64_t *next, uint64_t key, void **value )
{
    size_t bytes;
    rc_t rc = KVectorGetNext ( self, next, key, value, sizeof * value, & bytes );
    if ( rc == 0 && bytes != sizeof * value )
        rc = RC ( rcCont, rcVector, rcAccessing, rcType, rcUnsupported );
    return rc;
}

/* Set
 *  set an untyped value
 *
 *  "key" [ IN ] - vector index
 *
 *  "data" [ IN ] and "bytes" [ IN ] - value buffer
 */
static
rc_t Nancy1Set ( void **nancy, uint64_t idx, bool value )
{
    JError_t err;
    int status = ( value ? Judy1Set : Judy1Unset ) ( nancy, ( Word_t ) idx, & err );
    if ( status == JERR )
        return NancyError ( & err, rcWriting );
    return 0;
}

static
rc_t NancyLSet ( void **nancy, uint64_t idx, Word_t value )
{
    JError_t err;
    PPvoid_t datap = JudyLIns ( nancy, ( Word_t ) idx, & err );
    if ( datap == NULL )
        return RC ( rcCont, rcVector, rcWriting, rcMemory, rcExhausted );
    if ( datap == PPJERR )
        return NancyError ( & err, rcWriting );
    * ( Word_t* ) datap = value;
    return 0;
}

LIB_EXPORT rc_t CC KVectorSet ( KVector *self, uint64_t key,
    const void *data, size_t bytes )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcCont, rcVector, rcWriting, rcSelf, rcNull );
    else if ( data == NULL )
        rc = RC ( rcCont, rcVector, rcWriting, rcParam, rcNull );
    else if ( bytes == 0 )
        rc = RC ( rcCont, rcVector, rcWriting, rcParam, rcEmpty );
    else
    {
        Word_t value;

        /* detect first set */
        if ( self -> nancy == NULL )
        {
            switch ( bytes )
            {
            case 1:
            case 2:
            case 4:
                break;
            case 8:
                if ( sizeof value < 8 )
                    return RC ( rcCont, rcVector, rcWriting, rcParam, rcExcessive );
            }

            self -> fixed_size = ( uint32_t ) bytes;
        }
        else if ( bytes != ( size_t ) self -> fixed_size )
        {
            return RC ( rcCont, rcVector, rcWriting, rcParam, rcInconsistent );
        }
        else if ( self -> nancy_bool )
        {
            /* clang gets upset about switching on boolean
               the intent is to catch implementations that
               may treat anything non-zero as true. */
            switch ( ( uint32_t ) * ( const bool* ) data )
            {
            case 0:
            case 1:
                break;
            default:
                return RC ( rcCont, rcVector, rcWriting, rcParam, rcInvalid );
            }
            return Nancy1Set ( & self -> nancy, key, * ( const bool* ) data );
        }

        switch ( bytes )
        {
        case 1:
            value = * ( const uint8_t* ) data;
            break;
        case 2:
            value = * ( const uint16_t* ) data;
            break;
        case 4:
            value = * ( const uint32_t* ) data;
            break;
        case 8:
            value = ( Word_t ) * ( const uint64_t* ) data;
            break;
        }

        rc = NancyLSet ( & self -> nancy, key, value );
    }

    return rc;
}

/* Set
 *  set typed values
 *
 *  "key" [ IN ] - vector index
 *
 *  "data" [ IN ] - value
 */
LIB_EXPORT rc_t CC KVectorSetBoolOld ( KVector *self, uint64_t key, bool data )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcCont, rcVector, rcWriting, rcSelf, rcNull );
    else
    {
        /* detect first set */
        if ( self -> nancy == NULL )
        {
            self -> fixed_size = sizeof data;
            self -> nancy_bool = true;
        }
        else if ( ! self -> nancy_bool )
        {
            return RC ( rcCont, rcVector, rcWriting, rcParam, rcInvalid );
        }

        rc = Nancy1Set ( & self -> nancy, key, data );
    }

    return rc;
}

LIB_EXPORT rc_t CC KVectorSetBool ( KVector *self, uint64_t key, bool data )
{
    rc_t rc;

    bstored_bits_t stored_bits = 0;
    uint64_t key_qword = key / (sizeof(stored_bits) * 8 / BOOL_VECT_RECORD_SIZE_IN_BITS);
    size_t bit_offset_in_qword = (key % (sizeof(stored_bits) * 8 / BOOL_VECT_RECORD_SIZE_IN_BITS)) * BOOL_VECT_RECORD_SIZE_IN_BITS;
    size_t new_bit_record;
    size_t stored_bit_record;

    data = !!data; /* forcing bool to be E {0, 1} */

    rc = KVectorBoolGetStoredBits ( self, key_qword, &stored_bits );
    if ( rc != 0 )
    {
        if ( GetRCState ( rc ) != rcNotFound )
            return rc;

        assert ( stored_bits == 0 );
        rc = 0;
    }

    new_bit_record = (BOOL_VECT_BIT_SET_MASK | (uint64_t)data) << bit_offset_in_qword;
    stored_bit_record = BOOL_VECT_BIT_RECORD_MASK << bit_offset_in_qword & stored_bits;

    assert ( new_bit_record != 0 );
    assert ( rc == 0 );

    if ( new_bit_record != stored_bit_record )
    {
        stored_bits &= ~(BOOL_VECT_BIT_RECORD_MASK << bit_offset_in_qword); // clear stored record to assign a new value by bitwise OR
        stored_bits |= new_bit_record;

        rc = KVectorBoolSetStoredBits ( self, key_qword, stored_bits );
    }

    return rc;
}

LIB_EXPORT rc_t CC KVectorSetI8 ( KVector *self, uint64_t key, int8_t value )
{
    return KVectorSet ( self, key, & value, sizeof value );
}

LIB_EXPORT rc_t CC KVectorSetI16 ( KVector *self, uint64_t key, int16_t value )
{
    return KVectorSet ( self, key, & value, sizeof value );
}

LIB_EXPORT rc_t CC KVectorSetI32 ( KVector *self, uint64_t key, int32_t value )
{
    return KVectorSet ( self, key, & value, sizeof value );
}

LIB_EXPORT rc_t CC KVectorSetI64 ( KVector *self, uint64_t key, int64_t value )
{
    return KVectorSet ( self, key, & value, sizeof value );
}

LIB_EXPORT rc_t CC KVectorSetU8 ( KVector *self, uint64_t key, uint8_t value )
{
    return KVectorSet ( self, key, & value, sizeof value );
}

LIB_EXPORT rc_t CC KVectorSetU16 ( KVector *self, uint64_t key, uint16_t value )
{
    return KVectorSet ( self, key, & value, sizeof value );
}

LIB_EXPORT rc_t CC KVectorSetU32 ( KVector *self, uint64_t key, uint32_t value )
{
    return KVectorSet ( self, key, & value, sizeof value );
}

LIB_EXPORT rc_t CC KVectorSetU64 ( KVector *self, uint64_t key, uint64_t value )
{
    return KVectorSet ( self, key, & value, sizeof value );
}

LIB_EXPORT rc_t CC KVectorSetF32 ( KVector *self, uint64_t key, float value )
{
    return KVectorSet ( self, key, & value, sizeof value );
}

LIB_EXPORT rc_t CC KVectorSetF64 ( KVector *self, uint64_t key, double value )
{
    return KVectorSet ( self, key, & value, sizeof value );
}

LIB_EXPORT rc_t CC KVectorSetPtr ( KVector *self, uint64_t key, const void *value )
{
    return KVectorSet ( self, key, & value, sizeof value );
}


/* Unset
 *  unset an entry
 *  returns rc_t state rcNull if index is not set
 *
 *  "key" [ IN ] - vector index
 */
LIB_EXPORT rc_t CC KVectorUnset ( KVector *self, uint64_t key )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcCont, rcVector, rcRemoving, rcSelf, rcNull );
    else
    {
        int status;
        JError_t err;

        if ( self -> nancy_bool )
            return Nancy1Set ( & self -> nancy, key, false );

        status = JudyLDel ( & self -> nancy, ( Word_t ) key, & err );
        if ( status == JERR )
            return NancyError ( & err, rcRemoving );

        rc = 0;
    }

    return rc;
}


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
LIB_EXPORT rc_t CC KVectorVisit ( const KVector *self, bool reverse,
    rc_t ( CC * f ) ( uint64_t key, const void *value, size_t bytes, void *user_data ),
    void *user_data )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcCont, rcVector, rcVisiting, rcSelf, rcNull );
    else if ( f == NULL )
        rc = RC ( rcCont, rcVector, rcVisiting, rcFunction, rcNull );
    else
    {
        rc = 0;

        if ( self -> nancy != NULL )
        {
            JError_t err;
            Word_t first, last, idx;

	    /** Judy1First searches for first value higher then first**/ 
            first = 0;
	    /** Judy1Last searches for first value lower then last **/
            last = (Word_t)-1;

            if ( self -> nancy_bool )
            {
                bool value = true;
                int status = Judy1First ( self -> nancy, & first, & err );
                if ( status != JERR )
                    status = Judy1Last ( self -> nancy, & last, & err );
                if ( status == JERR )
                    rc = NancyError ( & err, rcVisiting );
                else if ( reverse )
                {
                    for ( idx = last; ; )
                    {
                        rc = ( * f ) ( idx, & value, sizeof value, user_data );
                        if ( rc != 0 )
                            break;
                        if ( idx == first )
                            break;
                        status = Judy1Prev ( self -> nancy, & idx, & err );
                        if ( status == JERR )
                        {
                            rc = NancyError ( & err, rcVisiting );
                            break;
                        }
                    }
                }
                else
                {
                    for ( idx = first; ; )
                    {
                        rc = ( * f ) ( idx, & value, sizeof value, user_data );
                        if ( rc != 0 )
                            break;
                        if ( idx == last )
                            break;
                        status = Judy1Next ( self -> nancy, & idx, & err );
                        if ( status == JERR )
                        {
                            rc = NancyError ( & err, rcVisiting );
                            break;
                        }
                    }
                }
            }
            else
            {
                Word_t value;
                PPvoid_t lastp, datap = JudyLFirst ( self -> nancy, & first, & err );
                if ( datap != PPJERR )
                    lastp = JudyLLast ( self -> nancy, & last, & err );
                if ( datap == PPJERR || lastp == PPJERR )
                    rc = NancyError ( & err, rcVisiting );
                else if ( reverse )
                {
                    for ( rc = 0, idx = last, datap = lastp; ; )
                    {
                        if ( datap == NULL )
                            break;
                        value = * ( const Word_t* ) datap;
                        rc = ( * f ) ( idx, & value, sizeof value, user_data );
                        if ( rc != 0 )
                            break;
                        if ( idx == first )
                            break;
                        datap = JudyLPrev ( self -> nancy, & idx, & err );
                        if ( datap == PPJERR )
                        {
                            rc = NancyError ( & err, rcVisiting );
                            break;
                        }
                    }
                }
                else
                {
                    for ( rc = 0, idx = first; ; )
                    {
                        if ( datap == NULL )
                            break;
                        value = * ( const Word_t* ) datap;
                        rc = ( * f ) ( idx, & value, sizeof value, user_data );
                        if ( rc != 0 )
                            break;
                        if ( idx == last )
                            break;
                        datap = JudyLNext ( self -> nancy, & idx, & err );
                        if ( datap == PPJERR )
                        {
                            rc = NancyError ( & err, rcVisiting );
                            break;
                        }
                    }
                }
            }
        }
    }

    return rc;
}

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
typedef struct KVectorVisitTypedData KVectorVisitTypedData;
struct KVectorVisitTypedData
{
    union
    {
        rc_t ( CC * b ) ( uint64_t key, bool value, void *user_data );
        rc_t ( CC * i ) ( uint64_t key, int64_t value, void *user_data );
        rc_t ( CC * u ) ( uint64_t key, uint64_t value, void *user_data );
        rc_t ( CC * f ) ( uint64_t key, double value, void *user_data );
        rc_t ( CC * p ) ( uint64_t key, const void *value, void *user_data );
        rc_t ( CC * u32 ) ( uint64_t key, uint32_t value, void *user_data );
    } f;
    void *user_data;
};

static
rc_t CC KVectorVisitBoolFunc ( uint64_t key, const void *ptr, size_t bytes, void *user_data )
{
    rc_t rc;
    bool value;
    if ( bytes != sizeof value )
        rc = RC ( rcCont, rcVector, rcVisiting, rcType, rcInvalid );
    else if ( * ( const uint8_t* ) ptr > 1 )
        rc = RC ( rcCont, rcVector, rcVisiting, rcData, rcInvalid );
    else
    {
        KVectorVisitTypedData *pb = user_data;
        rc = ( * pb -> f . b ) ( key,  * ( const bool* ) ptr, pb -> user_data );
    }

    return rc;
}

LIB_EXPORT rc_t CC KVectorVisitBoolOld ( const KVector *self, bool reverse,
    rc_t ( CC * f ) ( uint64_t key, bool value, void *user_data ),
    void *user_data )
{
    KVectorVisitTypedData pb;
    pb . f . b = f;
    pb . user_data = user_data;

    return KVectorVisit ( self, reverse, KVectorVisitBoolFunc, & pb );
}

typedef struct UserDataStoredBitstoBool UserDataStoredBitstoBool;
struct UserDataStoredBitstoBool
{
    rc_t ( * f ) ( uint64_t key, bool value, void *user_data );
    void* user_data;
};

static rc_t VisitStoredBitstoBoolAdapter ( uint64_t key, bstored_bits_t value, void *user_data )
{
    rc_t ( * bool_callback ) ( uint64_t key, bool value, void *user_data );

    rc_t rc = 0;
    size_t i;
    void* original_user_data = ((struct UserDataStoredBitstoBool*) user_data) -> user_data;
    bool_callback = ((UserDataStoredBitstoBool*) user_data) -> f;

    for ( i = 0; i < sizeof (value) * 8 / BOOL_VECT_RECORD_SIZE_IN_BITS; ++i )
    {
        uint64_t key_bool = key * sizeof(value) * 8 / BOOL_VECT_RECORD_SIZE_IN_BITS + i;
        size_t record = value >> i * BOOL_VECT_RECORD_SIZE_IN_BITS & BOOL_VECT_BIT_RECORD_MASK;
        if ( record & BOOL_VECT_BIT_SET_MASK )
        {
            rc = bool_callback ( key_bool, (bool) (record & BOOL_VECT_BIT_VALUE_MASK), original_user_data );
            if ( rc )
                return rc;
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC KVectorVisitBool ( const KVector *self, bool reverse,
    rc_t ( CC * f ) ( uint64_t key, bool value, void *user_data ),
    void *user_data )
{
    UserDataStoredBitstoBool user_data_adapter = { f, user_data };
    return KVectorBoolVisitStoredBits ( self, reverse, VisitStoredBitstoBoolAdapter, &user_data_adapter );
}

static
rc_t CC KVectorVisitI64Func ( uint64_t key, const void *ptr, size_t bytes, void *user_data )
{
    int64_t value;
    KVectorVisitTypedData *pb = user_data;

    switch ( bytes )
    {
    case 1:
        value = ( int8_t ) * ( const Word_t* ) ptr;
        break;
    case 2:
        value = ( int16_t ) * ( const Word_t* ) ptr;
        break;
    case 4:
        value = ( int32_t ) * ( const Word_t* ) ptr;
        break;
    default:
        value = ( int64_t ) * ( const Word_t* ) ptr;
    }

    return ( * pb -> f . i ) ( key, value, pb -> user_data );
}

LIB_EXPORT rc_t CC KVectorVisitI64 ( const KVector *self, bool reverse,
    rc_t ( CC * f ) ( uint64_t key, int64_t value, void *user_data ),
    void *user_data )
{
    KVectorVisitTypedData pb;
    pb . f . i = f;
    pb . user_data = user_data;

    return KVectorVisit ( self, reverse, KVectorVisitI64Func, & pb );
}

static
rc_t CC KVectorVisitU64Func ( uint64_t key, const void *ptr, size_t bytes, void *user_data )
{
    KVectorVisitTypedData *pb = user_data;
    return ( * pb -> f . u ) ( key, * ( const Word_t* ) ptr, pb -> user_data );
}

LIB_EXPORT rc_t CC KVectorVisitU64 ( const KVector *self, bool reverse,
    rc_t ( CC * f ) ( uint64_t key, uint64_t value, void *user_data ),
    void *user_data )
{
    KVectorVisitTypedData pb;
    pb . f . u = f;
    pb . user_data = user_data;

    return KVectorVisit ( self, reverse, KVectorVisitU64Func, & pb );
}

rc_t CC KVectorVisitU32Func ( uint64_t key, const void *ptr, size_t bytes, void *user_data )
{
    KVectorVisitTypedData *pb = user_data;
    return ( * pb -> f . u32 ) ( key, * ( const Word_t* ) ptr, pb -> user_data );
}

LIB_EXPORT rc_t CC KVectorVisitU32 ( const KVector *self, bool reverse,
    rc_t ( CC * f ) ( uint64_t key, uint32_t value, void *user_data ),
    void *user_data )
{
    KVectorVisitTypedData pb;
    pb . f . u32 = f;
    pb . user_data = user_data;

    return KVectorVisit ( self, reverse, KVectorVisitU32Func, & pb );
}

static
rc_t CC KVectorVisitF64Func ( uint64_t key, const void *ptr, size_t bytes, void *user_data )
{
    uint32_t u32;
    double value;
    KVectorVisitTypedData *pb = user_data;

    switch ( bytes )
    {
    case 4:
        u32 = ( uint32_t ) * ( const Word_t* ) ptr;
        value = * ( const float* ) & u32;
        break;
    default:
        value = * ( const double* ) ptr;
    }

    return ( * pb -> f . f ) ( key, value, pb -> user_data );
}

LIB_EXPORT rc_t CC KVectorVisitF64 ( const KVector *self, bool reverse,
    rc_t ( CC * f ) ( uint64_t key, double value, void *user_data ),
    void *user_data )
{
    KVectorVisitTypedData pb;
    pb . f . f = f;
    pb . user_data = user_data;

    return KVectorVisit ( self, reverse, KVectorVisitF64Func, & pb );
}

static
rc_t CC KVectorVisitPtrFunc ( uint64_t key, const void *ptr, size_t bytes, void *user_data )
{
    KVectorVisitTypedData *pb = user_data;
    return ( * pb -> f . p ) ( key, * ( const void** ) ptr, pb -> user_data );
}

LIB_EXPORT rc_t CC KVectorVisitPtr ( const KVector *self, bool reverse,
    rc_t ( CC * f ) ( uint64_t key, const void *value, void *user_data ),
    void *user_data )
{
    KVectorVisitTypedData pb;
    pb . f . p = f;
    pb . user_data = user_data;

    return KVectorVisit ( self, reverse, KVectorVisitPtrFunc, & pb );
}
