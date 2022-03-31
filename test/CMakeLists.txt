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

if(NOT CMAKE_TEST_OUTPUT_DIRECTORY)
    set(CMAKE_TEST_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test-bin)
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_TEST_OUTPUT_DIRECTORY} )

add_subdirectory(kfc)

set( COMMON_LINK_LIBRARIES ktst kapp )

# for the tests that link against libncbi-[w]vdb
if( WIN32 )
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:wmainCRTStartup")
    set( COMMON_LIBS_READ  ${COMMON_LINK_LIBRARIES} ncbi-vdb Ws2_32 Crypt32 )
    set( COMMON_LIBS_WRITE ${COMMON_LINK_LIBRARIES} ncbi-wvdb Ws2_32 Crypt32 )
else()
    link_libraries( ${COMMON_LINK_LIBRARIES} )
    set( COMMON_LIBS_READ   ncbi-vdb  dl pthread )
    set( COMMON_LIBS_WRITE  ncbi-wvdb dl pthread )
endif()

add_subdirectory(ktst)
add_subdirectory(cc)
add_subdirectory(ext)
add_subdirectory(judy_test)
add_subdirectory(klib)
add_subdirectory(kfs)
add_subdirectory(kfg)
add_subdirectory(kapp)
add_subdirectory(cloud)
add_subdirectory(krypto)
add_subdirectory(cipher)
add_subdirectory(kdb)
add_subdirectory(kproc)
add_subdirectory(vdb)
add_subdirectory(search)
add_subdirectory(sraxf)
add_subdirectory(vxf)
add_subdirectory(loader)
# add_subdirectory(align) TODO move to sra-tools
#add_subdirectory(align-access) #TODO: move to asm-trace
add_subdirectory(axf)
add_subdirectory(schema) # TODO: INT_TOOLS - see the schema CMakeLists.txt for the details
add_subdirectory(vfs)
add_subdirectory(ncbi-vdb)
# add_subdirectory(install)
add_subdirectory(kns) # TODO: EXT_TOOLS - see the kns CMakeLists.txt for the details
