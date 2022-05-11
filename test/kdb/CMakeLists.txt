if( WIN32 )
    set( ADDITIONAL_LIBS Crypt32 )
else()
    set( ADDITIONAL_LIBS "" )
endif()

# TODO: make sure runs are not cached in the user repository when running tests

add_compile_definitions( __mod__="test/kdb" )

# AddExecutableTest( Test_KDB_Btree "test-btree" "${COMMON_LIBS_READ}" ) #TODO: revive test-btree
AddExecutableTest( Test_KDB_Rowset "rowsettest" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS}" )
AddExecutableTest( Test_KDB_Kdb "kdbtest" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KDB_Wkdb "wkdbtest" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS}" )
AddExecutableTest( Test_KDB_CopyColumn "VDB-4705" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS}" )
#AddExecutableTest( Test_KDB_CopyMeta "VDB-4706" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS}" )
