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

cmake_minimum_required(VERSION 3.16)
if (WIN32)
	set (CMAKE_SYSTEM_VERSION 10.0 CACHE STRING INTERNAL FORCE)
endif()

# set( CMAKE_DISABLE_SOURCE_CHANGES ON ) # Cannot use with bison
# set( CMAKE_DISABLE_IN_SOURCE_BUILD ON )

option(LIBS_ONLY "Generate library targets only" ON)
option(TESTS_ONLY "Generate test targets only" OFF)

if( TESTS_ONLY AND LIBS_ONLY )
    message( "ERROR: TESTS_ONLY and LIBS_ONLY cannot be specified at the same time" )

elseif( TESTS_ONLY AND NOT LIBS_ONLY AND (${CMAKE_GENERATOR} MATCHES "Visual Studio.*" OR ${CMAKE_GENERATOR} STREQUAL "Xcode") )
    project(ncbi-vdb-tests)
    if( PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR )
        message(FATAL_ERROR "In-source builds are not allowed")
    endif()
    # calculate global settiungs, create output directories
    include( build/env.cmake NO_POLICY_SCOPE )

    message( "Generating tests only project..." )
    if(CMAKE_BUILD_TYPE STREQUAL "" OR CMAKE_BUILD_TYPE STREQUAL "Debug")
        link_directories( ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG} )
        link_directories( ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG} )
    else()
        link_directories( ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE} )
        link_directories( ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE} )
    endif()
    add_subdirectory(test)

elseif( NOT TESTS_ONLY AND LIBS_ONLY AND (${CMAKE_GENERATOR} MATCHES "Visual Studio.*" OR ${CMAKE_GENERATOR} STREQUAL "Xcode") )
    project(ncbi-vdb)
    if( PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR )
        message(FATAL_ERROR "In-source builds are not allowed")
    endif()
    # calculate global settiungs, create output directories
    include( build/env.cmake NO_POLICY_SCOPE )

    message( "Generating libs only project..." )
    add_subdirectory(libs)
else()

    if( TESTS_ONLY )
        message( "ignoring TESTS_ONLY, use it only for MSVC or XCode" )
    endif()

    project(ncbi-vdb)
    if( PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR )
        message(FATAL_ERROR "In-source builds are not allowed")
    endif()
    # calculate global settiungs, create output directories
    include( build/env.cmake NO_POLICY_SCOPE )
    add_subdirectory(libs)
    if ( NOT LIBS_ONLY )
        add_subdirectory(test)
    endif()

endif()

#add_subdirectory(vdb3)
