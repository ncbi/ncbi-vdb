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

add_compile_definitions( __mod__="test/vdb" )

if( WIN32 )
    set( ADDITIONAL_LIBS Crypt32 )
else()
    set( ADDITIONAL_LIBS "" )
endif()

AddExecutableTest( SlowTest_VDB_vdb "test-vdb" "${COMMON_LIBS_READ}" )

AddExecutableTest( Test_VDB_wvdb "test-wvdb" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS}" )
set_tests_properties(Test_VDB_wvdb PROPERTIES ENVIRONMENT VDB_CONFIG=${CMAKE_CURRENT_SOURCE_DIR}/kfg/empty)

# temp disable test
AddExecutableTest( Test_VDB_pagemap "test-pagemap" "${COMMON_LIBS_WRITE}" )

AddExecutableTest( SlowTest_VDB_dependencies "test-dependencies" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_VDB_sparse_col_rw "test-sparse-col" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS}" )
set_tests_properties( Test_VDB_sparse_col_rw PROPERTIES FIXTURES_SETUP SPARSE_COL )

AddExecutableTest( Test_VDB_sparse_col_rd "test-sparse-col" "${COMMON_LIBS_READ}" )
target_compile_definitions(Test_VDB_sparse_col_rd PUBLIC READ_ONLY)
set_tests_properties(Test_VDB_sparse_col_rd PROPERTIES FIXTURES_REQUIRED SPARSE_COL )
if( RUN_SANITIZER_TESTS )
    target_compile_definitions(Test_VDB_sparse_col_rd_asan PUBLIC READ_ONLY)
    set_tests_properties(Test_VDB_sparse_col_rd_asan PROPERTIES FIXTURES_REQUIRED SPARSE_COL)
    target_compile_definitions(Test_VDB_sparse_col_rd_tsan PUBLIC READ_ONLY)
    set_tests_properties(Test_VDB_sparse_col_rd_tsan PROPERTIES FIXTURES_REQUIRED SPARSE_COL)
endif()

AddExecutableTest( Test_VDB_blob_val "test-blob-val" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_VDB_VDB-3060 "test-VDB-3060" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_VDB_VDB-3061 "test-VDB-3061" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_VDB_bytecode "test-bytecode" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS}" )
AddExecutableTest( Test_VDB_database_cmn "test-database-cmn" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS}" )
AddExecutableTest( Test_VDB_view "test-view" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS};schema" )
AddExecutableTest( Test_VDB_wtablecursor "test-wtablecursor" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS}" )
AddExecutableTest( Test_VDB_view_cursor "test-view-cursor" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS};schema" )
AddExecutableTest( Test_VDB_VDB-4156 "test-VDB-4156" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS}" )

# Slow tests
AddExecutableTest( SlowTest_VDB_tablecursor "test-tablecursor" "${COMMON_LIBS_READ}" )
AddExecutableTest( SlowTest_VDB_vdb-slow "test-vdb-slow" "${COMMON_LIBS_READ}" )
AddExecutableTest( SlowTest_VDB_wvdb-slow "test-wvdb-slow" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS}" )

# TODO: make sure runs are not cached in the user repository when running tests

# TODO: valgrind
