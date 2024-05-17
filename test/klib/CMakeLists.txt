# ===========================================================================
#
#                            PUBLIC DOMAIN NOTICE
#               National Center for Biotechnology Information
#
#  This software/database is a "United States Government Work" under the
#  terms of the United States Copyright Act.  It was written as part of
#  the author's official duties as a United States Government employee and
#  thus cannot be copyrighted.  This software/database is freely available
#  to the public for use. The National Library of Medicine and the U.S.
#  Government have not placed any restriction on its use or reproduction.
#
#  Although all reasonable efforts have been taken to ensure the accuracy
#  and reliability of the software and data, the NLM and the U.S.
#  Government do not and cannot warrant the performance or results that
#  may be obtained by using this software or data. The NLM and the U.S.
#  Government disclaim all warranties, express or implied, including
#  warranties of performance, merchantability or fitness for any particular
#  purpose.
#
#  Please cite the author in any work or product based on this material.
#
# ===========================================================================

add_compile_definitions( __mod__="test/klib" )

AddExecutableTest( Test_KLIB_asm "asm-test" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KLIB_printf "printf-test" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KLIB_data_buffer_print "test-data-buffer-print" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KLIB_log "test-log" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KLIB_out "test-out" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KLIB_SraReleaseVersion "test-SraReleaseVersion" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KLIB_time "test-time" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KLIB_rcenum "test-rcenum" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_myKLIB "test-klib" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KLIB_KOutMsgOverflow "test-KOutMsgOverflow" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KLIB_vnamelist "test-vnamelist" "${COMMON_LIBS_READ}" )
AddExecutableTest( SlowTest_KLIB_progressbar "test-progress" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KLIB_symtab "test-symtab" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KLIB_json "test-json" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KLIB_base64 "test-base64" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KLIB_guid "test-guid" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KLIB_report "test-report" "${COMMON_LIBS_READ}" )
AddExecutableTest( SlowTest_KLIB_hashfile "test-hashfile" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KLIB_hashtable "test-hashtable" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KLIB_pack "test-pack" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KLIB_btree "test-btree" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KLIB_sha-32bit "test-sha-32bit" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_MemTrack "test-mem-track" "${COMMON_LIBS_READ}" )
