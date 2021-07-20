add_executable(test-ktst
    ktsttest
)

add_compile_definitions( __mod__="test/ktst" )

target_link_libraries( test-ktst ${COMMON_LIBS_READ} )

add_test( NAME Test_KTST COMMAND test-ktst )