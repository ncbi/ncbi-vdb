add_compile_definitions( __mod__="test/search" )

add_executable( test-agrep test-agrep agrep-main search-vdb )
target_link_libraries( test-agrep  ${COMMON_LIBS_READ} )
add_test( NAME Test_SEARCH_agrep COMMAND test-agrep WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
