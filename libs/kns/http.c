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
* ==============================================================================
*
*/
#include <kns/extern.h>

#include <kns/manager.h>
#include <kns/http.h>
#include <kns/adapt.h>
#include <kns/endpoint.h>
#include <kns/socket.h>
#include <kns/stream.h>
#include <kns/impl.h>
#include <kfs/file.h>
#include <kfs/directory.h>

#ifdef ERR
#undef ERR
#endif

#include <klib/text.h>
#include <klib/container.h>
#include <klib/debug.h> /* DBGMSG */
#include <klib/out.h>
#include <klib/log.h>
#include <klib/refcount.h>
#include <klib/rc.h>
#include <klib/printf.h>
#include <klib/vector.h>
#include <kproc/timeout.h>

#include <os-native.h>
#include <strtol.h>
#include <va_copy.h>

#include "mgr-priv.h"
#include "stream-priv.h"

#include <sysalloc.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "http-priv.h"

/*--------------------------------------------------------------------------
 * URLBlock
 *  RFC 3986
 *
 * TBD - replace with VPath
 */

/* Init
 *  Initialize to default values in case portions are missing
 */
void URLBlockInit ( URLBlock *self )
{
    CONST_STRING ( & self -> scheme, "http" );
    CONST_STRING ( & self -> host, "" );
    CONST_STRING ( & self -> path, "/" );
    CONST_STRING ( & self -> query, "" );
    CONST_STRING ( & self -> fragment, "" );

    self -> port = 0; /* 0 = DEFAULT 80 for http, 443 for https */

    self -> scheme_type = st_NONE;
    self -> tls = false;

    self -> port_dflt = true;

    self->cloud_type = ct_NONE;
}

/* ParseUrl
 *  accept standard, full http URL:
 *    <scheme>://<host>[:<port>]/<path>[?<query>][#<fragment>]
 *
 *  scheme can be missing, i.e.:
 *    //<host>[:<port>]/<path>[?<query>][#<fragment>]
 *
 *  we can also accept missing path[query][fragment], i.e.:
 *    <scheme>://<host>[:<port>]
 *    //<host>[:<port>]
 *
 *  finally, we can accept path without host, i.e.:
 *    /<path>[?<query>][#<fragment>]
 *
 *  patterns to reject:
 *    <scheme>:/<path>...    # scheme followed by anything other than '//'
 *    <path>...              # no leading '/'
 */
rc_t ParseUrl ( URLBlock * b, const char * url, size_t url_size ) 
{
    rc_t rc;
    char * sep;
    const char * buf = url;
    const char * end = buf + url_size;

    bool have_host, have_scheme;

    URLBlockInit ( b );

    /* scheme default to false because url may be a path */
    have_scheme = false;

    /* check if url is empty
       scheme cannot start with a forward slash - detecting an absolute path */
    if ( buf < end && buf [ 0 ] != '/' )
    {
        /* here we identify the scheme by finding the ':' */
        sep = string_chr ( url, end - buf, ':' );
        if ( sep != NULL )
        {
            String http;
            CONST_STRING ( & http, "http" );

            /* assign scheme to the url_block */
            StringInit ( & b -> scheme, buf, sep - buf, ( uint32_t ) ( sep - buf ) );

            /* here we assume the scheme will be http */
            b -> port = 80;
            b -> scheme_type = st_HTTP;
            if ( ! StringCaseEqual ( & b -> scheme, & http ) )
            {
                String https;
                CONST_STRING ( & https, "https" );

                /* check for https */
                b -> port = 443;
                b -> scheme_type = st_HTTPS;
                b -> tls = true;
                if ( ! StringCaseEqual ( & b -> scheme, & https ) )
                {
                    String s3;
                    CONST_STRING ( & s3, "s3" );
                
                    /* it is not http, check for s3 */
                    b -> port = 80;
                    b -> scheme_type = st_S3;
                    b -> tls = false;
                    if ( ! StringCaseEqual ( & b -> scheme, & s3 ) )
                    {
                        b -> port = 0;
                        b -> scheme_type = st_NONE;
                        rc = RC ( rcNS, rcUrl, rcEvaluating, rcName, rcIncorrect );
                        PLOGERR ( klogErr ,( klogErr, rc, "Scheme is '$(scheme)'", "scheme=%S", & b -> scheme ) );
                        return rc;
                    }
                }
            }

            /* accept scheme - skip past */
            buf = sep + 1;
            have_scheme = true;
        }
    }
    
    /* discard fragment - not sending to server, but still record it */
    sep = string_rchr ( buf, end - buf,  '#' );
    if ( sep != NULL )
    {
        /* advance to first character in fragment */
        const char *frag = sep + 1;

        /* assign fragment to the url_block */
        StringInit ( & b -> fragment, frag, end - frag, ( uint32_t ) ( end - frag ) );

        /* remove fragment from URL */
        end = sep;
    }
                         
    /* detect host */
    have_host = false;
    
    /* check for '//' in the first two elements 
       will fail if no scheme was detected */
    if ( string_match ( "//", 2, buf, end - buf, 2, NULL ) == 2 )
    {
        /* skip ahead to host spec */
        buf += 2;

        /* if we ran into the end of the string, we dont have a host */
        if ( buf == end )
        {
            rc = RC ( rcNS, rcUrl, rcParsing, rcOffset, rcIncorrect );
            PLOGERR ( klogErr ,( klogErr, rc, "expected hostspec in url '$(url)'", "url=%.*s", ( uint32_t ) url_size, url ) );
            return rc;
        }

        have_host = true;
    }

    /* if there is a scheme but no host, error */
    if ( have_scheme && ! have_host )
    {
        rc = RC ( rcNS, rcUrl, rcParsing, rcName, rcNotFound );
        PLOGERR ( klogErr ,( klogErr, rc, "Host is '$(host)'", "host=%s", "NULL" ) );
        return rc;
    }
        
    /* find dividing line between host and path, which MUST start with '/' */
    sep = string_chr ( buf, end - buf, '/' );

    /* detect no path */
    if ( sep == NULL )
    {
        /* no path and no host */
        if ( ! have_host )
        {
            rc = RC ( rcNS, rcUrl, rcParsing, rcName, rcNotFound );
            PLOGERR ( klogErr ,( klogErr, rc, "Path is '$(path)'", "path=%s", "/" ) );
            return rc;
        }
        /* no path but have host 
           default value for path is already '/' */
        sep = ( char* ) end;
    }

    /* capture host ( could be empty - just given a file system path ) */
    if ( have_host )
    {
        String amazonaws;
        String stor31;

        CONST_STRING(&amazonaws, "amazonaws.com");
        CONST_STRING(&stor31, "s3-stor31.st-va.ncbi.nlm.nih.gov");

        /* assign host to url_block */
        StringInit ( & b -> host, buf, sep - buf, ( uint32_t ) ( sep - buf ) );

        /* advance to path */
        buf = sep;

        /* detect "cloudy host" */
        if (b->host.size >= stor31.size) {
            size_t skip = b->host.size - stor31.size;
            if (string_cmp(stor31.addr, stor31.size, b->host.addr + skip,
                b->host.size - skip, stor31.size) == 0)
            {
                b->cloud_type = ct_S3;
            }
        }
        if (b->host.size >= amazonaws.size) {
            size_t skip = b->host.size - amazonaws.size;
            if (string_cmp(amazonaws.addr, amazonaws.size, b->host.addr + skip,
                b->host.size - skip, amazonaws.size) == 0)
            {
                b->cloud_type = ct_S3;
            }
        }
    }

    /* detect relative path 
       <hostname>/<path> - OK, handled above
       /<path> - OK
    */
    if ( buf != sep )
    {
        rc = RC ( rcNS, rcPath, rcParsing, rcOffset, rcIncorrect );
        PLOGERR ( klogErr ,( klogErr, rc, "Path is '$(path)'", "path=%s", "NULL" ) );
        return rc;
    }

    /* if we dont have a host we must have a path
       if we DO have a host and the path is not empty */
    if ( ! have_host || buf != end )
    {
        /* check for query */
        sep = string_chr ( buf, end - buf,  '?' );
        if ( sep != NULL )
        {
            const char *query = sep + 1;
            /* assign query to url_block */
            StringInit ( & b -> query, query, end - query, ( uint32_t ) ( end - query ) ); 

            /* advance end to sep */
            end = sep;
        }

        /* assign path ( could also be empty ) */
        StringInit ( & b -> path, buf, end - buf, ( uint32_t ) ( end - buf ) );
    }

    /* if we have a host, split on ':' to check for a port
       OK if not found */
    if ( have_host )
    {
        buf = b -> host . addr;
        end = buf + b -> host . size;

        /* check for port */
        sep = string_chr ( buf, end - buf,  ':' );
        if ( sep != NULL )
        {
            char *term;
            const char * port = sep + 1;
            /* assign port to url block converting to 32 bit int 
             term should point to end */
            b -> port = strtou32 ( port, & term, 10 );

            /* error if 0 or term isnt at the end of the buffer */
            if ( b -> port == 0 || ( const char* ) term != end )
            {
                rc = RC ( rcNS, rcUrl, rcParsing, rcNoObj, rcIncorrect );
                PLOGERR ( klogErr ,( klogErr, rc, "Port is '$(port)'", "port=%u", b -> port ) );
                return rc;
            }

            b -> port_dflt = false;

            /* assign host to url_block */
            StringInit ( & b -> host, buf, sep - buf, ( uint32_t ) ( sep - buf ) );
        }
    }

/*  DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_HTTP ),
        ( " ParseUrl (%.*s) = (path:%S)\n", ( int ) url_size, url,
                                                  & b -> path ) ); */

    return 0;
}

/*--------------------------------------------------------------------------
 * KHttpHeader
 *  node structure to place http header lines into a BSTree
 */

void CC KHttpHeaderWhack ( BSTNode *n, void *ignore )
{
    KHttpHeader * self = ( KHttpHeader* ) n;
    KDataBufferWhack ( & self -> value_storage );
    free ( self );
}

int64_t CC KHttpHeaderSort ( const BSTNode *na, const BSTNode *nb )
{
    const KHttpHeader *a = ( const KHttpHeader* ) na;
    const KHttpHeader *b = ( const KHttpHeader* ) nb;

    return StringCaseCompare ( & a -> name, & b -> name );
}

int64_t CC KHttpHeaderCmp ( const void *item, const BSTNode *n )
{
    const String *a = item;
    const KHttpHeader *b = ( const KHttpHeader * ) n;

    return StringCaseCompare ( a, & b -> name );
}
