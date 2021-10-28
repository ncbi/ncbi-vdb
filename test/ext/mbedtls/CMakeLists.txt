add_compile_definitions( __mod__="test/ext/mbedtls" )

AddExecutableTest( Test_EXT_mbedtls "test-mbedtls" "${COMMON_LIBS_READ}" )