# ===========================================================================
#
#                            PUBLIC DOMAIN NOTICE
#               National Center for Biotechnology Information
#
#  This software/database is a "United States Government Work" under the
#  terms of the United States Copyright Act.  It was written as part of
#  the author's official duties as a United States Government employee and
#  thus cannot be copyrighted.  This software/database is freely available
#  to the public for use. The National Library of Medicine and the U.S.
#  Government have not placed any restriction on its use or reproduction.
#
#  Although all reasonable efforts have been taken to ensure the accuracy
#  and reliability of the software and data, the NLM and the U.S.
#  Government do not and cannot warrant the performance or results that
#  may be obtained by using this software or data. The NLM and the U.S.
#  Government disclaim all warranties, express or implied, including
#  warranties of performance, merchantability or fitness for any particular
#  purpose.
#
#  Please cite the author in any work or product based on this material.
#
# ===========================================================================

add_compile_definitions( __mod__="test/schema" )
#set( CMAKE_DISABLE_SOURCE_CHANGES OFF )
file( MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/actual ) # TODO remove ./actual dir after tests pass (add_test POST?)
#set( CMAKE_DISABLE_SOURCE_CHANGES ON ) # Cannot use with bison

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

if (Python3_EXECUTABLE)
    add_test( NAME Test_SCHEMA_454_db
        COMMAND ${Python3_EXECUTABLE} test_VDB-5250.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_tests_properties( Test_SCHEMA_454_db
        PROPERTIES
        ENVIRONMENT VDB_LIBRARY_PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    )

    add_test( NAME Test_SCHEMA_IonTorrent_db
        COMMAND ${Python3_EXECUTABLE} test_VDB-5251.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_tests_properties( Test_SCHEMA_IonTorrent_db
        PROPERTIES
        ENVIRONMENT VDB_LIBRARY_PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    )
endif()
