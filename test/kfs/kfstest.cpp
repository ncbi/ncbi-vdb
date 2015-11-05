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
* Unit tests for Kfs interface
*/

#include <cstring>

#include <ktst/unit_test.hpp>
#include <kfs/mmap.h>
#include <kfs/directory.h>
#include <kfs/impl.h>
#include <kfs/tar.h>

#include <kfs/ffext.h>
#include <kfs/ffmagic.h>

#define class clss
#include <kfs/fileformat.h>
#undef class

#include "../../libs/kfs/toc-priv.h"

using namespace std;

TEST_SUITE(KfsTestSuite);

TEST_CASE(KMMapMakeRead_and_KMMapRelease)
{   // create a temporary file, open it with KMMapMakeRead, close KMMap, try to delete 

    KDirectory *wd;
    REQUIRE_RC(KDirectoryNativeDir ( & wd ));

    const char* fileName="test.file";

    {   // create temp file, close
        KFile* file;
        const char* contents="contents";
        REQUIRE_RC(KDirectoryCreateFile(wd, &file, true, 0664, kcmInit, fileName));
        size_t num_writ=0;
        REQUIRE_RC(KFileWrite(file, 0, contents, strlen(contents), &num_writ));
        REQUIRE_RC(KFileRelease(file));
    }

    {   // open, memory-map, close
        const KFile* file;
        REQUIRE_RC(KDirectoryOpenFileRead(wd, &file, fileName));
        const KMMap * mm;
        REQUIRE_RC(KMMapMakeRead(&mm, file));
        REQUIRE_RC(KMMapRelease(mm));

        REQUIRE_RC(KFileRelease(file));
    }

    // now, remove the file
    // on Windows: used to return ACCESS_DENIED, not removed file 
    // (cause: no call to UnmapViewOfFile in libs\kfs\win\KMapUnmap)
    REQUIRE_RC(KDirectoryRemove(wd, false, fileName)); 

    REQUIRE_RC(KDirectoryRelease ( wd ));
}

#ifdef HAVE_KFF

TEST_CASE(ExtFileFormat)
{
    struct KFileFormat* pft;
    const char format[] = {
        "ext1\tTestFormat1\n"
        "ext2\tTestFormat2\n"
    };
    const char typeAndClass[] = {
        "TestFormat1\tTestClass1\n"
        "TestFormat2\tTestClass2\n"
    };
    REQUIRE_RC(KExtFileFormatMake(&pft, format, sizeof(format) - 1, typeAndClass, sizeof(typeAndClass) - 1));
    
    KFileFormatType type;
    KFileFormatClass clss;
    char descr[1024];
    size_t length;
    REQUIRE_RC(KFileFormatGetTypePath(pft, 
                                      NULL, // ignored
                                      "qq.ext2", 
                                      &type, 
                                      &clss,
                                      descr, 
                                      sizeof(descr),
                                      &length));    
    REQUIRE_EQ(type, 2);
    REQUIRE_EQ(clss, 2);
    REQUIRE_EQ(string(descr, length), string("TestFormat2"));
    
    REQUIRE_RC(KFileFormatGetClassDescr(pft, clss, descr, sizeof (descr)));        
    REQUIRE_EQ(string(descr), string("TestClass2"));
    REQUIRE_EQ(length, string("TestClass2").length()+1);
    REQUIRE_RC(KFileFormatRelease(pft));
}

TEST_CASE(MagicFileFormat)
{
    struct KFileFormat* pft;
    const char magic[] = 
    {
        "Generic Format for Sequence Data (SRF)\tSequenceReadFormat\n"
        "GNU tar archive\tTapeArchive\n"
    };
    const char typeAndClass[] = {
        "SequenceReadFormat\tRead\n"
        "TapeArchive\tArchive\n"
    };
    REQUIRE_RC(KMagicFileFormatMake (&pft, "/usr/share/misc/magic", magic, sizeof(magic) - 1, typeAndClass, sizeof(typeAndClass) - 1));
    REQUIRE_RC(KFileFormatRelease(pft));
}

#endif

TEST_CASE(Tar_Parse)
{
    KDirectory *dir;
    REQUIRE_RC(KDirectoryNativeDir(&dir));    
    
    const KDirectory *tarDir;
    REQUIRE_RC(KDirectoryOpenTarArchiveRead(dir, &tarDir, false, "test.tar"));
    
    struct KNamelist *list;
    REQUIRE_RC(KDirectoryList(tarDir, &list, NULL, NULL, NULL));
    
    uint32_t count;
    REQUIRE_RC(KNamelistCount(list, &count));
    REQUIRE_EQ(count, (uint32_t)2);
    
    const char* name;
    REQUIRE_RC(KNamelistGet(list, 0, &name));
    REQUIRE_EQ(string(name), string("Makefile"));
    REQUIRE_RC(KNamelistGet(list, 1, &name));
    REQUIRE_EQ(string(name), string("kfstest.cpp"));
    
    REQUIRE_RC(KNamelistRelease(list));
    REQUIRE_RC(KDirectoryRelease(tarDir));
    REQUIRE_RC(KDirectoryRelease(dir));
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

const char UsageDefaultName[] = "test-kfs";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KfsTestSuite(argc, argv);
    return rc;
}

}
