add_executable( test-kproc kproctest )

add_definitions (-D__mod__="test/kproc")

target_link_libraries( test-kproc ${COMMON_LIBS_READ} )

add_test( NAME Test_KPROC_kproctest COMMAND test-kproc WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
