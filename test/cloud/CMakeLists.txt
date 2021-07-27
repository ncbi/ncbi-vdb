add_executable( test-aws test-aws )
add_executable( test-gcp test-gcp TestStream )
add_executable( test-cloud test-cloud )

# TODO: ALL_TOOLS, scripted tests

add_compile_definitions( __mod__="test/cloud" )

target_link_libraries( test-aws ${COMMON_LINK_LIBRARIES} ${COMMON_LIBS_READ} )
target_link_libraries( test-gcp ${COMMON_LINK_LIBRARIES} ${COMMON_LIBS_READ} )
target_link_libraries( test-cloud ${COMMON_LINK_LIBRARIES} ${COMMON_LIBS_READ} )

MSVS_StaticRuntime( test-aws )
MSVS_StaticRuntime( test-gcp )
MSVS_StaticRuntime( test-cloud )

add_test( NAME Test_CLOUD_aws COMMAND test-aws WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
add_test( NAME Test_CLOUD_gcp COMMAND test-gcp WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
add_test( NAME Test_CLOUD_cloud COMMAND test-cloud WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )