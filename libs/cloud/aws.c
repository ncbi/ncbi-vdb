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

struct AWS;
#define CLOUD_IMPL struct AWS

#include <cloud/extern.h>
#include <cloud/impl.h>
#include <cloud/aws.h>

#include <klib/rc.h>
#include <klib/status.h>
#include <klib/text.h>
#include <kns/http.h>

#include <assert.h>

/*--------------------------------------------------------------------------
 * AWS
 */
struct AWS
{
    Cloud dad;
};

/* Destroy
 */
static
rc_t CC AWSDestroy ( AWS * self )
{
}

/* MakeComputeEnvironmentToken
 *  contact cloud provider to get proof of execution environment in form of a token
 */
static
rc_t CC AWSMakeComputeEnvironmentToken ( const AWS * self, const String ** ce_token )
{
}

/* AddComputeEnvironmentTokenForSigner
 *  prepare a request object with a compute environment token
 *  for use by an SDL-associated "signer" service
 */
static
rc_t CC AWSAddComputeEnvironmentTokenForSigner ( const AWS * self, KClientHttpRequest * req )
{
}

/* AddUserPaysCredentials
 *  prepare a request object with credentials for user-pays
 */
static
rc_t CC AWSAddUserPaysCredentials ( const AWS * self, KClientHttpRequest * req )
{
}

static Cloud_vt_v1 AWS_vt_v1 =
{
    1, 0,

    AWSDestroy,
    AWSMakeComputeEnvironmentToken,
    AWSAddComputeEnvironmentTokenForSigner,
    AWSAddUserPaysCredentials
};

/* MakeAWS
 *  make an instance of an AWS cloud interface
 */
LIB_EXPORT rc_t CC CloudMgrMakeAWS ( const CloudMgr * self, AWS ** aws )
{
}

/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC AWSAddRef ( const AWS * self )
{
    return CloudAddRef ( & self -> dad );
}

LIB_EXPORT rc_t CC AWSRelease ( const AWS * self )
{
    return CloudRelease ( & self -> dad );
}

/* Cast
 *  cast from a Cloud to an AWS type or vice versa
 *  allows us to apply cloud-specific interface to cloud object
 *
 *  returns a new reference, meaning the "self" must still be released
 */
LIB_EXPORT rc_t CC AWSToCloud ( const AWS * cself, Cloud ** cloud )
{
    rc_t rc;
    AWS * self = ( AWS * ) cself;

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

LIB_EXPORT rc_t CC CloudToAWS ( const Cloud * self, AWS ** aws )
{
    rc_t rc;

    if ( aws == NULL )
        rc = RC ( rcCloud, rcProvider, rcCasting, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = 0;
        else if ( self -> vt != ( const Cloud_vt * ) & AWS_vt_v1 )
            rc = RC ( rcCloud, rcProvider, rcCasting, rcType, rcIncorrect );
        else
        {
            rc = CloudAddRef ( self );
            if ( rc == 0 )
            {
                * aws = ( AWS * ) self;
                return 0;
            }
        }

        * aws = NULL;
    }

    return rc;
}
