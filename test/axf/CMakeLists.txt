add_compile_definitions( __mod__="test/axf" )

if( NOT WIN32 )
    set( ADDITIONAL_LIBS "m" )
else()
    set( ADDITIONAL_LIBS "" )
endif()

AddExecutableTest( Test_Axf_RestoreRead "test-RestoreRead" "${COMMON_LINK_LIBRARIES};${COMMON_LIBS_READ};${ADDITIONAL_LIBS}" )