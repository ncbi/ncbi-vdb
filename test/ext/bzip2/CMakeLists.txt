add_executable(test-bzip2
    test-bzip2
)

add_compile_definitions( __mod__="test/ext/bzip2" )

target_link_libraries( test-bzip2  ${COMMON_LIBS_READ} )

add_test( NAME Test_EXT_bzip2 COMMAND test-bzip2 )