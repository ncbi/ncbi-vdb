add_executable(test-zstd
    test-zstd
)

add_definitions (-D__mod__="test/ext/zstd")

target_link_libraries( test-zstd  ${COMMON_LIBS_READ} )

add_test( NAME Test_EXT_zstd COMMAND test-zstd )