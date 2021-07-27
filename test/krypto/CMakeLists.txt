add_compile_definitions( __mod__="test/krypto" )

add_executable( test-cipher-speed test-cipher-speed )
MSVS_StaticRuntime( test-cipher-speed )
target_link_libraries( test-cipher-speed kryptotest ${COMMON_LIBS_READ} )
add_test( NAME SlowTest_KRYPTO_cipher_speed COMMAND test-cipher-speed -t 10 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-modes test-modes )
MSVS_StaticRuntime( test-modes )
target_link_libraries( test-modes ${COMMON_LIBS_READ} )
add_test( NAME Test_KRYPTO_modes COMMAND test-modes WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-encdec test-encdec test-cmn )
MSVS_StaticRuntime( test-encdec )
target_link_libraries( test-encdec ${COMMON_LIBS_READ} )
add_test( NAME Test_KRYPTO_encdec COMMAND test-encdec WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-reenc test-reenc test-cmn )
MSVS_StaticRuntime( test-reenc )
target_link_libraries( test-reenc ${COMMON_LIBS_READ} )
add_test( NAME Test_KRYPTO_reenc COMMAND test-reenc WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-rsa-aes-hmac test-rsa-aes-hmac )
MSVS_StaticRuntime( test-rsa-aes-hmac )
target_link_libraries( test-rsa-aes-hmac ${COMMON_LIBS_READ} )
add_test( NAME Test_KRYPTO_rsa-aes-hmac COMMAND test-rsa-aes-hmac WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-hash-in-pwd test-hash-in-pwd )
MSVS_StaticRuntime( test-hash-in-pwd )
target_link_libraries( test-hash-in-pwd ${COMMON_LIBS_READ} )
add_test( NAME Test_KRYPTO_hash-in-pwd COMMAND test-hash-in-pwd WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-aes-ciphers test-aes-ciphers )
MSVS_StaticRuntime( test-aes-ciphers )
target_link_libraries( test-aes-ciphers kryptotest ${COMMON_LIBS_READ} )
add_test( NAME Test_KRYPTO_aes-ciphers COMMAND test-aes-ciphers WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-krypto-slow test-krypto-slow test-cmn )
MSVS_StaticRuntime( test-krypto-slow )
target_link_libraries( test-krypto-slow ${COMMON_LIBS_READ} )
add_test( NAME SlowTest_KRYPTO_krypto-slow COMMAND test-krypto-slow WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
