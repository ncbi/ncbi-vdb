add_executable(test-judy
    judytest
)

add_definitions (-D__mod__="test/judy")

target_link_libraries( test-judy  ${COMMON_LIBS_READ} )

add_test( NAME Test_JUDY COMMAND test-judy )