add_definitions (-D__mod__="test/kfs_test")

add_executable( kdf kdf )
add_test( NAME Test_KFS_kdf COMMAND kdf )

add_executable( test-kfs kfstest )
add_test( NAME Test_KFS COMMAND test-kfs WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable( test-ramfile ramfiletest )
add_test( NAME Test_KFS_ramfile COMMAND test-ramfile )

add_executable( test-resolve resolvetest )
add_test( NAME Test_KFS_resolve COMMAND test-resolve )

add_executable( test-link test-link )
add_test( NAME Test_KFS_link COMMAND test-link )

add_executable( test-cachetee cacheteetest )
add_test( NAME Test_KFS_cachetee COMMAND test-cachetee )

add_executable( test-cachetee2 cachetee2test )
add_test( NAME Test_KFS_cachetee2 COMMAND test-cachetee2 )

add_executable( test-cachetee3 cachetee3test )
add_test( NAME Test_KFS_cachetee3 COMMAND  test-cachetee3 )

add_executable( test-lru-cache lru_cache_test )
add_test( NAME Test_KFS_lru_cache COMMAND  test-lru-cache )

add_executable( test-kfs-md5 md5test )
add_test( NAME Test_KFS_md5 COMMAND  test-kfs-md5 )

add_executable( test-appendfile appendfiletest )
add_test( NAME Test_KFS_appendfile COMMAND  test-appendfile )
