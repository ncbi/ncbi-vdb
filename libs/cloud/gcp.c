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
#include <klib/json.h>
#include <kns/endpoint.h>
#include <kns/socket.h>
#include <kns/http.h>
#include <kfs/directory.h>
#include <kfs/file.h>

#include <assert.h>

#include "cloud-priv.h"

static rc_t PopulateCredentials ( GCP * self );

/* Destroy
 */
static
rc_t CC GCPDestroy ( GCP * self )
{
    free ( self ->privateKey );
    free ( self -> client_email );
    return CloudWhack ( & self -> dad );
}

/* MakeComputeEnvironmentToken
 *  contact cloud provider to get proof of execution environment in form of a token
 */
static
rc_t CC GCPMakeComputeEnvironmentToken ( const GCP * self, const String ** ce_token )
{
    return 0; //TODO
}

/* AddComputeEnvironmentTokenForSigner
 *  prepare a request object with a compute environment token
 *  for use by an SDL-associated "signer" service
 */
static
rc_t CC GCPAddComputeEnvironmentTokenForSigner ( const GCP * self, KClientHttpRequest * req )
{
    return 0; //TODO
}

/* AddAuthentication
 *  prepare a request object with credentials for authentication
 */
static
rc_t CC GCPAddAuthentication ( const GCP * self, KClientHttpRequest * req, const char * http_method )
{
    return 0; //TODO
}

/* AddUserPaysCredentials
 *  prepare a request object with credentials for user-pays
 */
static
rc_t CC GCPAddUserPaysCredentials ( const GCP * self, KClientHttpRequest * req, const char * http_method )
{
    return 0; //TODO
}

static Cloud_vt_v1 GCP_vt_v1 =
{
    1, 0,

    GCPDestroy,
    GCPMakeComputeEnvironmentToken,
    GCPAddComputeEnvironmentTokenForSigner,
    GCPAddAuthentication,
    GCPAddUserPaysCredentials
};

/* MakeGCP
 *  make an instance of an GCP cloud interface
 */
LIB_EXPORT rc_t CC CloudMgrMakeGCP ( const CloudMgr * self, GCP ** p_gcp )
{
    rc_t rc;
//TODO: check self, gcp
    GCP * gcp = calloc ( 1, sizeof * gcp );
    if ( gcp == NULL )
    {
        rc = RC ( rcNS, rcMgr, rcAllocating, rcMemory, rcExhausted );
    }
    else
    {
        /* capture from self->kfg */
        bool user_agrees_to_pay = false;
        
        rc = CloudInit ( & gcp -> dad, ( const Cloud_vt * ) & GCP_vt_v1, "GCP", self -> kns, user_agrees_to_pay );
        if ( rc == 0 )
        {
            rc = PopulateCredentials( gcp );
            if ( rc == 0 )
            {
                * p_gcp = gcp;
            }
            else
            {
                CloudRelease( & gcp -> dad );
            }
        }
        else
        {
            free ( gcp );
        }

    }

    return rc;
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

    if ( self == NULL )
        rc = RC ( rcCloud, rcProvider, rcCasting, rcSelf, rcNull );
    else if ( cloud == NULL )
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

    if ( self == NULL )
        rc = RC ( rcCloud, rcProvider, rcCasting, rcSelf, rcNull );
    else if ( gcp == NULL )
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

/* WithinGCP
 *  answers true if within GCP
 */
bool CloudMgrWithinGCP ( CloudMgr * self )
{
    rc_t rc;
    KEndPoint ep;
    String hostname;

    /* describe address "metadata.google.internal" on port 80 */
    CONST_STRING ( & hostname, "metadata.google.internal" );
    rc = KNSManagerInitDNSEndpoint ( self -> kns, & ep, & hostname, 80 );
    if ( rc == 0 )
    {
        KSocket * conn;
        
        /* we already have a good idea that the environment looks like GCP */
        rc = KNSManagerMakeTimedConnection ( self -> kns, & conn, 0, 0, NULL, & ep );
        if ( rc == 0 )
        {
            /* TBD - is there any sense in finishing the HTTP transaction?
               somebody answered our call, so it looks like they're there,
               if we use the URL to verify a little more, it will confirm... something.
               But we're not prepared to retain any information, unless it's region */
            KSocketRelease ( conn );
            return true;
        }
    }

    return false;
}

static
rc_t PopulateCredentials ( GCP * self )
{
    rc_t rc = 0;

    const char *pathToJsonFile = getenv ( "GOOGLE_APPLICATION_CREDENTIALS" );
    if ( pathToJsonFile == NULL || *pathToJsonFile == 0 ) return 0;

    KDirectory *dir = NULL;
    rc = KDirectoryNativeDir ( &dir );
    if ( rc ) return rc;

    const KFile *cred_file = NULL;
    rc = KDirectoryOpenFileRead ( dir, &cred_file, "%s", pathToJsonFile );
    if ( rc ) return rc;
    uint64_t json_size = 0;
    rc = KFileSize ( cred_file, &json_size );
    if ( rc ) {
        KFileRelease ( cred_file );
        return rc;
    }

    char *buffer = (char *)calloc ( json_size + 1, 1 );

    rc = KFileReadExactly ( cred_file, 0, buffer, json_size );
    if ( rc ) {
        free ( buffer );
        KFileRelease ( cred_file );
        return rc;
    }
    KFileRelease ( cred_file );
    KDirectoryRelease ( dir );

    KJsonValue *root = NULL;
    rc = KJsonValueMake ( &root, buffer, NULL, 0 );
    if ( rc ) {
        free ( buffer );
        return rc;
    }

    free ( buffer );

    const KJsonObject *obj = KJsonValueToObject ( root );

    const char *required[] = {"type", "project_id", "private_key_id",
        "private_key", "client_email", "client_id", "auth_uri", "token_uri",
        "auth_provider_x509_cert_url", "client_x509_cert_url", NULL};

    size_t i = 0;
    while ( required[i] != NULL ) {
        const KJsonValue *v = KJsonObjectGetMember ( obj, required[i] );
        if ( v == NULL ) {
            return RC ( rcKFG, rcFile, rcParsing, rcParam, rcInvalid );
        }
        if ( KJsonGetValueType ( v ) != jsString ) {
            return RC ( rcKFG, rcFile, rcParsing, rcParam, rcInvalid );
        }
        ++i;
    }

    const KJsonValue *v = NULL;
    const char *val = NULL;

    String private_key;
    CONST_STRING ( &private_key, "private_key" );
    v = KJsonObjectGetMember ( obj, "private_key" );
    if ( v == NULL ) {
        return RC ( rcKFG, rcFile, rcParsing, rcParam, rcInvalid );
    }
    if ( KJsonGetValueType ( v ) != jsString ) {
        return RC ( rcKFG, rcFile, rcParsing, rcParam, rcInvalid );
    }
    rc = KJsonGetString ( v, &val );
    if ( rc ) return rc;

    self -> privateKey = string_dup( val, string_size( val ) );

    String client_email;
    CONST_STRING ( &client_email, "client_email" );
    v = KJsonObjectGetMember ( obj, "client_email" );
    if ( v == NULL ) {
        return RC ( rcKFG, rcFile, rcParsing, rcParam, rcInvalid );
    }
    if ( KJsonGetValueType ( v ) != jsString ) {
        return RC ( rcKFG, rcFile, rcParsing, rcParam, rcInvalid );
    }

    rc = KJsonGetString ( v, &val );
    if ( rc ) return rc;

    self -> client_email = string_dup( val, string_size( val ) );

    KJsonValueWhack ( root );

    return 0;
}
