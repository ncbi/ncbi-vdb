add_compile_definitions( __mod__="test/schema" )
file( MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/actual ) # TODO remove ./actual dir after tests pass (add_test POST?)

AddExecutableTest( Test_SCHEMA_wb-schema-lex "wb-test-schema-lex" "${COMMON_LIBS_READ};schema" )
AddExecutableTest( Test_SCHEMA_wb-schema-parser "wb-test-schema-parser" "${COMMON_LIBS_READ};schema" )
AddExecutableTest( Test_SCHEMA_wb-schema-ast "wb-test-schema-ast;AST_Fixture" "${COMMON_LIBS_READ};schema" )

# TODO the following binaries are not unit tests (INT_TOOLS), they should be started not as ctest

if( SINGLE_CONFIG )
    BuildExecutableForTest( Test_SCHEMA_schema-scan "test-schema-scan" "${COMMON_LIBS_READ};schema" )
    add_test( NAME Test_SCHEMA_schema-scan COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test-schema-scan.sh ${CMAKE_TEST_OUTPUT_DIRECTORY}/Test_SCHEMA_schema-scan ${CMAKE_SOURCE_DIR} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

    BuildExecutableForTest( Test_SCHEMA_schema-parse "test-schema-parse" "${COMMON_LIBS_READ};schema" )
    add_test( NAME Test_SCHEMA_schema-parse COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test-schema-parse.sh ${CMAKE_TEST_OUTPUT_DIRECTORY}/Test_SCHEMA_schema-parse ${CMAKE_SOURCE_DIR}  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

    BuildExecutableForTest( Test_SCHEMA_schema-diff "test-schema-diff" "${COMMON_LIBS_READ};schema" )
    add_test( NAME Test_SCHEMA_schema-diff COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test-schema-diff.sh ${CMAKE_TEST_OUTPUT_DIRECTORY}/Test_SCHEMA_schema-diff ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
endif()
