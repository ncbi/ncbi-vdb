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

#include <kfg/properties.h> /* KConfig_Get_User_Accept_Gcp_Charges */

#include <klib/base64.h>
#include <klib/data-buffer.h>
#include <klib/debug.h> /* DBGMSG */
#include <klib/json.h>
#include <klib/printf.h> /* string_printf */
#include <klib/rc.h>
#include <klib/status.h>
#include <klib/strings.h> /* ENV_MAGIC_CE_TOKEN */
#include <klib/text.h>

#include <kns/endpoint.h>
#include <kns/socket.h>
#include <kns/http.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kns/stream.h> /* KStreamRelease */
#include <kns/kns-mgr-priv.h> 
#include <kns/http-priv.h>

#include <kproc/procmgr.h>

#include <ext/mbedtls/md.h> /* vdb_mbedtls_md_hmac */
#include <ext/mbedtls/pk.h>
#include <ext/mbedtls/ctr_drbg.h>
#include <ext/mbedtls/entropy.h>

#include <assert.h>

#include <sys/types.h>

#include "cloud-cmn.h" /* KNSManager_Read */
#include "cloud-priv.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/*TODO: use log.h instead, or promote to cloud-priv.h (there is a copy in cloud-mgr.c) */
#if 0
#include <stdio.h>
#define TRACE( ... )                                              \
    do { fprintf ( stderr, "%s:%d - ", __func__, __LINE__ );      \
         fprintf ( stderr, __VA_ARGS__ );                         \
         fputc ( '\n', stderr ); } while ( 0 )
#else
#define TRACE( ... ) \
    ( ( void ) 0 )
#endif

static rc_t PopulateCredentials(GCP * self);

/* Destroy
*/
static
rc_t CC GCPDestroy(GCP * self)
{
    free(self->privateKey);
    free(self->client_email);
    free(self->project_id);
    free(self->access_token);
    free(self->jwt);
    return CloudWhack(&self->dad);
}

/* envCE
 * Get Compute Environment Token from environment variable
 *
 * NB. this is a one-shot function, but atomic is not important
 */
static char const *envCE()
{
    static bool firstTime = true;
    char const *const env = firstTime ? getenv(ENV_MAGIC_CE_TOKEN) : NULL;
    firstTime = false;
    if (env != NULL)
        DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH), (
            "Got location from environment\n"));
    return env;
}

/* readCE
 * Get Compute Environment Token by reading from provider network
 */
static rc_t readCE(GCP const *const self, size_t size, char location[])
{
    char const *const identityUrl =
        "http://metadata/computeMetadata/v1/instance/service-accounts/"
        "default/identity?audience=https://www.ncbi.nlm.nih.gov&format=full";

    DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH),
        ("Reading location from provider\n"));
    return KNSManager_Read(self->dad.kns, location, size,
                         identityUrl, "Metadata-Flavor", "Google");
}

/* MakeComputeEnvironmentToken
 *  contact cloud provider to get proof of execution environment in form of a token
*/
static
rc_t CC GCPMakeComputeEnvironmentToken ( const GCP * self, const String ** ce_token )
{
    assert(self);
    
    if (!self->dad.user_agrees_to_reveal_instance_identity)
        return RC(rcCloud, rcProvider, rcIdentifying,
                  rcCondition, rcUnauthorized);
    else {
        char const *const env = envCE();
        char location[4096] = "";
        rc_t const rc = env == NULL ? readCE(self, sizeof(location), location) : 0;
        if (rc == 0) {
            String s;
            StringInitCString(&s, env != NULL ? env : location);
            return StringCopy(ce_token, &s);
        }
        return rc;
    }
}

/* GetLocation
 */
static rc_t GCPGetLocation(const GCP * self, const String ** location) {
    rc_t rc = 0;

    static const char zoneUrl[]
        = "http://metadata.google.internal/computeMetadata/v1/instance/zone";

    char buffer[64] = "";
    const char * slash = NULL;

    char b[99] = "";
    const char * zone = b;

    assert(self);

    rc = KNSManager_Read(self->dad.kns, b, sizeof b,
        zoneUrl, "Metadata-Flavor", "Google");

    if (rc == 0)
        slash = string_rchr(b, sizeof b, '/');
    if (slash != NULL)
        zone = slash + 1;

    if (rc == 0)
        rc = string_printf(buffer, sizeof buffer, NULL, "gs.%s", zone);

    if (rc == 0) {
        String s;
        StringInitCString(&s, buffer);
        rc = StringCopy(location, &s);
    }

    return rc;
}

/* AddComputeEnvironmentTokenForSigner
 *  prepare a request object with a compute environment token
 *  for use by an SDL-associated "signer" service
 */
static
rc_t CC GCPAddComputeEnvironmentTokenForSigner ( const GCP * self, KClientHttpRequest * req )
{
    const String * ce_token = NULL;
    rc_t rc = GCPMakeComputeEnvironmentToken(self, &ce_token);

    if (rc == 0) {
        rc = KHttpRequestAddPostParam(req, "ident=%S", ce_token);
        StringWhack(ce_token);
    }

    return rc;
}

/* AddAuthentication
*  prepare a request object with credentials for authentication
*/
static
rc_t CC GCPAddAuthentication(const GCP * self, KClientHttpRequest * req, const char * http_method)
{
    return 0; /* TODO, if needed */
}

static
rc_t GetPID(uint32_t * pid)
{   /* platform-independent getpid() */
    KProcMgr * procMgr;
    rc_t rc = KProcMgrMakeSingleton(&procMgr);
    if (rc == 0)
    {
        rc_t rc2;
        rc = KProcMgrGetPID(procMgr, pid);
        rc2 = KProcMgrRelease(procMgr);
        if (rc == 0)
        {
            rc = rc2;
        }
    }
    return rc;
}

/* use mbedtls to sign using RSA SHA-256 algorithm
(also known as RSASSA-PKCS1-V1_5-SIGN with the SHA-256 hash function)
*/
rc_t
Sign_RSA_SHA256(
const char * key_PEM,
const char * input,
const String ** output)
{
    rc_t rc = 0;
    int ret;

    /* 0. Initialize (TODO: do only once, attach data to the GCP object) */
    mbedtls_entropy_context ent_ctx;
    mbedtls_ctr_drbg_context ctr_drbg;
    const mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
    char pers[4096];
    int pers_size;
    size_t num_writ;

    uint32_t pid;
    rc = GetPID(&pid);
    if (rc != 0)
    {
        return rc;
    }

    pers_size = string_printf(
        pers, sizeof pers, &num_writ
        , "%s[%d]"
        , __func__
        , pid
        );

    vdb_mbedtls_entropy_init(&ent_ctx);

    vdb_mbedtls_ctr_drbg_init(&ctr_drbg);
    ret = vdb_mbedtls_ctr_drbg_seed(
        &ctr_drbg,
        vdb_mbedtls_entropy_func,
        &ent_ctx,
        (const unsigned char *)pers,
        (size_t)pers_size
        );
    if (ret == 0)
    {
        /* 1. parse key_PEM */
        mbedtls_pk_context pk;
        vdb_mbedtls_pk_init(&pk);
        ret = vdb_mbedtls_pk_parse_key(&pk,
            (unsigned char *)key_PEM,
            string_measure(key_PEM, NULL) + 1,
            NULL, 0);
        if (ret == 0)
        {
            /* 2. generate the checksum */
            const mbedtls_md_info_t * info = vdb_mbedtls_md_info_from_type(md_type);
            size_t dsize = vdb_mbedtls_md_get_size(info);

            unsigned char checksum[512 / 8];
            assert(sizeof checksum >= dsize);
            ret = vdb_mbedtls_md(info,
                (const unsigned char *)input,
                string_measure(input, NULL),
                checksum);
            if (ret == 0)
            {
                /* 3. compute the signature */
                String * out = NULL;
                mbedtls_rsa_context * ctx = vdb_mbedtls_pk_rsa(pk);

                out = malloc(sizeof(String) + ctx->len);
                if (out != NULL)
                {
                    StringInit( out, (char*)out + sizeof(String), ctx->len, (uint32_t)ctx->len );

                    ret = vdb_mbedtls_rsa_rsassa_pkcs1_v15_sign(
                        ctx,
                        vdb_mbedtls_ctr_drbg_random,
                        (void *)& ctr_drbg,
                        MBEDTLS_RSA_PRIVATE,
                        md_type,
                        (unsigned int)dsize,
                        checksum,
                        (unsigned char *)out->addr
                        );
                    if (ret == 0)
                    {
#ifdef _DEBUGGING
                        /* 4. verify the signature */
                        ret = vdb_mbedtls_rsa_rsassa_pkcs1_v15_verify(
                            vdb_mbedtls_pk_rsa(pk),
                            NULL,
                            NULL,
                            MBEDTLS_RSA_PUBLIC,
                            md_type,
                            (unsigned int)dsize,
                            checksum,
                            (const unsigned char *)out->addr
                            );
                        if (ret != 0)
                        {
                            TRACE("vdb_mbedtls_rsa_rsassa_pkcs1_v15_verify = -%#.4X\n", -ret);
                            rc = RC(rcCloud, rcUri, rcInitializing, rcEncryption, rcFailed);
                        }
#endif
                    }
                    else
                    {
                        TRACE("vdb_mbedtls_rsa_rsassa_pkcs1_v15_sign = -%#.4X\n", -ret);
                        rc = RC(rcCloud, rcUri, rcInitializing, rcEncryption, rcFailed);
                    }

                    if (rc == 0)
                    {
                        *output = out;
                    }
                    else
                    {
                        StringWhack(out);
                    }
                }
                else
                {
                    rc = RC(rcCloud, rcUri, rcInitializing, rcMemory, rcExhausted);
                }
            }
            else
            {
                TRACE("vdb_mbedtls_md = -%#.4X\n", -ret);
                rc = RC(rcCloud, rcUri, rcInitializing, rcEncryption, rcFailed);
            }
        }
        else
        {
            TRACE("vdb_mbedtls_pk_parse_key = -%#.4X\n", -ret);
            rc = RC(rcCloud, rcUri, rcInitializing, rcEncryption, rcFailed);
        }

        vdb_mbedtls_pk_free(&pk);
    }
    else
    {
        TRACE("vdb_mbedtls_ctr_drbg_seed = -%#.4X\n", -ret);
        rc = RC(rcCloud, rcUri, rcInitializing, rcEncryption, rcFailed);
    }

    vdb_mbedtls_entropy_free(&ent_ctx);
    vdb_mbedtls_ctr_drbg_free(&ctr_drbg);

    return rc;
}

static
rc_t
MakeJWT(const GCP * self, char ** jwt)
{
    /* From https://developers.google.com/identity/protocols/OAuth2ServiceAccount#authorizingrequests */
    /*
    1. JWT header:
    {"alg":"RS256","typ":"JWT"},
    Base64url encoded: eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9
    */
    const char * jwtHeader_base64url = "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9";

    /*
    2. JWT claim set:
    {
    "iss":"<self->client_email>",
    "scope":"https://www.googleapis.com/auth/devstorage.readonly",
    "aud":"https://www.googleapis.com/oauth2/v4/token",
    "exp":<claim expiration minimum now+1hour, seconds since 00:00:00 UTC, January 1, 1970.>,
    "iat":<claim issue time, seconds since 00:00:00 UTC, January 1, 1970.>
    }
    Base64url encoded
    */
    char claimSet[4096];
    size_t num_writ;
    const KTime_t issued_at = KTimeStamp();
    const KTime_t expiration = issued_at + 60 * 60; /* 1 hour later */
    const String * claimSet_base64url;
    char to_sign[4096];
    const String * signature;
    const String * signature_base64url;
    size_t jwt_size;

    rc_t rc = string_printf(claimSet, sizeof(claimSet) - 1, &num_writ,
        "{"
        "\"iss\":\"%s\","
        "\"scope\":\"https://www.googleapis.com/auth/devstorage.read_only\","
        "\"aud\":\"https://www.googleapis.com/oauth2/v4/token\","
        "\"exp\":%li,"
        "\"iat\":%li"
        "}",
        self->client_email,
        expiration,
        issued_at
        );
    if (rc != 0)
    {
        return rc;
    }
    TRACE("claimSet='%s'\n\n", claimSet);
    /* base64url encode claimSet */
    rc = encodeBase64URL(&claimSet_base64url, claimSet, num_writ);
    if (rc != 0)
    {
        return rc;
    }
    TRACE("claimSet_base64url='%.*s'\n\n", (int)claimSet_base64url->size, claimSet_base64url->addr);

    /*
    3. JSW :
    {Base64url encoded header}.{Base64url encoded claim set}
    signed with self->privateKey
    Base64url encode
    */
    rc = string_printf(to_sign, sizeof(to_sign) - 1, &num_writ, "%s.%S", jwtHeader_base64url, claimSet_base64url);
    if (rc != 0)
    {
        StringWhack(claimSet_base64url);
        return rc;
    }
    TRACE("to_sign='%s'\n\n", to_sign);

    /* sign header_dot_claim with self->privateKey */
    rc = Sign_RSA_SHA256(self->privateKey, to_sign, &signature);
    if (rc != 0)
    {
        StringWhack(claimSet_base64url);
        return rc;
    }
    /* base64url encode signature */
    rc = encodeBase64URL(&signature_base64url, signature->addr, signature->size);
    StringWhack(signature);
    if (rc != 0)
    {
        StringWhack(claimSet_base64url);
        return rc;
    }
    TRACE("signature_base64url='%.*s'\n\n", (int)signature_base64url->size, signature_base64url->addr);

    /*      4. Base64url encode "header.claims.signature" into JWT
    */
    jwt_size = string_measure(jwtHeader_base64url, NULL) + claimSet_base64url->size + signature_base64url->size + 3;
    *jwt = malloc(jwt_size);
    rc = string_printf(*jwt, jwt_size, &num_writ, "%s.%S.%S", jwtHeader_base64url, claimSet_base64url, signature_base64url);
    StringWhack(claimSet_base64url);
    StringWhack(signature_base64url);
    if (rc != 0)
    {
        return rc;
    }
    TRACE("jwt='%s'\n\n", jwt);

    return 0;
}

static
rc_t
GetJsonStringMember(const KJsonObject *obj, const char * name, const char ** value)
{
    const KJsonValue * member = NULL;
    assert(obj != NULL);
    assert(name != NULL);
    assert(value != NULL);

    member = KJsonObjectGetMember(obj, name);
    if (member == NULL)
    {
        return RC(rcKFG, rcFile, rcParsing, rcParam, rcInvalid);
    }
    if (KJsonGetValueType(member) != jsString)
    {
        return RC(rcKFG, rcFile, rcParsing, rcParam, rcInvalid);
    }

    return KJsonGetString(member, value);
}

static
rc_t
GetJsonNumMember(const KJsonObject *obj, const char * name, int64_t * value)
{
    const KJsonValue * member = NULL;
    assert(obj != NULL);
    assert(name != NULL);
    assert(value != NULL);

    member = KJsonObjectGetMember(obj, name);
    if (member == NULL)
    {
        return RC(rcKFG, rcFile, rcParsing, rcParam, rcInvalid);
    }
    if (KJsonGetValueType(member) != jsNumber)
    {
        return RC(rcKFG, rcFile, rcParsing, rcParam, rcInvalid);
    }

    return KJsonGetNumber(member, value);
}

static
rc_t GetAccessToken(const GCP * self, const char * jwt, struct KStream * opt_conn, char ** token, KTime_t * expiration)
{
    /*      5. Https POST
    POST /oauth2/v4/token HTTP/1.1
    Host: www.googleapis.com
    Content-Type: application/x-www-form-urlencoded

    grant_type=urn%3Aietf%3Aparams%3Aoauth%3Agrant-type%3Ajwt-bearer&assertion=<JWT>
    */
    rc_t rc;
    rc_t rc2;
    KClientHttp * client;
    KClientHttpRequest * req;
    char jsonResponse[4096];

    String host;
    CONST_STRING(&host, "www.googleapis.com");

    assert(self);
    assert(jwt);
    assert(token);
    assert(expiration);

    rc = KNSManagerMakeClientHttps(self->dad.kns, &client, opt_conn, 0x01010000, &host, 443);
    if (rc == 0)
    {
        rc = KClientHttpMakeRequest(client, &req, "https://www.googleapis.com/oauth2/v4/token");
    }

    if (rc == 0)
    {
        rc = KClientHttpRequestAddHeader(req, "Content-Type", "application/x-www-form-urlencoded");
    }

    if (rc == 0)
    {
        rc = KHttpRequestAddPostParam(req, "grant_type=%s", "urn%3Aietf%3Aparams%3Aoauth%3Agrant-type%3Ajwt-bearer");
    }

    if (rc == 0)
    {
        rc = KHttpRequestAddPostParam(req, "assertion=%s", jwt);
    }

    if (rc == 0)
    {
        size_t num_read = 0;
        KClientHttpResult * rslt = NULL;

        rc = KClientHttpRequestPOST(req, &rslt);
        if (rc == 0)
        {
            KStream * s = NULL;
            rc = KClientHttpResultGetInputStream(rslt, &s);
            if (rc == 0)
            {
                rc = KStreamRead(s, jsonResponse, sizeof(jsonResponse), &num_read);
                if (rc == 0)
                {
                    if (num_read == sizeof(jsonResponse))
                    {
                        rc = RC(rcCloud, rcUri, rcReading, rcBuffer, rcInsufficient);
                    }
                    else
                    {
                        jsonResponse[num_read++] = '\0';
                        TRACE("Json received: '%s'", jsonResponse);
                    }
                }

                rc2 = KStreamRelease(s);
                if (rc == 0)
                {
                    rc = rc2;
                }
            }
        }

        rc2 = KClientHttpResultRelease(rslt);
        if (rc == 0)
        {
            rc = rc2;
        }
    }

    rc2 = KClientHttpRequestRelease(req);
    if (rc == 0)
    {
        rc = rc2;
    }
    rc2 = KClientHttpRelease(client);
    if (rc == 0)
    {
        rc = rc2;
    }

    if (rc == 0)
    {
        /* 5. Response:
        {
        "access_token" : "1/8xbJqaOZXSUZbHLl5EOtu1pxz3fmmetKx9W8CV4t79M",
        "token_type" : "Bearer",
        "expires_in" : 3600
        }
        */
        KJsonValue * root;
        char error[1024];
        rc = KJsonValueMake(&root, jsonResponse, error, sizeof(error));
        if (rc == 0)
        {
            const KJsonObject *obj = KJsonValueToObject(root);
            if (rc == 0)
            {
                const char * value;
                rc = GetJsonStringMember(obj, "access_token", &value);
                if (rc == 0)
                {
                    *token = string_dup(value, string_measure(value, NULL));
                    if (*token == NULL)
                    {
                        rc = RC(rcNS, rcMgr, rcAllocating, rcMemory, rcExhausted);
                    }
                }
                if (rc == 0)
                {
                    int64_t expires;
                    rc = GetJsonNumMember(obj, "expires_in", &expires);
                    if (rc == 0)
                    {
                        *expiration = KTimeStamp() + expires;
                    }
                }
            }
        }
        if (rc != 0)
        {
            rc = RC(rcCloud, rcUri, rcInitializing, rcFormat, rcUnexpected);
        }
        KJsonValueWhack(root);
    }
    return rc;
}

/* AddUserPaysCredentials
*  prepare a request object with credentials for user-pays
*/
static
rc_t CC GCPAddUserPaysCredentials(const GCP * cself, KClientHttpRequest * req, const char * http_method)
{
    /* Obtain GCP access token and add it to the URL with the project Id */
    GCP * self = (GCP *)cself;
    rc_t rc = 0;
    if (self->client_email == NULL || self->privateKey == NULL || self->project_id == NULL)
    {
        rc = RC(rcCloud, rcProvider, rcAccessing, rcParam, rcNull);
    }
    else
    {
        bool new_token = false;
        /* see if cached access_token has to be generated/refreshed */
        if ( self->access_token == NULL ||
             self->access_token_expiration < KTimeStamp() + 60 ) /* expires in less than a minute */
        {
            free(self->access_token);
            self->access_token = NULL;

            if (self->jwt == NULL)
            {   /* first time here, create the JWT and hold on to it */
                rc = MakeJWT(self, &self->jwt);
            }
            if (rc == 0)
            {
                rc = GetAccessToken(self, self->jwt, self->dad.conn, &self->access_token, &self->access_token_expiration);
            }
            new_token = true;
        }

        if ( rc == 0 )
        {   /* only update the URL if we have not done so yet, or if we have just refreshed the token */
            if ( ! new_token )
            {
                char buffer[4096];
                size_t num_read;
                rc = KClientHttpRequestGetHeader ( req, "Authorization", buffer, sizeof ( buffer ), &num_read );
                if ( GetRCState ( rc ) == rcNotFound )
                {
                    new_token = true;
                    rc = 0;
                }
            }

            if ( rc == 0 && new_token )
            {
                rc = KClientHttpRequestAddHeader(req, "Authorization", "Bearer %s", self->access_token);
            }

            /* Add alt=media&userProject=<project_id> to the URL if not already there */
            if ( rc == 0 )
            {
                const String * query;
                char * nulterm; /* to use strstr, need a 0-terminated string */
                KClientHttpRequestGetQuery( req, & query );
                nulterm = string_dup( query -> addr, query -> size );
                if ( strstr(nulterm, "alt=media" ) == NULL )
                {
                    rc = KClientHttpRequestAddQueryParam(req, "alt", "media");
                }
                if (rc == 0 && strstr(nulterm, "userProject=" ) == NULL)
                {
                    rc = KClientHttpRequestAddQueryParam(req, "userProject", "%s", self->project_id);
                }
                free (nulterm);
            }
        }
    }
    return rc;
}

static Cloud_vt_v1 GCP_vt_v1 =
{
    1, 0,

    GCPDestroy,
    GCPMakeComputeEnvironmentToken,
    GCPGetLocation,
    GCPAddComputeEnvironmentTokenForSigner,
    GCPAddAuthentication,
    GCPAddUserPaysCredentials
};

/* MakeGCP
*  make an instance of an GCP cloud interface
*/
LIB_EXPORT rc_t CC CloudMgrMakeGCP(const CloudMgr * self, GCP ** p_gcp)
{
    rc_t rc;
    GCP * gcp;

    if (self == NULL)
    {
        return RC(rcCloud, rcProvider, rcCasting, rcSelf, rcNull);
    }
    if (p_gcp == NULL)
    {
        return RC(rcCloud, rcProvider, rcCasting, rcParam, rcNull);
    }

    gcp = calloc(1, sizeof * gcp);
    if (gcp == NULL)
    {
        rc = RC(rcCloud, rcMgr, rcAllocating, rcMemory, rcExhausted);
    }
    else
    {
        /* capture from self->kfg */
        bool user_agrees_to_pay = false;
        bool user_agrees_to_reveal_instance_identity = false;
        if (self != NULL) {
            KConfig_Get_User_Accept_Gcp_Charges(self->kfg,
                &user_agrees_to_pay);
            KConfig_Get_Report_Cloud_Instance_Identity(self->kfg,
                &user_agrees_to_reveal_instance_identity);
        }

        rc = CloudInit ( & gcp -> dad, ( const Cloud_vt * ) & GCP_vt_v1, "GCP", self -> kns, user_agrees_to_pay,
            user_agrees_to_reveal_instance_identity );
        if ( rc == 0 )
        {
            rc = PopulateCredentials(gcp);
            if (rc == 0)
            {
                *p_gcp = gcp;
            }
            else
            {
                CloudRelease(&gcp->dad);
            }
        }
        else
        {
            free(gcp);
        }

    }

    return rc;
}

/* AddRef
* Release
*/
LIB_EXPORT rc_t CC GCPAddRef(const GCP * self)
{
    return CloudAddRef(&self->dad);
}

LIB_EXPORT rc_t CC GCPRelease(const GCP * self)
{
    return CloudRelease(&self->dad);
}

/* Cast
*  cast from a Cloud to a GCP type or vice versa
*  allows us to apply cloud-specific interface to cloud object
*
*  returns a new reference, meaning the "self" must still be released
*/
LIB_EXPORT rc_t CC GCPToCloud(const GCP * cself, Cloud ** cloud)
{
    rc_t rc;

    if (cloud == NULL)
        rc = RC(rcCloud, rcProvider, rcCasting, rcParam, rcNull);
    else
    {
        if (cself == NULL)
            rc = 0;
        else
        {
            GCP * self = (GCP *)cself;

            rc = CloudAddRef(&self->dad);
            if (rc == 0)
            {
                *cloud = &self->dad;
                return 0;
            }
        }

        *cloud = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC CloudToGCP(const Cloud * self, GCP ** gcp)
{
    rc_t rc;

    if (gcp == NULL)
        rc = RC(rcCloud, rcProvider, rcCasting, rcParam, rcNull);
    else
    {
        if (self == NULL)
            rc = 0;
        else if (self->vt != (const Cloud_vt *)& GCP_vt_v1)
            rc = RC(rcCloud, rcProvider, rcCasting, rcType, rcIncorrect);
        else
        {
            rc = CloudAddRef(self);
            if (rc == 0)
            {
                *gcp = (GCP *)self;
                return 0;
            }
        }

        *gcp = NULL;
    }

    return rc;
}

/* WithinGCP
*  answers true if within GCP
*/
bool CloudMgrWithinGCP(const CloudMgr * self)
{
    rc_t rc;
    KEndPoint ep;
    String hostname;
    const char host[] = "metadata.google.internal";

    /* describe address "metadata.google.internal" on port 80 */
    CONST_STRING(&hostname, host);
    rc = KNSManagerInitDNSEndpoint(self->kns, &ep, &hostname, 80);
    if (rc == 0)
    {
        DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH), (
            "'%s' DNS was resolved to '%s'\n", host, ep.ip_address));
        /* some DNS servers afford themselves the luxury of returning
           a non-authoritative answer in order to direct a web-browser
           to some other server, e.g. Verizon. This may also occur with
           wireless network access control... */
        if ((ep.u.ipv4.addr >> 16) != ((169U << 8) | 254U)) {
            DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH), (
                "'%s' DNS endpoint was resolved "
                "but IP is not in range 169.254...: ignored\n", host));
            return false;
        }
    }

    if (rc == 0)
    {
        KSocket * conn;

        /* we already have a good idea that the environment looks like GCP */
        rc = KNSManagerMakeTimedConnection(self->kns, &conn, 0, 0, NULL, &ep);
        if (rc == 0)
        {
            /* TBD - is there any sense in finishing the HTTP transaction?
            somebody answered our call, so it looks like they're there,
            if we use the URL to verify a little more, it will confirm...
            something.
            But we're not prepared to retain any information,
            unless it's region */
            KSocketRelease(conn);
            return true;
        }
    }

    return false;
}

static
rc_t PopulateCredentials(GCP * self)
{
    rc_t rc = 0;

    char buffer[PATH_MAX] = "";

    char *jsonCredentials = NULL;

    const char *pathToJsonFile = getenv("GOOGLE_APPLICATION_CREDENTIALS");

    assert(self);

    if (pathToJsonFile == NULL || *pathToJsonFile == 0)
    {
        KConfig * cfg = NULL;
        rc = KConfigMake(&cfg, NULL);

        if (rc == 0)
            rc = KConfig_Get_Gcp_Credential_File(
                cfg, buffer, sizeof buffer, NULL);

        if (rc == 0)
            pathToJsonFile = buffer;
        else
            rc = 0;

        {
            rc_t r2 = KConfigRelease(cfg);
            if (rc == 0 && r2 != 0)
                rc = r2;
        }
    }

    if (pathToJsonFile != NULL && *pathToJsonFile != 0)
    {   /* read the credentials file */
        const KFile *cred_file = NULL;
        uint64_t json_size = 0;

        KDirectory *dir = NULL;
        rc = KDirectoryNativeDir(&dir);
        if (rc == 0)
        {
            rc = KDirectoryOpenFileRead(dir, &cred_file, "%s", pathToJsonFile);
        }

        if (rc == 0)
        {
            rc = KFileSize(cred_file, &json_size);
        }

        if (rc == 0)
        {
            jsonCredentials = (char *)calloc(json_size + 1, 1);
            if (jsonCredentials == NULL)
            {
                rc = RC(rcNS, rcMgr, rcAllocating, rcMemory, rcExhausted);
            }
        }

        if (rc == 0)
        {
            rc = KFileReadExactly(cred_file, 0, jsonCredentials, json_size);
        }

        KFileRelease(cred_file);
        KDirectoryRelease(dir);

        if (rc == 0)
        {   /*extract the credentials */
            KJsonValue *root = NULL;
            rc = KJsonValueMake(&root, jsonCredentials, NULL, 0);
            if (rc == 0)
            {
                const KJsonObject *obj = KJsonValueToObject(root);

                const char *required[] = {
                    "type", "project_id", "private_key_id",
                    "private_key", "client_email", "client_id", "auth_uri", "token_uri",
                    "auth_provider_x509_cert_url", "client_x509_cert_url", NULL
                };

                /* check that all required members are present */
                size_t i = 0;
                while (rc == 0 && required[i] != NULL)
                {
                    const char * value;
                    rc = GetJsonStringMember(obj, required[i], &value);
                    if (rc == 0)
                    { /*TODO: extract a helper */
                        if (strcmp("private_key", required[i]) == 0)
                        {
                            self->privateKey = string_dup(value, string_size(value));
                            if (self->privateKey == NULL)
                            {
                                rc = RC(rcNS, rcMgr, rcAllocating, rcMemory, rcExhausted);
                            }
                        }
                        else if (strcmp("client_email", required[i]) == 0)
                        {
                            self->client_email = string_dup(value, string_size(value));
                            if (self->client_email == NULL)
                            {
                                rc = RC(rcNS, rcMgr, rcAllocating, rcMemory, rcExhausted);
                            }
                        }
                        else if (strcmp("project_id", required[i]) == 0)
                        {
                            self->project_id = string_dup(value, string_size(value));
                            if (self->project_id == NULL)
                            {
                                rc = RC(rcNS, rcMgr, rcAllocating, rcMemory, rcExhausted);
                            }
                        }
                    }
                    ++i;
                }

                KJsonValueWhack(root);
            }
        }

        free(jsonCredentials);
    }

    return rc;
}
