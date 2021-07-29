if( WIN32 )
    set( ADDITIONAL_LIBS Crypt32 )
else()
    set( ADDITIONAL_LIBS "" )
endif()

add_compile_definitions( __mod__="test/vxf" )

add_executable( wb-test-vxf wb-test-vxf wb-irzip-impl )
MSVS_StaticRuntime( wb-test-vxf )
target_link_libraries( wb-test-vxf  ${COMMON_LIBS_READ} )
add_test( NAME Test_VXF_wb_vxf COMMAND wb-test-vxf WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( vdb-2915-test VDB-2915 )
MSVS_StaticRuntime( vdb-2915-test )
target_link_libraries( vdb-2915-test  ${COMMON_LIBS_WRITE} ${ADDITIONAL_LIBS} )
add_test( NAME Test_VXF_VDB-2915-test COMMAND vdb-2915-test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
