add_compile_definitions( __mod__="test/kns" )

add_executable( test-kns-allow_all_certificates test_allow_all_certificates )
MSVS_StaticRuntime( test-kns-allow_all_certificates )
target_link_libraries( test-kns-allow_all_certificates ${COMMON_LIBS_READ} )
add_test( NAME Test_KNS_allow_all_certificates COMMAND test-kns-allow_all_certificates WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-kns-dis_allow_all_certificates test_dis_allow_all_certificates )
MSVS_StaticRuntime( test-kns-dis_allow_all_certificates )
target_link_libraries( test-kns-dis_allow_all_certificates ${COMMON_LIBS_READ} )
add_test( NAME Test_KNS_dis_allow_all_certificates COMMAND test-kns-dis_allow_all_certificates WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-kns-google-proxy test-google-proxy )
MSVS_StaticRuntime( test-kns-google-proxy )
target_link_libraries( test-kns-google-proxy ${COMMON_LIBS_READ} )
add_test( NAME Test_KNS_google-proxy COMMAND test-kns-google-proxy WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-kns-test200for-whole-file test200for-whole-file )
MSVS_StaticRuntime( test-kns-test200for-whole-file )
target_link_libraries( test-kns-test200for-whole-file ${COMMON_LIBS_READ} )
add_test( NAME Test_KNS_test200for-whole-file COMMAND test-kns-test200for-whole-file WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-kns knstest )
MSVS_StaticRuntime( test-kns )
target_link_libraries( test-kns ${COMMON_LIBS_READ} )
add_test( NAME Test_KNS COMMAND test-kns WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-kns-ipc test-ipc )
MSVS_StaticRuntime( test-kns-ipc )
target_link_libraries( test-kns-ipc ${COMMON_LIBS_READ} )
add_test( NAME SlowTest_KNS_ipc COMMAND test-kns-ipc WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-kns-http HttpFixture httptest )
MSVS_StaticRuntime( test-kns-http )
target_link_libraries( test-kns-http ${COMMON_LIBS_READ} )
add_test( NAME SlowTest_KNS_http COMMAND test-kns-http WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-kns-http-dropconn http_dropconnection_test )
MSVS_StaticRuntime( test-kns-http-dropconn )
target_link_libraries( test-kns-http-dropconn ${COMMON_LIBS_READ} )
add_test( NAME Test_KNS_http-dropconn COMMAND test-kns-http-dropconn WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-kns-http-request HttpFixture test-http-request )
MSVS_StaticRuntime( test-kns-http-request )
target_link_libraries( test-kns-http-request ${COMMON_LIBS_READ} )
add_test( NAME Test_KNS_http_request COMMAND test-kns-http-request WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-kns-KNSManagerSingletonTest KNSManagerSingletonTest )
MSVS_StaticRuntime( test-kns-KNSManagerSingletonTest )
target_link_libraries( test-kns-KNSManagerSingletonTest ${COMMON_LIBS_READ} )
add_test( NAME Test_KNS_KNSManagerSingletonTest COMMAND test-kns-KNSManagerSingletonTest WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-kns-connect HttpFixture test-connect )
MSVS_StaticRuntime( test-kns-connect )
target_link_libraries( test-kns-connect ${COMMON_LIBS_READ} )
add_test( NAME Test_KNS_connect COMMAND test-kns-connect WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-kns-refresh-expired HttpFixture KStableHttpFile test-refresh-expired-url )
MSVS_StaticRuntime( test-kns-refresh-expired )
target_link_libraries( test-kns-refresh-expired ${COMMON_LIBS_READ} )
add_test( NAME Test_KNS_refresh-expired COMMAND test-kns-refresh-expired WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-kns-timeouts test-timeouts )
MSVS_StaticRuntime( test-kns-timeouts )
target_link_libraries( test-kns-timeouts ${COMMON_LIBS_READ} )
add_test( NAME Test_KNS_timeouts COMMAND test-kns-timeouts WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

# TODO the following binaries are not unit tests (EXT_TOOLS), they should be started not as ctest

add_executable( test-kns-mutual-authentication test-mutual-authentication gosha )
MSVS_StaticRuntime( test-kns-mutual-authentication )
target_link_libraries( test-kns-mutual-authentication ${COMMON_LIBS_READ} )
# add_test( NAME Test_KNS_mutual-authentication COMMAND test-kns-mutual-authentication WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-kns-proxy-with-env test-proxy-with-env )
MSVS_StaticRuntime( test-kns-proxy-with-env )
target_link_libraries( test-kns-proxy-with-env ${COMMON_LIBS_READ} )
# add_test( NAME Test_KNS_proxy-with-env COMMAND test-kns-proxy-with-env WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

if( NOT WIN32 )
    add_executable( test-kns-proxy test-proxy )
    MSVS_StaticRuntime( test-kns-proxy )
    target_link_libraries( test-kns-proxy ${COMMON_LIBS_READ} )
    add_test( NAME Test_KNS_proxy COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test-proxy.sh ${CMAKE_TEST_OUTPUT_DIRECTORY}/test-kns-proxy WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
endif()

if( SINGLE_TARGET )
    add_executable( test-kns-proxy-with-scheme test-proxy-with-scheme )
    MSVS_StaticRuntime( test-kns-proxy-with-scheme )
    target_link_libraries( test-kns-proxy-with-scheme ${COMMON_LIBS_READ} )
    add_test( NAME Test_KNS_proxy-with-scheme COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test-proxy-with-scheme.sh ${CMAKE_TEST_OUTPUT_DIRECTORY}/test-kns-proxy-with-scheme WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
    #set_tests_properties(Test_KNS_proxy-with-scheme PROPERTIES ENVIRONMENT http_proxy=)
endif()
