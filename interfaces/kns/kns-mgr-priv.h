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
#ifndef _h_kns_mgr_priv_
#define _h_kns_mgr_priv_

#ifdef __cplusplus
extern "C" {
#endif

struct KStream;
struct KHttpFile;


/************************** HTTP-retry-related stuff **************************/

/** MakeReliableHttpFile:
 * Make HTTP file from a reliable URL:
 * we will try harder to recover upon any error
 * (make more retries)
 */
KNS_EXTERN rc_t CC KNSManagerMakeReliableHttpFile(
    struct KNSManager const *self, struct KFile const **file,
    struct KStream *conn, ver_t vers, const char *url, ...);


rc_t KNSManagerSetTriesForReliables(struct KNSManager *self, uint32_t tries);

uint32_t KNSManagerGetNumberOfRetriesOnFailure(const struct KNSManager *self);
uint32_t KNSManagerGetNumberOfRetriesOnFailureForReliableURLs
    (const struct KNSManager *self);
uint32_t KNSManagerGetLogFailuresNumber (const struct KNSManager *self);
uint32_t KNSManagerGetTestFailuresNumber(const struct KNSManager *self);


typedef struct {
    uint32_t triesNumber;
    uint32_t maxRetriesNumber;
    uint32_t testFailuresNumber;
    uint32_t logNumber; /* do not log if triesNumber < logNumber */
    bool logged; /* log just once */
    const char *url;
    uint32_t waitTime;
    uint32_t reportedTime;
} KHttpRetrier;

void KHttpRetrierInit(KHttpRetrier *self, uint32_t maxRetryNumber,
    uint32_t testFailuresNumber, const char *url, uint32_t logNumber);
bool KHttpRetrierWait(KHttpRetrier *self, rc_t rc);
rc_t KHttpRetrierForceFailure(const KHttpRetrier *self,
    const struct KHttpFile *socket);


#ifdef __cplusplus
}
#endif

#endif /* _h_kns_mgr_priv_ */
