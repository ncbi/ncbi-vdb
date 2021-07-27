add_compile_definitions( __mod__="test/axf" )

add_executable( test-RestoreRead test-RestoreRead )
MSVS_StaticRuntime( test-RestoreRead )
target_link_libraries( test-RestoreRead ${COMMON_LINK_LIBRARIES} ${COMMON_LIBS_READ} m )
add_test( NAME Test_Axf_RestoreRead COMMAND test-RestoreRead WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
