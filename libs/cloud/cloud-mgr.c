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

#include <cloud/extern.h>
#include <kfc/refcount-impl.h>

#include <klib/rc.h>
#include <klib/status.h>
#include <cloud/aws.h>
#include <cloud/gcp.h>

#include "cloud-priv.h"

#include <atomic.h>
#include <assert.h>

#ifndef ALLOW_EXT_CLOUD_ACCESS
#define ALLOW_EXT_CLOUD_ACCESS 1
#endif

#if 0
#include <stdio.h>
#define TRACE( ... )                                              \
    do { fprintf ( stderr, "%s:%d - ", __func__, __LINE__ );      \
         fprintf ( stderr, __VA_ARGS__ );                         \
         fputc ( '\n', stderr ); } while ( 0 )
#else
#define TRACE( ... ) \
    ( ( void ) 0 )
#endif

/*--------------------------------------------------------------------------
 * CloudMgr
 */

static atomic_ptr_t cloud_singleton;

/* Whack
 */
static
rc_t CloudMgrWhack ( CloudMgr * self )
{
    CloudMgr * our_mgr;

    TRACE ( "accessing CloudMgr singleton" );
    our_mgr = atomic_read_ptr ( & cloud_singleton );
    if ( self != our_mgr )
    {
        TRACE ( "releasing current cloud" );
        CloudRelease ( self -> cur );
#ifdef _h_cloud_aws_
        TRACE ( "releasing current AWS cloud" );
        AWSRelease ( self -> aws );
#endif
#ifdef _h_cloud_gcp_
        TRACE ( "releasing current GCP cloud" );
        GCPRelease ( self -> gcp );
#endif
#ifdef _h_cloud_azure_
        TRACE ( "releasing current Azure cloud" );
        AzureRelease ( self -> azure );
#endif
        TRACE ( "releasing KNSManager reference" );
        KNSManagerRelease ( self -> kns );
        TRACE ( "releasing KConfig reference" );
        KConfigRelease ( self -> kfg );
        TRACE ( "freeing CloudMgr" );
        free ( self );
    }
    return 0;
}

LIB_EXPORT const char * CC CloudProviderAsString(CloudProviderId cloud_provider)
{
    switch (cloud_provider) {
    case cloud_provider_none: return "outside of cloud";
    case cloud_provider_aws: return "AWS";
    case cloud_provider_gcp: return "Google";
    case cloud_provider_azure: return "Azure";
    default: assert(0); return "UNEXPECTED";
    }
}

static
CloudProviderId CloudMgrDetermineCurrentCloud ( const CloudMgr * self )
{
#ifdef _h_cloud_gcp_
    TRACE ( "probing operation within GCP" );
    if ( CloudMgrWithinGCP ( self ) )
    {
        TRACE ( "current compute environment is GCP" );
        return cloud_provider_gcp;
    }
#endif

#ifdef _h_cloud_aws_
    TRACE ( "probing operation within AWS" );
    if ( CloudMgrWithinAWS ( self ) )
    {
        TRACE ( "current compute environment is AWS" );
        return cloud_provider_aws;
    }
#endif

#ifdef _h_cloud_azure_
    TRACE ( "probing operation within Azure" );
    if ( CloudMgrWithinAzure ( self ) )
    {
        TRACE ( "current compute environment is Azure" );
        return cloud_provider_azure;
    }
#endif
    
    TRACE ( "no cloud compute environment detected" );
    return cloud_provider_none;
}

static
rc_t CloudMgrInit ( CloudMgr ** mgrp, const KConfig * kfg,
    const KNSManager * kns, CloudProviderId provider )
{
    rc_t rc = 0;

    CloudMgr * our_mgr;

    TRACE ( "allocating CloudMgr object" );
    our_mgr = calloc ( 1, sizeof * our_mgr );
    if ( our_mgr == NULL )
    {
        TRACE ( "failed to allocate %zu bytes", sizeof * our_mgr );
        rc = RC ( rcCloud, rcMgr, rcAllocating, rcMemory, rcExhausted );
    }
    else
    {
        /* convert allocation into a ref-counted object */
        TRACE ( "initializing KRefcount to 1" );
        KRefcountInit ( & our_mgr -> refcount, 1, "CloudMgr", "init", "cloud" );

        if ( kfg == NULL )
        {
            /* make KConfig if it was not provided */
            TRACE ( "making KConfig" );
            rc = KConfigMake ( ( KConfig ** ) & kfg, NULL );
        }
        else
        {
            /* attach reference to KConfig */
            TRACE ( "attaching to KConfig" );
            rc = KConfigAddRef ( kfg );
        }

        if ( rc == 0 )
        {
            TRACE ( "storing reference to KConfig" );
            our_mgr -> kfg = kfg;

            /* attach reference to KNSManager */
            if ( kns == NULL )
            {
                TRACE ( "making KNSManager" );
                rc = KNSManagerMake ( ( KNSManager ** ) & kns );
            }
            else
            {
                TRACE ( "attaching to KNSManager" );
                rc = KNSManagerAddRef ( kns );
            }

            if ( rc == 0 )
            {
                TRACE ( "storing reference to KNSManager" );
                our_mgr -> kns = kns;

                TRACE ( "initial cloud provider id is 'none'" );
                our_mgr -> cur_id = cloud_provider_none;

                if ( provider == cloud_provider_none )
                {
                    /* examine environment for current cloud */
                    TRACE ( "probing current environment" );
                    provider = CloudMgrDetermineCurrentCloud ( our_mgr );
                }

        if ( provider != cloud_provider_none )
                {
            TRACE ( "making current environment" );
            rc = CloudMgrMakeCloud ( our_mgr, & our_mgr -> cur, provider );
            if ( rc == 0 )
                    {
                TRACE ( "storing current environment" );
                our_mgr -> cur_id = provider;
            }
        }

                if ( rc == 0 )
                {
                    TRACE ( "returning CloudMgr reference" );
                    assert ( mgrp != NULL );
                    * mgrp = our_mgr;
                }
            }
        }
    }

    return rc;
}

/* Make
 *  this is a singleton
 */
LIB_EXPORT rc_t CC CloudMgrMake ( CloudMgr ** mgrp,
    const KConfig * kfg, const KNSManager * kns )
{
    rc_t rc = 0;

    TRACE ( "testing return parameter" );
    if ( mgrp == NULL )
        rc = RC ( rcCloud, rcMgr, rcAllocating, rcParam, rcNull );
    else
    {
        CloudMgr * our_mgr;

        /* grab single-shot singleton */
        TRACE ( "testing for existing CloudMgr" );
        our_mgr = atomic_read_ptr ( & cloud_singleton );
        if ( our_mgr == NULL )
        {
            /* singleton was NULL. call CloudMgrInit to make it from scratch. */
            TRACE ( "creating new CloudMgr" );
            rc = CloudMgrInit ( & our_mgr, kfg, kns, cloud_provider_none );
            if ( rc == 0 )
            {
                CloudMgr * new_mgr;

                /* try to set single-shot ( set once, never reset ) */
                TRACE ( "attempting to set CloudMgr singleton" );
                new_mgr = atomic_test_and_set_ptr ( & cloud_singleton, our_mgr, NULL );

                /* if "new_mgr" is NULL, then our thread won the race */
                if ( new_mgr == NULL )
                {
                    TRACE ( "CloudMgr singleton was set - returning CloudMgr" );
                    * mgrp = our_mgr;
                    return 0;
                }

                /* some other thread beat us to creating the CloudMgr */
                TRACE ( "failed to set CloudMgr singleton" );
                assert ( our_mgr != new_mgr );
                CloudMgrWhack ( our_mgr );
                our_mgr = new_mgr;
            }
        }

        /* add a new reference and return */
        if ( our_mgr != NULL )
            TRACE ( "attaching reference to CloudMgr singleton" );
        rc = CloudMgrAddRef ( our_mgr );
        if ( rc != 0 )
            our_mgr = NULL;

        * mgrp = our_mgr;
    }
    return rc;
}

LIB_EXPORT rc_t CC CloudMgrMakeWithProvider ( CloudMgr ** mgrp, CloudProviderId provider )
{
    rc_t rc;

    TRACE ( "testing return parameter" );
    if ( mgrp == NULL )
        rc = RC ( rcCloud, rcMgr, rcAllocating, rcParam, rcNull );
    else
    {
        CloudMgr * our_mgr;
        TRACE ( "creating new CloudMgr with specific provider" );
        rc = CloudMgrInit ( & our_mgr, NULL, NULL, provider );
        if ( rc == 0 )
        {
            TRACE ( "CloudMgr was created - returning CloudMgr" );
            * mgrp = our_mgr;
            return 0;
        }

        * mgrp = NULL;
    }

    return rc;
}


/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC CloudMgrAddRef ( const CloudMgr * self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "CloudMgr" ) )
        {
        case krefLimit:
            return RC ( rcCloud, rcMgr, rcAttaching, rcRange, rcExcessive );
        case krefNegative:
            return RC ( rcCloud, rcMgr, rcAttaching, rcSelf, rcInvalid );
        default:
            break;
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC CloudMgrRelease ( const CloudMgr * self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "CloudMgr" ) )
        {
        case krefWhack:
            return CloudMgrWhack ( ( CloudMgr * ) self );
        case krefNegative:
            return RC ( rcCloud, rcMgr, rcReleasing, rcRange, rcExcessive );
        default:
            break;
        }
    }

    return 0;
}

/* CurrentProvider
 *  ask whether we are currently executing within a cloud
 */
LIB_EXPORT rc_t CC CloudMgrCurrentProvider ( const CloudMgr * self, CloudProviderId * cloud_provider )
{
    rc_t rc;

    if ( cloud_provider == NULL )
        rc = RC ( rcCloud, rcMgr, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcCloud, rcMgr, rcAccessing, rcSelf, rcNull );
        else
        {
            * cloud_provider = self -> cur_id;
            return 0;
        }

        * cloud_provider = cloud_provider_none;
    }

    return rc;
}

/* MakeCloud
 * MakeCurrentCloud
 */
LIB_EXPORT rc_t CC CloudMgrMakeCloud ( CloudMgr * self, Cloud ** cloud, CloudProviderId cloud_provider )
{
    rc_t rc = 0;

    /* check return parameter */
    TRACE ( "checking return parameter" );
    if ( cloud == NULL )
        rc = RC ( rcCloud, rcMgr, rcAllocating, rcParam, rcNull );
    else
    {
        /* check input parameters */
        TRACE ( "checking input parameters" );
        if ( self == NULL )
            rc = RC ( rcCloud, rcMgr, rcAllocating, rcSelf, rcNull );
        else if ( cloud_provider == cloud_provider_none ||
                  cloud_provider >= cloud_num_providers )
            rc = RC ( rcCloud, rcMgr, rcAllocating, rcParam, rcInvalid );
        else
        {
            /* look for cached Cloud */
            TRACE ( "looking for previously cached Cloud reference" );
            switch ( cloud_provider )
            {
            case cloud_provider_aws:
                if ( self -> aws != NULL )
                {
                    TRACE ( "found previously cached AWS" );
                    return AWSToCloud ( self -> aws, cloud );
                }
                break;
            case cloud_provider_gcp:
                if ( self -> gcp != NULL )
                {
                    TRACE ( "found previously cached GCP" );
                    return GCPToCloud ( self -> gcp, cloud );
                }
                break;
            default:
                TRACE ( "requesting unsupported cloud... will fail later" );
                break;
            }

            /* create a Cloud object via selection matrix:

               where\target | aws | gcp | azure
               -------------+-----+-----+------
                    outside |  x  |  x  |  x
                     in aws |  x  |  +  |  +
                     in gcp |  +  |  x  |  +
                   in azure |  +  |  +  |  x
               -------------+-----+-----+------

               an 'x' indicates that an operation is allowed.
               the matrix was modified with '+' to indicate if
               allwed when ALLOW_EXT_CLOUD_ACCESS is not zero.

               this may be relaxed in the future, but for today
               it's hard coded that from within any cloud there is
               only access to the same cloud allowed. */

            TRACE ( "checking matrix to go from provider id %u to requested %u", self -> cur_id, cloud_provider );
            switch ( cloud_provider * cloud_num_providers + self -> cur_id )
            {
#define CASE( a, b ) \
    case ( a ) * cloud_num_providers + ( b )

#ifdef _h_cloud_aws_
            /* asking for AWS */
            CASE ( cloud_provider_aws, cloud_provider_none ):
            CASE ( cloud_provider_aws, cloud_provider_aws ):
#if defined _h_cloud_gcp_ && ALLOW_EXT_CLOUD_ACCESS
            CASE ( cloud_provider_aws, cloud_provider_gcp ):
#endif
#if defined _h_cloud_azure_ && ALLOW_EXT_CLOUD_ACCESS
            CASE ( cloud_provider_aws, cloud_provider_azure ):
#endif
            {
                assert ( self -> aws == NULL );
                TRACE ( "making AWS" );
                rc = CloudMgrMakeAWS ( self, & self -> aws );
                if ( rc == 0 )
                {
                    TRACE ( "casting AWS to Cloud" );
                    return AWSToCloud ( self -> aws, cloud );
                }
                break;
            }
#endif /* _h_cloud_aws_ */

#ifdef _h_cloud_gcp_
            /* asking for GCP */
            CASE ( cloud_provider_gcp, cloud_provider_none ):
#if defined _h_cloud_aws_ && ALLOW_EXT_CLOUD_ACCESS
            CASE ( cloud_provider_gcp, cloud_provider_aws ):
#endif
            CASE ( cloud_provider_gcp, cloud_provider_gcp ):
#if defined _h_cloud_azure_ && ALLOW_EXT_CLOUD_ACCESS
            CASE ( cloud_provider_gcp, cloud_provider_azure ):
#endif
            {
                assert ( self -> gcp == NULL );
                TRACE ( "making GCP" );
                rc = CloudMgrMakeGCP ( self, & self -> gcp );
                if ( rc == 0 )
                {
                    TRACE ( "casting GCP to Cloud" );
                    return GCPToCloud ( self -> gcp, cloud );
                }
                break;
            }
#endif /* _h_cloud_gcp */

#ifdef _h_cloud_azure_
            /* asking for Azure */
            CASE ( cloud_provider_azure, cloud_provider_none ):
#if defined _h_cloud_aws_ && ALLOW_EXT_CLOUD_ACCESS
            CASE ( cloud_provider_azure, cloud_provider_aws ):
#endif
#if defined _h_cloud_gcp_ && ALLOW_EXT_CLOUD_ACCESS
            CASE ( cloud_provider_azure, cloud_provider_gcp ):
#endif
            CASE ( cloud_provider_azure, cloud_provider_azure ):
            {
#error "not implemented"
            }
#endif /* _h_cloud_gcp */

            default:
                rc = RC ( rcCloud, rcMgr, rcAllocating, rcCloudProvider, rcUnsupported );

#undef CASE
            }
        }

        * cloud = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC CloudMgrGetCurrentCloud ( const CloudMgr * self, Cloud ** cloud )
{
    rc_t rc;

    if ( cloud == NULL )
        rc = RC ( rcCloud, rcMgr, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcCloud, rcMgr, rcAccessing, rcSelf, rcNull );
        else if ( self -> cur_id == cloud_provider_none )
            rc = RC ( rcCloud, rcMgr, rcAccessing, rcCloudProvider, rcNotFound );
        else
        {
            rc = CloudAddRef ( self -> cur );
            if ( rc == 0 )
            {
                * cloud = self -> cur;
                return 0;
            }
        }

        * cloud = NULL;
    }
    return rc;
}

/* Force a provider for testing
 */
void CC CloudMgrSetProvider ( CloudMgr * self, CloudProviderId provider )
{
#if _DEBUGGING
    if ( provider < cloud_num_providers )
    {
        if ( self != NULL && self -> cur_id != provider )
        {
            CloudRelease ( self -> cur );
            self -> cur = NULL;
            self -> cur_id = cloud_provider_none;

            if ( provider != cloud_provider_none )
            {
                CloudMgrMakeCloud ( self, & self -> cur, provider );
                self -> cur_id = provider;
            }
        }
    }
#endif
}

#if 0
#define GS "http://metadata.google.internal/computeMetadata/v1/instance/zone"
#define S3 "http://169.254.169.254/latest/meta-data/placement/availability-zone"

static
rc_t
_KNSManager_Read( struct KNSManager * self, bool gs, char * location, size_t locationSize )
{
    rc_t rc = 0;

    const char * url = gs ? GS : S3;

    KClientHttpRequest *req = NULL;

    /* save existing timeouts */
    int32_t cmsec = self -> conn_timeout;
    int32_t wmsec = self -> http_write_timeout;

    int32_t timeout = 1; /* milliseconds */

    /* minimize timeouts to check cloudy URLs */
    self->conn_timeout = self->http_write_timeout = timeout;

    rc = KNSManagerMakeRequest(self, &req, 0x01010000, NULL, url);

    if (rc == 0) {
        if (gs)
            rc = KClientHttpRequestAddHeader(req, "Metadata-Flavor", "Google");

        if (rc == 0) {
            KClientHttpResult * rslt = NULL;
            rc = KClientHttpRequestGET(req, &rslt);

            /* restore timeouts in KNSManager; may be not needed here */
            self->conn_timeout = cmsec;
            self->http_write_timeout=wmsec;

            if (rc == 0) {
                KStream * s = NULL;
                rc = KClientHttpResultGetInputStream(rslt, &s);
                if (rc == 0) {
                    size_t num_read = 0;
                    rc = KStreamRead(s, location, locationSize, &num_read);
                    if (rc == 0)
                    {
                        if ( num_read == locationSize )
                            --num_read;
                        buffer[num_read++] = '\0';
                    }
                }
                RELEASE(KStream, s);
            }
            RELEASE(KClientHttpResult, rslt);
        }
    }

    RELEASE(KClientHttpRequest, req);

    /* restore timeouts in KNSManager */
    self->conn_timeout = cmsec;
    self->http_write_timeout = wmsec;

    return rc;
}

static
rc_t CloudMgrSetProvider ( CloudMgr * self )
{
    KDirectory * dir;
    rc_t rc = KDirectoryNativeDir(&dir);
    if (rc == 0)
    {
        KNSManager * kns; //TODO: init
        bool gcsFirst = true;
        char location[99] = "";

        bool log = KNSManagerLogNcbiVdbNetError(kns);

        if (_KDirectory_FileExists(dir, "/usr/bin/gcloud"))
        {
            gcsFirst = true;
        }
        else if (_KDirectory_FileExists(dir, "/usr/bin/ec2-metadata"))
        {
            gcsFirst = false;
        }

        RELEASE(KDirectory, dir);

        if (log)
            KNSManagerSetLogNcbiVdbNetError(kns, false);

        rc = _KNSManager_Read ( kns, gcsFirst, location, sizeof location );
        if ( rc == 0 )
        {
            self -> cur = gcsFirst ? cloud_provider_gcp : cloud_provider_aws;
            //TODO: copy location to self
        }
        else
        {
            rc = _KNSManager_Read ( kns, ! gcsFirst, location, sizeof location );
            if ( rc == 0 )
            {
                self -> cur = gcsFirst ? cloud_provider_aws : cloud_provider_gcp;
                //TODO: copy location to self
            }
            else
            {
                rc = 0;
                self -> cur = cloud_provider_none;
            }
        }

        if (log)
            KNSManagerSetLogNcbiVdbNetError(kns, true);
    }

    return rc;
}
#endif
