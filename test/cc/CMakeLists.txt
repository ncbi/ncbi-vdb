add_executable(test-cc-asm
    asm-test
)

add_compile_definitions( __mod__="test/cc" )

target_link_libraries( test-cc-asm ${COMMON_LIBS_READ} )
MSVS_StaticRuntime( test-cc-asm )

add_test( NAME Test_CC_Asm COMMAND test-cc-asm )