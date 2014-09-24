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

#ifndef _h_ncbi_sra_cgi_
#define _h_ncbi_sra_cgi_

#include <stdint.h> /* uint32_t */
#include <unistd.h> /* ssize_t */

#ifdef __cplusplus
extern "C" {
#endif


/*------------------------------------------------------------------------------
 * SraCgiRequest
 */

struct SraCgiRequest;

/* Read
 *  Read() attempts to read up to buffer_size bytes
 *  from CGI request into the buffer.
 *
 * Return value:
 *  On success, the number of bytes read is returned (zero indicates end of
 *  file).
 *
 *  On error, negative value is returned
 *  and SraCgiRequest->err is set appropriately.
 */
ssize_t SraCgiRequestRead(struct SraCgiRequest *self,
    void *buffer, size_t buffer_size);


/*------------------------------------------------------------------------------
 * SraCgiResponse
 */

struct SraCgiResponse;

/* Status
 *
 * Set HTTP Response Status
 */
void SraCgiResponseStatus(struct SraCgiResponse *self, unsigned int code);


/* Header
 *
 * Set an HTTP Response Header
 *
 * "name" - header name
 * "buffer" contains the header string value
 * "bytes" - its size
 *
 * Return value:
 *  On success, the number of header bytes written are returned.
 *
 *  On error, negative value is returned
 *  and SraCgiResponse->err is set appropriately.
 */
ssize_t SraCgiResponseHeader(struct SraCgiResponse *self,
    const char *name, const void *buffer, size_t bytes);


/* ALL HEADERS SHOULD BE SET BEFORE CALLING WRITE() */


/* Write
 *  Write() writes up to count bytes to CGI output stream from the buffer.
 *
 * Return value:
 *  On success, the number of bytes written are returned (zero indicates nothing
 *  was written).
 *
 *  On error, negative value is returned
 *  and SraCgiResponse->err is set appropriately.
 */
ssize_t SraCgiResponseWrite(struct SraCgiResponse *self,
    const void *buffer, size_t bytes);


#ifdef __cplusplus
}
#endif

#endif /* _h_ncbi_sra_cgi_ */
