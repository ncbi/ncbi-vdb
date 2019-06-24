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
* Unit tests for Kfg interface
*/

#include <ktst/unit_test.hpp>

#include <kapp/args.h> /* Args */

#include <kfg/config.h>
#include <kfg/kfg-priv.h>
#include <kfg/extern.h>
#include <kfg/repository.h> /* KConfigImportNgc */

#include <os-native.h>

#if !WINDOWS
    #include <sys/utsname.h>
#endif

#include <vfs/manager.h>
#include <vfs/path.h>

#include <kfs/dyload.h>
#include <kfs/impl.h>

#include <klib/debug.h> /* KDbgSetModConds */
#include <klib/log.h>
#include <klib/printf.h>
#include <klib/text.h>
#include <klib/writer.h>

#include <cstdlib>
#include <cstring>

#include "kfg-fixture.hpp"

using namespace std;

#define ALL

static rc_t argsHandler(int argc, char* argv[]);
TEST_SUITE_WITH_ARGS_HANDLER(KfgTestSuite, argsHandler);

FIXTURE_TEST_CASE(testKConfigPrint, KfgFixture)
{
	REQUIRE_RC(KConfigPrint(kfg, 0));
}

///////////////////////////////////////////////// KFG parser test cases

#ifdef ALL
FIXTURE_TEST_CASE(KConfigLoadFile_should_report_null_inputs, KfgFixture)
{
    KFile file;
    REQUIRE_RC_FAIL(KConfigLoadFile ( 0, "qweert", &file));
    REQUIRE_RC_FAIL(KConfigLoadFile ( kfg, "qweert", 0));
}

FIXTURE_TEST_CASE(one_name_value_double_quotes, KfgFixture)
{
    CreateAndLoad(GetName(), "name=\"value\"");
    REQUIRE(ValueMatches("name", "value"));
}

FIXTURE_TEST_CASE(one_pathname_value_single_quotes, KfgFixture)
{
    CreateAndLoad(GetName(), "root/subname/name='val'");
    REQUIRE(ValueMatches("root/subname/name", "val"));
}

FIXTURE_TEST_CASE(numeric_pathnames, KfgFixture)
{
    CreateAndLoad(GetName(), " root1/1 = 'val1'\n");
    REQUIRE(ValueMatches("root1/1", "val1"));
}

FIXTURE_TEST_CASE(multiple_pathnames, KfgFixture)
{   // sprinkle some spaces and tabs (only allowed outside pathnames)
    const char* contents=" root1/subname1/name1 =\t \"val1\"\n"
                         "root1/subname1/name2 =\t \"val2\"\n"
                         "\troot1/subname2/name3\t = \"val3\"\n"
                         "root1/subname2/name4\t = \"val4\"\n"
                         "root2/subname1/name5 =\t \"val5\"\n"
                         "root2/subname2/name6 = \"val6\"\n";
    CreateAndLoad(GetName(), contents);

    REQUIRE(ValueMatches("root1/subname1/name1", "val1"));
    REQUIRE(ValueMatches("root1/subname1/name2", "val2"));
    REQUIRE(ValueMatches("root1/subname2/name3", "val3"));
    REQUIRE(ValueMatches("root1/subname2/name4", "val4"));
    REQUIRE(ValueMatches("root2/subname1/name5", "val5"));
    REQUIRE(ValueMatches("root2/subname2/name6", "val6"));
}

FIXTURE_TEST_CASE(comments, KfgFixture)
{
    const char* contents="root1/subname1/name1 ='val1'\n"
                         "root1/subname1/name2 ='val2'\n"
                         "#root1/subname1/name1='val11'\n"       // the commented out lines do not override the lines with the same paths above
                         "/*root1/subname1/name2=\"val12\"*/\n";
    CreateAndLoad(GetName(), contents);

    REQUIRE(ValueMatches("root1/subname1/name1", "val1"));
    REQUIRE(ValueMatches("root1/subname1/name2", "val2"));
}

FIXTURE_TEST_CASE(unescaping_escapes, KfgFixture)
{
    CreateAndLoad(GetName(), "name='\\a'\n");
    REQUIRE(ValueMatches("name", "\a"));
}

FIXTURE_TEST_CASE(dots_in_pathnames, KfgFixture)
{
    CreateAndLoad(GetName(), "root.1./subname1.ext='val100'\n");
    REQUIRE(ValueMatches("root.1./subname1.ext", "val100"));
}

FIXTURE_TEST_CASE(variable_expansion_simple, KfgFixture)
{
    const char* contents="var='value'\n"
                         "ref=$(var)\n";
    CreateAndLoad(GetName(), contents);

    REQUIRE(ValueMatches("var", "value"));
    REQUIRE(ValueMatches("ref", "value"));
}

FIXTURE_TEST_CASE(variable_expansion_concat, KfgFixture)
{
    const char* contents="var1='value1'\n"
                         "var2='value2'\n"
                         "ref=\"$(var1)'$(var2)\"\n";
    CreateAndLoad(GetName(), contents);

    REQUIRE(ValueMatches("ref", "value1'value2"));
}

FIXTURE_TEST_CASE(variable_expansion_path, KfgFixture)
{
    const char* contents="root/var='value'\n"
                         "ref=$(root/var)\n";
    CreateAndLoad(GetName(), contents);

    REQUIRE(ValueMatches("ref", "value"));
}

FIXTURE_TEST_CASE(in_string_variable_expansion_path, KfgFixture)
{
    const char* contents="root/var='value'\n"
                         "ref=\"+$(root/var)+\"\n";
    CreateAndLoad(GetName(), contents);

    REQUIRE(ValueMatches("ref", "+value+"));
}

FIXTURE_TEST_CASE(can_reference_keys_across_files, KfgFixture)
{
    const char* contents1="root/var='Value'\n";
    CreateAndLoad((string(GetName())+"1").c_str(), contents1);
    const char* contents2="ref=$(root/var)\n";
    CreateAndLoad((string(GetName())+"2").c_str(), contents2);

    REQUIRE(ValueMatches("ref", "Value"));
}

FIXTURE_TEST_CASE(long_key, KfgFixture)
{
    string key(1025, 'k');
    CreateAndLoad(GetName(), (key+"='value'").c_str());
    REQUIRE(ValueMatches(key.c_str(), "value"));
}

FIXTURE_TEST_CASE(long_path, KfgFixture)
{
    string path=string(4097, 'v');
    string line("k='");
    line+=path;
    line+="'";
    CreateAndLoad(GetName(), line.c_str());
    REQUIRE(ValueMatches("k", path.c_str()));
}

FIXTURE_TEST_CASE(KConfigParse_SelfNull, KfgFixture)
{
    REQUIRE_RC_FAIL ( KConfigParse (NULL, "", "") );
}
FIXTURE_TEST_CASE(KConfigParse_NullPath, KfgFixture)
{   // not a problem
    REQUIRE_RC ( KConfigParse ( kfg, NULL, "" ) );
}
FIXTURE_TEST_CASE(KConfigParse_NullString, KfgFixture)
{
    REQUIRE_RC_FAIL ( KConfigParse ( kfg, NULL, NULL ) );
}
FIXTURE_TEST_CASE(KConfigParse_Parse, KfgFixture)
{
    REQUIRE_RC ( KConfigParse ( kfg, "", "root/var='Value'\n" ) );
    REQUIRE ( ValueMatches ( "root/var", "Value" ) );
}

///////////////////////////////////////////////// predefined variables
FIXTURE_TEST_CASE(predef_LIBPATH, KfgFixture)
{
#if WINDOWS && !_STATIC
    // since this program and libkfg.dll live in different directories, they contain separate copies of KConfigMake under Windows,
    // so we cannot compare them
#else
    const char* contents="var=$(vdb/lib/paths/kfg)\n";
    CreateAndLoad(GetName(), contents);

    KDyld *dyld;
    REQUIRE_RC(KDyldMake ( & dyld ));
    const KDirectory *dir;
    REQUIRE_RC(KDyldHomeDirectory ( dyld, & dir, ( fptr_t ) KConfigMake ));
    REQUIRE(ValueMatches("var", DirPath(dir).c_str()));
    KDirectoryRelease ( dir );
    KDyldRelease ( dyld );
#endif
}

FIXTURE_TEST_CASE(predef_KFGDIR, KfgFixture)
{
    CreateAndLoad(GetName(), "var=$(kfg/dir)\n");
    REQUIRE(ValueMatches("var", DirPath(wd).c_str()));
}
FIXTURE_TEST_CASE(predef_KFGNAME, KfgFixture)
{
    CreateAndLoad(GetName(), "var=$(kfg/name)\n");
    REQUIRE(ValueMatches("var", GetName()));
}


FIXTURE_TEST_CASE(predef_ARCHNAME, KfgFixture)
{
    CreateAndLoad(GetName(), "var=$(kfg/arch/name)\n");
    #if WINDOWS
        REQUIRE(ValueMatches("var", ""));
    #else
        struct utsname name;
        REQUIRE_NE(uname(&name), -1);
        REQUIRE(ValueMatches("var", name.nodename));
    #endif
}
FIXTURE_TEST_CASE(predef_ARCHBITS, KfgFixture)
{
    CreateAndLoad(GetName(), "var=$(kfg/arch/bits)\n");
    string_printf(buf, sizeof(buf), &num_writ, "%d", _ARCH_BITS);
    REQUIRE(ValueMatches("var", buf));
}

FIXTURE_TEST_CASE(predef_OS, KfgFixture)
{
    CreateAndLoad(GetName(), "var=$(OS)\n");
    #if LINUX
        #define OS "linux"
    #elif SUN
        #define OS "sun"
    #elif MAC
        #define OS "mac"
    #elif WINDOWS
        #define OS "win"
    #endif
    REQUIRE(ValueMatches("var", OS));
    #undef OS
 }

#if 0 // 8/11/14 - no longer distinguishing linkage
FIXTURE_TEST_CASE(predef_BUILD_LINKAGE, KfgFixture)
{
    CreateAndLoad(GetName(), "var=$(BUILD_LINKAGE)\n");
    #if _STATIC
        #define BUILD_LINKAGE "STATIC"
    #else
        #define BUILD_LINKAGE "DYNAMIC"
    #endif
    REQUIRE(ValueMatches("var", BUILD_LINKAGE));
    #undef BUILD_LINKAGE
}
#endif

FIXTURE_TEST_CASE(predef_BUILD, KfgFixture)
{
    CreateAndLoad(GetName(), "var=$(BUILD)\n");
    #if _PROFILING
        #define BUILD "PROFILE"
    #else
        #if _DEBUGGING
            #define BUILD "DEBUG"
        #else
            #define BUILD "RELEASE"
        #endif
    #endif
    REQUIRE(ValueMatches("var", BUILD));
    #undef BUILD
}

#if 0 // only appropriate when invoked by a canonical path ?
FIXTURE_TEST_CASE(predef_APPPATH, KfgFixture)
{
    // REQUIRE_RC(CreateAndLoad(GetName(), "var=$(APPPATH)\n"));
    // APPPATH is only set correctly for the 1st instance of KConfig, so we saved it off in the first call to fixture's
    // constructor, test here
    string path(ncbi::NK::GetTestSuite()->argv[0]);
    string::size_type lastSlash=path.find_last_of("/");
    if (lastSlash == string::npos)
    {
        lastSlash=path.find_last_of("\\");
    }
    if (lastSlash != string::npos)
    {
        path.erase(lastSlash);
    }
    REQUIRE_EQ(strcase_cmp(apppath.c_str(), apppath.length(),
                           path.c_str(), path.length(),
                           max(apppath.length(), path.length())),
               0);
}
#endif

FIXTURE_TEST_CASE(predef_APPNAME, KfgFixture)
{
    CreateAndLoad(GetName(), "var=$(APPNAME)\n");
    REQUIRE_RC(LogAppName(buf, sizeof(buf), &num_writ));
    buf[num_writ]=0;
    REQUIRE(ValueMatches("var", buf));
}

FIXTURE_TEST_CASE(predef_PWD, KfgFixture)
{
    CreateAndLoad(GetName(), "var=$(PWD)\n");
    KDirectory* dir;
    REQUIRE_RC(KDirectoryNativeDir(&dir));
    REQUIRE(ValueMatches("var", DirPath(dir).c_str()));
    KDirectoryRelease(dir);
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(predef_ENV, KfgFixture)
{
    const char* contents=
        "host=$(HOST)\n"
        "user=$(USER)\n"
        "vdb_root=$(VDB_ROOT)\n"
        "vdb_config=$(VDB_CONFIG)\n"
        "home=$(HOME)\n"
        "ncbi_home=$(NCBI_HOME)\n"
        "ncbi_settings=$(NCBI_SETTINGS)\n"
        ;
    CreateAndLoad(GetName(), contents);
    REQUIRE(ValueMatches("host",            getenv("HOST"), true));
    REQUIRE(ValueMatches("user",            getenv("USER"), true));
    REQUIRE(ValueMatches("vdb_root",        getenv("VDB_ROOT"), true));
    REQUIRE(ValueMatches("vdb_config",      getenv("VDB_CONFIG"), true));
    REQUIRE(ValueMatches("home",            GetHomeDirectory().c_str(), true));
    REQUIRE(ValueMatches("ncbi_home",       (GetHomeDirectory()+"/.ncbi").c_str(), true));
    REQUIRE(ValueMatches("ncbi_settings",   (GetHomeDirectory()+"/.ncbi/user-settings.mkfg").c_str(), true));
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(predef_ENV_direct, KfgFixture)
{   // can also refer to predefs by their unadorned name
    REQUIRE(ValueMatches("HOST",            getenv("HOST"), true));
    REQUIRE(ValueMatches("USER",            getenv("USER"), true));
    REQUIRE(ValueMatches("VDB_ROOT",        getenv("VDB_ROOT"), true));
    REQUIRE(ValueMatches("VDB_CONFIG",      getenv("VDB_CONFIG"), true));
    REQUIRE(ValueMatches("HOME",            GetHomeDirectory().c_str(), true));
    REQUIRE(ValueMatches("NCBI_HOME",       (GetHomeDirectory()+"/.ncbi").c_str(), true));
    REQUIRE(ValueMatches("NCBI_SETTINGS",   (GetHomeDirectory()+"/.ncbi/user-settings.mkfg").c_str(), true));
}

#if 0
FIXTURE_TEST_CASE(include_files, KfgFixture)
{
#define includeName "include_file"
    const char* contents1="root/var='Value'\n";
    MakeFile((GetName()+"1").c_str(), contents1);
    const char* contents2="include ./" includeName "\n"
                          "ref=$(root/var)\n";
    CreateAndLoad((GetName()+"2").c_str(), contents2);

    REQUIRE(ValueMatches("ref", "Value"));
    REQUIRE_RC(KDirectoryRemove(wd, true, includeName));
}
#endif
#endif

///////////////////////////////////////////////// modification and commit

#ifdef ALL
FIXTURE_TEST_CASE(ChangeCommit, KfgFixture)
{
    const char* contents=
        "one=\"1\"\n"
        "one/two=\"2\"\n"
        "one/two/three=\"3\"\n"
        ;
    // override NCBI_SETTINGS
    const char* LocalSettingsFile = "settings.mkfg";
    string FullMagicPath = DirPath(wd) + "/" + LocalSettingsFile;
    CreateAndLoad( GetName(), (string(contents) + "NCBI_SETTINGS=\"" + FullMagicPath + "\"\n").c_str() );

    // make, commit changes
    UpdateNode("one", "1+0");
    UpdateNode("one/two", "0+2");
    REQUIRE_RC(KConfigCommit(kfg));
    REQUIRE_RC(KConfigRelease(kfg));

    // load the changes from the new location
    REQUIRE_RC(KConfigMake(&kfg,wd));
    LoadFile(FullMagicPath.c_str());

    // verify changes
    REQUIRE(ValueMatches("one", "1+0"));
    REQUIRE(ValueMatches("one/two", "0+2"));
    string s;
    REQUIRE(! GetValue("one/two/three", s)); // unchanged values are not saved

    REQUIRE_RC(KDirectoryRemove(wd, true, LocalSettingsFile));
}

FIXTURE_TEST_CASE(ChangeCommitEscapes, KfgFixture)
{
    const char* LocalSettingsFile = "settings.mkfg";
    string FullMagicPath = DirPath(wd) + "/" + LocalSettingsFile;
    CreateAndLoad( GetName(), (string() + "NCBI_SETTINGS=\"" + FullMagicPath + "\"\n").c_str() );

    // make, commit changes
    UpdateNode("double/quote", "\"");
    UpdateNode("escaped/hex", "\x0a");
    REQUIRE_RC(KConfigCommit(kfg));
    REQUIRE_RC(KConfigRelease(kfg));

    // load the changes from the new location
    REQUIRE_RC(KConfigMake(&kfg,wd));
    LoadFile(FullMagicPath.c_str());

    // verify changes
    REQUIRE(ValueMatches("double/quote", "\""));
    REQUIRE(ValueMatches("escaped/hex", "\x0a"));

    REQUIRE_RC(KDirectoryRemove(wd, true, LocalSettingsFile));
}

FIXTURE_TEST_CASE(DropAllChildren, KfgFixture)
{
    CreateAndLoad(GetName(), "parent/one='1'\nparent/two='2'");
    GetNode("parent");
    struct KNamelist *children;
    REQUIRE_RC(KConfigNodeListChildren(node, &children));
    uint32_t count;
    REQUIRE_RC(KNamelistCount(children, &count));
    REQUIRE_EQ(count, (uint32_t)2);
    KNamelistRelease(children);

    REQUIRE_RC(KConfigNodeDropAll((KConfigNode*)node));
    REQUIRE_RC(KConfigNodeListChildren(node, &children));
    REQUIRE_RC(KNamelistCount(children, &count));
    REQUIRE_EQ(count, (uint32_t)0);
    KNamelistRelease(children);
}

FIXTURE_TEST_CASE(FixUserSettings, KfgFixture)
{   // fix spelling of nodes dbGap-<number> to dbGaP-<number>

    // read values from a local .mkfg file;
    // if anything is changed the new values will be saved in the user's "global" .mkfg

    string mkfgFilename = "./user-settings.mkfg";
    // fake global settings since we do not want to modify the real ~/.ncbi/user-settings.mkfg
    string globalMkfgFilename = "./global-settings.mkfg";

    REQUIRE_RC(KConfigRelease(kfg));
    // create the local .mkfg with the old spelling of dbGap;
    MakeFile(mkfgFilename.c_str(),
                    (string("NCBI_SETTINGS=\"") + globalMkfgFilename + "\"\n"
                     "/repository/user/protected/dbGap-123=\"qq\"\n").c_str());
    REQUIRE_RC(KConfigMake ( & kfg, wd )); // this should load the local .mkfg, replace "dbGap" with "dbGaP" and update the global .mkfg

    // verify corrected value in memory
    REQUIRE(ValueMatches("/repository/user/protected/dbGaP-123", "qq"));

    // verify that user's global mkfg has been updated
    string globalContent = ReadContent(globalMkfgFilename);
    REQUIRE_NE(globalContent.find("/repository/user/protected/dbGaP-123 = \"qq\""), string::npos);

    REQUIRE_RC(KDirectoryRemove(wd, true, globalMkfgFilename.c_str()));
    REQUIRE_RC(KDirectoryRemove(wd, true, mkfgFilename.c_str()));
}

//////////////////////////////////////////// KConfig Accessors

FIXTURE_TEST_CASE(ConfigAccess_NullPath, KfgFixture)
{   // NULL is ok
    const char* contents="bool/f=\"FALSE\"\n";
    CreateAndLoad(GetName(), contents);
    REQUIRE_RC ( KConfigOpenNodeRead ( kfg, (const KConfigNode**)&node, NULL) );
}

FIXTURE_TEST_CASE(ConfigAccess_NonUtf8_Comment_InPath, KfgFixture)
{
    const char* contents="bool/f=\"FALSE\"\n";
    CreateAndLoad(GetName(), contents);
    REQUIRE_RC ( KConfigRead ( kfg, "#\377", 0, buf, sizeof buf, & num_read, & num_writ ) );
}

FIXTURE_TEST_CASE(ConfigAccessBool, KfgFixture)
{
    const char* contents=
        "bool/f=\"FALSE\"\n"
        "bool/t=\"true\"\n"
        "bool=\"dunno\"\n"
        ;
    CreateAndLoad(GetName(), contents);
    bool b = true;
    REQUIRE_RC(KConfigReadBool(kfg, "bool/f", &b));
    REQUIRE(! b);
    REQUIRE_RC(KConfigReadBool(kfg, "bool/t", &b));
    REQUIRE(b);
    REQUIRE_RC_FAIL(KConfigReadBool(kfg, "bool", &b));
}

FIXTURE_TEST_CASE(ConfigAccessInt, KfgFixture)
{
    const char* contents=
        "int/i1=\"100\"\n"
        "int/i2=\"-100000000000\"\n"
        "int=\"0dunno\"\n"
        ;
    CreateAndLoad(GetName(), contents);
    int64_t i = 0;
    REQUIRE_RC(KConfigReadI64(kfg, "int/i1", &i));
    REQUIRE_EQ(i, INT64_C(100));
    REQUIRE_RC(KConfigReadI64(kfg, "int/i2", &i));
    REQUIRE_EQ(i, INT64_C(-100000000000));
    REQUIRE_RC_FAIL(KConfigReadI64(kfg, "int", &i));
}

FIXTURE_TEST_CASE(ConfigAccessUnsigned, KfgFixture)
{
    const char* contents=
        "uint/i1=\"100000000000\"\n"
        "uint=\"1dunno\"\n"
        ;
    CreateAndLoad(GetName(), contents);
    uint64_t i = 0;
    REQUIRE_RC(KConfigReadU64(kfg, "uint/i1", &i));
    REQUIRE_EQ(i, UINT64_C(100000000000));
    REQUIRE_RC_FAIL(KConfigReadU64(kfg, "uint", &i));
}

FIXTURE_TEST_CASE(ConfigAccessF64, KfgFixture)
{
    const char* contents=
        "f64/i1=\"3.14\"\n"
        "f64=\"2.3dunno\"\n"
        ;
    CreateAndLoad(GetName(), contents);
    double f = 0.0;
    REQUIRE_RC(KConfigReadF64(kfg, "f64/i1", &f));
    REQUIRE_CLOSE(f, 3.14, 0.001);
    REQUIRE_RC_FAIL(KConfigReadF64(kfg, "v64", &f));
}

FIXTURE_TEST_CASE(ConfigAccessVPath, KfgFixture)
{
    // example from vfs/path.h
    // don't know where this comes from...
    #define VPATH "ncbi-file:///c/scanned-data/0001/file.sra?enc&pwd-file=/c/Users/JamesMcCoy/ncbi.pwd"

    const char* contents=
        "vpath/i1=\"" VPATH "\"\n"
        ;

    CreateAndLoad(GetName(), contents);
    REQUIRE_RC(KConfigReadVPath(kfg, "vpath/i1", &path));
    VPathReadUri(path, buf, sizeof(buf), &num_read);
    REQUIRE_EQ(string(buf), string(VPATH));
}

FIXTURE_TEST_CASE(ConfigAccessString, KfgFixture)
{
    // another change to get rid of C:
    // also, can't have multiple '?' in query
    #define STRING "ncbi-file:///c/scanned-data/0001/file.sra?enc&pwd-file=/c/Users/JamesMcCoy/ncbi.pwd"
    const char* contents=
        "string/i1=\"" STRING "\"\n"
        ;

    CreateAndLoad(GetName(), contents);
    String* str;
    REQUIRE_RC(KConfigReadString(kfg, "string/i1", &str));
    REQUIRE_NOT_NULL(str);
    REQUIRE_EQ(string(str->addr), string(STRING));
    StringWhack(str);
}

//////////////////////////////////////////// KConfigNode Accessors

FIXTURE_TEST_CASE(ConfigNodeAccessBool, KfgFixture)
{
    const char* contents=
        "bool/f=\"FALSE\"\n"
        ;
    CreateAndLoad(GetName(), contents);
    bool b = true;
    REQUIRE_RC(KConfigNodeReadBool(GetNode("bool/f"), &b));
    REQUIRE(! b);
}

FIXTURE_TEST_CASE(ConfigNodeAccessInt, KfgFixture)
{
    const char* contents=
        "int/i1=\"100\"\n"
        ;
    CreateAndLoad(GetName(), contents);
    int64_t i = 0;
    REQUIRE_RC(KConfigNodeReadI64(GetNode("int/i1"), &i));
    REQUIRE_EQ(i, (int64_t)100);
}

FIXTURE_TEST_CASE(ConfigNodeAccessUnsigned, KfgFixture)
{
    const char* contents=
        "uint/i1=\"100000000000\"\n"
        ;
    CreateAndLoad(GetName(), contents);
    uint64_t i = 0;
    REQUIRE_RC(KConfigNodeReadU64(GetNode("uint/i1"), &i));
    REQUIRE_EQ(i, UINT64_C(100000000000));
}

FIXTURE_TEST_CASE(ConfigNodeAccessF64, KfgFixture)
{
    const char* contents=
        "f64/i1=\"3.14\"\n"
        ;
    CreateAndLoad(GetName(), contents);
    double f = 0.0;
    REQUIRE_RC(KConfigNodeReadF64(GetNode("f64/i1"), &f));
    REQUIRE_CLOSE(f, 3.14, 0.001);
}

FIXTURE_TEST_CASE(ConfigNodeAccessVPath, KfgFixture)
{
    // example from vfs/path.h
    // changing
    #define VPATH "ncbi-file:///c/scanned-data/0001/file.sra?enc&pwd-file=/c/Users/JamesMcCoy/ncbi.pwd"

    const char* contents=
        "vpath/i1=\"" VPATH "\"\n"
        ;

    CreateAndLoad(GetName(), contents);
    REQUIRE_RC(KConfigNodeReadVPath(GetNode("vpath/i1"), &path));
    VPathReadUri(path, buf, sizeof(buf), &num_read);
    REQUIRE_EQ(string(buf), string(VPATH));
}

FIXTURE_TEST_CASE(ConfigNodeAccessString, KfgFixture)
{
    #define STRING "ncbi-file:///c/scanned-data/0001/file.sra?enc&pwd-file=/c/Users/JamesMcCoy/ncbi.pwd"
    const char* contents=
        "string/i1=\"" STRING "\"\n"
        ;

    CreateAndLoad(GetName(), contents);
    String* str;
    REQUIRE_RC(KConfigNodeReadString(GetNode("string/i1"), &str));
    REQUIRE_NOT_NULL(str);
    REQUIRE_EQ(string(str->addr), string(STRING));
    StringWhack(str);
}
#endif

//////////////////////////////////////////// Importing external objects

namespace {
    class C {
    public:
        static void t(const string &ngcPath) {
        }
    };
}

class Cleaner {
    KDirectory *dir;

    const char *home;

    const bool ncbi;
    const bool dbGaP;
    const bool enKey;

    static const char* Ncbi (void) { return  "ncbi"                   ; }
    static const char* DbGaP(void) { return  "ncbi/dbGaP-2956"        ; }
    static const char* EnKey(void) { return ".ncbi/dbGaP-2956.enc_key"; }

public:
    Cleaner(KDirectory *d)
        : dir(d), home(getenv("HOME"))
        , ncbi (KDirectoryPathType(dir, "%s/%s", home, Ncbi ()) != kptNotFound)
        , dbGaP(KDirectoryPathType(dir, "%s/%s", home, DbGaP()) != kptNotFound)
        , enKey(KDirectoryPathType(dir, "%s/%s", home, EnKey()) != kptNotFound)
    {}

    ~Cleaner() {
        if (!dbGaP)
            KDirectoryRemove(dir, false, "%s/%s", home, DbGaP());
        if (!ncbi)
            KDirectoryRemove(dir, false, "%s/%s", home, Ncbi ());
        if (!enKey)
            KDirectoryRemove(dir, false, "%s/%s", home, EnKey());
    }
};

#ifdef ALL
FIXTURE_TEST_CASE(KConfigImportNgc_Basic, KfgFixture)
{
    string s(GetName());
    cout << "FIXTURE_TEST_CASE(KConfigImportNgc_Basic) " << s << "\n";
    TEST_MESSAGE(s);
    CreateAndLoad(s.c_str(), "\n");
    TEST_MESSAGE("CreateAndLoad" << " " << s);

    const char* newRepo;

//    KDbgSetModConds(DBG_KFG, DBG_FLAG(DBG_KFG), DBG_FLAG(DBG_KFG));

    TEST_MESSAGE("KConfigImportNgc");
    string ngcPath("./prj_2956.ngc");
    C::t(ngcPath);
    Cleaner cleaner(wd);
    REQUIRE_RC(KConfigImportNgc(kfg, ngcPath.c_str(), "repos/ngc/", &newRepo));
    TEST_MESSAGE("KConfigImportNgc(" << ngcPath << ")");
    // contents of the input file:
    // id = "2956"
    // encyption key = "12"
    // download ticket = "943EF2DF-8C5E-4B8F-9D29-4BE5C1837798"
    // description = "SRA Test"
    //
    // The resulting KConfig subtree:
    // (root node=) /repository/user/protected/dbGaP-2956/...
    // .../encryption-key-path = "$(NCBI_HOME)/dbGaP-2956.enc_key"   (creates the file, writes the key into it with \n appended)
    // .../apps/sra/volumes/sraFlat="sra"
    // .../cache-enabled = "true"
    // .../root = "repos/ngc/" ; creates the dir
    //
    // also, creates the following node if it is not present
    // /repository/remote/protected/CGI/resolver-cgi="https://trace.ncbi.nlm.nih.gov/Traces/names/names.fcgi"
    {
        REQUIRE_RC(KConfigNodeReadVPath(GetNode("/repository/user/protected/dbGaP-2956/encryption-key-path"), &path));
        VPathReadPath(path, buf, sizeof(buf), &num_read);
        string encFileName = "/dbGaP-2956.enc_key";
        REQUIRE_EQ(string(buf).rfind(encFileName), num_read - encFileName.size()); // string(buf) ends with encFileName
        VPathRelease(path);
        path = 0;

        REQUIRE_EQ(ReadContent(buf), string("12\n"));
    }

    {
        String* str;
        REQUIRE_RC(KConfigNodeReadString(GetNode("/repository/user/protected/dbGaP-2956/apps/sra/volumes/sraFlat"), &str));
        REQUIRE_EQ(string(str->addr), string("sra"));
        StringWhack(str);
    }

    {
        bool b = true;
        REQUIRE_RC(KConfigNodeReadBool(GetNode("/repository/user/protected/dbGaP-2956/cache-enabled"), &b));
        REQUIRE(b);
    }

    {
        REQUIRE_RC(KConfigNodeReadVPath(GetNode("/repository/user/protected/dbGaP-2956/root"), &path));
        VPathReadPath(path, buf, sizeof(buf), &num_read);
        REQUIRE_EQ(string(buf), string("repos/ngc/"));
        VPathRelease(path);
        path = 0;

        const KDirectory *sub;
        REQUIRE_RC(KDirectoryOpenDirRead(wd, &sub, false, buf));
        REQUIRE_RC(KDirectoryRelease(sub));
        REQUIRE_RC(KDirectoryRemove(wd, true, buf));
    }

    {
        const String* str;
        REQUIRE_RC(KConfigNodeReadVPath(GetNode("/repository/remote/protected/CGI/resolver-cgi"), &path));
        REQUIRE_RC(VPathMakeString(path, &str));
        REQUIRE_EQ(string(str->addr),
            string("https://trace.ncbi.nlm.nih.gov/Traces/names/names.fcgi"));
        StringWhack(str);
    }

    REQUIRE_EQ(string(newRepo), string("repos/ngc/"));

    REQUIRE_RC(KDirectoryRemove(wd, true, "repos"));
}
#endif

FIXTURE_TEST_CASE(KConfigImportNgc_NullLocation, KfgFixture)
{
    CreateAndLoad(GetName(), "\n");
    const char* newRepo;
    Cleaner cleaner(wd);
    REQUIRE_RC(KConfigImportNgc(kfg, "./prj_2956.ngc", NULL, &newRepo));
    string encDirName = "/ncbi/dbGaP-2956";
    REQUIRE_EQ(string(newRepo).rfind(encDirName), string(newRepo).size() - encDirName.size()); // string(buf) ends with encDirName
}
FIXTURE_TEST_CASE(KConfigImportNgc_NullLocation_NullNewRepo, KfgFixture)
{
    CreateAndLoad(GetName(), "\n");
    Cleaner cleaner(wd);
    REQUIRE_RC(KConfigImportNgc(kfg, "./prj_2956.ngc", NULL, NULL));
}

//////////////////////////////////////////// Main
static rc_t argsHandler(int argc, char* argv[]) {
    Args* args = NULL;
    rc_t rc = ArgsMakeAndHandle(&args, argc, argv, 0, NULL, 0);
    ArgsWhack(args);
    return rc;
}

extern "C"
{

#include <kapp/args.h>

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

const char UsageDefaultName[] = "test-kfg";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KfgTestSuite(argc, argv);
    return rc;
}

}
