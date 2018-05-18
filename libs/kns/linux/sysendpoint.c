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
#include <klib/printf.h>
#include <klib/rc.h>
#include <klib/status.h> /* STATUS */
#include <klib/data-buffer.h>
#include <klib/debug.h> /* DBGMSG */

#include "stream-priv.h"

#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <assert.h>

#include <errno.h>  /* ERANGE definition */

#include <sysalloc.h>

extern int h_errno;

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
                    char BB [ 1024 ];
                    struct hostent ret;
                    struct hostent * remote = NULL;
                    int h_errnop = 0;
                    int ghbnr = 0;

                    ghbnr = gethostbyname_r (
                                        hostname,
                                        & ret,
                                        BB,
                                        sizeof ( BB ),
                                        & remote,
                                        & h_errnop
                                        );
                    if ( ghbnr == 0 && remote != NULL )
                    { 
                        struct in_addr ** addr_list
                            = ( struct in_addr ** ) remote -> h_addr_list;
                        string_copy_measure ( ep -> ip_address,
                            sizeof ep -> ip_address,
                            inet_ntoa ( * addr_list [ 0 ] ));
                        STATUS ( STAT_PRG, "%s resolved to %s\n",
                                           hostname , ep -> ip_address );

                        ep -> type = epIPV4;
                        memmove ( & ep -> u . ipv4 . addr, remote -> h_addr_list [ 0 ], sizeof ep -> u . ipv4 . addr );
                        ep -> u . ipv4 . addr = htonl ( ep -> u . ipv4 . addr );
                        ep -> u . ipv4 . port = ( uint16_t ) port;
                    }
                    else switch ( h_errnop )
                    {
                    case HOST_NOT_FOUND: /* The specified host is unknown */
                        rc = RC ( rcNS, rcNoTarg, rcValidating, rcConnection, rcNotFound );
                        break;
                    case NO_ADDRESS: /* The requested names valid but does not have an IP address */
                        rc = RC ( rcNS, rcNoTarg, rcValidating, rcConnection, rcInconsistent );
                        break;
#if ! defined NO_ADDRESS || ! defined NO_DATA || NO_ADDRESS != NO_DATA
                    case NO_DATA: /* The requested name s valid but does not have an IP address */
                        rc = RC ( rcNS, rcNoTarg, rcValidating, rcConnection, rcEmpty );
                        break;
#endif
                    case NO_RECOVERY: /* A nonrecoverable name server error occured */
                        rc = RC ( rcNS, rcNoTarg, rcValidating, rcConnection, rcDestroyed );
                        break;
                    case TRY_AGAIN: /* A temporary error occured on an authoritative name server. Try again later */
                        rc = RC ( rcNS, rcNoTarg, rcValidating, rcConnection, rcBusy );
                        break;
                    case ERANGE:
                        rc = RC ( rcNS, rcNoTarg, rcValidating, rcConnection, rcExhausted );
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
