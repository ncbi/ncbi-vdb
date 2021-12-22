# TODO: fuzz test

add_compile_definitions( __mod__="test/kfg" )

#AddExecutableTest( Test_KFG_NCBI_Env "ncbi-home-from-env" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KFG_Wb_test_kfg "wb-test-kfg" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KFG_test_kfg "kfg-fixture;kfgtest" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KFG_repository "repositorytest" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KFG_Keystore "keystoretest" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KFG_Properties "kfg-fixture;propertiestest" "${COMMON_LIBS_READ}" )
