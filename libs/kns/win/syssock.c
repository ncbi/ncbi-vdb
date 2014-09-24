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
#include <klib/log.h>
#include <klib/printf.h>
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

static rc_t HandleErrnoEx ( const char *func, unsigned int lineno );
#define HandleErrno() HandleErrnoEx ( __func__, __LINE__ )

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


static rc_t CC KIpv4SocketWhack ( KSocket * self )
{
    rc_t rc = 0;
    KSocketIPv4 * data;

    if ( self == NULL ) /* let's tolerate whacking NULL pointers... */
        return rc;

    if ( self->type != epIPV4 )
        return RC ( rcNS, rcSocket, rcClosing, rcParam, rcInvalid );

    data = &( self -> type_data.ipv4_data );
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
    
    free ( self );

    return rc;
}


static rc_t CC KIpv4SocketTimedRead ( const KSocket * self, void * buffer, size_t bsize,
                                      size_t * num_read, timeout_t * tm )
{
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
    {
        rc = HandleErrno();
    }
    else if ( selectRes == 0 )
    {
        rc = RC ( rcNS, rcSocket, rcReading, rcTimeout, rcExhausted ); /* timeout */
    }
    else if ( FD_ISSET( data -> fd, &readFds ) )
    {
        while ( rc == 0 )
        {
            ssize_t count = recv ( data -> fd, buffer, ( int )bsize, 0 );

            if ( count >= 0 )
            {
                if ( num_read != NULL );
                    * num_read = ( size_t ) count;
                return 0;
            }
            if ( WSAGetLastError() != WSAEINTR )
                rc = HandleErrno();
            break;
        }
    }
    else
        rc = HandleErrno();
            
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


static rc_t CC KIpv4SocketTimedWrite ( KSocket * self, const void * buffer, size_t bsize,
                                       size_t * num_writ, timeout_t * tm )
{
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
    {
        rc = HandleErrno();
    }
    else if ( selectRes == 0 )
    {
        rc = RC ( rcNS, rcSocket, rcWriting, rcTimeout, rcExhausted ); /* timeout */
    }
    else if ( FD_ISSET( data -> fd, &writeFds ) )
    {
        while ( rc == 0 )
        {
            ssize_t count = send ( data -> fd , buffer, ( int )bsize, 0 );
            if ( count >= 0 )
            {
                if ( num_writ != NULL );
                    * num_writ = count;
                return 0;
            }
            if ( WSAGetLastError() != WSAEINTR )
                rc = HandleErrno();
            break;        
        }
    }
    else
        rc = HandleErrno();

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

static rc_t KNSManagerMakeIPv4Connection ( struct KNSManager const * self,
                                           KSocket ** out,
                                           const KEndPoint * from,
                                           const KEndPoint * to,
                                           int32_t retryTimeout, 
                                           int32_t readMillis, 
                                           int32_t writeMillis )
{
    rc_t rc = 0;
    uint32_t retry_count = 0;
    SOCKET fd;

    * out = NULL;

    assert ( to != NULL );
    assert ( to -> type == epIPV4 );
    assert ( ( from == NULL || from -> type == to -> type ) );

    do
    {
        fd = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
        if ( fd == INVALID_SOCKET )
            rc = HandleErrno();
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
            if ( bind ( fd, ( const struct sockaddr* )&ss, sizeof ss  ) == SOCKET_ERROR ) 
                rc = HandleErrno();
                
            if ( rc == 0 )
            {
                ss . sin_port = htons ( to -> u . ipv4 . port );
                ss . sin_addr . s_addr = htonl ( to -> u . ipv4 . addr );
                
                if ( connect ( fd, (const struct sockaddr*)&ss, sizeof ss ) != SOCKET_ERROR )
                {   /* create the KSocket */
                    KSocket * ksock = calloc ( sizeof * ksock, 1 );
                    if ( ksock == NULL )
                        rc = RC ( rcNS, rcSocket, rcAllocating, rcNoObj, rcNull ); 
                    else
                    {   /* initialize the KSocket */
                        rc = KStreamInit ( & ksock -> dad, ( const KStream_vt* ) & vtKIpv4Socket,
                                           "KSocket", "tcp", true, true );
                        if ( rc == 0 )
                        {
                            KSocketIPv4 * data = &( ksock -> type_data.ipv4_data );
                            ksock -> read_timeout  = readMillis;
                            ksock -> write_timeout = writeMillis;
                            ksock -> type = epIPV4;
                            data -> fd = fd;
                            *out = ( KSocket * )& ksock -> dad;
                            return 0;
                        }
                        free( ksock );
                    }
                    /* we connected but then then ran out of memory or something bad like that, so no need to retry 
                       - simply close fd and return RC */
                    closesocket( fd );
                    return rc;
                }
                else /* connect () failed */
                    rc = HandleErrno();
            } 
            /* dump socket */
            closesocket( fd );
        }
        
        /* rc != 0 */
        if ( retryTimeout < 0 || ( int32_t )retry_count < retryTimeout )
        {   /* retry */
            Sleep ( 1000 ); /*ms*/
            ++retry_count;
            rc = 0;
        }            
    }
    while ( rc == 0 );
    
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
                rc = HandleErrno ();
            else
            {
                struct sockaddr_in ss;

                int on = 1;
                setsockopt ( data -> fd, SOL_SOCKET, SO_REUSEADDR, ( char* ) & on, sizeof on );

                memset ( & ss, 0, sizeof ss );
                ss . sin_family = AF_INET;
                ss . sin_addr . s_addr = htonl ( ep -> u . ipv4 . addr );
                ss . sin_port = htons ( ep -> u . ipv4 . port );

                if ( bind ( data -> fd, ( struct sockaddr* ) & ss, sizeof ss ) == 0 )
                {
                    if ( listen ( data -> fd, 5 ) == 0 )
                    {
                        * out = listener;
                        return 0;
                    }
                }
                rc = HandleErrno ();
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
                rc = HandleErrno ();
            else if ( len > sizeof new_data -> remote_addr )
            {
                closesocket ( new_data -> fd );
                new_data -> fd = -1;
                rc = RC ( rcNS, rcConnection, rcWaiting, rcBuffer, rcInsufficient );
            }
            else
                new_data -> remote_addr_valid = true;

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

    if ( remote )
    {
        if ( data -> remote_addr_valid )
        {
            /* the remote part was already recorded through calling accept() */
            addr . sin_addr . s_addr = data -> remote_addr . sin_addr . s_addr;
            addr . sin_port        = data -> remote_addr . sin_port;
        }
        else
            res = getpeername( data -> fd, ( struct sockaddr * )&addr, &l );
    }
    else
        res = getsockname( data -> fd, ( struct sockaddr * )&addr, &l );

    if ( res < 0 )
        rc = HandleErrno();
    else
    {
        ep -> u . ipv4.addr = ntohl( addr . sin_addr . s_addr );
        ep -> u . ipv4.port = ntohs( addr . sin_port );
        ep -> type = epIPV4;
    }

    return rc;
}


/* *********************************************************************************************

    IPv6 implementation :

    KNSManagerMakeIPv6Connection()  ... called from KNSManagerMakeRetryTimedConnection()
    KNSManagerMakeIPv6Listener()    ... called from KNSManagerMakeListener()
    KListenerIPv6Accept()           ... called from KListenerAccept()
    KSocketGetEndpointV6()          ... called from KSocketGetEndpoint()

********************************************************************************************* */

static rc_t KNSManagerMakeIPv6Connection ( struct KNSManager const * self,
                                           KSocket ** out,
                                           const KEndPoint * from,
                                           const KEndPoint * to,
                                           int32_t retryTimeout, 
                                           int32_t readMillis, 
                                           int32_t writeMillis )
{
    return RC ( rcNS, rcSocket, rcAllocating, rcFunction, rcUnsupported ); 
}


static rc_t KNSManagerMakeIPv6Listener ( const KNSManager *self, KSocket **out, const KEndPoint * ep )
{
    rc_t rc = 0;
    KSocket * listener = calloc ( 1, sizeof * listener );
    if ( listener == NULL )
        rc = RC ( rcNS, rcSocket, rcConstructing, rcMemory, rcExhausted );
    else
    {
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
                rc = HandleErrno ();
            else
            {
                struct sockaddr_in6 ss;

                int on = 1;
                setsockopt ( data -> fd, SOL_SOCKET, SO_REUSEADDR, ( char* ) & on, sizeof on );

                memset ( & ss, 0, sizeof ss );
                ss . sin6_family = AF_INET6;
                memcpy ( ss . sin6_addr . s6_addr,
                         ep -> u . ipv6 . addr,
                         sizeof ( ep -> u . ipv6 . addr ) );
                ss . sin6_port = htons ( ep -> u . ipv6 . port );

                if ( bind ( data -> fd, ( struct sockaddr* ) & ss, sizeof ss ) == 0 )
                {
                    if ( listen ( data -> fd, 5 ) == 0 )
                    {
                        * out = listener;
                        return 0;
                    }
                }
                rc = HandleErrno ();
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
                rc = HandleErrno ();
            else if ( len > sizeof new_data -> remote_addr )
            {
                closesocket ( new_data -> fd );
                new_data -> fd = -1;
                rc = RC ( rcNS, rcConnection, rcWaiting, rcBuffer, rcInsufficient );
            }
            else
                new_data -> remote_addr_valid = true;

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

    if ( remote )
    {
        if ( data -> remote_addr_valid )
        {
            /* the remote part was already recorded through calling accept() */
            memcpy ( ep -> u . ipv6 . addr,
                     data -> remote_addr . sin6_addr . s6_addr,
                     sizeof ( ep -> u . ipv6 . addr ) );
            ep->u.ipv6.port = ntohs( data -> remote_addr . sin6_port );
            ep->type = epIPV6;
            return 0;
        }
        else
            res = getpeername( data -> fd, ( struct sockaddr * )&addr, &l );
    }
    else
        res = getsockname( data -> fd, ( struct sockaddr * )&addr, &l );

    if ( res < 0 )
        rc = HandleErrno();
    else
    {
        memcpy ( ep -> u . ipv6 . addr,
                 addr . sin6_addr . s6_addr,
                 sizeof ( ep -> u . ipv6 . addr ) );
        ep -> u.ipv6.port = ntohs( addr . sin6_port );
        ep -> type = epIPV6;
    }

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
    rc_t rc = 0;
    KSocketIPC * data = &( self -> type_data.ipc_data );

    pLogLibMsg( klogInfo, "$(b): isIpcListener", "b=%p", self );    
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
            pLogLibMsg( klogInfo, "$(b): DisconnectNamedPipe failed", "b=%p", self );
        }
        
        if ( !CloseHandle( data->listenerPipe ) )
        {
            rc = HandleErrno();
            pLogLibMsg( klogInfo, "$(b): CloseHandle failed", "b=%p", self );
        }
    }
    return rc;
}


/* helper function called by KIPCSocketWhack() = static function for IPC-implementation vtable */
static rc_t KIPCSocketWhack_server_side_pipe ( KSocket * self )
{
    rc_t rc = 0;
    KSocketIPC * data = &( self -> type_data.ipc_data );

    pLogLibMsg( klogInfo, "$(b): isIpcPipeServer", "b=%p", self );
    if ( !FlushFileBuffers( data->pipe ) )
    {
        if ( GetLastError() != ERROR_BROKEN_PIPE )
        {
            rc = HandleErrno();
            pLogLibMsg( klogInfo, "$(b): FlushFileBuffers failed, err=$(e)", "b=%p,e=%d", self, GetLastError() );    
        }
    }
    if ( !DisconnectNamedPipe( data -> pipe ) )
    {
        rc = HandleErrno();
        pLogLibMsg(klogInfo, "$(b): DisconnectNamedPipe failed", "b=%p", self );    
    }
    if ( !CloseHandle( data -> pipe ) )
    {
        rc = HandleErrno();
        pLogLibMsg( klogInfo, "$(b): CloseHandle failed", "b=%p", self );
    }
    return rc;
}


/* helper function called by KIPCSocketWhack() = static function for IPC-implementation vtable */
static rc_t KIPCSocketWhack_client_side_pipe ( KSocket * self )
{
    rc_t rc = 0;
    KSocketIPC * data = &( self -> type_data.ipc_data );

    pLogLibMsg( klogInfo, "$(b): isIpcPipeClient", "b=%p", self );
    if ( !CloseHandle( data -> pipe ) )
    {
        rc = HandleErrno();
        pLogLibMsg( klogInfo, "$(b): CloseHandle failed", "b=%p", self );
    }
    return rc;
}


/* static function for IPC-implementation vtable */
static rc_t CC KIPCSocketWhack ( KSocket * self )
{
    rc_t rc = 0;
    /* we tolerate a whack on a NULL-pointer */
    if ( self == NULL ) return rc;

    pLogLibMsg( klogInfo, "$(b): KIPCSocketWhack()...", "b=%p", self ); 
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
    /* received a ERROR_NO_DATA trying to read from a pipe; wait for the data to arrive or a time out to expire */ 
    /* on success, will leave tmMs set to the remaining portion of timeout, if specified */

    const KSocketIPC * data = &( self -> type_data.ipc_data );
    uint32_t tm_decrement = 100; 

    pLogLibMsg( klogInfo, "$(b): no data on the pipe - going into a wait loop, tm=$(t)", "b=%p,t=%d",
                self, tmMs == 0 ? -1 : *tmMs );
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
            pLogLibMsg( klogInfo, "$(b): (wait loop) ReadFile completed synchronously, count=$(c)", "b=%p,c=%d",
                        self, count );
            assert ( num_read != NULL );
            * num_read = ( size_t ) count;
            CloseHandle( overlap -> hEvent );
            return 0;
        }
        
        switch ( GetLastError() )
        {
            case ERROR_IO_PENDING : return 0; /* the caller will wait for completion */
            
            case ERROR_NO_DATA :
                pLogLibMsg( klogInfo, "$(b): (wait loop) Sleep($(t))", "b=%p,t=%d", self, tm_decrement );
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
    rc_t rc = 0;
    const KSocketIPC * data = &( self -> type_data.ipc_data );
    OVERLAPPED overlap;
    
    if ( num_read == NULL )
        return RC ( rcNS, rcConnection, rcReading, rcParam, rcNull );

    pLogLibMsg( klogInfo, "$(b): KIPCSocketTimedRead($(t), $(buf), $(s))... ", "b=%p,t=%d,buf=%p,s=%d",
                self, tm == NULL ? -1 : tm -> mS, buffer, bsize );

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
            pLogLibMsg( klogInfo, "$(b): ReadFile completed synchronously, count=$(c)", "b=%p,c=%d", self, count );
            * num_read = ( size_t ) count;
            CloseHandle( overlap.hEvent );
            return 0;
        }
        
        *num_read = 0;

        /* asynch mode - wait for the operation to complete */
        if ( GetLastError() == ERROR_NO_DATA ) /* 232 */
        {
            pLogLibMsg( klogInfo,
                        "$(b): ReadFile($(h)) returned FALSE, GetLastError() = ERROR_NO_DATA", "b=%p,h=%x",
                        self, data -> pipe );
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
            pLogLibMsg( klogInfo,
                        "$(b): ReadFile($(h)) returned FALSE, GetLastError() = ERROR_IO_PENDING", "b=%p,h=%x",
                        self, data -> pipe );

            if ( tm == NULL )
                pLogLibMsg( klogInfo, "$(b): waiting forever", "b=%p", self );
            else
                pLogLibMsg( klogInfo, "$(b): waiting for $(t) ms", "b=%p,t=%d", self, tm -> mS );
                
            switch ( WaitForSingleObject( overlap.hEvent, tm == NULL ? INFINITE : tm -> mS ) )
            {
                case WAIT_TIMEOUT :
                    pLogLibMsg( klogInfo, "$(b): timed out", "b=%p", self );
                    rc = RC ( rcNS, rcFile, rcReading, rcTimeout, rcExhausted );
                    break;
                    
                case WAIT_OBJECT_0 :
                {
                    DWORD count;
                    pLogLibMsg( klogInfo, "$(b): successful", "b=%p", self );
                    /* wait to complete if necessary */
                    if ( GetOverlappedResult( data->pipe, &overlap, &count, TRUE ) )
                    {
                        pLogLibMsg( klogInfo, "$(b): $(c) bytes read", "b=%p,c=%d", self, count );
                        * num_read = ( size_t ) count;
                        rc = 0;
                    }
                    else
                    {
                        rc = HandleErrno();
                        pLogLibMsg( klogInfo, "$(b): GetOverlappedResult() failed", "b=%p", self );
                    }
                    break;
                }
                
                default:
                    rc = HandleErrno();
                    pLogLibMsg( klogInfo, "$(b): WaitForSingleObject() failed", "b=%p", self );
                    break;
            }
        }
        else if ( GetLastError() == ERROR_SUCCESS )
        {
            pLogLibMsg( klogInfo,
                        "$(b): ReadFile($(h)) returned FALSE, GetLastError() = ERROR_SUCCESS", "b=%p,h=%x",
                        self, data -> pipe );
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
    rc_t rc = 0;
    KSocketIPC * data;
    OVERLAPPED overlap;
    
    if ( self == NULL )
        return RC ( rcNS, rcConnection, rcWriting, rcSelf, rcNull );

    data = &( self -> type_data.ipc_data );

    pLogLibMsg( klogInfo,
                "$(b): KIPCSocketTimedWrite($(s), $(t))...", "b=%p,s=%d,t=%d",
                self, bsize, tm == NULL ? -1 : tm -> mS );

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
        /*pLogLibMsg(klogInfo, "$(b): WriteFile returned $(r), GetError() = $(e)", "b=%p,r=%s,e=%d", base, ret ? "TRUE" : "FALSE", err); */

        /* completed synchronously; either message is so short that is went out immediately, or the pipe is full */
        if ( ret )
        {   
            if ( count > 0 )
            {
                pLogLibMsg( klogInfo, "$(b): $(c) bytes written", "b=%p,c=%d", self, count );
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
                pLogLibMsg( klogInfo,
                            "$(b): pipe full - going into a wait loop for $(t) ms", "b=%p,t=%d",
                            self, ( tm == NULL ) ? -1 : tm -> mS );
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
                    
                    pLogLibMsg( klogInfo, "$(b): write wait loop: attempting to WriteFile", "b=%p", self );   

                    /* returns FALSE in asynch mode */
                    ret = WriteFile( data->pipe, buffer, ( DWORD )bsize, &count, &overlap );
                    err = GetLastError();
                    /* pLogLibMsg(klogInfo, "$(b): WriteFile returned $(r), GetError() = $(e)", "b=%p,r=%s,e=%d", base, ret ? "TRUE" : "FALSE", err); */
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
                pLogLibMsg( klogInfo, "$(b): timed out ", "b=%p", self );
                CloseHandle( overlap.hEvent );
                return RC ( rcNS, rcStream, rcWriting, rcTimeout, rcExhausted );

            case WAIT_OBJECT_0:
            {
                pLogLibMsg( klogInfo, "$(b): successful", "b=%p", self );
                /* wait to complete if necessary */
                if ( GetOverlappedResult( data->pipe, &overlap, &count, TRUE ) )
                {
                    pLogLibMsg( klogInfo, "$(b): $(c) bytes written", "b=%p,c=%d", self, count );
                    if ( num_writ != NULL )
                        * num_writ = count;
                    CloseHandle( overlap.hEvent );
                    return 0;
                }
                rc = HandleErrno();
                pLogLibMsg( klogInfo, "$(b): GetOverlappedResult() failed", "b=%p", self );
                break;
            }
            
            default:
                rc = HandleErrno();
                pLogLibMsg( klogInfo, "$(b): WaitForSingleObject() failed", "b=%p", self );
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
            pLogLibMsg( klogInfo, "$(b): WriteFile() failed", "b=%p", self );
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


static rc_t KNSManagerMakeIPCConnection ( struct KNSManager const *self, 
                                          KSocket **out, 
                                          const KEndPoint *to, 
                                          int32_t retryTimeout, 
                                          int32_t readMillis, 
                                          int32_t writeMillis )
{
    rc_t rc = 0;
    uint8_t retry_count = 0;
    char pipename[ PIPE_NAME_LENGTH ];
    wchar_t pipenameW[ PIPE_NAME_LENGTH ];
    size_t num_writ;
    
    if ( self == NULL )
        return RC ( rcNS, rcConnection, rcCreating, rcSelf, rcNull );
    if ( out == NULL )
        return RC ( rcNS, rcConnection, rcCreating, rcParam, rcNull );

    * out = NULL;

    if ( to == NULL )
        return RC ( rcNS, rcConnection, rcCreating, rcParam, rcNull );
    if ( to -> type != epIPC )
        return RC ( rcNS, rcConnection, rcCreating, rcParam, rcInvalid );

    /* use named pipes to implement unix domain socket - like behavior */
    rc = string_printf( pipename, sizeof( pipename ), &num_writ, "\\\\.\\pipe\\%s", to->u.ipc_name );
    if ( rc == 0 )
        string_cvt_wchar_copy( pipenameW, sizeof( pipenameW ), pipename, num_writ );
        
    while ( rc == 0 )
    {
        HANDLE h = CreateFileW( pipenameW,                    /* pipe name */
                                GENERIC_READ | GENERIC_WRITE, /* read and write access */
                                0,                            /* no sharing */
                                NULL,                         /* default security attributes */
                                OPEN_EXISTING,                /* opens existing pipe  */
                                FILE_FLAG_OVERLAPPED,         /* using overlapped IO */
                                NULL );                      /* no template file */
        if ( h != INVALID_HANDLE_VALUE )
        {   /* create the KSocket */
            /* need NOWAIT if pipe is created in asynch mode */
            DWORD dwMode = ( PIPE_READMODE_MESSAGE | PIPE_NOWAIT );
            if ( SetNamedPipeHandleState ( h,        /* pipe handle */
                                           &dwMode,  /* new pipe mode */
                                           NULL,     /* don't set maximum bytes */
                                           NULL ) )  /* don't set maximum time */
            {
                KSocket* ksock = calloc ( sizeof * ksock, 1 );

                if ( ksock == NULL )
                    rc = RC ( rcNS, rcNoTarg, rcAllocating, rcNoObj, rcNull ); 
                else
                {   
                    KSocketIPC * data;
                    /* initialize the KSocket */
                    rc = KStreamInit ( & ksock -> dad, ( const KStream_vt* ) & vtKIPCSocket,
                                       "KSocket", "tcp", true, true );
                    if ( rc == 0 )
                    {
                        pLogLibMsg( klogInfo,
                                   "$(b): KNSManagerMakeIPCConnection($(e),'$(n)')", "b=%p,e=%p,n=%s",
                                    ksock, to, pipename );
                    
                        ksock -> read_timeout  = readMillis;
                        ksock -> write_timeout = writeMillis;
                        ksock -> type = epIPC;
                        data = &( ksock -> type_data.ipc_data );
                        data -> type = isIpcPipeClient;
                        data -> pipe = h;
                        *out = ( KSocket * )& ksock -> dad;
                        return 0;
                    }
                    free ( ksock );
                }
            }
            else
                rc = HandleErrno();
        }
        else /* CreateFileW failed */
        {
            switch ( GetLastError() )
            {
                case ERROR_PIPE_BUSY :
                    LogLibMsg( klogInfo, "KNSManagerMakeIPCConnection: pipe busy, retrying" );
                    {
                        BOOL pipeAvailable = WaitNamedPipeW( pipenameW, NMPWAIT_USE_DEFAULT_WAIT );
                        if ( pipeAvailable )
                        {
                            LogLibMsg( klogInfo, "KNSManagerMakeIPCConnection: WaitNamedPipeW returned TRUE" );
                            continue;
                        }
                        /* time-out, try again */
                        rc = HandleErrno();
                        LogLibMsg( klogInfo, "KNSManagerMakeIPCConnection: WaitNamedPipeW returned FALSE(timeout)" );
                        if ( retryTimeout < 0 || retry_count < retryTimeout )
                        {
                            Sleep( 1000 ); /* ms */
                            ++retry_count;
                            rc = 0;
                            continue;
                        }
                    }
                    break;
                
                case ERROR_FILE_NOT_FOUND :
                    if ( retryTimeout < 0 || retry_count < retryTimeout )
                    {
                        LogLibMsg( klogInfo, "KNSManagerMakeIPCConnection: pipe not found, retrying" );
                        Sleep( 1000 ); /* ms */
                        ++retry_count;
                        rc = 0;
                        continue;
                    }
                    else
                        rc = HandleErrno();
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
                
                pLogLibMsg( klogInfo,
                            "$(b): KNSManagerMakeIPCListener($(e),'$(n)')", "b=%p,e=%p,n=%s",
                            ksock, ep, pipename );
                return 0;
            }
            KIPCSocketWhack( ksock );
        }
        else
            free ( ksock );
    }
        
    pLogLibMsg( klogInfo, "$(b): KNSManagerMakeIPCListener failed", "b=%p", ksock );
    return rc;
}


static rc_t KListenerIPCAccept ( KSocket * self, struct KSocket ** out )
{
    rc_t rc = 0;
    KSocketIPC * data = &( self -> type_data.ipc_data );

    pLogLibMsg( klogInfo, "$(b): KSocketAccept", "b=%p", self );

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
        LogLibMsg( klogInfo, "KSocketAccept: calling CreateEvent" );
        overlap.hEvent = CreateEvent( NULL,    /* default security attribute */
                                      TRUE,     /* manual reset event */
                                      FALSE,    /* initial state = nonsignalled */
                                      NULL ); 
        if ( overlap.hEvent != NULL )
        {
            BOOL connected =  ConnectNamedPipe( data -> listenerPipe, &overlap );
            /*LogLibMsg(klogInfo, "KSocketAccept: calling ConnectNamedPipe");*/
            if ( !connected ) /* normal for asynch mode */
            {
                switch ( GetLastError() )
                {
                case ERROR_PIPE_CONNECTED: /* client connected since the call to CreateNamedPipeW */
                    break;
                    
                case ERROR_IO_PENDING:
                    LogLibMsg( klogInfo, "KSocketAccept: calling WaitForSingleObject" );
                    if ( WaitForSingleObject( overlap.hEvent, INFINITE ) != WAIT_OBJECT_0 )
                    {
                        rc = HandleErrno();
                        CloseHandle( overlap.hEvent );
                        LogLibMsg( klogInfo, "KSocketAccept: WaitForSingleObject failed" );
                        return rc;
                    }
                    break;
                    
                default:
                    rc = HandleErrno();
                    CloseHandle( overlap.hEvent );
                    LogLibMsg( klogInfo, "KSocketAccept: ConnectNamedPipe failed" );
                    return rc;
                }
            }
            /* we are connected, create the socket stream */
            {
                KSocket * ksock = calloc ( sizeof * ksock, 1 );
                pLogLibMsg( klogInfo, "$(b): KSocketAccept", "b=%p", ksock );
                
                if ( ksock == NULL )
                {
                    rc = RC ( rcNS, rcSocket, rcAllocating, rcNoObj, rcNull ); 
                    LogLibMsg( klogInfo, "KSocketAccept: calloc failed" );
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
                    LogLibMsg( klogInfo, "KSocketAccept: KStreamInit failed" );
                }
                CloseHandle( overlap.hEvent );
                return rc;
            }
        }
    }
    else
    {
        rc = HandleErrno();
        LogLibMsg( klogInfo, "KSocketAccept: CreateNamedPipeW failed" );
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

LIB_EXPORT rc_t CC KNSManagerMakeRetryTimedConnection ( const KNSManager * self,
                                                        KSocket **out,
                                                        int32_t retryTimeout,
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
        else if ( to == NULL )
            rc = RC ( rcNS, rcStream, rcConstructing, rcParam, rcNull );
        else if ( from != NULL && from -> type != to -> type )
            rc = RC ( rcNS, rcStream, rcConstructing, rcParam, rcIncorrect );
        else
        {
            switch ( to -> type )
            {
            case epIPV4 :
                rc = KNSManagerMakeIPv4Connection ( self, out, from, to, retryTimeout, readMillis, writeMillis );
                break;

            case epIPV6 :
                rc = KNSManagerMakeIPv6Connection ( self, out, from, to, retryTimeout, readMillis, writeMillis );
                break;

            case epIPC :
                rc = KNSManagerMakeIPCConnection ( self, out, to, retryTimeout, readMillis, writeMillis );
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

static rc_t HandleErrnoEx ( const char *func_name, unsigned int lineno )
{
    rc_t rc;
    int lerrno = WSAGetLastError();

    switch ( lerrno )
    {
    case ERROR_FILE_NOT_FOUND:
        rc = RC ( rcNS, rcNoTarg, rcReading, rcFile, rcNotFound );            
        break;
    case ERROR_INVALID_HANDLE:
        rc = RC ( rcNS, rcNoTarg, rcReading, rcId, rcInvalid );            
        break;
    case ERROR_INVALID_PARAMETER:
        rc = RC ( rcNS, rcNoTarg, rcReading, rcParam, rcInvalid );            
        break;
    case ERROR_PIPE_BUSY:
        rc = RC ( rcNS, rcNoTarg, rcReading, rcConnection, rcCanceled );
        break;
    case ERROR_SEM_TIMEOUT:
        rc = RC ( rcNS, rcStream, rcReading, rcTimeout, rcExhausted );
        break;
    case WSAEACCES: /* write permission denied */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcMemory, rcUnauthorized );            
        break;
    case WSAEADDRINUSE:/* address is already in use */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcMemory, rcExists );
        break;
    case WSAEADDRNOTAVAIL: /* requested address was not local */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcMemory, rcNotFound );
        break;
    case WSAEAFNOSUPPORT: /* address didnt have correct address family in ss_family field */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcName, rcInvalid );
        break;
    case WSAEALREADY: /* socket is non blocking and a previous connection has not yet completed */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcId, rcUndefined );
        break;
    case WSAECONNABORTED: /* virtual circuit terminated. Application should close socket */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcId, rcInterrupted );
        break;
    case WSAECONNREFUSED: /* remote host refused to allow network connection */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcConnection, rcCanceled );
        break;
    case WSAECONNRESET: /* connection reset by peer */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcId, rcCanceled );
        break;
    case WSAEFAULT: /* name paremeter is not valid part of addr space */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcMemory, rcOutofrange );
        break;
    case WSAEHOSTUNREACH: /* remote hoste cannot be reached at this time */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcConnection, rcNotAvailable );
        break;
    case WSAEINPROGRESS: /* call is in progress */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcId, rcUndefined );
        break;
    case WSAEINVAL: /* invalid argument */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcParam, rcInvalid );
        break;
    case WSAEISCONN: /* connected already */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcConnection, rcExists );
        break;
    case WSAEMSGSIZE:  /* msg size too big */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcMessage, rcExcessive );
        break;
    case WSAENETDOWN:/* network subsystem failed */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcNoObj, rcFailed );
        break;
    case WSAENETRESET: /* connection broken due to keep-alive activity that 
                          detected a failure while in progress */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcConnection, rcCanceled );
        break;
    case WSAENETUNREACH: /* network is unreachable */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcConnection, rcNotAvailable );
        break;
    case WSAENOBUFS: /* output queue for a network connection was full. 
                     ( wont typically happen in linux. Packets are just silently dropped */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcConnection, rcInterrupted );
        break;
    case ERROR_PIPE_NOT_CONNECTED:
    case WSAENOTCONN: /* socket is not connected */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcConnection, rcInvalid );
        break;
    case WSANOTINITIALISED: /* Must have WSAStartup call */
        rc = RC ( rcNS, rcNoTarg, rcInitializing, rcEnvironment, rcUndefined );
        break;
    case WSAENOTSOCK: /* sock fd is not a socket */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcId, rcInvalid );
        break;
    case WSAEOPNOTSUPP: /* socket is not stream-style such as SOCK_STREAM */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcId, rcUnsupported );
        break;
    case WSAEPROTONOSUPPORT: /* specified protocol is not supported */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcAttr, rcUnsupported );
        break;
    case WSAEPROTOTYPE: /* wrong type of protocol for this socket */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcId, rcUnsupported );
        break;
    case WSAEPROVIDERFAILEDINIT: /* service provider failed to initialize */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcTransfer, rcIncorrect );
        break;
    case ERROR_BROKEN_PIPE:
    case WSAESHUTDOWN: /* socket had been shutdown */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcId, rcUnsupported );
        break;
    case WSAESOCKTNOSUPPORT: /* specified socket type is not supported */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcId, rcUnsupported );
        break;
    case WSAETIMEDOUT: /* connection dropped because of network failure */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcConnection, rcCanceled );
        break;
    case WSAEWOULDBLOCK: /* socket is marked as non-blocking but the recv operation
                            would block */
        rc = RC ( rcNS, rcNoTarg, rcReading, rcCmd, rcBusy );
        break;

    case WSAEINTR: /* call was cancelled */
    case WSAEMFILE: /* no more socket fd available */
    default:
        rc = RC ( rcNS, rcNoTarg, rcReading, rcError, rcUnknown );
        PLOGERR ( klogErr, ( klogErr, rc, "unknown system error '$(S)($(E))', line=$(L)",
                             "S=%!,E=%d,L=%d", lerrno, lerrno, lineno ) );
    }
    return rc;
}

