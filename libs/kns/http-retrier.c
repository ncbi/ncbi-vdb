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

#define KFILE_IMPL KHttpFile
typedef struct KHttpFile KHttpFile;
#include <kfs/impl.h>

#include <kns/manager.h>

#include <kfg/config.h>

#include <klib/log.h>
#include <klib/time.h>
#include <klib/rc.h>

#include <sysalloc.h>
#include <string.h>
#include <ctype.h>
#include <os-native.h>

#include "mgr-priv.h"

struct HttpRetrySchedule
{
    uint16_t    code; // 5 for 5xx, 4 for 4xx, otherwise http status code from 400 to 599
    uint8_t     max_retries;
    bool        open_ended; /* true = retry with the last sleep time until global retry limit or max sleep time is reached */
    
    uint16_t    sleep_before_retry[1]; // actual size determined at construction
    /*do not define anything here*/
};
typedef struct HttpRetrySchedule HttpRetrySchedule;

static
void AddSleepTime ( uint16_t * retries, uint16_t idx, uint16_t value )
{   /* make sure new value is not less than the previous */
    if ( idx == 0 || value >= retries [ idx - 1 ] )
    {
        retries [ idx ] = value;
    }
    else
    {
        retries [ idx ] = retries [ idx - 1 ];
    }
}

static
rc_t HttpRetryCodesConfig( HttpRetrySchedule** self, uint16_t code, const String* retries )
{   /* extract body, count commas, make sure all other characters are decimals */

    /* make sure only decimals and commas are used, count commas */
    String s = *retries; 
    uint16_t retryCount = 0;
    uint32_t ch;
    bool in_number = false;
    while ( StringPopHead ( &s, &ch ) == 0 && ch != 0 )
    {
        if ( ch == ',' )
        {   
            in_number = false;
        }
        else if ( ch == '+' )
        {   /* + is only allowed immediately after the last number */
            if ( ( s.len == 0 || s.addr [ 0 ] == 0 ) 
                 && 
                 in_number )
            {
                break;
            }
            else
            {
                return RC ( rcNS, rcData, rcCreating, rcNumeral, rcInvalid );
            }
        }
        else if ( isdigit ( ch ) )
        {
            if ( ! in_number ) 
            {   /* first digit of a new number */
                ++retryCount;
            }
            in_number = true;
        }
        else
        {
            return RC ( rcNS, rcData, rcCreating, rcNumeral, rcInvalid );
        }
    }
    
    *self = (HttpRetrySchedule*) malloc( sizeof ( HttpRetrySchedule ) + sizeof ( uint16_t ) * ( retryCount - 1 ) ); 
    if ( *self == NULL )
    {
        return RC ( rcNS, rcData, rcCreating, rcMemory, rcExhausted );
    }
    (*self) -> code = code;
    (*self) -> max_retries = retryCount;
    (*self) -> open_ended = false;
    
    {   /* populate sleep_before_retry and open_ended */
        uint16_t cur_value = 0;
        uint16_t curRetry = 0;
        s = *retries; 
        in_number = false;
        while ( StringPopHead ( &s, &ch ) == 0 && ch != 0 )
        {
            if ( ch == ',' )
            {   
                if ( in_number )
                {
                    AddSleepTime ( (*self) -> sleep_before_retry, curRetry, cur_value );
                    ++ curRetry;
                }
                in_number = false;
            }
            else if ( ch == '+' )
            {
                (*self) -> open_ended = true;
                AddSleepTime ( (*self) -> sleep_before_retry, curRetry, cur_value );
                return 0;
            }
            else if ( isdigit ( ch ) )
            {
                if ( ! in_number ) 
                {   /* first digit of a new number */
                    cur_value = ch - '0';
                }
                else
                {
                    cur_value = cur_value * 10 + ch - '0';
                }
                in_number = true;
            }
            else 
            {
                return RC ( rcNS, rcData, rcCreating, rcNumeral, rcInvalid );
            }
        }
        if ( in_number ) 
        {
            AddSleepTime ( (*self) -> sleep_before_retry, curRetry, cur_value );
        }
    }    
    return 0;
}

static
rc_t HttpRetryCodesDefault ( HttpRetrySchedule** self, uint16_t code )
{
    String def;
    CONST_STRING(&def, "0,5,10,15,30,60");
    return HttpRetryCodesConfig ( self, code, &def );
}

static
rc_t HttpRetrySpecsConfigNode ( const KConfigNode* node, const char* name, HttpRetrySchedule** self, uint16_t code )
{
    rc_t rc;
    rc_t rc2;
    const KConfigNode *child;
    rc = KConfigNodeOpenNodeRead ( node, &child, "%s", name );
    if ( rc == 0 )
    {
        String * value;
        rc = KConfigNodeReadString ( child, &value );
        if ( rc == 0 )
        {
            rc = HttpRetryCodesConfig( self, code, value);
            free(value);
        }
        rc2 = KConfigNodeRelease ( child );
        if ( rc == 0 )
        {
            rc = rc2;
        }
    }
    return rc;
}                

static
rc_t HttpRetrySpecsFromConfig ( HttpRetrySpecs* self, const KConfigNode* node )
{   /* retrieves only 5xx and 4xx codes */
    uint32_t total = 0;
    bool has4xx = false;
    bool has5xx = false;
    KNamelist * names;
    uint32_t nameCount;
    rc_t rc = KConfigNodeListChildren ( node, & names );
    if ( rc == 0 )
    {   /* count 4xx and 5xx */
        uint32_t i;
        rc = KNamelistCount ( names, & nameCount );
        if ( rc == 0 )
        {
            for ( i = 0; i < nameCount; ++ i )
            {
                const char *name;
                size_t size; 
                rc = KNamelistGet ( names, i, & name );
                if (rc != 0)
                {
                    KNamelistRelease ( names );
                    break;
                }
                size = string_size(name);
                if ( size == 3 )
                {
                    if ( string_cmp ( name, string_size(name), "4xx", 3, 3 ) == 0 )
                    {
                        has4xx = true;
                        ++total;
                    }
                    else if ( string_cmp ( name, string_size(name), "5xx", 3, 3 ) == 0 )
                    {
                        has5xx = true;
                        ++total;
                    }
                    else if ( name[0] == '4' || name[0] == '5' )
                    {
                        ++total;
                    }
                    else
                    { /*LOG a bad name*/
                    }
                }
                else
                {/*LOG a bad name*/
                }
            }
        }
        /* add extra for 4xx and 5xx catch-alls if not specified */
        if ( ! has4xx ) ++total;
        if ( ! has5xx ) ++total;
    }
    
    if (rc == 0)
    {
        rc_t rc2;
        uint32_t i;
        uint32_t cur = 0;
        self -> count = total;
        self -> codes = (HttpRetrySchedule**) calloc ( self -> count, sizeof * self -> codes );
        for ( i = 0; i < nameCount; ++ i )
        {
            const char *name;
            rc = KNamelistGet ( names, i, & name );  
            
            /* convert name to code */
            assert ( string_size(name) == 3 );
            if ( string_cmp ( name, 3, "4xx", 3, 3 ) == 0 )
                rc = HttpRetrySpecsConfigNode ( node, name, & self -> codes [ cur ], 4 );
            else if ( string_cmp ( name, 3, "5xx", 3, 3 ) == 0 )
                rc = HttpRetrySpecsConfigNode ( node, name, & self -> codes [ cur ], 5 );
            else if ( name[0] == '4' || name[0] == '5' )
                rc = HttpRetrySpecsConfigNode ( node, name, & self -> codes [ cur ], ( uint16_t ) strtoul ( name, NULL, 10 ) );
            else            
            { /* invalid code, skip */
                continue;
            }
            
            if ( rc != 0 )
            {
                break;
            }
            ++cur;
        }
        
        rc2 = KNamelistRelease ( names );
        if ( rc == 0 )
        {
            rc = rc2;
        }
        if ( rc == 0 )
        {
            /* if 4xx and/or 5xx were not specified, fill in the defaults */
            if ( ! has4xx ) 
            {
                rc = HttpRetryCodesDefault ( & self -> codes [cur], 4 );
                if ( rc != 0 )
                {
                    free ( self -> codes );
                    return rc;
                }
                ++cur;
            }
            if ( ! has5xx ) 
            {
                rc = HttpRetryCodesDefault ( & self -> codes [cur], 5 );
                if ( rc != 0 )
                {
                    free ( self -> codes );
                    return rc;
                }
            }
        }
    }
    return rc;
}

rc_t CC HttpRetrySpecsDestroy(HttpRetrySpecs* self)
{
    uint8_t i;
    for ( i = 0; i < self -> count; ++i )
    {
        free ( self -> codes [ i ] );
    }
    free ( self -> codes );
    return 0;
}

rc_t CC HttpRetrySpecsInit(HttpRetrySpecs* self, KConfig* kfg)
{
    const KConfigNode* node;
    rc_t rc = KConfigOpenNodeRead ( kfg, &node, "http/reliable" );
    if ( rc == 0 )
    {   
        rc_t rc2;
        rc = HttpRetrySpecsFromConfig ( self, node );
        rc2 = KConfigNodeRelease(node);
        if ( rc == 0 )
            rc = rc2;
    }
    else if ( GetRCState ( rc ) == rcNotFound )
    {   /* set to defaults */
        self -> count = 2;
        self -> codes = (HttpRetrySchedule**) malloc ( ( sizeof * self -> codes ) * self -> count );
        if ( self -> codes  == NULL )
        {
            return RC ( rcNS, rcData, rcCreating, rcMemory, rcExhausted );
        }
        
        rc = HttpRetryCodesDefault ( & self -> codes [0], 4 );
        if ( rc != 0 )
        {
            HttpRetrySpecsDestroy ( self );
            return rc;
        }
        rc = HttpRetryCodesDefault ( & self -> codes [1], 5 );
        if ( rc != 0 )
        {
            HttpRetrySpecsDestroy ( self );
            return rc;
        }
    }   
    
    return rc;
}

bool HttpGetRetryCodes ( const HttpRetrySpecs* self, 
                         uint16_t code, 
                         uint8_t * max_retries, 
                         const uint16_t ** sleep_before_retry, 
                         bool * open_ended )
{
    int8_t catchAllIdx = -1;
    uint8_t i;
    
    assert ( self );
    assert ( max_retries );
    assert ( sleep_before_retry );
    assert ( open_ended );
    
    for ( i = 0; i < self -> count; ++i )
    {
        if ( self -> codes [ i ] -> code == code ) 
        {
            * max_retries = self -> codes [ i ] -> max_retries;
            * sleep_before_retry = self -> codes [ i ] -> sleep_before_retry;
            * open_ended = self -> codes [ i ] -> open_ended;
            return true;
        }
        if ( catchAllIdx == -1 && self -> codes [ i ] -> code == code / 100 ) /* if more than one catch-all, use the first */
        {
            catchAllIdx = i;
        }
    }
    
    if ( catchAllIdx != -1 )
    {   // code not found but there is a common setting for its group (4xx or 5xx)
        * max_retries = self -> codes [ catchAllIdx ] -> max_retries;
        * sleep_before_retry = self -> codes [ catchAllIdx ] -> sleep_before_retry;
        * open_ended = self -> codes [ catchAllIdx ] -> open_ended;
        return true;
    }
    
    return false;
}

rc_t KHttpRetrierInit ( KHttpRetrier * self,    
                        const char * url, 
                        const struct KNSManager * kns )
{
    rc_t rc;
    assert ( self );
    assert ( kns );

    memset ( self, 0, sizeof *self );

    self -> url = url;
    
    rc = KNSManagerAddRef ( kns );
    if ( rc != 0 )
    {
        return rc;
    }
    
    self -> kns = kns;
    self -> max_retries = kns -> maxNumberOfRetriesOnFailureForReliableURLs;
    self -> max_total_wait_ms = kns -> maxTotalWaitForReliableURLs_ms;
    
    return 0;
}

rc_t KHttpRetrierDestroy ( KHttpRetrier * self )
{
    return KNSManagerRelease ( self -> kns );
}

bool KHttpRetrierWait ( KHttpRetrier * self, uint32_t status )
{
    assert ( self );
    
    if ( self -> last_status != status )
    {
        self -> last_status = status;
        self -> retries_count = 0;
        self -> total_wait_ms = 0;
    }
    
    if ( self -> retries_count >= self -> max_retries || self -> total_wait_ms >= self -> max_total_wait_ms )
    {
        return false;
    }
    else
    {
        uint32_t to_sleep;
        uint8_t max_retries;
        const uint16_t * sleep_before_retry;
        bool open_ended;
        if ( ! HttpGetRetryCodes ( & self -> kns -> retry_specs, status, & max_retries, & sleep_before_retry, & open_ended ) )
        {
            return false;
        }
        
        if ( self -> retries_count >= max_retries )
        {   
            if ( open_ended )
            {
                to_sleep = sleep_before_retry [ max_retries - 1 ];
            }
            else
            {
                return false;
            }
        }
        else
        {
            to_sleep = sleep_before_retry [ self -> retries_count ];
        }
        
        if ( self -> total_wait_ms + to_sleep > self -> max_total_wait_ms )
        {
            to_sleep = self -> max_total_wait_ms - self -> total_wait_ms;
        }
        self -> last_sleep = to_sleep;
        KSleepMs( to_sleep );
        self -> total_wait_ms += to_sleep;
        ++ self -> retries_count;
        
        PLOGMSG (klogInfo, ( klogInfo, "HTTP read failure: URL=\"$(u)\" status=$(s); tried $(c)/$(m) times for $(t) milliseconds total",
                            "u=%s,s=%d,c=%d,m=%d,t=%d", 
                            self -> url, 
                            status,
                            self -> retries_count,
                            open_ended ? self -> max_retries : max_retries, 
                            self -> total_wait_ms ) );        
        
        return true;
    }
}

