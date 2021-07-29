add_compile_definitions( __mod__="test/vdb" )

if( WIN32 )
    set( ADDITIONAL_LIBS Crypt32 )
else()
    set( ADDITIONAL_LIBS "" )
endif()

add_executable( test-vdb test-vdb )
MSVS_StaticRuntime( test-vdb )
target_link_libraries( test-vdb  ${COMMON_LIBS_READ} )
add_test( NAME SlowTest_VDB_vdb COMMAND test-vdb WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-wvdb test-wvdb )
MSVS_StaticRuntime( test-wvdb )
target_link_libraries( test-wvdb  ${COMMON_LIBS_WRITE} )
add_test( NAME Test_VDB_wvdb COMMAND test-wvdb WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
set_tests_properties(Test_VDB_wvdb PROPERTIES ENVIRONMENT VDB_CONFIG=${CMAKE_CURRENT_SOURCE_DIR}/kfg/empty)

add_executable( test-dependencies test-dependencies )
MSVS_StaticRuntime( test-dependencies )
target_link_libraries( test-dependencies  ${COMMON_LIBS_READ} )
add_test( NAME SlowTest_VDB_dependencies COMMAND test-dependencies WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

# first run test-sparse-col-rw, which will create a database,
# then test-sparse-col-rd, which will read the databsase,
add_executable( test-sparse-col-rw test-sparse-col )
MSVS_StaticRuntime( test-sparse-col-rw )
target_link_libraries( test-sparse-col-rw  ${COMMON_LIBS_WRITE} ${ADDITIONAL_LIBS} )

add_executable( test-sparse-col-rd test-sparse-col )
MSVS_StaticRuntime( test-sparse-col-rd )
target_compile_definitions(test-sparse-col-rd PUBLIC READ_ONLY)
target_link_libraries( test-sparse-col-rd  ${COMMON_LIBS_READ} )

add_test( NAME Test_VDB_sparse_col_rw COMMAND test-sparse-col-rw WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
add_test( NAME Test_VDB_sparse_col_rd COMMAND test-sparse-col-rd WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
#

add_executable( test-blob-val test-blob-val )
MSVS_StaticRuntime( test-blob-val )
target_link_libraries( test-blob-val  ${COMMON_LIBS_READ} )
add_test( NAME Test_VDB_blob_val COMMAND test-blob-val WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-VDB-3060 test-VDB-3060 )
MSVS_StaticRuntime( test-VDB-3060 )
target_link_libraries( test-VDB-3060  ${COMMON_LIBS_READ} )
add_test( NAME Test_VDB_VDB-3060 COMMAND test-VDB-3060 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-VDB-3061 test-VDB-3061 )
MSVS_StaticRuntime( test-VDB-3061 )
target_link_libraries( test-VDB-3061  ${COMMON_LIBS_READ} )
add_test( NAME Test_VDB_VDB-3061 COMMAND test-VDB-3061 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-bytecode test-bytecode )
MSVS_StaticRuntime( test-bytecode )
target_link_libraries( test-bytecode  ${COMMON_LIBS_WRITE} ${ADDITIONAL_LIBS} )
add_test( NAME Test_VDB_bytecode COMMAND test-bytecode WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-view test-view )
MSVS_StaticRuntime( test-view )
target_link_libraries( test-view  ${COMMON_LIBS_WRITE} ${ADDITIONAL_LIBS} schema )
add_test( NAME Test_VDB_view COMMAND test-view WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-tablecursor test-tablecursor )
MSVS_StaticRuntime( test-tablecursor )
target_link_libraries( test-tablecursor  ${COMMON_LIBS_READ} )
add_test( NAME SlowTest_VDB_tablecursor COMMAND test-tablecursor WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-wtablecursor test-wtablecursor )
MSVS_StaticRuntime( test-wtablecursor )
target_link_libraries( test-wtablecursor  ${COMMON_LIBS_WRITE} ${ADDITIONAL_LIBS} )
add_test( NAME Test_VDB_wtablecursor COMMAND test-wtablecursor WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-view-cursor test-view-cursor )
MSVS_StaticRuntime( test-view-cursor )
target_link_libraries( test-view-cursor  ${COMMON_LIBS_WRITE} ${ADDITIONAL_LIBS} schema )
add_test( NAME Test_VDB_view_cursor COMMAND test-view-cursor WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-VDB-4156 test-VDB-4156 )
MSVS_StaticRuntime( test-VDB-4156 )
target_link_libraries( test-VDB-4156  ${COMMON_LIBS_WRITE} ${ADDITIONAL_LIBS} )
add_test( NAME Test_VDB_VDB-4156 COMMAND test-VDB-4156 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

# Slow tests
add_executable( test-vdb-slow test-vdb-slow )
MSVS_StaticRuntime( test-vdb-slow )
target_link_libraries( test-vdb-slow  ${COMMON_LIBS_READ} )
add_test( NAME SlowTest_VDB_vdb-slow COMMAND test-vdb-slow WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-wvdb-slow test-wvdb-slow )
MSVS_StaticRuntime( test-wvdb-slow )
target_link_libraries( test-wvdb-slow  ${COMMON_LIBS_WRITE} )
add_test( NAME SlowTest_VDB_wvdb-slow COMMAND test-wvdb-slow WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )


# TODO: make sure runs are not cached in the user repository when running tests

# TODO: valgrind
