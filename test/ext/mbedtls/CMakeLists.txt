add_executable(test-mbedtls
    test-mbedtls
)

target_link_libraries( test-mbedtls kapp ktst ncbi-vdb )
add_definitions (-D__mod__="test/ext/mbedtls")

add_test( NAME Test_EXT_mbedtls COMMAND test-mbedtls )