add_compile_definitions( __mod__="test/kns" )

add_executable( test_allow_all_certificates test_allow_all_certificates )
MSVS_StaticRuntime( test_allow_all_certificates )
target_link_libraries( test_allow_all_certificates ${COMMON_LIBS_READ} )
add_test( NAME Test_KNS_allow_all_certificates COMMAND test_allow_all_certificates WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test_dis_allow_all_certificates test_dis_allow_all_certificates )
MSVS_StaticRuntime( test_dis_allow_all_certificates )
target_link_libraries( test_dis_allow_all_certificates ${COMMON_LIBS_READ} )
add_test( NAME Test_KNS_dis_allow_all_certificates COMMAND test_dis_allow_all_certificates WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-google-proxy test-google-proxy )
MSVS_StaticRuntime( test-google-proxy )
target_link_libraries( test-google-proxy ${COMMON_LIBS_READ} )
add_test( NAME Test_KNS_google-proxy COMMAND test-google-proxy WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test200for-whole-file test200for-whole-file )
MSVS_StaticRuntime( test200for-whole-file )
target_link_libraries( test200for-whole-file ${COMMON_LIBS_READ} )
add_test( NAME Test_KNS_test200for-whole-file COMMAND test200for-whole-file WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-kns knstest )
MSVS_StaticRuntime( test-kns )
target_link_libraries( test-kns ${COMMON_LIBS_READ} )
add_test( NAME Test_KNS COMMAND test-kns WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-ipc test-ipc )
MSVS_StaticRuntime( test-ipc )
target_link_libraries( test-ipc ${COMMON_LIBS_READ} )
add_test( NAME SlowTest_KNS_ipc COMMAND test-ipc WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-http HttpFixture httptest )
MSVS_StaticRuntime( test-http )
target_link_libraries( test-http ${COMMON_LIBS_READ} )
add_test( NAME SlowTest_KNS_http COMMAND test-http WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-http-dropconn http_dropconnection_test )
MSVS_StaticRuntime( test-http-dropconn )
target_link_libraries( test-http-dropconn ${COMMON_LIBS_READ} )
add_test( NAME Test_KNS_http-dropconn COMMAND test-http-dropconn WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-http-request HttpFixture test-http-request )
MSVS_StaticRuntime( test-http-request )
target_link_libraries( test-http-request ${COMMON_LIBS_READ} )
add_test( NAME Test_KNS_http_request COMMAND test-http-request WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( KNSManagerSingletonTest KNSManagerSingletonTest )
MSVS_StaticRuntime( KNSManagerSingletonTest )
target_link_libraries( KNSManagerSingletonTest ${COMMON_LIBS_READ} )
add_test( NAME Test_KNS_KNSManagerSingletonTest COMMAND KNSManagerSingletonTest WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-connect HttpFixture test-connect )
MSVS_StaticRuntime( test-connect )
target_link_libraries( test-connect ${COMMON_LIBS_READ} )
add_test( NAME Test_KNS_connect COMMAND test-connect WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-refresh-expired HttpFixture KStableHttpFile test-refresh-expired-url )
MSVS_StaticRuntime( test-refresh-expired )
target_link_libraries( test-refresh-expired ${COMMON_LIBS_READ} )
add_test( NAME Test_KNS_refresh-expired COMMAND test-refresh-expired WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-timeouts test-timeouts )
MSVS_StaticRuntime( test-timeouts )
target_link_libraries( test-timeouts ${COMMON_LIBS_READ} )
add_test( NAME Test_KNS_timeouts COMMAND test-timeouts WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

# TODO the following binaries are not unit tests (EXT_TOOLS), they should be started not as ctest

add_executable( test-mutual-authentication test-mutual-authentication gosha )
MSVS_StaticRuntime( test-mutual-authentication )
target_link_libraries( test-mutual-authentication ${COMMON_LIBS_READ} )
# add_test( NAME Test_KNS_mutual-authentication COMMAND test-mutual-authentication WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

if( NOT WIN32 )
    add_executable( test-proxy test-proxy )
    MSVS_StaticRuntime( test-proxy )
    target_link_libraries( test-proxy ${COMMON_LIBS_READ} )
    add_test( NAME Test_KNS_proxy COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test-proxy.sh ${CMAKE_TEST_OUTPUT_DIRECTORY}/test-proxy WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
endif()

add_executable( test-proxy-with-env test-proxy-with-env )
MSVS_StaticRuntime( test-proxy-with-env )
target_link_libraries( test-proxy-with-env ${COMMON_LIBS_READ} )
# add_test( NAME Test_KNS_proxy-with-env COMMAND test-proxy-with-env WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

if( SINGLE_TARGET )
    add_executable( test-proxy-with-scheme test-proxy-with-scheme )
    MSVS_StaticRuntime( test-proxy-with-scheme )
    target_link_libraries( test-proxy-with-scheme ${COMMON_LIBS_READ} )
    add_test( NAME Test_KNS_proxy-with-scheme COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test-proxy-with-scheme.sh ${CMAKE_TEST_OUTPUT_DIRECTORY}/test-proxy-with-scheme WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
    #set_tests_properties(Test_KNS_proxy-with-scheme PROPERTIES ENVIRONMENT http_proxy=)
endif()
