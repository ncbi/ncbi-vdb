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


#ifndef _h_kns_http_priv_
#define _h_kns_http_priv_


#ifndef _h_klib_defs_
#include <klib/defs.h> /* rc_t */
#endif

#ifndef _h_kns_extern_
#include <kns/extern.h> /* KNS_EXTERN */
#endif


struct KClientHttpResult;
struct KDataBuffer;
struct URLBlock;

#ifdef __cplusplus
extern "C" {
#endif


KNS_EXTERN rc_t CC KClientHttpRequestFormatMsg (
    const struct KClientHttpRequest * self, struct KDataBuffer * buffer, const char * method );

KNS_EXTERN rc_t CC KClientHttpRequestFormatPostMsg (
    const struct KClientHttpRequest * self, struct KDataBuffer * buffer );

KNS_EXTERN rc_t CC KClientHttpResultFormatMsg (
    const struct KClientHttpResult * self, struct KDataBuffer * buffer, const char * bol, const char * eol );

rc_t KClientHttpMakeRequestInt ( struct KClientHttp const *self,
    struct KClientHttpRequest **req, const struct URLBlock *block, const struct KDataBuffer *buf );

/* if the request followed redirects, the final URL will be different than the initial URL */
rc_t KClientHttpRequestURL ( struct KClientHttpRequest const *self, struct KDataBuffer *rslt );

rc_t KClientHttpRequestClear ( struct KClientHttpRequest *self );

rc_t KClientHttpRequestInit ( struct KClientHttpRequest * req, const struct URLBlock *b, const struct KDataBuffer *buf );

rc_t KClientHttpRequestGetQuery( struct KClientHttpRequest * req, const struct String ** query );

#ifdef __cplusplus
}
#endif


#endif /* _h_kns_http_priv_ */
