# ===========================================================================
#
#                            PUBLIC DOMAIN NOTICE
#               National Center for Biotechnology Information
#
#  This software/database is a "United States Government Work" under the
#  terms of the United States Copyright Act.  It was written as part of
#  the author's official duties as a United States Government employee and
#  thus cannot be copyrighted.  This software/database is freely available
#  to the public for use. The National Library of Medicine and the U.S.
#  Government have not placed any restriction on its use or reproduction.
#
#  Although all reasonable efforts have been taken to ensure the accuracy
#  and reliability of the software and data, the NLM and the U.S.
#  Government do not and cannot warrant the performance or results that
#  may be obtained by using this software or data. The NLM and the U.S.
#  Government disclaim all warranties, express or implied, including
#  warranties of performance, merchantability or fitness for any particular
#  purpose.
#
#  Please cite the author in any work or product based on this material.
#
# ===========================================================================

add_compile_definitions( __mod__="test/kns" )

set( LIBS kapp "${COMMON_LIBS_READ}")

AddExecutableTest( Test_KNS_allow_all_certificates "test_allow_all_certificates" "${LIBS}" )
AddExecutableTest( Test_KNS_dis_allow_all_certificates "test_dis_allow_all_certificates" "${LIBS}" )
AddExecutableTest( Test_KNS_google-proxy "test-google-proxy" "${LIBS}" )
AddExecutableTest( Test_KNS_test200for-whole-file "test200for-whole-file" "${LIBS}" )
AddExecutableTest( Test_KNS_dflt "knstest" "${COMMON_LIBS_READ}" )
AddExecutableTest( SlowTest_KNS_ipc "test-ipc" "${LIBS}" )
AddExecutableTest( Test_KNS_opt-bitmap "test-opt-bitmap" "${LIBS}" )
AddExecutableTest( SlowTest_KNS_http "HttpFixture;httptest" "${LIBS}" )
AddExecutableTest( Test_KNS_http-dropconn "http_dropconnection_test" "${LIBS}" )
AddExecutableTest( Test_KNS_http_request "HttpFixture;test-http-request" "${LIBS}" )
AddExecutableTest( Test_KNS_KNSManagerSingletonTest "KNSManagerSingletonTest" "${LIBS}" )
AddExecutableTest( Test_KNS_connect "HttpFixture;test-connect" "${LIBS}" )
AddExecutableTest( Test_KNS_refresh-expired "HttpFixture;KStableHttpFile;test-refresh-expired-url" "${LIBS}" )
AddExecutableTest( Test_KNS_timeouts "test-timeouts" "${LIBS}" )

# TODO the following binaries are not unit tests (EXT_TOOLS), they should be started not as ctest

BuildExecutableForTest( test-kns-mutual-authentication "test-mutual-authentication;gosha" "${LIBS}" )
# add_test( NAME Test_KNS_mutual-authentication COMMAND test-kns-mutual-authentication WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

BuildExecutableForTest( test-kns-proxy-with-env "test-proxy-with-env" "${LIBS}" )
# add_test( NAME Test_KNS_proxy-with-env COMMAND test-kns-proxy-with-env WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

if( SINGLE_CONFIG )

    BuildExecutableForTest( Test_KNS_proxy "test-proxy" "${LIBS}" )
    add_test( NAME Test_KNS_proxy COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test-proxy.sh ${CMAKE_TEST_OUTPUT_DIRECTORY}/Test_KNS_proxy WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

    BuildExecutableForTest( Test_KNS_proxy-with-scheme_dflt "test-proxy-with-scheme" "${LIBS}" )
    add_test( NAME Test_KNS_proxy-with-scheme_dflt COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test-proxy-with-scheme.sh ${CMAKE_TEST_OUTPUT_DIRECTORY}/Test_KNS_proxy-with-scheme_dflt WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
    #set_tests_properties(Test_KNS_proxy-with-scheme PROPERTIES ENVIRONMENT http_proxy=)

endif()

AddExecutableTest( Test_KNS_proxy-with-scheme_failure "test-proxy-with-scheme_failure" "${LIBS}" )
AddExecutableTest( Test_KNS_aws-proxy "test-aws-proxy" "${LIBS}" )
