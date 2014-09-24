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
 * this file contains the private parts (v-table) of a generic url-fetcher
    the user has to provide 3 things to the fetcher:
        (1) the url to fetch via vt->set_url()
        (2) eventually a context pointer via vt->set_context()
        (3) a callback function via vt->set_callback()
    before the user can "start" the fetcher:
        via vt->fetch()
 */

#ifndef _h_url_fetcher_impl_
#define _h_url_fetcher_impl_

#ifndef _h_kns_extern_
#include <kns/extern.h>
#endif

#ifndef _h_kfs_file_
#include <kfs/file.h>
#endif

#ifndef _h_url_fetcher_
#include <kns/url-fetcher.h>
#endif

#ifndef _h_klib_refcount_
#include <klib/refcount.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
typedef union KUrlFetcher_vt KUrlFetcher_vt;

/*--------------------------------------------------------------------------
 * KUrl_fetcher
 *  a virtual url-fetcher
 */
struct KUrlFetcher
{
    const KUrlFetcher_vt *vt;
    KRefcount refcount;
};


#ifndef KURL_FETCHER_IMPL
#define KURL_FETCHER_IMPL KUrlFetcher
#endif


typedef struct KUrlFetcher_vt_v1 KUrlFetcher_vt_v1;
struct KUrlFetcher_vt_v1
{
    /* version == 1.x */
    uint32_t maj;
    uint32_t min;

    /* start minor version == 0 */
    rc_t ( CC * destroy ) ( KURL_FETCHER_IMPL *self );
    rc_t ( CC * read ) ( KURL_FETCHER_IMPL *self, const char * uri,
                      void *dst, size_t to_read, size_t *num_read );
    rc_t ( CC * get_size ) ( KURL_FETCHER_IMPL *self, size_t *num_size );
    /* end minor version == 0 */
};

union KUrlFetcher_vt
{
    KUrlFetcher_vt_v1 v1;
};

/* Init
 */
KNS_EXTERN rc_t CC KUrlFetcherInit ( KUrlFetcher *self, const KUrlFetcher_vt *vt );

#ifdef __cplusplus
}
#endif

#endif
