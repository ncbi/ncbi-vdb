add_compile_definitions( __mod__="test/loader" )

add_executable( test-loader loadertest sequence-writer )
target_link_libraries( test-loader  ${COMMON_LIBS_WRITE} load )
add_test( NAME Test_LOADER COMMAND test-loader WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )