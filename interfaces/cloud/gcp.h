#ifndef _h_cloud_gcp_
#define _h_cloud_gcp_

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

#ifndef _h_cloud_cloud_
#include <cloud/cloud.h>
#endif

#ifndef _h_cloud_manager_
#include <cloud/manager.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* GCP
 *  Google Cloud Platform
 */
typedef struct GCP GCP;

/* MakeGCP
 *  make an instance of a GCP cloud interface
 */
CLOUD_EXTERN rc_t CC CloudMgrMakeGCP ( const CloudMgr * self, GCP ** gcp );

/* AddRef
 * Release
 */
CLOUD_EXTERN rc_t CC GCPAddRef ( const GCP * self );
CLOUD_EXTERN rc_t CC GCPRelease ( const GCP * self );

/* Cast
 *  cast from a Cloud to a GCP type or vice versa
 *  allows us to apply cloud-specific interface to cloud object
 *
 *  returns a new reference, meaning the "self" must still be released
 */
CLOUD_EXTERN rc_t CC GCPToCloud ( const GCP * self, Cloud ** cloud );
CLOUD_EXTERN rc_t CC CloudToGCP ( const Cloud * self, GCP ** gcp );


#ifdef __cplusplus
}
#endif

#endif /* _h_cloud_gcp_ */
