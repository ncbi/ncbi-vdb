add_executable(test-mbedtls
    test-mbedtls
)
MSVS_StaticRuntime( test-mbedtls )

add_compile_definitions( __mod__="test/ext/mbedtls" )

target_link_libraries( test-mbedtls  ${COMMON_LIBS_READ} )

add_test( NAME Test_EXT_mbedtls COMMAND test-mbedtls WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})