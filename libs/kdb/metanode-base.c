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

#include <kdb/extern.h>

#include "metanode-base.h"

#include <byteswap.h>
#include <strtol.h>
#include <limits.h>

rc_t
KMDataNodeBaseWhack ( KMDataNode *self )
{
    KRefcountWhack ( & self -> refcount, "KMDataNode" );
    free ( self );
    return 0;
}

/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
rc_t CC
KMDataNodeBaseAddRef ( const KMDataNode *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KMDataNode" ) )
        {
        case krefLimit:
            return RC ( rcDB, rcMetadata, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

rc_t CC
KMDataNodeBaseRelease ( const KMDataNode *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KMDataNode" ) )
        {
        case krefOkay:
            break;
        case krefWhack:
            KMDataNodeWhack( (KMDataNode*)self );
            break;
        case krefNegative:
            return RC ( rcDB, rcMetadata, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

/* Attach
 * Sever
 */
KMDataNode *KMDataNodeAttach ( const KMDataNode *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAddDep ( & self -> refcount, "KMDataNode" ) )
        {
        case krefLimit:
            return NULL;
        }
    }
    return ( KMDataNode* ) self;
}

rc_t KMDataNodeSever ( const KMDataNode *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDropDep ( & self -> refcount, "KMDataNode" ) )
        {
        case krefWhack:
            return self -> vt -> whack ( (KMDataNode *)self );
        case krefNegative:
            return RC ( rcDB, rcMgr, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

/******************* dispatch functions ***********************/
#define DISPATCH(call)  \
    if ( self != NULL && self -> vt != NULL )   \
        return self -> vt -> call;              \
    else                                        \
        return RC ( rcDB, rcMgr, rcAccessing, rcSelf, rcNull );
#define DISPATCH_BOOL(call)  \
    if ( self != NULL && self -> vt != NULL )   \
        return self -> vt -> call;              \
    else                                        \
        return false;

rc_t CC KMDataNodeWhack ( KMDataNode *self )
{
    if ( self == NULL ) return 0;
    DISPATCH( whack( self ) );
}

LIB_EXPORT rc_t CC KMDataNodeAddRef ( const KMDataNode *self )
{
    if ( self == NULL ) return 0;
    DISPATCH( addRef( self ) );
}
LIB_EXPORT rc_t CC KMDataNodeRelease ( const KMDataNode *self )
{
    if ( self == NULL ) return 0;
    DISPATCH( release( self ) );
}
LIB_EXPORT rc_t CC KMDataNodeByteOrder ( const KMDataNode *self, bool *reversed )
{
    DISPATCH( byteOrder( self, reversed ) );
}
LIB_EXPORT rc_t CC KMDataNodeRead ( const KMDataNode *self,size_t offset, void *buffer, size_t bsize, size_t *num_read, size_t *remaining )
{
    DISPATCH( read( self, offset, buffer, bsize, num_read, remaining ) );
}
LIB_EXPORT rc_t CC KMDataNodeVOpenNodeRead ( const KMDataNode *self, const KMDataNode **node, const char *path, va_list args )
{
    DISPATCH( openNodeRead( self, node, path, args ) );
}
LIB_EXPORT rc_t CC KMDataNodeOpenNodeRead ( const KMDataNode *self, const KMDataNode **node, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KMDataNodeVOpenNodeRead ( self, node, path, args );
    va_end ( args );

    return rc;
}

/* Read ( formatted )
 *  reads as integer or float value in native byte order
 *
 *  "bXX" [ OUT ] - return parameter for numeric value
 */
LIB_EXPORT rc_t CC KMDataNodeReadB8 ( const KMDataNode *self, void *b8 )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, b8, 1, & num_read, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
            return RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
        if ( num_read < 1 )
            return RC ( rcDB, rcMetadata, rcReading, rcTransfer, rcIncomplete );
    }
    return rc;
}
LIB_EXPORT rc_t CC KMDataNodeReadB16 ( const KMDataNode *self, void *b16 )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, b16, 2, & num_read, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
            return RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
        if ( num_read < 2 )
            return RC ( rcDB, rcMetadata, rcReading, rcTransfer, rcIncomplete );

        bool reversed;
        KMDataNodeByteOrder ( self, & reversed );
        if ( reversed )
            * ( uint16_t* ) b16 = bswap_16 ( * ( const uint16_t* ) b16 );
    }
    return rc;
}
LIB_EXPORT rc_t CC KMDataNodeReadB32 ( const KMDataNode *self, void *b32 )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, b32, 4, & num_read, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
            return RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
        if ( num_read < 4 )
            return RC ( rcDB, rcMetadata, rcReading, rcTransfer, rcIncomplete );

        bool reversed;
        KMDataNodeByteOrder ( self, & reversed );
        if ( reversed )
            * ( uint32_t* ) b32 = bswap_32 ( * ( const uint32_t* ) b32 );
    }
    return rc;
}
LIB_EXPORT rc_t CC KMDataNodeReadB64 ( const KMDataNode *self, void *b64 )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, b64, 8, & num_read, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
            return RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
        if ( num_read < 8 )
            return RC ( rcDB, rcMetadata, rcReading, rcTransfer, rcIncomplete );

        bool reversed;
        KMDataNodeByteOrder ( self, & reversed );
        if ( reversed )
            * ( uint64_t* ) b64 = bswap_64 ( * ( const uint64_t* ) b64 );
    }
    return rc;

}
LIB_EXPORT rc_t CC KMDataNodeReadB128 ( const KMDataNode *self, void *b128 )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, b128, 16,
        & num_read, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
            return RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
        if ( num_read < 16 )
            return RC ( rcDB, rcMetadata, rcReading, rcTransfer, rcIncomplete );

        bool reversed;
        KMDataNodeByteOrder ( self, & reversed );
        if ( reversed )
        {
            uint64_t *b64 = b128;
            uint64_t tmp = bswap_64 ( b64 [ 0 ] );
            b64 [ 0 ] = bswap_64 ( b64 [ 1 ] );
            b64 [ 1 ] = tmp;
        }
    }
    return rc;
}

/* ReadAs ( formatted )
 *  reads as integer or float value in native byte order
 *  casts smaller-sized values to desired size, e.g.
 *    uint32_t to uint64_t
 *
 *  "i" [ OUT ] - return parameter for signed integer
 *  "u" [ OUT ] - return parameter for unsigned integer
 *  "f" [ OUT ] - return parameter for double float
 */

LIB_EXPORT rc_t CC KMDataNodeReadAsI16 ( const KMDataNode *self, int16_t *i )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, i, sizeof * i,
        & num_read, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
            return RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );

        switch ( num_read )
        {
        case 1:
            * i = ( ( const int8_t* ) i ) [ 0 ];
            break;
        case 2:
        {
            bool reversed;
            KMDataNodeByteOrder ( self, & reversed );
            if ( reversed )
                * i = bswap_16 ( * i );
            break;
        }
        default:
            return RC ( rcDB, rcMetadata, rcReading, rcTransfer, rcIncomplete );
        }
    }
    return rc;

}
LIB_EXPORT rc_t CC KMDataNodeReadAsU16 ( const KMDataNode *self, uint16_t *u )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, u, sizeof * u,
        & num_read, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
            return RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );

        switch ( num_read )
        {
        case 1:
            * u = ( ( const uint8_t* ) u ) [ 0 ];
            break;
        case 2:
        {
            bool reversed;
            KMDataNodeByteOrder ( self, & reversed );
            if ( reversed )
                * u = bswap_16 ( * u );
            break;
        }
        default:
            return RC ( rcDB, rcMetadata, rcReading, rcTransfer, rcIncomplete );
        }
    }
    return rc;
}
LIB_EXPORT rc_t CC KMDataNodeReadAsI32 ( const KMDataNode *self, int32_t *i )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, i, sizeof * i,
        & num_read, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
            return RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );

        switch ( num_read )
        {
        case 1:
            * i = ( ( const int8_t* ) i ) [ 0 ];
            break;
        case 2:
        {
            bool reversed;
            KMDataNodeByteOrder ( self, & reversed );
            if ( reversed )
                * i = bswap_16 ( ( ( const int16_t* ) i ) [ 0 ] );
            else
                * i = ( ( const int16_t* ) i ) [ 0 ];
            break;
        }
        case 4:
        {
            bool reversed;
            KMDataNodeByteOrder ( self, & reversed );
            if ( reversed )
                * i = bswap_32 ( * i );
            break;
        }
        default:
            return RC ( rcDB, rcMetadata, rcReading, rcTransfer, rcIncomplete );
        }
    }
    return rc;
}
LIB_EXPORT rc_t CC KMDataNodeReadAsU32 ( const KMDataNode *self, uint32_t *u )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, u, sizeof * u,
        & num_read, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
            return RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );

        switch ( num_read )
        {
        case 1:
            * u = ( ( const uint8_t* ) u ) [ 0 ];
            break;
        case 2:
        {
            bool reversed;
            KMDataNodeByteOrder ( self, & reversed );
            if ( reversed )
                * u = bswap_16 ( ( ( const uint16_t* ) u ) [ 0 ] );
            else
                * u = ( ( const uint16_t* ) u ) [ 0 ];
            break;
        }
        case 4:
        {
            bool reversed;
            KMDataNodeByteOrder ( self, & reversed );
            if ( reversed )
                * u = bswap_32 ( * u );
            break;
        }
        default:
            return RC ( rcDB, rcMetadata, rcReading, rcTransfer, rcIncomplete );
        }
    }
    return rc;
}
LIB_EXPORT rc_t CC KMDataNodeReadAsI64 ( const KMDataNode *self, int64_t *i )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, i, sizeof * i,
        & num_read, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
            return RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );

        switch ( num_read )
        {
        case 1:
            * i = ( ( const int8_t* ) i ) [ 0 ];
            break;
        case 2:
        {
            bool reversed;
            KMDataNodeByteOrder ( self, & reversed );
            if ( reversed )
                * i = bswap_16 ( ( ( const int16_t* ) i ) [ 0 ] );
            else
                * i = ( ( const int16_t* ) i ) [ 0 ];
            break;
        }
        case 4:
        {
            bool reversed;
            KMDataNodeByteOrder ( self, & reversed );
            if ( reversed )
                * i = bswap_32 ( ( ( const int32_t* ) i ) [ 0 ] );
            else
                * i = ( ( const int32_t* ) i ) [ 0 ];
            break;
        }
        case 8:
        {
            bool reversed;
            KMDataNodeByteOrder ( self, & reversed );
            if ( reversed )
                * i = bswap_64 ( * i );
            break;
        }
        default:
            return RC ( rcDB, rcMetadata, rcReading, rcTransfer, rcIncomplete );
        }
    }
    return rc;

}
LIB_EXPORT rc_t CC KMDataNodeReadAsU64 ( const KMDataNode *self, uint64_t *u )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, u, sizeof * u,
        & num_read, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
            return RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );

        switch ( num_read )
        {
        case 1:
            * u = ( ( const uint8_t* ) u ) [ 0 ];
            break;
        case 2:
        {
            bool reversed;
            KMDataNodeByteOrder ( self, & reversed );
            if ( reversed )
                * u = bswap_16 ( ( ( const uint16_t* ) u ) [ 0 ] );
            else
                * u = ( ( const uint16_t* ) u ) [ 0 ];
            break;
        }
        case 4:
        {
            bool reversed;
            KMDataNodeByteOrder ( self, & reversed );
            if ( reversed )
                * u = bswap_32 ( ( ( const uint32_t* ) u ) [ 0 ] );
            else
                * u = ( ( const uint32_t* ) u ) [ 0 ];
            break;
        }
        case 8:
        {
            bool reversed;
            KMDataNodeByteOrder ( self, & reversed );
            if ( reversed )
                * u = bswap_64 ( * u );
            break;
        }
        default:
            return RC ( rcDB, rcMetadata, rcReading, rcTransfer, rcIncomplete );
        }
    }
    return rc;
}
LIB_EXPORT rc_t CC KMDataNodeReadAsF64 ( const KMDataNode *self, double *f )
{
    size_t num_read, remaining;
    rc_t rc = KMDataNodeRead ( self, 0, f, sizeof * f,
        & num_read, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
            return RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );

        switch ( num_read )
        {
        case 4:
        {
            bool reversed;
            KMDataNodeByteOrder ( self, & reversed );
            if ( reversed )
                * ( uint32_t* ) f = bswap_32 ( * ( const uint32_t* ) f );
            * f = ( ( const float* ) f ) [ 0 ];
            break;
        }
        case 8:
        {
            bool reversed;
            KMDataNodeByteOrder ( self, & reversed );
            if ( reversed )
                * ( uint64_t* ) f = bswap_64 ( * ( const uint64_t* ) f );
            break;
        }
        default:
            return RC ( rcDB, rcMetadata, rcReading, rcTransfer, rcIncomplete );
        }
    }
    return rc;
}

/* Read ( formatted )
 *  reads as C-string
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - output buffer for
 *  NUL terminated string.
 *
 *  "size" [ OUT ] - return parameter giving size of string
 *  not including NUL byte. the size is set both upon success
 *  and insufficient buffer space error.
 */
LIB_EXPORT rc_t CC KMDataNodeReadCString ( const KMDataNode *self, char *buffer, size_t bsize, size_t *size )
{
    size_t remaining;
    rc_t rc = KMDataNodeRead ( self, 0, buffer, bsize - 1, size, & remaining );
    if ( rc == 0 )
    {
        if ( remaining != 0 )
        {
            * size += remaining;
            return RC ( rcDB, rcMetadata, rcReading, rcBuffer, rcInsufficient );
        }

        buffer [ * size ] = 0;
    }
    return rc;
}

LIB_EXPORT rc_t CC KMDataNodeReadAttr ( const KMDataNode *self, const char *name, char *buffer, size_t bsize, size_t *size )
{
    DISPATCH( readAttr( self, name, buffer, bsize, size ) );
}
/* ReadAttrAs ( formatted )
 *  reads as integer or float value in native byte order
 *  casts smaller-sized values to desired size, e.g.
 *    uint32_t to uint64_t
 *
 *  "i" [ OUT ] - return parameter for signed integer
 *  "u" [ OUT ] - return parameter for unsigned integer
 *  "f" [ OUT ] - return parameter for double float
 */

LIB_EXPORT rc_t CC KMDataNodeReadAttrAsI16 ( const KMDataNode *self, const char *attr, int16_t *i )
{
    rc_t rc;
    if ( i == NULL )
        rc = RC ( rcDB, rcMetadata, rcReading, rcParam, rcNull );
    else
    {
        size_t size;
        char buffer [ 256 ];
        rc = KMDataNodeReadAttr ( self, attr, buffer, sizeof buffer, & size );
        if ( rc == 0 )
        {
            char *end;
            int64_t val = strtoi64 ( buffer, & end, 0 );
            if ( end [ 0 ] != 0 )
                rc = RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
            else if ( val < SHRT_MIN || val > SHRT_MAX )
                rc = RC ( rcDB, rcMetadata, rcReading, rcRange, rcExcessive );
            else
            {
                * i = ( int16_t ) val;
                return 0;
            }
        }

        * i = 0;
    }
    return rc;
}
LIB_EXPORT rc_t CC KMDataNodeReadAttrAsU16 ( const KMDataNode *self, const char *attr, uint16_t *u )
{
    rc_t rc;
    if ( u == NULL )
        rc = RC ( rcDB, rcMetadata, rcReading, rcParam, rcNull );
    else
    {
        size_t size;
        char buffer [ 256 ];
        rc = KMDataNodeReadAttr ( self, attr, buffer, sizeof buffer, & size );
        if ( rc == 0 )
        {
            char *end;
            uint64_t val = strtou64 ( buffer, & end, 0 );
            if ( end [ 0 ] != 0 )
                rc = RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
            else if ( val > USHRT_MAX )
                rc = RC ( rcDB, rcMetadata, rcReading, rcRange, rcExcessive );
            else
            {
                * u = ( uint16_t ) val;
                return 0;
            }
        }

        * u = 0;
    }
    return rc;
}
LIB_EXPORT rc_t CC KMDataNodeReadAttrAsI32 ( const KMDataNode *self, const char *attr, int32_t *i )
{
    rc_t rc;
    if ( i == NULL )
        rc = RC ( rcDB, rcMetadata, rcReading, rcParam, rcNull );
    else
    {
        size_t size;
        char buffer [ 256 ];
        rc = KMDataNodeReadAttr ( self, attr, buffer, sizeof buffer, & size );
        if ( rc == 0 )
        {
            char *end;
            int64_t val = strtoi64 ( buffer, & end, 0 );
            if ( end [ 0 ] != 0 )
                rc = RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
            else if ( val < INT_MIN || val > INT_MAX )
                rc = RC ( rcDB, rcMetadata, rcReading, rcRange, rcExcessive );
            else
            {
                * i = ( int32_t ) val;
                return 0;
            }
        }

        * i = 0;
    }
    return rc;
}
LIB_EXPORT rc_t CC KMDataNodeReadAttrAsU32 ( const KMDataNode *self, const char *attr, uint32_t *u )
{
    rc_t rc;
    if ( u == NULL )
        rc = RC ( rcDB, rcMetadata, rcReading, rcParam, rcNull );
    else
    {
        size_t size;
        char buffer [ 256 ];
        rc = KMDataNodeReadAttr ( self, attr, buffer, sizeof buffer, & size );
        if ( rc == 0 )
        {
            char *end;
            uint64_t val = strtou64 ( buffer, & end, 0 );
            if ( end [ 0 ] != 0 )
                rc = RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
            else if ( val > UINT_MAX )
                rc = RC ( rcDB, rcMetadata, rcReading, rcRange, rcExcessive );
            else
            {
                * u = ( uint32_t ) val;
                return 0;
            }
        }

        * u = 0;
    }
    return rc;
}
LIB_EXPORT rc_t CC KMDataNodeReadAttrAsI64 ( const KMDataNode *self, const char *attr, int64_t *i )
{
    rc_t rc;
    if ( i == NULL )
        rc = RC ( rcDB, rcMetadata, rcReading, rcParam, rcNull );
    else
    {
        size_t size;
        char buffer [ 256 ];
        rc = KMDataNodeReadAttr ( self, attr, buffer, sizeof buffer, & size );
        if ( rc == 0 )
        {
            char *end;
            int64_t val =  strtoi64 ( buffer, & end, 0 );
            if ( end [ 0 ] != 0 )
                rc = RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
            else
            {
                * i = val;
                return 0;
            }
        }

        * i = 0;
    }
    return rc;
}
LIB_EXPORT rc_t CC KMDataNodeReadAttrAsU64 ( const KMDataNode *self, const char *attr, uint64_t *u )
{
    rc_t rc;
    if ( u == NULL )
        rc = RC ( rcDB, rcMetadata, rcReading, rcParam, rcNull );
    else
    {
        size_t size;
        char buffer [ 256 ];
        rc = KMDataNodeReadAttr ( self, attr, buffer, sizeof buffer, & size );
        if ( rc == 0 )
        {
            char *end;
            uint64_t val =  strtou64 ( buffer, & end, 0 );
            if ( end [ 0 ] != 0 )
                rc = RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
            else
            {
                * u = val;
                return 0;
            }
        }

        * u = 0;
    }
    return rc;
}
LIB_EXPORT rc_t CC KMDataNodeReadAttrAsF64 ( const KMDataNode *self, const char *attr, double *f )
{
    rc_t rc;
    if ( f == NULL )
        rc = RC ( rcDB, rcMetadata, rcReading, rcParam, rcNull );
    else
    {
        size_t size;
        char buffer [ 256 ];
        rc = KMDataNodeReadAttr ( self, attr, buffer, sizeof buffer, & size );
        if ( rc == 0 )
        {
            char *end;
            double val = strtod ( buffer, & end );
            if ( end [ 0 ] != 0 )
                rc = RC ( rcDB, rcMetadata, rcReading, rcNode, rcIncorrect );
            else
            {
                * f = val;
                return 0;
            }
        }

        * f = 0.0;
    }
    return rc;
}
LIB_EXPORT rc_t CC KMDataNodeCompare( const KMDataNode *self, const KMDataNode *other, bool *equal )
{
    DISPATCH( compare( self, other, equal ) );
}
LIB_EXPORT rc_t CC KMDataNodeAddr ( const KMDataNode *self, const void **addr, size_t *size )
{
    DISPATCH( addr( self, addr, size ) );
}
LIB_EXPORT rc_t CC KMDataNodeListAttr ( const KMDataNode *self, struct KNamelist **names )
{
    DISPATCH( listAttr( self, names ) );
}
LIB_EXPORT rc_t CC KMDataNodeListChildren ( const KMDataNode *self, struct KNamelist **names )
{
    DISPATCH( listChildren( self, names ) );
}
