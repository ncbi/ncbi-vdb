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

struct GCP;
#define CLOUD_IMPL struct GCP

#include <cloud/extern.h>
#include <cloud/impl.h>
#include <cloud/gcp.h>

#include <klib/rc.h>
#include <klib/status.h>
#include <klib/text.h>
#include <kns/http.h>

#include <assert.h>

/*--------------------------------------------------------------------------
 * GCP
 */
struct GCP
{
    Cloud dad;
};

/* Destroy
 */
static
rc_t CC GCPDestroy ( GCP * self )
{
}

/* MakeComputeEnvironmentToken
 *  contact cloud provider to get proof of execution environment in form of a token
 */
static
rc_t CC GCPMakeComputeEnvironmentToken ( const GCP * self, const String ** ce_token )
{
}

/* AddComputeEnvironmentTokenForSigner
 *  prepare a request object with a compute environment token
 *  for use by an SDL-associated "signer" service
 */
static
rc_t CC GCPAddComputeEnvironmentTokenForSigner ( const GCP * self, KClientHttpRequest * req )
{
}

/* AddUserPaysCredentials
 *  prepare a request object with credentials for user-pays
 */
static
rc_t CC GCPAddUserPaysCredentials ( const GCP * self, KClientHttpRequest * req )
{
}

static Cloud_vt_v1 GCP_vt_v1 =
{
    1, 0,

    GCPDestroy,
    GCPMakeComputeEnvironmentToken,
    GCPAddComputeEnvironmentTokenForSigner,
    GCPAddUserPaysCredentials
};

/* MakeGCP
 *  make an instance of an GCP cloud interface
 */
LIB_EXPORT rc_t CC CloudMgrMakeGCP ( const CloudMgr * self, GCP ** gcp )
{
}

/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC GCPAddRef ( const GCP * self )
{
    return CloudAddRef ( & self -> dad );
}

LIB_EXPORT rc_t CC GCPRelease ( const GCP * self )
{
    return CloudRelease ( & self -> dad );
}

/* Cast
 *  cast from a Cloud to a GCP type or vice versa
 *  allows us to apply cloud-specific interface to cloud object
 *
 *  returns a new reference, meaning the "self" must still be released
 */
LIB_EXPORT rc_t CC GCPToCloud ( const GCP * cself, Cloud ** cloud )
{
    rc_t rc;
    GCP * self = ( GCP * ) cself;

    if ( cloud == NULL )
        rc = RC ( rcCloud, rcProvider, rcCasting, rcParam, rcNull );
    else
    {
        rc = CloudAddRef ( & self -> dad );
        if ( rc == 0 )
        {
            * cloud = & self -> dad;
            return 0;
        }

        * cloud = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC CloudToGCP ( const Cloud * self, GCP ** gcp )
{
    rc_t rc;

    if ( gcp == NULL )
        rc = RC ( rcCloud, rcProvider, rcCasting, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = 0;
        else if ( self -> vt != ( const Cloud_vt * ) & GCP_vt_v1 )
            rc = RC ( rcCloud, rcProvider, rcCasting, rcType, rcIncorrect );
        else
        {
            rc = CloudAddRef ( self );
            if ( rc == 0 )
            {
                * gcp = ( GCP * ) self;
                return 0;
            }
        }

        * gcp = NULL;
    }

    return rc;
}
