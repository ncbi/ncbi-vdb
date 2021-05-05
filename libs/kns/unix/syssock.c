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


/*--------------------------------------------------------------------------
 * forwards
 */
#define KSTREAM_IMPL KSocket

#include <kns/extern.h>
#include <kns/manager.h>
#include <kns/socket.h>
#include <kns/impl.h>
#include <kns/endpoint.h>

#ifdef ERR
#undef ERR
#endif

#include <klib/debug.h>
#include <klib/log.h>
#include <klib/out.h>
#include <klib/printf.h>
#include <klib/rc.h>
#include <klib/text.h>
#include <klib/time.h>

#include <kproc/timeout.h>

#include "mgr-priv.h"
#include "stream-priv.h"
#include "poll-priv.h"

#include <sysalloc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <os-native.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pwd.h>

#ifndef POLLRDHUP
#define POLLRDHUP 0
#endif


/*--------------------------------------------------------------------------
 * KSocket
 *  a socket IS a stream
 *
 *  in Berkeley socket terminology, a STREAM implies a CONTINUOUS stream,
 *  which is implemented by the TCP connection. A "chunked" or discontiguous
 *  stream would be a datagram stream, implemented usually by UDP.
 *
 *  in VDB terminology, a STREAM is a fluid, moving target that is observed
 *  from a stationary point, whereas a FILE or OBJECT is a static stationary
 *  target observed from a movable window. This means that a STREAM cannot be
 *  addressed randomly, whereas a FILE or OBJECT can.
 */
struct KSocket
{
    KStream dad;
    const char * path;
    uint32_t type;
    int32_t read_timeout;
    int32_t write_timeout;

    int fd;
    union
    {
        struct sockaddr_in  v4;     /* for ipv4 */
        struct sockaddr_in6 v6;     /* for ipv6 */
    } remote_addr;
    bool remote_addr_valid;

    char ip_address [ 256 ];
};

LIB_EXPORT rc_t CC KSocketAddRef( const KSocket *self )
{
    return KStreamAddRef ( & self -> dad );
}

LIB_EXPORT rc_t CC KSocketRelease ( const KSocket *self )
{
    return KStreamRelease ( & self -> dad );
}

/* GetStream
 */
LIB_EXPORT rc_t CC KSocketGetStream ( const KSocket * self, KStream ** s )
{
    rc_t rc;

    if ( s == NULL )
        rc = RC ( rcNS, rcSocket, rcOpening, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcNS, rcSocket, rcOpening, rcSelf, rcNull );
        else
        {
            rc = KSocketAddRef ( self );
            if ( rc == 0 )
            {
                * s = & ( ( KSocket* ) self ) -> dad;
                return 0;
            }
        }

        * s = NULL;
    }

    return rc;
}


static
rc_t CC KSocketWhack ( KSocket *self )
{
    assert ( self != NULL );

    shutdown ( self -> fd, SHUT_WR );

    while ( 1 )
    {
        char buffer [ 1024 ];
        ssize_t result = recv ( self -> fd, buffer, sizeof buffer, MSG_DONTWAIT );
        if ( result <= 0 )
            break;
    }

    shutdown ( self -> fd, SHUT_RD );

    close ( self -> fd );

    if ( self -> path != NULL )
    {
        unlink ( self -> path );
        free ( ( void* ) self -> path );
    }

    KStreamWhack ( & self -> dad, "KSocket" );
    free ( self );

    return 0;
}

static
rc_t KSocketHandleSocknameCall ( int status )
{
    switch ( status )
    {
    case EBADF:
        return RC ( rcNS, rcSocket, rcIdentifying, rcSocket, rcInvalid );
    case EFAULT:
    case EINVAL:
        return RC ( rcNS, rcSocket, rcIdentifying, rcParam, rcInvalid );
    case ENOBUFS:
        return RC ( rcNS, rcSocket, rcIdentifying, rcBuffer, rcExhausted );
    case ENOTCONN:
        return RC ( rcNS, rcSocket, rcIdentifying, rcSocket, rcNotOpen );
    case ENOTSOCK:
        return RC ( rcNS, rcSocket, rcIdentifying, rcSocket, rcIncorrect );
    }

    return RC ( rcNS, rcSocket, rcIdentifying, rcError, rcUnknown );
}

static
rc_t KSocketHandleSocketCall ( int status )
{
    switch ( status )
    {
    case EACCES:
        return RC ( rcNS, rcSocket, rcCreating, rcSocket, rcUnauthorized );
    case EAFNOSUPPORT:
    case EPROTONOSUPPORT:
        return RC ( rcNS, rcSocket, rcCreating, rcInterface, rcUnsupported );
    case EINVAL:
        return RC ( rcNS, rcSocket, rcCreating, rcInterface, rcUnrecognized );
    case EMFILE:
    case ENFILE:
        return RC ( rcNS, rcSocket, rcCreating, rcFileDesc, rcExhausted );
    case ENOBUFS:
        return RC ( rcNS, rcSocket, rcCreating, rcBuffer, rcExhausted );
    case ENOMEM:
        return RC ( rcNS, rcSocket, rcCreating, rcMemory, rcExhausted );
    }

    return RC ( rcNS, rcSocket, rcCreating, rcError, rcUnknown );
}

static
rc_t KSocketHandleBindCall ( int status )
{
    switch ( status )
    {
    case EACCES:
        return RC ( rcNS, rcSocket, rcCreating, rcSocket, rcUnauthorized );
    case EADDRINUSE:
        return RC ( rcNS, rcSocket, rcCreating, rcSocket, rcBusy );
    case EADDRNOTAVAIL:
        return RC ( rcNS, rcSocket, rcCreating, rcSocket, rcNotAvailable);
    case EBADF:
        return RC ( rcNS, rcSocket, rcCreating, rcSocket, rcInvalid );
    case EFAULT:
    case EINVAL:
        return RC ( rcNS, rcSocket, rcCreating, rcParam, rcInvalid );
    case ELOOP:
        return RC ( rcNS, rcSocket, rcCreating, rcError, rcCorrupt );
    case ENAMETOOLONG:
        return RC ( rcNS, rcSocket, rcCreating, rcName, rcExcessive );
    case ENOENT:
        return RC ( rcNS, rcSocket, rcCreating, rcFile, rcNotFound );
    case ENOMEM:
        return RC ( rcNS, rcSocket, rcCreating, rcMemory, rcExhausted );
    case ENOTDIR:
        return RC ( rcNS, rcSocket, rcCreating, rcName, rcIncorrect );
    case ENOTSOCK:
        return RC ( rcNS, rcSocket, rcCreating, rcSocket, rcIncorrect );
    case EROFS:
        return RC ( rcNS, rcSocket, rcCreating, rcDirectory, rcReadonly );
    }

    return RC ( rcNS, rcSocket, rcCreating, rcError, rcUnknown );
}

static
rc_t KSocketHandleConnectCall ( int status )
{
    switch ( status )
    {
    case EACCES:
    case EPERM:
        return RC ( rcNS, rcSocket, rcCreating, rcSocket, rcUnauthorized );
    case EADDRINUSE:
        return RC ( rcNS, rcSocket, rcCreating, rcSocket, rcBusy );
    case EADDRNOTAVAIL:
        return RC ( rcNS, rcSocket, rcCreating, rcSocket, rcNotAvailable );
    case EAFNOSUPPORT:
        return RC ( rcNS, rcSocket, rcCreating, rcInterface, rcUnsupported );
    case EALREADY:
        return RC ( rcNS, rcSocket, rcCreating, rcSocket, rcBusy );
    case EBADF:
        return RC ( rcNS, rcSocket, rcCreating, rcSocket, rcInvalid );
    case ECONNREFUSED:
        return RC ( rcNS, rcSocket, rcCreating, rcConnection, rcFailed );
    case EFAULT:
        return RC ( rcNS, rcSocket, rcCreating, rcParam, rcInvalid );
    case EINPROGRESS:
        return RC ( rcNS, rcSocket, rcCreating, rcConnection, rcBusy );
    case EINTR:
        return 0;
    case EISCONN:
        return RC ( rcNS, rcSocket, rcCreating, rcSocket, rcBusy );
    case ENETUNREACH:
        return RC ( rcNS, rcSocket, rcCreating, rcConnection, rcNotAvailable );
    case ENOTSOCK:
        return RC ( rcNS, rcSocket, rcCreating, rcSocket, rcIncorrect );
    case ETIMEDOUT:
        return RC ( rcNS, rcSocket, rcCreating, rcTimeout, rcExhausted );
    }

    return RC ( rcNS, rcSocket, rcCreating, rcError, rcUnknown );
}

static
rc_t KSocketHandleAcceptCall ( int status )
{
    switch ( status )
    {
    case EWOULDBLOCK:
        return RC ( rcNS, rcSocket, rcOpening, rcData, rcNotAvailable );
    case EBADF:
        return RC ( rcNS, rcSocket, rcOpening, rcSocket, rcInvalid );
    case ECONNABORTED:
        return RC ( rcNS, rcSocket, rcOpening, rcConnection, rcCanceled );
    case EFAULT:
    case EINVAL:
        return RC ( rcNS, rcSocket, rcOpening, rcData, rcNotAvailable );
    case EINTR:
        return 0;
    case EMFILE:
    case ENFILE:
        return RC ( rcNS, rcSocket, rcOpening, rcFileDesc, rcExhausted );
    case ENOBUFS:
        return RC ( rcNS, rcSocket, rcOpening, rcBuffer, rcExhausted );
    case ENOMEM:
        return RC ( rcNS, rcSocket, rcOpening, rcMemory, rcExhausted );
    case ENOTSOCK:
        return RC ( rcNS, rcSocket, rcOpening, rcSocket, rcIncorrect );
    case EOPNOTSUPP:
        return RC ( rcNS, rcSocket, rcOpening, rcSocket, rcUnsupported );
    case EPROTO:
    case EPERM: /* LINUX ONLY */
        return RC ( rcNS, rcSocket, rcOpening, rcConstraint, rcViolated );
    }

    return RC ( rcNS, rcSocket, rcOpening, rcError, rcUnknown );
}




static rc_t KSocketGetEndpointV6 ( const KSocket * self, KEndPoint * ep, bool remote )
{
    rc_t rc = 0;
    struct sockaddr_in6 addr;
    socklen_t l = sizeof addr;
    int res = 0;

    if ( ! remote )
        res = getsockname( self -> fd, ( struct sockaddr * )&addr, &l );
    else if ( ! self -> remote_addr_valid )
        res = getpeername( self -> fd, ( struct sockaddr * )&addr, &l );
    else
    {
        /* the remote part was already recorded through calling accept() */
        memmove ( ep -> u . ipv6 . addr,
                 self -> remote_addr . v6 . sin6_addr . s6_addr,
                 sizeof ( ep -> u . ipv6 . addr ) );
        ep->u.ipv6.port = ntohs( self -> remote_addr . v6 . sin6_port );
        ep->type = epIPV6;
        return 0;
    }

    if ( res == 0 )
    {
        memmove ( ep -> u . ipv6 . addr,
                 addr . sin6_addr . s6_addr,
                 sizeof ( ep -> u . ipv6 . addr ) );
        ep->u.ipv6.port = ntohs( addr . sin6_port );
        ep->type = epIPV6;
        return 0;
    }

    rc = KSocketHandleSocknameCall ( errno );

    ep -> type = epInvalid;

    return rc;
}


static rc_t KSocketGetEndpointV4 ( const KSocket * self, KEndPoint * ep, bool remote )
{
    rc_t rc = 0;
    struct sockaddr_in addr;
    socklen_t l = sizeof addr;
    int res = 0;

    if ( ! remote )
        res = getsockname( self -> fd, ( struct sockaddr * )&addr, &l );
    else if ( ! self -> remote_addr_valid )
        res = getpeername( self -> fd, ( struct sockaddr * )&addr, &l );
    else
    {
        /* the remote part was already recorded through calling accept() */
        addr.sin_addr.s_addr = self -> remote_addr.v4.sin_addr.s_addr;
        addr.sin_port        = self -> remote_addr.v4.sin_port;
    }

    if ( res == 0 )
    {
        ep->u.ipv4.addr = ntohl( addr.sin_addr.s_addr );
        ep->u.ipv4.port = ntohs( addr.sin_port );
        ep->type = epIPV4;
        string_copy_measure ( ep -> ip_address, sizeof ep -> ip_address,
                              inet_ntoa ( addr . sin_addr ) );
        return 0;
    }

    rc = KSocketHandleSocknameCall ( errno );

    ep -> type = epInvalid;

    return rc;
}


static rc_t KSocketGetEndpoint ( const KSocket * self, KEndPoint * ep, bool remote )
{
    rc_t rc = 0;
    if ( ep == NULL )
        rc = RC ( rcNS, rcSocket, rcEvaluating, rcParam, rcNull );
    else
    {
        memset ( ep, 0, sizeof * ep );

        ep -> type = epInvalid;

        if ( self == NULL )
            rc = RC ( rcNS, rcSocket, rcEvaluating, rcSelf, rcNull );
        else
        {
            switch( self->type )
            {
            case epIPV4:
                rc = KSocketGetEndpointV4( self, ep, remote );
                break;
            case epIPV6:
                rc = KSocketGetEndpointV6( self, ep, remote );
                break;
            default:
                rc = RC ( rcNS, rcSocket, rcEvaluating, rcFunction, rcUnsupported );
                break;
            }
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC KSocketGetRemoteEndpoint ( const KSocket * self, KEndPoint * ep )
{
    return KSocketGetEndpoint ( self, ep, true );
}


LIB_EXPORT rc_t CC KSocketGetLocalEndpoint ( const KSocket * self, KEndPoint * ep )
{
    return KSocketGetEndpoint ( self, ep, false );
}


static
rc_t CC KSocketTimedRead ( const KSocket *self,
    void *buffer, size_t bsize, size_t *num_read, timeout_t *tm )
{
    rc_t rc;
    int revents;

    assert ( self != NULL );
    assert ( num_read != NULL );

    DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketTimedRead(%d, %d)...\n", self, bsize, tm == NULL ? -1 : tm -> mS ) );

    /* wait for socket to become readable */
    revents = socket_wait ( self -> fd
                            , POLLIN
                            | POLLRDNORM
                            | POLLRDBAND
                            | POLLPRI
                            | POLLRDHUP
                            , tm );

    /* check for error */
    if ( revents < 0 )
    {
        switch ( errno )
        {
        case EFAULT:
        case EINVAL:
            rc = RC ( rcNS, rcSocket, rcReading, rcParam, rcInvalid );
            break;
        case EINTR:
            rc = RC ( rcNS, rcSocket, rcReading, rcTransfer, rcInterrupted );
            break;
        case ENOMEM:
            rc = RC ( rcNS, rcSocket, rcReading, rcMemory, rcExhausted );
            break;
        default:
            rc = RC ( rcNS, rcSocket, rcReading, rcError, rcUnknown );
            break;
        }

        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketTimedRead socket_wait returned '%!'\n", self, errno ) );
        return rc;
    }

    /* no error in errno - check for other potential errors */
    if ( ( revents & ( POLLERR | POLLNVAL ) ) != 0 )
    {
        if ( ( revents & POLLERR ) != 0)
        {
            int optval = 0;
            socklen_t optlen = sizeof optval;
            if ( ( getsockopt ( self -> fd, SOL_SOCKET, SO_ERROR, & optval, & optlen ) == 0 ) && optval > 0 )
            {
                errno = optval;
                DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketTimedRead socket_wait/getsockopt returned '%!'\n",
                                                            self, optval ) );
                switch ( errno )
                {
                case EFAULT:
                case EINVAL:
                    rc = RC ( rcNS, rcSocket, rcReading, rcParam, rcInvalid );
                    break;
                case EINTR:
                    rc = RC ( rcNS, rcSocket, rcReading, rcTransfer, rcInterrupted );
                    break;
                case ENOMEM:
                    rc = RC ( rcNS, rcSocket, rcReading, rcMemory, rcExhausted );
                    break;
                default:
                    rc = RC ( rcNS, rcSocket, rcReading, rcError, rcUnknown );
                    break;
                }

                return rc;
            }
        }

        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketTimedRead socket_wait returned POLLERR | POLLNVAL\n", self ) );
        return RC ( rcNS, rcSocket, rcReading, rcError, rcUnknown );
    }

    /* check for read availability */
    if ( ( revents & ( POLLRDNORM | POLLRDBAND ) ) != 0 )
    {
        ssize_t count = recv ( self -> fd, buffer, bsize, 0 );
        if ( count >= 0 )
        {
            * num_read = count;
            return 0;
        }

        switch ( errno )
        {
        case EWOULDBLOCK:
            rc = RC ( rcNS, rcSocket, rcReading, rcData, rcNotAvailable );
            break;
        case EBADF:
            rc = RC ( rcNS, rcSocket, rcReading, rcSocket, rcInvalid );
            break;
        case ECONNREFUSED:
            rc = RC ( rcNS, rcSocket, rcReading, rcConnection, rcFailed );
            break;
        case EFAULT:
        case EINVAL:
            rc = RC ( rcNS, rcSocket, rcReading, rcParam, rcInvalid );
            break;
        case EINTR:
            rc = RC ( rcNS, rcSocket, rcReading, rcTransfer, rcInterrupted );
            break;
        case ENOMEM:
            rc = RC ( rcNS, rcSocket, rcReading, rcMemory, rcExhausted );
            break;
        case ENOTCONN:
            rc = RC ( rcNS, rcSocket, rcReading, rcConnection, rcInvalid );
            break;
        case ENOTSOCK:
            rc = RC ( rcNS, rcSocket, rcReading, rcSocket, rcIncorrect );
            break;
        default:
            rc = RC ( rcNS, rcSocket, rcReading, rcError, rcUnknown );
            break;
        }

        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketTimedRead recv returned count %d\n", self, count ) );
        return rc;
    }

    /* check for broken connection */
    if ( ( revents & ( POLLHUP | POLLRDHUP ) ) != 0 )
    {
        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketTimedRead broken connection\n", self ) );
        * num_read = 0;
        return 0;
    }

    /* anything else in revents is an error */
    if ( ( revents & ~ POLLIN ) != 0 && errno != 0 )
    {
        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketTimedRead error '%!'\n", self, errno ) );
        return RC ( rcNS, rcSocket, rcReading, rcError, rcUnknown );
    }

    /* finally, call this a timeout */
    DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketTimedRead timeout\n", self ) );
    return RC ( rcNS, rcStream, rcReading, rcTimeout, rcExhausted );
}

static
rc_t CC KSocketRead ( const KSocket *self,
    void *buffer, size_t bsize, size_t *num_read )
{
    timeout_t tm;
    assert ( self != NULL );

    if ( self -> read_timeout < 0 )
        return KSocketTimedRead ( self, buffer, bsize, num_read, NULL );

    TimeoutInit ( & tm, self -> read_timeout );
    return KSocketTimedRead ( self, buffer, bsize, num_read, & tm );
}

static
rc_t CC KSocketTimedWrite ( KSocket *self,
    const void *buffer, size_t bsize, size_t *num_writ, timeout_t *tm )
{
    rc_t rc;
    int revents;
    ssize_t count;

    assert ( self != NULL );
    assert ( buffer != NULL );
    assert ( bsize != 0 );
    assert ( num_writ != NULL );

    DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketTimedWrite(%d, %d)...\n", self, bsize, tm == NULL ? -1 : tm -> mS ) );

    /* wait for socket to become writable */
    revents = socket_wait ( self -> fd
                            , POLLOUT
                            | POLLWRNORM
                            | POLLWRBAND
                            , tm );

    /* check for error */
    if ( revents < 0 )
    {
        switch ( errno )
        {
        case EFAULT:
        case EINVAL:
            rc = RC ( rcNS, rcSocket, rcWriting, rcParam, rcInvalid );
            break;
        case EINTR:
            rc = RC ( rcNS, rcSocket, rcWriting, rcTransfer, rcInterrupted );
            break;
        case ENOMEM:
            rc = RC ( rcNS, rcSocket, rcWriting, rcMemory, rcExhausted );
            break;
        default:
            rc = RC ( rcNS, rcSocket, rcWriting, rcError, rcUnknown );
            break;
        }

        assert ( rc != 0 );
        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketTimedWrite socket_wait returned '%s'\n", self, strerror ( errno ) ) );
        return rc;
    }

    if ( ( revents & ( POLLERR | POLLNVAL ) ) != 0 )
    {
        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketTimedWrite socket_wait returned POLLERR | POLLNVAL\n", self ) );
        return RC ( rcNS, rcStream, rcWriting, rcNoObj, rcUnknown );
    }

    /* check for broken connection */
    if ( ( revents & POLLHUP ) != 0 )
    {
        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: POLLHUP received\n", self ) );
        * num_writ = 0;
        return  RC ( rcNS, rcFile, rcWriting, rcTransfer, rcIncomplete );
    }

    /* check for ability to send */
    if ( ( revents & ( POLLWRNORM | POLLWRBAND ) ) != 0 )
    {
        count = send ( self -> fd, buffer, bsize, 0 );
        if ( count < 0 )
        {
            switch ( errno )
            {
            case EACCES:
                rc = RC ( rcNS, rcSocket, rcWriting, rcSocket, rcUnauthorized);
                break;
            case EWOULDBLOCK:
                rc = RC ( rcNS, rcSocket, rcWriting, rcData, rcNotAvailable );
                break;
            case EBADF:
                rc = RC ( rcNS, rcSocket, rcWriting, rcSocket, rcInvalid );
                break;
            case ECONNRESET:
                rc = RC ( rcNS, rcSocket, rcWriting, rcConnection, rcCanceled );
                break;
            case EINTR:
                rc = RC ( rcNS, rcSocket, rcWriting, rcTransfer, rcInterrupted );
                break;
            case EFAULT:
            case EINVAL:
                rc = RC ( rcNS, rcSocket, rcWriting, rcParam, rcInvalid );
                break;
            case EMSGSIZE:
                rc = RC ( rcNS, rcSocket, rcWriting, rcData, rcExcessive );
                break;
            case ENOBUFS:
                rc = RC ( rcNS, rcSocket, rcWriting, rcBuffer, rcExhausted );
                break;
            case ENOMEM:
                rc = RC ( rcNS, rcSocket, rcWriting, rcMemory, rcExhausted );
                break;
            case ENOTCONN:
                rc = RC ( rcNS, rcSocket, rcWriting, rcConnection, rcInvalid );
                break;
            case ENOTSOCK:
                rc = RC ( rcNS, rcSocket, rcWriting, rcSocket, rcIncorrect );
                break;
            case EOPNOTSUPP:
                rc = RC ( rcNS, rcSocket, rcWriting, rcParam, rcIncorrect );
                break;
            case EPIPE:
                rc = RC ( rcNS, rcSocket, rcWriting, rcConnection, rcInterrupted );
                break;
            default:
                rc = RC ( rcNS, rcSocket, rcWriting, rcError, rcUnknown );
                break;
            }

            DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketTimedWrite recv returned count %d\n", self, count ) );
            return rc;
        }

        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: %d bytes written\n", self, count ) );
        * num_writ = count;
        return 0;
    }

    /* anything else in revents is an error */
    if ( ( revents & ~ POLLOUT ) != 0 && errno != 0 )
    {
        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketTimedWrite error '%s'\n", self, strerror ( errno ) ) );
        return RC ( rcNS, rcSocket, rcWriting, rcError, rcUnknown );
    }

    /* finally, call this a timeout */
    DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketTimedWrite timeout\n", self ) );
    return RC ( rcNS, rcStream, rcWriting, rcTimeout, rcExhausted );
}

static
rc_t CC KSocketWrite ( KSocket *self,
    const void *buffer, size_t bsize, size_t *num_writ )
{
    timeout_t tm;
    assert ( self != NULL );

    if ( self -> write_timeout < 0 )
        return KSocketTimedWrite ( self, buffer, bsize, num_writ, NULL );

    TimeoutInit ( & tm, self -> write_timeout );
    return KSocketTimedWrite ( self, buffer, bsize, num_writ, & tm );
}

static KStream_vt_v1 vtKSocket =
{
    1, 1,
    KSocketWhack,
    KSocketRead,
    KSocketWrite,
    KSocketTimedRead,
    KSocketTimedWrite
};

/* for testing */
void KStreamForceSocketClose(const struct KStream *self) {
    assert(self);
    close(((KSocket*)self)->fd);
}

static
rc_t KSocketMakePath ( const char * name, char * buf, size_t buf_size )
{
    size_t num_writ;
#if 0
    rc_t rc;
    struct passwd* pwd;
    errno = 0; /* man page claims errno should be set to 0 before call if wanting to check after */
    pwd = getpwuid ( geteuid () );
    if ( pwd == NULL )
    {
        /* check the context 'rcAccessing' */
        switch ( errno )
        {
        case EBADF:
            rc = RC ( rcNS, rcFile, rcAccessing, rcFormat, rcInvalid );
            break;
        case EINTR:
            rc = RC ( rcNS, rcFile, rcAccessing, rcConnection, rcInterrupted );
            break;
        case EIO:
            rc = RC ( rcNS, rcFile, rcAccessing, rcNoObj, rcUndefined);
            break;
        case EMFILE:
        case ENFILE:
            rc = RC ( rcNS, rcFile, rcAccessing, rcFileDesc, rcExhausted );
            break;
        case ENOMEM:
            rc = RC ( rcNS, rcFile, rcAccessing, rcMemory, rcExhausted );
            break;
        case ERANGE:
            rc = RC ( rcNS, rcFile, rcAccessing, rcBuffer, rcInsufficient );
            break;
        default:
            rc = RC ( rcNS, rcFile, rcAccessing, rcError, rcUnknown );
            break;
        }

        return rc;
    }

    return string_printf ( buf, buf_size, & num_writ, "%s/.ncbi/%s", pwd -> pw_dir, name );
#else
    const char *HOME = getenv ( "HOME" );
    if ( HOME == NULL )
        return RC ( rcNS, rcProcess, rcAccessing, rcPath, rcNotFound );

    return string_printf ( buf, buf_size, & num_writ, "%s/.ncbi/%s", HOME, name );
#endif
}

static
rc_t
TimedConnect( int socketFd, const struct sockaddr* ss, size_t ss_size, int32_t timeoutMs )
{
    int res;
    /* set non-blocking mode */
    int flag = fcntl ( socketFd, F_GETFL );
    if ( flag == -1 )
    {
        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "TimedConnect(%d): fcntl(F_GETFL) failed\n", socketFd ) );
        return KSocketHandleConnectCall ( errno );
    }
    if ( fcntl ( socketFd, F_SETFL, flag | O_NONBLOCK ) == -1 )
    {
        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "TimedConnect(%d): fcnl(F_SETFL) failed\n", socketFd ) );
        return KSocketHandleConnectCall ( errno );
    }

    res = connect ( socketFd, ss, ss_size );
    if ( res == 0 )
    {
        return 0;
    }

    if ( errno == EINPROGRESS )
    {
        int res = connect_wait( socketFd, timeoutMs );
        if ( res > 0 )
        {
            return 0;
        }

        if ( res == 0 ) /* timed out */
        {
            DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "TimedConnect(%d): connect_wait() timed out\n", socketFd ) );
            return KSocketHandleConnectCall ( ETIMEDOUT );
        }

        if ( errno == EINTR )
        {
            DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "TimedConnect(%d): connect_wait() interrupted\n", socketFd ) );
            return RC ( rcNS, rcSocket, rcCreating, rcConnection, rcInterrupted );
        }
        else
        {
            DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "TimedConnect(%d): connect_wait() failed\n", socketFd ) );
            return KSocketHandleConnectCall ( errno );
        }
    }
    else
    {
        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "TimedConnect(%d): connect() failed\n", socketFd ) );
        return KSocketHandleConnectCall ( errno );
    }
}

static
rc_t KSocketConnectIPv4 ( KSocket *self, const KEndPoint *from, const KEndPoint *to, int32_t timeoutMs )
{
    rc_t rc = 0;

    /* create the OS socket */
    self -> fd = socket ( AF_INET, SOCK_STREAM, 0 );
    if ( self -> fd < 0 )
        rc = KSocketHandleSocketCall ( errno );
    else
    {
        /* disable nagle algorithm */
        int flag = 1;
        setsockopt ( self -> fd, IPPROTO_TCP, TCP_NODELAY, ( char* ) & flag, sizeof flag );

        struct sockaddr_in ss;
        memset ( & ss, 0, sizeof ss );
        ss . sin_family = AF_INET;
        if ( from != NULL )
        {
            ss . sin_addr . s_addr = htonl ( from -> u . ipv4 . addr );
            ss . sin_port = htons ( from -> u . ipv4 . port );
        }

        /* bind */
        if ( bind ( self -> fd, ( struct sockaddr* ) & ss, sizeof ss ) != 0 )
            rc = KSocketHandleBindCall ( errno );

        if ( rc == 0 )
        {
            ss . sin_port = htons ( to -> u . ipv4 . port );
            ss . sin_addr . s_addr = htonl ( to -> u . ipv4 . addr );

            rc = TimedConnect ( self -> fd, ( struct sockaddr* ) & ss, sizeof ss, timeoutMs );
            if ( rc == 0 )
            {
                string_copy_measure ( self -> ip_address,
                    sizeof self -> ip_address, to -> ip_address );

                return 0;
            }
        }

        /* dump socket */
        close ( self -> fd );
        self -> fd = -1;
    }

    DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketConnectIPv4 failed - %R\n", self, rc ) );

    return rc;
}


static
rc_t KSocketConnectIPv6 ( KSocket *self, const KEndPoint *from, const KEndPoint *to, int32_t timeoutMs )
{
    rc_t rc = 0;
    struct sockaddr_in6 ss_from, ss_to;

    memset ( & ss_from, 0, sizeof ss_from );
    if ( from != NULL )
    {
        ss_from . sin6_family = AF_INET6;
        memmove ( ss_from . sin6_addr . s6_addr,
                 from -> u . ipv6 . addr,
                 sizeof ( from -> u . ipv6 . addr ) );
        ss_from . sin6_port = htons ( from -> u . ipv6 . port );
    }

    memset ( & ss_to, 0, sizeof ss_to );
    ss_to . sin6_family = AF_INET6;
    memmove ( ss_to . sin6_addr . s6_addr,
             to -> u . ipv6 . addr,
             sizeof ( to -> u . ipv6 . addr ) );
    ss_to . sin6_port = htons ( to -> u . ipv6 . port );

    /* create the OS socket */
    self -> fd = socket ( AF_INET6, SOCK_STREAM, 0 );
    if ( self -> fd < 0 )
        rc = KSocketHandleSocketCall ( errno );
    else
    {
        /* disable nagle algorithm */
        int flag = 1;
        setsockopt ( self -> fd, IPPROTO_TCP, TCP_NODELAY, ( char* ) & flag, sizeof flag );

        /* bind */
        if ( from != NULL && bind ( self -> fd, ( struct sockaddr* ) & ss_from, sizeof ss_from ) != 0 )
            rc = KSocketHandleBindCall ( errno );

        if ( rc == 0 )
        {
            rc = TimedConnect ( self -> fd, ( struct sockaddr* ) & ss_to, sizeof ss_to, timeoutMs );
            if ( rc == 0 )
            {
                return 0;
            }
        }

        /* dump socket */
        close ( self -> fd );
        self -> fd = -1;
    }

    DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketConnectIPv6 failed - %R\n", self, rc ) );

    return rc;
}


static
rc_t KSocketConnectIPC ( KSocket *self, const KEndPoint *to )
{
    rc_t rc = 0;
    struct sockaddr_un ss_to;

    memset ( & ss_to, 0, sizeof ss_to );
    ss_to . sun_family = AF_UNIX;
    rc = KSocketMakePath ( to -> u . ipc_name, ss_to . sun_path, sizeof ss_to . sun_path );

    /* create the OS socket */
    self -> fd = socket ( AF_UNIX, SOCK_STREAM, 0 );
    if ( self -> fd < 0 )
        rc = KSocketHandleSocketCall ( errno );
    else
    {
        /* connect */
        if ( connect ( self -> fd, ( struct sockaddr* ) & ss_to, sizeof ss_to ) != 0 )
            rc = KSocketHandleConnectCall ( errno );
        else
            return 0;

        /* dump socket */
        close ( self -> fd );
        self -> fd = -1;
    }

    DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketConnectIPC failed - %R\n", self, rc ) );

    return rc;
 }

KNS_EXTERN rc_t CC KNSManagerMakeRetryTimedConnection ( struct KNSManager const * self,
    struct KSocket ** out, timeout_t * retryTimeout, int32_t readMillis, int32_t writeMillis,
    struct KEndPoint const * from, struct KEndPoint const * to )
{
    rc_t rc;

    if ( out == NULL )
        rc = RC ( rcNS, rcStream, rcConstructing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcNS, rcStream, rcConstructing, rcSelf, rcNull );
        else if ( retryTimeout == NULL || to == NULL )
            rc = RC ( rcNS, rcStream, rcConstructing, rcParam, rcNull );
        else if ( from != NULL && from -> type != to -> type )
            rc = RC ( rcNS, rcStream, rcConstructing, rcParam, rcIncorrect );
        else
        {
            KSocket *conn = calloc ( 1, sizeof * conn );
            if ( conn == NULL )
                rc = RC ( rcNS, rcStream, rcConstructing, rcMemory, rcExhausted );
            else
            {
                conn -> fd = -1;
                conn -> read_timeout = readMillis;
                conn -> write_timeout = writeMillis;

                rc = KStreamInit ( & conn -> dad, ( const KStream_vt* ) & vtKSocket,
                    "KSocket - RetryTimedConnection", to -> ip_address, true, true );
                if ( rc == 0 )
                {
                    /* prepare the timeout */
                    rc = TimeoutPrepare ( retryTimeout );
                    if ( rc == 0 )
                    {
                        uint32_t retry;

                        switch ( to -> type )
                        {

                            /* ensure the type is correct */
                        case epIPV4:
                        case epIPV6:
                        case epIPC:

                            /* a retry loop - retry upon a schedule for the alloted time */
                            for ( retry = 0; ; ++ retry )
                            {
                                uint32_t remaining, delay;

                                /* try to connect using appropriate protocol */
                                conn -> type = to -> type;
                                switch ( to -> type )
                                {
                                case epIPV4:
                                    rc = KSocketConnectIPv4 ( conn, from, to, retryTimeout -> mS );
                                    break;

                                case epIPV6:
                                    rc = KSocketConnectIPv6 ( conn, from, to, retryTimeout -> mS );
                                    break;

                                case epIPC:
                                    rc = KSocketConnectIPC ( conn, to );
                                    break;
                                }

                                /* if connection was successful, return socket */
                                if ( rc == 0 )
                                {
                                    * out = conn;
                                    return 0;
                                }
                                /* if was interrupted by Ctrl-C, return immediately */
                                if ( rcConnection == GetRCObject ( rc ) && rcInterrupted == GetRCState ( rc ) )
                                {
                                    KSocketRelease ( conn );
                                    * out = NULL;
                                    return rc;
                                }

                                /* check time remaining on timeout ( if any ) */
                                remaining = TimeoutRemaining ( retryTimeout );

                                /* break out of loop if no time left */
                                if ( remaining == 0 )
                                    break;

                                /* apply delay schedule */
                                switch ( retry )
                                {
                                case 0:
                                    /* try immediately */
                                    continue;
                                case 1:
                                case 2:
                                case 3:
                                    /* wait for 100mS between tries */
                                    delay = 100;
                                    break;
                                default:
                                    /* wait for 250mS between tries */
                                    delay = 250;
                                }

                                /* never wait for more than the remaining timeout */
                                if ( delay > remaining )
                                    delay = remaining;

                                KSleepMs ( delay );
                            }

                            DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET),
                                   ( "%p: KSocketConnect timed out\n", self ) );
                            break;

                        default:
                            rc = RC ( rcNS, rcStream, rcConstructing, rcParam, rcIncorrect );
                        }

                    } /* if TimeoutPrepare () == 0 */

                    KStreamWhack ( & conn -> dad, "KSocket" );

                } /* if KStreamInit () == 0 */

                free ( conn );
            }
        }

        * out = NULL;
    }

    return rc;
}


/*--------------------------------------------------------------------------
 * KListener
 */
static
rc_t KNSManagerMakeIPv6Listener ( KSocket *listener, const KEndPoint * ep )
{
    rc_t rc;

    listener -> fd = socket ( AF_INET6, SOCK_STREAM, 0 );
    if ( listener -> fd < 0 )
        rc = KSocketHandleSocketCall ( errno );
    else
    {
        struct sockaddr_in6 ss;

        int on = 1;
        setsockopt ( listener -> fd, SOL_SOCKET, SO_REUSEADDR, ( char* ) & on, sizeof on );

        memset ( & ss, 0, sizeof ss );
        ss . sin6_family = AF_INET6;

        memmove ( ss . sin6_addr . s6_addr,
                 ep -> u . ipv6 . addr,
                 sizeof ( ep -> u . ipv6 . addr ) );

        ss . sin6_port = htons ( ep -> u . ipv6 . port );

        if ( bind ( listener -> fd, ( struct sockaddr* ) & ss, sizeof ss ) != 0 )
           rc = KSocketHandleBindCall ( errno );
        else
            return 0;

        close ( listener -> fd );
        listener -> fd = -1;
    }

    return rc;
}


static
rc_t KNSManagerMakeIPv4Listener ( KSocket *listener, const KEndPoint * ep )
{
    rc_t rc;

    listener -> fd = socket ( AF_INET, SOCK_STREAM, 0 );
    if ( listener -> fd < 0 )
        rc = KSocketHandleSocketCall ( errno );
    else
    {
        struct sockaddr_in ss;

        int on = 1;
        setsockopt ( listener -> fd, SOL_SOCKET, SO_REUSEADDR, ( char* ) & on, sizeof on );

        memset ( & ss, 0, sizeof ss );
        ss . sin_family = AF_INET;
        ss . sin_addr . s_addr = htonl ( ep -> u . ipv4 . addr );
        ss . sin_port = htons ( ep -> u . ipv4 . port );

        if ( bind ( listener -> fd, ( struct sockaddr* ) & ss, sizeof ss ) != 0 )
            rc = KSocketHandleBindCall ( errno );
        else
            return 0;

        close ( listener -> fd );
        listener -> fd = -1;
    }

    return rc;
}

static
rc_t KNSManagerMakeIPCListener ( KSocket *listener, const KEndPoint * ep )
{
    rc_t rc;

    listener -> fd = socket ( AF_UNIX, SOCK_STREAM, 0 );
    if ( listener -> fd < 0 )
        rc = KSocketHandleSocketCall ( errno );
    else
    {
        struct sockaddr_un ss;
        memset ( & ss, 0, sizeof ss );
        ss.sun_family = AF_UNIX;
        rc = KSocketMakePath ( ep -> u. ipc_name, ss . sun_path, sizeof ss . sun_path );
        if ( rc == 0 )
        {
            char * path = string_dup ( ss . sun_path, string_measure ( ss . sun_path, NULL ) );
            if ( path == NULL )
                rc = RC ( rcNS, rcSocket, rcConstructing, rcMemory, rcExhausted );
            else
            {
                unlink ( ss . sun_path );
                if ( bind ( listener -> fd, ( struct sockaddr* ) & ss, sizeof ss ) != 0 )
                    rc = KSocketHandleBindCall ( errno );
                else
                {
                    listener -> path = path;
                    return 0;
                }

                free ( path );
            }
        }

        close ( listener -> fd );
        listener -> fd = -1;
    }

    return rc;
}

LIB_EXPORT rc_t CC KNSManagerMakeListener ( const KNSManager *self,
    KListener ** out, const KEndPoint * ep )
{
    rc_t rc;

    if ( out == NULL )
        rc = RC ( rcNS, rcSocket, rcConstructing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcNS, rcSocket, rcConstructing, rcSelf, rcNull );
        else if ( ep == NULL )
            rc = RC ( rcNS, rcSocket, rcConstructing, rcParam, rcNull );
        else
        {
            KSocket *listener = calloc ( 1, sizeof * listener );
            if ( listener == NULL )
                rc = RC ( rcNS, rcSocket, rcConstructing, rcMemory, rcExhausted );
            else
            {
                listener -> fd = -1;

                /* pass these along to accepted sockets */
                listener -> read_timeout = self -> conn_read_timeout;
                listener -> write_timeout = self -> conn_write_timeout;

                rc = KStreamInit ( & listener -> dad, ( const KStream_vt* ) & vtKSocket,
                    "KSocket - Listener", ep -> ip_address, true, true );
                if ( rc == 0 )
                {
                    listener -> type = ep -> type;
                    switch ( ep -> type )
                    {
                    case epIPV6:
                        rc = KNSManagerMakeIPv6Listener ( listener, ep );
                        break;

                    case epIPV4:
                        rc = KNSManagerMakeIPv4Listener ( listener, ep );
                        break;

                    case epIPC:
                        rc = KNSManagerMakeIPCListener ( listener, ep );
                        break;

                    default:
                        rc = RC ( rcNS, rcSocket, rcConstructing, rcParam, rcIncorrect );
                    }

                    if ( rc == 0 )
                    {
                        /* the classic 5 connection queue... ? */
                        if ( listen ( listener -> fd, 5 ) == 0 )
                        {
                            * out = ( KListener* ) listener;
                            return 0;
                        }
                        else
                        {
                            switch ( errno )
                            {
                            case EADDRINUSE:
                                rc = RC ( rcNS, rcSocket, rcCreating, rcSocket, rcBusy );
                                break;
                            case EBADF:
                                rc = RC ( rcNS, rcSocket, rcCreating, rcSocket, rcInvalid );
                                break;
                            case ENOTSOCK:
                                rc = RC ( rcNS, rcSocket, rcCreating, rcSocket, rcIncorrect );
                                break;
                            case EOPNOTSUPP:
                                rc = RC ( rcNS, rcSocket, rcCreating, rcInterface, rcUnsupported );
                                break;
                            default:
                                rc = RC ( rcNS, rcSocket, rcCreating, rcError, rcUnknown );
                                break;
                            }
                        }

                        if ( listener -> path != NULL )
                            free ( ( void* ) listener -> path );
                    }
                }

                free ( listener );
            }
        }

        * out = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC KListenerAddRef( const KListener *self )
{
    return KSocketAddRef ( ( const KSocket* ) self );
}

LIB_EXPORT rc_t CC KListenerRelease ( const KListener *self )
{
    return KSocketRelease ( ( const KSocket* ) self );
}

static
rc_t KListenerAcceptIPv4 ( KSocket *self, KSocket *conn )
{
    socklen_t len = sizeof conn->remote_addr.v4;
    conn -> fd = accept ( self -> fd, ( struct sockaddr * ) & conn->remote_addr.v4, & len );
    if ( conn -> fd < 0 )
        return KSocketHandleAcceptCall ( errno );

    if ( len > sizeof conn->remote_addr.v4 )
        return RC ( rcNS, rcConnection, rcWaiting, rcBuffer, rcInsufficient );

    return 0;
}


static
rc_t KListenerAcceptIPv6 ( KSocket *self, KSocket *conn )
{
    socklen_t len = sizeof conn->remote_addr.v6;
    conn -> fd = accept ( self -> fd, ( struct sockaddr * ) & conn->remote_addr.v6, & len );
    if ( conn -> fd < 0 )
        return KSocketHandleAcceptCall ( errno );

    if ( len > sizeof conn->remote_addr.v6 )
        return RC ( rcNS, rcConnection, rcWaiting, rcBuffer, rcInsufficient );

    return 0;
}

static
rc_t KListenerAcceptIPC ( KSocket *self, KSocket *conn )
{
    struct sockaddr_un remote;
    socklen_t len = sizeof remote;
    conn -> fd = accept ( self -> fd, ( struct sockaddr* ) & remote, & len );
    if ( conn -> fd < 0 )
        return KSocketHandleAcceptCall ( errno );

    if ( len > sizeof remote )
        return RC ( rcNS, rcConnection, rcWaiting, rcBuffer, rcInsufficient );

    return 0;
}

LIB_EXPORT rc_t CC KListenerAccept ( KListener *iself, struct KSocket **out )
{
    rc_t rc;
    KSocket * self = ( KSocket* ) iself;


    if ( out == NULL )
        rc = RC ( rcNS, rcConnection, rcWaiting, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcNS, rcConnection, rcWaiting, rcSelf, rcNull);
        else
        {
            KSocket tmp_socket; /* we only need to populate fd and remote_addr */
            tmp_socket . fd = -1;

            switch ( self -> type )
            {
            case epIPV6:
                rc = KListenerAcceptIPv6 ( self, & tmp_socket );
                break;

            case epIPV4:
                rc = KListenerAcceptIPv4 ( self, & tmp_socket );
                break;

            case epIPC:
                rc = KListenerAcceptIPC ( self, & tmp_socket );
                break;

            default:
                rc = RC ( rcNS, rcSocket, rcConstructing, rcSelf, rcCorrupt );
            }

            if ( rc == 0 )
            {
                KSocket * new_socket = calloc ( 1, sizeof * new_socket );
                if ( new_socket == NULL )
                    rc = RC ( rcNS, rcConnection, rcWaiting, rcMemory, rcExhausted );
                else
                {
                    new_socket -> fd                = tmp_socket . fd;
                    new_socket -> remote_addr       = tmp_socket . remote_addr;
                    new_socket -> read_timeout      = self -> read_timeout;
                    new_socket -> write_timeout     = self -> write_timeout;
                    new_socket -> remote_addr_valid = true;

                    rc = KStreamInit ( & new_socket -> dad, ( const KStream_vt* ) & vtKSocket,
                        "KSocket - Accept", "", true, true );
                    if ( rc == 0 )
                    {
                        * out = new_socket;
                        return 0;
                    }

                    free ( new_socket );
                }
            }
        }

        * out = NULL;
    }

    return rc;
}
