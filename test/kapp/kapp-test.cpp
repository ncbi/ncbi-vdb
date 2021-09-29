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

#include <cstdlib>
#include <cstring>
#include <cstdio>

#include <ktst/unit_test.hpp>
#include <klib/out.h>
#include <klib/rc.h>
#include <kapp/main.h>
#include <kapp/args.h>
#include <kapp/loader-file.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/impl.h>
#include <kapp/queue-file.h>
#include <kfg/config.h>

using namespace std;
using namespace ncbi::NK;

TEST_SUITE(KAppTestSuite);

const char UsageDefaultName[] = "args-test";

#if ! ALLOW_TESTING_CODE_TO_RELY_UPON_CODE_BEING_TESTED
extern "C"
{
    rc_t CC UsageSummary ( const char *progname )
    {
        return TestEnv::UsageSummary ( progname );
    }

    rc_t CC Usage ( const Args *args )
    {
        const char* progname = UsageDefaultName;
        const char* fullpath = UsageDefaultName;

        rc_t rc = (args == NULL) ?
            RC (rcApp, rcArgv, rcAccessing, rcSelf, rcNull):
            ArgsProgram(args, &fullpath, &progname);
        if ( rc == 0 )
            rc = TestEnv::Usage ( progname );
        return rc;
    }
}
#endif

#define OPTION_TEST "test"

static char arg_append_string[] = "__T__";
static size_t arg_append_string_len = sizeof arg_append_string / sizeof arg_append_string[0] - 1;

rc_t TestArgConvAppender(const Args * args, uint32_t arg_index, const char * arg, size_t arg_len, void ** result, WhackParamFnP * whack)
{
    char * res = (char *)malloc(arg_len + arg_append_string_len + 1);
    assert(res);
    
    memmove(res, arg, arg_len);
    memmove(res + arg_len, arg_append_string, arg_append_string_len);
    res[arg_len + arg_append_string_len] = 0;
    
    *result = res;
    
    return 0;
}

void WhackArgFile(void * file)
{
    KFileRelease(reinterpret_cast<KFile *>(file));
}

rc_t TestArgConvFileCreator(const Args * args, uint32_t arg_index, const char * arg, size_t arg_len, void ** result, WhackParamFnP * whack)
{
    rc_t rc;
    KDirectory * dir;
    KFile * file;
    
    char * file_path = const_cast<char *>(arg);
    
    rc = KDirectoryNativeDir( &dir );
    if (rc == 0)
    {
        KDirectoryRemove(dir, true, "%s", file_path);
        rc = KDirectoryCreateFile(dir, &file, false, 0664, kcmCreate, "%s", file_path);
        if (rc == 0)
        {
            char buffer[4] = { 'a', 'b', 'c', 'd' };
            size_t num_written;
            
            rc = KFileWriteAll(file, 0, buffer, sizeof buffer / sizeof buffer[0], &num_written);
            if (rc == 0)
            {
                assert(num_written == sizeof buffer / sizeof buffer[0]);
                
                *result = file;
                *whack = WhackArgFile;
                KDirectoryRelease( dir );
                return 0;
            }
            
            fprintf(stderr, "cannot write test buffer to create file: %s\n", file_path);
            KFileRelease(file);
        }
        else
        {
            fprintf(stderr, "cannot create file from argument: %s\n", file_path);
        }
        KDirectoryRelease( dir );
    }
    
    return rc;
}

TEST_CASE(KApp_ArgsMakeParams)
{
    int argc;
    const char * argv[16];

    /* testing params */
    argc = 6;
    argv[0] = "test_1";
    argv[1] = "0";
    argv[2] = "1";
    argv[3] = "2";
    argv[4] = "3";
    argv[5] = "4";

    Args * args;
    REQUIRE_RC(ArgsMake (&args));
    REQUIRE_RC(ArgsParse (args, argc, (char**)argv));

    {
        uint32_t param_count;
        uint32_t ix;

        REQUIRE_RC(ArgsParamCount (args, &param_count));
        REQUIRE_EQ(param_count, (uint32_t)argc-1);
        for (ix = 0; ix < param_count; ix++)
        {
            const char * value;
            REQUIRE_RC(ArgsParamValue (args, ix, reinterpret_cast<const void**>(&value)));
            {
                /* valgrind whines about the line below.  I can't see
                 * the problem with a uninitialized variable used for
                 * a conditional jump unless its in libc */
                REQUIRE_EQ(atoi(value), (int)ix);
            }
        }
    }
    REQUIRE_RC(ArgsWhack (args));
}

TEST_CASE(KApp_ArgsMakeParamsConvAppend)
{
    ParamDef Parameters[] =
    {
        { TestArgConvAppender }
    };
    int argc;
    const char * argv[16];
    
    /* testing params */
    argc = 2;
    argv[0] = "test_1";
    argv[1] = "abcd";
    
    Args * args;
    REQUIRE_RC(ArgsMake (&args));
    REQUIRE_RC(ArgsAddParamArray (args, Parameters, sizeof Parameters / sizeof Parameters[0]));
    REQUIRE_RC(ArgsParse (args, argc, (char**)argv));
    
    {
        const char * value;
        uint32_t param_count;
        
        REQUIRE_RC(ArgsParamCount (args, &param_count));
        REQUIRE_EQ(param_count, (uint32_t)argc-1);
        
        REQUIRE_RC(ArgsParamValue (args, 0, reinterpret_cast<const void**>(&value)));
        
        REQUIRE(memcmp(value, argv[1], 4) == 0);
        REQUIRE(memcmp(value + 4, arg_append_string, arg_append_string_len + 1) == 0);
    }
    REQUIRE_RC(ArgsWhack (args));
}

TEST_CASE(KApp_ArgsMakeOptions)
{
    OptDef Options[] =
    {                                         /* needs_value, required */
        { OPTION_TEST, NULL, NULL, NULL, 1, true, false }
    };
    int argc;
    const char * argv[16];
    
    /* testing params */
    argc = 3;
    argv[0] = "test_2";
    argv[1] = "--test";
    argv[2] = "abcd";
    
    Args * args;
    REQUIRE_RC(ArgsMake (&args));
    REQUIRE_RC(ArgsAddOptionArray (args, Options, sizeof Options / sizeof Options[0]));
    REQUIRE_RC(ArgsParse (args, argc, (char**)argv));
    
    {
        const char * value;
        uint32_t count;
        
        REQUIRE_RC(ArgsParamCount (args, &count));
        REQUIRE_EQ(count, (uint32_t)0);
        
        REQUIRE_RC(ArgsOptionCount (args, OPTION_TEST, &count));
        REQUIRE_EQ(count, (uint32_t)1);
        
        REQUIRE_RC(ArgsOptionValue (args, OPTION_TEST, 0, reinterpret_cast<const void**>(&value)));
        REQUIRE_EQ(std::string(value), std::string(argv[2]));
    }
    REQUIRE_RC(ArgsWhack (args));
}

TEST_CASE(KApp_ArgsMakeOptionsConversion)
{
    OptDef Options[] =
    {                                         /* needs_value, required */
        { OPTION_TEST, NULL, NULL, NULL, 1, true, false, TestArgConvFileCreator }
    };
    int argc;
    const char * argv[16];
    KDirectory * dir;
    
    /* testing params */
    argc = 3;
    argv[0] = "test_2";
    argv[1] = "--test";
    argv[2] = "file.test";
    
    Args * args;
    REQUIRE_RC(ArgsMake (&args));
    REQUIRE_RC(ArgsAddOptionArray (args, Options, sizeof Options / sizeof Options[0]));
    REQUIRE_RC(ArgsParse (args, argc, (char**)argv));
    
    {
        const KFile * file;
        uint32_t count;
        uint64_t file_size;
        
        REQUIRE_RC(ArgsParamCount (args, &count));
        REQUIRE_EQ(count, (uint32_t)0);
        
        REQUIRE_RC(ArgsOptionCount (args, OPTION_TEST, &count));
        REQUIRE_EQ(count, (uint32_t)1);
        
        REQUIRE_RC(ArgsOptionValue (args, OPTION_TEST, 0, reinterpret_cast<const void**>(&file)));
        
        REQUIRE_RC(KFileSize (file, &file_size));
        
        REQUIRE_EQ(file_size, (uint64_t)4);
    }
    REQUIRE_RC(ArgsWhack (args));
    
    REQUIRE_RC(KDirectoryNativeDir ( &dir ));
    REQUIRE_RC(KDirectoryRemove(dir, true, "%s", argv[2]));
    REQUIRE_RC(KDirectoryRelease (dir));
}

#ifndef WINDOWS
TEST_CASE(KQueueFile_ReadTimeout_FGsleeps)
{
    KDirectory *dir;
    REQUIRE_RC(KDirectoryNativeDir(&dir));

    KFile const* src;
    REQUIRE_RC(KDirectoryOpenFileRead(dir, &src, "queuefile.data"));

    const size_t BlockSize=10;
    struct KFile const* qf;
    const uint32_t timeoutSec=1;
    // create a queue of 1 short block 
    REQUIRE_RC(KQueueFileMakeRead(&qf, 0, src, BlockSize*1, BlockSize, timeoutSec*1000));

    char buffer[BlockSize];
    size_t num_read;
    uint64_t pos=0;
    REQUIRE_RC(KFileRead(qf, pos, buffer, BlockSize, &num_read)); REQUIRE_EQ(num_read, BlockSize); pos+=num_read;
    // now sleep longer than the timeout and make sure the background thread does not seal the queue
    TestEnv::Sleep(timeoutSec*2);
    // read the block that has been populated while we slept:
    REQUIRE_RC(KFileRead(qf, pos, buffer, BlockSize, &num_read)); REQUIRE_EQ(num_read, BlockSize); pos+=num_read;
    // if queue has been sealed because of the timeout, the second read would return 0 bytes:
    REQUIRE_RC(KFileRead(qf, pos, buffer, BlockSize, &num_read)); REQUIRE_EQ(num_read, BlockSize); pos+=num_read;

    REQUIRE_RC(KFileRelease(qf));
    REQUIRE_RC(KFileRelease(src));
    REQUIRE_RC(KDirectoryRelease(dir));
}

//// A mock KFile object for imitating slow read on a background thread. Sends its thread to sleep after reading each block.
struct SleepyReader
{
    const static uint64_t Size=10000;
    const static int SleepSec=1;

    static rc_t MakeFileRead(KFile const** f)
    {
        KFile* ret=(KFile*)calloc(1, sizeof(KFile));
        ret-> vt = (const KFile_vt*)&vt;
        ret-> dir = NULL;
        atomic32_set ( & ret-> refcount, 1 );
        ret-> read_enabled = 1;
        ret-> write_enabled = 0;            
        *f = ret;
        return 0;
    }
    static rc_t CC get_size ( const KFILE_IMPL *self, uint64_t *size ) 
    { 
        *size=Size; 
        return 0; 
    }
    static rc_t CC read( const KFILE_IMPL *self, uint64_t pos, void *buffer, size_t bsize, size_t *num_read )
    {
        memset(buffer, 1, bsize);
        *num_read=bsize;
        TestEnv::Sleep(SleepSec);
        return 0;
    }
    static rc_t CC destroy( KFILE_IMPL *self ) 
    { 
        free(self); 
        return 0; 
    }

    // the rest of the functions do not matter
    static struct KSysFile* CC get_sysfile ( const KFILE_IMPL *self, uint64_t *offset ) { *offset=0; return 0; }
    static rc_t CC random_access ( const KFILE_IMPL *self ) { return 0; } 
    static rc_t CC set_size ( KFILE_IMPL *self, uint64_t size ) { return 0; }
    static rc_t CC write( KFILE_IMPL *self, uint64_t pos, const void *buffer, size_t size, size_t *num_writ ) { *num_writ=0; return 0; }

    static KFile_vt_v1 vt;
};
KFile_vt_v1 SleepyReader::vt=
{   1, 0, 
    SleepyReader::destroy,
    SleepyReader::get_sysfile,
    SleepyReader::random_access,
    SleepyReader::get_size,
    SleepyReader::set_size,
    SleepyReader::read,
    SleepyReader::write
};

TEST_CASE(KQueueFile_ReadTimeout_BGsleeps)
{   
    KFile const* src;
    REQUIRE_RC(SleepyReader::MakeFileRead(&src)); // this reader will sleep for a second after each block

    const size_t BlockSize=10;
    struct KFile const* qf;
    // create a queue of 1 short block 
    REQUIRE_RC(KQueueFileMakeRead(&qf, 0, src, BlockSize*1, BlockSize, 100)); // a short timeout on the foreground thread

    char buffer[BlockSize];
    size_t num_read;
    uint64_t pos=0;
    REQUIRE_RC(KFileRead(qf, pos, buffer, BlockSize, &num_read)); REQUIRE_EQ(num_read, BlockSize); pos+=num_read;
    // the background read thread will read the bytes and go to sleep for 1 second
    // now, read the next block to make sure the foreground read thread recovers from the timeouts
    REQUIRE_RC(KFileRead(qf, pos, buffer, BlockSize, &num_read)); REQUIRE_EQ(num_read, BlockSize); pos+=num_read;

    REQUIRE_RC(KFileRelease(qf));
    REQUIRE_RC(KFileRelease(src));
}

TEST_CASE(KQueueFile_WriteTimeout)
{   
    KDirectory *dir;
    REQUIRE_RC(KDirectoryNativeDir(&dir));

    const char* fileName="queuefile.temp";
    KFile *dest;
    REQUIRE_RC(KDirectoryCreateFile(dir, (KFile**)&dest, false, 0664, kcmInit, fileName));

    const size_t BlockSize=10;
    struct KFile* qf;
    const uint32_t timeoutSec=1;
    // create a queue of 1 short block 
    REQUIRE_RC(KQueueFileMakeWrite(&qf, dest, BlockSize*1, BlockSize, timeoutSec*1000));

    char buffer[BlockSize*2];
    memset(buffer, 0, sizeof(buffer));
    size_t num_writ;
    uint64_t pos=0;
    REQUIRE_RC(KFileWrite(qf, pos, buffer, BlockSize, &num_writ)); REQUIRE_EQ(num_writ, BlockSize); pos+=num_writ;
    // now sleep longer than the timeout and make sure the background thread does not seal the queue
    TestEnv::Sleep(timeoutSec*2);
    // make sure we can continue writing:
    REQUIRE_RC(KFileWrite(qf, pos, buffer, BlockSize, &num_writ)); REQUIRE_EQ(num_writ, BlockSize); 

    REQUIRE_RC(KFileRelease(qf));
    REQUIRE_RC(KFileRelease(dest));
    REQUIRE_RC(KDirectoryRemove(dir, true, fileName)); 
    REQUIRE_RC(KDirectoryRelease(dir));
}
#endif

class LoaderFileFixture
{
public:
    LoaderFileFixture() 
    :   wd(0), lf(0)
    {
        if ( KDirectoryNativeDir ( & wd ) != 0 )
            FAIL("KDirectoryNativeDir failed");
    }
    ~LoaderFileFixture() 
    {
        if ( lf != 0 && KLoaderFile_Release( lf, true ) != 0)
            FAIL("KLoaderFile_Release failed");
            
        if ( !filename.empty() && KDirectoryRemove(wd, true, filename.c_str()) != 0)
            FAIL("KDirectoryRemove failed");

        if ( KDirectoryRelease ( wd ) != 0 )
            FAIL("KDirectoryRelease failed");
    }
    rc_t CreateFile(const char* p_filename, const char* contents)
    {   // create and open for read
        KFile* file;
        filename=p_filename;
        rc_t rc=KDirectoryCreateFile(wd, &file, true, 0664, kcmInit, p_filename);
        if (rc == 0)
        {
            size_t num_writ=0;
            rc=KFileWrite(file, 0, contents, strlen(contents), &num_writ);
            if (rc == 0)
            {
                rc=KFileRelease(file);
            }
            else
            {
                KFileRelease(file);
            }
            file=0;
        }
        return KLoaderFile_Make(&lf, wd, p_filename, 0, true);
    }
    KDirectory* wd;
    string filename;
    const KLoaderFile* lf;
};

FIXTURE_TEST_CASE(KLoaderFile_eolBeforeEof, LoaderFileFixture)
{   
    string input="qqq abcd\n";
    CreateFile(GetName(), input.c_str());
    const char* buf = 0;
    size_t length = 0;
    REQUIRE_RC(KLoaderFile_Readline(lf, (const void**)&buf, &length));
    REQUIRE_NOT_NULL(buf);
    REQUIRE_EQ(input, string(buf, length + 1)); // \n is not included in length but should be in the buffer
}


FIXTURE_TEST_CASE(KLoaderFile_noEolBeforeEof, LoaderFileFixture)
{   // formerly a bug: if no \n on the last line of a file, the line was lost
    string input="qqq abcd";
    CreateFile(GetName(), input.c_str());
    const char* buf = 0;
    size_t length = 0;
    REQUIRE_RC(KLoaderFile_Readline(lf, (const void**)&buf, &length));
    REQUIRE_NOT_NULL(buf);
    REQUIRE_EQ(input, string(buf, length));
}

TEST_CASE(KAppCheckEnvironment_requireAmd64)
{
    rc_t rc = KAppCheckEnvironment ( true, 0 );
#if _ARCH_BITS != 64    
    REQUIRE_NE ( (int)rc, 0 );
#else
    REQUIRE_EQ ( (int)rc, 0 );
#endif
}

//////////////////////////////////////////// Main

extern "C"
{

/* Version  EXTERN
 *  return 4-part version code: 0xMMmmrrrr, where
 *      MM = major release
 *      mm = minor release
 *    rrrr = bug-fix release
 */
ver_t CC KAppVersion (void)
{
    return 0;
}

/* KMain - EXTERN
 *  executable entrypoint "main" is implemented by
 *  an OS-specific wrapper that takes care of establishing
 *  signal handlers, logging, etc.
 *
 *  in turn, OS-specific "main" will invoke "KMain" as
 *  platform independent main entrypoint.
 *
 *  "argc" [ IN ] - the number of textual parameters in "argv"
 *  should never be < 0, but has been left as a signed int
 *  for reasons of tradition.
 *
 *  "argv" [ IN ] - array of NUL terminated strings expected
 *  to be in the shell-native character set: ASCII or UTF-8
 *  element 0 is expected to be executable identity or path.
 */
rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KAppTestSuite(argc, argv);
    return rc;
}

}
