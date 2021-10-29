add_compile_definitions( __mod__="test/ext/bzip2" )

AddExecutableTest( Test_EXT_bzip2 "test-bzip2" "${COMMON_LIBS_READ}" )