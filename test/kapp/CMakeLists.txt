if( WIN32 )
    set( ADDITIONAL_LIBS Crypt32 )
else()
    set( ADDITIONAL_LIBS "" )
endif()

add_executable( test-qfile qfiletest )
add_executable( test-kapp kapp-test )

MSVS_StaticRuntime( test-qfile )
MSVS_StaticRuntime( test-kapp )

# TODO: ALL_TOOLS, scripted tests

add_compile_definitions( __mod__="test/kapp" )

target_link_libraries( test-qfile ${COMMON_LIBS_READ} load kq )
target_link_libraries( test-kapp load ${COMMON_LIBS_WRITE} kq ${ADDITIONAL_LIBS} )

#add_test( NAME Test_KFG_NCBI_Env COMMAND ncbi-home-from-env )
add_test( NAME Test_KAPP_qfile COMMAND test-qfile )
add_test( NAME Test_KAPP_args COMMAND test-kapp WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
