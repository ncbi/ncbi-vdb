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

#pragma once

#ifndef _h_cloud_cloud_priv_
#include <cloud/cloud-priv.h>
#endif

#ifndef _h_cloud_impl_
#include <cloud/impl.h>
#endif

#ifndef _h_kfg_config_
#include <kfg/config.h>
#endif

#ifndef _h_cloud_aws_
#include <cloud/aws.h>
#endif

#ifndef _h_cloud_gcp_
#include <cloud/gcp.h>
#endif

#ifdef __cplusplus
extern "C" {}
#endif


/*--------------------------------------------------------------------------
 * CloudMgr
 */
struct CloudMgr
{
    /* attached from "make" */
    const KConfig * kfg;
#ifdef _h_cloud_aws_
    /* cached AWS */
    AWS * aws;
#endif
#ifdef _h_cloud_gcp_
    /* cached GCP */    
    GCP * gcp;
#endif
#ifdef _h_cloud_azure_
    /* cached AZURE */
    AZURE * azure;
#endif
    /* pointer ( not reference ) to a cached Cloud above */
    Cloud * cur;

    /* object is reference-counted */
    KRefcount refcount;

    /* id of "cur" */
    CloudProviderId cur_id;
};

/*--------------------------------------------------------------------------
 * AWS
 */
#ifdef _h_cloud_aws_
struct AWS
{
    Cloud dad;

    char * profile;

    char * access_key_id;
    char * secret_access_key;

    char * region;
    char * output;
};

/* WithinComputeEnvironment
 *  answers true if within AWS
 */
bool AWSWithinComputeEnvironment ( void );

#endif

/*--------------------------------------------------------------------------
 * GCP
 */
#ifdef _h_cloud_gcp_
struct GCP
{
    Cloud dad;

    char * privateKey;
    char * client_email;
};

/* WithinComputeEnvironment
 *  answers true if within GCP
 */
bool GCPWithinComputeEnvironment ( void );

#endif

/*--------------------------------------------------------------------------
 * AZURE
 */
#ifdef _h_cloud_azure_
struct AZURE
{
    Cloud dad;
};

/* WithinComputeEnvironment
 *  answers true if within Azure
 */
bool AZUREWithinComputeEnvironment ( void );

#endif

#ifdef __cplusplus
}
#endif
