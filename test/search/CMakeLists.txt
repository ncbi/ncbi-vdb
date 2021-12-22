add_compile_definitions( __mod__="test/search" )

AddExecutableTest( Test_SEARCH_agrep "test-agrep;agrep-main;search-vdb" "${COMMON_LIBS_READ}" )