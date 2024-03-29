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
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/impl.h>
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
