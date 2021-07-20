#add_executable( ncbi-home-from-env ncbi-home-from-env )
add_executable( wb-test-kfg wb-test-kfg )
add_executable( test-kfg kfg-fixture kfgtest )
add_executable( test-repository repositorytest )
add_executable( test-keystore keystoretest )
add_executable( test-properties kfg-fixture propertiestest )

# TODO: fuzz test

add_compile_definitions( __mod__="test/kfg" )

#target_link_libraries( ncbi-home-from-env ${COMMON_LIBS_READ} )
target_link_libraries( wb-test-kfg ${COMMON_LIBS_READ} )
target_link_libraries( test-kfg ${COMMON_LIBS_READ} )
target_link_libraries( test-repository ${COMMON_LIBS_READ} )
target_link_libraries( test-keystore ${COMMON_LIBS_READ} )
target_link_libraries( test-properties ${COMMON_LIBS_READ} )

#add_test( NAME Test_KFG_NCBI_Env COMMAND ncbi-home-from-env )
add_test( NAME Test_KFG_Wb_test_kfg COMMAND wb-test-kfg )
add_test( NAME Test_KFG_test_kfg COMMAND test-kfg WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
add_test( NAME Test_KFG_repository COMMAND test-repository )
add_test( NAME Test_KFG_Keystore COMMAND test-keystore )
add_test( NAME Test_KFG_Properties COMMAND test-properties )
