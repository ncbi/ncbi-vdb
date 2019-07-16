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

#include <kns/extern.h>
#include <kns/impl.h>
#include <klib/rc.h>
#include <kproc/timeout.h>

#include <os-native.h>

#include <sysalloc.h>

#include "stream-priv.h"

#include <assert.h>

/*--------------------------------------------------------------------------
 * KStream
 *  a file
 */

/* Destroy
 *  destroy stream
 */
static
rc_t KStreamDestroy ( KStream *self )
{
    if ( self == NULL )
        return RC ( rcNS, rcStream, rcDestroying, rcSelf, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . destroy ) ( self );
    }

    return RC ( rcNS, rcStream, rcDestroying, rcInterface, rcBadVersion );
}

/* AddRef
 *  creates a new reference
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC KStreamAddRef ( const KStream *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KStream" ) )
        {
        case krefLimit:
            return RC ( rcNS, rcStream, rcAttaching, rcRange, rcExcessive );
        case krefNegative:
            return RC ( rcNS, rcStream, rcAttaching, rcSelf, rcInvalid );
        default:
            break;
        }
    }
    return 0;
}

/* Release
 *  discard reference to file
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC KStreamRelease ( const KStream *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KStream" ) )
        {
        case krefWhack:
            return KStreamDestroy ( ( KStream* ) self );
        case krefNegative:
            return RC ( rcNS, rcStream, rcReleasing, rcRange, rcExcessive );
        default:
            break;
        }
    }

    return 0;
}

/* Read
 * TimedRead
 *  read data from stream
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_read" [ OUT ] - return parameter giving number of bytes
 *  actually read. when returned value is zero and return code is
 *  also zero, interpreted as end of stream.
 *
 *  "tm" [ IN/OUT, NULL OKAY ] - an optional indicator of
 *  blocking behavior. not all implementations will support
 *  timed reads. a NULL timeout will block indefinitely,
 *  a value of "tm->mS == 0" will have non-blocking behavior
 *  if supported by implementation, and "tm->mS > 0" will indicate
 *  a maximum wait timeout.
 */
LIB_EXPORT rc_t CC KStreamRead ( const KStream *self,
    void *buffer, size_t bsize, size_t *num_read )
{
    if ( num_read == NULL )
        return RC ( rcNS, rcStream, rcReading, rcParam, rcNull );

    * num_read = 0;

    if ( self == NULL )
        return RC ( rcNS, rcStream, rcReading, rcSelf, rcNull );

    if ( ! self -> read_enabled )
        return RC ( rcNS, rcStream, rcReading, rcStream, rcNoPerm );

    if ( buffer == NULL )
        return RC ( rcNS, rcStream, rcReading, rcBuffer, rcNull );
    if ( bsize == 0 )
        return RC ( rcNS, rcStream, rcReading, rcBuffer, rcInsufficient );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . read ) ( self, buffer, bsize, num_read );
    }

    return RC ( rcNS, rcStream, rcReading, rcInterface, rcBadVersion );
}

LIB_EXPORT rc_t CC KStreamTimedRead ( const KStream *self,
    void *buffer, size_t bsize, size_t *num_read, struct timeout_t *tm )
{
    if ( num_read == NULL )
        return RC ( rcNS, rcStream, rcReading, rcParam, rcNull );

    * num_read = 0;

    if ( self == NULL )
        return RC ( rcNS, rcStream, rcReading, rcSelf, rcNull );

    if ( ! self -> read_enabled )
        return RC ( rcNS, rcStream, rcReading, rcStream, rcNoPerm );

    if ( buffer == NULL )
        return RC ( rcNS, rcStream, rcReading, rcBuffer, rcNull );
    if ( bsize == 0 )
        return RC ( rcNS, rcStream, rcReading, rcBuffer, rcInsufficient );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        if ( self -> vt -> v1 . min >= 1 )
            return ( * self -> vt -> v1 . timed_read ) ( self, buffer, bsize, num_read, tm );
        if ( tm == NULL )
            return ( * self -> vt -> v1 . read ) ( self, buffer, bsize, num_read );
        break;
    }


    return RC ( rcNS, rcStream, rcReading, rcInterface, rcBadVersion );
}


/* ReadAll
 * TimedReadAll
 *  read from stream until "bsize" bytes have been retrieved
 *  or until end-of-input
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_read" [ OUT ] - return parameter giving number of bytes
 *  actually read. when returned value is zero and return code is
 *  also zero, interpreted as end of stream.
 *
 *  "tm" [ IN/OUT, NULL OKAY ] - an optional indicator of
 *  blocking behavior. not all implementations will support
 *  timed reads. a NULL timeout will block indefinitely,
 *  a value of "tm->mS == 0" will have non-blocking behavior
 *  if supported by implementation, and "tm->mS > 0" will indicate
 *  a maximum wait timeout.
 */
LIB_EXPORT rc_t CC KStreamReadAll ( const KStream *self,
    void *buffer, size_t bsize, size_t *num_read )
{
    rc_t rc;
    uint8_t *b;
    size_t total, count;

    if ( num_read == NULL )
        return RC ( rcNS, rcStream, rcReading, rcParam, rcNull );

    * num_read = 0;

    if ( self == NULL )
        return RC ( rcNS, rcStream, rcReading, rcSelf, rcNull );

    if ( ! self -> read_enabled )
        return RC ( rcNS, rcStream, rcReading, rcStream, rcNoPerm );

    if ( buffer == NULL )
        return RC ( rcNS, rcStream, rcReading, rcBuffer, rcNull );
    if ( bsize == 0 )
        return RC ( rcNS, rcStream, rcReading, rcBuffer, rcInsufficient );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        count = 0;
        rc = ( * self -> vt -> v1 . read ) ( self, buffer, bsize, & count );
        total = count;

        if ( rc == 0 && count != 0 && count < bsize )
        {
            if ( self -> vt -> v1 . min >= 1 )
            {
                timeout_t no_block;
                TimeoutInit ( & no_block, 0 );

                for ( rc = 0, b = buffer; total < bsize; total += count )
                {
                    count = 0;
                    rc = ( * self -> vt -> v1 . timed_read ) ( self, b + total, bsize - total, & count, & no_block );
                    if ( rc != 0 )
                        break;
                    if ( count == 0 )
                        break;
                }
            }
            else
            {
                for ( rc = 0, b = buffer; total < bsize; total += count )
                {
                    count = 0;
                    rc = ( * self -> vt -> v1 . read ) ( self, b + total, bsize - total, & count );
                    if ( rc != 0 )
                        break;
                    if ( count == 0 )
                        break;
                }
            }
        }
        break;

    default:
        return RC ( rcNS, rcStream, rcReading, rcInterface, rcBadVersion );
    }

    if ( total != 0 )
    {
        * num_read = total;
        return 0;
    }

    return rc;
}

LIB_EXPORT rc_t CC KStreamTimedReadAll ( const KStream *self,
    void *buffer, size_t bsize, size_t *num_read, struct timeout_t *tm )
{
    rc_t rc;
    uint8_t *b;
    size_t total, count;

    if ( num_read == NULL )
        return RC ( rcNS, rcStream, rcReading, rcParam, rcNull );

    * num_read = 0;

    if ( self == NULL )
        return RC ( rcNS, rcStream, rcReading, rcSelf, rcNull );

    if ( ! self -> read_enabled )
        return RC ( rcNS, rcStream, rcReading, rcStream, rcNoPerm );

    if ( buffer == NULL )
        return RC ( rcNS, rcStream, rcReading, rcBuffer, rcNull );
    if ( bsize == 0 )
        return RC ( rcNS, rcStream, rcReading, rcBuffer, rcInsufficient );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        if ( self -> vt -> v1 . min >= 1 )
        {
            count = 0;
            rc = ( * self -> vt -> v1 . timed_read ) ( self, buffer, bsize, & count, tm );
            total = count;

            if ( rc == 0 && count != 0 && count < bsize )
            {
                timeout_t no_block;
                TimeoutInit ( & no_block, 0 );
                
                for ( b = buffer; total < bsize; total += count )
                {
                    count = 0;
                    rc = ( * self -> vt -> v1 . timed_read ) ( self, b + total, bsize - total, & count, & no_block );
                    if ( rc != 0 )
                        break;
                    if ( count == 0 )
                        break;
                }
            }
            break;
        }

        if ( tm == NULL )
        {
            for ( rc = 0, b = buffer, total = 0; total < bsize; total += count )
            {
                count = 0;
                rc = ( * self -> vt -> v1 . read ) ( self, b + total, bsize - total, & count );
                if ( rc != 0 )
                    break;
                if ( count == 0 )
                    break;
            }
            break;
        }

        /* no break */

    default:
        return RC ( rcNS, rcStream, rcReading, rcInterface, rcBadVersion );
    }

    if ( total != 0 )
    {
        * num_read = total;
        return 0;
    }

    return rc;
}


/* ReadExactly
 * TimedReadExactly
 *  read from stream until "bytes" have been retrieved
 *  or return incomplete transfer
 *
 *  "buffer" [ OUT ] and "bytes" [ IN ] - return buffer for read
 *
 *  "tm" [ IN/OUT, NULL OKAY ] - an optional indicator of
 *  blocking behavior. not all implementations will support
 *  timed reads. a NULL timeout will block indefinitely,
 *  a value of "tm->mS == 0" will have non-blocking behavior
 *  if supported by implementation, and "tm->mS > 0" will indicate
 *  a maximum wait timeout.
 */
LIB_EXPORT rc_t CC KStreamReadExactly ( const KStream *self,
    void *buffer, size_t bytes )
{
    rc_t rc;
    uint8_t *b;
    size_t total, count;

    if ( self == NULL )
        return RC ( rcNS, rcStream, rcReading, rcSelf, rcNull );

    if ( ! self -> read_enabled )
        return RC ( rcNS, rcStream, rcReading, rcStream, rcNoPerm );

    if ( bytes == 0 )
        return 0;
    if ( buffer == NULL )
        return RC ( rcNS, rcStream, rcReading, rcBuffer, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        assert ( bytes != 0 );
        for ( b = buffer, total = 0; total < bytes; total += count )
        {
            count = 0;
            rc = ( * self -> vt -> v1 . read ) ( self, b + total, bytes - total, & count );
            if ( rc != 0 )
            {
                if ( GetRCObject ( rc ) != ( enum RCObject )rcTimeout || GetRCState ( rc ) != rcExhausted )
                    break;
            }
            else if ( count == 0 )
            {
                rc = RC ( rcNS, rcStream, rcReading, rcTransfer, rcIncomplete );
                break;
            }
        }
        break;

    default:
        return RC ( rcNS, rcStream, rcReading, rcInterface, rcBadVersion );
    }

    return rc;
}

LIB_EXPORT rc_t CC KStreamTimedReadExactly ( const KStream *self,
    void *buffer, size_t bytes, struct timeout_t *tm )
{
    rc_t rc;
    uint8_t *b;
    size_t total, count;

    if ( self == NULL )
        return RC ( rcNS, rcStream, rcReading, rcSelf, rcNull );

    if ( ! self -> read_enabled )
        return RC ( rcNS, rcStream, rcReading, rcStream, rcNoPerm );

    if ( bytes == 0 )
        return RC ( rcNS, rcStream, rcReading, rcBuffer, rcInsufficient );
    if ( buffer == NULL )
        return RC ( rcNS, rcStream, rcReading, rcBuffer, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        if ( self -> vt -> v1 . min >= 1 )
        {
            assert ( bytes != 0 );
            for ( b = buffer, total = 0; total < bytes; total += count )
            {
                count = 0;
                rc = ( * self -> vt -> v1 . timed_read ) ( self, b + total, bytes - total, & count, tm );
                if ( rc != 0 )
                {
                    if ( tm != NULL )
                        break;
                    if ( GetRCObject ( rc ) != ( enum RCObject ) rcTimeout || GetRCState ( rc ) != rcExhausted )
                        break;
                }
                else if ( count == 0 )
                {
                    rc = RC ( rcNS, rcStream, rcReading, rcTransfer, rcIncomplete );
                    break;
                }
            }

            break;
        }

        if ( tm == NULL )
        {
            assert ( bytes != 0 );
            for ( b = buffer, total = 0; total < bytes; total += count )
            {
                count = 0;
                rc = ( * self -> vt -> v1 . read ) ( self, b + total, bytes - total, & count );
                if ( rc != 0 )
                {
                    if ( GetRCObject ( rc ) != ( enum RCObject )rcTimeout || GetRCState ( rc ) != rcExhausted )
                        break;
                }
                else if ( count == 0 )
                {
                    rc = RC ( rcNS, rcStream, rcReading, rcTransfer, rcIncomplete );
                    break;
                }
            }
            break;
        }

        /* no break */

    default:
        return RC ( rcNS, rcStream, rcReading, rcInterface, rcBadVersion );
    }

    return rc;
}

/* Write
 * TimedWrite
 *  send data to stream
 *
 *  "buffer" [ IN ] and "size" [ IN ] - data to be written
 *
 *  "num_writ" [ OUT, NULL OKAY ] - optional return parameter
 *  giving number of bytes actually written
 *
 *  "tm" [ IN/OUT, NULL OKAY ] - an optional indicator of
 *  blocking behavior. not all implementations will support
 *  timed writes. a NULL timeout will block indefinitely,
 *  a value of "tm->mS == 0" will have non-blocking behavior
 *  if supported by implementation, and "tm->mS > 0" will indicate
 *  a maximum wait timeout.
 */
LIB_EXPORT rc_t CC KStreamWrite ( KStream *self,
    const void *buffer, size_t size, size_t *num_writ )
{
    size_t ignore;
    if ( num_writ == NULL )
        num_writ = & ignore;

    * num_writ = 0;

    if ( self == NULL )
        return RC ( rcNS, rcStream, rcWriting, rcSelf, rcNull );

    if ( ! self -> write_enabled )
        return RC ( rcNS, rcStream, rcWriting, rcStream, rcNoPerm );

    if ( size == 0 )
        return 0;
    if ( buffer == NULL )
        return RC ( rcNS, rcStream, rcWriting, rcBuffer, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . write ) ( self, buffer, size, num_writ );
    }

    return RC ( rcNS, rcStream, rcWriting, rcInterface, rcBadVersion );
}

LIB_EXPORT rc_t CC KStreamTimedWrite ( KStream *self,
    const void *buffer, size_t size, size_t *num_writ, struct timeout_t *tm )
{
    size_t ignore;
    if ( num_writ == NULL )
        num_writ = & ignore;

    * num_writ = 0;

    if ( self == NULL )
        return RC ( rcNS, rcStream, rcWriting, rcSelf, rcNull );

    if ( ! self -> write_enabled )
        return RC ( rcNS, rcStream, rcWriting, rcStream, rcNoPerm );

    if ( size == 0 )
        return 0;
    if ( buffer == NULL )
        return RC ( rcNS, rcStream, rcWriting, rcBuffer, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        if ( self -> vt -> v1 . min >= 1 )
            return ( * self -> vt -> v1 . timed_write ) ( self, buffer, size, num_writ, tm );
        if ( tm == NULL )
            return ( * self -> vt -> v1 . write ) ( self, buffer, size, num_writ );
        break;
    }

    return RC ( rcNS, rcStream, rcWriting, rcInterface, rcBadVersion );
}

/* WriteAll
 * TimedWriteAll
 *  write to stream until "size" bytes have been transferred
 *  or until no further progress can be made
 *
 *  "buffer" [ IN ] and "size" [ IN ] - data to be written
 *
 *  "num_writ" [ OUT, NULL OKAY ] - optional return parameter
 *  giving number of bytes actually written
 *
 *  "tm" [ IN/OUT, NULL OKAY ] - an optional indicator of
 *  blocking behavior. not all implementations will support
 *  timed writes. a NULL timeout will block indefinitely,
 *  a value of "tm->mS == 0" will have non-blocking behavior
 *  if supported by implementation, and "tm->mS > 0" will indicate
 *  a maximum wait timeout.
 */
LIB_EXPORT rc_t CC KStreamWriteAll ( KStream *self,
    const void *buffer, size_t size, size_t *num_writ )
{
    rc_t rc;
    const uint8_t *b;
    size_t total, count;

    size_t ignore;
    if ( num_writ == NULL )
        num_writ = & ignore;

    * num_writ = 0;

    if ( self == NULL )
        return RC ( rcNS, rcStream, rcWriting, rcSelf, rcNull );

    if ( ! self -> write_enabled )
        return RC ( rcNS, rcStream, rcWriting, rcStream, rcNoPerm );

    if ( size == 0 )
        return 0;
    if ( buffer == NULL )
        return RC ( rcNS, rcStream, rcWriting, rcBuffer, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        count = 0;
        rc = ( * self -> vt -> v1 . write ) ( self, buffer, size, & count );
        total = count;

        if ( rc == 0 && count != 0 && count < size )
        {
            if ( self -> vt -> v1 . min >= 1 )
            {
                timeout_t no_block;
                TimeoutInit ( & no_block, 0 );

                for ( b = buffer; total < size; total += count )
                {
                    count = 0;
                    rc = ( * self -> vt -> v1 . timed_write ) ( self, b + total, size - total, & count, & no_block );
                    if ( rc != 0 )
                        break;
                    if ( count == 0 )
                        break;
                }
            }
            else
            {
                for ( b = buffer; total < size; total += count )
                {
                    count = 0;
                    rc = ( * self -> vt -> v1 . write ) ( self, b + total, size - total, & count );
                    if ( rc != 0 )
                        break;
                    if ( count == 0 )
                        break;
                }
            }
        }
        break;

    default:
        return RC ( rcNS, rcStream, rcWriting, rcInterface, rcBadVersion );
    }

    * num_writ = total;
    if ( total == size )
        return 0;
    if ( rc == 0 )
        return RC ( rcNS, rcStream, rcWriting, rcTransfer, rcIncomplete );
    return rc;
}

LIB_EXPORT rc_t CC KStreamTimedWriteAll ( KStream *self,
    const void *buffer, size_t size, size_t *num_writ, struct timeout_t *tm )
{
    rc_t rc;
    const uint8_t *b;
    size_t total, count;

    size_t ignore;
    if ( num_writ == NULL )
        num_writ = & ignore;

    * num_writ = 0;

    if ( self == NULL )
        return RC ( rcNS, rcStream, rcWriting, rcSelf, rcNull );

    if ( ! self -> write_enabled )
        return RC ( rcNS, rcStream, rcWriting, rcStream, rcNoPerm );

    if ( size == 0 )
        return 0;
    if ( buffer == NULL )
        return RC ( rcNS, rcStream, rcWriting, rcBuffer, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        if ( self -> vt -> v1 . min >= 1 )
        {
            for ( rc = 0, b = buffer, total = 0; total < size; total += count )
            {
                count = 0;
                rc = ( * self -> vt -> v1 . timed_write ) ( self, b + total, size - total, & count, tm );
                if ( rc != 0 )
                    break;
                if ( count == 0 )
                    break;
            }
            break;
        }

        if ( tm == NULL )
        {
            for ( rc = 0, b = buffer, total = 0; total < size; total += count )
            {
                count = 0;
                rc = ( * self -> vt -> v1 . write ) ( self, b + total, size - total, & count );
                if ( rc != 0 )
                    break;
                if ( count == 0 )
                    break;
            }
            break;
        }

        /* no break */

    default:
        return RC ( rcNS, rcStream, rcWriting, rcInterface, rcBadVersion );
    }

    * num_writ = total;
    if ( total == size )
        return 0;
    if ( rc == 0 )
        return RC ( rcNS, rcStream, rcWriting, rcTransfer, rcIncomplete );
    return rc;
}


/* Init
 *  initialize a newly allocated stream object
 */
LIB_EXPORT rc_t CC KStreamInit ( KStream *self, const KStream_vt *vt,
    const char *classname, const char *strname,
    bool read_enabled, bool write_enabled )
{
    if ( self == NULL )
        return RC ( rcNS, rcStream, rcConstructing, rcSelf, rcNull );
    if ( vt == NULL )
        return RC ( rcNS, rcStream, rcConstructing, rcInterface, rcNull );
    switch ( vt -> v1 . maj )
    {
    case 0:
        return RC ( rcNS, rcStream, rcConstructing, rcInterface, rcInvalid );

    case 1:
        switch ( vt -> v1 . min )
        {
            /* ADD NEW MINOR VERSION CASES HERE */
        case 1:
#if _DEBUGGING
            if ( vt -> v1 . timed_write == NULL ||
                 vt -> v1 . timed_read == NULL )
                return RC ( rcNS, rcStream, rcConstructing, rcInterface, rcNull );
#endif
        case 0:
#if _DEBUGGING
        if ( vt -> v1 . write == NULL         ||
             vt -> v1 . read == NULL          ||
             vt -> v1 . destroy == NULL )
        return RC ( rcNS, rcStream, rcConstructing, rcInterface, rcNull );
#endif
            break;
        default:
            return RC ( rcNS, rcStream, rcConstructing, rcInterface, rcBadVersion );
        }
        break;

    default:
        return RC ( rcNS, rcStream, rcConstructing, rcInterface, rcBadVersion );
    }

    self -> vt = vt;
    KRefcountInit ( & self -> refcount, 1, classname, "init", strname );
    self -> read_enabled = ( uint8_t ) ( read_enabled != 0 );
    self -> write_enabled = ( uint8_t ) ( write_enabled != 0 );

    return 0;
}

/* Whack
 *  does anything necessary to tear down KStream
 *  but does not delete memory
 */
LIB_EXPORT rc_t CC KStreamWhack ( KStream * self, const char * classname )
{
    if ( self != NULL )
    {
        KRefcountWhack ( & self -> refcount, classname ? classname : "KStream" );
        self -> read_enabled = false;
        self -> write_enabled = false;
    }
    return 0;
}
