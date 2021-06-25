add_executable(test-except
    kfc.c
    test-except.c
)

add_definitions (-D__mod__="test/kfc")
target_link_libraries( test-except klib )

add_test( NAME Test_KFC_Except COMMAND test-except )