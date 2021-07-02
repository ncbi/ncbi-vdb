add_executable(test-cc-asm
    asm-test
)

add_definitions (-D__mod__="test/cc")

target_link_libraries( test-cc-asm ${COMMON_LIBS_READ} )

add_test( NAME Test_CC_Asm COMMAND test-cc-asm )