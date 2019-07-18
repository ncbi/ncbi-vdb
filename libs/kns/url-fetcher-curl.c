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

#include <kns/extern.h>
#include <klib/rc.h>
#include <kns/manager.h>
#include <kns/url-fetcher.h>

/* IMPORTANT define this before including the *-impl.h !!!! */
#define KURL_FETCHER_IMPL KUrlFetcherCurl
#include "url-fetcher-impl.h"

#include "kns_mgr_priv.h"

#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>

/*
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
*/

/*--------------------------------------------------------------------------
 * KVectorNamelist ... a generic Namelist based on a Vector
 */
struct KUrlFetcherCurl
{
    KUrlFetcher dad;

    struct KNSManager *kns_mgr;
    CURL *curl_handle;

    /* this buffer is passed in from read() to the callback 
       it is owned by the caller not this class */
    char *buff;
    size_t buffsize;
    size_t in_buff;
    
    /* this is the spill-over buffer, used by the callback
       if the caller-provided buffer is too small */
    char *spill_over_buff;
    size_t spill_over_buffsize;
};

/* implementation of the the virtual KUrlFetcher-Interface for curl */

static
rc_t CC KUrlFetcherCurlDestroy ( KUrlFetcherCurl *self )
{
    self->kns_mgr->curl_easy_cleanup_fkt( self -> curl_handle );
    KNSManagerRelease( self->kns_mgr );

    if ( self -> spill_over_buff != NULL )
        free( self -> spill_over_buff );
    free ( self );
    return 0;
}

static
size_t CC KUrlFetcherCurlCallback( void *ptr, size_t size, size_t nmemb, void *data )
{
    size_t given_bytes = size * nmemb; /* calculate the size given in ptr */
    KUrlFetcherCurl *self = ( KUrlFetcherCurl * )data;
    if ( self != NULL )
    {
        if ( ( self -> in_buff + given_bytes ) > self -> buffsize )
        {
            /* the caller-provided buffer IS NOT enough... */
            size_t to_copy = ( self -> buffsize - self -> in_buff );
            if ( to_copy > 0 )
            {
                /* the caller-provided buffer can hold a part of it... */
                if ( self -> buff )
                {
                    memmove( &( self -> buff [ self -> in_buff ] ), ptr, given_bytes );
                    self -> in_buff += given_bytes;
                }
            }
            else
            {
                /* the spill-over-buffer has to take care of the rest... */
                size_t remaining = ( given_bytes - to_copy );
                if ( self -> spill_over_buff == NULL )
                    self -> spill_over_buff = malloc( remaining );
                else
                    self -> spill_over_buff = realloc( self -> spill_over_buff,
                                                       self -> spill_over_buffsize + remaining );
                if ( self -> spill_over_buff != NULL )
                {
                    memmove( &( self -> spill_over_buff [ self -> spill_over_buffsize ] ), ptr, remaining );
                    self -> spill_over_buffsize += remaining;
                }
            }
        }
        else
        {
            /* the caller-provided buffer IS enough... */
            if ( self -> buff )
            {
                memmove( &( self -> buff [ self -> in_buff ] ), ptr, given_bytes );
                self -> in_buff += given_bytes;
            }
        }
    }
    return given_bytes;
}


static
rc_t CC KUrlFetcherCurlRead( KUrlFetcherCurl *self, const char * uri,
                          void *dst, size_t to_read, size_t *num_read )
{
    CURLcode rcc;

    /* the virtual interface has already checked that self is not NULL */
    if ( uri == NULL || dst == NULL || to_read == 0 )
        return RC ( rcApp, rcFunction, rcConstructing, rcParam, rcNull );

    self -> buff = dst;
    self -> buffsize = to_read;
    self -> in_buff = 0;

    if ( self -> spill_over_buff != NULL )
        free( self -> spill_over_buff );
    self -> spill_over_buff = NULL;
    self -> spill_over_buffsize = 0;

    if ( self -> curl_handle == NULL )
        return RC ( rcApp, rcFunction, rcConstructing, rcParam, rcNull );

    rcc = self->kns_mgr->curl_easy_setopt_fkt( self -> curl_handle, CURLOPT_URL, uri );
    if ( rcc != CURLE_OK )
    {
        if ( num_read ) *num_read = 0;
        return RC ( rcApp, rcFunction, rcConstructing, rcParam, rcNull );
    }
    else
    {
        rcc = self->kns_mgr->curl_easy_perform_fkt( self-> curl_handle );
        if ( rcc != CURLE_OK )
        {
            if ( num_read ) *num_read = 0;
            return RC ( rcApp, rcFunction, rcConstructing, rcParam, rcNull );
        }
    }
    
    if ( num_read )
        *num_read = self -> in_buff;

    return 0;
}

static KUrlFetcher_vt_v1 sKUrlFetcherCurl_vt =
{
    1, 0,
    KUrlFetcherCurlDestroy,
    KUrlFetcherCurlRead,
};

LIB_EXPORT rc_t CC KUrlFetcherCurlMake( KUrlFetcher **fetcher, const bool verbose )
{
    rc_t rc;

    if ( fetcher == NULL )
        rc = RC ( rcApp, rcFunction, rcConstructing, rcParam, rcNull );
    else
    {
        KUrlFetcherCurl * tmp;
        *fetcher = NULL;
        tmp = malloc( sizeof( * tmp ) );
        if ( tmp != NULL )
        {
            rc = KUrlFetcherInit ( &(tmp)->dad, (const KUrlFetcher_vt*)&sKUrlFetcherCurl_vt );
            if ( rc != 0 )
                free( tmp );
            else
            {
                tmp -> buff = NULL;
                tmp -> buffsize = 0;
                tmp -> in_buff = 0;

                tmp -> spill_over_buff = NULL;
                tmp -> spill_over_buffsize = 0;

                rc = KNSManagerMake( &tmp->kns_mgr );
                if ( rc == 0 )
                {
                    tmp -> curl_handle = tmp -> kns_mgr -> curl_easy_init_fkt();
                    if ( tmp -> curl_handle == NULL )
                    {
                        KNSManagerRelease( tmp->kns_mgr );
                        free( tmp );
                        rc = RC ( rcApp, rcFunction, rcConstructing, rcParam, rcNull );
                    }
                    else
                    {
                        CURLcode rcc;
                        
                        if ( verbose )
                            tmp -> kns_mgr -> curl_easy_setopt_fkt( tmp -> curl_handle, CURLOPT_VERBOSE , 1 );
                        
                        rcc = tmp -> kns_mgr -> curl_easy_setopt_fkt( tmp -> curl_handle, CURLOPT_WRITEFUNCTION, KUrlFetcherCurlCallback );
                        if ( rcc != CURLE_OK )
                        {
                            KNSManagerRelease( tmp -> kns_mgr );
                            free( tmp );
                            rc = RC ( rcApp, rcFunction, rcConstructing, rcParam, rcNull );
                        }
                        else
                        {
                            rcc = tmp -> kns_mgr -> curl_easy_setopt_fkt( tmp -> curl_handle, CURLOPT_WRITEDATA, (void *)tmp );
                            if ( rcc != CURLE_OK )
                            {
                                KNSManagerRelease( tmp -> kns_mgr );
                                free( tmp );
                                rc = RC ( rcApp, rcFunction, rcConstructing, rcParam, rcNull );
                            }
                            else
                                *fetcher = & tmp -> dad;
                        }
                    }
                }
            }
        }
        else
            rc = RC( rcApp, rcFunction, rcListing, rcParam, rcNull );
    }
    return rc;
}
