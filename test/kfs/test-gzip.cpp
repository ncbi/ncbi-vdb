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

/**
* Unit tests for the KGZipFile
*/

#include <ktst/unit_test.hpp>

#include <klib/rc.h>
#include <kfg/config.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/gzip.h>
#include <zlib.h>

#include <os-native.h>

#include <fstream>
#include <string>
#include <cstdlib>
#include <sys/wait.h>

static rc_t argsHandler(int argc, char* argv[]);
TEST_SUITE_WITH_ARGS_HANDLER(KGZipFileTestSuite, argsHandler);

using namespace std;
using namespace ncbi::NK;

static bool TestWithGZip(std::string const &path)
{
    auto const command = std::string("gzip --quiet --test ") + path;
    auto exitcode = std::system(command.c_str());
    if (exitcode < 0)
        throw std::logic_error("Can't run gzip!");
    if (WIFSIGNALED(exitcode))
        throw std::logic_error("gzip died!");
    if (WIFEXITED(exitcode))
        return WEXITSTATUS(exitcode) == 0;
    throw std::logic_error("gzip WTF!");
}

#define BASE_FILE "ipsum.txt"

class KGZipFileFixture
{
    KDirectory *cwd;
    uint64_t fileSize;
    void *inputBuffer;

    bool TestWithKFile(std::string const &path, bool isOriginal = false) const
    {
        KFile const *c_input = nullptr;
        KFile const *input = nullptr;
        char buffer[4096];
        char const *const orig = (char const *)inputBuffer;
        size_t actual = 0;
        rc_t rc = 0;

        THROW_ON_RC(KDirectoryOpenFileRead(cwd, &c_input, "%s", path.c_str()));
        THROW_ON_RC(KFileMakeGzipForRead(&input, c_input));
        KFileRelease(c_input);

        for ( ; ; ) {
            size_t numread = 0;
            rc = KFileReadAll(input, actual, buffer, sizeof(buffer), &numread);
            if (rc != 0)
                return false;
            if (numread == 0)
                break;
            if (isOriginal && memcmp(buffer, orig + actual, numread) != 0)
                throw std::logic_error("KGZipFile did not recreate the original");
            actual += numread;
        }
        if (isOriginal && actual != fileSize)
            throw std::logic_error("KGZipFile did not recreate the original");
        return true;
    }
    void CreateGZipFile() {
        // This is weirdly complicated in order to recreate the failure to
        // detect truncation. It has to have a Z_STREAM_END and then the next
        // read needs to hit the truncation.
        //
        // The reader code will get a Z_STREAM_END, set the done flag, get a
        // Z_OK, then a Z_BUF_ERROR and not have any more data to read. This is
        // exactly what a normal EOF looks like. The truncation is not detected.
        KFile const *input = nullptr;
        KFile *output = nullptr;
        size_t actual = 0;

        THROW_ON_RC(KDirectoryOpenFileRead(cwd, &input, "%s", BASE_FILE));
        THROW_ON_RC(KDirectoryCreateFile(cwd, &output, false, 0644, kcmInit, "%s.gz", BASE_FILE));

        THROW_ON_RC(KFileSize(input, &fileSize));
        inputBuffer = malloc(fileSize);
        if (inputBuffer == nullptr)
            throw std::bad_alloc();

        THROW_ON_RC(KFileReadAll(input, 0, inputBuffer, fileSize, &actual));
        if (actual != fileSize)
            throw std::logic_error("KFileReadAll didn't");

        KFileRelease(input); input = nullptr;

        auto outputBuffer = new char [fileSize];
        auto strm = z_stream();
        auto zr = deflateInit2(&strm, Z_BEST_COMPRESSION, Z_DEFLATED
                               , MAX_WBITS + 15 // use gzip
                               , 9, Z_DEFAULT_STRATEGY);

        if (!(zr == Z_OK))
            throw std::logic_error("zr != Z_OK");

        strm.next_in = (Bytef *)inputBuffer;
        strm.next_out = (Bytef *)outputBuffer;

        strm.avail_in = fileSize >> 2;
        strm.avail_out = fileSize;

        for ( ; ; ) {
            zr = deflate(&strm, Z_FINISH);
            if (zr == Z_OK)
                continue;
            break;
        }
        if (!(zr == Z_STREAM_END))
            throw std::logic_error("zr != Z_STREAM_END");

        auto start_in = strm.next_in;
        auto remain_in = fileSize - strm.total_in;
        auto start_out = strm.next_out;
        auto remain_out = strm.avail_out;
        auto total_out = strm.total_out;

        zr = deflateReset(&strm);
        if (!(zr == Z_OK))
            throw std::logic_error("zr != Z_OK");

        strm.next_in = start_in;
        strm.avail_in = remain_in;
        strm.next_out = start_out;
        strm.avail_out = remain_out;

        zr = deflate(&strm, Z_FINISH);
        if (!(zr == Z_STREAM_END))
            throw std::logic_error("zr != Z_STREAM_END");
        total_out += strm.total_out;

        deflateEnd(&strm);

        THROW_ON_RC(KFileWrite(output, 0, outputBuffer, total_out, &actual));
        delete [] outputBuffer;
        KFileRelease(output); output = nullptr;

        if (actual != total_out)
            throw std::logic_error("KFileWrite didn't");

        if (!TestWithGZip(BASE_FILE ".gz")) {
            throw std::logic_error("gzip doesn't like the output");
        }
    }
    void CreateBadGZipFiles() {
        KFile const *input = nullptr;
        KFile *trunced = nullptr;
        KFile *junked = nullptr;
        uint64_t buffer_size = 0;
        size_t actual = 0;
        void *buffer = nullptr;

        THROW_ON_RC(KDirectoryOpenFileRead(cwd, &input, "%s.gz", BASE_FILE));
        THROW_ON_RC(KDirectoryCreateFile(cwd, &trunced, false, 0644, kcmInit, "%s.gz.trunc", BASE_FILE));
        THROW_ON_RC(KDirectoryCreateFile(cwd, &junked, false, 0644, kcmInit, "%s.gz.junk", BASE_FILE));

        THROW_ON_RC(KFileSize(input, &buffer_size));
        buffer = malloc(buffer_size);
        if (buffer == nullptr)
            throw std::bad_alloc();

        THROW_ON_RC(KFileReadAll(input, 0, buffer, buffer_size, &actual));
        if (actual != buffer_size)
            throw std::logic_error("KFileReadAll didn't");
        KFileRelease(input); input = nullptr;

        THROW_ON_RC(KFileWrite(trunced, 0, buffer, buffer_size, &actual));
        if (actual != buffer_size)
            throw std::logic_error("KFileWrite didn't");
        KFileSetSize(trunced, buffer_size >> 1);
        KFileRelease(trunced); trunced = nullptr;

        memmove(&((char *)buffer)[buffer_size >> 1] - 1, buffer, 2);
        THROW_ON_RC(KFileWrite(junked, 0, buffer, buffer_size, &actual));
        if (actual != buffer_size)
            throw std::logic_error("KFileWrite didn't");
        KFileRelease(junked); junked = nullptr;

        if (TestWithGZip(BASE_FILE ".gz.trunc")) {
            throw std::logic_error("gzip shouldn't like a truncated file");
        }
        if (TestWithGZip(BASE_FILE ".gz.junked")) {
            throw std::logic_error("gzip shouldn't like a junked file");
        }
    }
public:
    KGZipFileFixture()
    : cwd(nullptr)
    , inputBuffer(nullptr)
    {
        THROW_ON_RC(KDirectoryNativeDir(&cwd));
        CreateGZipFile();
        CreateBadGZipFiles();
    }
    ~KGZipFileFixture ()
    {
        free(inputBuffer);
        KDirectoryRemove(cwd, false, "%s.gz", BASE_FILE);
        KDirectoryRemove(cwd, false, "%s.gz.junk", BASE_FILE);
        KDirectoryRemove(cwd, false, "%s.gz.trunc", BASE_FILE);
        KDirectoryRelease(cwd);
    }
    void TestRead() {
        if (!TestWithKFile(BASE_FILE ".gz", true))
            throw std::logic_error("Could not read back the original");
        if (TestWithKFile(BASE_FILE ".gz.junk"))
            throw std::logic_error("KGZipFile did not detected junked file");
        if (TestWithKFile(BASE_FILE ".gz.trunc"))
            throw std::logic_error("KGZipFile did not detected truncated file");
    }
};

FIXTURE_TEST_CASE(KGZipFile_Test, KGZipFileFixture)
{
    TestRead();
}

#include <kapp/args.h>

static rc_t argsHandler(int argc, char * argv[]) {
    Args * args = NULL;
    rc_t rc = ArgsMakeAndHandle(&args, argc, argv, 0, NULL, 0);
    ArgsWhack(args);
    return rc;
}

#include <kfg/config.h>
#include <klib/debug.h>

extern "C"
{
ver_t CC KAppVersion ( void )
{
    return 0x1000000;
}
rc_t CC UsageSummary (const char * progname)
{
    return 0;
}

rc_t CC Usage ( const Args * args )
{
    return 0;
}
const char UsageDefaultName[] = "test-gzip-file";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();

	// this makes messages from the test code appear
	// (same as running the executable with "-l=message")
	//TestEnv::verbosity = LogLevel::e_message;

    rc_t rc = KGZipFileTestSuite(argc, argv);
    return rc;
}

}
