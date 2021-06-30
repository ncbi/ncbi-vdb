add_definitions (-D__mod__="test/klib")

add_executable( test-klib-asm asm-test )
add_test( NAME Test_KLIB_asm COMMAND test-klib-asm )

add_executable( test-klib-printf printf-test )
add_test( NAME Test_KLIB_printf COMMAND test-klib-printf )

add_executable( test-klib-data-buffer-print test-data-buffer-print )
add_test( NAME Test_KLIB_data_buffer_print COMMAND test-klib-data-buffer-print )

add_executable( test-klib-log test-log )
add_test( NAME Test_KLIB_log COMMAND test-klib-log )

add_executable( test-klib-out test-out )
add_test( NAME Test_KLIB_out COMMAND test-klib-out )

add_executable( test-klib-SraReleaseVersion test-SraReleaseVersion )
add_test( NAME Test_KLIB_SraReleaseVersion COMMAND test-klib-SraReleaseVersion )

add_executable( test-klib-time test-time )
add_test( NAME Test_KLIB_time COMMAND test-klib-time )

add_executable( test-klib-rcenum test-rcenum )
add_test( NAME Test_KLIB_rcenum COMMAND test-klib-rcenum )

add_executable( test-klib test-klib )
add_test( NAME Test_KLIB COMMAND test-klib )

add_executable( test-klib-KOutMsgOverflow test-KOutMsgOverflow )
add_test( NAME Test_KLIB_KOutMsgOverflow COMMAND test-klib-KOutMsgOverflow )

add_executable( test-klib-vnamelist test-vnamelist )
add_test( NAME Test_KLIB_vnamelist COMMAND test-klib-vnamelist )

add_executable( test-klib-progressbar test-progress )
add_test( NAME Test_KLIB_progressbar COMMAND test-klib-progressbar )

add_executable( test-klib-symtab test-symtab )
add_test( NAME Test_KLIB_symtab COMMAND test-klib-symtab )

add_executable( test-klib-json test-json )
add_test( NAME Test_KLIB_json COMMAND test-klib-json )

# test-json \
# test-base64 \
# test-guid \
# test-report \
