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

#ifdef _WINSOCK_DEPRECATED_NO_WARNINGS
#define WINSOCK_DEP_NO_WARN_WAS_DEFINED 1
#else
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <kns/impl.h>
#include <kns/endpoint.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/status.h> /* STATUS */
#include <klib/rc.h>
#include <klib/data-buffer.h>

#include <sysalloc.h>
#include <os-native.h>

#include "stream-priv.h"

#include <assert.h>

/* InitDNSEndpoint
 *  initialize the endpoint with a DNS name and a port number
 *
 *  "ep" [ OUT ] - address of endpoint block to be intialized
 *
 *  "dns" [ IN ] - textual DNS address.
 *
 *  "port" [ IN, DEFAULT 0 ] - binary port number in native integer byte order.
 *   if the special port number 0 is given, it represents any available port.
 */
LIB_EXPORT
rc_t CC KNSManagerInitDNSEndpoint ( struct KNSManager const *self,
    KEndPoint *ep, struct String const *dns, uint16_t port )
{
    rc_t rc = 0;

    if ( ep == NULL )
        rc = RC (rcNS, rcNoTarg, rcInitializing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcNS, rcNoTarg, rcInitializing, rcSelf, rcNull );
        else if ( dns == NULL )
            rc = RC ( rcNS, rcNoTarg, rcInitializing, rcParam, rcNull );
        else if ( dns -> size == 0 )
            rc = RC ( rcNS, rcNoTarg, rcInitializing, rcSelf, rcInsufficient );
        else
        {
            KDataBuffer b;
            char buffer [ 4096 ], * hostname = buffer;
            size_t buff_size = sizeof buffer;

            if ( dns -> size >= sizeof buffer )
            {
                rc = KDataBufferMakeBytes ( & b, dns -> size + 1 );
                if ( rc == 0 )
                {
                    hostname = b . base;
                    buff_size = ( size_t ) b . elem_count;
                }
            }

            if ( rc == 0 )
            {
                size_t size;
                rc = string_printf ( hostname, buff_size, & size, "%S", dns );

                assert ( rc == 0 );
                assert ( size < buff_size );
                assert ( hostname [ size ] == 0 );

                if ( rc ==  0 )
                {
                                       struct addrinfo hints;
                    memset(&hints, 0, sizeof hints);
                    hints.ai_family = AF_INET; // IPv4

                    char port_s [ 6 ];
                    rc = string_printf ( port_s, sizeof( port_s ), NULL, "%u", port );

                    struct addrinfo * res = NULL;
                    int ret = getaddrinfo ( hostname, port_s, & hints, & res );
                    if ( ret == 0 && res != NULL )
                    {
                        struct sockaddr_in * ipv4
                            = ( struct sockaddr_in * ) res -> ai_addr;
                        string_copy_measure ( ep -> ip_address,
                            sizeof ep -> ip_address,
                            inet_ntoa ( ipv4->sin_addr ) );
                        STATUS ( STAT_PRG, "%s resolved to %s\n",
                                    hostname , ep -> ip_address );

                        ep -> type = epIPV4;
                        ep -> u . ipv4 . addr = htonl ( ipv4 ->sin_addr.s_addr );
                        ep -> u . ipv4 . port = ( uint16_t ) port;

                        freeaddrinfo( res );
                    }
                    else switch ( ret )
                    {
                    case EAI_NONAME: /* The specified host is unknown */
                        rc = RC ( rcNS, rcNoTarg, rcValidating, rcConnection, rcNotFound );
                        break;
                    case EAI_FAMILY: /* The requested name is valid but does not have an IPv4 address */
                        rc = RC ( rcNS, rcNoTarg, rcValidating, rcConnection, rcEmpty );
                        break;
                    case EAI_FAIL: /* A nonrecoverable name server error occured */
                        rc = RC ( rcNS, rcNoTarg, rcValidating, rcConnection, rcDestroyed );
                        break;
                    case EAI_AGAIN: /* A temporary error occured on an authoritative name server. Try again later */
                        rc = RC ( rcNS, rcNoTarg, rcValidating, rcConnection, rcBusy );
                        break;
                    default :
                        rc = RC ( rcNS, rcNoTarg, rcValidating, rcConnection, rcUnknown );
                    }
                }
            }

            if ( hostname != buffer )
                KDataBufferWhack ( & b );
        }

        if ( rc != 0 )
            memset ( ep, 0, sizeof * ep );        
    }

    return rc;
}

#ifdef WINSOCK_DEP_NO_WARN_WAS_DEFINED
#undef WINSOCK_DEP_NO_WARN_WAS_DEFINED
#else
#undef _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
