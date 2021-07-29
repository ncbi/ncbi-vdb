add_compile_definitions( __mod__="test/loader" )

if( WIN32 )
    set( ADDITIONAL_LIBS Crypt32 )
else()
    set( ADDITIONAL_LIBS "" )
endif()

add_executable( test-loader loadertest sequence-writer )
MSVS_StaticRuntime( test-loader )
target_link_libraries( test-loader  ${COMMON_LIBS_WRITE} load ${ADDITIONAL_LIBS} )
add_test( NAME Test_LOADER COMMAND test-loader WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )