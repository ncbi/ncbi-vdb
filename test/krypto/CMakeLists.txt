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
