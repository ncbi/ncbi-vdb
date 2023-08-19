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

#include <klib/base64.h> /* encodeBase64URL */
#include <klib/json.h> /* KJsonObject */
#include <klib/printf.h> /* string_printf */
#include <klib/rc.h> /* RC */
#include <klib/text.h> /* string_dup */

#include <kns/http.h> /* KClientHttpRelease */
#include <kns/stream.h> /* KStreamRelease */

#include <assert.h>

#include "cloud-priv.h" /* GCP */
#include "gcp-priv.h" /* GCPAddAuthentication */

static
rc_t
MakeJWTAuth(const GCP * self, char ** jwt)
{
    char jwtHeader[4096] = "";
    size_t num_writ = 0;

    String * jwtHeader_base64url = NULL;;

    char claimSet[4096] = "";
    const KTime_t issued_at = KTimeStamp();
    const KTime_t expiration = issued_at + 60 * 60; /* 1 hour later */
    String * claimSet_base64url = NULL;
    char to_sign[4096] = "";
    const String * signature = NULL;
    String * signature_base64url = NULL;
    size_t jwt_size = 0;

    /* 1. JWT header */
    rc_t rc = string_printf(jwtHeader, sizeof(jwtHeader) - 1, &num_writ,
        "{"
            "\"alg\":\"RS256\","
            "\"kid\":\"%s\","
            "\"typ\":\"JWT\""
        "}",
        self->private_key_id
    );
    if (rc != 0)
        return rc;

    rc = encodeBase64URL((const String**)(&jwtHeader_base64url),
        jwtHeader, num_writ);
    if (rc != 0)
        return rc;
    while (jwtHeader_base64url->size > 0 &&
        (jwtHeader_base64url->addr)[jwtHeader_base64url->size - 1] == '='
        )
    {
        ((char*)jwtHeader_base64url->addr)[jwtHeader_base64url->size - 1]
            = '\0';
        --jwtHeader_base64url->size;
        --jwtHeader_base64url->len;
    }

    /* 2. JWT claim set */

    rc = string_printf(claimSet, sizeof(claimSet) - 1, &num_writ,
        "{"
            "\"aud\":\"https://oauth2.googleapis.com/token\","
            "\"exp\":%li,"
            "\"iat\":%li,"
            "\"iss\":\"%s\","
            "\"scope\":\"https://www.googleapis.com/auth/cloud-platform\""
        "}",
        expiration,
        issued_at,
        self->client_email
    );
    if (rc != 0)
        return rc;
    TRACE("claimSet='%s'\n\n", claimSet);
    /* base64url encode claimSet */
    rc = encodeBase64URL((const String**)&claimSet_base64url, claimSet,
        num_writ);
    if (rc != 0)
        return rc;
    while (claimSet_base64url->size > 0 &&
        (claimSet_base64url->addr)[claimSet_base64url->size - 1] == '='
        )
    {
        ((char*)claimSet_base64url->addr)[claimSet_base64url->size - 1]
            = '\0';
        --claimSet_base64url->size;
        --claimSet_base64url->len;
    }
    TRACE("claimSet_base64url='%.*s'\n\n",
        (int)claimSet_base64url->size, claimSet_base64url->addr);

    /*
        3. JSW :
            {Base64url encoded header}.{Base64url encoded claim set}
            signed with self->privateKey
            Base64url encode
    */
    rc = string_printf(to_sign, sizeof(to_sign) - 1, &num_writ,
        "%S.%S", jwtHeader_base64url, claimSet_base64url);
    if (rc != 0) {
        StringWhack(claimSet_base64url);
        return rc;
    }
    TRACE("to_sign='%s'\n\n", to_sign);

    /* sign header_dot_claim with self->privateKey */
    rc = Sign_RSA_SHA256(self->privateKey, to_sign, &signature);
    if (rc != 0) {
        StringWhack(claimSet_base64url);
        return rc;
    }
    /* base64url encode signature */
    rc = encodeBase64URL((const String**)&signature_base64url,
        signature->addr, signature->size);
    StringWhack(signature);
    if (rc != 0) {
        StringWhack(claimSet_base64url);
        return rc;
    }
    while (signature_base64url->size > 0 &&
        (signature_base64url->addr)[signature_base64url->size - 1] == '='
        )
    {
        ((char*)signature_base64url->addr)[signature_base64url->size - 1]
            = '\0';
        --signature_base64url->size;
        --signature_base64url->len;
    }
    TRACE("signature_base64url='%.*s'\n\n",
        (int)signature_base64url->size, signature_base64url->addr);

    /*      4. Base64url encode "header.claims.signature" into JWT */
    jwt_size = jwtHeader_base64url->size
        + claimSet_base64url->size + signature_base64url->size + 3;
    *jwt = malloc(jwt_size);
    rc = string_printf(*jwt, jwt_size, &num_writ, "%S.%S.%S",
        jwtHeader_base64url, claimSet_base64url, signature_base64url);
    StringWhack(claimSet_base64url);
    StringWhack(signature_base64url);
    if (rc != 0)
        return rc;
    TRACE("jwt='%s'\n\n", *jwt);

    return 0;
}

static
rc_t GetAccessTokenAuth(const GCP * self, const char * jwt,
    struct KStream * opt_conn, char ** token, KTime_t * expiration)
{
    /*      5. Https POST
    Content-Type: application/x-www-form-urlencoded

    grant_type=urn%3Aietf%3Aparams%3Aoauth%3Agrant-type%3Ajwt-bearer
               &assertion=<JWT>
    */
    rc_t rc = 0;
    rc_t rc2 = 0;
    KClientHttp * client = NULL;
    KClientHttpRequest * req = NULL;
    char jsonResponse[4096] = "";

    String host;
    CONST_STRING(&host, "www.googleapis.com");

    assert(self);
    assert(jwt);
    assert(token);
    assert(expiration);

    rc = KNSManagerMakeClientHttps(self->dad.kns,
        &client, opt_conn, 0x01010000, &host, 443);
    if (rc == 0)
        rc = KClientHttpMakeRequest(client, &req,
            "https://oauth2.googleapis.com/token");

    if (rc == 0)
        rc = KClientHttpRequestAddHeader(req, "Content-Type",
            "application/x-www-form-urlencoded");

    if (rc == 0)
        rc = KHttpRequestAddPostParam(req, "grant_type=%s",
            "urn%3Aietf%3Aparams%3Aoauth%3Agrant-type%3Ajwt-bearer");

    if (rc == 0)
        rc = KHttpRequestAddPostParam(req, "assertion=%s", jwt);

    if (rc == 0)
    {
        size_t num_read = 0;
        KClientHttpResult * rslt = NULL;

        rc = KClientHttpRequestPOST(req, &rslt);
        if (rc == 0) {
            KStream * s = NULL;
            rc = KClientHttpResultGetInputStream(rslt, &s);
            if (rc == 0) {
                rc = KStreamRead(s, jsonResponse, sizeof jsonResponse,
                    &num_read);
                if (rc == 0) {
                    if (num_read == sizeof(jsonResponse))
                        rc = RC(rcCloud, rcUri, rcReading,
                            rcBuffer, rcInsufficient);
                    else {
                        jsonResponse[num_read++] = '\0';
                        TRACE("Json received: '%s'", jsonResponse);
                    }
                }

                rc2 = KStreamRelease(s);
                if (rc == 0 && rc2 != 0)
                    rc = rc2;
            }
        }

        rc2 = KClientHttpResultRelease(rslt);
        if (rc == 0 && rc2 != 0)
            rc = rc2;
    }

    rc2 = KClientHttpRequestRelease(req);
    if (rc == 0 && rc2 != 0)
        rc = rc2;
    rc2 = KClientHttpRelease(client);
    if (rc == 0 && rc2 != 0)
        rc = rc2;

    if (rc == 0) {
        /* 5. Response:
            {
               "dad.access_token" : "1/8xbJqaOZXSUZbHLl5EOtu1pxz3fmmetKx9W8CV4t79M",
               "token_type" : "Bearer",
               "expires_in" : 3600
            }
        */
        KJsonValue * root = NULL;
        char error[1024] = "";
        rc = KJsonValueMake(&root, jsonResponse, error, sizeof error);
        if (rc == 0) {
            const KJsonObject *obj = KJsonValueToObject(root);
            if (rc == 0) {
                const char * value = NULL;
                rc = GetJsonStringMember(obj, "dad.access_token", &value);
                if (rc == 0) {
                    *token = string_dup(value, string_measure(value, NULL));
                    if (*token == NULL)
                        rc = RC(rcNS, rcMgr, rcAllocating,
                            rcMemory, rcExhausted);
                }
                if (rc == 0) {
                    int64_t expires = 0;
                    rc = GetJsonNumMember(obj, "expires_in", &expires);
                    if (rc == 0)
                        *expiration = KTimeStamp() + expires;
                }
            }
        }
        if (rc != 0)
            rc = RC(rcCloud, rcUri, rcInitializing, rcFormat, rcUnexpected);
        KJsonValueWhack(root);
    }
    return rc;
}

/* GCPAddAuthentication
*  prepare a request object with credentials for authentication
*/
rc_t CC GCPAddAuthentication(const GCP * cself,
    KClientHttpRequest * req, const char * http_method)
{
    /* Obtain GCP access token and add it to the URL with the project Id */
    GCP * self = (GCP *)cself;
    rc_t rc = 0;
    if (self->client_email == NULL || self->privateKey == NULL
        || self->project_id == NULL)
    {
        rc = RC(rcCloud, rcProvider, rcAccessing, rcParam, rcNull);
    }
    else {
        bool new_token = false;
        /* see if cached dad.access_token has to be generated/refreshed */
        if ( self->dad.access_token == NULL ||
             self->dad.access_token_expiration < KTimeStamp() + 60 )
               /* expires in less than a minute */
        {
            free(self->dad.access_token);
            self->dad.access_token = NULL;

            if (self->jwt == NULL)
                /* first time here, create the JWT and hold on to it */
                rc = MakeJWTAuth(self, &self->jwt);
            if (rc == 0)
                rc = GetAccessTokenAuth(self, self->jwt, self->dad.conn,
                    &self->dad.access_token, &self->dad.access_token_expiration);
            new_token = true;
        }

        if ( rc == 0 ) { /* only update the URL
           if we have not done so yet, or if we have just refreshed the token */
            if ( ! new_token ) {
                char buffer[4096] = "";
                size_t num_read = 0;
                rc = KClientHttpRequestGetHeader ( req,
                    "Authorization", buffer, sizeof ( buffer ), &num_read );
                if ( GetRCState ( rc ) == rcNotFound ) {
                    new_token = true;
                    rc = 0;
                }
            }

            if ( rc == 0 && new_token )
                rc = KClientHttpRequestAddHeader(req, "Authorization",
                    "Bearer %s", self->dad.access_token);
        }
    }
    return rc;
}
