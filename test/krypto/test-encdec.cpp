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
 * Unit tests for KEncDecTestSuite
 */

#include <ktst/unit_test.hpp>
#include <krypto/key.h>
#include <krypto/encfile.h>
#include <krypto/encfile-priv.h>
#include <krypto/reencfile.h>
#include <kfs/impl.h>
#include <klib/rc.h>
#include <klib/log.h>
#include <kapp/args.h>
#include <kfg/config.h>

#include "test-cmn.hpp"

#include <string.h>
#include <stdio.h>

TEST_SUITE(KEncDecTestSuite);

TEST_CASE(KEncryptDecrypt)
{
    const char pw [] = "first pw";
    KKey key;
    REQUIRE_RC (KKeyInitUpdate (&key, kkeyAES128, pw, strlen (pw)));
    
    const char enc_file_path_fmt [] = TMP_FOLDER "/enc_file%llu";

    KFile * enc_file, * pt_file;
    
    struct KDirectory * current_dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &current_dir ) );
    
    // just in case if it still there
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    
    uint64_t file_sizes_n_32k[] = { 0, 1, 2, 10, 46, 51 };
    int8_t file_size_variants[] = { -2, -1, 0, 1, 2 };
    
    const uint8_t* file_fillers[] = { (const uint8_t *)"\0", (const uint8_t *)"\1\2\3\0" };
    size_t file_fillers_sizes[] = { 1, 4 };
    
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
                sprintf(file_path, enc_file_path_fmt, ( long long unsigned int ) file_size);
                
                printf("encrypting/decrypting file %s, size: %llu, i: %zu, j: %zu\n", file_path, ( long long unsigned int ) file_size, i, j);
                
                // create file
                REQUIRE_RC ( TCreateEncFile( current_dir, file_path, TFileOpenMode_ReadWrite, &key, &enc_file ) );
                
                // write file
                REQUIRE_RC ( TFillFile( enc_file, file_fillers[filler_index], file_fillers_sizes[filler_index], file_size ) );
                
                uint64_t size_data_actual;
                REQUIRE_RC ( KFileSize ( enc_file, &size_data_actual ) );
                
                // check content size
                REQUIRE ( file_size == size_data_actual );
              
                REQUIRE_RC ( KFileRelease ( enc_file ) );
                
                // check raw file size and checksums
                REQUIRE_RC ( TOpenPtFile ( current_dir, file_path, TFileOpenMode_Read, &pt_file ) );
                
                uint64_t size_raw;
                REQUIRE_RC ( KFileSize ( pt_file, &size_raw ) );
                REQUIRE ( size_raw == TEncSizeFromPtSize(size_data_actual) );
                
                REQUIRE_RC ( KEncFileValidate( pt_file ) );
                
                REQUIRE_RC ( KFileRelease ( pt_file ) );
                
                // check file content
                REQUIRE_RC ( TOpenEncFile( current_dir, file_path, TFileOpenMode_Read, &key, &enc_file ) );
                
                REQUIRE_RC ( TCheckFileContent( enc_file, file_fillers[filler_index], file_fillers_sizes[filler_index] ) );
                
                REQUIRE_RC ( KFileRelease ( enc_file ) );
            }
        }
    }
    
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    REQUIRE_RC ( KDirectoryRelease ( current_dir ) );
}

TEST_CASE(KDecryptZeroRawSize)
{
    const char pw [] = "first pw";
    KKey key;
    REQUIRE_RC (KKeyInitUpdate (&key, kkeyAES128, pw, strlen (pw)));
    
    const char file_path [] = TMP_FOLDER "/zero_size_file_to_dec";

    KFile * enc_file, * pt_file;
    
    uint64_t file_size;
    
    struct KDirectory * current_dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &current_dir ) );
    
    // just in case if it still there
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    // create file
    REQUIRE_RC ( TCreatePtFile( current_dir, file_path, TFileOpenMode_ReadWrite, &pt_file ) );
    REQUIRE_RC ( KFileSize ( pt_file, &file_size ) );
    REQUIRE ( file_size == 0 );
    REQUIRE_RC ( KFileRelease ( pt_file ) );
    
    LOGMSG ( klogWarn, "Expect errors after this line:" );
    REQUIRE ( TOpenEncFile( current_dir, file_path, TFileOpenMode_Read, &key, &enc_file ) == RC( rcKrypto, rcFile, rcConstructing, rcSize, rcIncorrect ) );
    LOGMSG ( klogWarn, "No more errors are expected" );
    
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    REQUIRE_RC ( KDirectoryRelease ( current_dir ) );
}

TEST_CASE(KDecryptZeroContentSizeRW)
{
    const char pw [] = "first pw";
    KKey key;
    REQUIRE_RC (KKeyInitUpdate (&key, kkeyAES128, pw, strlen (pw)));
    
    const char file_path [] = TMP_FOLDER "/zero_content_rw_file_to_dec";

    KFile * enc_file, * pt_file;
    
    uint64_t file_size;
    
    struct KDirectory * current_dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &current_dir ) );
    
    // just in case if it still there
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    // create file
    REQUIRE_RC ( TCreateEncFile( current_dir, file_path, TFileOpenMode_ReadWrite, &key, &enc_file ) );
    REQUIRE_RC ( KFileRelease ( enc_file ) );
    
    // check raw size
    REQUIRE_RC ( TOpenPtFile( current_dir, file_path, TFileOpenMode_Read, &pt_file ) );
    REQUIRE_RC ( KFileSize ( pt_file, &file_size ) );
    REQUIRE ( file_size == sizeof(KEncFileHeader) + sizeof(KEncFileFooter) );
    REQUIRE_RC ( KEncFileValidate( pt_file ) );
    REQUIRE_RC ( KFileRelease ( pt_file ) );
    
    // check enc open
    REQUIRE_RC ( TOpenEncFile( current_dir, file_path, TFileOpenMode_Read, &key, &enc_file ) );
    REQUIRE_RC ( KFileRelease ( enc_file ) );
    
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    REQUIRE_RC ( KDirectoryRelease ( current_dir ) );
}

TEST_CASE(KDecryptZeroContentSizeWOnly)
{
    const char pw [] = "first pw";
    KKey key;
    REQUIRE_RC (KKeyInitUpdate (&key, kkeyAES128, pw, strlen (pw)));
    
    const char file_path [] = TMP_FOLDER "/zero_content_w_file_to_dec";

    KFile * enc_file, * pt_file;
    
    uint64_t file_size;
    
    struct KDirectory * current_dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &current_dir ) );
    
    // just in case if it still there
    KDirectoryRemove ( current_dir, true, "temp"
#if defined(__APPLE__)
        "mac");
#else
        "linux");
#endif
    
    // create file
    REQUIRE_RC ( TCreateEncFile( current_dir, file_path, TFileOpenMode_Write, &key, &enc_file ) );
    REQUIRE_RC ( KFileRelease ( enc_file ) );
    
    // check raw size
    REQUIRE_RC ( TOpenPtFile( current_dir, file_path, TFileOpenMode_Read, &pt_file ) );
    REQUIRE_RC ( KFileSize ( pt_file, &file_size ) );
    REQUIRE ( file_size == sizeof(KEncFileHeader) + sizeof(KEncFileFooter) );
    REQUIRE_RC ( KEncFileValidate( pt_file ) );
    REQUIRE_RC ( KFileRelease ( pt_file ) );
    
    // check enc open
    REQUIRE_RC ( TOpenEncFile( current_dir, file_path, TFileOpenMode_Read, &key, &enc_file ) );
    REQUIRE_RC ( KFileRelease ( enc_file ) );
    
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    REQUIRE_RC ( KDirectoryRelease ( current_dir ) );
}


TEST_CASE(KDectryptOnlyHeader)
{
    const char pw [] = "first pw";
    KKey key;
    REQUIRE_RC (KKeyInitUpdate (&key, kkeyAES128, pw, strlen (pw)));
    
    const char file_path [] = TMP_FOLDER "/file_only_header";

    KFile * enc_file, * pt_file;
    
    struct KDirectory * current_dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &current_dir ) );
    
    // just in case if it still there
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    // create file
    REQUIRE_RC ( TCreateEncFile( current_dir, file_path, TFileOpenMode_Write, &key, &enc_file ) );
    REQUIRE_RC ( TFillFile( enc_file, (const uint8_t *)"\0\1", 2, 500 ) );
    REQUIRE_RC ( KFileRelease ( enc_file ) );
    
    // truncate it to header size
    REQUIRE_RC ( TOpenPtFile ( current_dir, file_path, TFileOpenMode_ReadWrite, &pt_file ) );
    REQUIRE_RC ( KFileSetSize ( pt_file, sizeof(KEncFileHeader) ) );
    REQUIRE_RC ( KFileRelease ( pt_file ) );
    
    LOGMSG ( klogWarn, "Expect errors after this line:" );
    REQUIRE ( TOpenEncFile( current_dir, file_path, TFileOpenMode_Read, &key, &enc_file ) == RC( rcKrypto, rcFile, rcConstructing, rcSize, rcIncorrect ) );
    LOGMSG ( klogWarn, "No more errors are expected" );
    
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    REQUIRE_RC ( KDirectoryRelease ( current_dir ) );
}

TEST_CASE(KDectryptWithoutFooter)
{
    const char pw [] = "first pw";
    KKey key;
    REQUIRE_RC (KKeyInitUpdate (&key, kkeyAES128, pw, strlen (pw)));
    
    const char file_path [] = TMP_FOLDER "/file_no_footer";
    KFile * enc_file, * pt_file;
    
    uint64_t file_size;
    
    struct KDirectory * current_dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &current_dir ) );
    
    // just in case if it still there
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    // create file
    REQUIRE_RC ( TCreateEncFile( current_dir, file_path, TFileOpenMode_Write, &key, &enc_file ) );
    REQUIRE_RC ( TFillFile( enc_file, (const uint8_t *)"\0\1", 2, 500 ) );
    REQUIRE_RC ( KFileRelease ( enc_file ) );
    
    // truncate footer
    REQUIRE_RC ( TOpenPtFile ( current_dir, file_path, TFileOpenMode_ReadWrite, &pt_file ) );
    REQUIRE_RC ( KFileSize ( pt_file, &file_size ) );
    REQUIRE_RC ( KFileSetSize ( pt_file, file_size - sizeof(KEncFileFooter) ) );
    REQUIRE_RC ( KFileRelease ( pt_file ) );
    
    LOGMSG ( klogWarn, "Expect errors after this line:" );
    REQUIRE ( TOpenEncFile( current_dir, file_path, TFileOpenMode_Read, &key, &enc_file ) == RC( rcKrypto, rcFile, rcConstructing, rcSize, rcIncorrect ) );
    LOGMSG ( klogWarn, "No more errors are expected" );
    
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    REQUIRE_RC ( KDirectoryRelease ( current_dir ) );
}

TEST_CASE(KDectryptCorruptHeader)
{
    const char pw [] = "first pw";
    KKey key;
    REQUIRE_RC (KKeyInitUpdate (&key, kkeyAES128, pw, strlen (pw)));
    
    const char file_path [] = TMP_FOLDER "/file_corrupt_header";

    KFile * enc_file, * pt_file;
    
    const size_t buffer_size = sizeof(KEncFileHeader);
    size_t num_written;
    uint8_t buffer[buffer_size];
    
    struct KDirectory * current_dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &current_dir ) );
    
    // just in case if it still there
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    
    // create file
    REQUIRE_RC ( TCreateEncFile( current_dir, file_path, TFileOpenMode_Write, &key, &enc_file ) );
    REQUIRE_RC ( TFillFile( enc_file, (const uint8_t *)"\0\1", 2, 500 ) );
    REQUIRE_RC ( KFileRelease ( enc_file ) );
    
    // corrupt header
    REQUIRE_RC ( TOpenPtFile ( current_dir, file_path, TFileOpenMode_ReadWrite, &pt_file ) );
    REQUIRE_RC ( KFileReadAll ( pt_file, 0, buffer, buffer_size, &num_written ) );
    buffer[0] ^= 4;
    REQUIRE_RC ( KFileWriteAll ( pt_file, 0, buffer, buffer_size, &num_written ) );
    assert(buffer_size == num_written);
    REQUIRE_RC ( KFileRelease ( pt_file ) );
    
    LOGMSG ( klogWarn, "Expect errors after this line:" );
    REQUIRE ( TOpenEncFile( current_dir, file_path, TFileOpenMode_Read, &key, &enc_file ) == RC( rcFS, rcFile, rcConstructing, rcHeader, rcInvalid ) );
    LOGMSG ( klogWarn, "No more errors are expected" );
    
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    REQUIRE_RC ( KDirectoryRelease ( current_dir ) );
}

TEST_CASE(KDectryptCorruptFooterCrc)
{
    const char pw [] = "first pw";
    KKey key;
    REQUIRE_RC (KKeyInitUpdate (&key, kkeyAES128, pw, strlen (pw)));
    
    const char file_path [] = TMP_FOLDER "/file_corrupt_footer_crc";

    KFile * enc_file, * pt_file;

    uint64_t file_size;
    const size_t buffer_size = sizeof(KEncFileFooter);
    size_t num_written, num_read;
    uint8_t buffer[buffer_size];
    
    struct KDirectory * current_dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &current_dir ) );
    
    // just in case if it still there
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    
    // create file
    REQUIRE_RC ( TCreateEncFile( current_dir, file_path, TFileOpenMode_Write, &key, &enc_file ) );
    REQUIRE_RC ( TFillFile( enc_file, (const uint8_t *)"\0\1", 2, 500 ) );
    REQUIRE_RC ( KFileRelease ( enc_file ) );
    
    // corrupt footer
    REQUIRE_RC ( TOpenPtFile ( current_dir, file_path, TFileOpenMode_ReadWrite, &pt_file ) );
    REQUIRE_RC ( KFileSize ( pt_file, &file_size ) );
    REQUIRE_RC ( KFileReadAll ( pt_file, file_size - buffer_size, buffer, buffer_size, &num_read ) );
    assert(buffer_size == num_read);
    ((KEncFileFooter*)&buffer)->crc_checksum ^= 4;
    REQUIRE_RC ( KFileWriteAll ( pt_file, file_size - buffer_size, buffer, buffer_size, &num_written ) );
    assert(buffer_size == num_written);
    REQUIRE_RC ( KFileRelease ( pt_file ) );
    
    REQUIRE_RC ( TOpenPtFile( current_dir, file_path, TFileOpenMode_Read, &pt_file ) );
    LOGMSG ( klogWarn, "Expect errors after this line:" );
    REQUIRE ( KEncFileValidate( pt_file ) == RC(rcKrypto, rcFile, rcValidating, rcChecksum, rcCorrupt) );
    LOGMSG ( klogWarn, "No more errors are expected" );
    REQUIRE_RC ( KFileRelease ( pt_file ) );
    
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );

    REQUIRE_RC ( KDirectoryRelease ( current_dir ) );
}

TEST_CASE(KDectryptCorruptFooterBlockCount)
{
    const char pw [] = "first pw";
    KKey key;
    REQUIRE_RC (KKeyInitUpdate (&key, kkeyAES128, pw, strlen (pw)));
    
    const char file_path [] = TMP_FOLDER "/file_corrupt_footer_block_count";

    KFile * enc_file, * pt_file;
    
    uint64_t file_size;
    const size_t buffer_size = sizeof(KEncFileFooter);
    size_t num_written, num_read;
    uint8_t buffer[buffer_size];
    
    struct KDirectory * current_dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &current_dir ) );
    
    // just in case if it still there
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    
    // create file
    REQUIRE_RC ( TCreateEncFile( current_dir, file_path, TFileOpenMode_Write, &key, &enc_file ) );
    REQUIRE_RC ( TFillFile( enc_file, (const uint8_t *)"\0\1", 2, 500 ) );
    REQUIRE_RC ( KFileRelease ( enc_file ) );
    
    // corrupt footer
    REQUIRE_RC ( TOpenPtFile ( current_dir, file_path, TFileOpenMode_ReadWrite, &pt_file ) );
    REQUIRE_RC ( KFileSize ( pt_file, &file_size ) );
    REQUIRE_RC ( KFileReadAll ( pt_file, file_size - buffer_size, buffer, buffer_size, &num_read ) );
    assert(buffer_size == num_read);
    ((KEncFileFooter*)&buffer)->block_count ^= 4;
    REQUIRE_RC ( KFileWriteAll ( pt_file, file_size - buffer_size, buffer, buffer_size, &num_written ) );
    assert(buffer_size == num_written);
    REQUIRE_RC ( KFileRelease ( pt_file ) );
    
    REQUIRE_RC ( TOpenPtFile( current_dir, file_path, TFileOpenMode_Read, &pt_file ) );
    LOGMSG ( klogWarn, "Expect errors after this line:" );
    REQUIRE ( KEncFileValidate( pt_file ) == RC(rcKrypto, rcFile, rcValidating, rcSize, rcIncorrect) );
    LOGMSG ( klogWarn, "No more errors are expected" );
    REQUIRE_RC ( KFileRelease ( pt_file ) );
    
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );

    REQUIRE_RC ( KDirectoryRelease ( current_dir ) );
}

TEST_CASE(KDectryptCorruptBlockStruct)
{
    const char pw [] = "first pw";
    KKey key;
    REQUIRE_RC (KKeyInitUpdate (&key, kkeyAES128, pw, strlen (pw)));
    
    const char file_path [] = TMP_FOLDER "/file_corrupt_block_struct";

    KFile * enc_file, * pt_file;
    
    uint64_t file_size;
    const size_t buffer_size = sizeof(KEncFileBlock);
    size_t num_written, num_read;
    uint8_t buffer[buffer_size];
    
    struct KDirectory * current_dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &current_dir ) );
    
    // just in case if it still there
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    
    // create file
    REQUIRE_RC ( TCreateEncFile( current_dir, file_path, TFileOpenMode_Write, &key, &enc_file ) );
    REQUIRE_RC ( TFillFile( enc_file, (const uint8_t *)"\0\1", 2, 500 ) );
    REQUIRE_RC ( KFileRelease ( enc_file ) );
    
    // corrupt block struct
    REQUIRE_RC ( TOpenPtFile ( current_dir, file_path, TFileOpenMode_ReadWrite, &pt_file ) );
    REQUIRE_RC ( KFileSize ( pt_file, &file_size ) );
    REQUIRE_RC ( KFileReadAll ( pt_file, file_size - sizeof(KEncFileFooter) - buffer_size, buffer, buffer_size, &num_read ) );
    assert(buffer_size == num_read);
    buffer[0] ^= 4;
    REQUIRE_RC ( KFileWriteAll ( pt_file, file_size - sizeof(KEncFileFooter) - buffer_size, buffer, buffer_size, &num_written ) );
    assert(buffer_size == num_written);
    REQUIRE_RC ( KFileRelease ( pt_file ) );
    
    REQUIRE_RC ( TOpenPtFile( current_dir, file_path, TFileOpenMode_Read, &pt_file ) );
    LOGMSG ( klogWarn, "Expect errors after this line:" );
    REQUIRE ( KEncFileValidate( pt_file ) == RC(rcKrypto, rcFile, rcValidating, rcChecksum, rcCorrupt) );
    LOGMSG ( klogWarn, "No more errors are expected" );
    REQUIRE_RC ( KFileRelease ( pt_file ) );
    
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );

    REQUIRE_RC ( KDirectoryRelease ( current_dir ) );
}

TEST_CASE(KDectryptCorruptBlockData)
{
    const char pw [] = "first pw";
    KKey key;
    REQUIRE_RC (KKeyInitUpdate (&key, kkeyAES128, pw, strlen (pw)));
    
    const char file_path [] = TMP_FOLDER "/file_corrupt_block_data";

    KFile * enc_file, * pt_file;
    
    uint64_t file_size;
    const size_t buffer_size = sizeof(KEncFileBlock);
    size_t num_written, num_read;
    uint8_t buffer[buffer_size];
    
    struct KDirectory * current_dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &current_dir ) );
    
    // just in case if it still there
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    
    // create file
    REQUIRE_RC ( TCreateEncFile( current_dir, file_path, TFileOpenMode_Write, &key, &enc_file ) );
    REQUIRE_RC ( TFillFile( enc_file, (const uint8_t *)"\0\1", 2, 500 ) );
    REQUIRE_RC ( KFileRelease ( enc_file ) );
    
    // corrupt block struct
    REQUIRE_RC ( TOpenPtFile ( current_dir, file_path, TFileOpenMode_ReadWrite, &pt_file ) );
    REQUIRE_RC ( KFileSize ( pt_file, &file_size ) );
    REQUIRE_RC ( KFileReadAll ( pt_file, file_size - sizeof(KEncFileFooter) - buffer_size, buffer, buffer_size, &num_read ) );
    assert(buffer_size == num_read);
    ((KEncFileBlock*)&buffer)->data[0] ^= 4;
    REQUIRE_RC ( KFileWriteAll ( pt_file, file_size - sizeof(KEncFileFooter) - buffer_size, buffer, buffer_size, &num_written ) );
    assert(buffer_size == num_written);
    REQUIRE_RC ( KFileRelease ( pt_file ) );

    REQUIRE_RC ( TOpenPtFile( current_dir, file_path, TFileOpenMode_Read, &pt_file ) );
    LOGMSG ( klogWarn, "Expect errors after this line:" );
    REQUIRE ( KEncFileValidate( pt_file ) == RC(rcKrypto, rcFile, rcValidating, rcChecksum, rcCorrupt) );
    LOGMSG ( klogWarn, "No more errors are expected" );
    REQUIRE_RC ( KFileRelease ( pt_file ) );
    
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );

    REQUIRE_RC ( KDirectoryRelease ( current_dir ) );
}


TEST_CASE(KDectryptInvalidKey)
{
    const char pw1 [] = "first pw";
    const char pw2 [] = "second pw";
    KKey key1, key2;
    REQUIRE_RC (KKeyInitUpdate (&key1, kkeyAES128, pw1, strlen (pw1)));
    REQUIRE_RC (KKeyInitUpdate (&key2, kkeyAES128, pw2, strlen (pw2)));
    
    const char file_path [] = TMP_FOLDER "/enc_file_invalid_key";

    KFile * enc_file;
    
    struct KDirectory * current_dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &current_dir ) );
    
    // just in case if it still there
    KDirectoryRemove ( current_dir, true, TMP_FOLDER );
    
    // create file
    REQUIRE_RC ( TCreateEncFile( current_dir, file_path, TFileOpenMode_Write, &key1, &enc_file ) );
    REQUIRE_RC ( TFillFile( enc_file, (const uint8_t *)"\0\1", 2, 500 ) );
    REQUIRE_RC ( KFileRelease ( enc_file ) );
    
    REQUIRE_RC ( TOpenEncFile( current_dir, file_path, TFileOpenMode_Read, &key2, &enc_file ) );
    LOGMSG ( klogWarn, "Expect errors after this line:" );
    REQUIRE ( TCheckFileContent( enc_file, (const uint8_t *)"\0\1", 2 ) == RC( rcKrypto, rcFile, rcValidating, rcEncryption, rcCorrupt ) );
    LOGMSG ( klogWarn, "No more errors are expected" );
    REQUIRE_RC ( KFileRelease ( enc_file ) );
    
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
    
    const char UsageDefaultName[] = "test-encdec";
    
    rc_t CC KMain ( int argc, char *argv [] )
    {
        KConfigDisableUserSettings();
        rc_t rc=KEncDecTestSuite(argc, argv);
        return rc;
    }
    
}
