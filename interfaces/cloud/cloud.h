#ifndef _h_cloud_cloud_
#define _h_cloud_cloud_

/*=====================================================================================
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
* ================================================================================== */


#ifndef _h_cloud_extern_
#include <cloud/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* forwards
 */
struct String;
struct KClientHttpRequest;

/* CloudProvider
 */
typedef uint32_t CloudProviderId;
enum
{
    cloud_provider_none,
    cloud_provider_aws,
    cloud_provider_gcp,
    cloud_provider_azure,

    cloud_num_providers
};

/* Cloud
 *  generic cloud interface
 */
typedef struct Cloud Cloud;

/* AddRef
 * Release
 */
CLOUD_EXTERN rc_t CC CloudAddRef ( const Cloud * self );
CLOUD_EXTERN rc_t CC CloudRelease ( const Cloud * self );

/* MakeComputeEnvironmentToken
 *  contact cloud provider to get proof of execution environment in form of a token
 */
CLOUD_EXTERN rc_t CC CloudMakeComputeEnvironmentToken ( const Cloud * self, struct String const ** ce_token );

/* AddComputeEnvironmentTokenForSigner
 *  prepare a request object with a compute environment token
 *  for use by an SDL-associated "signer" service
 */
CLOUD_EXTERN rc_t CC CloudAddComputeEnvironmentTokenForSigner ( const Cloud * self, struct KClientHttpRequest * req );

/* AddUserPaysCredentials
 *  prepare a request object with credentials for user-pays
 */
CLOUD_EXTERN rc_t CC CloudAddUserPaysCredentials ( const Cloud * self, struct KClientHttpRequest * req );



/* CloudMgr
 *  singleton object to access cloud-related resources
 */
typedef struct CloudMgr CloudMgr;

/* Make
 */
CLOUD_EXTERN rc_t CC CloudMgrMake ( CloudMgr ** mgr );

/* AddRef
 * Release
 */
CLOUD_EXTERN rc_t CC CloudMgrAddRef ( const CloudMgr * self );
CLOUD_EXTERN rc_t CC CloudMgrRelease ( const CloudMgr * self );

/* CurrentProvider
 *  ask whether we are currently executing within a cloud
 */
CLOUD_EXTERN rc_t CC CloudMgrCurrentProvider ( const CloudMgr * self, CloudProviderId * cloud_provider );

/* MakeCloud
 * MakeCurrentCloud
 */
CLOUD_EXTERN rc_t CC CloudMgrMakeCloud ( const CloudMgr * self, Cloud ** cloud, CloudProviderId cloud_provider );
CLOUD_EXTERN rc_t CC CloudMgrMakeCurrentCloud ( const CloudMgr * self, Cloud ** cloud );

#ifdef __cplusplus
}
#endif

#endif /* _h_cloud_cloud_ */
