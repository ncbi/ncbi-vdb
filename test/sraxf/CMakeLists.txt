add_compile_definitions( __mod__="test/sraxf" )

add_executable( test-format_spot_name test-format_spot_name )
MSVS_StaticRuntime( test-format_spot_name )
target_link_libraries( test-format_spot_name ${COMMON_LIBS_READ} )
add_test( NAME Test_SRAXF_format_spot_name COMMAND test-format_spot_name WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-fix_read_seg fix_read_seg-test )
MSVS_StaticRuntime( test-fix_read_seg )
target_link_libraries( test-fix_read_seg ${COMMON_LIBS_READ} )
add_test( NAME Test_SRAXF_fix_read_seg COMMAND test-fix_read_seg WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-syn_quality test_syn_quality )
MSVS_StaticRuntime( test-syn_quality )
target_link_libraries( test-syn_quality  ${COMMON_LIBS_READ} )
add_test( NAME Test_SRAXF_syn_quality COMMAND test-syn_quality WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
