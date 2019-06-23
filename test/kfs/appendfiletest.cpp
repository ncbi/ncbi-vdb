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
* Unit tests for KAppendFile
*/
#include <cstring>
#include <ktst/unit_test.hpp>

#include <kfs/impl.h>
#include <kfs/appendfile.h>

#include <klib/out.h>
#include <kapp/args.h>
#include <kfg/config.h>

using namespace std;

TEST_SUITE(AppendFileTestSuite);

static const char * StringOne = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char * FileName = "JoJoBa.Ba";

static
rc_t
checkWriteFile ( KFile * File, size_t Pos, size_t Size )
{
    rc_t RCt;
    size_t NumWrit;
    size_t StrLen;
    size_t Offset;
    size_t ToWr;

    RCt = 0;
    NumWrit = 0;
    StrLen = strlen ( StringOne );
    Offset = 0;
    ToWr = 0;

    printf ( "[WRI] [%lu]@[%lu]\n", Size, Pos );

    while ( Offset < Size ) {
        ToWr = Size - Offset;
        if ( StrLen < ToWr ) {
            ToWr = StrLen;
        }

        RCt =  KFileWrite (
                            File, 
                            Offset + Pos,
                            StringOne,
                            ToWr,
                            & NumWrit
                            );
        if ( RCt != 0 ) {
            break;
        }

        Offset += ToWr;
    }

    return RCt;
}   /* checkWriteFile () */

static
rc_t
checkReadFile ( KFile * File, size_t Pos, size_t Size )
{
    rc_t RCt;
    size_t NumRead;
    size_t StrLen;
    size_t Offset;
    char * Buf;

    RCt = 0;
    NumRead = 0;
    StrLen = strlen ( StringOne );
    Offset = 0;
    Buf = NULL;

    printf ( "[REA] [%lu]@[%lu]\n", Size, Pos );

    Buf = (char *) calloc ( Size, sizeof ( char ) );
    if ( Buf == NULL ) {
        return 22;
    }

    RCt =  KFileReadAll (
                        File, 
                        Pos,
                        Buf,
                        Size,
                        & NumRead
                        );
    if ( RCt == 0 ) {
            /* Checking read result */
        for ( Offset = 0; Offset < Size; Offset ++ ) {
            if ( Buf [ Offset ] != StringOne [ ( Pos + Offset ) % StrLen ] ) {
                RCt = 33;
                break;
            }
        }
    }

    free ( Buf );

    return RCt;
}   /* checkReadFile () */

static
rc_t
checkMakeFile ( const char * Name, size_t Size )
{
    rc_t RCt;
    struct KDirectory * Dir;
    struct KFile * File;

    RCt = 0;
    Dir = NULL;
    File = NULL;

    printf ( "[CRE] [%s] SZ[%lu]\n", FileName, Size );

    RCt = KDirectoryNativeDir ( & Dir );
    if ( RCt == 0 ) {
        RCt = KDirectoryCreateFile ( 
                                    Dir,
                                    & File,
                                    true,
                                    0664,
                                    kcmInit,
                                    "%s",
                                    Name
                                    );
        if ( RCt == 0 ) {
            if ( Size != 0 ) {
                checkWriteFile ( File, 0, Size );
            }

            KFileRelease ( File );
        }

        KDirectoryRelease ( Dir );
    }
    return RCt;
}   /* checkMakeFile () */

static
rc_t
checkOpenFile ( KFile ** File, const char * Name, bool Write )
{
    rc_t RCt;
    struct KDirectory * Dir;

    RCt = 0;
    Dir = NULL;

    printf ( "[OPE] [%s] [%s]\n", FileName, ( Write ? "WR" : "RD" ) );

    RCt = KDirectoryNativeDir ( & Dir );
    if ( RCt == 0 ) {
        RCt = Write
                    ? KDirectoryOpenFileWrite ( Dir, File, "%s", Name )
                    : KDirectoryOpenFileRead ( Dir, ( const KFile ** ) File, "%s", Name )
                    ;
        KDirectoryRelease ( Dir );
    }

    return RCt;
}   /* checkOpenFile () */

static
bool
checkFileSize ( const char * FileName, uint64_t Size )
{
    rc_t RCt;
    KDirectory * Dir;
    uint64_t FileSize;

    RCt = 0;
    Dir = NULL;
    FileSize = 0;

    printf ( "[SIZ] [%s] [%llu]\n", FileName, ( long long unsigned int ) Size );

    RCt = KDirectoryNativeDir ( & Dir );
    if ( RCt == 0 ) {
        RCt = KDirectoryFileSize ( Dir, & FileSize, "%s", FileName );

        KDirectoryRelease ( Dir );
    }

    return RCt == 0 ? ( FileSize == Size ) : false;
}   /* checkFileSize () */

static
bool
checkFileDelete ( const char * FileName )
{
    rc_t RCt;
    KDirectory * Dir;

    RCt = 0;
    Dir = NULL;

    printf ( "[REM] [%s]\n", FileName );

    RCt = KDirectoryNativeDir ( & Dir );
    if ( RCt == 0 ) {
        RCt = KDirectoryRemove ( Dir, "%s", FileName );

        KDirectoryRelease ( Dir );
    }

    return RCt;
}   /* checkFileDelete () */

static
rc_t
checkFileSetSize ( KFile * File, size_t Size )
{
    printf ( "[SET] [%s] SZ[%lu]\n", FileName, Size );

    return KFileSetSize ( File, Size );
}   /* checkFileSetSize () */


TEST_CASE(KAppendFile_read_write)
{
    KFile * File;
    KFile * AFile;
    uint64_t FileSize;
    uint64_t NewSize;

    File = NULL;
    AFile = NULL;
    FileSize = 0;
    NewSize = 0;

    printf ( "\n[[[[TST:1] [Simple read / write]\n" );
    /*  Simple read/write file
     */
    FileSize = 64;
    REQUIRE_RC(checkMakeFile(FileName, FileSize));

    REQUIRE(checkFileSize(FileName, FileSize));

    REQUIRE_RC(checkOpenFile ( & File, FileName, true ));

    REQUIRE_RC(KFileMakeAppend(&AFile, File));
    KFileRelease ( File );

    REQUIRE_RC(checkWriteFile(AFile, 0, FileSize));
    REQUIRE_RC(KFileSize(AFile, &NewSize));
    REQUIRE(FileSize == NewSize);
    REQUIRE_RC(checkReadFile(AFile, 2, 22));


    KFileRelease ( AFile );
    REQUIRE(checkFileSize(FileName, FileSize * 2));

    REQUIRE_RC(checkFileDelete(FileName));
}

TEST_CASE(KAppendFile_read_write_zerosize)
{
    KFile * File;
    KFile * AFile;
    uint64_t FileSize;
    uint64_t NewSize;

    File = NULL;
    AFile = NULL;
    FileSize = 0;
    NewSize = 0;

    printf ( "\n[[[[TST:2] [Simple read / write on empty file]\n" );
    /*  Simple read/write file
     */
    FileSize = 64;
    REQUIRE_RC(checkMakeFile(FileName, 0));

    REQUIRE(checkFileSize(FileName, 0));

    REQUIRE_RC(checkOpenFile ( & File, FileName, true ));

    REQUIRE_RC(KFileMakeAppend(&AFile, File));
    KFileRelease ( File );

    REQUIRE_RC(checkWriteFile(AFile, 0, FileSize));
    REQUIRE_RC(KFileSize(AFile, &NewSize));
    REQUIRE(FileSize == NewSize);
    REQUIRE_RC(checkReadFile(AFile, 2, 22));


    KFileRelease ( AFile );
    REQUIRE(checkFileSize(FileName, FileSize));

    REQUIRE_RC(checkFileDelete(FileName));
}

TEST_CASE(KAppendFile_set_size)
{
    KFile * File;
    KFile * AFile;
    uint64_t FileSize;
    uint64_t NewSize;

    File = NULL;
    AFile = NULL;
    FileSize = 0;
    NewSize = 0;

    printf ( "\n[[[[TST:3] [Setting file size]\n" );

    FileSize = 64;
    REQUIRE_RC(checkMakeFile(FileName, FileSize));

    REQUIRE(checkFileSize(FileName, FileSize));

    REQUIRE_RC(checkOpenFile ( & File, FileName, true ));

    REQUIRE_RC(KFileMakeAppend(&AFile, File));
    KFileRelease ( File );

    REQUIRE_RC(checkWriteFile(AFile, 0, FileSize));

    REQUIRE_RC(KFileSize(AFile, &NewSize));
    REQUIRE(FileSize == NewSize);

    REQUIRE_RC(checkFileSetSize(AFile, FileSize / 2));
    REQUIRE_RC(KFileSize(AFile, &NewSize));
    REQUIRE(FileSize / 2 == NewSize);

    KFileRelease ( AFile );
    REQUIRE(checkFileSize(FileName, FileSize + (FileSize / 2)));

    REQUIRE_RC(checkFileDelete(FileName));
}

//////////////////////////////////////////// Main

extern "C"
{

ver_t CC KAppVersion ( void )
{
    return 0x1000000;
}

rc_t CC UsageSummary (const char * prog_name)
{
    return 0;
}

rc_t CC Usage ( const Args * args)
{
    return 0;
}

const char UsageDefaultName[] = "test-appendfile";

rc_t CC KMain ( int argc, char *argv [] )
{
    rc_t rc=AppendFileTestSuite(argc, argv);
    return rc;
}

}
