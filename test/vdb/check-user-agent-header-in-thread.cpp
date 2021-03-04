/*  $Id: bam_test.cpp 621765 2020-12-16 19:23:32Z vasilche $
 * ===========================================================================
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
 * Authors:  Eugene Vasilchenko
 *
 * File Description:
 *   Test application for User-Agent setting
 *
 */

#include <iostream>
#include <vector>
#include <klib/rc.h>
#include <klib/out.h>
#include <klib/log.h>
#include <klib/debug.h>
#include <kns/manager.h>
#include <vfs/manager.h>
#include <vfs/path.h>
#include <kfs/file.h>
#include <pthread.h>
#ifdef _MSC_VER
# include <io.h>
#else
# include <unistd.h>
#endif

using namespace std;

#define CALL(call) CheckRc((call), #call, __FILE__, __LINE__)

void CheckRc(rc_t rc, const char* code, const char* file, int line)
{
    if ( rc ) {
        char buffer1[4096];
        size_t error_len;
        RCExplain(rc, buffer1, sizeof(buffer1), &error_len);
        char buffer2[8192];
        unsigned len = sprintf(buffer2, "%s:%d: %s failed: %#x: %s\n",
                             file, line, code, rc, buffer1);
        write(2, buffer2, len);
        exit(1);
    }
}

rc_t x_KFileRead(const KFile* file, uint64_t pos, char* buffer, size_t size, size_t* nread)
{
    *nread = 0;
    while ( size ) {
        size_t nread1;
        rc_t rc = KFileRead(file, pos, buffer, size, &nread1);
        if ( rc ) {
            return rc;
        }
        buffer += nread1;
        size -= nread1;
        *nread += nread1;
    }
    return 0;
}

const char* kFileName[] = {
    "http://ftp.ncbi.nlm.nih.gov/toolbox/gbench/samples/udc_seqgraphic_rmt_testing/remote_BAM_remap_UUD-324/human/grch38_wgsim_gb_accs.bam.bai",
    "http://ftp.ncbi.nlm.nih.gov/toolbox/gbench/samples/udc_seqgraphic_rmt_testing/remote_BAM_remap_UUD-324/human/grch38_wgsim_rs_accs.bam.bai",
    "http://ftp.ncbi.nlm.nih.gov/toolbox/gbench/samples/udc_seqgraphic_rmt_testing/remote_BAM_remap_UUD-324/human/grch38_wgsim_short.bam.bai",
    "http://ftp.ncbi.nlm.nih.gov/toolbox/gbench/samples/udc_seqgraphic_rmt_testing/remote_BAM_remap_UUD-324/human/grch38_wgsim_mixed.bam.bai",
};
const size_t kFileCount = sizeof(kFileName)/sizeof(kFileName[0]);
const size_t kReadSize = 1000000;

void x_Init()
{
    CALL(KWrtInit("test", 0));
    CALL(KOutHandlerSetStdOut());
    CALL(KDbgHandlerSetStdOut());
    CALL(KLogHandlerSetStdOut());
    CALL(KLogLibHandlerSetStdOut());
    CALL(KDbgSetString("KNS"));
}

void x_Read(const char* file_name, size_t read_id)
{
    // prepare ids
    const char* session_id = strrchr(file_name, '/')+1;
    char client_ip[] = "1.2.3.1"; client_ip[6] += read_id;
    const char* page_hit_id = session_id + 1;
    // set ids
    KNSManager* kns = 0;
    CALL(KNSManagerMake(&kns));
    CALL(KNSManagerSetSessionID(kns, session_id));
    CALL(KNSManagerSetClientIP(kns, client_ip));
    CALL(KNSManagerSetPageHitID(kns, page_hit_id));
    CALL(KNSManagerRelease(kns)); kns = 0;

    // read file
    VFSManager* vfs = 0;
    CALL(VFSManagerMake(&vfs));
    VPath* path = 0;
    CALL(VFSManagerMakePath(vfs, &path, file_name));
    const KFile* file = 0;
    CALL(VFSManagerOpenFileRead(vfs, &file, path));
    vector<char> buffer(kReadSize);
    size_t nread;
    CALL(x_KFileRead(file, 0, buffer.data(), buffer.size(), &nread));
    assert(nread == buffer.size());
    CALL(KFileRelease(file)); file = 0;
    CALL(VPathRelease(path)); path = 0;
    CALL(VFSManagerRelease(vfs)); vfs = 0;
}

void* read_thread_func(void* arg)
{
    size_t read_id = (size_t)arg;
    x_Read(kFileName[read_id], read_id);
    return 0;
}

int LowLevelTest()
{
    cout << "Running test for User-Agent." << endl;

    x_Init();

    if ( 1 ) {
        x_Read(kFileName[0], 0);
    }
    else {
        pthread_t threads[kFileCount];
        for ( size_t i = 0; i < kFileCount; ++i ) {
            pthread_create(&threads[i], 0, read_thread_func, (void*)i);
        }
        for ( size_t i = 0; i < kFileCount; ++i ) {
            void* ret = 0;
            pthread_join(threads[i], &ret);
        }
    }
    
    cout << "Success." << endl;
    return 0;
}

int main(int argc, const char* argv[])
{
    return LowLevelTest();
}
