add_compile_definitions( __mod__="test/ext/zstd" )

AddExecutableTest( Test_EXT_zstd "test-zstd" "${COMMON_LIBS_READ}" )