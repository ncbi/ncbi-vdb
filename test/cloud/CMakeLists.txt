add_executable( test-aws test-aws )
add_executable( test-gcp test-gcp TestStream )
add_executable( test-cloud test-cloud )

# TODO: ALL_TOOLS, scripted tests

set( CLOUD_LIB
    ktst
    kapp
    ncbi-vdb
)

add_definitions (-D__mod__="test/cloud")

# TODO: fails to link with kapp
target_link_libraries( test-aws ${CLOUD_LIB} )
target_link_libraries( test-gcp ${CLOUD_LIB} )
target_link_libraries( test-cloud ${CLOUD_LIB} )

add_test( NAME Test_CLOUD_aws COMMAND test-aws WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
add_test( NAME Test_CLOUD_gcp COMMAND test-gcp WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
add_test( NAME Test_CLOUD_cloud COMMAND test-cloud WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )