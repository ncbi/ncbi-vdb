add_compile_definitions( __mod__="test/loader" )

if( WIN32 )
    set( ADDITIONAL_LIBS Crypt32 )
else()
    set( ADDITIONAL_LIBS "" )
endif()

AddExecutableTest( Test_LOADER "loadertest;sequence-writer" "${COMMON_LIBS_WRITE};load;${ADDITIONAL_LIBS}" )