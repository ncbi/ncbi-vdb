add_compile_definitions( __mod__="test/kfs" )

if( NOT WIN32 )
    add_executable( test-kfs-kdf kdf )
    MSVS_StaticRuntime( test-kfs-kdf )
    target_link_libraries( test-kfs-kdf  ${COMMON_LIBS_READ} )
    add_test( NAME Test_KFS_kdf COMMAND test-kfs-kdf )
endif()

add_executable( test-kfs kfstest )
MSVS_StaticRuntime( test-kfs )
target_link_libraries( test-kfs  ${COMMON_LIBS_READ} )
add_test( NAME Test_KFS COMMAND test-kfs WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable( test-kfs-ramfile ramfiletest )
MSVS_StaticRuntime( test-kfs-ramfile )
target_link_libraries( test-kfs-ramfile  ${COMMON_LIBS_READ} )
add_test( NAME Test_KFS_ramfile COMMAND test-kfs-ramfile )

add_executable( test-kfs-resolve resolvetest )
MSVS_StaticRuntime( test-kfs-resolve )
target_link_libraries( test-kfs-resolve  ${COMMON_LIBS_READ} )
add_test( NAME Test_KFS_resolve COMMAND test-kfs-resolve )

add_executable( test-kfs-link test-link )
MSVS_StaticRuntime( test-kfs-link )
target_link_libraries( test-kfs-link  ${COMMON_LIBS_READ} )
add_test( NAME Test_KFS_link COMMAND test-kfs-link )

add_executable( test-kfs-cachetee cacheteetest )
MSVS_StaticRuntime( test-kfs-cachetee )
target_link_libraries( test-kfs-cachetee  ${COMMON_LIBS_READ} )
add_test( NAME Test_KFS_cachetee COMMAND test-kfs-cachetee )

if( NOT WIN32 ) # TODO: these fail on Windows
    add_executable( test-kfs-cachetee2 cachetee2test )
    MSVS_StaticRuntime( test-kfs-cachetee2 )
    target_link_libraries( test-kfs-cachetee2  ${COMMON_LIBS_READ} )
    add_test( NAME Test_KFS_cachetee2 COMMAND test-kfs-cachetee2 )

    add_executable( test-kfs-cachetee3 cachetee3test )
    MSVS_StaticRuntime( test-kfs-cachetee3 )
    target_link_libraries( test-kfs-cachetee3  ${COMMON_LIBS_READ} )
    add_test( NAME SlowTest_KFS_cachetee3 COMMAND  test-kfs-cachetee3 )
endif()

add_executable( test-kfs-lru-cache lru_cache_test )
MSVS_StaticRuntime( test-kfs-lru-cache )
target_link_libraries( test-kfs-lru-cache  ${COMMON_LIBS_READ} )
add_test( NAME Test_KFS_lru_cache COMMAND  test-kfs-lru-cache )

add_executable( test-kfs-md5 md5test )
MSVS_StaticRuntime( test-kfs-md5 )
target_link_libraries( test-kfs-md5  ${COMMON_LIBS_READ} )
add_test( NAME Test_KFS_md5 COMMAND  test-kfs-md5 )

add_executable( test-kfs-appendfile appendfiletest )
MSVS_StaticRuntime( test-kfs-appendfile )
target_link_libraries( test-kfs-appendfile  ${COMMON_LIBS_READ} )
add_test( NAME Test_KFS_appendfile COMMAND  test-kfs-appendfile )
