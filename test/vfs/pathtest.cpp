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

/**
* Unit tests for VPath interface
*/


#include "../../libs/vfs/path-priv.h"

#include <klib/text.h>

#include <ktst/unit_test.hpp>

#include <vfs/manager.h>
#include <vfs/path.h>
#include <vfs/path-priv.h>

#include <stdexcept>

#include <cstdlib>

#include <climits> /* PATH_MAX */
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#include <sysalloc.h>


#define IGNORE_FAILURE_VDB_1551 0


TEST_SUITE(VPathTestSuite);

using namespace std;

class PathFixture
{
public:
    PathFixture()
        : path(0)
        , vfs ( 0 )
    {
        rc_t rc = VFSManagerMake ( & vfs );
        if ( rc != 0 )
            throw logic_error ( "PathFixture: VFSManagerMake failed" );
    }
    ~PathFixture()
    {
        VFSManagerRelease ( vfs );
        if (path && VPathRelease(path) != 0)
           throw logic_error("~PathFixture: VPathRelease failed");
    }
    string PathToString(const string& p_path)
    {
        if (VFSManagerMakePath(vfs, &path, p_path.c_str()))
           throw logic_error("PathToString: VPathMake failed");
        const String* uri;
        if ( VPathMakeString(path, &uri) != 0 )
           throw logic_error("PathToString: VPathMakeString failed");
        
        string ret = string(uri->addr, uri->size);
        
        free((void*)uri);
        if (VPathRelease(path))
           throw logic_error("PathToString: VPathRelease failed");
        path = 0;
          
        return ret;
    }
    
    VPath* path;
    VFSManager * vfs;
    
    static const int BufSize = 1024;
    char buf[BufSize]; 
    size_t num_read;
};

FIXTURE_TEST_CASE(ReadPath, PathFixture)
{
    REQUIRE_RC(VFSManagerMakePath ( vfs, &path, "ncbi-file:qq?enc"));

    REQUIRE_RC(VPathReadPath(path, buf, BufSize, &num_read));
    REQUIRE_EQ(string(buf, num_read), string("qq"));
}

FIXTURE_TEST_CASE(MakeStringPlain, PathFixture)
{
    string filename = "filename";
    REQUIRE_EQ(PathToString(filename), filename);
}

FIXTURE_TEST_CASE(MakeStringScheme, PathFixture)
{
    string filename = "ncbi-acc:filename";
    REQUIRE_EQ(PathToString(filename), filename);
}

FIXTURE_TEST_CASE(MakeStringBadScheme, PathFixture)
{
    string filename = "ncbi-file:filename";
    REQUIRE_EQ(PathToString(filename), filename);
}

FIXTURE_TEST_CASE(GetScheme, PathFixture)
{
    REQUIRE_RC(VFSManagerMakePath ( vfs, &path, "ncbi-file:qq?enc"));

    String sch;
    REQUIRE_RC(VPathGetScheme(path, &sch));
    REQUIRE_EQ(string(sch.addr, sch.size), string("ncbi-file"));
}

FIXTURE_TEST_CASE(GetScheme_t, PathFixture)
{
    REQUIRE_RC(VFSManagerMakePath ( vfs, &path, "ncbi-file:qq?enc"));

    VPUri_t uri_type;
    REQUIRE_RC(VPathGetScheme_t(path, &uri_type));
    REQUIRE_EQ(uri_type, (VPUri_t)vpuri_ncbi_file);
}

FIXTURE_TEST_CASE(GetScheme_NcbiObj, PathFixture)
{
    REQUIRE_RC(VFSManagerMakePath ( vfs, &path, "ncbi-obj:12/1/345"));
    
    VPUri_t uri_type;
    REQUIRE_RC(VPathGetScheme_t(path, &uri_type));
    REQUIRE_EQ(uri_type, (VPUri_t)vpuri_ncbi_obj);

    String sch;
    REQUIRE_RC(VPathGetScheme(path, &sch));
    REQUIRE_EQ(string(sch.addr, sch.size), string("ncbi-obj"));
    
    REQUIRE_RC(VPathReadPath(path, buf, BufSize, &num_read));
    REQUIRE_EQ(string(buf, num_read), string("12/1/345"));
}

#if WINDOWS

//  VPathMakeSysPath

FIXTURE_TEST_CASE( VFS_Native2Internal_1, PathFixture )
{
    cout << "VFSManagerMakeSysPath(native) -> VPathMakeString(internal)\n";
    REQUIRE_RC(
        VFSManagerMakeSysPath( vfs, &path, "C:\\somepath\\somefile.something"));
    
    const String *uri = NULL;
    REQUIRE_RC( VPathMakeString( path, &uri ) );
    REQUIRE_NOT_NULL( uri );
    REQUIRE_EQ( string( "/C/somepath/somefile.something" ),
                string( uri->addr, uri->size ) );
}

FIXTURE_TEST_CASE( VFS_Native2Internal_2, PathFixture )
{
    cout << "VFSManagerMakeSysPath(native) -> VPathReadPath(internal)" << endl;
    REQUIRE_RC( VFSManagerMakeSysPath ( vfs, &path, "C:\\somepath\\somefile.something" ) );

    char buffer[ 1024 ];
    size_t num_writ;
    REQUIRE_RC( VPathReadPath( path, buffer, sizeof buffer, &num_writ ) );
    REQUIRE_EQ( string( "/C/somepath/somefile.something" ), string ( buffer, num_writ ) );
}

FIXTURE_TEST_CASE(VFS_Native2InternalNetwork, PathFixture) {
    const string n("\\\\abc\\def\\ghi\\000379\\SRR388696");
    const string p("//abc/def/ghi/000379/SRR388696");

    {
        REQUIRE_RC(VFSManagerMakeSysPath(vfs, &path, n.c_str()));

        const String *uri = NULL;
        REQUIRE_RC(VPathMakeString(path, &uri));
        REQUIRE_NOT_NULL(uri);
#if IGNORE_FAILURE_VDB_1551
        REQUIRE_EQ(p, string(uri->addr, uri->size));
#endif

        char buffer[PATH_MAX] = "";
        size_t num_writ = 0;
        REQUIRE_RC(VPathReadPath(path, buffer, sizeof buffer, &num_writ));
#if IGNORE_FAILURE_VDB_1551
        REQUIRE_EQ(p, string(buffer, num_writ));
#endif

        REQUIRE_RC(VPathRelease(path));
        path = NULL;
    }
    {
        REQUIRE_RC( VFSManagerMakePath(vfs, &path, p.c_str()));

        const String *uri = NULL;
        REQUIRE_RC(VPathMakeSysPath(path, &uri));
        REQUIRE_NOT_NULL(uri);
#if IGNORE_FAILURE_VDB_1551
        REQUIRE_EQ(n, string(uri->addr, uri->size));

        char buffer[PATH_MAX] = "";
        size_t num_writ = 0;
        REQUIRE_RC(VPathReadSysPath(path, buffer, sizeof buffer, &num_writ));
        REQUIRE_EQ(n, string(buffer, num_writ));
#endif
    }
}

//  VPathMakePath

FIXTURE_TEST_CASE( VFS_Internal2Native_1, PathFixture )
{
    cout << "VFSManagerMakePath(internal) -> VPathReadSysPath(native)" << endl;
    REQUIRE_RC( VFSManagerMakePath ( vfs, &path, "/C/somepath/somefile.something" ) );

    const String * uri;
    REQUIRE_RC( VPathMakeSysPath( path, &uri ) );
    REQUIRE_NOT_NULL( uri );
    REQUIRE_EQ( string( "C:\\somepath\\somefile.something" ), string ( uri->addr, uri->size ) );
}

FIXTURE_TEST_CASE( VFS_Internal2Native_2, PathFixture )
{
    cout << "VFSManagerMakePath(internal) -> VPathReadSysPath(native)" << endl;
    REQUIRE_RC( VFSManagerMakePath ( vfs, &path, "/C/somepath/somefile.something" ) );

    char buffer[ 1024 ];
    size_t num_writ;
    REQUIRE_RC( VPathReadSysPath( path, buffer, sizeof buffer, &num_writ ) );
    REQUIRE_EQ( string( "C:\\somepath\\somefile.something" ), string ( buffer, num_writ ) );
}

#endif // WINDOWS

//TODO:
//  VPathGetPath

// Functions from path-priv.h
    
FIXTURE_TEST_CASE(Option_Encrypt, PathFixture)
{
    REQUIRE_RC(VFSManagerMakePath ( vfs, &path, "ncbi-file:qq?enc"));
    
    REQUIRE_RC(VPathOption (path, vpopt_encrypted, buf, BufSize, &num_read));
    REQUIRE_EQ(num_read, (size_t)0);
}

//TODO:
// VPathMakeFmt
// VPathMakeVFmt
// VPathMakeRelative 
// VPathMakeRelativeFmt 
// VPathVMakeRelativeFmt 
// VPathMakeCurrentPath 
// VPathMakeURI 
// VPathGetCWD

FIXTURE_TEST_CASE(MarkHighReliability, PathFixture)
{
    REQUIRE_RC ( VFSManagerMakePath ( vfs, &path, "ncbi-file:qq?enc" ) );
    REQUIRE ( ! VPathIsHighlyReliable ( path ) );
    
    REQUIRE_RC ( VPathMarkHighReliability(path, true) );
    REQUIRE ( VPathIsHighlyReliable ( path ) );
    
    REQUIRE_RC ( VPathMarkHighReliability(path, false) );
    REQUIRE ( ! VPathIsHighlyReliable ( path ) );
}

FIXTURE_TEST_CASE(NAME_SERVER_PROTECTED_HTTP, PathFixture) {
    {
#define HOST "gap-download.ncbi.nlm.nih.gov"
#define PATH "/1234ABCD-22BB-CC33-4C4C-D5E6F7890A1B/SRR123456.sra"
#undef URL
#define URL "http://" HOST PATH
#define TIC "1A2B3C4D-2B3C-4D5E-6F78-90A1B23C4D5E"
        String download_ticket, url;
        CONST_STRING(&download_ticket, TIC);
        CONST_STRING(&url, URL);
        REQUIRE_RC(VPathMakeFmt(&path, "%S?tic=%S", &url, &download_ticket));
        REQUIRE(path);
    }
/*  {   TODO VFSManagerExtractAccessionOrOID should extract SRR123456
        VPath *acc_or_oid = NULL;
        REQUIRE_RC(VFSManagerExtractAccessionOrOID(vfs, &acc_or_oid, path));
        REQUIRE(acc_or_oid);
        REQUIRE_RC(VPathRelease(acc_or_oid));
        acc_or_oid = NULL;
    }*/
    REQUIRE(!VPathIsAccessionOrOID(path));
    REQUIRE(VPathIsFSCompatible(path));
    REQUIRE(VPathFromUri(path));
    REQUIRE(!VPathIsHighlyReliable(path));

    size_t num_read = 0;
    {
        const string e(URL "?tic=" TIC);
        char buffer[4096] = "";
        REQUIRE_RC(VPathReadUri(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
        {
            const String *str = NULL;
            REQUIRE_RC(VPathMakeUri(path, &str));
            REQUIRE(str);
            REQUIRE_EQ(string(str->addr), e);
            REQUIRE_EQ(str->size, e.size());
            REQUIRE_EQ(str->size, (size_t)str->len);
            free(const_cast<String*>(str));
        }
        {
            const String *str = NULL;
            REQUIRE_RC(VPathMakeString(path, &str));
            REQUIRE(str);
            REQUIRE_EQ(string(str->addr), e);
            REQUIRE_EQ(str->size, e.size());
            REQUIRE_EQ(str->size, (size_t)str->len);
            free(const_cast<String*>(str));
        }
    }
    {
        char buffer[4096] = "";
        const string e("http");
        REQUIRE_RC(VPathReadScheme(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
        {
            String str;
            REQUIRE_RC(VPathGetScheme(path, &str));
            REQUIRE_EQ(string(str.addr, 0, str.len), e);
            REQUIRE_EQ(str.size, e.size());
            REQUIRE_EQ(str.size, (size_t)str.len);
        }
    }
    {
        char buffer[4096] = "";
        const string e;
        REQUIRE_RC(VPathReadAuth(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
        {
            String str;
            REQUIRE_RC(VPathGetAuth(path, &str));
            REQUIRE(! str.addr);
            REQUIRE_EQ(str.size, e.size());
            REQUIRE_EQ(str.size, (size_t)str.len);
        }
    }
    {
        char buffer[4096] = "";
        const string e(HOST);
        REQUIRE_RC(VPathReadHost(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
        {
            String str;
            REQUIRE_RC(VPathGetHost(path, &str));
            REQUIRE_EQ(string(str.addr, 0, str.len), e);
            REQUIRE_EQ(str.size, e.size());
            REQUIRE_EQ(str.size, (size_t)str.len);
        }
    }
    {
        char buffer[4096] = "";
        const string e;
        REQUIRE_RC(VPathReadPortName(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
        {
            String str;
            REQUIRE_RC(VPathGetPortName(path, &str));
            REQUIRE(! str.addr);
            REQUIRE_EQ(str.size, e.size());
            REQUIRE_EQ(str.size, (size_t)str.len);
            REQUIRE( ! VPathGetPortNum(path));
        }
    }
    {
        char buffer[4096] = "";
        const string e(PATH);
        REQUIRE_RC(VPathReadPath(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
        {
            String str;
            REQUIRE_RC(VPathGetPath(path, &str));
            REQUIRE_EQ(string(str.addr, 0, str.len), e);
            REQUIRE_EQ(str.size, e.size());
            REQUIRE_EQ(str.size, (size_t)str.len);
        }
    }
    {
        char buffer[4096] = "";
        const string e(PATH);
        REQUIRE_RC(VPathReadSysPath(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
        {
            const String *s = NULL;
            REQUIRE_RC(VPathMakeSysPath(path, &s));
            REQUIRE(s);
            REQUIRE_EQ(string(s->addr), e);
            REQUIRE_EQ(s->size, e.size());
            REQUIRE_EQ(s->size, (size_t)s->len);
            free(const_cast<String*>(s));
        }
    }
    {
        char buffer[4096] = "";
        const string e("tic=" TIC);
        REQUIRE_RC(VPathReadQuery(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size()); REQUIRE_EQ(string(buffer), e);
        {
            String str;
            REQUIRE_RC(VPathGetQuery(path, &str));
            REQUIRE_EQ(string(str.addr, 0, str.len), e);
            REQUIRE_EQ(str.size, e.size());
            REQUIRE_EQ(str.size, (size_t)str.len);
        }
    }
    {
        char buffer[4096] = "";
        const string e(TIC);
        REQUIRE_RC
            (VPathReadParam(path, "tic", buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size()); REQUIRE_EQ(string(buffer), e);
        {
            String str;
            REQUIRE_RC(VPathGetParam(path, "tic", &str));
            REQUIRE_EQ(string(str.addr, 0, str.len), e);
            REQUIRE_EQ(str.size, e.size());
            REQUIRE_EQ(str.size, (size_t)str.len);
        }
    }
        REQUIRE(num_read);
    {
        char buffer[4096] = "";
        const string e;
        REQUIRE_RC_FAIL
            (VPathReadParam(path, "ti", buffer, sizeof buffer, &num_read));
        REQUIRE(! num_read);
        {
            String str;
            REQUIRE_RC_FAIL(VPathGetParam(path, "ti", &str));
        }
    }
    {
        char buffer[4096] = "";
        const string e;
        REQUIRE_RC(VPathReadFragment(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size()); REQUIRE_EQ(string(buffer), e);
        {
            String str;
            REQUIRE_RC(VPathGetFragment(path, &str));
            REQUIRE_EQ(str.addr, (const char*)NULL);
            REQUIRE_EQ(str.size, e.size());
            REQUIRE_EQ(str.size, (size_t)str.len);
        }
    }

    REQUIRE( ! VPathGetOid(path));
}

FIXTURE_TEST_CASE(NAME_SERVER_PROTECTED_FASP, PathFixture) {
    {
#undef URL
#define URL "fasp://dbtest@gap-download.ncbi.nlm.nih.gov:data/sracloud/1234ABCD-22BB-CC33-4C4C-D5E6F7890A1B/SRR123456.sra"
        String download_ticket, url;
        CONST_STRING(&download_ticket, TIC);
        CONST_STRING(&url, URL);
        REQUIRE_RC(VPathMakeFmt(&path, "%S?tic=%S", &url, &download_ticket));
        REQUIRE(path);
    }
    {
        const string e(URL "?tic=" TIC);
        char buffer[4096] = "";
        size_t num_read = 0;
        REQUIRE_RC(VPathReadUri(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
    }
}

FIXTURE_TEST_CASE(FILENAME, PathFixture) {
#undef PATH
#define PATH "file.txt"
    REQUIRE_RC(VFSManagerMakePath(vfs, &path, "%s", PATH));
    REQUIRE(path);
    {
        char buffer[4096] = "";
        const string e("file");
        REQUIRE_RC(VPathReadScheme(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
    }
    {
        char buffer[4096] = "";
        const string e(PATH);
        REQUIRE_RC(VPathReadPath(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
    }
}

FIXTURE_TEST_CASE(SPACE_IN_FILENAME, PathFixture) {
#undef PATH
#define PATH "file elif.txt"
    REQUIRE_RC(VFSManagerMakePath(vfs, &path, "%s", PATH));
    REQUIRE(path);
    {
        char buffer[4096] = "";
        const string e("file");
        REQUIRE_RC(VPathReadScheme(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
    }
    {
        char buffer[4096] = "";
        const string e(PATH);
        REQUIRE_RC(VPathReadPath(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
    }
}

FIXTURE_TEST_CASE(CYRYLLIC_WIN_IN_FILENAME, PathFixture) {
#undef PATH
#define PATH "./:OJDF800"
    REQUIRE_RC(VFSManagerMakePath(vfs, &path, "%s", PATH));
    REQUIRE(path);
    {
        char buffer[4096] = "";
        const string e("file");
        REQUIRE_RC(VPathReadScheme(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
    }
    {
        char buffer[4096] = "";
        const string e(PATH);
        REQUIRE_RC(VPathReadPath(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
    }
}

FIXTURE_TEST_CASE(CYRYLLIC_IN_FILENAME, PathFixture) {
#undef PATH
//   #define PATH "Ä°¹».txt"
#define PATH "\xC4\xB0\xB9\xBB.txt"
    REQUIRE_RC(VFSManagerMakePath(vfs, &path, "%s", PATH));
    REQUIRE(path);
    {
        char buffer[4096] = "";
        const string e("file");
        REQUIRE_RC(VPathReadScheme(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
    }
    {
        char buffer[4096] = "";
        const string e(PATH);
        REQUIRE_RC(VPathReadPath(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
    }
}

FIXTURE_TEST_CASE(TAB_IN_FILENAME, PathFixture) {
#undef PATH
#define PATH "file\telif.txt"
    REQUIRE_RC(VFSManagerMakePath(vfs, &path, "%s", PATH));
    REQUIRE(path);
    {
        char buffer[4096] = "";
        const string e("file");
        REQUIRE_RC(VPathReadScheme(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
    }
    {
        char buffer[4096] = "";
        const string e(PATH);
        REQUIRE_RC(VPathReadPath(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
    }
}

FIXTURE_TEST_CASE(POUNT_IN_FILENAME, PathFixture) {
#undef PATH
#define PATH "./file#elif.txt"
    REQUIRE_RC(VFSManagerMakePath(vfs, &path, "%s", PATH));
    REQUIRE(path);
    {
        char buffer[4096] = "";
        const string e("file");
        REQUIRE_RC(VPathReadScheme(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
    }
    {
        char buffer[4096] = "";
        const string e(PATH);
        REQUIRE_RC(VPathReadPath(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
    }
}

FIXTURE_TEST_CASE(QUESTION_IN_FILENAME, PathFixture) {
#undef PATH
#define PATH "./file?elif.txt"
    REQUIRE_RC(VFSManagerMakePath(vfs, &path, "%s", PATH));
    REQUIRE(path);
    {
        char buffer[4096] = "";
        const string e("file");
        REQUIRE_RC(VPathReadScheme(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
    }
    {
        char buffer[4096] = "";
        const string e(PATH);
        REQUIRE_RC(VPathReadPath(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
    }
}

FIXTURE_TEST_CASE(COLON_IN_FILENAME, PathFixture) {
#undef PATH
#define PATH "./file:elif.txt"
    REQUIRE_RC(VFSManagerMakePath(vfs, &path, "%s", PATH));
    REQUIRE(path);
    {
        char buffer[4096] = "";
        const string e("file");
        REQUIRE_RC(VPathReadScheme(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
    }
    {
        char buffer[4096] = "";
        const string e(PATH);
        REQUIRE_RC(VPathReadPath(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
    }
}

FIXTURE_TEST_CASE(UTF8_FILENAME, PathFixture) {
#undef PATH
#define PATH "f\xD0\xA4" "e.txt"
    REQUIRE_RC(VFSManagerMakePath(vfs, &path, "%s", PATH));
    REQUIRE(path);
    {
        char buffer[4096] = "";
        const string e("file");
        REQUIRE_RC(VPathReadScheme(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
    }
    {
        char buffer[4096] = "";
        const string e(PATH);
        REQUIRE_RC(VPathReadPath(path, buffer, sizeof buffer, &num_read));
        REQUIRE_EQ(num_read, e.size());
        REQUIRE_EQ(string(buffer), e);
    }
}

FIXTURE_TEST_CASE(Http, PathFixture)
{
#undef SRC
#define SRC "http://u@h.d:9/d/f"
    REQUIRE_RC(VFSManagerMakePath ( vfs, &path, SRC));
    char buffer[4096] = "";
    size_t num_read = 0;
    REQUIRE_RC(VPathReadUri(path, buffer, sizeof buffer, &num_read));
    REQUIRE_EQ(num_read, sizeof SRC - 1);
    REQUIRE_EQ(string(buffer), string(SRC));
}

FIXTURE_TEST_CASE(Fasp, PathFixture)
{
#undef SRC
#define SRC "fasp://u@hst.com:dir/file"
    REQUIRE_RC(VFSManagerMakePath ( vfs, &path, SRC));
    char buffer[4096] = "";
    size_t num_read = 0;
    REQUIRE_RC(VPathReadUri(path, buffer, sizeof buffer, &num_read));
    REQUIRE_EQ(num_read, sizeof SRC - 1);
    REQUIRE_EQ(string(buffer), string(SRC));

    string e("dir/file");
    REQUIRE_RC(VPathReadPath(path, buffer, sizeof buffer, &num_read));
    REQUIRE_EQ(num_read, e.size());
    REQUIRE_EQ(string(buffer), e);
}

FIXTURE_TEST_CASE(F_asp, PathFixture)
{
#undef SRC
#define SRC "fasp://u@hst.com:a-dir/file"
    REQUIRE_RC(VFSManagerMakePath ( vfs, &path, SRC));
    char buffer[4096] = "";
    size_t num_read = 0;
    REQUIRE_RC(VPathReadUri(path, buffer, sizeof buffer, &num_read));
    REQUIRE_EQ(num_read, sizeof SRC - 1);
    REQUIRE_EQ(string(buffer), string(SRC));

    string e("a-dir/file");
    REQUIRE_RC(VPathReadPath(path, buffer, sizeof buffer, &num_read));
    REQUIRE_EQ(num_read, e.size());
    REQUIRE_EQ(string(buffer), e);
}

FIXTURE_TEST_CASE(Fasp1G, PathFixture)
{
#undef SRC
#define SRC "fasp://u@ftp.gov:1G"
    REQUIRE_RC(VFSManagerMakePath ( vfs, &path, SRC));
    char buffer[4096] = "";
    size_t num_read = 0;
    REQUIRE_RC(VPathReadUri(path, buffer, sizeof buffer, &num_read));
    REQUIRE_EQ(num_read, sizeof SRC - 1);
    REQUIRE_EQ(string(buffer), string(SRC));

    string e("1G");
    REQUIRE_RC(VPathReadPath(path, buffer, sizeof buffer, &num_read));
    REQUIRE_EQ(num_read, e.size());
    REQUIRE_EQ(string(buffer), e);
}

FIXTURE_TEST_CASE(Pileup, PathFixture)
{
    REQUIRE_RC(VFSManagerMakePath(vfs, &path, "http://s/4.pileup"));

    String str;
    REQUIRE_RC(VPathGetPath(path, & str));
    REQUIRE_EQ ( static_cast<size_t> (str.len), str.size  );
}

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>
#include <klib/rc.h>

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
const char UsageDefaultName[] = "test-path";

static void clear_recorded_errors( void )
{
    rc_t rc;
    const char * filename;
    const char * funcname;
    uint32_t line_nr;
    while ( GetUnreadRCInfo ( &rc, &filename, &funcname, &line_nr ) )
    {
    }
}

rc_t CC KMain ( int argc, char *argv [] )
{
    rc_t rc=VPathTestSuite(argc, argv);
    clear_recorded_errors();
    return rc;
}

}
