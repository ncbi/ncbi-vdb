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

#include <kfs/extern.h>
#include <kfs/impl.h>
#include <klib/rc.h>
#include <klib/status.h>
#include <sysalloc.h>

#include <assert.h>

/*--------------------------------------------------------------------------
 * KChunkReader
 *  an interface for driving chunked responses from a single read
 */

/* AddRef
 * Release
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC KChunkReaderAddRef ( const KChunkReader * self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KChunkReader" ) )
        {
        case krefLimit:
            return RC ( rcFS, rcBuffer, rcAttaching, rcRange, rcExcessive );
        case krefNegative:
            return RC ( rcFS, rcBuffer, rcAttaching, rcSelf, rcInvalid );
        default:
            break;
        }
    }
    return 0;
}

static
rc_t KChunkReaderDestroy ( KChunkReader * self )
{
    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . destroy ) ( self );
    }

    return RC ( rcFS, rcBuffer, rcDestroying, rcInterface, rcBadVersion );
}

LIB_EXPORT rc_t CC KChunkReaderRelease ( const KChunkReader * self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KChunkReader" ) )
        {
        case krefWhack:
            return KChunkReaderDestroy ( ( KChunkReader * ) self );
        case krefNegative:
            return RC ( rcFS, rcBuffer, rcReleasing, rcRange, rcExcessive );
        default:
            break;
        }
    }

    return 0;
}

/* BufferSize
 *  returns 0 if size is not known or not known to be consistent
 *  returns > 0 if size is known and consistent
 *  returns 0 if "self" is NULL
 */
LIB_EXPORT size_t CC KChunkReaderBufferSize ( const KChunkReader * self )
{
    if ( self == NULL )
        return 0;

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . buffer_size ) ( self );
    }

    return 0;
}

/* NextBuffer
 *  returns a pointer to and size of the next available buffer for chunk
 *
 *  "buf" [ OUT ] - pointer to chunk buffer
 *
 *  "size" [ OUT ] - size of chunk buffer
 */
LIB_EXPORT rc_t CC KChunkReaderNextBuffer ( KChunkReader * self, void ** buf, size_t * size )
{
    rc_t rc;

    if ( buf == NULL )
        rc = RC ( rcFS, rcBuffer, rcAllocating, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcFS, rcBuffer, rcAllocating, rcSelf, rcNull );
        else
        {
            size_t dummy;

            if ( size == NULL )
                size = & dummy;

            switch ( self -> vt -> v1 . maj )
            {
            case 1:
                rc = ( * self -> vt -> v1 . next_buffer ) ( self, buf, size );
                break;
            default:
                rc = RC ( rcFS, rcBuffer, rcAllocating, rcInterface, rcBadVersion );
            }

            if ( rc == 0 && * size < 256U )
            {
                KChunkReaderReturnBuffer ( self, * buf, * size );
                rc = RC ( rcFS, rcBuffer, rcAllocating, rcBuffer, rcInsufficient );
            }
        }
    }

    return rc;
}

/* ConsumeChunk
 *  delivered to consumer to process the chunk as desired,
 *  e.g. to write to a file or display to a monitor
 *
 *  "pos" - the position of the chunk within the stream or object
 *
 *  "buf" - pointer to buffer
 *
 *  "bytes - the number of valid bytes in buffer; <= size
 */
LIB_EXPORT rc_t CC KChunkReaderConsumeChunk ( KChunkReader * self,
    uint64_t pos, const void * buf, size_t size )
{
    if ( self == NULL )
        return RC ( rcFS, rcBuffer, rcWriting, rcSelf, rcNull );

    if ( buf == NULL )
        return  RC ( rcFS, rcBuffer, rcWriting, rcParam, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . consume_chunk ) ( self, pos, buf, size );
    }

    return RC ( rcFS, rcBuffer, rcWriting, rcInterface, rcBadVersion );
}

/* ReturnBuffer
 *  allows reader to recycle or free the chunk buffer
 *
 *  "buf" - pointer to chunk buffer
 *
 *  "size" - size of chunk buffer
 */
LIB_EXPORT rc_t CC KChunkReaderReturnBuffer ( KChunkReader * self, void * buf, size_t size )
{
    if ( self == NULL )
        return RC ( rcFS, rcBuffer, rcReleasing, rcSelf, rcNull );

    if ( buf == NULL )
        return  RC ( rcFS, rcBuffer, rcReleasing, rcParam, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . return_buffer ) ( self, buf, size );
    }

    return RC ( rcFS, rcBuffer, rcReleasing, rcInterface, rcBadVersion );
}

/* Init
 *  initialize a newly allocated chunk-reader object
 */
LIB_EXPORT rc_t CC KChunkReaderInit ( KChunkReader * self, const KChunkReader_vt *vt )
{
    if ( self == NULL )
        return RC ( rcFS, rcBuffer, rcConstructing, rcSelf, rcNull );

    if ( vt == NULL )
        return RC ( rcFS, rcBuffer, rcConstructing, rcInterface, rcNull );

    switch ( vt -> v1 . maj )
    {
    case 0:
        return RC ( rcFS, rcBuffer, rcConstructing, rcInterface, rcInvalid );

    case 1:
        switch ( vt -> v1 . min )
        {
            /* ADD NEW MINOR VERSION CASES HERE */
        case 0:
#if _DEBUGGING
        if ( vt -> v1 . return_buffer == NULL ||
             vt -> v1 . consume_chunk == NULL ||
             vt -> v1 . next_buffer == NULL   ||
             vt -> v1 . buffer_size == NULL   ||
             vt -> v1 . destroy == NULL )
        return RC ( rcFS, rcBuffer, rcConstructing, rcInterface, rcNull );
#endif
            break;
        default:
            return RC ( rcFS, rcBuffer, rcConstructing, rcInterface, rcBadVersion );
        }
        break;

    default:
        return RC ( rcFS, rcBuffer, rcConstructing, rcInterface, rcBadVersion );
    }

    self -> vt = vt;
    KRefcountInit ( & self -> refcount, 1, "KChunkReader", "init", "" );

    return 0;
}

