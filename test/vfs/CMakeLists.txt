add_definitions (-D__mod__="test/vfs")

add_executable( test-attached-vdbcache test-attached-vdbcache )
target_link_libraries( test-attached-vdbcache ${COMMON_LIBS_READ} )
add_test( NAME Test_VFS_attached-vdbcache COMMAND test-attached-vdbcache WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-log-names-error test-log-names-error )
target_link_libraries( test-log-names-error ${COMMON_LIBS_READ} )
add_test( NAME Test_VFS_log-names-error COMMAND test-log-names-error WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-redirect-rejected-names-cgi-http-to-https redirect-rejected-names-cgi-http-to-https )
target_link_libraries( test-redirect-rejected-names-cgi-http-to-https ${COMMON_LIBS_READ} )
add_test( NAME Test_VFS_redirect-rejected-names-cgi-http-to-https COMMAND test-redirect-rejected-names-cgi-http-to-https WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-resolver-with-log test-resolver-with-log )
target_link_libraries( test-resolver-with-log ${COMMON_LIBS_READ} )
add_test( NAME Test_VFS_resolver-with-log COMMAND test-resolver-with-log WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-sdl-response test-sdl-response )
target_link_libraries( test-sdl-response ${COMMON_LIBS_READ} )
add_test( NAME Test_VFS_sdl-response COMMAND test-sdl-response WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-services test-services )
target_link_libraries( test-services ${COMMON_LIBS_READ} )
add_test( NAME Test_VFS_services COMMAND test-services WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-SraDesc test-SraDesc )
target_link_libraries( test-SraDesc ${COMMON_LIBS_READ} )
add_test( NAME Test_VFS_SraDesc COMMAND test-SraDesc WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-caching test-caching )
target_link_libraries( test-caching ${COMMON_LIBS_READ} )
add_test( NAME Test_VFS_caching COMMAND test-caching WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-names-30 test-names-30 )
target_link_libraries( test-names-30 ${COMMON_LIBS_READ} )
add_test( NAME Test_VFS_names-30 COMMAND test-names-30 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-path pathtest )
target_link_libraries( test-path ${COMMON_LIBS_READ} )
add_test( NAME Test_VFS_path COMMAND test-path WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-path-c path-test )
target_link_libraries( test-path-c ${COMMON_LIBS_READ} )
add_test( NAME Test_VFS_path-c COMMAND test-path-c WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-perm test-perm )
target_link_libraries( test-perm ${COMMON_LIBS_READ} )
add_test( NAME Test_VFS_perm COMMAND test-perm WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-resolve test-resolve )
target_link_libraries( test-resolve ${COMMON_LIBS_READ} )
add_test( NAME Test_VFS_resolve COMMAND test-resolve WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-resolver resolvertest )
target_link_libraries( test-resolver ${COMMON_LIBS_READ} )
add_test( NAME Test_VFS_resolver COMMAND test-resolver WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-vfsmanager managertest )
target_link_libraries( test-vfsmanager ${COMMON_LIBS_READ} )
add_test( NAME Test_VFS_vfsmanager COMMAND test-vfsmanager WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-json-response test-json-response )
target_link_libraries( test-json-response ${COMMON_LIBS_READ} )
add_test( NAME Test_VFS_json-response COMMAND test-json-response WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-resolver4 test-resolver4 )
target_link_libraries( test-resolver4 ${COMMON_LIBS_READ} )
add_test( NAME Test_VFS_resolver4 COMMAND test-resolver4 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( test-resolve-qual test-resolve-qual )
target_link_libraries( test-resolve-qual ${COMMON_LIBS_READ} )
add_test( NAME Test_VFS_resolve-qual COMMAND test-resolve-qual WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
