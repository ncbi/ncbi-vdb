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

#include <ktst/unit_test.hpp>

#include <krypto/rsa-aes-hmac.h>
#include <klib/rc.h>
#include <klib/base64.h>
#include <klib/data-buffer.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfg/config.h>
#include <kapp/args.h>

#include <string.h>
#include <stdio.h>

/*

  RSA PEM key pair created just for this test using:

  $ openssl genrsa -des3 -out private.key 2048
  Enter pass phrase for private.key: test-rsa-aes-hmac
  $ openssl rsa -in private.key -pubout > public.key

  The two keys were then base64 enoded:
  $ base64 public.key
  $ base64 private.key

  The base64-encoded results are included as textual constants.
  The reason for their obfuscation, of course, is to attempt to
  avoid triggering alarms by security scans that get upset when
  they see private keys included in code or checked into a source
  repository. There is nothing private about these keys, however.

 */

// the public encryption key for this test
const char PEK_base64 [] =
    "LS0tLS1CRUdJTiBQVUJMSUMgS0VZLS0tLS0KTUlJQklqQU5CZ2txaGtpRzl3MEJBUUVGQUFPQ0FR"
    "OEFNSUlCQ2dLQ0FRRUFyNGQwc3F0eFlDczd3NVQ0N0hydgpCTEJuREcrQmFkTHBNbGhlS29NZ1c4"
    "bGxxMjFob0oxRlQ0d2U5bDlJWTVZWElsZURlcnhNS3pMKzIzVGZ2bGV5CkNxa28vS2NlanhGUjFV"
    "a2FFaXhsams2T1poRW9weW1uMVdVVmxhTEZvMDhRdklqWEJjREkxQ2RSRFhmTVFhY0kKb0thVWZy"
    "ODdseUxGeldPUnpaMU9WNmJhZHQwZGcyUWlGdGwycmhLZFVVTm1nWEFscHkzcmF3MFNEVStHRWFk"
    "VApaVFdYSi9xZUlNc1ZESm1TQmZ0M0puUjRTUFNLRlp4MVg0SjNhN2pZZFYxakxIeU05Y2pxR2hw"
    "UzJmeVRjYU1LCm9GRVdWRXRrc0NoelJQMmhFM245SmMwUHRFSVVRbkN2VWwwVXZaa0s2WDF1ODVK"
    "WE9NY3picUg3bGNyWVd3NGkKbndJREFRQUIKLS0tLS1FTkQgUFVCTElDIEtFWS0tLS0tCg==";

static
rc_t PEK ( char * pek_buffer, size_t pek_buffer_size )
{
    // capture base64-encoded public encryption key as a String
    String pek;
    CONST_STRING ( & pek, PEK_base64 );

    // decode into a KDataBuffer
    KDataBuffer decoded;
    rc_t rc = decodeBase64 ( & decoded, & pek );
    if ( rc == 0 )
    {
        size_t num_writ = string_copy ( pek_buffer, pek_buffer_size,
            ( const char * ) decoded . base, ( size_t ) decoded . elem_count );

        if ( num_writ == pek_buffer_size )
            rc = RC ( rcExe, rcString, rcDecoding, rcBuffer, rcInsufficient );
        else if ( ( size_t ) string_len ( pek_buffer, num_writ ) != num_writ )
            rc = RC ( rcExe, rcString, rcDecoding, rcConstraint, rcViolated );
        else if ( pek_buffer [ num_writ ] != 0 )
            rc = RC ( rcExe, rcString, rcDecoding, rcConstraint, rcViolated );
#if 0
        else
        {
            std :: cerr
                << "PEK =\n,  '"
                << pek_buffer
                << "'\n"
                ;
        }
#endif

        KDataBufferWhack ( & decoded );
    }

    return rc;
}

// the private decryption key material for this test
const char PDK_base64 [] =
    "LS0tLS1CRUdJTiBSU0EgUFJJVkFURSBLRVktLS0tLQpQcm9jLVR5cGU6IDQsRU5DUllQVEVECkRF"
    "Sy1JbmZvOiBERVMtRURFMy1DQkMsQjFGMTgwMjYwMEI2RTQ3MQoKcCtOMzJ3cTd6ZDhVT0VwcHdJ"
    "b2xTWTVWa3BTbmU4Mnl3eTJHUHJscStTZjZDYXNrUlFYS3dxbHRLWXE5eGpHVAphVmFVU21TbTFn"
    "Y0R3Q3pYUXhGODJNRkNoYWU2OGFidnRuTStmaFlVRFhSQUl6enBab1ZuYVhZdHYvNWYrM0JtCkla"
    "aFBoNFhCZHl5QXExSS9yZVNwaFY1S3pmSmw5YWlqaDF6Vjd6QzNXNTVqdkZDWFBPNEFUYThEbjA4"
    "dTE0Z1YKM0c4azN3RVNHaWpzL0FmanRjWFJSZTRFOWNYYUdHSDZYQ1R6cVpEdkQxZy9nT1djQnZV"
    "YStnREZodzljTVg2cApGR0N1UFdXN053Y3hUaUdWa3BmekZkTGZhSlpDNTJ4THh5aUZkbkpHWHB1"
    "Wmo0QUtWMWtvL0JqVndKR2F4ckVPCnk4UHpINHRwbFJvakxQdXl6U1hHTElGcEhwVUM5TVo5YzU4"
    "bkhiTkp2Z0RnNG9Kc24vMHQ5ZllGcmZITk9qdTEKZjlFaTI5VzROQ2lxTnRGeTVnMHV3MEV2S05G"
    "cG5xY1V6TDBHN2FucWRCc3BvRkQ1V0xOT1J0VGJNdFlabjVyeApUQ3p2R0JVdFZLVlVuandxSTFQ"
    "RW1xVkNKWmdrYnlCNGpLQTd3MjdXRVFjTU53clFQTmdPQ3pLc2FsNFp5TkxWCkhJcDhQWDNDNk5D"
    "QUgySHBzRnBNcERjcVR6QVlvMGd3SkpPcWhUbmVCUm1kTWpjRDJ1NDVLUG4yVktBYUx3eGoKREEv"
    "ekJsT1oxU2o1Tms3bStzSGxuc29zeXdpbGdQTjlOdlEvMnl0a0c0Qk9sRGRvZUNxUmI0aW95ajd3"
    "WEF2UgpqdE1jNUM3bkFxWnkyYXk5Zm9lZzUyUzljbHkwZFllV2lqTU1FZEQrR2xDeTU5WStzM3N2"
    "Nm9CalJOeVozaTZrCk5oelRIbldiQTR3TTJKcnNKZnRxME1haWQwMXk0QUlEV2J2cFdXMkVONmhG"
    "VlR5MmJDdHRFbWdMQm96M1I0QkcKdE5ZM2FlR3BqR2dOMmVKS3JQd3BCWnAyMmNocHd6TWJMTW9Q"
    "MU5BRis0ZEt5cWZxeU9VUE4rQjNPSzdTSUNlcwp0dndqZnJIOFdoSEYxZDFBbm11S2d5MDZkN2Yv"
    "UjdUcFhqSnhMNkgza2pBOVBYa1RoS1BpQmlVUWRWWEg4RC81CjBBMDhNL08zc3hEN2tCWk1UNHZG"
    "aStTVVU0U0pDeDd6L3AzTGsxcjJGODRiNkRWUWRLSGtoUkM0Y3dJdXhXYzYKQm9RN0c0aFpyZnU0"
    "RHVCOHExdlhWR1dhbkhNZU1nckIySndmT0ZDNUpZM2ZRQnA2czh1UmlSRWJIQnUraUI2WApQeTRl"
    "ekx0Z3FqRGNIK0dOeDYzSHlld1IwOWZDZ1ZBTFdjK3pDaVNXQ291V0xtNytMeTVWV0NZaEtwNlZ3"
    "WUphCjFxcDZzSVZ0Sm9hL1ZuV2RmQjNnZGF5dXJHY3VVRlJwMStsNDRySEFFbkdaVno5RzNEY3VS"
    "ZUxIQU1kSTJqZUEKb0hSS2xvM0l2NW1hUTFvd1M0enpSRUZ3ZzV1dExZc3hKTmFIZnFWUmVpb1g3"
    "QTVtZm5DbkdxN08rVGMzYmsvWQp1V2gwNE1uSFVCY3V6dDFEeStnWHZGYnNsYTd5end4Mmg3YTJq"
    "cXBnS0RiM2VXMVpUVVFxWU80aGp6KzFyNTJOCktMbHVBbklZV2ZBWEs3Ri80R2JZTDRPR25tWTda"
    "ZlB3N2d4d04vaFhNV1BQdExGT1NCamREMHdmRGFUbXVVYWoKVkFuWFNaSHRiclhaZWI2TkJpbHpu"
    "am5BeDlzaEMwTnZPdjhJaVRCSDl3bjlodGxhRmFkSDlNdlVTNDl6elhKcwpkRExudktFYUY0YUxk"
    "Zm5MdUkzVmZmTDhFWG9qOWN3S3VRWUdmR1dON204TjViNU5OVVRBOXlJVEI2NGZRRnR6CkNuREl6"
    "NzNRYjlKSWpXM0czODB5VzVyV1A2SEFIbHl2ZHp3MjVITnhsbUF1RnJoeFUwVjArbFdKM2ttc2pO"
    "TGcKQUtMT2d2ZDBhVkVyZEw2VGgyb1EwVGNucjlvUTRHN0ZjbDFPZ1NZNk9qVUdMVjZZMi80OGNU"
    "NzRaY3UzRFJyMgotLS0tLUVORCBSU0EgUFJJVkFURSBLRVktLS0tLQo=";

const char PDK_password [] = "test-rsa-aes-hmac";

static
rc_t makePDK ( const char * path )
{
    // capture base64-encoded private decryption key as a String
    String pdk;
    CONST_STRING ( & pdk, PDK_base64 );

    // decode into a KDataBuffer
    KDataBuffer decoded;
    rc_t rc = decodeBase64 ( & decoded, & pdk );
    if ( rc == 0 )
    {
        KDirectory * wd;
        rc = KDirectoryNativeDir ( & wd );
        if ( rc == 0 )
        {
            KFile * pdk_file;
            rc = KDirectoryCreateFile ( wd, & pdk_file, false, 0644, kcmInit, "%s", path );
            if ( rc == 0 )
            {
                rc = KFileWriteExactly ( pdk_file, 0,
                    decoded . base, ( size_t ) decoded . elem_count);
#if 0
                if ( rc == 0 )
                {
                    std :: cerr
                        << "PDK =\n,  '"
                        << std :: string ( ( char* ) decoded . base, ( size_t ) decoded . elem_count )
                        << "'\n"
                        ;
                }
#endif
                
                KFileRelease ( pdk_file );
            }
            KDirectoryRelease ( wd );
        }
        
        KDataBufferWhack ( & decoded );
    }

    return rc;
}

static
void whackPDK ( const char * path )
{
    KDirectory * wd;
    rc_t rc = KDirectoryNativeDir ( & wd );
    if ( rc == 0 )
    {
        KDirectoryRemove ( wd, false, "%s", path );
        KDirectoryRelease ( wd );
    }
}
    

TEST_SUITE ( KRsaAesHmacTestSuite );

TEST_CASE ( KEncryptDecryptRoundTrip )
{
    // prepare the parameters
    
    // parameter 1 - the output buffer
    // documentation says:
    //   pointer to a ZEROED KDataBuffer that will receive encrypted result.
    //   must be zeroed with "memset ( & out, 0, sizeof out );" or equivalent.
    KDataBuffer ct;
    :: memset ( & ct, 0, sizeof ct );

    // parameters 2 & 3 - the input data ( plaintext )
    // documentation says:
    //   block of data to be encrypted
    const char pt_orig [] = "this is really some super-secret text. I hope nobody can read it...";
    const size_t pt_size = sizeof pt_orig - 1;

    // parameter 4 - the RSA public encryption key
    // documentation says:
    //   NUL-terminated RSA public encryption key in PEM format.
    //
    // assemble public encryption key
    // needed by ( but not part of ) what's being tested
    char zpek [ 1024 ];
    REQUIRE_RC ( PEK ( zpek, sizeof zpek ) );

    // give it a try
    REQUIRE_RC ( RsaAesHmacEncrypt ( & ct
                                     , pt_orig, pt_size
                                     , zpek
                     ) );

    // okay! now prepare to decrypt it

    // parameter 1 - the output buffer
    // documentation says:
    //   pointer to a ZEROED KDataBuffer that will receive decrypted result.
    //   must be zeroed with "memset ( & out, 0, sizeof out );" or equivalent.
    KDataBuffer pt_prime;
    :: memset ( & pt_prime, 0, sizeof pt_prime );

    // parameters 2 & 3 - the input data ( including ciphertext )
    // documentation says:
    //   block of enciphered data to be decrypted
    // this is what we have in "ct"

    // parameter 4 - the RSA private decryption keyfile path
    // documentation says:
    //   a NUL-terminated FILE PATH to an RSA private decryption key in PEM format.
    //
    // we build this file here before decrypting
    const char zpdk [] = "./test-rsa-aes-hmac.pdk";
    REQUIRE_RC ( makePDK ( zpdk ) );

    // parameters 5 & 6 - the password for decrypting the pdk file, haha!
    // documentation says:
    //   a mutable buffer containing NUL-terminated password string used
    //   to decrypt the file designated by "zpdk". "zpwd_size" should be
    //   the total size of the BUFFER, not the size of the password itself.
    //   The buffer will be wiped after first use regardless of success or failure.
    //   
    // constant PDK_password is given above.
    char zpwd [ 256 ];
    :: strcpy ( zpwd, PDK_password );

    // decrypt
    REQUIRE_RC ( RsaAesHmacDecrypt ( & pt_prime
                                     , ct . base, ( size_t ) ct . elem_count
                                     , zpdk
                                     , zpwd, sizeof zpwd
                     ) );

    // finally, require that pt_orig and pt_prime compare favorably
    REQUIRE_EQ ( pt_size, ( size_t ) pt_prime . elem_count );
    REQUIRE_EQ ( :: memcmp ( ( const void * ) pt_orig, pt_prime . base, pt_size ), 0 );

    // clean up
    KDataBufferWhack ( & pt_prime );
    KDataBufferWhack ( & ct );
    whackPDK ( zpdk );
}

TEST_CASE ( KEncryptOneWayForNCBI )
{
    // prepare the parameters
    
    // parameter 1 - the output buffer
    // documentation says:
    //   pointer to a ZEROED KDataBuffer that will receive encrypted result.
    //   must be zeroed with "memset ( & out, 0, sizeof out );" or equivalent.
    KDataBuffer out;
    :: memset ( & out, 0, sizeof out );

    // parameters 2 & 3 - the input data ( plaintext )
    // documentation says:
    //   block of data to be encrypted
    const char pt [] = "this is really some super-secret text. I hope nobody can read it...";
    const size_t pt_size = sizeof pt - 1;

    // give it a try
    REQUIRE_RC ( EncryptForNCBI ( & out, pt, pt_size ) );

    // clean up
    KDataBufferWhack ( & out );
}

#if 0
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
#endif

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
    
    const char UsageDefaultName[] = "test-rsa-aes-hmac";
    
    rc_t CC KMain ( int argc, char *argv [] )
    {
        KConfigDisableUserSettings ();
        return KRsaAesHmacTestSuite ( argc, argv );
    }
    
}
