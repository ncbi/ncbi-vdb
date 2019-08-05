#ifndef _h_cloud_manager_
#define _h_cloud_manager_

/*==============================================================================
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
* =========================================================================== */


#ifndef _h_cloud_extern_
#include <cloud/extern.h>
#endif

#ifndef _h_cloud_cloud_
#include <cloud/cloud.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* forwards
 */
struct KConfig;
struct KNSManager;

/* CloudProvider
 */
typedef uint32_t CloudProviderId;
enum
{
    cloud_provider_none,        /* not within any discernible cloud provider */
    cloud_provider_aws,         /* Amazon Web Services                       */
    cloud_provider_gcp,         /* Google Cloud Platform                     */
    cloud_provider_azure,       /* Microsoft Azure Cloud Computing Platform  */

    cloud_num_providers
};

CLOUD_EXTERN
const char * CC CloudProviderAsString(CloudProviderId cloud_provider);

/* CloudMgr
 *  singleton object to access cloud-related resources
 */
typedef struct CloudMgr CloudMgr;

/* Make
*  Discovers the cloud provider
 */
CLOUD_EXTERN rc_t CC CloudMgrMake ( CloudMgr ** mgr,
    struct KConfig const * kfg, struct KNSManager const * kns );

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
CLOUD_EXTERN rc_t CC CloudMgrMakeCloud ( CloudMgr * self, Cloud ** cloud, CloudProviderId cloud_provider );
CLOUD_EXTERN rc_t CC CloudMgrGetCurrentCloud ( const CloudMgr * self, Cloud ** cloud );

#ifdef __cplusplus
}
#endif

#endif /* _h_cloud_manager_ */
