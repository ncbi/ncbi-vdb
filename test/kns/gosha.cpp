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
*  An example of calling ncbi-vdb API
*  to establishing an HTTP connection using mutual authentication.
* =========================================================================== */

#include <cmath>

#include <klib/out.h> // OUTMSG
#include <klib/text.h> // String
#include <kns/manager.h> // KNSManager
#include <kns/http.h> // KClientHttp
#include <kns/stream.h> // KStream

rc_t MutualConnection(const char * own_cert, const char * pk_key,
    const char * url, const char * aHost, uint32_t port)
{
    KNSManager * mgr = NULL;
    rc_t rc = KNSManagerMake(&mgr);
    if (rc != 0)
        return rc;

    if (own_cert != NULL)
        rc = KNSManagerSetOwnCert(mgr, own_cert, pk_key);

    if (rc == 0) {
        ver_t vers = 0x01010000;
        KHttp * http = NULL;
        String host;
        StringInitCString(&host, aHost);
        rc = KNSManagerMakeClientHttps(mgr, &http, NULL, vers, &host, port);
        /*   KNSManagerMakeClientHttp */

        if (rc == 0) {
            KHttpRequest * req = NULL;
            rc = KHttpMakeRequest(http, &req, "%s", url);

            if (rc == 0) {
                rc = KHttpRequestAddHeader(req,
                    "Content-Type", "application/x-www-form-urlencoded");
                if (rc == 0)
                    rc = KHttpRequestAddHeader(req,
                        "Accept", "application/json");

                if (rc == 0) {
                    KHttpResult * rslt = NULL;
                    rc = KHttpRequestPOST(req, &rslt);

                    if (rc == 0) {
                        uint32_t code = 0;
                        rc = KHttpResultStatus(rslt, &code, NULL, 0, NULL);

                        if (rc == 0) {
                            KStream * s = NULL;
                            rc = KClientHttpResultGetInputStream(rslt, &s);
                            if (rc == 0) {
                                char buf[4096];
                                size_t num_read = 0;

                                rc = KStreamRead(s, buf, sizeof buf, &num_read);
                                if (rc == 0 && num_read < sizeof buf)
                                    buf[num_read] = '\0';

                                if (rc == 0)
                                    OUTMSG(("%.*s\n", num_read, buf));

                                KStreamRelease(s);
                            }
                        }
                        KHttpResultRelease(rslt);
                    }
                }
                KHttpRequestRelease(req);
            }

            KHttpRelease(http);
        }
    }

    KNSManagerRelease(mgr);
    return rc;
}

rc_t example() {
    const char own_cert[] =
        "-----BEGIN CERTIFICATE-----\n"
        "MIICoDCCAYgCCQCkJAxHivQFkjANBgkqhkiG9w0BAQsFADAQMQ4wDAYDVQQDDAVt\n"
        "...\n"
        "CjWtgpId/KZFQ4A+1IIcW7l6gGJxLNosraYzRb7ZvLF+SmGdtueGumVw26fg+KLQ\n"
        "CwfO9w==\n"
        "-----END CERTIFICATE-----";
    const char pk_key[] =
        "-----BEGIN RSA PRIVATE KEY-----\n"
        "MIIEpAIBAAKCAQEAv/4qWUktt3XT3/1pLzJQSrPcKmvDJvgWj1Mvg+0uM6p1sIcT\n"
        "...\n"
        "vXxJFyQy3SbsXCJuiHlUkPqsDdf4dhw86D8UIFDpIXG9QMbupTOEH2biOOJAVLTC\n"
        "3fGbbJM8ANujqkqnOasmkDEEj6aF3tzbC5vuzXjDoGlNRUFDAHxgLg==\n"
        "-----END RSA PRIVATE KEY-----";

    const char url[] = " https://localhost:3000?access_token=access_token";
    const char host[] =         "localhost";
    uint32_t port =                        3000;

    rc_t rc = MutualConnection(own_cert, pk_key, url, host, port);
    return rc;
}
