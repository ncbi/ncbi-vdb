#ifndef _h_kfggcp_credentials_
#define _h_kfggcp_credentials_

/*============================================================================
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
 * ========================================================================= */

#include <kfg/extern.h>
#include <kfs/file.h>
#include <klib/defs.h>
#include <klib/json.h>

#ifdef __cplusplus
extern "C" {
#endif


/* LoadAWSCredentials
 * Loads AWS credentials from ~/.aws or environment
 *
 * profile [ IN ] which AWS profile to use
 * aws_access_key_id [ OUT ] - where to place access_key, caller frees
 * aws_secret_access_key [ OUT ] - where to place secret_access_key, caller
 * frees
 */


KFG_EXTERN rc_t CC LoadAwsCredentialsFromFile ( const char *profile,
    char **aws_access_key_id, char **aws_secret_access_key );

KFG_EXTERN rc_t CC LoadAwsCredentialsFromEnv (
    char **aws_access_key_id, char **aws_secret_access_key );

#ifdef __cplusplus
}
#endif

/*************************************************************************************/

#endif /* _h_kfg_cloud_ */
