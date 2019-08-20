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

#include <klib/rc.h>

#include <kns/http.h> /* KClientHttpRequestRelease */
#include <kns/stream.h> /* KStreamRelease */

#include "cloud-cmn.h" /* KNSManager_Read */

#include "../kns/mgr-priv.h" /* KNSManager */

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 && !rc) { rc = rc2; } obj = NULL; } while (false)

/******************************************************************************/

rc_t KNSManager_Read(const struct KNSManager *cself, char *buffer, size_t bsize,
    const char *url, const char * hdrName, const char * hdrValue)
{
    rc_t rc = 0;

    struct KNSManager * self = (struct KNSManager *) cself;

    KClientHttpRequest *req = NULL;

    int32_t cmsec;
    int32_t wmsec;

    assert(self);

    /* save existing timeouts */
    cmsec = self->conn_timeout;
    wmsec = self->http_write_timeout;

    /* minimize timeouts to check cloudy URLs */
    self->conn_timeout = self->http_write_timeout = 500;

    rc = KNSManagerMakeRequest(self, &req, 0x01010000, NULL, url);

    if (rc == 0 && hdrName != NULL)
        rc = KClientHttpRequestAddHeader(req, hdrName, hdrValue);

    if (rc == 0) {
        KClientHttpResult * rslt = NULL;
        rc = KClientHttpRequestGET(req, &rslt);

        if (rc == 0) {
            KStream * s = NULL;
            rc = KClientHttpResultGetInputStream(rslt, &s);
            if (rc == 0) {
                size_t num_read = 0;
                rc = KStreamRead(s, buffer, bsize, &num_read);
                if (rc == 0) {
                    if (num_read == bsize)
                        rc = RC(rcCloud,
                            rcUri, rcReading, rcBuffer, rcInsufficient);
                    else
                        buffer[num_read++] = '\0';
                }
            }
            RELEASE(KStream, s);
        }

        RELEASE(KClientHttpResult, rslt);
    }

    /* restore timeouts in KNSManager */
    self->conn_timeout = cmsec;
    self->http_write_timeout = wmsec;

    RELEASE(KClientHttpRequest, req);

    return rc;
}
