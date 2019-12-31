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
typedef struct KSocket KSocket;

#include <kns/extern.h>
#include <kns/manager.h>
#include <kns/socket.h>
#include <kns/impl.h>
#include <kns/endpoint.h>
#include <klib/rc.h>
#include <klib/debug.h>
#include <klib/log.h>
#include <klib/printf.h>
#include <klib/time.h>
#include <sysalloc.h>
#include <kproc/timeout.h>

#include "mgr-priv.h"
#include "stream-priv.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* os-native.h includes windows.h and winsock2.h and klib/text.h !!! */
#include <os-native.h>

#define LOG

#define SHUT_RD 0
#define SHUT_WR 1
typedef SSIZE_T ssize_t;

static rc_t HandleErrnoEx ( const char *func, unsigned int lineno, rc_t rc_ctx );
#define HandleErrno() HandleErrnoEx ( __func__, __LINE__, rc_ctx )
#define RC_CTX( t, c ) \
    const rc_t rc_ctx = CTX ( rcNS, t, c )
    

#define PIPE_NAME_LENGTH 256

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
 
 /*
  * On Windows, we have 2 different mechanisms to implement KSockets, 
  * WinSock based for Ipv4 connections and named pipes based for IPC
  */


struct KSocketIPv4
{
    SOCKET fd;
    struct sockaddr_in  remote_addr;     /* for ipv4 addr from accept */
    bool remote_addr_valid;
};
typedef struct KSocketIPv4 KSocketIPv4;
  

struct KSocketIPv6
{
    SOCKET fd;
    struct sockaddr_in6 remote_addr;     /* for ipv6 addr from accept */
    bool remote_addr_valid;
};
typedef struct KSocketIPv6 KSocketIPv6;


enum { isIpcListener, isIpcPipeServer, isIpcPipeClient };

struct KSocketIPC
{
    HANDLE pipe;
    wchar_t pipename [ PIPE_NAME_LENGTH ];

    uint8_t type;
    HANDLE listenerPipe;            /* only used if type == isIpcListener */ 
};
typedef struct KSocketIPC KSocketIPC;


struct KSocket
{
    /* we have only one KSocket-type with a union of the implementation specific data */
    KStream dad;

    int32_t read_timeout;
    int32_t write_timeout;
    uint32_t type; /* epIPV4|epIPV6|epIPC ... KEndPointType from endpoint.h */

    union {
        KSocketIPv4 ipv4_data;
        KSocketIPv6 ipv6_data;
        KSocketIPC  ipc_data;
    } type_data;
};

static
rc_t CC KSocketHandleShutdownCallWin ()
{
    switch ( WSAGetLastError () )
    {
    case WSAECONNABORTED:
        return RC ( rcNS, rcSocket, rcClosing, rcConnection, rcCanceled );
    case WSAECONNRESET:
        return RC ( rcNS, rcSocket, rcClosing, rcConnection, rcCanceled );
    case WSAEINPROGRESS:
        return RC ( rcNS, rcSocket, rcClosing, rcConnection, rcBusy );
    case WSAEINVAL:
        return RC ( rcNS, rcSocket, rcClosing, rcParam, rcInvalid );
    case WSAENETDOWN: /* ENETUNREACH */
        return RC ( rcNS, rcSocket, rcClosing, rcConnection, rcNotAvailable );
    case WSAENOTCONN:
        return RC ( rcNS, rcSocket, rcClosing, rcConnection, rcInvalid );
    case WSAENOTSOCK:
        return RC ( rcNS, rcSocket, rcClosing, rcSocket, rcIncorrect );
    case WSANOTINITIALISED:
        return RC ( rcNS, rcSocket, rcClosing, rcEnvironment, rcInvalid );
    }
    return RC ( rcNS, rcSocket, rcClosing, rcError, rcUnknown );
}

static
rc_t CC KSocketHandleRecvCallWin ( rc_t context )
{
    switch ( WSAGetLastError () )
    {
    case WSANOTINITIALISED:
        return RC ( rcNS, rcSocket, context, rcEnvironment, rcInvalid );
    case WSAENETDOWN: /* ENETUNREACH */
        return RC ( rcNS, rcSocket, context, rcConnection, rcNotAvailable );
    case WSAEINVAL:
    case WSAEFAULT:
        return RC ( rcNS, rcSocket, context, rcParam, rcInvalid );
    case WSAENOTCONN:
        return RC ( rcNS, rcSocket, context, rcConnection, rcInvalid );
    case WSAEINTR: /* canceled through WSACancelBlockingCall */
        return 0;
    case WSAEINPROGRESS:
        return RC ( rcNS, rcSocket, context, rcConnection, rcBusy );
    case WSAENETRESET:
        return RC ( rcNS, rcSocket, context, rcConnection, rcCanceled );
    case WSAENOTSOCK:
        return RC ( rcNS, rcSocket, context, rcSocket, rcIncorrect );
    case WSAEOPNOTSUPP:
        return RC ( rcNS, rcSocket, context, rcParam, rcIncorrect );
    case WSAESHUTDOWN:
        return RC ( rcNS, rcSocket, context, rcSocket, rcNotOpen );
    case WSAEWOULDBLOCK:
        return RC ( rcNS, rcSocket, context, rcData, rcNotAvailable );
    case WSAEMSGSIZE:
        return RC ( rcNS, rcSocket, context, rcData, rcExcessive );
    case WSAECONNABORTED:
        return RC ( rcNS, rcSocket, context, rcConnection, rcCanceled );
    case WSAETIMEDOUT:
        return RC ( rcNS, rcSocket, context, rcTimeout, rcExhausted );
    case WSAECONNRESET:
        return RC ( rcNS, rcSocket, context, rcConnection, rcCanceled );
    }

    return RC ( rcNS, rcSocket, context, rcError, rcUnknown );
}

static
rc_t CC KSocketHandleSelectCallWin ( rc_t context )
{
    switch ( WSAGetLastError () )
    {
    case WSANOTINITIALISED:
        return RC ( rcNS, rcSocket, context, rcEnvironment, rcInvalid );
    case WSAEINVAL:
    case WSAEFAULT:
        return RC ( rcNS, rcSocket, context, rcParam, rcInvalid );
    case WSAENETDOWN: /* ENETUNREACH */
        return RC ( rcNS, rcSocket, context, rcConnection, rcNotAvailable );
    case WSAEINTR: /* canceled through WSACancelBlockingCall */
        return 0;
    case WSAEINPROGRESS:
        return RC ( rcNS, rcSocket, context, rcConnection, rcBusy );
    case WSAENOTSOCK:
        return RC ( rcNS, rcSocket, context, rcSocket, rcIncorrect );
    }

    return RC ( rcNS, rcSocket, context, rcError, rcUnknown );
}

static rc_t CC KIpv4SocketWhack ( KSocket * self )
{
    rc_t rc = 0;
    KSocketIPv4 * data;

    if ( self == NULL ) /* let's tolerate whacking NULL pointers... */
        return rc;

    if ( self->type != epIPV4 )
        return RC ( rcNS, rcSocket, rcClosing, rcParam, rcInvalid );

    data = &( self -> type_data.ipv4_data );

    if ( shutdown ( data -> fd, SHUT_WR ) == -1 )
        rc = KSocketHandleShutdownCallWin ();
    else
    {
        while ( 1 ) 
        {
            char buffer [ 1024 ];
            ssize_t result = recv ( data -> fd, buffer, sizeof buffer, 0 );
            if ( result < 0 )
            {
                rc = KSocketHandleRecvCallWin ( rcClosing );
                break;
            }
            else if (result == 0)
            {
                break;
            }
        }

        if ( shutdown ( data -> fd, SHUT_RD ) == -1 )
            rc = KSocketHandleShutdownCallWin ();
        else if ( closesocket ( data -> fd ) == SOCKET_ERROR )
            rc = RC ( rcNS, rcSocket, rcClosing, rcError, rcUnknown );  /* maybe report */
    }

#if 0
    RC_CTX ( rcSocket, rcClosing );
    if ( shutdown ( data -> fd, SHUT_WR ) != -1 )
    {
        while ( 1 ) 
        {
            char buffer [ 1024 ];
            ssize_t result = recv ( data -> fd, buffer, sizeof buffer, 0 );
            if ( result <= 0 )
                break;
        }

        if ( shutdown ( data -> fd, SHUT_RD ) != -1 )
        {
            if ( closesocket ( data -> fd ) == SOCKET_ERROR )
                rc = RC ( rcNS, rcSocket, rcClosing, rcError, rcUnknown );
                /* maybe report */
        }
        else
            rc = HandleErrno();
    }
    else
        rc = HandleErrno();
#endif 

    free ( self );

    return rc;
}


static rc_t CC KIpv4SocketTimedRead ( const KSocket * self, void * buffer, size_t bsize,
                                      size_t * num_read, timeout_t * tm )
{
    RC_CTX ( rcSocket, rcReading );

    /* self != NULL and self->type == epIPV4 already checked by the caller */

    rc_t rc = 0;
    const KSocketIPv4 * data = &( self -> type_data.ipv4_data );
    struct timeval ts;
    fd_set readFds;
    int selectRes;
    
    /* convert timeout (relative time) */
    if ( tm != NULL )
    {
        ts.tv_sec = tm -> mS / 1000;
        ts.tv_usec = ( tm -> mS % 1000 ) * 1000;
    }
    
    /* wait for socket to become readable */
    FD_ZERO( &readFds );
    FD_SET( data -> fd, &readFds );
    selectRes = select( 0, &readFds, NULL, NULL, ( tm == NULL ) ? NULL : &ts );
    
    /* check for error */
    if ( selectRes == -1 )
        rc = KSocketHandleSelectCallWin ( rcReading );
    else if ( selectRes == 0 )
        rc = RC ( rcNS, rcSocket, rcReading, rcTimeout, rcExhausted ); /* timeout */
    else if ( ! FD_ISSET( data -> fd, &readFds ) )
        rc = HandleErrno (); /* cant determine return codes of FD_ISSET */
    else
    {
        while ( rc == 0 )
        {
            ssize_t count = recv ( data -> fd, buffer, ( int )bsize, 0 );

            if ( count < 0 )
            {
                rc = KSocketHandleRecvCallWin ( rcReading );
                break;
            }
            else
            {
                if ( num_read != NULL );
                    * num_read = ( size_t ) count;
                return 0;
            }
        }
    }
            
    return rc;
}


static rc_t CC KIpv4SocketRead ( const KSocket * self, void * buffer, size_t bsize, size_t * num_read )
{
    timeout_t tm;

    if ( self->type != epIPV4 )
        return RC ( rcNS, rcSocket, rcReading, rcParam, rcInvalid );

    if ( self -> read_timeout < 0 )
        return KIpv4SocketTimedRead ( self, buffer, bsize, num_read, NULL );

    TimeoutInit ( & tm, self -> read_timeout );
    return KIpv4SocketTimedRead ( self, buffer, bsize, num_read, & tm );
}

static
rc_t KSocketHandleSendCallWin ()
{
    switch ( WSAGetLastError () )
    {
    case WSANOTINITIALISED:
        return RC ( rcNS, rcSocket, rcWriting, rcEnvironment, rcInvalid );
    case WSAENETDOWN:
    case WSAEHOSTUNREACH:
        return RC ( rcNS, rcSocket, rcWriting, rcConnection, rcNotAvailable );
    case WSAEINVAL:
    case WSAEFAULT:
        return RC ( rcNS, rcSocket, rcWriting, rcParam, rcInvalid );
    case WSAEACCES:
        return RC ( rcNS, rcSocket, rcWriting, rcSocket, rcUnauthorized );
    case WSAEINTR: /* canceled through WSACancelBlockingCall */
        return 0;
    case WSAEINPROGRESS:
        return RC ( rcNS, rcSocket, rcWriting, rcConnection, rcBusy );
    case WSAENETRESET:
        return RC ( rcNS, rcSocket, rcWriting, rcConnection, rcCanceled );
    case WSAENOBUFS:
        return RC ( rcNS, rcSocket, rcOpening, rcBuffer, rcExhausted );
    case WSAENOTCONN:
        return RC ( rcNS, rcSocket, rcWriting, rcConnection, rcInvalid );
    case WSAENOTSOCK:
        return RC ( rcNS, rcSocket, rcWriting, rcSocket, rcIncorrect );
    case WSAEOPNOTSUPP:
        return RC ( rcNS, rcSocket, rcWriting, rcParam, rcIncorrect );
    case WSAESHUTDOWN:
        return RC ( rcNS, rcSocket, rcWriting, rcSocket, rcNotOpen );
    case WSAEWOULDBLOCK:
        return RC ( rcNS, rcSocket, rcWriting, rcData, rcNotAvailable );
    case WSAEMSGSIZE:
        return RC ( rcNS, rcSocket, rcWriting, rcData, rcExcessive );
    case WSAECONNABORTED:
        return RC ( rcNS, rcSocket, rcWriting, rcConnection, rcCanceled );
    case WSAECONNRESET:
        return RC ( rcNS, rcSocket, rcWriting, rcConnection, rcCanceled );
    case WSAETIMEDOUT:
        return RC ( rcNS, rcSocket, rcWriting, rcTimeout, rcExhausted );
    }
    
    return RC ( rcNS, rcSocket, rcWriting, rcError, rcUnknown );
 }

static rc_t CC KIpv4SocketTimedWrite ( KSocket * self, const void * buffer, size_t bsize,
                                       size_t * num_writ, timeout_t * tm )
{
    RC_CTX ( rcSocket, rcWriting );

    /* self != NULL and self->type == epIPV4 already checked by the caller */

    rc_t rc = 0;
    KSocketIPv4 * data = &( self -> type_data.ipv4_data );
    struct timeval ts;
    fd_set writeFds;
    int selectRes;
    
    /* convert timeout (relative time) */
    if ( tm != NULL )
    {
        ts.tv_sec = tm -> mS / 1000;
        ts.tv_usec = ( tm -> mS % 1000 ) * 1000;
    }
    
    /* wait for socket to become writable */
    FD_ZERO( &writeFds );
    FD_SET( data -> fd, &writeFds );
    selectRes = select( 0, NULL, &writeFds, NULL, ( tm == NULL ) ? NULL : &ts );
    
    /* check for error */
    if ( selectRes == -1 )
        rc = KSocketHandleSelectCallWin ( rcWriting );
    else if ( selectRes == 0 )
        rc = RC ( rcNS, rcSocket, rcWriting, rcTimeout, rcExhausted ); /* timeout */
    else if ( ! FD_ISSET( data -> fd, &writeFds ) )
        rc = HandleErrno (); /* cannot determine return codes of FD_ISSET */
    {
        while ( rc == 0 )
        {
            ssize_t count = send ( data -> fd , buffer, ( int )bsize, 0 );
            if ( count < 0 )
                rc = KSocketHandleSendCallWin ();
            else
            {
                if ( num_writ != NULL )
                    * num_writ = count;
                return 0;
            }

            break;        
        }
    }

    return rc;
}


static rc_t CC KIpv4SocketWrite ( KSocket *self, const void *buffer, size_t bsize, size_t *num_writ )
{
    timeout_t tm;

    if ( self->type != epIPV4 )
        return RC ( rcNS, rcSocket, rcReading, rcParam, rcInvalid );

    if ( self -> write_timeout < 0 )
        return KIpv4SocketTimedWrite ( self, buffer, bsize, num_writ, NULL );

    TimeoutInit ( &tm, self -> write_timeout );
    return KIpv4SocketTimedWrite ( self, buffer, bsize, num_writ, & tm );
}

static KStream_vt_v1 vtKIpv4Socket =
{
    1, 1,
    KIpv4SocketWhack,
    KIpv4SocketRead,
    KIpv4SocketWrite,
    KIpv4SocketTimedRead,
    KIpv4SocketTimedWrite
};

void KStreamForceSocketClose(const struct KStream *self) {
    assert(self);
    closesocket(((KSocket*)self)->type_data.ipv4_data.fd);
}

/* *********************************************************************************************

    IPv4 implementation :

    KNSManagerMakeIPv4Connection()  ... called from KNSManagerMakeRetryTimedConnection()
    KNSManagerMakeIPv4Listener()    ... called from KNSManagerMakeListener()
    KListenerIPv4Accept()           ... called from KListenerAccept()
    KSocketGetEndpointV4()          ... called from KSocketGetEndpoint()

********************************************************************************************* */

static
rc_t KSocketHandleSocketCallWin ()
{
    switch ( WSAGetLastError () )
    {
    case WSANOTINITIALISED:
        return RC ( rcNS, rcSocket, rcCreating, rcEnvironment, rcInvalid );
    case WSAENETDOWN: /* ENETUNREACH */
        return RC ( rcNS, rcSocket, rcCreating, rcConnection, rcNotAvailable );
    case WSAEAFNOSUPPORT:
    case WSAEPROTONOSUPPORT:
    case WSAEPROTOTYPE:
        return RC ( rcNS, rcSocket, rcCreating, rcInterface, rcUnsupported );
    case WSAEINPROGRESS:
        return RC ( rcNS, rcSocket, rcCreating, rcConnection, rcBusy );
    case WSAEMFILE:
        return RC ( rcNS, rcSocket, rcCreating, rcFileDesc, rcExhausted );
    case WSAEINVAL:
        return RC ( rcNS, rcSocket, rcCreating, rcParam, rcInvalid );
    case WSAENOBUFS:
        return RC ( rcNS, rcSocket, rcCreating, rcBuffer, rcExhausted );
    case WSAESOCKTNOSUPPORT:
        return RC ( rcNS, rcSocket, rcCreating, rcSocket, rcIncorrect );
#if 0
    case WSAEINVALIDPROVIDER:
        return /* service provider returned a version other than 2.2 */
    case WSAEINVALIDPROCTABLE:
        return /* service provider returned an invalid or incomplete procedure table to WSPStartup */
    case WSAEPROVIDERFAILEDINIT:
        return /* service provider failed to initialize */
#endif
    }

    return RC ( rcNS, rcSocket, rcCreating, rcError, rcUnknown );
}

static
rc_t KSocketHandleBindCallWin ()
{
    switch ( WSAGetLastError () )
    {
    case WSANOTINITIALISED:
        return RC ( rcNS, rcSocket, rcCreating, rcEnvironment, rcInvalid );
    case WSAENETDOWN: /* ENETUNREACH */
        return RC ( rcNS, rcSocket, rcCreating, rcConnection, rcNotAvailable );
    case WSAEADDRINUSE:
        return RC ( rcNS, rcSocket, rcCreating, rcSocket, rcBusy );
    case WSAEADDRNOTAVAIL:
        return RC ( rcNS, rcSocket, rcCreating, rcSocket, rcNotAvailable );
    case WSAEINVAL:
    case WSAEFAULT:
        return RC ( rcNS, rcSocket, rcCreating, rcParam, rcInvalid );
    case WSAEINPROGRESS:
        return RC ( rcNS, rcSocket, rcCreating, rcConnection, rcBusy );
    case WSAENOBUFS:
        return RC ( rcNS, rcSocket, rcCreating, rcBuffer, rcExhausted );
    case WSAENOTSOCK:
        return RC ( rcNS, rcSocket, rcCreating, rcSocket, rcIncorrect );
    }

    return RC ( rcNS, rcSocket, rcCreating, rcError, rcUnknown );
}

static
rc_t KSocketHandleConnectCallWin ()
{
    switch ( WSAGetLastError () )
    {
    case WSANOTINITIALISED:
        return RC ( rcNS, rcSocket, rcCreating, rcEnvironment, rcInvalid );
    case WSAENETDOWN: /* ENETUNREACH */
    case WSAENETUNREACH:
    case WSAEHOSTUNREACH:
        return RC ( rcNS, rcSocket, rcCreating, rcConnection, rcNotAvailable );
    case WSAEADDRINUSE:
        return RC ( rcNS, rcSocket, rcCreating, rcSocket, rcBusy );
    case WSAEINTR: /* canceled through WSACancelBlockingCall */
        return 0;
    case WSAEINPROGRESS:
        return RC ( rcNS, rcSocket, rcCreating, rcConnection, rcBusy );
    case WSAEALREADY:
        return RC ( rcNS, rcSocket, rcCreating, rcSocket, rcBusy );
    case WSAEADDRNOTAVAIL:
        return RC ( rcNS, rcSocket, rcCreating, rcSocket, rcNotAvailable );
    case WSAEAFNOSUPPORT:
        return RC ( rcNS, rcSocket, rcCreating, rcInterface, rcUnsupported );
    case WSAECONNREFUSED:
        return RC ( rcNS, rcSocket, rcCreating, rcConnection, rcFailed );
    case WSAEINVAL:
    case WSAEFAULT:
        return RC ( rcNS, rcSocket, rcCreating, rcParam, rcInvalid );
    case WSAEISCONN:
        return RC ( rcNS, rcSocket, rcCreating, rcSocket, rcBusy );
    case WSAENOTSOCK:
        return RC ( rcNS, rcSocket, rcCreating, rcSocket, rcIncorrect );
    case WSAETIMEDOUT:
        return RC ( rcNS, rcSocket, rcCreating, rcTimeout, rcExhausted );
    case WSAEWOULDBLOCK:
        return RC ( rcNS, rcSocket, rcCreating, rcData, rcNotAvailable );
    case WSAEACCES: /* failed to connect to socket because setsockopt SO_BROADCAST is not enabled */
        return RC ( rcNS, rcNoTarg, rcCreating, rcMemory, rcUnauthorized );            
    }

    return RC ( rcNS, rcSocket, rcCreating, rcError, rcUnknown );
}

static 
rc_t KSocketHandleListenCallWin ()
{
    switch ( WSAGetLastError () )
    {
    case WSANOTINITIALISED:
        return RC ( rcNS, rcSocket, rcCreating, rcEnvironment, rcInvalid );
    case WSAENETDOWN: /* ENETUNREACH */
        return RC ( rcNS, rcSocket, rcCreating, rcConnection, rcNotAvailable );
    case WSAEADDRINUSE:
        return RC ( rcNS, rcSocket, rcCreating, rcSocket, rcBusy );
    case WSAEINPROGRESS:
        return RC ( rcNS, rcSocket, rcCreating, rcConnection, rcBusy );
    case WSAEINVAL:
        return RC ( rcNS, rcSocket, rcCreating, rcParam, rcInvalid );
    case WSAEISCONN:
        return RC ( rcNS, rcSocket, rcCreating, rcSocket, rcBusy );
    case WSAEMFILE:
        return RC ( rcNS, rcSocket, rcCreating, rcFileDesc, rcExhausted );
    case WSAENOBUFS:
        return RC ( rcNS, rcSocket, rcCreating, rcBuffer, rcExhausted );
    case WSAENOTSOCK:
        return RC ( rcNS, rcSocket, rcCreating, rcSocket, rcIncorrect );
    case WSAEOPNOTSUPP: /* socket is not stream-style such as SOCK_STREAM */
        return RC ( rcNS, rcSocket, rcCreating, rcParam, rcIncorrect );
    }

    return RC ( rcNS, rcSocket, rcCreating, rcError, rcUnknown );
}

static
rc_t KSocketHandleAcceptCallWin ()
{
    switch ( WSAGetLastError () )
    {
    case WSANOTINITIALISED:
        return RC ( rcNS, rcSocket, rcOpening, rcEnvironment, rcInvalid );
    case WSAECONNRESET:
        return RC ( rcNS, rcSocket, rcOpening, rcConnection, rcCanceled );
    case WSAEINVAL:
    case WSAEFAULT:
        return RC ( rcNS, rcSocket, rcOpening, rcParam, rcInvalid );
    case WSAEINTR: /* canceled through WSACancelBlockingCall */
        return 0;
    case WSAEINPROGRESS:
        return RC ( rcNS, rcSocket, rcOpening, rcConnection, rcBusy );
    case WSAEMFILE:
        return RC ( rcNS, rcSocket, rcOpening, rcFileDesc, rcExhausted );
    case WSAENETDOWN: /* ENETUNREACH */
        return RC ( rcNS, rcSocket, rcOpening, rcConnection, rcNotAvailable );
    case WSAENOBUFS:
        return RC ( rcNS, rcSocket, rcOpening, rcBuffer, rcExhausted );
    case WSAENOTSOCK:
        return RC ( rcNS, rcSocket, rcOpening, rcSocket, rcIncorrect );
    case WSAEOPNOTSUPP: /* socket is not stream-style such as SOCK_STREAM */
        return RC ( rcNS, rcSocket, rcOpening, rcParam, rcIncorrect );
    case WSAEWOULDBLOCK:
        return RC ( rcNS, rcSocket, rcOpening, rcData, rcNotAvailable );
    }

    return RC ( rcNS, rcSocket, rcOpening, rcError, rcUnknown );
}

static
rc_t KSocketHandleSocknameCallWin ()
{
    switch ( WSAGetLastError () )
    {
    case WSANOTINITIALISED:
        return RC ( rcNS, rcSocket, rcIdentifying, rcEnvironment, rcInvalid );
    case WSAENETDOWN: /* ENETUNREACH */
        return RC ( rcNS, rcSocket, rcIdentifying, rcConnection, rcNotAvailable );        
    case WSAEINVAL:
    case WSAEFAULT:
        return RC ( rcNS, rcSocket, rcIdentifying, rcParam, rcInvalid );
    case WSAEINPROGRESS:
        return RC ( rcNS, rcSocket, rcIdentifying, rcConnection, rcBusy );
    case WSAENOTSOCK:
        return RC ( rcNS, rcSocket, rcIdentifying, rcSocket, rcIncorrect );
    }

    return RC ( rcNS, rcSocket, rcIdentifying, rcError, rcUnknown );
}

static
rc_t KSocketConnectIPv4 ( KSocket * self, const KEndPoint * from, const KEndPoint * to )
{
    rc_t rc = 0;

    self -> type_data . ipv4_data . fd = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if ( self -> type_data . ipv4_data . fd == INVALID_SOCKET )
        rc = KSocketHandleSocketCallWin ();
    else
    {
        struct sockaddr_in ss;
        memset ( & ss, 0, sizeof ss );
        ss . sin_family = AF_INET;
        if ( from != NULL )
        {
            ss . sin_port = htons ( from -> u. ipv4 . port );
            ss . sin_addr . s_addr = htonl ( from -> u . ipv4 . addr );
        }

        if ( bind ( self -> type_data . ipv4_data . fd, ( struct sockaddr * ) & ss, sizeof ss  ) == SOCKET_ERROR ) 
            rc = KSocketHandleBindCallWin ();
                
        if ( rc == 0 )
        {
            ss . sin_port = htons ( to -> u . ipv4 . port );
            ss . sin_addr . s_addr = htonl ( to -> u . ipv4 . addr );
                
            if ( connect ( self -> type_data . ipv4_data . fd, ( struct sockaddr * ) & ss, sizeof ss ) == SOCKET_ERROR )
                rc = KSocketHandleConnectCallWin ();
            else
            {
                /* not doing non-blocking */
                return 0;
            }

            /* dump socket */
            closesocket ( self -> type_data . ipv4_data . fd );
            self -> type_data . ipv4_data . fd = INVALID_SOCKET;
        }
    }

    DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_SOCKET), ( "%p: KSocketConnectIPv4 failed - %R\n", self, rc ) );    
    return rc;
}


static rc_t KNSManagerMakeIPv4Listener ( const KNSManager *self, KSocket **out, const KEndPoint * ep )
{
    rc_t rc = 0;
    KSocket * listener = calloc ( 1, sizeof * listener );
    if ( listener == NULL )
        rc = RC ( rcNS, rcSocket, rcConstructing, rcMemory, rcExhausted );
    else
    {
        listener -> type = epIPV4;

        /* pass these along to accepted sockets */
        listener -> read_timeout = self -> conn_read_timeout;
        listener -> write_timeout = self -> conn_write_timeout;

        rc = KStreamInit ( & listener -> dad, ( const KStream_vt* ) & vtKIpv4Socket,
                           "KSocket", "", true, true );
        if ( rc == 0 )
        {
            KSocketIPv4 * data = &( listener -> type_data . ipv4_data );
            data -> fd = socket ( AF_INET, SOCK_STREAM, 0 );
            if ( data -> fd < 0 )
                rc = KSocketHandleSocketCallWin ();
            else
            {
                struct sockaddr_in ss;

                int on = 1;
                setsockopt ( data -> fd, SOL_SOCKET, SO_REUSEADDR, ( char* ) & on, sizeof on );

                memset ( & ss, 0, sizeof ss );
                ss . sin_family = AF_INET;
                ss . sin_addr . s_addr = htonl ( ep -> u . ipv4 . addr );
                ss . sin_port = htons ( ep -> u . ipv4 . port );

                if ( bind ( data -> fd, ( struct sockaddr* ) & ss, sizeof ss ) != 0 )
                    rc = KSocketHandleBindCallWin ();
                else if ( listen ( data -> fd, 5 ) != 0 )
                    rc = KSocketHandleListenCallWin (); 
                else
                {
                    * out = listener;
                    return 0;
                }

                closesocket ( data -> fd );
                data -> fd = -1;
            }
        }

        free( listener );
    }
    return rc;
}


static rc_t KListenerIPv4Accept ( KSocket * self, struct KSocket ** out )
{
    rc_t rc = 0;
    KSocket * new_socket = calloc ( 1, sizeof * new_socket );
    if ( new_socket == NULL )
        rc = RC ( rcNS, rcConnection, rcWaiting, rcMemory, rcExhausted );
    else
    {
        new_socket -> type = epIPV4;

        new_socket -> read_timeout = self -> read_timeout;
        new_socket -> write_timeout = self -> write_timeout;

        rc = KStreamInit ( & new_socket -> dad, ( const KStream_vt* ) & vtKIpv4Socket,
                           "KSocket", "tcp", true, true );
        if ( rc == 0 )
        {
            int len;
            KSocketIPv4 * new_data = &( new_socket -> type_data . ipv4_data );
            KSocketIPv4 * self_data = &( self -> type_data . ipv4_data );

            new_data -> remote_addr_valid = false;
            len = sizeof new_data -> remote_addr;

            new_data -> fd = accept ( self_data -> fd, ( struct sockaddr * ) & new_data -> remote_addr, & len );
            if ( new_data -> fd < 0 )
                rc = KSocketHandleAcceptCallWin ();
            else if ( len <= sizeof new_data -> remote_addr )
                new_data -> remote_addr_valid = true;
            else
            {
                closesocket ( new_data -> fd );
                new_data -> fd = -1;
                rc = RC ( rcNS, rcConnection, rcWaiting, rcBuffer, rcInsufficient );
            }

            if ( rc == 0 )
            {
                * out = new_socket;
                return 0;
            }
        }
        free( new_socket );
    }
    return rc;
}


static rc_t KSocketGetEndpointV4 ( const KSocket * self, KEndPoint * ep, bool remote )
{
    rc_t rc = 0;
    const KSocketIPv4 * data = &( self -> type_data . ipv4_data );
    struct sockaddr_in addr;
    int l = sizeof( addr );
    int res = 0;

    if ( ! remote )
        res = getsockname( data -> fd, ( struct sockaddr * )&addr, &l );
    else if ( ! data -> remote_addr_valid )
        res = getpeername( data -> fd, ( struct sockaddr * )&addr, &l );
    else
    {
        /* the remote part was already recorded through calling accept() */
        addr . sin_addr . s_addr = data -> remote_addr . sin_addr . s_addr;
        addr . sin_port        = data -> remote_addr . sin_port;
    }

    if ( res == 0 )
    {
        ep -> u . ipv4.addr = ntohl( addr . sin_addr . s_addr );
        ep -> u . ipv4.port = ntohs( addr . sin_port );
        ep -> type = epIPV4;
        string_copy_measure ( ep -> ip_address, sizeof ep -> ip_address,
                              inet_ntoa ( addr . sin_addr ) );
        return 0;
    }

    rc = KSocketHandleSocknameCallWin ();

    ep -> type = epInvalid;

    return rc;
}


/* *********************************************************************************************

    IPv6 implementation :

    KNSManagerMakeIPv6Connection()  ... called from KNSManagerMakeRetryTimedConnection()
    KNSManagerMakeIPv6Listener()    ... called from KNSManagerMakeListener()
    KListenerIPv6Accept()           ... called from KListenerAccept()
    KSocketGetEndpointV6()          ... called from KSocketGetEndpoint()

********************************************************************************************* */

static
rc_t KSocketConnectIPv6 ( KSocket * self, const KEndPoint * from, const KEndPoint * to )
{
    return RC ( rcNS, rcSocket, rcConstructing, rcFunction, rcUnsupported ); 
}


static rc_t KNSManagerMakeIPv6Listener ( const KNSManager *self, KSocket **out, const KEndPoint * ep )
{
    rc_t rc = 0;
    KSocket * listener = calloc ( 1, sizeof * listener );
    if ( listener == NULL )
        rc = RC ( rcNS, rcSocket, rcConstructing, rcMemory, rcExhausted );
    else
    {
        listener -> type = epIPV6;

        /* pass these along to accepted sockets */
        listener -> read_timeout = self -> conn_read_timeout;
        listener -> write_timeout = self -> conn_write_timeout;

        rc = KStreamInit ( & listener -> dad, ( const KStream_vt* ) & vtKIpv4Socket,
                           "KSocket", "", true, true );
        if ( rc == 0 )
        {
            KSocketIPv6 * data = &( listener -> type_data.ipv6_data );

            data -> fd = socket ( AF_INET6, SOCK_STREAM, 0 );
            if ( data -> fd < 0 )
                rc = KSocketHandleSocketCallWin ();
            else
            {
                struct sockaddr_in6 ss;

                int on = 1;
                setsockopt ( data -> fd, SOL_SOCKET, SO_REUSEADDR, ( char* ) & on, sizeof on );

                memset ( & ss, 0, sizeof ss );
                ss . sin6_family = AF_INET6;
                memmove ( ss . sin6_addr . s6_addr,
                         ep -> u . ipv6 . addr,
                         sizeof ( ep -> u . ipv6 . addr ) );
                ss . sin6_port = htons ( ep -> u . ipv6 . port );

                if ( bind ( data -> fd, ( struct sockaddr* ) & ss, sizeof ss ) != 0 )
                    rc = KSocketHandleBindCallWin ();
                else if ( listen ( data -> fd, 5 ) != 0 )
                    rc = KSocketHandleListenCallWin (); 
                else
                {
                    * out = listener;
                    return 0;
                }

                closesocket ( data -> fd );
                data -> fd = -1;
            }
        }
    }

    return rc;
}


static rc_t KListenerIPv6Accept ( KSocket * self, struct KSocket ** out )
{
    rc_t rc = 0;
    KSocket * new_socket = calloc ( 1, sizeof * new_socket );
    if ( new_socket == NULL )
        rc = RC ( rcNS, rcConnection, rcWaiting, rcMemory, rcExhausted );
    else
    {
        new_socket -> type = epIPV6;

        new_socket -> read_timeout = self -> read_timeout;
        new_socket -> write_timeout = self -> write_timeout;

        rc = KStreamInit ( & new_socket -> dad, ( const KStream_vt* ) & vtKIpv4Socket,
                           "KSocket", "tcp", true, true );
        if ( rc == 0 )
        {
            int len;
            KSocketIPv6 * new_data = &( new_socket -> type_data . ipv6_data );
            KSocketIPv6 * self_data = &( self -> type_data . ipv6_data );

            new_data -> remote_addr_valid = false;
            len = sizeof new_data -> remote_addr;

            new_data -> fd = accept ( self_data -> fd, ( struct sockaddr * ) & new_data -> remote_addr, & len );
            if ( new_data -> fd < 0 )
                rc = KSocketHandleAcceptCallWin ();
            else if ( len <= sizeof new_data -> remote_addr )
                new_data -> remote_addr_valid = true;
            else
            {
                closesocket ( new_data -> fd );
                new_data -> fd = -1;
                rc = RC ( rcNS, rcConnection, rcWaiting, rcBuffer, rcInsufficient );
            }

            if ( rc == 0 )
            {
                * out = new_socket;
                return 0;
            }
        }
        free( new_socket );
    }
    return rc;
}


static rc_t KSocketGetEndpointV6 ( const KSocket * self, KEndPoint * ep, bool remote )
{
    rc_t rc = 0;
    const KSocketIPv6 * data = &( self -> type_data.ipv6_data );
    struct sockaddr_in6 addr;
    int l = sizeof( addr );
    int res = 0;

    if ( ! remote )
        res = getsockname( data -> fd, ( struct sockaddr * )&addr, &l );
    else if ( ! data -> remote_addr_valid )
        res = getpeername( data -> fd, ( struct sockaddr * )&addr, &l );
    else
    {
        /* the remote part was already recorded through calling accept() */
        memmove ( ep -> u . ipv6 . addr,
                 data -> remote_addr . sin6_addr . s6_addr,
                 sizeof ( ep -> u . ipv6 . addr ) );
        ep->u.ipv6.port = ntohs( data -> remote_addr . sin6_port );
        ep->type = epIPV6;
        return 0;
    }

    if ( res == 0 )
    {
        memmove ( ep -> u . ipv6 . addr,
                 addr . sin6_addr . s6_addr,
                 sizeof ( ep -> u . ipv6 . addr ) );
        ep -> u.ipv6.port = ntohs( addr . sin6_port );
        ep -> type = epIPV6;
        return 0;
    }

    rc = KSocketHandleSocknameCallWin ();

    ep -> type = epInvalid;

    return rc;
}


/* *********************************************************************************************

    IPC implementation :

    static functions for vtable
    vtable

    KNSManagerMakeIPCConnection()   ... called from KNSManagerMakeRetryTimedConnection()
    KNSManagerMakeIPCListener()     ... called from KNSManagerMakeListener()
    KListenerIPCAccept()            ... called from KListenerAccept()

********************************************************************************************* */

/* helper function called by KIPCSocketWhack() = static function for IPC-implementation vtable */
static rc_t KIPCSocketWhack_unconnected_server_side_pipe ( KSocket * self )
{
    RC_CTX ( rcSocket, rcClosing );

    rc_t rc = 0;
    KSocketIPC * data = &( self -> type_data.ipc_data );

    DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: isIpcListener\n", self ) );    
    if ( data->listenerPipe != INVALID_HANDLE_VALUE )
    {
        /* !!! In case there is an active call to ConnectNamedPipe()
           on some thread, "wake" the synchronous named pipe,
           otherwise DisconnectNamedPipe/CloseHandle will block forever */
        HANDLE hPipe = CreateFileW( data->pipename, 
                                    GENERIC_READ, 
                                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 
                                    NULL, 
                                    OPEN_EXISTING, 
                                    0, 
                                    NULL);

        if ( hPipe != INVALID_HANDLE_VALUE )
            CloseHandle( hPipe );

        /* now, Disconnect/Close the original pipe */
        if ( !DisconnectNamedPipe( data->listenerPipe ) )
        {
            rc = HandleErrno();
            DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: DisconnectNamedPipe failed\n", self ) );
        }
        
        if ( !CloseHandle( data->listenerPipe ) )
        {
            rc = HandleErrno();
            DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: CloseHandle failed\n", self ) );
        }
    }
    return rc;
}


/* helper function called by KIPCSocketWhack() = static function for IPC-implementation vtable */
static rc_t KIPCSocketWhack_server_side_pipe ( KSocket * self )
{
    RC_CTX ( rcSocket, rcClosing );

    rc_t rc = 0;
    KSocketIPC * data = &( self -> type_data.ipc_data );

    DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: isIpcPipeServer\n", self ) );
    if ( !FlushFileBuffers( data->pipe ) )
    {
        if ( GetLastError() != ERROR_BROKEN_PIPE )
        {
            rc = HandleErrno();
            DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: FlushFileBuffers failed, err=%d\n", self, GetLastError() ) );    
        }
    }
    if ( !DisconnectNamedPipe( data -> pipe ) )
    {
        rc = HandleErrno();
        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: DisconnectNamedPipe failed\n", self ) );    
    }
    if ( !CloseHandle( data -> pipe ) )
    {
        rc = HandleErrno();
        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: CloseHandle failed\n", self ) );
    }
    return rc;
}


/* helper function called by KIPCSocketWhack() = static function for IPC-implementation vtable */
static rc_t KIPCSocketWhack_client_side_pipe ( KSocket * self )
{
    RC_CTX ( rcSocket, rcClosing );

    rc_t rc = 0;
    KSocketIPC * data = &( self -> type_data.ipc_data );

    DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: isIpcPipeClient\n", self ) );
    if ( !CloseHandle( data -> pipe ) )
    {
        rc = HandleErrno();
        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: CloseHandle failed\n", self ) );
    }
    return rc;
}


/* static function for IPC-implementation vtable */
static rc_t CC KIPCSocketWhack ( KSocket * self )
{
    rc_t rc = 0;
    /* we tolerate a whack on a NULL-pointer */
    if ( self == NULL ) return rc;

    DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: KIPCSocketWhack()...\n", self ) ); 
    switch ( self -> type_data.ipc_data.type )
    {
        case isIpcListener   : rc = KIPCSocketWhack_unconnected_server_side_pipe ( self ); break;

        case isIpcPipeServer : rc = KIPCSocketWhack_server_side_pipe ( self ); break;

        case isIpcPipeClient : rc = KIPCSocketWhack_client_side_pipe ( self ); break;
    }
    
    free ( self );
    return rc;
}


/* helper function called by KIPCSocketTimedRead() = static function for IPC-implementation vtable */
static rc_t WaitForData( const KSocket * self, void * buffer, size_t bsize,
                         size_t * num_read, uint32_t * tmMs, OVERLAPPED * overlap )
{
    RC_CTX ( rcSocket, rcReading );

    /* received a ERROR_NO_DATA trying to read from a pipe; wait for the data to arrive or a time out to expire */ 
    /* on success, will leave tmMs set to the remaining portion of timeout, if specified */

    const KSocketIPC * data = &( self -> type_data.ipc_data );
    uint32_t tm_decrement = 100; 

    DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), 
            ( "%p: no data on the pipe - going into a wait loop, tm=%d\n", 
                self, tmMs == 0 ? -1 : *tmMs ) );
    while ( true )
    {
        BOOL ret;
        DWORD count;
    
        if ( tmMs != NULL )
        {
            if ( *tmMs <= tm_decrement )
            {
                CloseHandle( overlap -> hEvent );
                return RC ( rcNS, rcFile, rcReading, rcTimeout, rcExhausted );
            }
            *tmMs -= tm_decrement;
        }

        /* *usually* returns FALSE in asynch mode */
        ret = ReadFile( data -> pipe, buffer, ( DWORD )bsize, &count, overlap );
        if ( ret )
        {
            DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), 
                    ( "%p: (wait loop) ReadFile completed synchronously, count=%d\n", 
                        self, count ) );
            assert ( num_read != NULL );
            * num_read = ( size_t ) count;
            CloseHandle( overlap -> hEvent );
            return 0;
        }
        
        switch ( GetLastError() )
        {
            case ERROR_IO_PENDING : return 0; /* the caller will wait for completion */
            
            case ERROR_NO_DATA :
                DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: (wait loop) Sleep(%d)\n", self, tm_decrement ) );
                Sleep( tm_decrement );
                break;
            
            case ERROR_SUCCESS: /* not expected in asynch mode */
                return RC ( rcNS, rcFile, rcReading, rcError, rcUnexpected);
            
            default:
                return HandleErrno();
        }
    }
    return 0;
}


/* static function for IPC-implementation vtable */
static rc_t CC KIPCSocketTimedRead ( const KSocket * self, void * buffer, size_t bsize,
                                     size_t * num_read, timeout_t * tm )
{
    RC_CTX ( rcSocket, rcReading );

    rc_t rc = 0;
    const KSocketIPC * data = &( self -> type_data.ipc_data );
    OVERLAPPED overlap;
    
    if ( num_read == NULL )
        return RC ( rcNS, rcConnection, rcReading, rcParam, rcNull );

    DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), 
                ( "%p: KIPCSocketTimedRead(%d, %p, %d)... \n",
                  self, tm == NULL ? -1 : tm -> mS, buffer, bsize ) );

    /* TODO: wait for pipe to become readable? */
    memset( &overlap, 0, sizeof( overlap ) );
    overlap.hEvent = CreateEvent( NULL,     /* default security attribute */
                                  TRUE,     /* manual reset event */
                                  FALSE,    /* initial state = nonsignalled */
                                  NULL ); 
    if ( overlap.hEvent != NULL )
    {
        DWORD count;

        /* *usually* returns FALSE in asynch mode */
        BOOL ret = ReadFile( data->pipe, buffer, ( DWORD )bsize, &count, &overlap );
        if ( ret )
        {
            /* done: must be synch mode */
            DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: ReadFile completed synchronously, count=%d\n", self, count ) );
            * num_read = ( size_t ) count;
            CloseHandle( overlap.hEvent );
            return 0;
        }
        
        *num_read = 0;

        /* asynch mode - wait for the operation to complete */
        if ( GetLastError() == ERROR_NO_DATA ) /* 232 */
        {
            DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), 
                ( "%p: ReadFile(%x) returned FALSE, GetLastError() = ERROR_NO_DATA\n", self, data -> pipe ) );
            rc = WaitForData( self, buffer, bsize, num_read, tm == NULL ? NULL : &tm -> mS, &overlap );
            if ( *num_read != 0 ) /* read completed*/
            {
                CloseHandle( overlap.hEvent );
                return 0;
            }
            if ( rc != 0 )
            {
                CloseHandle( overlap.hEvent );
                return rc;
            }
        }   

        if ( GetLastError() == ERROR_IO_PENDING ) /* 997 */
        {
            DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), 
                ( "%p: ReadFile(%x) returned FALSE, GetLastError() = ERROR_IO_PENDING\n",
                        self, data -> pipe ) );

            if ( tm == NULL )
                DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: waiting forever\n", self ) );
            else
                DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: waiting for %d ms\n", self, tm -> mS ) );
                
            switch ( WaitForSingleObject( overlap.hEvent, tm == NULL ? INFINITE : tm -> mS ) )
            {
                case WAIT_TIMEOUT :
                    DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: timed out\n", self ) );
                    rc = RC ( rcNS, rcFile, rcReading, rcTimeout, rcExhausted );
                    break;
                    
                case WAIT_OBJECT_0 :
                {
                    DWORD count;
                    DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: successful\n", self ) );
                    /* wait to complete if necessary */
                    if ( GetOverlappedResult( data->pipe, &overlap, &count, TRUE ) )
                    {
                        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: %d bytes read\n", self, count ) );
                        * num_read = ( size_t ) count;
                        rc = 0;
                    }
                    else
                    {
                        rc = HandleErrno();
                        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: GetOverlappedResult() failed\n", self ) );
                    }
                    break;
                }
                
                default:
                    rc = HandleErrno();
                    DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: WaitForSingleObject() failed\n", self ) );
                    break;
            }
        }
        else if ( GetLastError() == ERROR_SUCCESS )
        {
            DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), 
                        ( "%p: ReadFile(%x) returned FALSE, GetLastError() = ERROR_SUCCESS\n",
                        self, data -> pipe ) );
            rc = RC ( rcNS, rcFile, rcReading, rcError, rcUnexpected );
        }
        else
        {
            rc = HandleErrno();
        }
        CloseHandle( overlap.hEvent );
    }
    else
        rc = HandleErrno();

    return rc;
}


/* static function for IPC-implementation vtable */
static rc_t CC KIPCSocketRead ( const KSocket *self, void * buffer, size_t bsize, size_t *num_read )
{
    timeout_t tm;

    if ( self == NULL )
        return RC ( rcNS, rcConnection, rcReading, rcSelf, rcNull );

    if ( self -> read_timeout < 0 )
        return KIPCSocketTimedRead ( self, buffer, bsize, num_read, NULL );

    TimeoutInit ( & tm, self -> read_timeout );
    return KIPCSocketTimedRead ( self, buffer, bsize, num_read, & tm );
}


/* static function for IPC-implementation vtable */
static rc_t CC KIPCSocketTimedWrite ( KSocket * self, const void * buffer, size_t bsize,
                                      size_t * num_writ, timeout_t * tm )
{
    RC_CTX ( rcSocket, rcWriting );

    rc_t rc = 0;
    KSocketIPC * data;
    OVERLAPPED overlap;
    
    if ( self == NULL )
        return RC ( rcNS, rcConnection, rcWriting, rcSelf, rcNull );

    data = &( self -> type_data.ipc_data );

    DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), 
                ( "%p: KIPCSocketTimedWrite(%d, %d)...", "b=%p,s=%d,t=%d\n",
                self, bsize, tm == NULL ? -1 : tm -> mS ) );

    memset( &overlap, 0, sizeof( overlap ) );

    overlap.hEvent = CreateEvent( NULL,     /* default security attribute */
                                  TRUE,     /* manual reset event */
                                  FALSE,    /* initial state = nonsignalled */
                                  NULL); 
    if ( overlap.hEvent != NULL )
    {
        DWORD count;

        /* returns FALSE in asynch mode */
        BOOL ret = WriteFile( data->pipe, buffer, ( DWORD )bsize, &count, &overlap );
        int err = GetLastError();
        /*DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: WriteFile returned %s, GetError() = %d\n", base, ret ? "TRUE" : "FALSE", err ) ); */

        /* completed synchronously; either message is so short that is went out immediately, or the pipe is full */
        if ( ret )
        {   
            if ( count > 0 )
            {
                DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: %d bytes written\n", self, count ) );
                if ( num_writ != NULL )
                    * num_writ = ( size_t ) count;
                CloseHandle( overlap.hEvent );
                return 0;
            }
            else 
            {
                /* pipe is full - go into a wait loop */
                uint32_t tm_left = ( tm == NULL ) ? 0 : tm -> mS;
                uint32_t tm_decrement = 100; 
                DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), 
                            ( "%p: pipe full - going into a wait loop for %d ms\n", 
                                self, ( tm == NULL ) ? -1 : tm -> mS ) );
                while ( count == 0 )
                {
                    if ( tm != NULL )
                    {
                        if ( tm_left <= tm_decrement )
                        {
                            CloseHandle( overlap.hEvent );
                            return RC ( rcNS, rcFile, rcWriting, rcTimeout, rcExhausted );
                        }
                        tm_left -= tm_decrement;
                    }
                    
                    Sleep( 1 ); /*ms*/
                    
                    DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: write wait loop: attempting to WriteFile\n", self ) );   

                    /* returns FALSE in asynch mode */
                    ret = WriteFile( data->pipe, buffer, ( DWORD )bsize, &count, &overlap );
                    err = GetLastError();
                    /* DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: WriteFile returned %s, GetError() = %d\n", base, ret ? "TRUE" : "FALSE", err ) ); */
                    if ( !ret )
                        break; /* and proceed to handling the asynch mode */
                }
            }
        }
        
        /* asynch mode - wait for the operation to complete */
        switch ( err ) /* set by the last call to WriteFile */
        {
        case NO_ERROR:
        case ERROR_IO_PENDING:
        {
            switch ( WaitForSingleObject( overlap.hEvent, tm == NULL ? INFINITE : tm -> mS ) )
            {
            case WAIT_TIMEOUT:
                DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: timed out\n", self ) );
                CloseHandle( overlap.hEvent );
                return RC ( rcNS, rcStream, rcWriting, rcTimeout, rcExhausted );

            case WAIT_OBJECT_0:
            {
                DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: successful\n", self ) );
                /* wait to complete if necessary */
                if ( GetOverlappedResult( data->pipe, &overlap, &count, TRUE ) )
                {
                    DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: %d bytes written\n", self, count ) );
                    if ( num_writ != NULL )
                        * num_writ = count;
                    CloseHandle( overlap.hEvent );
                    return 0;
                }
                rc = HandleErrno();
                DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: GetOverlappedResult() failed\n", self ) );
                break;
            }
            
            default:
                rc = HandleErrno();
                DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: WaitForSingleObject() failed\n", self ) );
                break;
            }
        }
        case ERROR_NO_DATA:
            /* the secret MS lore says when WriteFile to a pipe returns ERROR_NO_DATA, it's 
                "Pipe was closed (normal exit path)." - see http://support.microsoft.com/kb/190351 */
            CloseHandle( overlap.hEvent );
            return 0;

        default:
            rc = HandleErrno();
            DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: WriteFile() failed\n", self ) );
            break;
        }

        CloseHandle( overlap.hEvent );
    }
    else
        rc = HandleErrno();
        
    return rc;
}


/* static function for IPC-implementation vtable */
static rc_t CC KIPCSocketWrite ( KSocket *self, const void * buffer, size_t bsize, size_t * num_writ )
{
    timeout_t tm;

    if ( self == NULL )
        return RC ( rcNS, rcConnection, rcWriting, rcSelf, rcNull );

    if ( self -> write_timeout < 0 )
        return KIPCSocketTimedWrite ( self, buffer, bsize, num_writ, NULL );

    TimeoutInit ( & tm, self -> write_timeout );
    return KIPCSocketTimedWrite ( self, buffer, bsize, num_writ, & tm );
}


static KStream_vt_v1 vtKIPCSocket =
{
    1, 1,
    KIPCSocketWhack,
    KIPCSocketRead,
    KIPCSocketWrite,
    KIPCSocketTimedRead,
    KIPCSocketTimedWrite
};


static
rc_t KSocketConnectIPC ( KSocket * self, timeout_t * retryTimeout, const KEndPoint * to )
{
    RC_CTX ( rcStream, rcConstructing );

    rc_t rc = 0;
    char pipename[ PIPE_NAME_LENGTH ];
    wchar_t pipenameW[ PIPE_NAME_LENGTH ];
    size_t num_writ;

    /* use named pipes to implement unix domain socket - like behavior */
    rc = string_printf( pipename, sizeof( pipename ), &num_writ, "\\\\.\\pipe\\%s", to->u.ipc_name );
    if ( rc == 0 )
        string_cvt_wchar_copy( pipenameW, sizeof( pipenameW ), pipename, num_writ );
        
    while ( rc == 0 )
    {
        HANDLE h = CreateFileW( pipenameW,                    /* pipe name                   */
                                GENERIC_READ | GENERIC_WRITE, /* read and write access       */
                                0,                            /* no sharing                  */
                                NULL,                         /* default security attributes */
                                OPEN_EXISTING,                /* opens existing pipe         */
                                FILE_FLAG_OVERLAPPED,         /* using overlapped IO         */
                                NULL );                       /* no template file            */
        if ( h != INVALID_HANDLE_VALUE )
        {
            /* create the KSocket */
            /* need NOWAIT if pipe is created in asynch mode */
            DWORD dwMode = ( PIPE_READMODE_MESSAGE | PIPE_NOWAIT );
            if ( ! SetNamedPipeHandleState ( h,        /* pipe handle             */
                                             &dwMode,  /* new pipe mode           */
                                             NULL,     /* don't set maximum bytes */
                                             NULL ) )  /* don't set maximum time  */
            {
                rc = HandleErrno();
            }
            else
            {
                DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), 
                         ( "%p: KNSManagerMakeIPCConnection(%p,'%s')\n", self, to, pipename ) );

                self -> type_data . ipc_data . type = isIpcPipeClient;
                self -> type_data . ipc_data . pipe = h;
            }
        }
        else /* CreateFileW failed */
        {
            switch ( GetLastError() )
            {
            case ERROR_PIPE_BUSY :
                DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "KNSManagerMakeIPCConnection: pipe busy, retrying\n" ) );
                {
                    uint32_t remaining;

                    BOOL pipeAvailable = WaitNamedPipeW( pipenameW, NMPWAIT_USE_DEFAULT_WAIT );
                    if ( pipeAvailable )
                    {
                        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "KNSManagerMakeIPCConnection: WaitNamedPipeW returned TRUE\n" ) );
                        continue;
                    }

                    /* time-out, grab RC from global, then try again */
                    rc = HandleErrno();

                    /* check time remaining on timeout ( if any ) */
                    remaining = TimeoutRemaining ( retryTimeout );

                    /* only retry if time remains */
                    if ( remaining != 0 )
                    {
                        /* never wait for more than the remaining timeout */
                        uint32_t delay = 250;
                        if ( delay > remaining )
                            delay = remaining;

                        KSleepMs ( delay );
                        rc = 0;
                        continue;
                    }
                }
                break;
                
            default:
                rc = HandleErrno();
                break;
            }
        }
        break;
    }

    return rc;
}


static rc_t KNSManagerMakeIPCListener( struct KNSManager const *self, struct KSocket** out,
                                       struct KEndPoint const * ep )
{   
    rc_t rc = 0;
    KSocket * ksock;

    if ( self == NULL )
        return RC ( rcNS, rcConnection, rcCreating, rcSelf, rcNull );
    if ( out == NULL )
        return RC ( rcNS, rcConnection, rcCreating, rcParam, rcNull );

    * out = NULL;

    if ( ep == NULL )
        return RC ( rcNS, rcConnection, rcCreating, rcParam, rcNull );
    if ( ep -> type != epIPC )
        return RC ( rcNS, rcConnection, rcCreating, rcParam, rcInvalid );

    
    /* use named pipes to implement unix domain socket - like behavior */
    ksock = calloc ( sizeof *ksock, 1 );
    if ( ksock == NULL )
        rc = RC ( rcNS, rcNoTarg, rcAllocating, rcNoObj, rcNull ); 
    else
    {
        ksock -> type = epIPC;

        ksock -> read_timeout  = self -> conn_read_timeout;
        ksock -> write_timeout = self -> conn_write_timeout;

        rc = KStreamInit ( & ksock -> dad, ( const KStream_vt* ) & vtKIPCSocket,
                           "KSocket", "tcp", true, true );
        if ( rc == 0 )
        {
            size_t num_writ;
            char pipename[ PIPE_NAME_LENGTH ];
            rc = string_printf( pipename, sizeof( pipename ), &num_writ, "\\\\.\\pipe\\%s", ep -> u . ipc_name );
            if ( rc == 0 )
            {
                KSocketIPC * data = &( ksock -> type_data.ipc_data );
                string_cvt_wchar_copy( data -> pipename, sizeof( data -> pipename ), pipename, num_writ );
                
                data -> type = isIpcListener;
                data -> listenerPipe = INVALID_HANDLE_VALUE;
                ksock -> type = epIPC;
                *out = ( KSocket * )& ksock -> dad;
                
                DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: KNSManagerMakeIPCListener(%p,'%s')\n", 
                            ksock, ep, pipename ) );
                return 0;
            }
            KIPCSocketWhack( ksock );
        }
        else
            free ( ksock );
    }
        
    DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: KNSManagerMakeIPCListener failed\n", ksock ) );
    return rc;
}


static rc_t KListenerIPCAccept ( KSocket * self, struct KSocket ** out )
{
    RC_CTX ( rcConnection, rcWaiting );

    rc_t rc = 0;
    KSocketIPC * data = &( self -> type_data.ipc_data );

    DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: KSocketAccept\n", self ) );

    /* make sure listener points to a KIPCSocket */
    if ( data->type != isIpcListener )
        return RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcInvalid );
        
    data -> listenerPipe = CreateNamedPipeW( data -> pipename,        /* pipe name */
                                              FILE_FLAG_OVERLAPPED |   /* using overlapped IO */
                                              PIPE_ACCESS_DUPLEX,      /* read/write access  */
                                              PIPE_TYPE_MESSAGE |      /* message type pipe  */
                                              PIPE_READMODE_MESSAGE |  /* message-read mode  */
                                              PIPE_WAIT,               /* blocking mode  */
                                              PIPE_UNLIMITED_INSTANCES,/* max. instances   */
                                              1024,                    /* output buffer size  */
                                              1024,                    /* input buffer size  */
                                              0,                       /* client time-out  */
                                              NULL );                 /* default security attribute  */
    if ( data -> listenerPipe != INVALID_HANDLE_VALUE )
    {
        OVERLAPPED overlap;
        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "KSocketAccept: calling CreateEvent\n" ) );
        overlap.hEvent = CreateEvent( NULL,    /* default security attribute */
                                      TRUE,     /* manual reset event */
                                      FALSE,    /* initial state = nonsignalled */
                                      NULL ); 
        if ( overlap.hEvent != NULL )
        {
            BOOL connected =  ConnectNamedPipe( data -> listenerPipe, &overlap );
            /*DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "KSocketAccept: calling ConnectNamedPipe\n") );*/
            if ( !connected ) /* normal for asynch mode */
            {
                switch ( GetLastError() )
                {
                case ERROR_PIPE_CONNECTED: /* client connected since the call to CreateNamedPipeW */
                    break;
                    
                case ERROR_IO_PENDING:
                    DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "KSocketAccept: calling WaitForSingleObject\n" ) );
                    if ( WaitForSingleObject( overlap.hEvent, INFINITE ) != WAIT_OBJECT_0 )
                    {
                        rc = HandleErrno();
                        CloseHandle( overlap.hEvent );
                        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "KSocketAccept: WaitForSingleObject failed\n" ) );
                        return rc;
                    }
                    break;
                    
                default:
                    rc = HandleErrno();
                    CloseHandle( overlap.hEvent );
                    DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "KSocketAccept: ConnectNamedPipe failed\n" ) );
                    return rc;
                }
            }
            /* we are connected, create the socket stream */
            {
                KSocket * ksock = calloc ( sizeof * ksock, 1 );
                DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "%p: KSocketAccept\n", ksock ) );
                
                if ( ksock == NULL )
                {
                    rc = RC ( rcNS, rcSocket, rcAllocating, rcNoObj, rcNull ); 
                    DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "KSocketAccept: calloc failed\n" ) );
                }
                else
                {
                    rc = KStreamInit ( & ksock -> dad, ( const KStream_vt* ) & vtKIPCSocket,
                                       "KSocket", "tcp", true, true );
                    if ( rc == 0 )
                    {
                        KSocketIPC * ksock_data = &( ksock -> type_data.ipc_data );
                        ksock -> type = epIPC;
                        ksock_data -> type = isIpcPipeServer;
                        ksock_data -> pipe = data -> listenerPipe;
                        ksock_data -> listenerPipe = INVALID_HANDLE_VALUE; /* this is only to be used while ConnectNamedPipe() is in progress */
                        *out = ( KSocket * )& ksock -> dad;
                        CloseHandle( overlap.hEvent );
                        return 0;
                    }
                    free ( ksock );
                    DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "KSocketAccept: KStreamInit failed\n" ) );
                }
                CloseHandle( overlap.hEvent );
                return rc;
            }
        }
    }
    else
    {
        rc = HandleErrno();
        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), ( "KSocketAccept: CreateNamedPipeW failed\n" ) );
    }
    return rc;
}   


/* *********************************************************************************************

    exported KSocket - interface :

    KNSManagerMakeConnection()  ... implemented in manager.c calling KNSManagerMakeRetryTimedConnection()    
    KNSManagerMakeTimedConnection()         ... like KNSManagerMakeConnection()
    KNSManagerMakeRetryConnection()         ... like KNSManagerMakeConnection()
    KNSManagerMakeRetryTimedConnection()    ... switches via endpoint.type into correct implementation specific creation
    KSocketAddRef();                ... delegates to KStreamAddRef()
    KSocketRelease();               ... delegates to KStreamRelease() calls correct destroy-function via vtable
    KSocketGetStream();             ... returns the internal KStream obj, which has its vtable wired to the implementation specific functions
    KSocketGetRemoteEndpoint();     ... delegates to KSocketGetEndpoint() which switches via socket.type into implementations specific functions
    KSocketGetLocalEndpoint();      ... same as above

********************************************************************************************* */

KNS_EXTERN rc_t CC KNSManagerMakeRetryTimedConnection ( struct KNSManager const * self,
    struct KSocket ** out, timeout_t * retryTimeout, int32_t readMillis, int32_t writeMillis,
    struct KEndPoint const * from, struct KEndPoint const * to )
{
    rc_t rc = 0;

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
                const KStream_vt * vt;

                conn -> read_timeout = readMillis;
                conn -> write_timeout = writeMillis;

                conn -> type = to -> type;
                switch ( to -> type )
                {
                case epIPV4:
                    conn -> type_data . ipv4_data . fd = INVALID_SOCKET;
                    vt = ( const KStream_vt * ) & vtKIpv4Socket;
                    break;
                    
                case epIPV6:
                    conn -> type_data . ipv6_data . fd = INVALID_SOCKET;
                    rc = RC ( rcNS, rcSocket, rcConstructing, rcFunction, rcUnsupported ); 
                    break;

                case epIPC:
                    conn -> type_data . ipc_data . pipe = INVALID_HANDLE_VALUE;
                    vt = ( const KStream_vt * ) & vtKIPCSocket;
                    break;

                default:
                    rc = RC ( rcNS, rcStream, rcConstructing, rcParam, rcIncorrect );
                }

                /* initialize the socket */
                if ( rc == 0 )
                    rc = KStreamInit ( & conn -> dad, vt, "KSocket", "", true, true );

                /* prepare the timeout */
                if ( rc == 0 )
                    rc = TimeoutPrepare ( retryTimeout );

                if ( rc == 0 )
                {
                    uint32_t retry;

                    /* a retry loop - retry upon a schedule for the alloted time */
                    for ( retry = 0; ; ++ retry )
                    {
                        uint32_t remaining, delay;

                        /* try to connect using appropriate protocol */
                        switch ( to -> type )
                        {
                        case epIPV4:
                            rc = KSocketConnectIPv4 ( conn, from, to );
                            break;

                        case epIPV6:
                            rc = KSocketConnectIPv6 ( conn, from, to );
                            break;

                        case epIPC:
                            rc = KSocketConnectIPC ( conn, retryTimeout, to );
                            break;
                        }

                        /* if connection was successful, return socket */
                        if ( rc == 0 )
                        {
                            * out = conn;
                            return 0;
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
                }

                free ( conn );
            }
        }

        * out = NULL;
    }

    return rc;
}

#if 0
LIB_EXPORT rc_t CC KNSManagerMakeRetryTimedConnection ( const KNSManager * self,
                                                        KSocket **out,
                                                        timeout_t * retryTimeout,
                                                        int32_t readMillis,
                                                        int32_t writeMillis,
                                                        const KEndPoint *from,
                                                        const KEndPoint *to )
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
            switch ( to -> type )
            {
            case epIPV4 :
                rc = KNSManagerMakeIPv4Connection ( self, out, from, to, retryTimeoutMillis, readMillis, writeMillis );
                break;

            case epIPV6 :
                rc = KNSManagerMakeIPv6Connection ( self, out, from, to, retryTimeoutMillis, readMillis, writeMillis );
                break;

            case epIPC :
                rc = KNSManagerMakeIPCConnection ( self, out, to, retryTimeoutMillis, readMillis, writeMillis );
                break;

            default:
                rc = RC ( rcNS, rcStream, rcConstructing, rcParam, rcIncorrect );
            }

            if ( rc == 0 )
                return 0;
        }

        * out = NULL;
    }
    return rc;
}
#endif

LIB_EXPORT rc_t CC KSocketAddRef( const KSocket *self )
{   /* this will handle all derived types */
    return KStreamAddRef( &self -> dad );
}


LIB_EXPORT rc_t CC KSocketRelease( const KSocket *self )
{   /* this will handle all derived types, it will call the correct whack-function via VTable ! */
    return KStreamRelease( &self -> dad );
}


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


/* helper-function called by KSocketGetRemoteEndpoint() and KSocketGetLocalEndpoint() */
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
            rc = RC ( rcNS, rcSocket, rcEvaluating, rcFunction, rcUnsupported );
            switch( self->type )
            {
                case epIPV4 : rc = KSocketGetEndpointV4( self, ep, remote ); break;
                case epIPV6 : rc = KSocketGetEndpointV6( self, ep, remote ); break;
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


/* *********************************************************************************************

    exported KListener - interface ( from kns/socket.h ):

    KNSManagerMakeListener()    ... switches via endpoint.type into implementations
    KListenerAddRef()           ... delegates to KSocketAddRef()
    KListenerRelease()          ... delegates to KSocketRelease()
    KListenerAccept()           ... switches via socket.type into implementations

********************************************************************************************* */

LIB_EXPORT rc_t CC KNSManagerMakeListener( struct KNSManager const *self,
                                           struct KListener ** out,
                                           struct KEndPoint const * ep )
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
            switch ( ep -> type )
            {
            case epIPV4:
                rc = KNSManagerMakeIPv4Listener ( self, ( struct KSocket** ) out, ep );
                break;

            case epIPV6:
                rc = KNSManagerMakeIPv6Listener ( self, ( struct KSocket** ) out, ep );
                break;

            case epIPC:
                rc = KNSManagerMakeIPCListener ( self, ( struct KSocket** ) out, ep );
                break;

            default:
                rc = RC ( rcNS, rcSocket, rcConstructing, rcParam, rcIncorrect );
            }

            if ( rc == 0 )
                return 0;
        }

        * out = NULL;
    }

    return rc;
}


LIB_EXPORT rc_t CC KListenerAddRef( const KListener *self )
{
    return KSocketAddRef ( ( const KSocket* ) self );
}


LIB_EXPORT rc_t CC KListenerRelease( const KListener *self )
{
    return KSocketRelease ( ( const KSocket* ) self );
}


LIB_EXPORT rc_t CC KListenerAccept ( KListener * self, struct KSocket ** out )
{
    rc_t rc;

    if ( out == NULL )
        rc = RC ( rcNS, rcSocket, rcConstructing, rcParam, rcNull );

    * out = NULL;

    if ( self == NULL )
        rc = RC ( rcNS, rcSocket, rcConstructing, rcSelf, rcNull );
    else
    {
        KSocket * listener = ( KSocket * ) self;
        switch( listener -> type )
        {
            case epIPV4 : rc = KListenerIPv4Accept ( listener, out ); break;
            case epIPV6 : rc = KListenerIPv6Accept ( listener, out ); break;
            case epIPC  : rc = KListenerIPCAccept ( listener, out ); break;
        }
    }
    return rc;
}


/* *********************************************************************************************

    Local helpers

********************************************************************************************* */

static rc_t HandleErrnoEx ( const char *func_name, unsigned int lineno, rc_t rc_ctx )
{
    rc_t rc;
    int lerrno = WSAGetLastError();

    switch ( lerrno )
    {
    case ERROR_FILE_NOT_FOUND:
        rc = RC_FROM_CTX ( rc_ctx, rcFile, rcNotFound );
        break;
    case ERROR_INVALID_HANDLE:
        rc = RC_FROM_CTX ( rc_ctx, rcId, rcInvalid );            
        break;
    case ERROR_INVALID_PARAMETER:
        rc = RC_FROM_CTX ( rc_ctx, rcParam, rcInvalid );            
        break;
    case ERROR_PIPE_BUSY:
        rc = RC_FROM_CTX ( rc_ctx, rcConnection, rcCanceled );
        break;
    case ERROR_SEM_TIMEOUT:
        rc = RC_FROM_CTX ( rc_ctx, rcTimeout, rcExhausted );
        break;
    case WSAEACCES: /* write permission denied */
        rc = RC_FROM_CTX ( rc_ctx, rcMemory, rcUnauthorized );            
        break;
    case WSAEADDRINUSE:/* address is already in use */
        rc = RC_FROM_CTX ( rc_ctx, rcMemory, rcExists );
        break;
    case WSAEADDRNOTAVAIL: /* requested address was not local */
        rc = RC_FROM_CTX ( rc_ctx, rcMemory, rcNotFound );
        break;
    case WSAEAFNOSUPPORT: /* address didnt have correct address family in ss_family field */
        rc = RC_FROM_CTX ( rc_ctx, rcName, rcInvalid );
        break;
    case WSAEALREADY: /* socket is non blocking and a previous connection has not yet completed */
        rc = RC_FROM_CTX ( rc_ctx, rcId, rcUndefined );
        break;
    case WSAECONNABORTED: /* virtual circuit terminated. Application should close socket */
        rc = RC_FROM_CTX ( rc_ctx, rcId, rcInterrupted );
        break;
    case WSAECONNREFUSED: /* remote host refused to allow network connection */
        rc = RC_FROM_CTX ( rc_ctx, rcConnection, rcCanceled );
        break;
    case WSAECONNRESET: /* connection reset by peer */
        rc = RC_FROM_CTX ( rc_ctx, rcId, rcCanceled );
        break;
    case WSAEFAULT: /* name paremeter is not valid part of addr space */
        rc = RC_FROM_CTX ( rc_ctx, rcMemory, rcOutofrange );
        break;
    case WSAEHOSTUNREACH: /* remote hoste cannot be reached at this time */
        rc = RC_FROM_CTX ( rc_ctx, rcConnection, rcNotAvailable );
        break;
    case WSAEINPROGRESS: /* call is in progress */
        rc = RC_FROM_CTX ( rc_ctx, rcId, rcUndefined );
        break;
    case WSAEINVAL: /* invalid argument */
        rc = RC_FROM_CTX ( rc_ctx, rcParam, rcInvalid );
        break;
    case WSAEISCONN: /* connected already */
        rc = RC_FROM_CTX ( rc_ctx, rcConnection, rcExists );
        break;
    case WSAEMSGSIZE:  /* msg size too big */
        rc = RC_FROM_CTX ( rc_ctx, rcMessage, rcExcessive );
        break;
    case WSAENETDOWN:/* network subsystem failed */
        rc = RC_FROM_CTX ( rc_ctx, rcNoObj, rcFailed );
        break;
    case WSAENETRESET: /* connection broken due to keep-alive activity that 
                          detected a failure while in progress */
        rc = RC_FROM_CTX ( rc_ctx, rcConnection, rcCanceled );
        break;
    case WSAENETUNREACH: /* network is unreachable */
        rc = RC_FROM_CTX ( rc_ctx, rcConnection, rcNotAvailable );
        break;
    case WSAENOBUFS: /* output queue for a network connection was full. 
                     ( wont typically happen in linux. Packets are just silently dropped */
        rc = RC_FROM_CTX ( rc_ctx, rcConnection, rcInterrupted );
        break;
    case ERROR_PIPE_NOT_CONNECTED:
    case WSAENOTCONN: /* socket is not connected */
        rc = RC_FROM_CTX ( rc_ctx, rcConnection, rcInvalid );
        break;
    case WSANOTINITIALISED: /* Must have WSAStartup call */
        rc = RC_FROM_CTX ( rc_ctx, rcEnvironment, rcUndefined );
        break;
    case WSAENOTSOCK: /* sock fd is not a socket */
        rc = RC_FROM_CTX ( rc_ctx, rcId, rcInvalid );
        break;
    case WSAEOPNOTSUPP: /* socket is not stream-style such as SOCK_STREAM */
        rc = RC_FROM_CTX ( rc_ctx, rcId, rcUnsupported );
        break;
    case WSAEPROTONOSUPPORT: /* specified protocol is not supported */
        rc = RC_FROM_CTX ( rc_ctx, rcAttr, rcUnsupported );
        break;
    case WSAEPROTOTYPE: /* wrong type of protocol for this socket */
        rc = RC_FROM_CTX ( rc_ctx, rcId, rcUnsupported );
        break;
    case WSAEPROVIDERFAILEDINIT: /* service provider failed to initialize */
        rc = RC_FROM_CTX ( rc_ctx, rcTransfer, rcIncorrect );
        break;
    case ERROR_BROKEN_PIPE:
    case WSAESHUTDOWN: /* socket had been shutdown */
        rc = RC_FROM_CTX ( rc_ctx, rcId, rcUnsupported );
        break;
    case WSAESOCKTNOSUPPORT: /* specified socket type is not supported */
        rc = RC_FROM_CTX ( rc_ctx, rcId, rcUnsupported );
        break;
    case WSAETIMEDOUT: /* connection dropped because of network failure */
        rc = RC_FROM_CTX ( rc_ctx, rcConnection, rcCanceled );
        break;
    case WSAEWOULDBLOCK: /* socket is marked as non-blocking but the recv operation
                            would block */
        rc = RC_FROM_CTX ( rc_ctx, rcCmd, rcBusy );
        break;

    case WSAEINTR: /* call was cancelled */
    case WSAEMFILE: /* no more socket fd available */
    default:
        rc = RC_FROM_CTX ( rc_ctx, rcError, rcUnknown );
        PLOGERR ( klogErr, ( klogErr, rc, "unknown system error '$(S)($(E))', line=$(L)",
                             "S=%!,E=%d,L=%d", lerrno, lerrno, lineno ) );
    }
    return rc;
}

