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

#include <klib/debug.h> /* DBGMSG */
#include <klib/log.h>
#include <klib/out.h>
#include <klib/printf.h>
#include <klib/rc.h>
#include <klib/text.h>

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
    union {
        struct sockaddr_in  v4;     /* for ipv4 */
        struct sockaddr_in6 v6;     /* for ipv6 */
    } remote_addr;
    bool remote_addr_valid;
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
        
    free ( self );

    return 0;
}

static
rc_t HandleErrno ( const char *func_name, unsigned int lineno )
{
    int lerrno;
    rc_t rc = 0;
    
    switch ( lerrno = errno )
    {
    case EACCES: /* write permission denied */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcMemory, rcUnauthorized );            
        break;
    case EADDRINUSE: /* address is already in use */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcMemory, rcExists );
        break;
    case EADDRNOTAVAIL: /* requested address was not local */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcMemory, rcNotFound );
        break;
    case EAGAIN: /* no more free local ports or insufficient rentries in routing cache */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcNoObj, rcExhausted );            
        break;
    case EAFNOSUPPORT: /* address didnt have correct address family in ss_family field */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcName, rcInvalid );            
        break;
    case EALREADY: /* socket is non blocking and a previous connection has not yet completed */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcId, rcUndefined );
        break;
    case EBADF: /* invalid sock fd */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcId, rcInvalid );
        break;
    case ECONNREFUSED: /* remote host refused to allow network connection */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcConnection, rcCanceled );
        break;
    case ECONNRESET: /* connection reset by peer */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcConnection, rcCanceled );
        break;
    case EDESTADDRREQ: /* socket is not connection-mode and no peer address set */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcId, rcInvalid );
        break;
    case EFAULT: /* buffer pointer points outside of process's adress space */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcMemory, rcOutofrange );
        break;
    case EINPROGRESS: /* call is in progress */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcId, rcUndefined );
        break;
    case EINTR: /* recv interrupted before any data available */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcConnection, rcCanceled );
        break;
    case EINVAL: /* invalid argument */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcParam, rcInvalid );
        break;
    case EISCONN: /* connected already */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcConnection, rcExists );
        break;
    case ELOOP: /* too many symbolic links in resolving addr */
        rc = RC ( rcNS, rcNoTarg, rcResolving, rcLink, rcExcessive );
        break;
    case EMFILE: /* process file table overflow */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcProcess, rcExhausted );
        break;
    case EMSGSIZE: /* msg size too big */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcMessage, rcExcessive );
        break;
    case ENAMETOOLONG: /* addr name is too long */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcName, rcExcessive );
        break;
    case ENETUNREACH: /* network is unreachable */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcConnection, rcNotAvailable );
        break;
    case ENOBUFS: /* output queue for a network connection was full. 
                     ( wont typically happen in linux. Packets are just silently dropped */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcConnection, rcInterrupted );
        break;
    case ENOENT: /* file does not exist */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcId, rcNotFound );
        break;
    case ENOMEM: /* Could not allocate memory */
        rc = RC ( rcNS, rcNoTarg, rcAllocating, rcMemory, rcExhausted );
        break;
    case ENOTCONN: /* socket has not been connected */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcConnection, rcInvalid );
        break;
    case ENOTDIR: /* component of path is not a directory */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcDirEntry, rcInvalid );
        break;
    case ENOTSOCK: /* sock fd does not refer to socket */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcId, rcInvalid );
        break;
    case EOPNOTSUPP: /* bits in flags argument is inappropriate */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcParam, rcInvalid );
        break;
    case EPERM:
        rc = RC ( rcNS, rcNoTarg, rcReading, rcMemory, rcUnauthorized );            
        break;
    case EPIPE: /* local end has been shut down. Will also receive SIGPIPE or MSG_NOSIGNAL */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcConnection, rcCanceled );
        break;
    case EPROTONOSUPPORT: /* specified protocol is not supported */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcAttr, rcUnsupported );
        break;
    case EROFS: /* socket inode on read only file system */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcNoObj, rcReadonly );
        break;
    case ETIMEDOUT: /* timeout */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcConnection, rcNotAvailable );
        break;
#if ! defined EAGAIN || ! defined EWOULDBLOCK || EAGAIN != EWOULDBLOCK
    case EWOULDBLOCK:
        rc = RC ( rcNS, rcNoTarg, rcReading, rcCmd, rcBusy );
        break;
#endif
    default:
        rc = RC ( rcNS, rcNoTarg, rcReading, rcError, rcUnknown );
        PLOGERR (klogErr,
                 (klogErr, rc, "unknown system error '$(S)($(E))'",
                  "S=%!,E=%d", lerrno, lerrno));
    }
    
    if ( rc != 0 )
    {
        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%R\n", rc ) );
    }

    return rc;
}


static rc_t KSocketGetEndpointV6 ( const KSocket * self, KEndPoint * ep, bool remote )
{
    rc_t rc = 0;
    struct sockaddr_in6 addr;
    socklen_t l = sizeof( addr );
    int res = 0;

    if ( remote )
    {
        if ( self -> remote_addr_valid )
        {
            /* the remote part was already recorded through calling accept() */
            memcpy ( ep -> u . ipv6 . addr,
                     self -> remote_addr . v6 . sin6_addr . s6_addr,
                     sizeof ( ep -> u . ipv6 . addr ) );
            ep->u.ipv6.port = ntohs( self -> remote_addr . v6 . sin6_port );
            ep->type = epIPV6;
            return 0;
        }
        else
            res = getpeername( self -> fd, ( struct sockaddr * )&addr, &l );
    }
    else
        res = getsockname( self -> fd, ( struct sockaddr * )&addr, &l );

    if ( res < 0 )
        rc = HandleErrno ( __func__, __LINE__ );
    else
    {
        memcpy ( ep -> u . ipv6 . addr,
                 addr . sin6_addr . s6_addr,
                 sizeof ( ep -> u . ipv6 . addr ) );
        ep->u.ipv6.port = ntohs( addr . sin6_port );
        ep->type = epIPV6;
    }

    return rc;
}


static rc_t KSocketGetEndpointV4 ( const KSocket * self, KEndPoint * ep, bool remote )
{
    rc_t rc = 0;
    struct sockaddr_in addr;
    socklen_t l = sizeof( addr );
    int res = 0;

    if ( remote )
    {
        if ( self -> remote_addr_valid )
        {
            /* the remote part was already recorded through calling accept() */
            addr.sin_addr.s_addr = self -> remote_addr.v4.sin_addr.s_addr;
            addr.sin_port        = self -> remote_addr.v4.sin_port;
        }
        else
            res = getpeername( self -> fd, ( struct sockaddr * )&addr, &l );
    }
    else
        res = getsockname( self -> fd, ( struct sockaddr * )&addr, &l );

    if ( res < 0 )
        rc = HandleErrno ( __func__, __LINE__ );
    else
    {
        ep->u.ipv4.addr = ntohl( addr.sin_addr.s_addr );
        ep->u.ipv4.port = ntohs( addr.sin_port );
        ep->type = epIPV4;
    }

    return rc;
}


static rc_t KSocketGetEndpoint ( const KSocket * self, KEndPoint * ep, bool remote )
{
    rc_t rc = 0;
    if ( ep == NULL )
        rc = RC ( rcNS, rcSocket, rcEvaluating, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcNS, rcSocket, rcEvaluating, rcSelf, rcNull );
        else
        {
            switch( self->type )
            {
                case epIPV6 : rc = KSocketGetEndpointV6( self, ep, remote ); break;
                case epIPV4 : rc = KSocketGetEndpointV4( self, ep, remote ); break;
                default     : rc = RC ( rcNS, rcSocket, rcEvaluating, rcFunction, rcUnsupported );
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
        rc = HandleErrno ( __func__, __LINE__ );
        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketTimedRead socket_wait returned '%s'\n", self, strerror(errno) ) );
        return rc;
    }
    if ( ( revents & ( POLLERR | POLLNVAL ) ) != 0 )
    {
        if ( ( revents & POLLERR ) != 0)
        {
            int optval = 0;
            socklen_t optlen = sizeof optval;
            if ( ( getsockopt ( self -> fd, SOL_SOCKET, SO_ERROR, & optval, & optlen ) == 0 )
                 && optval > 0)
            {
                errno = optval;
                DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketTimedRead socket_wait/getsockopt returned '%s'\n", 
                                                            self, strerror(optval) ) );
                return HandleErrno(__func__, __LINE__);
            }
        }

        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketTimedRead socket_wait returned POLLERR | POLLNVAL\n", self ) );
        return RC ( rcNS, rcStream, rcReading, rcNoObj, rcUnknown );
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
        rc = HandleErrno ( __func__, __LINE__ );
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
        rc = HandleErrno ( __func__, __LINE__ );
        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketTimedRead error '%s'\n", self, strerror ( errno ) ) );
        return rc;
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
        rc = HandleErrno ( __func__, __LINE__ );
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
        if ( count >= 0 )
        {
            DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: %d bytes written\n", self, count ) );
            * num_writ = count;
            return 0;
        }

        rc = HandleErrno ( __func__, __LINE__ );
        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketTimedWrite recv returned count %d\n", self, count ) );
        return rc;
    }

    /* anything else in revents is an error */
    if ( ( revents & ~ POLLOUT ) != 0 && errno != 0 )
    {
        rc = HandleErrno ( __func__, __LINE__ );
        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketTimedWrite error '%s'\n", self, strerror ( errno ) ) );
        return rc;
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
    struct passwd* pwd;
    pwd = getpwuid ( geteuid () );
    if ( pwd == NULL )
        return HandleErrno ( __func__, __LINE__ );

    return string_printf ( buf, buf_size, & num_writ, "%s/.ncbi/%s", pwd -> pw_dir, name );
#else
    const char *HOME = getenv ( "HOME" );
    if ( HOME == NULL )
        return RC ( rcNS, rcProcess, rcAccessing, rcPath, rcNotFound );

    return string_printf ( buf, buf_size, & num_writ, "%s/.ncbi/%s", HOME, name );
#endif
}

static
rc_t KSocketConnectIPv4 ( KSocket *self, int32_t retryTimeout, const KEndPoint *from, const KEndPoint *to )
{
    rc_t rc = 0;
    uint32_t retry_count = 0;
    struct sockaddr_in ss_from, ss_to;

    memset ( & ss_from, 0, sizeof ss_from );
    if ( from != NULL )
    {
        ss_from . sin_family = AF_INET;
        ss_from . sin_addr . s_addr = htonl ( from -> u . ipv4 . addr );
        ss_from . sin_port = htons ( from -> u . ipv4 . port );
    }

    memset ( & ss_to, 0, sizeof ss_to );
    ss_to . sin_family = AF_INET;
    ss_to . sin_addr . s_addr = htonl ( to -> u . ipv4 . addr );
    ss_to . sin_port = htons ( to -> u . ipv4 . port );

    do 
    {
        /* create the OS socket */
        self -> fd = socket ( AF_INET, SOCK_STREAM, 0 );
        if ( self -> fd < 0 )
            rc = HandleErrno ( __func__, __LINE__ );
        else
        {
            /* disable nagle algorithm */
            int flag = 1;
            setsockopt ( self -> fd, IPPROTO_TCP, TCP_NODELAY, ( char* ) & flag, sizeof flag );

            /* bind */
            if ( from != NULL && bind ( self -> fd, ( struct sockaddr* ) & ss_from, sizeof ss_from ) != 0 )
                rc = HandleErrno ( __func__, __LINE__ );
                
            if ( rc == 0 )
            {
                /* connect */
                if ( connect ( self -> fd, ( struct sockaddr* ) & ss_to, sizeof ss_to ) == 0 )
                {
                    /* set non-blocking mode */
                    flag = fcntl ( self -> fd, F_GETFL );
                    fcntl ( self -> fd, F_SETFL, flag | O_NONBLOCK );
                    return 0;
                }
                rc = HandleErrno ( __func__, __LINE__ );
            }

            /* dump socket */
            close ( self -> fd );
            self -> fd = -1;
        }
        
        /* rc != 0 */
        if (retryTimeout < 0 || retry_count < retryTimeout)
        {   /* retry */
            sleep ( 1 );
            ++retry_count;
            rc = 0;
        }
    }
    while (rc == 0);
    
    DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketConnectIPv4 timed out\n", self ) );

    return rc;
}


static
rc_t KSocketConnectIPv6 ( KSocket *self, int32_t retryTimeout, const KEndPoint *from, const KEndPoint *to )
{
    rc_t rc = 0;
    uint32_t retry_count = 0;
    struct sockaddr_in6 ss_from, ss_to;

    memset ( & ss_from, 0, sizeof ss_from );
    if ( from != NULL )
    {
        ss_from . sin6_family = AF_INET6;
        memcpy ( ss_from . sin6_addr . s6_addr,
                 from -> u . ipv6 . addr,
                 sizeof ( from -> u . ipv6 . addr ) );
        ss_from . sin6_port = htons ( from -> u . ipv6 . port );
    }

    memset ( & ss_to, 0, sizeof ss_to );
    ss_to . sin6_family = AF_INET6;
    memcpy ( ss_to . sin6_addr . s6_addr,
             to -> u . ipv6 . addr,
             sizeof ( to -> u . ipv6 . addr ) );
    ss_to . sin6_port = htons ( to -> u . ipv6 . port );

    do 
    {
        /* create the OS socket */
        self -> fd = socket ( AF_INET6, SOCK_STREAM, 0 );
        if ( self -> fd < 0 )
            rc = HandleErrno ( __func__, __LINE__ );
        else
        {
            /* disable nagle algorithm */
            int flag = 1;
            setsockopt ( self -> fd, IPPROTO_TCP, TCP_NODELAY, ( char* ) & flag, sizeof flag );

            /* bind */
            if ( from != NULL && bind ( self -> fd, ( struct sockaddr* ) & ss_from, sizeof ss_from ) != 0 )
                rc = HandleErrno ( __func__, __LINE__ );
                
            if ( rc == 0 )
            {
                /* connect */
                if ( connect ( self -> fd, ( struct sockaddr* ) & ss_to, sizeof ss_to ) == 0 )
                {
                    /* set non-blocking mode */
                    flag = fcntl ( self -> fd, F_GETFL );
                    fcntl ( self -> fd, F_SETFL, flag | O_NONBLOCK );
                    return 0;
                }
                rc = HandleErrno ( __func__, __LINE__ );
            }

            /* dump socket */
            close ( self -> fd );
            self -> fd = -1;
        }
        
        /* rc != 0 */
        if (retryTimeout < 0 || retry_count < retryTimeout)
        {   /* retry */
            sleep ( 1 );
            ++retry_count;
            rc = 0;
        }
    }
    while (rc == 0);
    
    DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketConnectIPv6 timed out\n", self ) );

    return rc;
}


static
rc_t KSocketConnectIPC ( KSocket *self, int32_t retryTimeout, const KEndPoint *to )
{
    rc_t rc = 0;
    uint32_t retry_count = 0;
    struct sockaddr_un ss_to;

    memset ( & ss_to, 0, sizeof ss_to );
    ss_to . sun_family = AF_UNIX;
    rc = KSocketMakePath ( to -> u . ipc_name, ss_to . sun_path, sizeof ss_to . sun_path );

    do 
    {
        /* create the OS socket */
        self -> fd = socket ( AF_UNIX, SOCK_STREAM, 0 );
        if ( self -> fd < 0 )
            rc = HandleErrno ( __func__, __LINE__ );
        else
        {
            /* connect */
            if ( connect ( self -> fd, ( struct sockaddr* ) & ss_to, sizeof ss_to ) == 0 )
            {
                return 0;
            }
            rc = HandleErrno ( __func__, __LINE__ );

            /* dump socket */
            close ( self -> fd );
            self -> fd = -1;
        }
        
        /* rc != 0 */
        if (retryTimeout < 0 || retry_count < retryTimeout)
        {   /* retry */
            sleep ( 1 );
            ++retry_count;
            rc = 0;
        }
    }
    while (rc == 0);

    DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketConnectIPC timed out\n", self ) );            

    return rc;
 }

KNS_EXTERN rc_t CC KNSManagerMakeRetryTimedConnection ( struct KNSManager const * self,
    struct KSocket **out, int32_t retryTimeout, int32_t readMillis, int32_t writeMillis,
    struct KEndPoint const *from, struct KEndPoint const *to )
{
    rc_t rc;

    if ( out == NULL )
        rc = RC ( rcNS, rcStream, rcConstructing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcNS, rcStream, rcConstructing, rcSelf, rcNull );
        else if ( to == NULL )
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
                                   "KSocket", "", true, true );
                if ( rc == 0 )
                {
                    switch ( to -> type )
                    {
                    case epIPV6:
                        rc = KSocketConnectIPv6 ( conn, retryTimeout, from, to );
                        break;

                    case epIPV4:
                        rc = KSocketConnectIPv4 ( conn, retryTimeout, from, to );
                        break;

                    case epIPC:
                        rc = KSocketConnectIPC ( conn, retryTimeout, to );
                        break;

                    default:
                        rc = RC ( rcNS, rcStream, rcConstructing, rcParam, rcIncorrect );
                    }

                    if ( rc == 0 )
                    {
                        * out = conn;
                        return 0;
                    }
                }

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
        rc = HandleErrno ( __func__, __LINE__ );
    else
    {
        struct sockaddr_in6 ss;

        int on = 1;
        setsockopt ( listener -> fd, SOL_SOCKET, SO_REUSEADDR, ( char* ) & on, sizeof on );

        memset ( & ss, 0, sizeof ss );
        ss . sin6_family = AF_INET6;

        memcpy ( ss . sin6_addr . s6_addr,
                 ep -> u . ipv6 . addr,
                 sizeof ( ep -> u . ipv6 . addr ) );

        ss . sin6_port = htons ( ep -> u . ipv6 . port );

        if ( bind ( listener -> fd, ( struct sockaddr* ) & ss, sizeof ss ) == 0 )
            return 0;
        rc = HandleErrno ( __func__, __LINE__ );

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
        rc = HandleErrno ( __func__, __LINE__ );
    else
    {
        struct sockaddr_in ss;

        int on = 1;
        setsockopt ( listener -> fd, SOL_SOCKET, SO_REUSEADDR, ( char* ) & on, sizeof on );

        memset ( & ss, 0, sizeof ss );
        ss . sin_family = AF_INET;
        ss . sin_addr . s_addr = htonl ( ep -> u . ipv4 . addr );
        ss . sin_port = htons ( ep -> u . ipv4 . port );

        if ( bind ( listener -> fd, ( struct sockaddr* ) & ss, sizeof ss ) == 0 )
            return 0;
        rc = HandleErrno ( __func__, __LINE__ );

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
        rc = HandleErrno ( __func__, __LINE__ );
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
                    rc = HandleErrno ( __func__, __LINE__ );
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
                                   "KSocket", "", true, true );
                if ( rc == 0 )
                {
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

                        rc = HandleErrno ( __func__, __LINE__ );

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
        return HandleErrno ( __func__, __LINE__ );
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
        return HandleErrno ( __func__, __LINE__ );
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
        return HandleErrno ( __func__, __LINE__ );
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
                                       "KSocket", "", true, true );
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
