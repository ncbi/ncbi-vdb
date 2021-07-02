add_definitions (-D__mod__="test/klib")

add_executable( test-klib-asm asm-test )
target_link_libraries( test-klib-asm  ${COMMON_LIBS_READ} )
add_test( NAME Test_KLIB_asm COMMAND test-klib-asm )

add_executable( test-klib-printf printf-test )
target_link_libraries( test-klib-printf  ${COMMON_LIBS_READ} )
add_test( NAME Test_KLIB_printf COMMAND test-klib-printf )

add_executable( test-klib-data-buffer-print test-data-buffer-print )
target_link_libraries( test-klib-data-buffer-print  ${COMMON_LIBS_READ} )
add_test( NAME Test_KLIB_data_buffer_print COMMAND test-klib-data-buffer-print )

add_executable( test-klib-log test-log )
target_link_libraries( test-klib-log  ${COMMON_LIBS_READ} )
add_test( NAME Test_KLIB_log COMMAND test-klib-log )

add_executable( test-klib-out test-out )
target_link_libraries( test-klib-out  ${COMMON_LIBS_READ} )
add_test( NAME Test_KLIB_out COMMAND test-klib-out )

add_executable( test-klib-SraReleaseVersion test-SraReleaseVersion )
target_link_libraries( test-klib-SraReleaseVersion  ${COMMON_LIBS_READ} )
add_test( NAME Test_KLIB_SraReleaseVersion COMMAND test-klib-SraReleaseVersion )

add_executable( test-klib-time test-time )
target_link_libraries( test-klib-time  ${COMMON_LIBS_READ} )
add_test( NAME Test_KLIB_time COMMAND test-klib-time )

add_executable( test-klib-rcenum test-rcenum )
target_link_libraries( test-klib-rcenum  ${COMMON_LIBS_READ} )
add_test( NAME Test_KLIB_rcenum COMMAND test-klib-rcenum )

add_executable( test-klib test-klib )
target_link_libraries( test-klib  ${COMMON_LIBS_READ} )
add_test( NAME Test_KLIB COMMAND test-klib )

add_executable( test-klib-KOutMsgOverflow test-KOutMsgOverflow )
target_link_libraries( test-klib-KOutMsgOverflow  ${COMMON_LIBS_READ} )
add_test( NAME Test_KLIB_KOutMsgOverflow COMMAND test-klib-KOutMsgOverflow )

add_executable( test-klib-vnamelist test-vnamelist )
target_link_libraries( test-klib-vnamelist  ${COMMON_LIBS_READ} )
add_test( NAME Test_KLIB_vnamelist COMMAND test-klib-vnamelist )

add_executable( test-klib-progressbar test-progress )
target_link_libraries( test-klib-progressbar  ${COMMON_LIBS_READ} )
add_test( NAME SlowTest_KLIB_progressbar COMMAND test-klib-progressbar )

add_executable( test-klib-symtab test-symtab )
target_link_libraries( test-klib-symtab  ${COMMON_LIBS_READ} )
add_test( NAME Test_KLIB_symtab COMMAND test-klib-symtab )

add_executable( test-klib-json test-json )
target_link_libraries( test-klib-json  ${COMMON_LIBS_READ} )
add_test( NAME Test_KLIB_json COMMAND test-klib-json )

add_executable( test-klib-base64 test-base64 )
target_link_libraries( test-klib-base64  ${COMMON_LIBS_READ} )
add_test( NAME Test_KLIB_base64 COMMAND test-klib-base64 )

add_executable( test-klib-guid test-guid )
target_link_libraries( test-klib-guid  ${COMMON_LIBS_READ} )
add_test( NAME Test_KLIB_guid COMMAND test-klib-guid )

add_executable( test-klib-report test-report )
target_link_libraries( test-klib-report  ${COMMON_LIBS_READ} )
add_test( NAME Test_KLIB_report COMMAND test-klib-report )

add_executable( test-klib-hashfile test-hashfile )
target_link_libraries( test-klib-hashfile  ${COMMON_LIBS_READ} )
add_test( NAME SlowTest_KLIB_hashfile COMMAND test-klib-hashfile )

 #TODO: fix assert in Klib_HashTableMapDeletes (currently commented out in hashtable.c)
add_executable( test-klib-hashtable test-hashtable )
target_link_libraries( test-klib-hashtable  ${COMMON_LIBS_READ} )
add_test( NAME Test_KLIB_hashtable COMMAND test-klib-hashtable )
