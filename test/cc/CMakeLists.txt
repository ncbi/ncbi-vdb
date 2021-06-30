add_executable(test-cc-asm
    asm-test
)

add_definitions (-D__mod__="test/cc")

add_test( NAME Test_CC_Asm COMMAND test-cc-asm )