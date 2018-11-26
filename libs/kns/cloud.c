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

#include <kfs/directory.h> /* KDirectoryRelease */

#include <klib/printf.h> /* string_printf */
#include <klib/rc.h> /* RC */

#include <kns/http.h> /* KClientHttpRequestRelease */
#include <kns/stream.h> /* KStreamRelease */

#include "mgr-priv.h" /* KNSManagerSetLogNcbiVdbNetError */
#include "cloud.h" /* KNSManagerMakeCloud */

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)

typedef struct Cloud {
    char * location;
} Cloud;

static bool _KDirectory_FileExists(const KDirectory * self,
    const char * path)
{
    return KDirectoryPathType_v1(self, path) != kptNotFound;
}

rc_t CloudRelease(const Cloud * cself) {
    Cloud * self = (Cloud*)cself;

    if (self != NULL) {
        free(self->location);

        memset(self, 0, sizeof * self);

        free(self);
    }

    return 0;
}

static rc_t CloudMakeEmpty(Cloud ** self) {
    *self = calloc(1, sizeof **self);

    if (*self == NULL)
        return RC(rcNS, rcStorage, rcAllocating, rcMemory, rcExhausted);
    else
        return 0;
}

static rc_t CloudMake(const Cloud ** self,
    bool gs, const char * zone, size_t zsize)
{
    size_t size = 2 + 1 + zsize;

    const char* slash = NULL;

    Cloud * p = NULL;

    rc_t rc = CloudMakeEmpty(&p);
    if (rc != 0)
        return rc;

    slash = string_rchr(zone, zsize, '/');
    if (slash != NULL && slash - zone < zsize) {
        size -= slash - zone + 1;
        zone = slash + 1;
    }

    p->location = calloc(1, size);
    if (p->location == NULL)
        rc = RC(rcNS, rcStorage, rcAllocating, rcMemory, rcExhausted);

    if (rc == 0)
        rc = string_printf(p->location, size, NULL,
            "%s.%s", gs ? "gs" : "s3", zone);

    assert(self);
    if (rc == 0)
        *self = p;
    else
        CloudRelease(p);

    return rc;
}

#define GS "http://metadata.google.internal/computeMetadata/v1/instance/zone"
#define S3 "http://169.254.169.254/latest/meta-data/placement/availability-zone"

static rc_t _KNSManager_Read(struct KNSManager * self,
    bool gs, const Cloud ** cloud)
{
    rc_t rc = 0;

    const char * url = gs ? GS : S3;

    KClientHttpRequest *req = NULL;

    int32_t timeout = 1; /* milliseconds */
    int32_t msec = self->conn_timeout;

    /* avoid connection retry loop in KNSManagerMakeRetryTimedConnection */
    self->conn_timeout = timeout;

    rc = KNSManagerMakeClientRequest(self, &req, 0x01010000, NULL, url);

    /* restore connection timeout in KNSManager */
    self->conn_timeout = msec;

    if (rc != 0)
        return rc;
    else {
        if (gs)
            rc = KClientHttpRequestAddHeader(req, "Metadata-Flavor", "Google");

        if (rc == 0) {
            KClientHttpResult * rslt = NULL;
            rc = KClientHttpRequestGET(req, &rslt);
            if (rc == 0) {
                KStream * s = NULL;
                rc = KClientHttpResultGetInputStream(rslt, &s);
                if (rc == 0) {
                    char buffer[99] = "";
                    size_t num_read = 0;
                    rc = KStreamRead(s, buffer, sizeof buffer, &num_read);
                    if (rc == 0) {
                        if (num_read == sizeof buffer)
                            --num_read;
                        buffer[num_read++] = '\0';

                        if (rc == 0)
                            rc = CloudMake(cloud, gs, buffer, num_read);
                    }
                }
                RELEASE(KStream, s);
            }
            RELEASE(KClientHttpResult, rslt);
        }
    }

    RELEASE(KClientHttpRequest, req);

    return rc;
}

rc_t KNSManagerMakeCloud(struct KNSManager * self,
    const Cloud ** cloud)
{
    KDirectory * dir = NULL;

    rc_t rc = KDirectoryNativeDir(&dir);
    if (rc == 0) {
        bool gcsFirst = true;

        bool log = KNSManagerLogNcbiVdbNetError(self);

        if (_KDirectory_FileExists(dir, "/usr/bin/gcloud"))
            gcsFirst = true;
        else if (_KDirectory_FileExists(dir, "/usr/bin/ec2-metadata"))
            gcsFirst = false;

        if (log)
            KNSManagerSetLogNcbiVdbNetError(self, false);

        rc = _KNSManager_Read(self, gcsFirst, cloud);

        if (rc != 0)
            rc = _KNSManager_Read(self, !gcsFirst, cloud);

        if (rc != 0)
            rc = CloudMakeEmpty((Cloud **)cloud);

        if (log)
            KNSManagerSetLogNcbiVdbNetError(self, true);
    }

    RELEASE(KDirectory, dir);

    return rc;
}

const char * CloudGetLocation(const struct Cloud * self) {
    if (self != NULL)
        return self->location;
    else
        return NULL;
}

/******************************************************************************/
