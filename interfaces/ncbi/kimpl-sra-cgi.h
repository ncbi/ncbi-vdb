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
 * =============================================================================
 *
 */

#ifndef _h_ncbi_impl_sra_cgi_
#define _h_ncbi_impl_sra_cgi_

#include <stdint.h> // uint32_t
#include <unistd.h> // ssize_t

#ifdef __cplusplus
extern "C" {
#endif


typedef union SraCgiRequest_vt SraCgiRequest_vt;
struct SraCgiRequest {
    int err;
    const SraCgiRequest_vt *vt;
};
typedef struct SraCgiRequest SraCgiRequest;

#ifndef SRA_CGI_REQUEST
#define SRA_CGI_REQUEST SraCgiRequest
#endif

typedef struct SraCgiRequest_vt_v1 SraCgiRequest_vt_v1;
struct SraCgiRequest_vt_v1 {
    uint32_t maj, min;
    ssize_t (*read) (SRA_CGI_REQUEST *self,
        void *buffer, size_t buffer_size);
};
union SraCgiRequest_vt {
    SraCgiRequest_vt_v1 v1;
};
int SraCgiRequestInit(SraCgiRequest *self, const SraCgiRequest_vt *vt);


typedef union SraCgiResponse_vt SraCgiResponse_vt;
struct SraCgiResponse {
    int err;
    const SraCgiResponse_vt *vt;
};
typedef struct SraCgiResponse SraCgiResponse;
#ifndef SRA_CGI_RESPONSE
#define SRA_CGI_RESPONSE SraCgiResponse
#endif

typedef struct SraCgiResponse_vt_v1 SraCgiResponse_vt_v1;
struct SraCgiResponse_vt_v1 {
    uint32_t maj, min;
    void (*status) (SRA_CGI_RESPONSE *self, unsigned int code);
    void (*header) (SRA_CGI_RESPONSE *self,
        const char *name, const void *buffer, size_t bytes);
    ssize_t (*write) (SRA_CGI_RESPONSE *self,
        const void *buffer, size_t bytes);
};

union SraCgiResponse_vt {
    SraCgiResponse_vt_v1 v1;
};
int SraCgiResponseInit(SraCgiResponse *self, const SraCgiResponse_vt *vt);

#ifdef __cplusplus
}
#endif

#endif /* _h_ncbi_impl_sra_cgi_ */

