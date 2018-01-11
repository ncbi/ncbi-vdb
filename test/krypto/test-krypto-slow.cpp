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
 * Unit tests for KReencTestSuite
 */

#include <ktst/unit_test.hpp>

#include <krypto/key.h>
#include <krypto/encfile.h>
#include <krypto/encfile-priv.h>
#include <krypto/reencfile.h>
#include <kfs/impl.h>
#include <klib/rc.h>
#include <kapp/args.h>
#include <kfg/config.h>

#include "test-cmn.hpp"

#include <string.h>
#include <stdio.h>

TEST_SUITE(KKryptoSlowTestSuite);

// check for file sizes which can only fit into 64-bit
TEST_CASE(KReencryptBigSparseFile)
{
    rc_t rc;
    bool space_exhausted = false;
    
    const char pw2 [] = "second pw";
    KKey key_reenc;
    REQUIRE_RC (KKeyInitUpdate (&key_reenc, kkeyAES128, pw2, strlen (pw2)));
    
    const char file_path [] = TMP_FOLDER "/big_file";

    const char file_path_reenc [] = TMP_FOLDER "/big_file_reenc";

    KFile * pt_file, *reenc_file, * reenc_pt_file;
    
    uint64_t file_size = 5LL * 1024 * 1024 * 1024;
    uint64_t pt_size, reenc_size;
    
    struct KDirectory * current_dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &current_dir ) );
    
    // just in case if it still there
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    
    // create file
    REQUIRE_RC ( TCreatePtFile( current_dir, file_path, TFileOpenMode_Write, &pt_file ) );
    REQUIRE_RC ( KFileSetSize( pt_file, file_size ) );
    REQUIRE_RC ( KFileRelease ( pt_file ) );
    
    REQUIRE_RC ( TOpenPtFile( current_dir, file_path, TFileOpenMode_Read, &pt_file ) );
    REQUIRE_RC ( KEncryptFileMakeRead( (const KFile **)&reenc_file, (const KFile *)pt_file, &key_reenc ) );
    
    REQUIRE_RC ( KFileSize ( pt_file, &pt_size ) );
    REQUIRE_RC ( KFileSize ( reenc_file, &reenc_size ) );
    
    REQUIRE ( reenc_size == TEncSizeFromPtSize(pt_size) );
    
    REQUIRE_RC ( TCreatePtFile( current_dir, file_path_reenc, TFileOpenMode_Write, &reenc_pt_file ) );
    
    rc = TCopyFile( reenc_pt_file, reenc_file );
    if (rc != 0 && GetRCObject(rc) == static_cast <RCObject> ( rcStorage )
                && GetRCState(rc) == rcExhausted)
    {
        space_exhausted = true;
        printf("WARNING! Test failed due to absence of free FS space - SKIPPING\n");
    }
    else
    {
        REQUIRE_RC ( rc );
    }
    
    REQUIRE_RC ( KFileRelease ( reenc_pt_file ) );
    REQUIRE_RC ( KFileRelease ( reenc_file ) );
    REQUIRE_RC ( KFileRelease ( pt_file ) );
    
    if ( !space_exhausted )
    {
        // check file content
        REQUIRE_RC ( TOpenEncFile( current_dir, file_path_reenc, TFileOpenMode_Read, &key_reenc, &reenc_file ) );
        
        REQUIRE_RC ( TCheckFileContent( reenc_file, (const uint8_t *)"\0", 1 ) );
        
        REQUIRE_RC ( KFileRelease ( reenc_file ) );
    }

    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    REQUIRE_RC ( KDirectoryRelease ( current_dir ) );
}

TEST_CASE(KReencrypt4GbMarginsSparseFiles)
{
    rc_t rc;
    bool space_exhausted = false;
    
    const char pw2 [] = "second pw";
    KKey key_reenc;
    REQUIRE_RC (KKeyInitUpdate (&key_reenc, kkeyAES128, pw2, strlen (pw2)));
    
    const char file_path [] = TMP_FOLDER "/big_4gb_file";

    const char file_path_reenc [] = TMP_FOLDER "/big_4gb_file_reenc";

    KFile * pt_file, *reenc_file, * reenc_pt_file;
    
    uint64_t file_size = 4LL * 1024 * 1024 * 1024;
    uint64_t pt_size, reenc_size;
    
    struct KDirectory * current_dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &current_dir ) );
    
    int8_t size_variants[] = { -1, 1 };
    for (size_t i = 0; i < sizeof size_variants / sizeof size_variants[0]; ++i )
    {
        // just in case if it still there
        KDirectoryRemove ( current_dir, true, TMP_FOLDER );
        
        // create file
        REQUIRE_RC ( TCreatePtFile( current_dir, file_path, TFileOpenMode_Write, &pt_file ) );
        REQUIRE_RC ( KFileSetSize( pt_file, file_size + size_variants[i] ) );
        REQUIRE_RC ( KFileRelease ( pt_file ) );
        
        REQUIRE_RC ( TOpenPtFile( current_dir, file_path, TFileOpenMode_Read, &pt_file ) );
        REQUIRE_RC ( KEncryptFileMakeRead( (const KFile **)&reenc_file, (const KFile *)pt_file, &key_reenc ) );
        
        REQUIRE_RC ( KFileSize ( pt_file, &pt_size ) );
        REQUIRE_RC ( KFileSize ( reenc_file, &reenc_size ) );
        
        REQUIRE ( file_size + size_variants[i] == pt_size );
        REQUIRE ( reenc_size == TEncSizeFromPtSize(pt_size) );
        
        REQUIRE_RC ( TCreatePtFile( current_dir, file_path_reenc, TFileOpenMode_Write, &reenc_pt_file ) );
        rc = TCopyFile( reenc_pt_file, reenc_file );
        if (rc != 0 && GetRCObject(rc) == static_cast <RCObject> ( rcStorage )
	            && GetRCState(rc) == rcExhausted)
        {
            space_exhausted = true;
            printf("WARNING! Test failed due to absence of free FS space - SKIPPING\n");
        }
        else
        {
            REQUIRE_RC ( rc );
        }
        
        REQUIRE_RC ( KFileRelease ( reenc_pt_file ) );
        REQUIRE_RC ( KFileRelease ( reenc_file ) );
        REQUIRE_RC ( KFileRelease ( pt_file ) );
        
        if ( !space_exhausted )
        {
            // check file content
            REQUIRE_RC ( TOpenEncFile( current_dir, file_path_reenc, TFileOpenMode_Read, &key_reenc, &reenc_file ) );
            
            REQUIRE_RC ( TCheckFileContent( reenc_file, (const uint8_t *)"\0", 1 ) );
            
            REQUIRE_RC ( KFileRelease ( reenc_file ) );
        }
        
        KDirectoryRemove ( current_dir, true, TMP_FOLDER );
        if ( space_exhausted )
        {
            break;
        }
    }
    REQUIRE_RC ( KDirectoryRelease ( current_dir ) );
}

TEST_CASE(KEncDecBigFile)
{
    if ( getenv ( "NOT_VERY_SLOW" ) ) {
        TEST_MESSAGE ( "SKIPPING VERY SLOW " << GetName () );
        return;
    }

    rc_t rc;
    bool space_exhausted = false;
    
    const char pw [] = "first pw";
    KKey key;
    REQUIRE_RC (KKeyInitUpdate (&key, kkeyAES128, pw, strlen (pw)));
    
    const char file_path [] = TMP_FOLDER "/enc_big_file";

    KFile * enc_file, * pt_file;
    
    uint64_t file_size = 5LL * 1024 * 1024 * 1024;
    
    struct KDirectory * current_dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &current_dir ) );
    
    // just in case if it still there
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    
    // create file
    REQUIRE_RC ( TCreateEncFile( current_dir, file_path, TFileOpenMode_ReadWrite, &key, &enc_file ) );
    
    // write file
    rc = TFillFile( enc_file, (const uint8_t *)"\1\5", 2, file_size );
    if (rc != 0 && GetRCObject(rc) == static_cast <RCObject> ( rcStorage )
                && GetRCState(rc) == rcExhausted)
    {
        space_exhausted = true;
        printf("WARNING! Test failed due to absence of free FS space - SKIPPING\n");
    }
    else
    {
        REQUIRE_RC ( rc );
    }
    
    uint64_t size_data_actual;
    if ( !space_exhausted )
    {
        REQUIRE_RC ( KFileSize ( enc_file, &size_data_actual ) );
        
        // check content size
        REQUIRE ( file_size == size_data_actual );
    }
    rc = KFileRelease ( enc_file );
    if (!space_exhausted )
    {
        // we write file footer when closing file, which may fail too
        if (rc != 0 && GetRCObject(rc) == static_cast <RCObject> ( rcStorage )
	            && GetRCState(rc) == rcExhausted)
        {
            space_exhausted = true;
            printf("WARNING! Test failed due to absence of free FS space - SKIPPING\n");
        }
        else
        {
            REQUIRE_RC ( rc );
        }
    }
    
    if ( !space_exhausted )
    {
        // check plaintext file size and checksums
        REQUIRE_RC ( TOpenPtFile ( current_dir, file_path, TFileOpenMode_Read, &pt_file ) );
        
        uint64_t size_pt;
        REQUIRE_RC ( KFileSize ( pt_file, &size_pt ) );
        REQUIRE ( size_pt == TEncSizeFromPtSize(size_data_actual) );
        
        REQUIRE_RC ( KEncFileValidate( pt_file ) );
        
        REQUIRE_RC ( KFileRelease ( pt_file ) );
        
        // check file content
        REQUIRE_RC ( TOpenEncFile( current_dir, file_path, TFileOpenMode_Read, &key, &enc_file ) );
        
        REQUIRE_RC ( TCheckFileContent( enc_file, (const uint8_t *)"\1\5", 2 ) );
        
        REQUIRE_RC ( KFileRelease ( enc_file ) );
    }

    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    REQUIRE_RC ( KDirectoryRelease ( current_dir ) );
}

TEST_CASE(KEncDec4GbMarginsFiles)
{
    if ( getenv ( "NOT_VERY_SLOW" ) ) { 
        TEST_MESSAGE ( "SKIPPING VERY SLOW " << GetName () );
        return;
    }

    rc_t rc;
    bool space_exhausted = false;
 
    const char pw [] = "first pw";
    KKey key;
    REQUIRE_RC (KKeyInitUpdate (&key, kkeyAES128, pw, strlen (pw)));
    
    const char file_path [] = TMP_FOLDER "/enc_4gb_file";

    KFile * enc_file, * pt_file;
    
    uint64_t file_size = 4LL * 1024 * 1024 * 1024;
    
    struct KDirectory * current_dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &current_dir ) );
    
    int8_t size_variants[] = { -1, 1 };
    for (size_t i = 0; i < sizeof size_variants / sizeof size_variants[0]; ++i )
    {
        // just in case if it still there
        KDirectoryRemove ( current_dir, true, TMP_FOLDER );
        
        // create file
        REQUIRE_RC ( TCreateEncFile( current_dir, file_path, TFileOpenMode_ReadWrite, &key, &enc_file ) );
        
        // write file
        rc = TFillFile( enc_file, (const uint8_t *)"\4\3", 2, file_size );
        if (rc != 0 && GetRCObject(rc) == static_cast <RCObject> ( rcStorage )
	            && GetRCState(rc) == rcExhausted)
        {
            space_exhausted = true;
            printf("WARNING! Test failed due to absence of free FS space - SKIPPING\n");
        }
        else
        {
            REQUIRE_RC ( rc );
        }
        
        uint64_t size_data_actual;
        if ( !space_exhausted )
        {
            REQUIRE_RC ( KFileSize ( enc_file, &size_data_actual ) );
            
            // check content size
            REQUIRE ( file_size == size_data_actual );
        }
        rc = KFileRelease ( enc_file );
        if ( !space_exhausted )
        {
            if (rc != 0 &&
	        GetRCObject(rc) == static_cast <RCObject> ( rcStorage ) &&
		GetRCState(rc) == rcExhausted)
            {
                // we write file footer when closing file, which may fail too
                space_exhausted = true;
                printf("WARNING! Test failed due to absence of free FS space - SKIPPING\n");
            }
            else
            {
                REQUIRE_RC ( rc );
            }
        }
        
        if ( !space_exhausted )
        {
            // check plaintext file size and checksums
            REQUIRE_RC ( TOpenPtFile ( current_dir, file_path, TFileOpenMode_Read, &pt_file ) );
            
            uint64_t size_pt;
            REQUIRE_RC ( KFileSize ( pt_file, &size_pt ) );
            REQUIRE ( size_pt == TEncSizeFromPtSize(size_data_actual) );
            
            REQUIRE_RC ( KEncFileValidate( pt_file ) );
            
            REQUIRE_RC ( KFileRelease ( pt_file ) );
            
            // check file content
            REQUIRE_RC ( TOpenEncFile( current_dir, file_path, TFileOpenMode_Read, &key, &enc_file ) );
            
            REQUIRE_RC ( TCheckFileContent( enc_file, (const uint8_t *)"\4\3", 2 ) );
            
            REQUIRE_RC ( KFileRelease ( enc_file ) );
        }
        
        KDirectoryRemove ( current_dir, true, TMP_FOLDER );
        if ( space_exhausted )
        {
            break;
        }
    }
    
    REQUIRE_RC ( KDirectoryRelease ( current_dir ) );
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
    
    const char UsageDefaultName[] = "test-krypto-slow";
    
    rc_t CC KMain ( int argc, char *argv [] )
    {
        KConfigDisableUserSettings();
        ncbi::NK::TestEnv::SetVerbosity(ncbi::NK::LogLevel::e_all);
        rc_t rc=KKryptoSlowTestSuite(argc, argv);
        return rc;
    }
    
}
