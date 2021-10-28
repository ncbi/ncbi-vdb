add_compile_definitions( __mod__="test/kfs" )

if( NOT WIN32 )
    AddExecutableTest( Test_KFS_kdf "kdf" "${COMMON_LIBS_READ}" )
endif()

AddExecutableTest( Test_KFS "kfstest" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KFS_ramfile "ramfiletest" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KFS_resolve "resolvetest" "${COMMON_LIBS_READ}" )

if( NOT WIN32 )
    AddExecutableTest( Test_KFS_link "test-link" "${COMMON_LIBS_READ}" )
endif()

AddExecutableTest( Test_KFS_cachetee "cacheteetest" "${COMMON_LIBS_READ}" )

if( NOT WIN32 ) # TODO: these fail on Windows
    AddExecutableTest( Test_KFS_cachetee2 "cachetee2test" "${COMMON_LIBS_READ}" )
    AddExecutableTest( SlowTest_KFS_cachetee3 "cachetee3test" "${COMMON_LIBS_READ}" )
endif()

AddExecutableTest( Test_KFS_lru_cache "lru_cache_test" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KFS_md5 "md5test" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KFS_appendfile "appendfiletest" "${COMMON_LIBS_READ}" )
