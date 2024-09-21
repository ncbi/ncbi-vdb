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
* ===========================================================================
*
*/

struct AWS;
#define CLOUD_IMPL struct AWS

#include <cloud/extern.h>
#include <cloud/impl.h>

#include <cloud/aws.h>

#include <klib/debug.h> /* DBGMSG */
#include <klib/log.h> /* PLOGMSG */
#include <klib/printf.h>
#include <klib/rc.h>
#include <klib/status.h>
#include <klib/strings.h> /* ENV_MAGIC_CE_TOKEN */
#include <klib/text.h>

#include <kfg/config.h>
#include <kfg/kfg-priv.h>
#include <kfg/properties.h>
#include <kns/endpoint.h>
#include <kns/socket.h>
#include <kns/http.h>
#include <kns/stream.h>
#include <kfs/directory.h>
#include <kfs/file.h>

#include <assert.h>

#include "aws-priv.h" /* AWSDoAuthentication */
#include "cloud-cmn.h" /* KNSManager_Read */
#include "cloud-priv.h" /* CloudGetCachedComputeEnvironmentToken */

#ifdef WINDOWS
#pragma warning(disable:4127)
/*
to suppress the following condition warning:
sizeof buf_size < sizeof file_size && ( uint64_t ) buf_size != file_size
*/
#endif

static rc_t PopulateCredentials ( AWS * self, KConfig * kfg );

/* Destroy
 */
static
rc_t CC AWSDestroy ( AWS * self )
{
    free ( self -> region );
    free ( self -> output );
    free ( self -> access_key_id );
    free ( self -> secret_access_key );
    free ( self -> profile );
    return CloudWhack ( & self -> dad );
}

static uint64_t AccessTokenLifetime_sec = 21600; // 6 hours

#define INSTANCE_URL_PREFIX "http://169.254.169.254/latest/"

static
uint8_t
IMDS_version( const KNSManager * kns, char *buffer, size_t bsize )
{
    // try IMDSv1 first
    if ( KNSManager_Read( kns, buffer, bsize,
                          INSTANCE_URL_PREFIX "meta-data",  HttpMethod_Get,
                          NULL, NULL) == 0
         && buffer[ 0 ] != '\0' )
    {
        buffer[ 0 ] = 0;
        return 1;
    }

    if ( KNSManager_Read( kns, buffer, bsize,
                          INSTANCE_URL_PREFIX "api/token", HttpMethod_Put,
                          "X-aws-ec2-metadata-token-ttl-seconds", "%u", AccessTokenLifetime_sec ) == 0 )
    {
        return 2;
    }
    return 0;
}

static
void
AWSInitAccess( AWS * self )
{
    // try IMDSv1 first
    char buffer[4096];
    self -> IMDS_version = IMDS_version( self -> dad . kns, buffer, sizeof( buffer ) );
    if ( self -> IMDS_version == 2 )
    {   // save off the session token and its expiration
        self -> dad . access_token = string_dup ( buffer, string_size ( buffer ) );
        self -> dad . access_token_expiration = KTimeStamp() + AccessTokenLifetime_sec;
    }
}

static
rc_t
GetInstanceInfo( const AWS * cself, const char * url, char *buffer_ext, size_t bsize )
{
    switch ( cself -> IMDS_version )
    {
    case 1:
        return KNSManager_Read( cself -> dad . kns, buffer_ext, bsize, url, HttpMethod_Get, NULL, NULL );
    case 2:
        {
            /* see if cached access_token has to be generated/refreshed */
            if ( cself -> dad . access_token_expiration < KTimeStamp() + 60 ) /* expires in less than a minute */
            {
                AWS * self = (AWS *)cself;
                free( self -> dad . access_token );
                self -> dad . access_token = NULL;
                char buf[4096];
                rc_t rc = KNSManager_Read( self -> dad . kns, buf, sizeof( buf),
                          INSTANCE_URL_PREFIX "api/token", HttpMethod_Put,
                          "X-aws-ec2-metadata-token-ttl-seconds", "%u", AccessTokenLifetime_sec );
                if ( rc != 0 )
                {
                    return rc;
                }
                self -> dad . access_token = string_dup ( buf, string_size ( buf) );
                self -> dad . access_token_expiration = KTimeStamp() + AccessTokenLifetime_sec;
            }

            // curl -H "X-aws-ec2-metadata-token: $TOKEN" http://169.254.169.254/latest/meta-data/placement/availability-zone
            return KNSManager_Read( cself -> dad . kns, buffer_ext, bsize, url, HttpMethod_Get,
                                    "X-aws-ec2-metadata-token", "%s", cself -> dad . access_token );
        }
    default:
        // on AWS; the object can still be useful for  testing
        buffer_ext[ 0 ] = 0;
        return 0;
    }
}

static
rc_t
KNSManager_GetAWSLocation( const AWS * self, char *buffer, size_t bsize )
{
    return GetInstanceInfo( self, INSTANCE_URL_PREFIX "meta-data/placement/availability-zone", buffer, bsize );
}

/* envCE
 * Get Compute Environment Token from environment variable
 *
 * NB. this is a one-shot function, but atomic is not important
 */
static char const *envCE(char* buf, size_t buf_size)
{
    static bool firstTime = true;
#ifdef WINDOWS
    char const* env = NULL;
    if ( firstTime )
    {
        size_t buf_count = 0;
        errno_t err = getenv_s ( & buf_count, buf, buf_size, ENV_MAGIC_CE_TOKEN );
        assert ( buf_count <= buf_size );
        assert ( err != ERANGE );
        if ( !err && buf_count != 0 )
            env = buf;
    }
#else
    char const *const env = firstTime ? getenv(ENV_MAGIC_CE_TOKEN) : NULL;
#endif
    firstTime = false;
    if ( env != NULL && *env != 0 )
        DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_CE),
               ("Got AWS location from environment\n"));
    return env;
}

/* Get Pkcs7 signature from provider network
 */
rc_t GetPkcs7( const struct AWS * self, char *buffer, size_t bsize )
{
    return GetInstanceInfo( self, INSTANCE_URL_PREFIX "dynamic/instance-identity/pkcs7", buffer, bsize );
}

/* readCE
 * Get Compute Environment Token by reading from provider network
 */
static rc_t readCE(AWS const *const self, size_t size, char location[])
{
    char document[4096] = "";
    char pkcs7[4096] = "";
    rc_t rc;

    DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_CE),
           ("Reading AWS location from provider\n"));
    rc = GetInstanceInfo( self, INSTANCE_URL_PREFIX "dynamic/instance-identity/document", document, sizeof document );
    if (rc) return rc;

    rc = GetPkcs7( self, pkcs7, sizeof pkcs7 );
    if (rc) return rc;

    return MakeLocation(pkcs7, document, location, size);
}

/* MakeComputeEnvironmentToken
 *  contact cloud provider to get proof of execution environment in form of a token
 */
static
rc_t CC AWSMakeComputeEnvironmentToken ( const AWS * self, const String ** ce_token )
{
    assert(self);

    if (!self->dad.user_agrees_to_reveal_instance_identity)
        return RC(rcCloud, rcProvider, rcIdentifying,
                  rcCondition, rcUnauthorized);
    else {
        char env_buf[4096];
        char const *const env = envCE(env_buf, sizeof(env_buf));
        char location[4096] = "";
        rc_t rc = 0;
        if (CloudGetCachedComputeEnvironmentToken(&self->dad, ce_token))
            return 0;
        rc = env == NULL ? readCE(self, sizeof(location), location) : 0;
        if (rc == 0) {
            String s;
            StringInitCString(&s, env != NULL ? env : location);
            rc = StringCopy(ce_token, &s);
            if (rc == 0) { /* ignore rc below */
                assert(ce_token);
                CloudSetCachedComputeEnvironmentToken(&self->dad, *ce_token);
            }
            return rc;
        }
        return rc;
    }
}

/* AwsGetLocation
 */
static rc_t AwsGetLocation(const AWS * self, const String ** location)
{
    rc_t rc = 0;

    char zone[64] = "";
    char buffer[64] = "";

    assert(self);

    rc = KNSManager_GetAWSLocation( self, zone, sizeof zone );

    if (rc == 0)
        rc = string_printf(buffer, sizeof buffer, NULL, "s3.%s", zone);

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
rc_t CC AWSAddComputeEnvironmentTokenForSigner ( const AWS * self, KClientHttpRequest * req )
{
    const String * ce_token = NULL;
    rc_t rc = AWSMakeComputeEnvironmentToken(self, &ce_token);

    if (rc == 0) {
        rc = KHttpRequestAddPostParam(req, "ident=%S", ce_token);
        StringWhack(ce_token);
    }

    return rc;
}

/* AddAuthentication
 *  prepare a request object with credentials for authentication
 */
static rc_t CC AWSAddAuthentication ( const AWS * self,
    KClientHttpRequest * req, const char * http_method )
{
    return AWSDoAuthentication(self, req, http_method, false);
}

/* AddUserPaysCredentials
 *  prepare a request object with credentials for user-pays
 */
static
rc_t CC AWSAddUserPaysCredentials ( const AWS * self, KClientHttpRequest * req, const char * http_method )
{
    assert(self);

    if (self->dad.user_agrees_to_pay)
        return AWSDoAuthentication(self, req, http_method, true);
    else
        return 0;
}

static Cloud_vt_v1 AWS_vt_v1 =
{
    1, 0,

    AWSDestroy,
    AWSMakeComputeEnvironmentToken,
    AwsGetLocation,
    AWSAddComputeEnvironmentTokenForSigner,
    AWSAddAuthentication,
    AWSAddUserPaysCredentials
};


/* MakeAWS
 *  make an instance of an AWS cloud interface
 */
LIB_EXPORT rc_t CC CloudMgrMakeAWS ( const CloudMgr * self, AWS ** p_aws )
{
    rc_t rc;
//TODO: check self, aws
    AWS * aws = calloc ( 1, sizeof * aws );
    if ( aws == NULL )
    {
        rc = RC ( rcCloud, rcMgr, rcAllocating, rcMemory, rcExhausted );
    }
    else
    {
        /* capture from self->kfg */
        bool user_agrees_to_pay = false;
        bool user_agrees_to_reveal_instance_identity = false;
        if (self != NULL) {
            KConfig_Get_User_Accept_Aws_Charges(self->kfg,
                &user_agrees_to_pay);
            KConfig_Get_Report_Cloud_Instance_Identity(self->kfg,
                &user_agrees_to_reveal_instance_identity);
        }

        rc = CloudInit ( & aws -> dad, ( const Cloud_vt * ) & AWS_vt_v1, "AWS",
            self, user_agrees_to_pay, user_agrees_to_reveal_instance_identity );
        if ( rc == 0 )
        {
            if ( user_agrees_to_pay )
                rc = PopulateCredentials( aws, (KConfig*)self->kfg );
            if ( rc == 0 )
            {
                AWSInitAccess( aws );
                * p_aws = aws;
            }
            else
            {
                CloudRelease( & aws -> dad );
            }
        }
        else
        {
            free ( aws );
        }
    }

    return rc;
}

/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC AWSAddRef ( const AWS * self )
{
    return CloudAddRef ( & self -> dad );
}

LIB_EXPORT rc_t CC AWSRelease ( const AWS * self )
{
    return CloudRelease ( & self -> dad );
}

/* Cast
 *  cast from a Cloud to an AWS type or vice versa
 *  allows us to apply cloud-specific interface to cloud object
 *
 *  returns a new reference, meaning the "self" must still be released
 */
LIB_EXPORT rc_t CC AWSToCloud ( const AWS * cself, Cloud ** cloud )
{
    rc_t rc;

    if ( cloud == NULL )
        rc = RC ( rcCloud, rcProvider, rcCasting, rcParam, rcNull );
    else
    {
        if ( cself == NULL )
            rc = 0;
        else
        {
            AWS * self = ( AWS * ) cself;

            rc = CloudAddRef ( & self -> dad );
            if ( rc == 0 )
            {
                * cloud = & self -> dad;
                return 0;
            }
        }

        * cloud = NULL;
    }

    return rc;
}

/* WithinAWS
 *  answers true if within AWS
 */
bool CloudMgrWithinAWS ( const CloudMgr * self )
{
    assert(self);
    char buffer[1024];
    return IMDS_version( self -> kns, buffer, sizeof buffer ) != 0;
}

/*** Finding/loading credentials  */

static rc_t aws_extract_key_value_pair (
    const String *source, String *key, String *val )
{
    String k, v;
    const char *start = source->addr;
    const char *end = start + source->size;

    char *eql = string_chr ( start, source->size, '=' );
    if ( eql == NULL )
        return RC (rcCloud, rcChar, rcSearching, rcFormat, rcInvalid );

    /* key */
    StringInit ( &k, start, eql - start, string_len ( start, eql - start ) );
    StringTrim ( &k, key );

    start = eql + 1;

    /* value */
    StringInit ( &v, start, end - start, string_len ( start, end - start ) );
    StringTrim ( &v, val );

    return 0;
}

static
rc_t aws_parse_csv_file(AWS * self, const char * start, size_t buf_size,
    const char * path)
{
    int i = 0;

    const char *comma = NULL;
    const char *end = start + buf_size;
    const char *sep = start;

    int idxID = -1, idxSecret = -1;

    String ID, Secret, string, id = {0}, secret = { 0 };
    CONST_STRING(&ID, "Access key ID");
    CONST_STRING(&Secret, "Secret access key");

    assert(self);

    /* Header line */
    {
        sep = string_chr(start, end - start, '\n');
        if (sep == NULL)
            sep = (char *)end;

        comma = string_chr(start, sep - start, ',');
        if (comma == NULL) {
            rc_t rc
                = RC(rcCloud, rcFile, rcReading, rcFile, rcUnrecognized);
            self->credentials_state = eCCUnrecognized;
            PLOGERR(klogErr, (klogErr, rc,
                "unrecognized format of credentials file '$(path)'",
                "path=%s", path));
            return rc;
        }

        for (i = 0; start < sep; ++i, start = comma + 1) {
            String key;

            comma = string_chr(start, sep - start, ',');
            if (comma == NULL)
                comma = (char *)sep;

            StringInit(&string,
                start, comma - start, string_len(start, comma - start));

            StringTrim(&string, &key);
            if (StringEqual(&key, &ID))
                idxID = i;
            else if (StringEqual(&key, &Secret))
                idxSecret = i;
        }

        if (idxID < 0 || idxSecret < 0) { /* incomplete header */
            rc_t rc
                = RC(rcCloud, rcFile, rcReading, rcFile, rcInsufficient);
            self->credentials_state = eCCIncomplete;
            PLOGERR(klogErr, (klogErr, rc,
                "credentials file '$(path)' has incomplete header",
                "path=%s", path));
            return rc;
        }
    }

    /* Data line */
    {
        bool idFound = false, secretFound = false;

        if (end > start) {
            sep = string_chr(start, end - start, '\n');
            if (sep == NULL)
                sep = (char *)end;
        } else
            sep = (char *)end;

        for (i = 0; start < sep; ++i, start = comma + 1) {
            comma = string_chr(start, sep - start, ',');
            if (comma == NULL)
                comma = (char *)sep;

            if (i != idxID && i != idxSecret)
                continue;

            StringInit(&string,
                start, comma - start, string_len(start, comma - start));

            if (i == idxID) {
                StringTrim(&string, &id);
                idFound = true;
            }
            else if (i == idxSecret) {
                StringTrim(&string, &secret);
                secretFound = true;
            }
        }

        if (!idFound || !secretFound) { /* no data */
            rc_t rc
                = RC(rcCloud, rcFile, rcReading, rcFile, rcInsufficient);
            self->credentials_state = eCCIncomplete;
            PLOGERR(klogErr, (klogErr, rc,
                "credentials file '$(path)' is incomplete", "path=%s", path));
            return rc;
        }
    }

    /* Check extra lines */
    {
        sep = string_chr(start, end - start, '\n');
        if (sep == NULL)
            sep = (char *)end;

        for (i = 0; start < sep; ++i, start = comma + 1) {
            comma = string_chr(start, sep - start, ',');
            if (comma != NULL) {
                rc_t rc
                    = RC(rcCloud, rcFile, rcReading, rcFile, rcExcessive);
                PLOGERR(klogWarn, (klogWarn, rc, "credentials file '$(path)' "
                                        "has extra lines", "path=%s", path));
            }
            break;
        }
    }

    free(self->access_key_id);
    self->access_key_id = string_dup(id.addr, id.size);

    free(self->secret_access_key);
    self->secret_access_key = string_dup(secret.addr, secret.size);

    return 0;
}

/*TODO: improve error handling (at least report) */
static void aws_parse_file ( AWS * self, const KFile * cred_file,
    const char * path )
{
    rc_t rc;
    size_t buf_size;
    size_t num_read;

    char * buffer;
    uint64_t file_size;

    assert ( self != NULL );
    assert ( self -> profile != NULL );

    rc = KFileSize ( cred_file, &file_size );
    if ( rc != 0 )
        return;

    buf_size = ( size_t ) file_size;
    if ( sizeof buf_size < sizeof file_size && ( uint64_t ) buf_size != file_size )
        return;

    if (file_size == 0) {
        rc = RC(rcCloud, rcFile, rcReading, rcFile, rcEmpty);
        self->credentials_state = eCCEmpty;
        PLOGERR(klogErr, (klogErr, rc,
            "credentials file '$(path)' is empty", "path=%s", path));
        return;
    }


    buffer = malloc ( buf_size );
    rc = KFileReadAll ( cred_file, 0, buffer, buf_size, &num_read );

    if ( rc != 0 )
    {
        free ( buffer );
    }
    else
    {
        String bracket;
        String profile;
        const String *temp1;
        const String *brack_profile;

        bool in_profile = false, in_profile_found = false;
        bool found = false;

        const char *start = buffer;
        const char *end = NULL;
        const char *sep = NULL;

        end = start + buf_size;

        if (buf_size > 3 &&
            start[0] == '\xEF' && start[1] == '\xBB' && start[2] == '\xBF')
        {   /* skip BOM */
            start += 3;
            buf_size -= 3;
        }

        end = start + buf_size;
        sep = start;

        CONST_STRING ( &bracket, "[" );

        StringInitCString( & profile, self -> profile );

        StringConcat ( &temp1, &bracket, &profile );
        CONST_STRING ( &bracket, "]" );
        StringConcat ( &brack_profile, temp1, &bracket );

        --sep;

        if (buf_size > 0 && *start != '[') {
            aws_parse_csv_file(self, start, buf_size, path);
            found = true; /* don't print error message - it was done already */
        }
        else
          for ( ; start < end; start = sep + 1 ) {
            rc_t rc2;
            String string, trim;
            String key, value;
            String access_key_id, secret_access_key;
            String region, output;

            sep = string_chr ( start, end - start, '\n' );
            if ( sep == NULL ) sep = (char *)end;

            StringInit (
                &string, start, sep - start, string_len ( start, sep - start ) );

            StringTrim ( &string, &trim );
            /* check for empty line and skip */
            if ( StringLength ( &trim ) == 0 ) continue;
            /*
                    {
                        char *p = string_dup ( trim.addr, StringLength ( &trim ) );
                        fprintf ( stderr, "line: %s\n", p );
                        free ( p );
                    }
            */
            /* check for comment line and skip */
            if ( trim.addr[0] == '#' ) continue;

            /* check for [profile] line */
            if ( trim.addr[0] == '[' ) {
                in_profile = StringEqual ( &trim, brack_profile );
                if (in_profile)
                    in_profile_found = true;
                continue;
            }

            if ( !in_profile ) continue;

            /* check for key/value pairs and skip if none found */
            rc2 = aws_extract_key_value_pair ( &trim, &key, &value );
            if ( rc2 != 0 ) continue;

            /* now check keys we are looking for and populate the node*/

            CONST_STRING ( &access_key_id, "aws_access_key_id" );
            CONST_STRING ( &secret_access_key, "aws_secret_access_key" );

            if ( StringCaseEqual ( &key, &access_key_id ) ) {
                free ( self -> access_key_id );
                self -> access_key_id = string_dup ( value . addr, value . size );
                found = true;
            }

            if ( StringCaseEqual ( &key, &secret_access_key ) ) {
                free ( self -> secret_access_key );
                self -> secret_access_key = string_dup ( value . addr, value . size );
                found = true;
            }

            CONST_STRING ( &region, "region" );
            CONST_STRING ( &output, "output" );

            if ( StringCaseEqual ( &key, &region ) ) {
                free ( self -> region );
                self -> region = string_dup ( value . addr, value . size );
            }
            if ( StringCaseEqual ( &key, &output ) ) {
                free ( self -> output );
                self -> output = string_dup ( value . addr, value . size );
            }
          }

        if (!found) {
            rc = RC(rcCloud, rcFile, rcReading, rcString, rcNotFound);
            if (!in_profile_found) {
                self->credentials_state = eCCProfileNotFound;
                PLOGERR(klogErr, (klogErr, rc,
                    "profile '$(p)' is not found in credentials file '$(path)'",
                    "p=%s,path=%s", profile.addr, path));
            }
            else {
                self->credentials_state = eCCProfileNotComplete;
                PLOGERR(klogErr, (klogErr, rc, "profile '$(p)' is not complete "
                    "in credentials file '$(path)'",
                    "p=%s,path=%s", profile.addr, path));
            }
        }

        StringWhack ( temp1 );
        StringWhack ( brack_profile );
        free ( buffer );
    }
}

static void make_home_node ( char *path, size_t path_size, KConfig * aKfg )
{
    size_t num_read;
    char *home = NULL;

    rc_t rc = 0;
    KConfig * kfg = aKfg;
    if (kfg == NULL)
        rc = KConfigMakeLocal(&kfg, NULL);
    if ( rc == 0 )
    {
        const KConfigNode *home_node;

        /* Check to see if home node exists */
        rc = KConfigOpenNodeRead ( kfg, &home_node, "HOME" );
        if ( home_node == NULL ) {
            /* just grab the HOME env variable */
#ifdef WINDOWS
            size_t buf_count = 0;
            errno_t err = _dupenv_s(&home, &buf_count, "HOME");
            if (!err && home != NULL)
            {
                if (*home != 0)
                {
                    num_read = string_copy_measure ( path, path_size, home );
                    if ( num_read >= path_size ) path[0] = 0;
                }
                free ( home );
            }
#else
            home = getenv ( "HOME" );
            if ( home != NULL && *home != 0 ) {
                num_read = string_copy_measure ( path, path_size, home );
                if ( num_read >= path_size ) path[0] = 0;
            }
#endif
        } else {
            /* if it exists check for a path */
            rc = KConfigNodeRead ( home_node, 0, path, path_size, &num_read, NULL );
            if ( rc != 0 ) {
#ifdef WINDOWS
                size_t buf_count = 0;
                errno_t err = _dupenv_s(&home, &buf_count, "HOME");
                if (!err && home != NULL)
                {
                    if (*home != 0)
                    {
                        num_read = string_copy_measure ( path, path_size, home );
                        if ( num_read >= path_size ) path[0] = 0;
                    }
                    free ( home );
                }
#else
                home = getenv ( "HOME" );
                if ( home != NULL && *home != 0 ) {
                    num_read = string_copy_measure ( path, path_size, home );
                    if ( num_read >= path_size ) path[0] = 0;
                }
#endif
            }

            KConfigNodeRelease ( home_node );
        }

        if (aKfg == NULL)
            KConfigRelease ( kfg );
    }
}

static rc_t LoadCredentials ( AWS * self, KConfig * kfg )
{
#ifdef WINDOWS
    char buf_conf_env[4096], buf_cred_env[4096];
    const char *conf_env = NULL;
    const char *cred_env = NULL;
    size_t buf_count = 0;
    errno_t err = getenv_s ( & buf_count, buf_conf_env, sizeof ( buf_conf_env ), "AWS_CONFIG_FILE" );
    assert ( err != ERANGE );
    assert ( buf_count <= sizeof(buf_conf_env) );
    if (!err)
        conf_env = buf_conf_env;

    buf_count = 0;
    err = getenv_s ( & buf_count, buf_cred_env, sizeof ( buf_cred_env ), "AWS_SHARED_CREDENTIAL_FILE" );
    assert ( err != ERANGE );
    assert ( buf_count <= sizeof(buf_cred_env) );
    if (!err)
        cred_env = buf_cred_env;
#else
    const char *conf_env = getenv ( "AWS_CONFIG_FILE" );
    const char *cred_env = getenv ( "AWS_SHARED_CREDENTIAL_FILE" );
#endif
    KDirectory *wd = NULL;
    rc_t rc = KDirectoryNativeDir ( &wd );
    if ( rc ) return rc;

    if ( conf_env && *conf_env != 0 )
    {
        const KFile *cred_file = NULL;
        DBGMSG(DBG_CLOUD, DBG_FLAG(DBG_CLOUD_LOAD),
            ("Got AWS_CONFIG_FILE '%s' from environment\n", conf_env));
        rc = KDirectoryOpenFileRead ( wd, &cred_file, "%s", conf_env );
        if ( rc == 0 )
        {
            aws_parse_file ( self, cred_file, conf_env );
            KFileRelease ( cred_file );
        }
        KDirectoryRelease ( wd );
        return rc;
    }

    if ( cred_env && *cred_env != 0 )
    {
        const KFile *cred_file = NULL;
        PLOGMSG ( klogInfo, ( klogInfo,
                    "Got AWS_SHARED_CREDENTIAL_FILE '$(F)' from environment",
                    "F=%s", cred_env ) );
        rc = KDirectoryOpenFileRead ( wd, &cred_file, "%s", cred_env );
        if ( rc == 0 )
        {
            aws_parse_file ( self, cred_file, cred_env );
            KFileRelease ( cred_file );
        }
        KDirectoryRelease ( wd );
        return rc;
    }

    {
        char home[4096] = "";
        make_home_node ( home, sizeof home, kfg );

        if ( home[0] != 0 )
        {
            char aws_path[4096] = "";
            size_t num_writ = 0;
            rc = string_printf (aws_path, sizeof aws_path, &num_writ, "%s/.aws",
                home );
            if ( rc == 0 && num_writ != 0 )
            {
                const KFile *cred_file = NULL;
                rc = KDirectoryOpenFileRead ( wd, &cred_file, "%s%s", aws_path, "/credentials" );

                if (rc == 0)
                    PLOGMSG ( klogInfo, ( klogInfo,
                                              "Loading AWS credentials from '$(P)/credentials'",
                                              "P=%s", aws_path ) );
                else {
                    KConfig * cfg = kfg;
                    rc = 0;
                    if (cfg == NULL)
                        rc = KConfigMake(&cfg, NULL);

                    if (rc == 0)
                        rc = KConfig_Get_Aws_Credential_File(
                            cfg, aws_path, sizeof aws_path, NULL);

                    if (rc == 0) {
                        PLOGMSG ( klogInfo, ( klogInfo,
                                        "Loading AWS credential file '$(F)' from configuration",
                                        "F=%s", aws_path ) );
                        rc = KDirectoryOpenFileRead(
                            wd, &cred_file, "%s", aws_path);
                    }

                    if (kfg == NULL) {
                        rc_t r2 = KConfigRelease(cfg);
                        if (rc == 0 && r2 != 0)
                            rc = r2;
                    }
                }

                if ( rc == 0 )
                {
                    aws_parse_file ( self, cred_file, aws_path );
                    KFileRelease ( cred_file );

                    rc = KDirectoryOpenFileRead ( wd, &cred_file, "%s%s", aws_path, "/config" );
                    if ( rc == 0 )
                    {
                        PLOGMSG ( klogInfo, ( klogInfo,
                                              "Loading AWS credentials from '$(P)/config'",
                                              "P=%s", aws_path ) );
                        aws_parse_file ( self, cred_file, aws_path );
                        KFileRelease ( cred_file );
                    }
                }
            }
        }
    }

    KDirectoryRelease ( wd );
    return rc;
}

//TODO: check results of strdups and string_dups
static
rc_t PopulateCredentials ( AWS * self, KConfig * aKfg )
{
    /* Check Environment first */
    char * profile = NULL;

#ifdef WINDOWS
    char buf_aws_access_key_id[4096], buf_aws_secret_access_key[4096];
    const char *aws_access_key_id = NULL;
    const char *aws_secret_access_key = NULL;
    size_t buf_count = 0;
    errno_t err = getenv_s ( & buf_count, buf_aws_access_key_id, sizeof (buf_aws_access_key_id), "AWS_ACCESS_KEY_ID" );
    assert ( err != ERANGE );
    assert ( buf_count <= sizeof(buf_aws_access_key_id) );
    if (!err)
        aws_access_key_id = buf_aws_access_key_id;

    buf_count = 0;
    err = getenv_s ( & buf_count, buf_aws_secret_access_key, sizeof (buf_aws_secret_access_key), "AWS_SECRET_ACCESS_KEY" );
    assert ( err != ERANGE );
    assert ( buf_count <= sizeof(buf_aws_secret_access_key) );
    if (!err)
        aws_secret_access_key = buf_aws_secret_access_key;
#else
    const char * aws_access_key_id = getenv ( "AWS_ACCESS_KEY_ID" );
    const char * aws_secret_access_key = getenv ( "AWS_SECRET_ACCESS_KEY" );
#endif

    if ( aws_access_key_id != NULL && aws_secret_access_key != NULL
        && strlen ( aws_access_key_id ) > 0
        && strlen ( aws_secret_access_key ) > 0 )
    {   /* Use environment variables */
        self -> access_key_id = string_dup( aws_access_key_id, string_size( aws_access_key_id ) );
        self -> secret_access_key = string_dup( aws_secret_access_key, string_size( aws_secret_access_key ) );
        DBGMSG(DBG_CLOUD, DBG_FLAG(DBG_CLOUD_LOAD), ("Got "
            "AWS_ACCESS_KEY_ID and AWS_SECRET_ACCESS_KEY from environment\n"));
        return 0;
    }

    /* Get Profile */
#ifdef WINDOWS
    buf_count = 0;
    err = _dupenv_s(&profile, &buf_count, "AWS_PROFILE");
#else
    profile = getenv("AWS_PROFILE");
#endif
    if ( profile != NULL && *profile != 0 )
    {
        self -> profile = string_dup ( profile, string_size ( profile ) );
        PLOGMSG ( klogInfo, ( klogInfo, "Got AWS_PROFILE '$(P)' from environment",
                                        "P=%s", self->profile ) );
#ifdef WINDOWS
        free(profile);
        profile = NULL;
#endif
    }
    else
    {
        KConfig * kfg = aKfg;
        rc_t rc = 0;
        if (kfg == NULL)
            rc = KConfigMakeLocal(&kfg, NULL);
/*KConfigPrint(kfg,0);*/
        if ( rc == 0 )
        {
            char buffer[4096];
            size_t num_writ = 0;
            rc = KConfig_Get_Aws_Profile ( kfg, buffer, sizeof ( buffer ), &num_writ );
            if ( rc == 0 && num_writ > 0 )
            {
                self -> profile = string_dup ( buffer, string_size ( buffer ) );
                PLOGMSG ( klogInfo, ( klogInfo,
                                           "Got AWS profile '$(P)' from configuration",
                                           "P=%s", self->profile ) );
            }
            if (aKfg == NULL)
                KConfigRelease(kfg);
        }
    }

    if ( self -> profile == NULL )
    {
        self -> profile = string_dup_measure ( "default", NULL );
        DBGMSG(DBG_CLOUD, DBG_FLAG(DBG_CLOUD_LOAD),
            ("Set '%s' AWS_PROFILE\n", self->profile));
    }

    /* OK if no credentials are found */
    LoadCredentials ( self, aKfg );
    return 0;
}

#if 0

    /* Check AWS_CONFIG_FILE and AWS_SHARED_CREDENTIAL_FILE, if specified check for credentials and/or profile name */
    const char *conf_env = getenv ( "AWS_CONFIG_FILE" );
    if ( conf_env == NULL || *conf_env == 0 )
    {
        conf_env = getenv ( "AWS_SHARED_CREDENTIAL_FILE" );
    }
    if ( conf_env && *conf_env != 0 )
    {
        KDirectory *wd = NULL;
        const KFile *cred_file = NULL;

        rc = KDirectoryNativeDir ( &wd );
        if ( rc != 0 )
        {
            return rc;
        }

        rc = KDirectoryOpenFileRead ( wd, &cred_file, "%s", conf_env );
        if ( rc == 0 )
        {
            aws_parse_file ( self, cred_file );
            KFileRelease ( cred_file );
        }
        KDirectoryRelease ( wd );
        if ( rc != 0 )
        {
            return rc;
        }
        if ( self -> access_key_id != NULL && self -> secret_access_key != NULL )
        {
            return 0;
        }

        /* proceed to other sources */
    }

    /* Check paths and parse */
    char home[4096] = "";
    make_home_node ( self, home, sizeof home );

    if ( home[0] != 0 ) {
        char path[4096] = "";
        size_t num_writ = 0;
        rc = string_printf ( path, sizeof path, &num_writ, "%s/.aws", home );
        if ( rc == 0 && num_writ != 0 ) {
            /* Use config files */
            if ( rc == 0 ) {
                rc = aws_find_nodes ( aws_node, path, &sprofile );
                if ( rc ) {
                    /* OK if no .aws available */
                    rc = 0;
                }
            }
        }
    }

    KConfigNodeRelease ( aws_node );
    return rc;
}
#endif

LIB_EXPORT rc_t CC CloudToAWS ( const Cloud * self, AWS ** aws )
{
    rc_t rc;

    if ( aws == NULL )
        rc = RC ( rcCloud, rcProvider, rcCasting, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = 0;
        else if ( self -> vt != ( const Cloud_vt * ) & AWS_vt_v1 )
            rc = RC ( rcCloud, rcProvider, rcCasting, rcType, rcIncorrect );
        else
        {
            rc = CloudAddRef ( self );
            if ( rc == 0 )
            {
                * aws = ( AWS * ) self;
                return 0;
            }
        }

        * aws = NULL;
    }

    return rc;
}
