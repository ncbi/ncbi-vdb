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
* AWS Authentication
*/

#include <klib/data-buffer.h> /* KDataBuffer */
#include <klib/debug.h> /* DBGMSG */
#include <klib/log.h> /* LOGERR */
#include <klib/printf.h> /* string_printf */
#include <klib/rc.h>
#include <klib/text.h> /* String */
#include <klib/time.h> /* KTimeStamp */

#include <kns/http.h> /* KClientHttpRequest */
#include <kns/http-priv.h> /* KClientHttpRequestGetRegion */

#include "../kns/http-priv.h" /* KClientHttpRequestGetBody */

#include <mbedtls/base64.h> /* mbedtls_base64_encode */
#include <mbedtls/md.h> /* mbedtls_md_hmac */
#include "mbedtls/sha256.h" /* mbedtls_sha256 */

#include "aws-priv.h" /* AWSDoAuthentication */
#include "cloud-priv.h" /* struct AWS */

/* use mbedtls to generate HMAC_SHA1 */
static rc_t HMAC_SHA1(
    const char *key,
    const char *input,
    unsigned char *output)
{
    int ret = 0;

    const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);

    size_t keylen = string_measure(key, NULL);
    size_t ilen = string_measure(input, NULL);

    ret = mbedtls_md_hmac(md_info, (unsigned char *)key, keylen,
        (unsigned char *)input, ilen, output);

    return ret == 0
        ? 0 : RC(rcCloud, rcUri, rcInitializing, rcEncryption, rcFailed);
}

/* Encode a buffer into base64 format */
static rc_t Base64(
    const unsigned char *src, size_t slen,
    char *dst, size_t dlen)
{
    rc_t rc = 0;

    size_t olen = 0;

#if DEBUGGING
    puts("vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv");
    printf("SRC   : ");
    size_t i = 0;
    for (i = 0; i < slen; ++i)
        printf("%x", src[i]);
    puts("");
#endif

    if (mbedtls_base64_encode((unsigned char *)dst, dlen, &olen, src, slen) != 0)
        rc = RC(rcCloud, rcUri, rcEncoding, rcString, rcInsufficient);

#if DEBUGGING
    olen = strlen((char*)dst);
    printf("DST   : ");
    for (i = 0; i < olen; ++i)
        printf("%c", dst[i]);
    puts("");
    printf("base64: %s\n", dst);
    puts("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
#endif

    return rc;
}

/* Compute AWS authenticating Signature:
Signature =
Base64( HMAC-SHA1( YourSecretAccessKeyID, UTF-8-Encoding-Of( StringToSign ) ) );
*/
static rc_t Signature(const char * YourSecretAccessKeyID,
    const char * StringToSign,
    char *dst, size_t dlen)
{
    unsigned char src[64] = "";

#if DEBUGGING
    int i = 0;
    unsigned char pbuf[64] = "";
#endif

    size_t slen = 20;
    rc_t rc = HMAC_SHA1(YourSecretAccessKeyID, StringToSign, src);

#if DEBUGGING
    rc_t r = pHMAC_SHA1(YourSecretAccessKeyID, StringToSign, pbuf, sizeof pbuf, &slen);

    assert(rc == r);
    assert(slen == 20);
    for (i = 0; i < slen; ++i) {
        assert(src[i] == pbuf[i]);
    }
#endif

    if (rc == 0)
        rc = Base64(src, slen, dst, dlen);

    return rc;
}

rc_t MakeAwsAuthenticationHeader(
    const char *AWSAccessKeyId,
    const char *YourSecretAccessKeyID,
    const char *StringToSign,
    char *dst, size_t dlen)
{
    size_t num_writ = 0;

    rc_t rc = string_printf(dst, dlen, &num_writ, "AWS %s:", AWSAccessKeyId);

    if (rc == 0) {
        if (num_writ >= dlen)
            return RC(rcCloud, rcUri, rcIdentifying, rcString, rcInsufficient);

        rc = Signature(YourSecretAccessKeyID, StringToSign,
            dst + num_writ, dlen - num_writ);
    }

    return rc;
}

#define X_AMZ_REQUEST_PAYER "x-amz-request-payer"
#define REQUESTER "requester"

/* https://docs.aws.amazon.com/AmazonS3/latest/dev/RESTAuthentication.html */
static rc_t StringToSign(
    const String * HTTPVerb,
    const String * Date,
    const String * hostname,
    const String * HTTPRequestURI,
    bool requester_payer,
    char * buffer, size_t bsize, size_t * len)
{
    rc_t rc = 0;
    rc_t r2 = 0;
    size_t total = 0;
    size_t skip = 0;
    size_t p_bsize = 0;

    String dateString;
    String s3;
    CONST_STRING(&s3, ".s3.amazonaws.com");
    CONST_STRING(&dateString, "Date");
    assert(buffer && len);

    /* StringToSign = HTTP-Verb + "\n" */
    assert(HTTPVerb);
    rc = string_printf(buffer, bsize, len, "%S\n", HTTPVerb);
    total += *len;

    /* StringToSign += Content-MD5 + "\n" */
    {
        const char ContentMD5[] = "";
        p_bsize = bsize >= total ? bsize - total : 0;
        r2 = string_printf(&buffer[total], p_bsize, len, "%s\n", ContentMD5);
        total += *len;
        if (rc == 0 && r2 != 0)
            rc = r2;
    }

    /* StringToSign += Content-Type + "\n" */
    {
        const char ContentType[] = "";
        p_bsize = bsize >= total ? bsize - total : 0;
        r2 = string_printf(&buffer[total], p_bsize, len, "%s\n", ContentType);
        total += *len;
        if (rc == 0 && r2 != 0)
            rc = r2;
    }

    /* StringToSign += Date + "\n" */
    assert(Date); /* Signed Amazon queries: Date header is required. */
    p_bsize = bsize >= total ? bsize - total : 0;
    r2 = string_printf(&buffer[total], p_bsize, len, "%S\n", Date);
    total += *len;
    if (rc == 0 && r2 != 0)
        rc = r2;

    /* StringToSign += CanonicalizedAmzHeaders */
    if (requester_payer) {
        p_bsize = bsize >= total ? bsize - total : 0;
        r2 = string_printf(&buffer[total], p_bsize, len,
            X_AMZ_REQUEST_PAYER ":" REQUESTER "\n");
        total += *len;
    }

    /* StringToSign += CanonicalizedResource */
    skip = hostname->size - s3.size;
    if (skip > 0 && hostname->size >= s3.size &&
        string_cmp(s3.addr, s3.size, hostname->addr + skip,
            hostname->size - skip, (uint32_t)s3.size) == 0)
    { /* CanonicalizedResource = [ "/" + Bucket ] */
        String Bucket;
        StringInit(&Bucket, hostname->addr, skip, (uint32_t)skip);
        p_bsize = bsize >= total ? bsize - total : 0;
        r2 = string_printf(&buffer[total], p_bsize, len, "/%S", &Bucket);
        total += *len;
        if (rc == 0 && r2 != 0)
            rc = r2;
    }
    /* CanonicalizedResource += <HTTP-Request-URI protocol name to the query> */
    p_bsize = bsize >= total ? bsize - total : 0;
    assert(HTTPRequestURI);
    r2 = string_printf(&buffer[total], p_bsize, len, "%S", HTTPRequestURI);
    total += *len;
    if (rc == 0 && r2 != 0)
        rc = r2;

    return rc;
}

/* AddAuthentication
 *  prepare a request object with credentials for authentication
 *  use AWS Signature Version 2
 */
rc_t AWSDoAuthentication_v2(const struct AWS * self, KClientHttpRequest * req,
    const char * http_method, bool requester_payer)
{
    rc_t rc = 0;

    char buf[4096] = "";
    const String * sdate = NULL;
    char date[64] = "";
    String dates;
    char stringToSign[4096] = "";
    char authorization[4096] = "";

    if (self->access_key_id == NULL && self->secret_access_key == NULL) {
        rc = RC(rcCloud, rcEncryptionKey, rcIdentifying,
            rcEncryptionKey, rcNotFound);
        switch (self->credentials_state) {
        case eCCEmpty:
            LOGERR(klogErr, rc, "cloud credentials file is empty");
            break;
        case eCCUnrecognized:
            LOGERR(klogErr, rc,
                "unrecognized format of cloud credentials file");
            break;
        case eCCIncomplete:
            LOGERR(klogErr, rc, "cloud credentials file is incomplete");
            break;
        case eCCProfileNotFound:
            LOGERR(klogErr, rc, "profile not found in cloud credentials file");
            break;
        case eCCProfileNotComplete:
            LOGERR(
                klogErr, rc, "profile not complete in cloud credentials file");
            break;
        default:
            LOGERR(klogErr, rc, "cannot find cloud credentials");
            break;
        }
        return rc;
    }

    rc = KClientHttpRequestGetHeader(req, "Authorization",
        buf, sizeof buf, NULL);
    if (rc == 0)
        return 0; /* already has Authorization header */

    /* To add of get Date header */
    rc = KClientHttpRequestGetHeader(req, "Date", buf, sizeof buf, NULL);
    if (rc == 0) {
        StringInitCString(&dates, buf);
        sdate = &dates;
    }
    else {
        KTime_t t = KTimeStamp();

#if _DEBUGGING
        size_t sz =
#endif
            KTimeRfc2616(t, date, sizeof date);
#if _DEBUGGING
        assert(sz < sizeof date);
#endif

        StringInitCString(&dates, date);
        sdate = &dates;
        rc = KClientHttpRequestAddHeader(req, "Date", date);
    }

    if (rc == 0) {
        size_t num_read = 0;
        size_t len = 0;
        char host[4096] = "";
        char path[4096] = "";
        String HTTPVerb;
        String shost;
        String spath;
        StringInitCString(&HTTPVerb, http_method);
        rc = KClientHttpRequestGetHost(req, host, sizeof host, &num_read);
        if (rc == 0)
            rc = KClientHttpRequestGetPath(req, path, sizeof path, &num_read);
        if (rc == 0) {
            StringInitCString(&shost, host);
            StringInitCString(&spath, path);
            assert(sdate);
            rc = StringToSign(&HTTPVerb, sdate, &shost, &spath, requester_payer,
                stringToSign, sizeof stringToSign, &len);
/*          puts(stringToSign); */
        }
    }

    if (rc == 0)
        rc = MakeAwsAuthenticationHeader(
            self->access_key_id, self->secret_access_key,
            stringToSign, authorization, sizeof authorization);

    if (rc == 0)
        rc = KClientHttpRequestAddHeader(req, "Authorization", authorization);

    if (rc == 0 && requester_payer)
        rc = KClientHttpRequestAddHeader(req, X_AMZ_REQUEST_PAYER, REQUESTER);

    return rc;
}

rc_t CalculateSHA256Hash(
    const unsigned char* input, size_t input_len, char hash[65])
{
    int i = 0;

    unsigned char output[32] = "";
    rc_t rc = mbedtls_sha256(input, input_len, output, 0);

    for (i = 0; rc == 0 && i < 32; ++i)
        rc = string_printf(hash + i * 2, 3, NULL, "%02x", output[i]);

    return rc;
}

rc_t BuildStringToSign(const char* requestDateTime,
    const char* region, const char* service,
    const char* hashedCanonicalRequest, KDataBuffer* stringToSign)
{
    rc_t rc = KDataBufferPrintf(stringToSign,
        "AWS4-HMAC-SHA256\n"        /* Algorithm */
        "%s\n"                      /* RequestDateTime */
        "%.*s/%s/%s/aws4_request\n" /* CredentialScope:
                                       date/region/service/aws4_request */
        "%s"                        /* HashedCanonicalRequest */
        , requestDateTime
        , (int)8, requestDateTime, region, service
        , hashedCanonicalRequest);

    if (rc == 0) {
        assert(stringToSign);
        DBGMSG(DBG_CLOUD, DBG_FLAG(DBG_CLOUD_SIGN),
            ("AWSAuthV4Signer: Final String to sign: %.*s\n",
                (int)(stringToSign->elem_count - 1),
                stringToSign->base));
    }

    return rc;
}

rc_t HMAC_SHA256(const mbedtls_md_info_t* md_info,
    const unsigned char* key, size_t keylen,
    const char* input, size_t ilen, unsigned char* output)
{
    return mbedtls_md_hmac(md_info, key, keylen,
        (const unsigned char* )input, ilen, output);
}

/* Calculate the signature for SigV4 for a signed AWS API request.
 *
 * User Guide:
 https://docs.aws.amazon.com/IAM/latest/UserGuide/reference_sigv-create-signed-request.html
 *
 * Example:
 * https://docs.aws.amazon.com/AmazonS3/latest/developerguide/sig-v4-header-based-auth.html
 *
 * Perform a keyed hash operation on the string to sign
 * using the derived signing key as the hash key.
 *
 * "secretAccessKey" [ IN ] -  a string that contains your secret access key
 * "date" [ IN ] - a string that contains the date used in the credential scope,
 *                 in the format YYYYMMDD. (date length is 6)
 * "region/regionLen" [ IN ] - a string that contains the Region code
 *                             (for example, us-east-1)
 *  Service is ec2 (hardcoded) - a string that contains the service code
 * "stringToSign/stringToSignLen" [ IN ] - the string to sign
 *     to perform a keyed hash operation to calculate
 *     the signature for a signed AWS API request
 * "signatureHex" [ OUT ] - the Signature for the Authorization Header
 *                          in hexadecimal representation
 */
rc_t CalculateSignature(
    const char* secretAccessKey,
    const char* date,
    const char* region, size_t regionLen,
    const char* service, size_t serviceLen,
    const char* stringToSign, size_t stringToSignLen,
    char signatureHex[65])
{
    rc_t rc = 0;
    int i = 0;

    char termination[] = "aws4_request";

    size_t num_writ = 0;
    const mbedtls_md_info_t* md_info = NULL;

    unsigned char key[99] = "";
    unsigned char dateKey[32] = ""; /* SHA-256 outputs 32 bytes */
    unsigned char dateRegionKey[32] = "";
    unsigned char dateRegionServiceKey[32] = "";
    unsigned char signingKey[32] = "";
    unsigned char signature[32] = "";

    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);

    md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (md_info == NULL) {
        rc = RC(rcCloud, rcUri, rcInitializing, rcEncryption, rcFailed);
        LOGERR(klogErr, rc, "Failed to locate SHA256 information");
        mbedtls_md_free(&ctx);
        return rc;
    }

    /* DateKey = HMAC-SHA256("AWS4" + "<SecretAccessKey>", "<YYYYMMDD>") */
    rc = string_printf(
        (char*)key, sizeof key, &num_writ, "AWS4%s", secretAccessKey);
    if (rc == 0)
        rc = HMAC_SHA256(md_info, key, num_writ, date, 8, dateKey);

    /* DateRegionKey = HMAC-SHA256(<DateKey>, "<aws-region>") */
    if (rc == 0)
        rc = HMAC_SHA256(md_info, dateKey, sizeof dateKey,
            region, regionLen, dateRegionKey);

    /* DateRegionServiceKey = HMAC-SHA256(<DateRegionKey>, "<aws-service>") */
    if (rc == 0)
        rc = HMAC_SHA256(md_info, dateRegionKey, sizeof dateRegionKey,
            service, serviceLen, dateRegionServiceKey);

    /* SigningKey = HMAC-SHA256(<DateRegionServiceKey>, "aws4_request") */
    if (rc == 0)
        rc = HMAC_SHA256(md_info,
            dateRegionServiceKey, sizeof dateRegionServiceKey,
            termination, sizeof termination - 1, signingKey);

    /* calculate a signature for SigV4
       signature = hash(SigningKey, string-to-sign) */
    if (rc == 0)
        rc = HMAC_SHA256(md_info,
            signingKey, 32,
            stringToSign, stringToSignLen, signature);

    /* Convert the signature from binary to hexadecimal representation,
       in lowercase characters */
    for (i = 0; rc == 0 && i < 32; ++i)
        rc = string_printf(
            signatureHex + i * 2, 3, NULL, "%02x", signature[i]);

    mbedtls_md_free(&ctx);

    if (rc == 0)
        DBGMSG(DBG_CLOUD, DBG_FLAG(DBG_CLOUD_SIGN),
            ("AWSAuthV4Signer: Final computed signing hash: %s\n",
                signatureHex));

    return rc;
}

rc_t CreateAuthorizationHeader(const char* awsAccessKeyId,
    const char* date, const char* region,
    const char* signedHeaders, const char* signature,
    KDataBuffer* header)
{
    rc_t rc = KDataBufferPrintf(header,
        "AWS4-HMAC-SHA256 "
        "Credential=%s/%.*s/%s/s3/aws4_request, "
        "SignedHeaders=%s, "
        "Signature=%s",
        awsAccessKeyId, 8, date, region, signedHeaders, signature);

    if (rc == 0) {
        assert(header);
        DBGMSG(DBG_CLOUD, DBG_FLAG(DBG_CLOUD_SIGN),
            ("AWSAuthV4Signer: Signing request with: %.*s\n",
                (int)(header->elem_count - 1),
                header->base));
    }

    return rc;
}

/* AddAuthentication
 *  prepare a request object with credentials for authentication
 *  use AWS Signature Version 4
 */
rc_t AWSDoAuthentication_v4(const struct AWS* self, KClientHttpRequest* req,
    const char* http_method, bool requester_payer)
{
    char buf[4096] = "";
    char hashedCanonicalRequest[65] = "";
    char signature[65] = "";
    char bodyHashHex[65] = "";
    char* signedHeaders = NULL;
    KDataBuffer b;
    rc_t rc = 0;

    KTime_t now = KTimeStamp();

    char t[17] = "";
    size_t s = KTimeIso8601Basic(now, t, sizeof t);

    const char* body = KClientHttpRequestGetBody(req);

    rc = KClientHttpRequestGetHeader(req, "authorization",
        buf, sizeof buf, NULL);
    if (rc == 0)
        return 0; /* already has Authorization header */

    if (s == 0)
        return RC(rcCloud, rcUri, rcEncoding, rcString, rcInsufficient);

    rc = KClientHttpRequestAddHeader(req, "x-amz-date", t);
    if (rc == 0) { /* the hash of payload */
        uint32_t len = string_measure(body, NULL);
        rc = CalculateSHA256Hash((unsigned char*)body, len, bodyHashHex);
    }

    if (rc == 0)
        rc = KClientHttpRequestAddHeader(req, "x-amz-content-sha256",
            bodyHashHex);

    if (rc == 0)
        rc = KDataBufferMake(&b, 8, 0);

    assert(self && req);

    if (rc == 0)
        rc = KClientHttpRequestCreateCanonicalRequestString(
            req, http_method, bodyHashHex, &b, &signedHeaders);
    if (rc == 0)
        rc = CalculateSHA256Hash(b.base, b.elem_count - 1,
            hashedCanonicalRequest);
    if (rc == 0)
        rc = KDataBufferWhack(&b);

    if (rc == 0) {
        String region;
        String service;
        if (rc == 0)
            rc = KClientHttpRequestGetRegion(req, &region);
        if (rc == 0)
            rc = KClientHttpRequestGetService(req, &service);

        if (rc == 0)
            rc = BuildStringToSign(
                t, region.addr, service.addr, hashedCanonicalRequest, &b);

        if (rc == 0)
            rc = CalculateSignature(self->secret_access_key, t,
                region.addr, region.size, service.addr, service.size,
                b.base, b.elem_count - 1, signature);
        if (rc == 0)
            rc = KDataBufferWhack(&b);

        if (rc == 0)
            rc = CreateAuthorizationHeader(self->access_key_id,
                t, region.addr, signedHeaders, signature, &b);

        if (rc == 0)
            rc = KClientHttpRequestAddHeader(req, "authorization", "%.*s",
                (int)b.elem_count - 1, b.base);
    }

    {
        rc_t r2 = KDataBufferWhack(&b);
        if (r2 != 0 && rc == 0)
            rc = r2;

        free(signedHeaders);
    }

    return rc;
}

/* AddAuthentication
 *  prepare a request object with credentials for authentication
 */
rc_t AWSDoAuthentication(const struct AWS* self, KClientHttpRequest* req,
    const char* http_method, bool requester_payer)
{
    assert(self);

    if (self->version2)
        return AWSDoAuthentication_v2(self, req, http_method, requester_payer);
    else
        return AWSDoAuthentication_v4(self, req, http_method, requester_payer);
}

/* get (allocate?) the following as char * or String (*?):
- curl -s http://169.254.169.254/latest/dynamic/instance-identity/pkcs7
- curl -s http://169.254.169.254/latest/dynamic/instance-identity/document
- base64 on document:
        curl -s http://169.254.169.254/latest/dynamic/instance-identity/document
             | base64 -w0
*/

rc_t Base64InIdentityDocument(const char *src, char *dst, size_t dlen) {
    size_t slen = string_measure(src, NULL);
    return Base64((const unsigned char *)src, slen, dst, dlen);
}

rc_t WrapInIdentityPkcs7(const char *src, char *dst, size_t dlen) {
    return string_printf(dst, dlen, NULL,
        "-----BEGIN PKCS7-----\n%s\n-----END PKCS7-----\n", src);
}

rc_t Base64InIdentityPkcs7(const char *src, char *dst, size_t dlen) {
    char wrapped[4096] = "";
    rc_t rc = WrapInIdentityPkcs7(src, wrapped, sizeof wrapped);
    if (rc == 0) {
        size_t slen = string_measure(wrapped, NULL);
        rc = Base64((unsigned char *)wrapped, slen, dst, dlen);
    }
    return rc;
}

rc_t MakeLocation(const char *pkcs7, const char *document,
    char *dst, size_t dlen)
{
    char bpkcs7[4096] = "";
    rc_t rc = Base64InIdentityPkcs7(pkcs7, bpkcs7, sizeof bpkcs7);
    if (rc == 0) {
        char documnt[4096] = "";
        rc = Base64InIdentityDocument(document, documnt, sizeof documnt);
        if (rc == 0)
            rc = string_printf(dst, dlen, NULL, "%s.%s", bpkcs7, documnt);
    }
    return rc;
}
