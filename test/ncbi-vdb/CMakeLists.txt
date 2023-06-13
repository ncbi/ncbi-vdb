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

if( WIN32 )
    add_executable( test-link-md test-link )
    MSVS_DLLRuntime( test-link-md )
    target_link_libraries( test-link-md ktst-md ncbi-vdb-shared-md kapp-md Ws2_32 ${MBEDTLS_LIBS} )

    add_executable( test-link-mt test-link )
    MSVS_StaticRuntime( test-link-mt )
    target_link_libraries( test-link-mt ktst kapp ncbi-vdb-shared Ws2_32 ${MBEDTLS_LIBS} )

    add_executable( test-wlink-md test-wlink )
    MSVS_DLLRuntime( test-wlink-md )
    target_link_libraries( test-wlink-md ktst-md ncbi-wvdb-shared-md kapp-md Ws2_32 ${MBEDTLS_LIBS} )

    add_executable( test-wlink-mt test-wlink )
    MSVS_StaticRuntime( test-wlink-mt )
    target_link_libraries( test-wlink-mt ktst kapp ncbi-wvdb-shared Ws2_32 ${MBEDTLS_LIBS} )
elseif( NOT HAVE_MBEDTLS_F )
    if ( CMAKE_CXX_COMPILER_ID MATCHES "^(Apple)?Clang$" OR CMAKE_C_COMPILER_ID MATCHES "^(Apple)?Clang$")
        message("ncbi-vdb: ignoring so_check check test for clang")
    elseif()
        add_test( NAME Test_NCBI_VDB_vdb COMMAND ./so_check.sh ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libncbi-vdb.so WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

        add_test( NAME Test_NCBI_VDB_wvdb COMMAND ./so_check.sh ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libncbi-wvdb.so WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
    endif()
endif()
