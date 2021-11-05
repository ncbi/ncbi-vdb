add_compile_definitions( __mod__="test/cloud" )

AddExecutableTest( Test_CLOUD_aws "test-aws" "${COMMON_LINK_LIBRARIES};${COMMON_LIBS_READ}" )
AddExecutableTest( Test_CLOUD_gcp "test-gcp;TestStream" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_CLOUD_cloud "test-cloud" "${COMMON_LINK_LIBRARIES};${COMMON_LIBS_READ}" )
