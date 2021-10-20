add_compile_definitions( __mod__="test/align-access" )

add_executable( test-align-access test main )
MSVS_StaticRuntime( test-align-access )
target_link_libraries( test-align-access ${COMMON_LINK_LIBRARIES} ${COMMON_LIBS_READ} )
add_test( NAME Test_Align_access COMMAND test-align-access WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
