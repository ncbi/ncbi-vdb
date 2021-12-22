add_compile_definitions( __mod__="test/sraxf" )

AddExecutableTest( Test_SRAXF_format_spot_name "test-format_spot_name" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_SRAXF_fix_read_seg "fix_read_seg-test" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_SRAXF_syn_quality "test_syn_quality" "${COMMON_LIBS_READ}" )
