add_executable(test-zstd
    test-zstd
)

add_definitions (-D__mod__="test/ext/zstd")

add_test( NAME Test_EXT_zstd COMMAND test-zstd )