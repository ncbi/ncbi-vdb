add_compile_definitions( __mod__="test/kns" )

AddExecutableTest( Test_KNS_proxy-with-scheme_failure "test-proxy-with-scheme_failure" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KNS_allow_all_certificates "test_allow_all_certificates" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KNS_dis_allow_all_certificates "test_dis_allow_all_certificates" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KNS_google-proxy "test-google-proxy" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KNS_test200for-whole-file "test200for-whole-file" "${COMMON_LIBS_READ}" )

# TODO: only succeeds when the executable name is test-kns
set( test_kns_name "test-kns" )
add_executable( ${test_kns_name} knstest )
MSVS_StaticRuntime( ${test_kns_name} )
target_link_libraries( ${test_kns_name} ${COMMON_LIBS_READ} )
add_test( NAME Test_KNS COMMAND ${test_kns_name} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
#AddExecutableTest( Test_KNS "knstest" "${COMMON_LIBS_READ}" )

AddExecutableTest( SlowTest_KNS_ipc "test-ipc" "${COMMON_LIBS_READ}" )
AddExecutableTest( SlowTest_KNS_http "HttpFixture;httptest" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KNS_http-dropconn "http_dropconnection_test" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KNS_http_request "HttpFixture;test-http-request" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KNS_KNSManagerSingletonTest "KNSManagerSingletonTest" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KNS_connect "HttpFixture;test-connect" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KNS_refresh-expired "HttpFixture;KStableHttpFile;test-refresh-expired-url" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KNS_timeouts "test-timeouts" "${COMMON_LIBS_READ}" )

# TODO the following binaries are not unit tests (EXT_TOOLS), they should be started not as ctest

BuildExecutableForTest( test-kns-mutual-authentication "test-mutual-authentication;gosha" "${COMMON_LIBS_READ}" )
# add_test( NAME Test_KNS_mutual-authentication COMMAND test-kns-mutual-authentication WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

BuildExecutableForTest( test-kns-proxy-with-env "test-proxy-with-env" "${COMMON_LIBS_READ}" )
# add_test( NAME Test_KNS_proxy-with-env COMMAND test-kns-proxy-with-env WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

if( SINGLE_CONFIG )

    BuildExecutableForTest( Test_KNS_proxy "test-proxy" "${COMMON_LIBS_READ}" )
    add_test( NAME Test_KNS_proxy COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test-proxy.sh ${CMAKE_TEST_OUTPUT_DIRECTORY}/Test_KNS_proxy WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

    BuildExecutableForTest( Test_KNS_proxy-with-scheme "test-proxy-with-scheme" "${COMMON_LIBS_READ}" )
    add_test( NAME Test_KNS_proxy-with-scheme COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test-proxy-with-scheme.sh ${CMAKE_TEST_OUTPUT_DIRECTORY}/Test_KNS_proxy-with-scheme WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
    #set_tests_properties(Test_KNS_proxy-with-scheme PROPERTIES ENVIRONMENT http_proxy=)

endif()
