add_executable(test-kfc-except
    kfc.c
    test-except.c
)

add_compile_definitions( __mod__="test/kfc" )
target_link_libraries( test-kfc-except klib )

add_test( NAME Test_KFC_Except COMMAND test-kfc-except )