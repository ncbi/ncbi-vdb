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

/*--------------------------------------------------------------------------
 * CloudMgr
 */
struct CloudMgr
{
    KRefcount refcount;
    CloudProviderId cur;
};

static atomic_ptr_t cloud_singleton;

/* Whack
 */
static
rc_t CloudMgrWhack ( CloudMgr * self )
{
    CloudMgr * our_mgr = atomic_test_and_set_ptr ( & cloud_singleton, NULL, NULL );
    if ( self == our_mgr )
    {
        atomic_test_and_set_ptr ( & cloud_singleton, NULL, self );
    }

    free ( self );

    return 0;
}

/* Make
 *  this is a singleton
 */
LIB_EXPORT rc_t CC CloudMgrMake ( CloudMgr ** mgrp )
{
    rc_t rc = 0;

    if ( mgrp == NULL )
    {
        rc = RC ( rcCloud, rcMgr, rcAllocating, rcParam, rcNull );
    }
    else
    {   /* build or attach reference to singleton */
        CloudMgr * our_mgr;

        * mgrp = NULL;

        /* grab single-shot singleton */
        our_mgr = atomic_test_and_set_ptr ( & cloud_singleton, NULL, NULL );
        if ( our_mgr != NULL )
        {
            /* add a new reference and return */
            rc = CloudMgrAddRef ( our_mgr );
            if ( rc == 0 )
            {
                * mgrp = our_mgr;
            }
            return rc;
        }
        else
        {   /* singleton was NULL. make from scratch. */
            our_mgr = calloc ( 1, sizeof * our_mgr );
            if ( our_mgr == NULL )
            {
                rc = RC ( rcNS, rcMgr, rcAllocating, rcMemory, rcExhausted );
            }
            else
            {
                KRefcountInit ( & our_mgr -> refcount, 1, "CloudManager", "init", "cloud" );

                /* try to set single-shot ( set once, never reset ) */
                CloudMgr * new_mgr = atomic_test_and_set_ptr ( & cloud_singleton, our_mgr, NULL );
                if ( new_mgr != NULL )
                {
                    /* somebody else got here first - drop our version */
                    assert ( our_mgr != new_mgr );
                    CloudMgrRelease ( our_mgr );

                    /* use the new manager, just add a reference and return */
                    rc = CloudMgrAddRef ( new_mgr );
                    if ( rc == 0 )
                    {
                        * mgrp = new_mgr;
                    }
                }
                else
                {
                    /*TODO: this code will need to discover where we are operating */
                    our_mgr -> cur = cloud_provider_none;

                    * mgrp = our_mgr;
                }
            }
        }
    }

    return rc;
}

/* Force a provider for testing
 */
void CC CloudMgrSetProvider ( CloudMgr * mgr, CloudProviderId provider )
{
    assert ( mgr != NULL );
    mgr -> cur = provider;
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
            * cloud_provider = self -> cur;
            return 0;
        }

        * cloud_provider = cloud_provider_none;
    }

    return rc;
}

/* MakeCloud
 * MakeCurrentCloud
 */
LIB_EXPORT rc_t CC CloudMgrMakeCloud ( const CloudMgr * self, Cloud ** cloud, CloudProviderId cloud_provider )
{
    rc_t rc;

    if ( cloud == NULL )
        rc = RC ( rcCloud, rcMgr, rcAllocating, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcCloud, rcMgr, rcAllocating, rcSelf, rcNull );
        else if ( cloud_provider == cloud_provider_none ||
                  cloud_provider >= cloud_num_providers )
            rc = RC ( rcCloud, rcMgr, rcAllocating, rcParam, rcInvalid );
        else
        {
            switch ( cloud_provider * cloud_num_providers + self -> cur )
            {
#define CASE( a, b ) \
    case ( a ) * cloud_num_providers + ( b )

#ifdef _h_cloud_aws_
                /* asking for AWS */
            CASE ( cloud_provider_aws, cloud_provider_none ):
            CASE ( cloud_provider_aws, cloud_provider_aws ):
            {
                AWS * aws;
                rc = CloudMgrMakeAWS ( self, & aws );
                if ( rc == 0 )
                {
                    rc = AWSToCloud ( aws, cloud );
                    AWSRelease ( aws );
                    if ( rc == 0 )
                        return 0;

                }
                break;
            }
#if ALLOW_EXT_CLOUD_ACCESS
            CASE ( cloud_provider_aws, cloud_provider_gcp ):
            CASE ( cloud_provider_aws, cloud_provider_azure ):
#error "this should require a special class"
#endif
#endif

#ifdef _h_cloud_gcp_
                /* asking for GCP */
            CASE ( cloud_provider_gcp, cloud_provider_none ):
            CASE ( cloud_provider_gcp, cloud_provider_gcp ):
            {
                GCP * gcp;
                rc = CloudMgrMakeGCP ( self, & gcp );
                if ( rc == 0 )
                {
                    rc = GCPToCloud ( gcp, cloud );
                    GCPRelease ( gcp );
                    if ( rc == 0 )
                        return 0;

                }
                break;
            }
#if ALLOW_EXT_CLOUD_ACCESS
            CASE ( cloud_provider_gcp, cloud_provider_aws ):
            CASE ( cloud_provider_gcp, cloud_provider_azure ):
#error "this should require a special class"
#endif
#endif

#ifdef _h_cloud_azure_
                /* asking for Azure */
            CASE ( cloud_provider_azure, cloud_provider_none ):
            CASE ( cloud_provider_azure, cloud_provider_azure ):
#error "not implemented"
#if ALLOW_EXT_CLOUD_ACCESS
            CASE ( cloud_provider_azure, cloud_provider_aws ):
            CASE ( cloud_provider_azure, cloud_provider_gcp ):
#error "this should require a special class"
#endif
#endif

            default:
                ( void ) 0;
#undef CASE
            }
        }

        * cloud = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC CloudMgrMakeCurrentCloud ( const CloudMgr * self, Cloud ** cloud )
{
    rc_t rc;

    if ( cloud == NULL )
        rc = RC ( rcCloud, rcMgr, rcAllocating, rcParam, rcNull );
    else
    {
        CloudProviderId cur;
        rc = CloudMgrCurrentProvider ( self, & cur );
        if ( rc == 0 )
        {
            rc = CloudMgrMakeCloud ( self, cloud, cur );
            if ( rc == 0 )
                return 0;
        }

        * cloud = NULL;
    }
    return rc;
}
