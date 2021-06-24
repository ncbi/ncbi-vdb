add_executable(test-asm
    asm-test
)

target_link_libraries( test-asm kapp ktst )
add_definitions (-D__mod__="test/cc")

add_test( NAME Test_CC_Asm COMMAND test-asm )