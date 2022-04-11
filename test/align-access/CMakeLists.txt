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

add_compile_definitions( __mod__="test/align-access" )

add_executable( test-align-access test main )
MSVS_StaticRuntime( test-align-access )
target_link_libraries( test-align-access ${COMMON_LINK_LIBRARIES} ${COMMON_LIBS_READ} )
add_test( NAME Test_Align_access COMMAND test-align-access WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

if( RUN_SANITIZER_TESTS )
	set( asan_defs "-fsanitize=address" )
	add_executable( test-align-access-asan test main )
	target_link_libraries( test-align-access-asan ${COMMON_LINK_LIBRARIES} ${COMMON_LIBS_READ} )
	add_test( NAME Test_Align_access_asan COMMAND test-align-access-asan WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
	target_compile_options( test-align-access-asan PRIVATE ${asan_defs} )
	target_link_options( test-align-access-asan PRIVATE ${asan_defs} )

	set( tsan_defs "-fsanitize=thread" )
	add_executable( test-align-access-tsan test main )
	target_link_libraries( test-align-access-tsan ${COMMON_LINK_LIBRARIES} ${COMMON_LIBS_READ} )
	add_test( NAME Test_Align_access_tsan COMMAND test-align-access-tsan WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
	target_compile_options( test-align-access-tsan PRIVATE ${tsan_defs} )
	target_link_options( test-align-access-tsan PRIVATE ${tsan_defs} )
endif()
