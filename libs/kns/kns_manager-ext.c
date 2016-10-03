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

#include <kns/extern.h>

#include <kns/manager-ext.h>
#include <kns/http.h>
#include <kns/stream.h>

#include <klib/sra-release-version.h>
#include <klib/data-buffer.h>
#include <klib/debug.h>
#include <klib/rc.h>

#include <ctype.h>
#include <string.h>
#include <assert.h>


/** These functions belong to Extended KNSManager **/


#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)

static rc_t CC KNSManagerNewReleaseVersionImpl(const struct KNSManager *self,
    SraReleaseVersion *newVersion)
{
    rc_t rc = 0;
    KDataBuffer result;
    KHttpRequest *req = NULL;
    KHttpResult *rslt = NULL;
    if (newVersion == NULL) {
        return RC(rcNS, rcArgv, rcAccessing, rcParam, rcNull);
    }
    memset(newVersion, 0, sizeof *newVersion);
    if (self == NULL) {
        return RC(rcNS, rcArgv, rcAccessing, rcSelf, rcNull);
    }
    memset(&result, 0, sizeof result);
    if (rc == 0) {
        rc = KNSManagerMakeRequest(self, &req, 0x01010000, NULL,
  "https://ftp-trace.ncbi.nlm.nih.gov/sra/sdk/current/sratoolkit.current.version"
        );
    }
    if (rc == 0) {
        rc = KHttpRequestGET(req, &rslt);
    }
    if (rc == 0) {
        uint32_t code = 0;
        rc = KHttpResultStatus(rslt, &code, NULL, 0, NULL);
        if (rc == 0) {
            if (code != 200) {
                rc = RC(rcNS, rcFile, rcReading, rcFile, rcInvalid);
            }
        }
    }
    if (rc == 0) {
        size_t total = 0;
        KStream *response = NULL;
        rc = KHttpResultGetInputStream(rslt, &response);
        if (rc == 0) {
            rc = KDataBufferMakeBytes(&result, 1024);
        }
        while (rc == 0) {
            size_t num_read = 0;
            uint8_t *base = NULL;
            uint64_t avail = result.elem_count - total;
            if (avail < 256) {
                rc = KDataBufferResize(&result, result.elem_count + 1024);
                if (rc != 0) {
                    break;
                }
            }
            base = result.base;
            rc = KStreamRead(response, &base[total], result.elem_count - total,
                &num_read);
            if (num_read > 0 || rc != 0) {
                DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS), ("KStreamRead"
                    "(sratoolkit.current.version, %zu) = %R\n", num_read, rc));
            }
            if (rc != 0) {
                /* TBD - look more closely at rc */
                if (num_read > 0) {
                    rc = 0;
                }
                else {
                    break;
                }
            }
            if (num_read == 0) {
                break;
            }
            total += num_read;
        }
        RELEASE(KStream, response);
        if (rc == 0) {
            DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS),
                ("sratoolkit.current.version (%zu)\n", total));
            result.elem_count = total;
        }
    }

    if (rc == 0) {
        const char *start = (const void*)(result.base);
        size_t size = KDataBufferBytes(&result);
        DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS),
            ("sratoolkit.current.version = '%.*s'\n", (uint32_t)size, start));
        rc = SraReleaseVersionInit(newVersion, start, size);
    }

    KDataBufferWhack(&result);
    RELEASE(KHttpResult, rslt);
    RELEASE(KHttpRequest, req);

    return rc;
}

LIB_EXPORT rc_t CC KNSManagerNewReleaseVersion(const struct KNSManager *self,
    SraReleaseVersion *newVersion)
{
    rc_t rc = 0;
    int i = 0, retryOnFailure = 2;
    for (i = 0; i < retryOnFailure; ++i) {
        rc = KNSManagerNewReleaseVersionImpl(self, newVersion);
        if (rc == 0) {
            break;
        }
        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_ERR), (
            "@@@@@@@@2: KNSManagerNewReleaseVersion %d/%d = %R"
            "\n", i + 1, retryOnFailure, rc));
    }
    return rc;
}
