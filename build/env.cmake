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

#
# Calculation of the global settings for the CMake build.
#

# allow implicit source file extensions
if ( ${CMAKE_VERSION} VERSION_EQUAL "3.20" OR
     ${CMAKE_VERSION} VERSION_GREATER "3.20")
    cmake_policy(SET CMP0115 OLD)
endif()

# ===========================================================================
# set up CMake variables describing the environment

# version, taken from the source
set( VERSION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/libs/ncbi-vdb/libncbi-vdb.vers")
file( READ ${VERSION_FILE} VERSION )
string( STRIP ${VERSION} VERSION )
message( VERSION=${VERSION} )
string( REGEX MATCH "^[0-9]+" MAJVERS ${VERSION} )

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# determine OS
if ( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL  "Darwin" )
    set(OS "mac")
    set(SHLX "dylib")
elseif ( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL  "Linux" )
    set(OS "linux")
    set(SHLX "so")
elseif ( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL  "Windows" )
    set(OS "windows")
else()
    message ( FATAL_ERROR "unknown OS " ${CMAKE_HOST_SYSTEM_NAME})
endif()

# determine architecture
if ( ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "armv7l")
	set(ARCH "armv7l")
elseif ( ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "arm64")
    set(ARCH "arm64")
elseif ( ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "aarch64")
    set(ARCH "arm64")
elseif ( ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    set(ARCH "x86_64")
elseif ( ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64")
    set(ARCH "x86_64")
else ()
    message ( FATAL_ERROR "unknown architecture " ${CMAKE_HOST_SYSTEM_PROCESSOR})
endif ()

# create variables based entirely upon OS
if ( "mac" STREQUAL ${OS} )
    add_compile_definitions( MAC BSD UNIX )
elseif( "linux" STREQUAL ${OS} )
    add_compile_definitions( LINUX UNIX )
    set( LMCHECK -lmcheck )
elseif( "windows" STREQUAL ${OS} )
    add_compile_definitions( WINDOWS _WIN32_WINNT=0x0502 )
endif()

# create variables based entirely upon ARCH
if ("armv7l" STREQUAL ${ARCH})
	set( BITS 32 )
    add_compile_definitions( RPI )
	add_compile_options( -mcpu=cortex-a53 -mfpu=neon-vfpv4 -Wno-psabi )
	set ( Z128SRC z128 )
	set ( Z128LSRC z128.nopt )
elseif ("arm64" STREQUAL ${ARCH} )
	set ( BITS 64 )
	add_compile_definitions( HAVE_Z128 )
elseif ("x86_64" STREQUAL ${ARCH} )
    set ( BITS 64 )
    add_compile_definitions( LONG_DOUBLE_IS_NOT_R128 )
    if( NOT WIN32 )
        add_compile_definitions( HAVE_Z128 )
    endif()
endif()

# now any unique combinations of OS and ARCH
if     ( "mac-x86_84" STREQUAL ${OS}-${ARCH})
elseif ( "linux-x86_64" STREQUAL ${OS}-${ARCH})
    add_compile_definitions( HAVE_R128 )
elseif ( "linux-armv7l" STREQUAL ${OS}-${ARCH})
elseif ( "linux-arm64" STREQUAL ${OS}-${ARCH})
    add_compile_definitions( HAVE_R128 __float128=_Float128 )
endif()

# combinations of OS and COMP
if ( ${OS}-${CMAKE_CXX_COMPILER_ID} STREQUAL "linux-GNU"  )
    add_definitions( -rdynamic )
    add_compile_definitions( _GNU_SOURCE )
endif()

# combinations of OS, ARCH and COMP
if ( ${OS}-${ARCH}-${CMAKE_CXX_COMPILER_ID} STREQUAL "linux-x86_64-GNU"  )
    add_compile_definitions( HAVE_QUADMATH )
	set ( LQUADMATH -lquadmath )
elseif ( ${OS}-${ARCH}-${CMAKE_CXX_COMPILER_ID} STREQUAL "linux-x86_64-Clang"  )
endif()

add_compile_definitions( _ARCH_BITS=${BITS} ${ARCH} )
add_definitions( -Wall )

# assume debug build by default
if ( "${CMAKE_BUILD_TYPE}" STREQUAL "" )
    set ( CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE )
endif()

if ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" )
    add_compile_definitions( DEBUG _DEBUGGING )
else()
    add_compile_definitions( NDEBUG )
endif()

if ( SINGLE_CONFIG )
    message("CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
endif()

#message( "OS=" ${OS} " ARCH=" ${ARCH} " CXX=" ${CMAKE_CXX_COMPILER} " LMCHECK=" ${LMCHECK} " BITS=" ${BITS} " CMAKE_C_COMPILER_ID=" ${CMAKE_C_COMPILER_ID} " CMAKE_CXX_COMPILER_ID=" ${CMAKE_CXX_COMPILER_ID} )

# ===========================================================================
# include directories for C/C++ compilation
#

include_directories(interfaces)
include_directories(interfaces/os)
include_directories(interfaces/ext)

if ( "GNU" STREQUAL "${CMAKE_C_COMPILER_ID}")
    include_directories(interfaces/cc/gcc)
    include_directories(interfaces/cc/gcc/${ARCH})
elseif ( CMAKE_CXX_COMPILER_ID MATCHES "^(Apple)?Clang$" )
    include_directories(interfaces/cc/clang)
    include_directories(interfaces/cc/clang/${ARCH})
elseif ( "MSVC" STREQUAL "${CMAKE_C_COMPILER_ID}")
    include_directories(interfaces/cc/vc++)
    include_directories(interfaces/cc/vc++/${ARCH})
    add_compile_definitions( UNICODE _UNICODE )
endif()

if ( "mac" STREQUAL ${OS} )
    include_directories(interfaces/os/mac)
    include_directories(interfaces/os/unix)
elseif( "linux" STREQUAL ${OS} )
    include_directories(interfaces/os/linux)
    include_directories(interfaces/os/unix)
elseif( "windows" STREQUAL ${OS} )
    include_directories(interfaces/os/win)
endif()

if( NGS_INCDIR )
    include_directories( ${NGS_INCDIR} )
else ()
    include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../ngs/ngs-sdk )
    #TODO: if not found, checkout and build
endif ()

# ===========================================================================
# 3d party packages

# Flex/Bison
find_package( FLEX 2.6 )
find_package( BISON 3 )

if (XML2_LIBDIR)
    find_library( LIBXML2_LIBRARIES libxml2.a HINTS ${XML2_LIBDIR} )
    if ( LIBXML2_LIBRARIES )
        set( LibXml2_FOUND true )
        if ( XML2_INCDIR )
            set( LIBXML2_INCLUDE_DIR ${XML2_INCDIR} )
        endif()
    endif()
else()
    find_package( LibXml2 )
endif()
if( LibXml2_FOUND )
    message( LIBXML2_INCLUDE_DIR=${LIBXML2_INCLUDE_DIR} )
    message( LIBXML2_LIBRARIES=${LIBXML2_LIBRARIES} )
endif()

if ( PYTHON_PATH )
    set( Python3_EXECUTABLE ${PYTHON_PATH} )
endif()
find_package( Python3 COMPONENTS Interpreter )

# ===========================================================================
# Installation location
#

#message( CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX} )

if ( NOT INST_BINDIR )
    set( INST_BINDIR ${CMAKE_INSTALL_PREFIX}/bin )
endif()
if ( NOT INST_LIBDIR )
    set( INST_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib${BITS} )
endif()
if ( NOT INST_INCDIR )
    set( INST_INCDIR ${CMAKE_INSTALL_PREFIX}/include )
endif()

# ===========================================================================
# Build artefact locations.
# For XCode and MSVC specify per-configuration output directories

function(SetAndCreate var path )
    if( NOT DEFINED ${var} )
        set( ${var} ${path} PARENT_SCOPE )
    endif()
    file(MAKE_DIRECTORY ${path} )
endfunction()

#message(CMAKE_GENERATOR=${CMAKE_GENERATOR})
if ( ${CMAKE_GENERATOR} MATCHES "Visual Studio.*" OR
     ${CMAKE_GENERATOR} STREQUAL "Xcode" )
    # a multi-config generator
    SetAndCreate( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/Debug/ilib )
    SetAndCreate( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/ilib )
    SetAndCreate( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/Debug/lib )
    SetAndCreate( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/lib )
    SetAndCreate( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/Debug/bin )
    SetAndCreate( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/bin )
    set( SINGLE_CONFIG false )

else() # assume a single-config generator
    SetAndCreate( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/ilib )
    SetAndCreate( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
    SetAndCreate( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

    set( SINGLE_CONFIG true )
endif()

# ===========================================================================
# testing
enable_testing()

# ===========================================================================
# documentation
find_package(Doxygen)
if ( Doxygen_FOUND)
    # set DOXYGEN_* variables here
    doxygen_add_docs(docs interfaces)
endif()

# ===========================================================================
# common functions
include( ${CMAKE_CURRENT_SOURCE_DIR}/build/common.cmake )

# ===========================================================================
# installation

if ( SINGLE_CONFIG )
    install( SCRIPT CODE
        "execute_process(COMMAND /bin/bash -c \"${CMAKE_SOURCE_DIR}/build/install-root.sh ${VERSION} ${INST_INCDIR} ${INST_LIBDIR} \" )"
    )
endif()
