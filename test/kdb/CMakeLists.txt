add_executable( test-kdb kdbtest )
add_executable( test-wkdb wkdbtest )
add_executable( test-rowset rowsettest )
#add_executable( test-btree test-btree ) #TODO: revive test-btree

MSVS_StaticRuntime( test-kdb )
MSVS_StaticRuntime( test-wkdb )
MSVS_StaticRuntime( test-rowset )

# TODO: make sure runs are not cached in the user repository when running tests

add_compile_definitions( __mod__="test/kdb" )

#target_link_libraries( test-btree ncbi-vdb )
target_link_libraries( test-rowset ${COMMON_LIBS_WRITE} )
target_link_libraries( test-kdb ${COMMON_LIBS_READ} )
target_link_libraries( test-wkdb ${COMMON_LIBS_WRITE} )

#add_test( NAME Test_KDB_Btree COMMAND test-btree WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
add_test( NAME Test_KDB_Rowset COMMAND test-rowset WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
add_test( NAME Test_KDB_Kdb COMMAND test-kdb WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
add_test( NAME Test_KDB_Wkdb COMMAND test-wkdb WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
