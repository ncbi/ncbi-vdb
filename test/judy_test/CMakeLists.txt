add_executable(test-judy
    judytest
)

add_definitions (-D__mod__="test/judy_test")

add_test( NAME Test_JUDY COMMAND test-judy )