if( WIN32 )
    set( ADDITIONAL_LIBS Crypt32 )
else()
    set( ADDITIONAL_LIBS "" )
endif()

add_compile_definitions( __mod__="test/kapp" )

AddExecutableTest( Test_KAPP_qfile "qfiletest" "${COMMON_LIBS_READ};load;kq" )
AddExecutableTest( Test_KAPP_args "kapp-test" "load;${COMMON_LIBS_WRITE};kq;${ADDITIONAL_LIBS}" )
