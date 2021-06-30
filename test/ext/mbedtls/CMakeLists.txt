add_executable(test-mbedtls
    test-mbedtls
)

add_definitions (-D__mod__="test/ext/mbedtls")

add_test( NAME Test_EXT_mbedtls COMMAND test-mbedtls WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})