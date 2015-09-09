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
* Unit tests for KMD5File
*/

#include <cstring>
#include <cstdio>

#include <ktst/unit_test.hpp>

#include <kfs/directory.h>
#include <kfs/impl.h>
#include <kfs/md5.h>


#define class clss
#include <kfs/fileformat.h>
#undef class

using namespace std;

TEST_SUITE(KMD5FileTestSuite);

TEST_CASE(KMD5WriteFileTruncateAfterChange)
{
    KDirectory *wd;
    REQUIRE_RC(KDirectoryNativeDir ( & wd ));

    const char* filename = "md5test_trunc.file";
    const char* md5sums_filename = "md5test_sums_trunc.file";
    const char* contents = "contents";

    {   // create temp file, close
        KFile* file;
        REQUIRE_RC( KDirectoryCreateFile(wd, &file, true, 0664, kcmInit, filename) );
        
        size_t num_writ;
        REQUIRE_RC( KFileWrite(file, 0, contents, strlen(contents), &num_writ) );
        assert(num_writ == strlen(contents));
        
        uint64_t file_size;
        REQUIRE_RC( KFileSize(file, &file_size) );
        REQUIRE( file_size == strlen(contents) );
        
        REQUIRE_RC( KFileRelease(file) );
    }

    {
        KFile * file, * md5sums_file;
        KMD5File * md5_file_obj;
        KMD5SumFmt * md5sums;
        
        REQUIRE_RC( KDirectoryCreateFile(wd, &md5sums_file, true, 0664, kcmInit, md5sums_filename) );
        REQUIRE_RC( KMD5SumFmtMakeUpdate(&md5sums, md5sums_file) );
        
        REQUIRE_RC( KDirectoryOpenFileWrite(wd, &file, true, filename) );
        
        REQUIRE_RC( KMD5FileMakeWrite(&md5_file_obj, file, md5sums, filename) );
        file = KMD5FileToKFile(md5_file_obj);
        
        size_t num_writ;
        REQUIRE_RC( KFileWrite(file, 0, contents, strlen(contents) - 2, &num_writ) );
        assert(num_writ == strlen(contents) - 2);
        
        REQUIRE_RC( KFileRelease(file) );
        REQUIRE_RC( KMD5SumFmtRelease(md5sums) );
    }
    
    {
        const KFile * file;
        REQUIRE_RC( KDirectoryOpenFileRead(wd, &file, filename) );
        
        uint64_t file_size;
        REQUIRE_RC( KFileSize(file, &file_size) );
        REQUIRE( file_size == strlen(contents) - 2 );
        
        REQUIRE_RC( KFileRelease(file) );
    }

    REQUIRE_RC( KDirectoryRemove(wd, false, filename) );
    REQUIRE_RC( KDirectoryRemove(wd, false, md5sums_filename) );

    REQUIRE_RC( KDirectoryRelease ( wd ) );
}

TEST_CASE(KMD5WriteFileNotTruncateWithoutChange)
{
    KDirectory *wd;
    REQUIRE_RC(KDirectoryNativeDir ( & wd ));
    
    const char* filename = "md5test_notrunc.file";
    const char* md5sums_filename = "md5test_sums_notrunc.file";
    const char* contents = "contents";
    
    {   // create temp file, close
        KFile* file;
        REQUIRE_RC( KDirectoryCreateFile(wd, &file, true, 0664, kcmInit, filename) );
        
        size_t num_writ;
        REQUIRE_RC( KFileWrite(file, 0, contents, strlen(contents), &num_writ) );
        assert(num_writ == strlen(contents));
        
        uint64_t file_size;
        REQUIRE_RC( KFileSize(file, &file_size) );
        REQUIRE( file_size == strlen(contents) );
        
        REQUIRE_RC( KFileRelease(file) );
    }
    
    {
        KFile * file, * md5sums_file;
        KMD5File * md5_file_obj;
        KMD5SumFmt * md5sums;
        
        REQUIRE_RC( KDirectoryCreateFile(wd, &md5sums_file, true, 0664, kcmInit, md5sums_filename) );
        REQUIRE_RC( KMD5SumFmtMakeUpdate(&md5sums, md5sums_file) );
        
        REQUIRE_RC( KDirectoryOpenFileWrite(wd, &file, true, filename) );
        
        REQUIRE_RC( KMD5FileMakeWrite(&md5_file_obj, file, md5sums, filename) );
        file = KMD5FileToKFile(md5_file_obj);
        
        REQUIRE_RC( KFileRelease(file) );
        REQUIRE_RC( KMD5SumFmtRelease(md5sums) );
    }
    
    {
        const KFile * file;
        REQUIRE_RC( KDirectoryOpenFileRead(wd, &file, filename) );
        
        uint64_t file_size;
        REQUIRE_RC( KFileSize(file, &file_size) );
        REQUIRE( file_size == strlen(contents) );
        
        REQUIRE_RC( KFileRelease(file) );
    }
    
    REQUIRE_RC( KDirectoryRemove(wd, false, filename) );
    REQUIRE_RC( KDirectoryRemove(wd, false, md5sums_filename) );
    
    REQUIRE_RC( KDirectoryRelease ( wd ) );
}

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>
#include <kfg/config.h>

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

const char UsageDefaultName[] = "test-kfs-md5";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KMD5FileTestSuite(argc, argv);
    return rc;
}

}
