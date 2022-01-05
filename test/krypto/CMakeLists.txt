# ===========================================================================
#
#                            PUBLIC DOMAIN NOTICE
#               National Center for Biotechnology Information
#
#  This software/database is a "United States Government Work" under the
#  terms of the United States Copyright Act.  It was written as part of
#  the author's official duties as a United States Government employee and
#  thus cannot be copyrighted.  This software/database is freely available
#  to the public for use. The National Library of Medicine and the U.S.
#  Government have not placed any restriction on its use or reproduction.
#
#  Although all reasonable efforts have been taken to ensure the accuracy
#  and reliability of the software and data, the NLM and the U.S.
#  Government do not and cannot warrant the performance or results that
#  may be obtained by using this software or data. The NLM and the U.S.
#  Government disclaim all warranties, express or implied, including
#  warranties of performance, merchantability or fitness for any particular
#  purpose.
#
#  Please cite the author in any work or product based on this material.
#
# ===========================================================================

add_compile_definitions( __mod__="test/krypto" )

if( WIN32 )
    set( ADDITIONAL_LIBS Crypt32 )
else()
    set( ADDITIONAL_LIBS "" )
endif()	

BuildExecutableForTest( SlowTest_KRYPTO_cipher_speed "test-cipher-speed" "kryptotest;${COMMON_LIBS_READ}" )
add_test( NAME SlowTest_KRYPTO_cipher_speed COMMAND SlowTest_KRYPTO_cipher_speed -t 10 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

AddExecutableTest( Test_KRYPTO_modes "test-modes" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KRYPTO_encdec "test-encdec;test-cmn" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KRYPTO_reenc "test-reenc;test-cmn" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KRYPTO_rsa-aes-hmac "test-rsa-aes-hmac" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KRYPTO_hash-in-pwd "test-hash-in-pwd" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KRYPTO_aes-ciphers "test-aes-ciphers" "kryptotest;${COMMON_LIBS_READ}" )

AddExecutableTest( SlowTest_KRYPTO_krypto-slow "test-krypto-slow;test-cmn" "${COMMON_LIBS_READ}" )
