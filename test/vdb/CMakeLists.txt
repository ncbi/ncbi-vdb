add_executable( test-vdb test-vdb )
#add_executable( test-wvdb test-wvdb )
add_executable( test-dependencies test-dependencies ) # fails
add_executable( test-sparse-col-rw test-sparse-col )
add_executable( test-sparse-col-rd test-sparse-col )
# add_executable( test-blob-val
# add_executable( test-VDB-3060
# add_executable( test-VDB-3061
# add_executable( test-bytecode
# add_executable( test-view
# add_executable( test-tablecursor
# add_executable( test-wtablecursor
# add_executable( test-view-cursor
# add_executable( test-VDB-4156

# Slow tests
# add_executable( test-vdb-slow 
# add_executable( test-wvdb-slow 

set( READ_LIBS
    kapp
    ktst
    ncbi-vdb
    dl
)

set( WRITE_LIBS
    kapp
    ktst
    ncbi-wvdb
    dl
)

# TODO: make sure runs are not cached in the user repository when running tests

add_definitions (-D__mod__="test/vdb")

# TODO: link problems
target_link_libraries( test-vdb  ${READ_LIBS} )
#target_link_libraries( test-wvdb  ${WRITE_LIBS} )
target_link_libraries( test-dependencies  ${READ_LIBS} )
target_link_libraries( test-sparse-col-rw  ${WRITE_LIBS} )
target_link_libraries( test-sparse-col-rd  ${READ_LIBS} )

target_compile_definitions(test-sparse-col-rd PUBLIC READ_ONLY)

# TODO: valgrind

add_test( NAME Test_VDB_vdb COMMAND test-vdb WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
#add_test( NAME Test_VDB_wvdb COMMAND test-wvdb WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
add_test( NAME Test_VDB_dependencies COMMAND test-dependencies WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
add_test( NAME Test_VDB_sparse_col_rw COMMAND test-sparse-col-rw WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
add_test( NAME Test_VDB_sparse_col_rd COMMAND test-sparse-col-rd WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
