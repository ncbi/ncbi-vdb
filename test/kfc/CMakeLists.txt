add_executable(test-kfc-except
    kfc.c
    test-except.c
)

add_definitions (-D__mod__="test/kfc")
target_link_libraries( test-kfc-except klib )

add_test( NAME Test_KFC_Except COMMAND test-kfc-except )