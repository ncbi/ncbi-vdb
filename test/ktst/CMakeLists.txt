add_executable(test-ktst
    ktsttest
)

target_link_libraries( test-ktst ktst kapp ncbi-vdb dl pthread )
add_definitions (-D__mod__="test/ktst")

add_test( NAME Test_KTST COMMAND test-ktst )