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

#include <kns/impl.h>
#include <kns/endpoint.h>
#include <klib/text.h>
#include <klib/rc.h>

#include "stream-priv.h"

#include <string.h>
#include <assert.h>

#include <sysalloc.h>


/* InitIPv4Endpoint
 *  initialize the endpoint with an IPv4 address and port
 *
 *  "ep" [ OUT ] - address of endpoint block to be intialized
 *
 *  "ipv4" [ IN, DEFAULT 0 ] - binary IPv4 address in native integer byte order
 *   i.e. 0x7F000001 represents 127.0.0.1. if the special address 0 is given, it
 *   represents any host-local interface address.
 *
 *  "port" [ IN, DEFAULT 0 ] - binary port number in native integer byte order.
 *   if the special port number 0 is given, it represents any available port.
 */
LIB_EXPORT 
rc_t CC KNSManagerInitIPv4Endpoint ( struct KNSManager const *self,
    KEndPoint *ep, uint32_t ipv4, uint16_t port )
{
    rc_t rc;

    if ( ep == NULL )
        rc = RC ( rcNS, rcNoTarg, rcInitializing, rcParam, rcNull);
    else
    {
        if ( self == NULL )
            rc = RC ( rcNS, rcNoTarg, rcInitializing, rcSelf, rcNull );
        else
        {
            ep -> type = epIPV4;
            ep -> u . ipv4. addr = ipv4;
            ep -> u. ipv4 . port = port;
            return 0;
        }

        memset ( ep, 0, sizeof * ep );
    }
    return rc;
}


/* InitIPv6Endpoint
 *  initialize the endpoint with an IPv6 address and port
 *
 *  "ep" [ OUT ] - address of endpoint block to be intialized
 *
 *  "ipv6" [ IN, DEFAULT NULL ] - binary IPv6 address in native integer byte order
 *   has to point to a 16-byte long array ( = 128 bit ). The function will make
 *   a copy of this array. 0x1 represents 0:0:0:0:0:0:0:1 the loopback-device.
 *   If the special address ::0 is given, it represents any host-local interface
 *   address. ( in6addr_any is a 16-byte array with all bytes set to zero )
 *   If a NULL-pointer is passed in, the function treats it as in6addr_any.
 *
 *  "port" [ IN, DEFAULT 0 ] - binary port number in native integer byte order.
 *   if the special port number 0 is given, it represents any available port.
 */
LIB_EXPORT
rc_t CC KNSManagerInitIPv6Endpoint ( struct KNSManager const *self,
    KEndPoint *ep, uint8_t *ipv6, uint16_t port )
{
    rc_t rc;

    if ( ep == NULL )
        rc = RC ( rcNS, rcNoTarg, rcInitializing, rcParam, rcNull);
    else
    {
        if ( self == NULL )
            rc = RC ( rcNS, rcNoTarg, rcInitializing, rcSelf, rcNull );
        else
        {
            ep -> type = epIPV6;
            if ( ipv6 == NULL )
                memset ( ep -> u . ipv6 . addr, 0, sizeof ( ep -> u . ipv6 . addr ) );
            else
                memmove ( ep -> u . ipv6 . addr, ipv6, sizeof ( ep -> u . ipv6 . addr ) );
            ep -> u. ipv6 . port = port;
            return 0;
        }

        memset ( ep, 0, sizeof * ep );
    }
    return rc;
}



/* InitIPCEndpoint
 *  initialize the endpoint for interprocess communication
 *
 *  "ep" [ OUT ] - address of endpoint block to be intialized
 *
 *  "name" [ IN ] - IPC channel name (a POSIX path to a Unix socket, or a name of a Windows pipe)
 */
LIB_EXPORT 
rc_t CC KNSManagerInitIPCEndpoint ( struct KNSManager const *self,
    KEndPoint *ep, struct String const * name )
{
    rc_t rc;

    if ( ep == NULL )
        rc = RC ( rcNS, rcNoTarg, rcInitializing, rcParam, rcNull);
    else
    {
        if ( self == NULL )
            rc = RC ( rcNS, rcNoTarg, rcInitializing, rcSelf, rcNull );
        else
        {
            size_t copied = string_copy(ep -> u.ipc_name, IPC_NAME_MAX - 1, name->addr, name->len);
            if (copied != name->len)
                rc = RC ( rcNS, rcNoTarg, rcInitializing, rcSize, rcExcessive );
            ep -> u.ipc_name[copied] = 0;
            ep -> type = epIPC;
            return 0;
        }

        memset ( ep, 0, sizeof * ep );
    }
    return rc;
}
