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

add_compile_definitions( __mod__="test/vfs" )

set( LIBS kapp "${COMMON_LIBS_READ}")

AddExecutableTest( Test_VFS_attached-vdbcache "test-attached-vdbcache" "${LIBS}" )
AddExecutableTest( Test_VFS_log-names-error "test-log-names-error" "${LIBS}" )
AddExecutableTest( Test_VFS_redirect-rejected-names-cgi-http-to-https "redirect-rejected-names-cgi-http-to-https.cpp" "${LIBS}" )
AddExecutableTest( Test_VFS_resolver-with-log "test-resolver-with-log" "${LIBS}" )
AddExecutableTest( Test_VFS_sdl-response "test-sdl-response" "${LIBS}" )
AddExecutableTest( Test_VFS_services "test-services" "${LIBS}" )
AddExecutableTest( Test_VFS_SraDesc "test-SraDesc" "${LIBS}" )
AddExecutableTest( Test_VFS_cache_sdl "test_VFS_cache_sdl" "${LIBS}" )
AddExecutableTest( Test_VFS_caching "test-caching" "${LIBS}" )
# AddExecutableTest( Test_VFS_names-30 "test-names-30" "${LIBS}" )
AddExecutableTest(Test_VFS_no-nm-cgi "Test_VFS_no-nm-cgi" "${LIBS}")
AddExecutableTest(
    Test_VFS_vfs_resolve "test-vfs-resolve" "${LIBS}")
AddExecutableTest( Test_VFS_path "pathtest" "${LIBS}" )
AddExecutableTest( Test_VFS_path-c "path-test" "${LIBS}" )
AddExecutableTest( Test_VFS_perm "test-perm" "${LIBS}" )
AddExecutableTest( Test_VFS_resolve "test-resolve" "${LIBS}" )
AddExecutableTest( Test_VFS_resolver_dflt "Test_VFS_resolver_dflt" "${LIBS}" )

AddExecutableTest(
 Test_VFS_VPATH_open-read-read   test-vpath-open-read    "${LIBS}" )
AddExecutableTest(
 Test_VFS_VPATH_open-read-update test-vpath-open-read-update "${LIBS}"
)
AddExecutableTest(
 Test_VFS_VPATH_open-update-read test-vpath-open-update-read "${COMMON_LIBS_WRITE}"
)
AddExecutableTest(
 Test_VFS_VPATH_open-update-update test-vpath-open-update-update "${COMMON_LIBS_WRITE}"
)

AddExecutableTest( Test_VFS_vfsmanager "managertest" "${LIBS}" )
AddExecutableTest( Test_VFS_json-response "test-json-response" "${LIBS}" )
# AddExecutableTest( Test_VFS_resolver4 "test-resolver4" "${LIBS}" )
AddExecutableTest( Test_VFS_resolve-qual "test-resolve-qual" "${LIBS}" )
