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
* Unit tests for VPath interface
*/

#include <klib/text.h>
#include <vfs/manager.h>
#include <vfs/path.h>
#include <vfs/path-priv.h>
#include <ktst/unit_test.hpp>

#include "../../libs/vfs/path-priv.h"

#include <cstdlib>
#include <stdexcept>

#include <sysalloc.h>

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
FIXTURE_TEST_CASE(VFSManagerMakeSysPath_Windows_DriveLetter, PathFixture)
{
    REQUIRE_RC(VFSManagerMakeSysPath ( vfs, &path, "C:\\somepath\\somefile.something"));
    
    const String* uri;
    REQUIRE_RC ( VPathMakeString(path, &uri) );
    REQUIRE_NOT_NULL ( uri );
    REQUIRE_EQ ( string("/C/somepath/somefile.something"), string ( uri->addr, uri->size ) );
}
#endif

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



//////////////////////////////////////////// Main
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
const char UsageDefaultName[] = "test-path";

rc_t CC KMain ( int argc, char *argv [] )
{
    rc_t rc=VPathTestSuite(argc, argv);
    return rc;
}

}
