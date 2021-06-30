add_executable(test-ktst
    ktsttest
)

add_definitions (-D__mod__="test/ktst")

add_test( NAME Test_KTST COMMAND test-ktst )