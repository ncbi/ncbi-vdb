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

#ifndef _h_http_priv_
#define _h_http_priv_

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifndef _h_klib_text_
#include <klib/text.h>
#endif

#ifndef _h_klib_data_buffer
#include <klib/data-buffer.h>
#endif

#ifndef _h_klib_container
#include <klib/container.h>
#endif

#ifndef _h_klib_refcount_
#include <klib/refcount.h>
#endif

#ifndef _h_kns_mgr_priv_
#include <kns/kns-mgr-priv.h>
#endif

#ifndef _h_kns_http_
#include <kns/http.h>
#endif

#ifndef _h_kns_endpoint_
#include <kns/endpoint.h>
#endif

#ifndef MAX_HTTP_READ_LIMIT
#define MAX_HTTP_READ_LIMIT ( 5 * 60 * 1000 ) /* 5 minutes */
#endif

#ifndef MAX_HTTP_WRITE_LIMIT
#define MAX_HTTP_WRITE_LIMIT ( 15 * 1000 )
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct KFile;
struct KNSManager;
struct KClientHttp;
struct KClientHttpRequest;
struct KEndPoint;
struct KStream;
struct timeout_t;
struct URLBlock;

/* Form of Request-URI in HTTP Request-Line used when connecting via proxy */
typedef enum {
    eUFUndefined,

    /* absoluteURI: https://tools.ietf.org/html/rfc2616#section-5.1.2
       standard form recommended to use with proxies */
    eUFAbsolute,

    /* origin-form: https://tools.ietf.org/html/rfc7230#section-5.3.1
       use it when connect to googleapis.com: it rejects absoluteURI */
    eUFOrigin,
} EUriForm;

/*--------------------------------------------------------------------------
 * KHttpHeader
 *  node structure to place http header lines into a BSTree
 */
typedef struct KHttpHeader KHttpHeader;
struct KHttpHeader
{
    BSTNode dad;
    String name;
    String value;
    KDataBuffer value_storage;
};

extern void KHttpHeaderWhack ( BSTNode *n, void *ignore );
extern int64_t CC KHttpHeaderSort ( const BSTNode *na, const BSTNode *nb );
extern int64_t CC KHttpHeaderCmp ( const void *item, const BSTNode *n );

extern rc_t KClientHttpGetHeaderLine ( struct KClientHttp *self, struct timeout_t *tm, BSTree *hdrs, bool *blank, bool * len_zero, bool *close_connection );
extern rc_t KClientHttpGetStatusLine ( struct KClientHttp *self, struct timeout_t *tm, String *msg, uint32_t *status, ver_t *version );


/* compatibility for existing code */

#define KHttpGetHeaderLine KClientHttpGetHeaderLine
#define KHttpGetStatusLine KClientHttpGetStatusLine


rc_t KClientHttpGetHeaderLine ( struct KClientHttp * self,
    struct timeout_t * tm, BSTree * hdrs, bool * blank, bool *  len_zero,
    bool * close_connection );
rc_t KClientHttpGetStatusLine ( struct KClientHttp * self,
    struct timeout_t * tm, String * msg, uint32_t * status, ver_t * version );


/*--------------------------------------------------------------------------
 * KClientHttp
 */

struct KClientHttp
{
    const KNSManager *mgr;
    struct KStream * sock;
    struct KStream * test_sock; /* if not NULL, use to communicate with a mocked server in testing, do not reopen on redirects */

    /* buffer for accumulating response data from "sock" */
    KDataBuffer block_buffer;
    size_t block_valid;         /* number of valid response bytes in buffer            */
    size_t block_read;          /* number of bytes read out by line reader or stream   */
    size_t body_start;          /* offset to first byte in body                        */

    KDataBuffer line_buffer;    /* data accumulates for reading headers and chunk size */
    size_t line_valid;

    KDataBuffer hostname_buffer;
    String hostname;
    uint32_t port;

    ver_t vers;

    KRefcount refcount;

    int32_t read_timeout;
    int32_t write_timeout;

    /* Remote EndPoint */
    KEndPoint ep;
    bool ep_valid;
    bool proxy_ep;
    bool proxy_default_port;

    KEndPoint local_ep; /* Local EndPoint */

    bool reliable;
    bool tls;

    bool close_connection;

    EUriForm uf; /* Form of Request-URI in Request-Line when using proxy */
};

void KClientHttpClose ( struct KClientHttp * self );
rc_t KClientHttpReopen ( struct KClientHttp * self );

rc_t KNSManagerMakeClientHttpInt ( struct KNSManager const *self, struct KClientHttp **_http,
    const KDataBuffer *hostname_buffer,  struct KStream *opt_conn,
    ver_t vers, int32_t readMillis, int32_t writeMillis,
    const String *host, uint32_t port, bool reliable, bool tls );

rc_t KClientHttpVAddHeader ( BSTree *hdrs, bool add, const char *_name, const char *_val, va_list args );
rc_t KClientHttpFindHeader ( const BSTree *hdrs, const char *_name, char *buffer, size_t bsize, size_t *num_read );
rc_t KClientHttpAddHeader ( BSTree *hdrs, const char *name, const char *val, ... );
rc_t KClientHttpReplaceHeader ( BSTree *hdrs, const char *name, const char *val, ... );

rc_t KClientHttpClear ( KClientHttp *self );
rc_t KClientHttpInit ( KClientHttp * http, const KDataBuffer *hostname_buffer, ver_t _vers, const String * _host, uint32_t port, bool tls );

rc_t KClientHttpSendReceiveMsg ( KClientHttp *self, KClientHttpResult **rslt,
    const char *buffer, size_t len, const KDataBuffer *body, const char *url );

/*--------------------------------------------------------------------------
 * KClientHttpRequest
 */

struct KClientHttpRequest
{
    struct KClientHttp * http;

    URLBlock url_block;
    KDataBuffer url_buffer;

    KDataBuffer body;

    BSTree hdrs;

    KRefcount refcount;
    bool accept_not_modified;

    bool ceRequired; /* computing environment token required to access this URL */
    bool payRequired; /* payment info required to access this URL */

    bool rangeRequested;
};

void KClientHttpGetRemoteEndpoint ( const struct KClientHttp * self,
                                    struct KEndPoint * ep );
void KClientHttpGetLocalEndpoint ( const struct KClientHttp * self,
                                   struct KEndPoint * ep );

rc_t KClientHttpRequestAttachEnvironmentToken( struct KClientHttpRequest * self );

/* exported private functions
*/

const char * KClientHttpRequestGetBody( struct KClientHttpRequest * self );

/* exported private functions
*/

/* a hook to redefine KClientHttpReopen (for testing,_DEBUG only) */
#if _DEBUGGING
extern void SetClientHttpReopenCallback ( struct KStream * (*fn) ( void ) );
#endif

/*--------------------------------------------------------------------------
 * KClientHttpResult
 *  hyper text transfer protocol
 *  Holds all the headers in a BSTree
 *  Records the status msg, status code and version of the response
 */
struct KClientHttpResult
{
    KClientHttp *http;

    BSTree hdrs;

    String msg;
    uint32_t status;
    ver_t version;

    KRefcount refcount;
    bool len_zero;

    char * expiration;

    bool rangeRequested;
};

/* internal encodiung  function, exposed for testing */
extern rc_t KClientHttpRequestUrlEncodeBase64(const String ** encoding);

#ifdef __cplusplus
}
#endif

#endif /* _h_kttp_priv_ */
