#ifndef _h_cloud_cloud_priv_
#define _h_cloud_cloud_priv_

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

#ifndef _h_cloud_manager_
#include <cloud/manager.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct KStream;

/* CloudMgrMakeWithProvider
 *  Create a non-singleton, object to access cloud-related resources, with a set provider
 */
CLOUD_EXTERN rc_t CC CloudMgrMakeWithProvider ( CloudMgr ** mgrp, CloudProviderId provider, const struct KConfig * kfg );

/* Set a pre-opened HTTP connection, for testing (NULL OK)
 * TODO: remove when mocked connection becomes a regular feature of KNS
 */
CLOUD_EXTERN rc_t CC CloudSetHttpConnection ( Cloud  * cloud, struct KStream * conn );

#ifdef __cplusplus
}
#endif

#endif /* _h_cloud_cloud_priv_ */
