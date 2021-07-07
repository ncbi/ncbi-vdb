add_executable( test-aws test-aws )
add_executable( test-gcp test-gcp TestStream )
add_executable( test-cloud test-cloud )

# TODO: ALL_TOOLS, scripted tests

add_definitions (-D__mod__="test/cloud")

target_link_libraries( test-aws ${COMMON_LIBS_READ} )
target_link_libraries( test-gcp ${COMMON_LIBS_READ} )
target_link_libraries( test-cloud ${COMMON_LIBS_READ} )

add_test( NAME Test_CLOUD_aws COMMAND test-aws WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
add_test( NAME Test_CLOUD_gcp COMMAND test-gcp WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
add_test( NAME Test_CLOUD_cloud COMMAND test-cloud WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )