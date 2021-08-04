add_compile_definitions( __mod__="test/schema" )
file( MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/actual ) # TODO remove ./actual dir after tests pass (add_test POST?)

add_executable( wb-test-schema-lex wb-test-schema-lex )
MSVS_StaticRuntime( wb-test-schema-lex )
target_link_libraries( wb-test-schema-lex ${COMMON_LIBS_READ} schema )
add_test( NAME Test_SCHEMA_wb-schema-lex COMMAND wb-test-schema-lex WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( wb-test-schema-parser wb-test-schema-parser )
MSVS_StaticRuntime( wb-test-schema-parser )
target_link_libraries( wb-test-schema-parser ${COMMON_LIBS_READ} schema )
add_test( NAME Test_SCHEMA_wb-schema-parser COMMAND wb-test-schema-parser WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( wb-test-schema-ast wb-test-schema-ast AST_Fixture )
MSVS_StaticRuntime( wb-test-schema-ast )
target_link_libraries( wb-test-schema-ast ${COMMON_LIBS_READ} schema )
add_test( NAME Test_SCHEMA_wb-schema-ast COMMAND wb-test-schema-ast WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

# TODO the following binaries are not unit tests (INT_TOOLS), they should be started not as ctest

if( SINGLE_CONFIG )
    add_executable( test-schema-scan test-schema-scan )
    target_link_libraries( test-schema-scan ${COMMON_LIBS_READ} schema )
    add_test( NAME Test_SCHEMA_schema-scan COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test-schema-scan.sh ${CMAKE_TEST_OUTPUT_DIRECTORY}/test-schema-scan ${CMAKE_SOURCE_DIR} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )


    add_executable( test-schema-parse test-schema-parse )
    target_link_libraries( test-schema-parse ${COMMON_LIBS_READ} schema )
    add_test( NAME Test_SCHEMA_schema-parse COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test-schema-parse.sh ${CMAKE_TEST_OUTPUT_DIRECTORY}/test-schema-parse ${CMAKE_SOURCE_DIR}  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

    add_executable( test-schema-diff test-schema-diff )
    target_link_libraries( test-schema-diff ${COMMON_LIBS_READ} schema )
    add_test( NAME Test_SCHEMA_schema-diff COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test-schema-diff.sh ${CMAKE_TEST_OUTPUT_DIRECTORY}/test-schema-diff ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
endif()
