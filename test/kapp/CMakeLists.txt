add_executable( test-qfile qfiletest )
add_executable( test-kapp kapp-test )

# TODO: ALL_TOOLS, scripted tests

add_definitions (-D__mod__="test/kapp")

# TODO: fails to link with kapp
target_link_libraries( test-qfile ${COMMON_LIBS_READ} kfs load kq )
target_link_libraries( test-kapp ${COMMON_LIBS_WRITE} kfs load kq )

#add_test( NAME Test_KFG_NCBI_Env COMMAND ncbi-home-from-env )
add_test( NAME Test_KAPP_qfile COMMAND test-qfile )
add_test( NAME Test_KAPP_args COMMAND test-kapp WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
