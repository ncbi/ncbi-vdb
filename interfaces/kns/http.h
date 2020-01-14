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

#ifndef _h_kns_http_
#define _h_kns_http_

#ifndef _h_kns_extern_
#include <kns/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KFile;
struct String;
struct KStream;
struct KNSManager;


/*--------------------------------------------------------------------------
 * KNSManager
 */

/* SetHTTPTimeouts
 *  sets default read/write timeouts to supply to HTTP connections
 *
 *  "readMillis" [ IN ] and "writeMillis" - when negative, infinite timeout
 *   when 0, return immediately, positive gives maximum wait time in mS
 *   for reads and writes respectively.
 */
KNS_EXTERN rc_t CC KNSManagerSetHTTPTimeouts ( struct KNSManager * self,
    int32_t readMillis, int32_t writeMillis );


/* GetHTTPProxyPath
 *  returns path to HTTP proxy server ( if set ) or NULL.
 *  return status is 0 if the path is valid, non-zero otherwise
 *
 *  returned reference to String must be freed via StringWhack.
 *
 * DEPRECATED : THIS FUNCTION IS INCORRECT !!!
 */
KNS_EXTERN rc_t CC KNSManagerGetHTTPProxyPath ( struct KNSManager const * self,
    struct String const ** proxy );


/* SetHTTPProxyPath
 *  sets a path to HTTP proxy server.
 *  a NULL path format value removes all proxy settings.
 */
KNS_EXTERN rc_t CC KNSManagerSetHTTPProxyPath ( struct KNSManager * self,
    const char * fmt, ... );
KNS_EXTERN rc_t CC KNSManagerVSetHTTPProxyPath ( struct KNSManager * self,
    const char * fmt, va_list args );


/* GetHTTPProxyEnabled
 *  returns true iff a non-NULL proxy path exists and user wants to use it
 *  users indicate desire to use proxy through configuration or SetHTTPProxyEnabled
 */
KNS_EXTERN bool CC KNSManagerGetHTTPProxyEnabled ( struct KNSManager const * self );


/* SetHTTPProxyEnabled
 *  sets http-proxy enabled state to supplied value
 *  returns the prior value as a convenience
 */
KNS_EXTERN bool CC KNSManagerSetHTTPProxyEnabled ( struct KNSManager * self, bool enabled );


/*------------------------------------------------------------------------------
 * KFile
 *  a KFile over HTTP
 */

/* Make
 */
KNS_EXTERN rc_t CC KNSManagerMakeHttpFile ( struct KNSManager const *self,
    struct KFile const **file, struct KStream *conn, ver_t vers, const char *url, ... );
KNS_EXTERN rc_t CC KNSManagerVMakeHttpFile ( struct KNSManager const *self,
    struct KFile const **file, struct KStream *conn, ver_t vers, const char *url, va_list args );


/*--------------------------------------------------------------------------
 * KClientHttp
 *  hyper text transfer protocol
 */
typedef struct KClientHttp KClientHttp, KHttp;


/* MakeClientHttp
 *  create an HTTP protocol
 *
 *  "http" [ OUT ] - return parameter for HTTP object
 *
 *  "opt_conn" [ IN, NULL OKAY ] - previously opened stream for communications.
 *
 *  "vers" [ IN ] - http version
 *   the only legal types are 1.0 ( 0x01000000 ) and 1.1 ( 0x01010000 )
 *
 *  "readMillis" [ IN ] and "writeMillis" - when negative, infinite timeout
 *   when 0, return immediately, positive gives maximum wait time in mS
 *   for reads and writes respectively.
 *
 *  "host" [ IN ] - parameter to give the host dns name for the connection
 *
 *  "port" [ IN, DEFAULT ZERO ] - if zero, defaults to standard for scheme
 *   if non-zero, is taken as explicit port specification
 */
KNS_EXTERN rc_t CC KNSManagerMakeClientHttp ( struct KNSManager const *self,
    KClientHttp **http, struct KStream *conn, ver_t vers,
    struct String const *host, uint32_t port );

KNS_EXTERN rc_t CC KNSManagerMakeTimedClientHttp ( struct KNSManager const *self,
    KClientHttp **http, struct KStream *opt_conn, ver_t vers,
    int32_t readMillis, int32_t writeMillis,
    struct String const *host, uint32_t port );


/* MakeClientHttps
 *  create an HTTPS protocol
 *
 *  "https" [ OUT ] - return parameter for HTTPS object
 *
 *  "opt_conn" [ IN, NULL OKAY ] - previously opened stream for communications.
 *
 *  "vers" [ IN ] - https version
 *   the only legal types are 1.0 ( 0x01000000 ) and 1.1 ( 0x01010000 )
 *
 *  "readMillis" [ IN ] and "writeMillis" - when negative, infinite timeout
 *   when 0, return immediately, positive gives maximum wait time in mS
 *   for reads and writes respectively.
 *
 *  "host" [ IN ] - parameter to give the host dns name for the connection
 *
 *  "port" [ IN, DEFAULT ZERO ] - if zero, defaults to standard for scheme
 *   if non-zero, is taken as explicit port specification
 */
KNS_EXTERN rc_t CC KNSManagerMakeClientHttps ( struct KNSManager const *self,
    KClientHttp **https, struct KStream *conn, ver_t vers,
    struct String const *host, uint32_t port );

KNS_EXTERN rc_t CC KNSManagerMakeTimedClientHttps ( struct KNSManager const *self,
    KClientHttp **https, struct KStream *opt_conn, ver_t vers,
    int32_t readMillis, int32_t writeMillis,
    struct String const *host, uint32_t port );


/* AddRef
 * Release
 *  ignores NULL references
 */
KNS_EXTERN rc_t CC KClientHttpAddRef ( const KClientHttp *self );
KNS_EXTERN rc_t CC KClientHttpRelease ( const KClientHttp *self );

/* compatibility for existing code */
#define KNSManagerMakeHttp KNSManagerMakeClientHttp
#define KNSManagerMakeTimedHttp KNSManagerMakeTimedClientHttp
#define KHttpAddRef KClientHttpAddRef
#define KHttpRelease KClientHttpRelease


/*------------------------------------------------------------------------------
 * KClientHttpRequest
 *  hyper text transfer protocol
 *  a client request
 */
typedef struct KClientHttpRequest KClientHttpRequest, KHttpRequest;


/* MakeRequest
 *  create a request that can be used to contact HTTP server
 *
 *  "req" [ OUT ] - return parameter for HTTP request object
 *
 *  "vers" [ IN ] - http version
 *
 *  "conn" [ IN, NULL OKAY ] - previously opened stream for communications.
 *
 *  "url" [ IN ] - full resource identifier. if "conn" is NULL,
 *   the url is parsed for remote endpoint and is opened by mgr.
 */
KNS_EXTERN rc_t CC KClientHttpMakeRequest ( const KClientHttp *self,
    KClientHttpRequest **req, const char *url, ... );

KNS_EXTERN rc_t CC KNSManagerMakeClientRequest ( struct KNSManager const *self,
    KClientHttpRequest **req, ver_t version, struct KStream *conn, const char *url, ... );


/* AddRef
 * Release
 *  ignores NULL references
 */
KNS_EXTERN rc_t CC KClientHttpRequestAddRef ( const KClientHttpRequest *self );
KNS_EXTERN rc_t CC KClientHttpRequestRelease ( const KClientHttpRequest *self );


/* Connection
 *  sets connection management headers
 *
 *  "close" [ IN ] - if "true", inform the server to close the connection
 *   after its response ( default for version 1.0 ). when "false" ( default
 *   for version 1.1 ), ask the server to keep the connection open.
 *
 * NB - the server is not required to honor the request
 */
KNS_EXTERN rc_t CC KClientHttpRequestConnection ( KClientHttpRequest *self, bool close );


/* SetNoCache
 *  guard against over-eager proxies that try to cache entire files
 *  and handle byte-ranges locally.
 */
KNS_EXTERN rc_t CC KClientHttpRequestSetNoCache ( KClientHttpRequest *self );


/* ByteRange
 *  set requested byte range of response
 *
 *  "pos" [ IN ] - beginning offset within remote entity
 *
 *  "bytes" [ IN ] - the number of bytes being requested
 */
KNS_EXTERN rc_t CC KClientHttpRequestByteRange ( KClientHttpRequest *self, uint64_t pos, size_t bytes );


/* AddHeader
 *  allow addition of an arbitrary HTTP header to message
 */
KNS_EXTERN rc_t CC KClientHttpRequestAddHeader ( KClientHttpRequest *self,
    const char *name, const char *val, ... );

/* GetHeader
 *  retrieve named header if present
 *  this can potentially return a comma separated value list
 */
KNS_EXTERN rc_t CC KClientHttpRequestGetHeader ( const KClientHttpRequest *self, const char *name,
    char *buffer, size_t bsize, size_t *num_read );

/* GetHost
 *  retrieve host
 */
KNS_EXTERN rc_t CC KClientHttpRequestGetHost(const KClientHttpRequest *self,
    char *buffer, size_t bsize, size_t *num_read);

/* GetPath
 *  retrieve path
 */
KNS_EXTERN rc_t CC KClientHttpRequestGetPath(const KClientHttpRequest *self,
    char *buffer, size_t bsize, size_t *num_read);

/* AddQueryParam
 *  adds a parameter to the query part of the URL, as "[opt_name=]value" (value is the result of formatting)
 *  inserts ? before the first parameter and & between parameters
 *  if opt_name is NULL or empty, there will be no '=' before the value
 *  URL-encodes the formatted value
 */
KNS_EXTERN rc_t CC KClientHttpRequestAddQueryParam ( KClientHttpRequest *self, const char * opt_name, const char *fmt, ... );
KNS_EXTERN rc_t CC KClientHttpRequestVAddQueryParam ( KClientHttpRequest *self, const char * opt_name, const char *fmt, va_list args );

/* AddPostParam
 *  adds a parameter for POST
 */
KNS_EXTERN rc_t CC KClientHttpRequestAddPostParam ( KClientHttpRequest *self, const char *fmt, ... );
KNS_EXTERN rc_t CC KClientHttpRequestVAddPostParam ( KClientHttpRequest *self, const char *fmt, va_list args );

/* AddPostFileParam
 *  adds a file to be transmitted with POST on the URL in the form "name=<base64encodedContentsOfTheFile>"
 *  "name" [ IN ] - field name identifying this file in the resulting HTML form. An empty name is OK
 *  "path" [ IN ] - NUL terminated string in directory-native character set denoting target file
**/
KNS_EXTERN rc_t CC KClientHttpRequestAddPostFileParam ( KClientHttpRequest *self, const char *name, const char *path );

KNS_EXTERN rc_t CC KClientHttpRequestSetCloudParams(KClientHttpRequest * self,
    bool ceRequired, bool payRequired);

/* compatibility for existing code */
#define KHttpMakeRequest KClientHttpMakeRequest
#define KNSManagerMakeRequest KNSManagerMakeClientRequest
#define KHttpRequestAddRef KClientHttpRequestAddRef
#define KHttpRequestRelease KClientHttpRequestRelease
#define KHttpRequestConnection KClientHttpRequestConnection
#define KHttpRequestByteRange KClientHttpRequestByteRange
#define KHttpRequestAddHeader KClientHttpRequestAddHeader
#define KHttpRequestAddPostParam KClientHttpRequestAddPostParam
#define KHttpRequestVAddPostParam KClientHttpRequestVAddPostParam
#define KHttpRequestSetCloudParams KClientHttpRequestSetCloudParams

/*--------------------------------------------------------------------------
 * KClientHttpResult
 *  hyper text transfer protocol
 */
typedef struct KClientHttpResult KClientHttpResult, KHttpResult;


/* AddRef
 * Release
 *  ignores NULL references
 */
KNS_EXTERN rc_t CC KClientHttpResultAddRef ( const KClientHttpResult *self );
KNS_EXTERN rc_t CC KClientHttpResultRelease ( const KClientHttpResult *self );


/* HEAD
 *  send HEAD message
 */
KNS_EXTERN rc_t CC KClientHttpRequestHEAD ( KClientHttpRequest *self, KClientHttpResult **rslt );

/* GET
 *  send GET message
 *  all query AND post parameters are combined in URL
 */
KNS_EXTERN rc_t CC KClientHttpRequestGET ( KClientHttpRequest *self, KClientHttpResult **rslt );

/* POST
 *  send POST message
 *  query parameters are sent in URL
 *  post parameters are sent in body
 */
KNS_EXTERN rc_t CC KClientHttpRequestPOST ( KClientHttpRequest *self, KClientHttpResult **rslt );


/* Status
 *  access the response status code
 *  and optionally the message
 *
 *  "code" [ OUT ] - return parameter for status code
 *
 *  "msg_buff" [ IN, NULL OKAY ] and "buff_size" [ IN, ZERO OKAY ] -
 *   buffer for capturing returned message
 *
 *  "msg_size" [ OUT, NULL OKAY ] - size of returned message in bytes
 */
KNS_EXTERN rc_t CC KClientHttpResultStatus ( const KClientHttpResult *self, uint32_t *code,
    char *msg_buff, size_t buff_size, size_t *msg_size );


/* KeepAlive
 *  retrieves keep-alive property of response
 *  requires HTTP/1.1
 */
KNS_EXTERN bool CC KClientHttpResultKeepAlive ( const KClientHttpResult *self );


/* Range
 *  retrieves position and partial size for partial requests
 *
 *  "pos" [ OUT ] - offset to beginning portion of response
 *
 *  "bytes" [ OUT ] - size of range
 */
KNS_EXTERN rc_t CC KClientHttpResultRange ( const KClientHttpResult *self, uint64_t *pos, size_t *bytes );


/* Size
 *  retrieves overall size of entity, if known
 *
 *  "response_size" [ OUT ] - size in bytes of response
 *   this is the number of bytes that may be expected from the input stream
 */
KNS_EXTERN bool CC KClientHttpResultSize ( const KClientHttpResult *self, uint64_t *size );


/* AddHeader
 *  allow addition of an arbitrary HTTP header to RESPONSE
 *  this can be used to repair or normalize odd server behavior
 */
KNS_EXTERN rc_t CC KClientHttpResultAddHeader ( KClientHttpResult *self,
    const char *name, const char *val, ... );


/* GetHeader
 *  retrieve named header if present
 *  this can potentially return a comma separated value list
 */
KNS_EXTERN rc_t CC KClientHttpResultGetHeader ( const KClientHttpResult *self, const char *name,
    char *buffer, size_t bsize, size_t *num_read );


/* TestHeaderValue
 *  test for existence of header and a particular value
 *  if the header exists and has a comma-separated list of values,
 *  test each value individually, i.e. splits on comma before comparison
 */
KNS_EXTERN bool CC KClientHttpResultTestHeaderValue ( const KClientHttpResult *self,
    const char *name, const char *value );


/* GetInputStream
 *  access the body of response as a stream
 *  only reads are supported
 *
 *  "s" [ OUT ] - return parameter for input stream reference
 *   must be released via KStreamRelease
 */
KNS_EXTERN rc_t CC KClientHttpResultGetInputStream ( KClientHttpResult *self,
    struct KStream  ** s );

KNS_EXTERN bool CC KFileIsKHttpFile ( const struct KFile * self );

    /* compatibility defines */
#define KHttpResultAddRef KClientHttpResultAddRef
#define KHttpResultRelease KClientHttpResultRelease
#define KHttpRequestHEAD KClientHttpRequestHEAD
#define KHttpRequestGET KClientHttpRequestGET
#define KHttpRequestPOST KClientHttpRequestPOST
#define KHttpResultStatus KClientHttpResultStatus
#define KHttpResultKeepAlive KClientHttpResultKeepAlive
#define KHttpResultRange KClientHttpResultRange
#define KHttpResultSize KClientHttpResultSize
#define KHttpResultAddHeader KClientHttpResultAddHeader
#define KHttpResultGetHeader KClientHttpResultGetHeader
#define KHttpResultGetInputStream KClientHttpResultGetInputStream


#ifdef __cplusplus
}
#endif

#endif /* _h_kns_http_ */
