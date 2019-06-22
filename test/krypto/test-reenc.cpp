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

TEST_SUITE(KReencTestSuite);

TEST_CASE(KReEncryptEncFile)
{
    const char pw1 [] = "first pw";
    const char pw2 [] = "second pw";
    KKey key_enc, key_reenc;
    REQUIRE_RC (KKeyInitUpdate (&key_enc, kkeyAES128, pw1, strlen (pw1)));
    REQUIRE_RC (KKeyInitUpdate (&key_reenc, kkeyAES256, pw2, strlen (pw2)));
    
    const char enc_file_path_fmt [] = TMP_FOLDER "/file_enc%llu";

    const char reenc_file_path_fmt [] = TMP_FOLDER "/file_reenc%llu";

    KFile * enc_file, * reenc_file, * reenc_pt_file, * enc_pt_file;
    
    uint64_t enc_pt_size, reenc_size;
    
    struct KDirectory * current_dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &current_dir ) );
    
    // just in case if it still there
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    
    uint64_t file_sizes_n_32k[] = { 0, 1, 2, 10, 46, 51 };
    int8_t file_size_variants[] = { -2, -1, 0, 1, 2 };
    
    const uint8_t* file_fillers[] = { (const uint8_t *)"\3\5\1\7" };
    size_t file_fillers_sizes[] = { 4 };
    
    assert( sizeof file_fillers / sizeof file_fillers[0] == sizeof file_fillers_sizes / sizeof file_fillers_sizes[0] );
    
    for (size_t filler_index = 0; filler_index < sizeof file_fillers / sizeof file_fillers[0]; ++filler_index )
    {
        printf("filler pattern: ");
        for (size_t i = 0; i < file_fillers_sizes[filler_index]; ++i)
        {
            printf("0x%X ", file_fillers[filler_index][i]);
        }
        printf("\n");
        for (size_t i = 0; i < sizeof file_sizes_n_32k / sizeof file_sizes_n_32k[0]; ++i)
        {
            for (size_t j = 0; j < sizeof file_size_variants / sizeof file_size_variants[0]; ++j)
            {
                if (file_sizes_n_32k[i] == 0 && file_size_variants[j] <= 0)
                {
                    continue;
                }
                
                uint64_t file_size = file_sizes_n_32k[i] * BLOCK_32K_SIZE + file_size_variants[j];
                
                char file_path[1024];
                char file_path_reenc[1024];
                
                sprintf(file_path, enc_file_path_fmt, ( long long unsigned int ) file_size);
                sprintf(file_path_reenc, reenc_file_path_fmt, ( long long unsigned int ) file_size);
                
                printf("reencrypting encrypted file %s, size: %llu, i: %zu, j: %zu\n", file_path, ( long long unsigned int ) file_size, i, j);
    
                // create file
                REQUIRE_RC ( TCreateEncFile( current_dir, file_path, TFileOpenMode_Write, &key_enc, &enc_file ) );
                REQUIRE_RC ( TFillFile( enc_file, file_fillers[filler_index], file_fillers_sizes[filler_index], file_size ) );
                REQUIRE_RC ( KFileRelease ( enc_file ) );
                
                REQUIRE_RC ( TOpenPtFile( current_dir, file_path, TFileOpenMode_Read, &enc_pt_file ) );
                REQUIRE_RC ( KReencFileMakeRead( (const KFile **)&reenc_file, (const KFile *)enc_pt_file, &key_enc, &key_reenc ) );
                
                REQUIRE_RC ( KFileSize ( enc_pt_file, &enc_pt_size ) );
                REQUIRE_RC ( KFileSize ( reenc_file, &reenc_size ) );
                
                REQUIRE ( reenc_size == enc_pt_size ) ;
                
                REQUIRE_RC ( TCreatePtFile( current_dir, file_path_reenc, TFileOpenMode_Write, &reenc_pt_file ) );
                REQUIRE_RC ( TCopyFile( reenc_pt_file, reenc_file ) );
                
                REQUIRE_RC ( KFileRelease ( reenc_pt_file ) );
                REQUIRE_RC ( KFileRelease ( reenc_file ) );
                REQUIRE_RC ( KFileRelease ( enc_pt_file ) );
                
                REQUIRE_RC ( TOpenPtFile( current_dir, file_path_reenc, TFileOpenMode_Read, &reenc_pt_file ) );
                REQUIRE_RC ( KEncFileValidate( reenc_pt_file ) );
                REQUIRE_RC ( KFileRelease ( reenc_pt_file ) );
                
                // check file content
                REQUIRE_RC ( TOpenEncFile( current_dir, file_path_reenc, TFileOpenMode_Read, &key_reenc, &reenc_file ) );
                
                REQUIRE_RC ( TCheckFileContent( reenc_file, file_fillers[filler_index], file_fillers_sizes[filler_index] ) );
                
                REQUIRE_RC ( KFileRelease ( reenc_file ) );
                
            }
        }
    }
    
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );

    REQUIRE_RC ( KDirectoryRelease ( current_dir ) );
}

TEST_CASE(KReEncryptPtFile)
{
    const char pw2 [] = "second pw";
    KKey key_reenc;
    REQUIRE_RC (KKeyInitUpdate (&key_reenc, kkeyAES256, pw2, strlen (pw2)));
    
    const char file_path_fmt [] = TMP_FOLDER "/file%llu";

    const char reenc_file_path_fmt [] = TMP_FOLDER "/file_reenc%llu";

    KFile * pt_file, *reenc_file, * reenc_pt_file;
    
    uint64_t pt_size, reenc_size;
    
    struct KDirectory * current_dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &current_dir ) );
    
    // just in case if it still there
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    
    uint64_t file_sizes_n_32k[] = { 0, 1, 2, 10, 46, 51 };
    int8_t file_size_variants[] = { -2, -1, 0, 1, 2 };
    
    const uint8_t* file_fillers[] = { (const uint8_t *)"\1\5\3\7" };
    size_t file_fillers_sizes[] = { 4 };
    
    assert( sizeof file_fillers / sizeof file_fillers[0] == sizeof file_fillers_sizes / sizeof file_fillers_sizes[0] );
    
    for (size_t filler_index = 0; filler_index < sizeof file_fillers / sizeof file_fillers[0]; ++filler_index )
    {
        printf("filler pattern: ");
        for (size_t i = 0; i < file_fillers_sizes[filler_index]; ++i)
        {
            printf("0x%X ", file_fillers[filler_index][i]);
        }
        printf("\n");
        for (size_t i = 0; i < sizeof file_sizes_n_32k / sizeof file_sizes_n_32k[0]; ++i)
        {
            for (size_t j = 0; j < sizeof file_size_variants / sizeof file_size_variants[0]; ++j)
            {
                if (file_sizes_n_32k[i] == 0 && file_size_variants[j] <= 0)
                {
                    continue;
                }
                
                uint64_t file_size = file_sizes_n_32k[i] * BLOCK_32K_SIZE + file_size_variants[j];
                
                char file_path[1024];
                char file_path_reenc[1024];
                
                sprintf(file_path, file_path_fmt, ( long long unsigned int ) file_size);
                sprintf(file_path_reenc, reenc_file_path_fmt, ( long long unsigned int ) file_size);
                
                printf("reencrypting NOT encrypted file %s, size: %llu, i: %zu, j: %zu\n", file_path, ( long long unsigned int ) file_size, i, j);
                
                // create file
                REQUIRE_RC ( TCreatePtFile( current_dir, file_path, TFileOpenMode_Write, &pt_file ) );
                REQUIRE_RC ( TFillFile( pt_file, file_fillers[filler_index], file_fillers_sizes[filler_index], file_size ) );
                REQUIRE_RC ( KFileRelease ( pt_file ) );
                
                REQUIRE_RC ( TOpenPtFile( current_dir, file_path, TFileOpenMode_Read, &pt_file ) );
                REQUIRE_RC ( KEncryptFileMakeRead( (const KFile **)&reenc_file, (const KFile *)pt_file, &key_reenc ) );
                
                REQUIRE_RC ( KFileSize ( pt_file, &pt_size ) );
                REQUIRE_RC ( KFileSize ( reenc_file, &reenc_size ) );
                
                REQUIRE ( file_size == pt_size );
                REQUIRE ( reenc_size == TEncSizeFromPtSize(pt_size) );
                
                REQUIRE_RC ( TCreatePtFile( current_dir, file_path_reenc, TFileOpenMode_Write, &reenc_pt_file ) );
                REQUIRE_RC ( TCopyFile( reenc_pt_file, reenc_file ) );
                
                REQUIRE_RC ( KFileRelease ( reenc_pt_file ) );
                REQUIRE_RC ( KFileRelease ( reenc_file ) );
                REQUIRE_RC ( KFileRelease ( pt_file ) );
                
                // check file content
                REQUIRE_RC ( TOpenEncFile( current_dir, file_path_reenc, TFileOpenMode_Read, &key_reenc, &reenc_file ) );
                
                REQUIRE_RC ( TCheckFileContent( reenc_file, file_fillers[filler_index], file_fillers_sizes[filler_index] ) );
                
                REQUIRE_RC ( KFileRelease ( reenc_file ) );
                
            }
        }
    }
    
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );

    REQUIRE_RC ( KDirectoryRelease ( current_dir ) );
}

TEST_CASE(KReencryptZeroContentSizeEncFile)
{
    const char pw1 [] = "first pw";
    const char pw2 [] = "second pw";
    KKey key_enc, key_reenc;
    REQUIRE_RC (KKeyInitUpdate (&key_enc, kkeyAES128, pw1, strlen (pw1)));
    REQUIRE_RC (KKeyInitUpdate (&key_reenc, kkeyAES256, pw2, strlen (pw2)));
    
    const char enc_file_path [] = TMP_FOLDER "/zero_content_file_to_reenc";

    const char reenc_file_path [] = TMP_FOLDER "/reenc_zero_content_file";

    KFile * enc_file, * enc_pt_file, * reenc_file, * reenc_pt_file;
    
    uint64_t enc_pt_size, reenc_size;
    
    struct KDirectory * current_dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &current_dir ) );
    
    // just in case if it still there
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    
    // create file
    REQUIRE_RC ( TCreateEncFile( current_dir, enc_file_path, TFileOpenMode_Write, &key_enc, &enc_file ) );
    REQUIRE_RC ( KFileRelease ( enc_file ) );
    
    REQUIRE_RC ( TOpenPtFile( current_dir, enc_file_path, TFileOpenMode_Read, &enc_pt_file ) );
    REQUIRE_RC ( KReencFileMakeRead( (const KFile **)&reenc_file, (const KFile *)enc_pt_file, &key_enc, &key_reenc ) );
    
    REQUIRE_RC ( KFileSize ( enc_pt_file, &enc_pt_size ) );
    REQUIRE_RC ( KFileSize ( reenc_file, &reenc_size ) );
    
    REQUIRE ( reenc_size == enc_pt_size ) ;
    
    REQUIRE_RC ( TCreatePtFile( current_dir, reenc_file_path, TFileOpenMode_Write, &reenc_pt_file ) );
    REQUIRE_RC ( TCopyFile( reenc_pt_file, reenc_file ) );
    
    REQUIRE_RC ( KFileRelease ( reenc_pt_file ) );
    REQUIRE_RC ( KFileRelease ( reenc_file ) );
    REQUIRE_RC ( KFileRelease ( enc_pt_file ) );
    
    REQUIRE_RC ( TOpenPtFile( current_dir, reenc_file_path, TFileOpenMode_Read, &reenc_pt_file ) );
    REQUIRE_RC ( KEncFileValidate( reenc_pt_file ) );
    REQUIRE_RC ( KFileRelease ( reenc_pt_file ) );
    
    // check file content
    REQUIRE_RC ( TOpenEncFile( current_dir, reenc_file_path, TFileOpenMode_Read, &key_reenc, &reenc_file ) );
    REQUIRE_RC ( KFileSize ( reenc_file, &reenc_size ) );
    REQUIRE ( reenc_size == 0 );
    REQUIRE_RC ( KFileRelease ( reenc_file ) );
    
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    
    REQUIRE_RC ( KDirectoryRelease ( current_dir ) );
}

TEST_CASE(KReencryptZeroContentSizePtFile)
{
    const char pw2 [] = "second pw";
    KKey key_reenc;
    REQUIRE_RC (KKeyInitUpdate (&key_reenc, kkeyAES256, pw2, strlen (pw2)));
    
    const char pt_file_path [] = TMP_FOLDER "/zero_content_file_to_reenc_pt";

    const char reenc_file_path [] = TMP_FOLDER "/reenc_zero_content_file_pt";

    KFile * pt_file, * reenc_file, * reenc_pt_file;
    
    uint64_t pt_size, reenc_size;
    
    struct KDirectory * current_dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &current_dir ) );
    
    // just in case if it still there
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    
    // create file
    REQUIRE_RC ( TCreatePtFile( current_dir, pt_file_path, TFileOpenMode_Write, &pt_file ) );
    REQUIRE_RC ( KFileRelease ( pt_file ) );
    
    REQUIRE_RC ( TOpenPtFile( current_dir, pt_file_path, TFileOpenMode_Read, &pt_file ) );
    REQUIRE_RC ( KEncryptFileMakeRead( (const KFile **)&reenc_file, (const KFile *)pt_file, &key_reenc ) );
    
    REQUIRE_RC ( KFileSize ( pt_file, &pt_size ) );
    REQUIRE_RC ( KFileSize ( reenc_file, &reenc_size ) );
    
    REQUIRE ( reenc_size == pt_size + sizeof(KEncFileHeader) + sizeof(KEncFileFooter) ) ;
    
    REQUIRE_RC ( TCreatePtFile( current_dir, reenc_file_path, TFileOpenMode_Write, &reenc_pt_file ) );
    REQUIRE_RC ( TCopyFile( reenc_pt_file, reenc_file ) );
    
    REQUIRE_RC ( KFileRelease ( reenc_pt_file ) );
    REQUIRE_RC ( KFileRelease ( reenc_file ) );
    REQUIRE_RC ( KFileRelease ( pt_file ) );
    
    REQUIRE_RC ( TOpenPtFile( current_dir, reenc_file_path, TFileOpenMode_Read, &reenc_pt_file ) );
    REQUIRE_RC ( KEncFileValidate( reenc_pt_file ) );
    REQUIRE_RC ( KFileRelease ( reenc_pt_file ) );
    
    // check file content
    REQUIRE_RC ( TOpenEncFile( current_dir, reenc_file_path, TFileOpenMode_Read, &key_reenc, &reenc_file ) );
    REQUIRE_RC ( KFileSize ( reenc_file, &reenc_size ) );
    REQUIRE ( reenc_size == 0 );
    REQUIRE_RC ( KFileRelease ( reenc_file ) );
    
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    
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
    
    const char UsageDefaultName[] = "test-reenc";
    
    rc_t CC KMain ( int argc, char *argv [] )
    {
        KConfigDisableUserSettings();
        rc_t rc=KReencTestSuite(argc, argv);
        return rc;
    }
    
}
