add_executable(test-judy
    judytest
)
MSVS_StaticRuntime( test-judy )

add_compile_definitions( __mod__="test/judy" )

target_link_libraries( test-judy  ${COMMON_LIBS_READ} )

add_test( NAME Test_JUDY COMMAND test-judy )