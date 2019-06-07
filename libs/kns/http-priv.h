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

/*
rc_t KClientHttpOpen ( struct KClientHttp * self, const String * hostname, uint32_t port );
*/
void KClientHttpClose ( struct KClientHttp * self );
rc_t KClientHttpReopen ( struct KClientHttp * self );

rc_t KNSManagerMakeClientHttpInt ( struct KNSManager const *self, struct KClientHttp **_http,
    const KDataBuffer *hostname_buffer,  struct KStream *opt_conn,
    ver_t vers, int32_t readMillis, int32_t writeMillis,
    const String *host, uint32_t port, bool reliable, bool tls );

/* test */
/*
void KClientHttpForceSocketClose(const struct KClientHttp *self);
*/

/*--------------------------------------------------------------------------
 * KClientHttpRequest
 */
rc_t KClientHttpMakeRequestInt ( struct KClientHttp const *self,
    struct KClientHttpRequest **req, const struct URLBlock *block, const KDataBuffer *buf );


void KClientHttpGetRemoteEndpoint ( const struct KClientHttp * self,
                                    struct KEndPoint * ep );
void KClientHttpGetLocalEndpoint ( const struct KClientHttp * self,
                                   struct KEndPoint * ep );

/* if the request followed redirects, the final URL will be different than the initial URL */
rc_t KClientHttpRequestURL ( struct KClientHttpRequest const *self, KDataBuffer *rslt );
    

/* exported private functions
*/

/* a hook to redefine KClientHttpReopen (for testing,_DEBUG only) */
#if _DEBUGGING
extern void SetClientHttpReopenCallback ( struct KStream * (*fn) ( void ) ); 
#endif

#ifdef __cplusplus
}
#endif

#endif /* _h_kttp_priv_ */
