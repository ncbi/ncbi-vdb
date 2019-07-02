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

#ifndef _h_http_file_priv_
#define _h_http_file_priv_

#define KFILE_IMPL KHttpFile
typedef struct KHttpFile KHttpFile;
#include <kfs/impl.h>

#include <kproc/lock.h>
#include <kns/http.h>
#include <kns/manager.h>
#include <kns/kns-mgr-priv.h>
#include <klib/data-buffer.h>
#include <klib/time.h>

/*--------------------------------------------------------------------------
 * KHttpFile
 */
struct KHttpFile
{
    KFile dad;

    uint64_t file_size;

    const KNSManager * kns;

    KLock * lock;
    KClientHttp *http;

    KDataBuffer orig_url_buffer;
    URLBlock block; /* the original URL, parsed */

    KDataBuffer url_buffer;

    bool url_is_temporary; /* The original request received a 307 Temp Redirect */
    KTime url_expiration; /* if url_is_temporary == true, refresh url_buffer using orig_url_buffer */

    /* if true, add environment token to the URL */
    bool need_env_token;

    /* if true, add user-account info headers to cloud URLs */
    bool payRequired;

    bool no_cache;
};

#endif
