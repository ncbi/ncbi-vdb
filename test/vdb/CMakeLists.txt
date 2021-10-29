add_compile_definitions( __mod__="test/vdb" )

if( WIN32 )
    set( ADDITIONAL_LIBS Crypt32 )
else()
    set( ADDITIONAL_LIBS "" )
endif()

AddExecutableTest( SlowTest_VDB_vdb "test-vdb" "${COMMON_LIBS_READ}" )

AddExecutableTest( Test_VDB_wvdb "test-wvdb" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS}" )
set_tests_properties(Test_VDB_wvdb PROPERTIES ENVIRONMENT VDB_CONFIG=${CMAKE_CURRENT_SOURCE_DIR}/kfg/empty)

AddExecutableTest( SlowTest_VDB_dependencies "test-dependencies" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_VDB_sparse_col_rw "test-sparse-col" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS}" )

AddExecutableTest( Test_VDB_sparse_col_rd "test-sparse-col" "${COMMON_LIBS_READ}" )
target_compile_definitions(Test_VDB_sparse_col_rd PUBLIC READ_ONLY)

AddExecutableTest( Test_VDB_blob_val "test-blob-val" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_VDB_VDB-3060 "test-VDB-3060" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_VDB_VDB-3061 "test-VDB-3061" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_VDB_bytecode "test-bytecode" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS}" )
AddExecutableTest( Test_VDB_view "test-view" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS};schema" )
AddExecutableTest( SlowTest_VDB_tablecursor "test-tablecursor" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_VDB_wtablecursor "test-wtablecursor" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS}" )
AddExecutableTest( Test_VDB_view_cursor "test-view-cursor" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS};schema" )
AddExecutableTest( Test_VDB_VDB-4156 "test-VDB-4156" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS}" )

# Slow tests
AddExecutableTest( SlowTest_VDB_vdb-slow "test-vdb-slow" "${COMMON_LIBS_READ}" )
AddExecutableTest( SlowTest_VDB_wvdb-slow "test-wvdb-slow" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS}" )

# TODO: make sure runs are not cached in the user repository when running tests

# TODO: valgrind
