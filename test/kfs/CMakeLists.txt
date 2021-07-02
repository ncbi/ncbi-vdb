add_definitions (-D__mod__="test/kfs")

add_executable( test-kfs-kdf kdf )
target_link_libraries( test-kfs-kdf  ${COMMON_LIBS_READ} )
add_test( NAME Test_KFS_kdf COMMAND test-kfs-kdf )

add_executable( test-kfs kfstest )
target_link_libraries( test-kfs  ${COMMON_LIBS_READ} )
add_test( NAME Test_KFS COMMAND test-kfs WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable( test-kfs-ramfile ramfiletest )
target_link_libraries( test-kfs-ramfile  ${COMMON_LIBS_READ} )
add_test( NAME Test_KFS_ramfile COMMAND test-kfs-ramfile )

add_executable( test-kfs-resolve resolvetest )
target_link_libraries( test-kfs-resolve  ${COMMON_LIBS_READ} )
add_test( NAME Test_KFS_resolve COMMAND test-kfs-resolve )

add_executable( test-kfs-link test-link )
target_link_libraries( test-kfs-link  ${COMMON_LIBS_READ} )
add_test( NAME Test_KFS_link COMMAND test-kfs-link )

add_executable( test-kfs-cachetee cacheteetest )
target_link_libraries( test-kfs-cachetee  ${COMMON_LIBS_READ} )
add_test( NAME Test_KFS_cachetee COMMAND test-kfs-cachetee )

add_executable( test-kfs-cachetee2 cachetee2test )
target_link_libraries( test-kfs-cachetee2  ${COMMON_LIBS_READ} )
add_test( NAME Test_KFS_cachetee2 COMMAND test-kfs-cachetee2 )

add_executable( test-kfs-cachetee3 cachetee3test )
target_link_libraries( test-kfs-cachetee3  ${COMMON_LIBS_READ} )
add_test( NAME Test_KFS_cachetee3 COMMAND  test-kfs-cachetee3 )

add_executable( test-kfs-lru-cache lru_cache_test )
target_link_libraries( test-kfs-lru-cache  ${COMMON_LIBS_READ} )
add_test( NAME Test_KFS_lru_cache COMMAND  test-kfs-lru-cache )

add_executable( test-kfs-md5 md5test )
target_link_libraries( test-kfs-md5  ${COMMON_LIBS_READ} )
add_test( NAME Test_KFS_md5 COMMAND  test-kfs-md5 )

add_executable( test-kfs-appendfile appendfiletest )
target_link_libraries( test-kfs-appendfile  ${COMMON_LIBS_READ} )
add_test( NAME Test_KFS_appendfile COMMAND  test-kfs-appendfile )
