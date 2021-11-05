if( WIN32 )
    set( ADDITIONAL_LIBS Crypt32 )
else()
    set( ADDITIONAL_LIBS "" )
endif()

add_compile_definitions( __mod__="test/vxf" )

AddExecutableTest( Test_VXF_wb_vxf "wb-test-vxf;wb-irzip-impl" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_VXF_VDB-2915-test "VDB-2915" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS}" )
