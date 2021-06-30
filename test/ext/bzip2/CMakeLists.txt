add_executable(test-bzip2
    test-bzip2
)

add_definitions (-D__mod__="test/ext/bzip2")

add_test( NAME Test_EXT_bzip2 COMMAND test-bzip2 )