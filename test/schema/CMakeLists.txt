add_definitions (-D__mod__="test/schema")

add_executable( wb-test-schema-lex wb-test-schema-lex )
target_link_libraries( wb-test-schema-lex ${COMMON_LIBS_READ} schema )
add_test( NAME Test_SCHEMA_wb-schema-lex COMMAND wb-test-schema-lex WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

# TODO the following binaries are not unit tests (INT_TOOLS), they should be started not as ctest

add_executable( wb-test-schema-parser wb-test-schema-parser )
target_link_libraries( wb-test-schema-parser ${COMMON_LIBS_READ} schema )
#add_test( NAME Test_SCHEMA_wb-schema-parser COMMAND wb-test-schema-parser WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( wb-test-schema-ast wb-test-schema-ast AST_Fixture )
target_link_libraries( wb-test-schema-ast ${COMMON_LIBS_READ} schema )
#add_test( NAME Test_SCHEMA_wb-schema-ast COMMAND wb-test-schema-ast WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-schema-scan test-schema-scan )
target_link_libraries( test-schema-scan ${COMMON_LIBS_READ} schema )
#add_test( NAME Test_SCHEMA_schema-scan COMMAND test-schema-scan WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-schema-parse test-schema-parse )
target_link_libraries( test-schema-parse ${COMMON_LIBS_READ} schema )
#add_test( NAME Test_SCHEMA_schema-parse COMMAND test-schema-parse WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-schema-diff test-schema-diff )
target_link_libraries( test-schema-diff ${COMMON_LIBS_READ} schema )
#add_test( NAME Test_SCHEMA_schema-diff COMMAND test-schema-diff WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
