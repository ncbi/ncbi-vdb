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
* ==============================================================================
*
*/

#ifndef _h_stable_http_file_priv_
#define _h_stable_http_file_priv_

#define KFILE_IMPL KStableHttpFile
typedef struct KStableHttpFile KStableHttpFile;
#include <kfs/impl.h> /* KFile_v1 */

#include <klib/data-buffer.h>

#include <kns/manager.h> /* quitting_t */

/*--------------------------------------------------------------------------
 * KStableHttpFile: retry wrapper on KHttpFile
 */

/* retrier states */
typedef enum {
    eRSJustRetry, /* just retry read */
    eRSReopen,    /* reopen the underlying file */
    eRSIncTO,     /* increase sleep between calls to retry */
    eRSMax,
} ERetryState;

struct KStableHttpFile
{
    KFile dad;

    /************************************************************************/
    /* readWaitMillis and totalReadWaitMillis HAVE TO BE FIRST RIGHT AFTER dad
    TO BE ABLE TO GET IT FROM BOTH KHttpFile AND KStableHttpFile:
    SEE stable-http-file.c : HttpFileGetReadTimeouts() */
    int32_t readWaitMillis;
    int32_t totalReadWaitMillis;
    /************************************************************************/

    const struct KFile * file; /* underlying file */

    int32_t totalConnectWaitMillis;

    /* arguments to reopen the underlying file */
    const struct KNSManager * mgr;
    struct KStream * conn;
    ver_t vers;
    bool reliable;
    bool need_env_token;
    bool payRequired;
    char * url;
    KDataBuffer buf;

    bool live; /* there was a successfull read on the file */

    /* retrier properties */
    bool _failed;       /* KStableHttpFile is in failed state */
    KTime_t _tFailed;   /* timestamp when retry loop started */
    ERetryState _state; /* retrier state */
    uint32_t _sleepTO;

    quitting_t quitting;
};

#endif
