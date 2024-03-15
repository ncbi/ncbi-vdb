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

option( RUN_SANITIZER_TESTS "Run ASAN and TSAN tests" OFF )

# ===========================================================================
# include Conan setup
if(_NCBIVDB_CFG_PACKAGING)
    foreach(_sub IN LISTS CMAKE_BINARY_DIR CMAKE_MODULE_PATH)
        if (EXISTS "${_sub}/conanbuildinfo.cmake")
            include(${_sub}/conanbuildinfo.cmake)
            conan_basic_setup()
            break()
        endif()
    endforeach()
    if(NOT DEFINED CONAN_LIBS)
        find_package(BZip2 REQUIRED)
        find_package(ZLIB REQUIRED)
        find_package(zstd REQUIRED)
        include_directories(${BZip2_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} ${zstd_INCLUDE_DIRS})
    	add_compile_definitions( ${BZip2_DEFINITIONS}  ${ZLIB_DEFINITIONS} ${zstd_DEFINITIONS})
    	set( CONAN_LIBS ${BZip2_LIBRARIES}  ${ZLIB_LIBRARIES} ${zstd_LIBRARIES})
    endif()
endif()

# ===========================================================================
# set up CMake variables describing the environment

# version, taken from the source
set( VERSION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/libs/ncbi-vdb/libncbi-vdb.vers")
file( READ ${VERSION_FILE} VERSION )
string( STRIP ${VERSION} VERSION )
message( VERSION=${VERSION} )
string( REGEX MATCH "^[0-9]+" MAJVERS ${VERSION} )

set(C_STANDARD 11)
#set(C_STANDARD 17)
set(CMAKE_C_STANDARD ${C_STANDARD})
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON) # need for some gnu-isms
set(CMAKE_CXX_STANDARD ${C_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# determine OS
if ( ${CMAKE_SYSTEM_NAME} STREQUAL  "Darwin" )
    set(OS "mac")
    set(SHLX "dylib")
elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL  "FreeBSD" )
    set(OS "bsd")
    set(SHLX "so")
elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL  "Linux" )
    set(OS "linux")
    set(SHLX "so")
elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL  "NetBSD" )
    set(OS "bsd")
    set(SHLX "so")
elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL  "Windows" )
    set(OS "windows")
else()
    message ( FATAL_ERROR "unknown OS " ${CMAKE_SYSTEM_NAME})
endif()

# determine architecture
set(_system_processor ${CMAKE_SYSTEM_PROCESSOR})
if (APPLE AND NOT "${CMAKE_OSX_ARCHITECTURES}" STREQUAL "")
    set(_system_processor ${CMAKE_OSX_ARCHITECTURES})
endif ()
if ( ${_system_processor} STREQUAL "armv7l")
	set(ARCH "armv7l")
elseif ( ${_system_processor} STREQUAL "arm64")
    set(ARCH "arm64")
elseif ( ${_system_processor} STREQUAL "aarch64")
    set(ARCH "arm64")
elseif ( ${_system_processor} STREQUAL "x86_64")
    set(ARCH "x86_64")
elseif ( ${_system_processor} STREQUAL "AMD64")
    set(ARCH "x86_64")
elseif ( ${_system_processor} STREQUAL "amd64")
    set(ARCH "x86_64")
else ()
    message ( FATAL_ERROR "unknown architecture " ${_system_processor})
endif ()

# create variables based entirely upon OS
if( "bsd" STREQUAL ${OS} )
    add_compile_definitions( BSD UNIX )
    set( LMCHECK "" )
elseif ( "mac" STREQUAL ${OS} )
    add_compile_definitions( MAC BSD UNIX )
    set(CMAKE_C_ARCHIVE_CREATE   "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_C_ARCHIVE_FINISH   "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
    set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
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

# global compiler warnings settings
if ( "GNU" STREQUAL "${CMAKE_C_COMPILER_ID}")
    set( DISABLED_WARNINGS_C "-Wno-unused-function")
    set( DISABLED_WARNINGS_CXX "-Wno-noexcept-type")
elseif ( CMAKE_CXX_COMPILER_ID MATCHES "^(Apple)?Clang$" )
    set( DISABLED_WARNINGS_C "-Wno-unused-function")
    set( DISABLED_WARNINGS_CXX "-Wno-c++17-compat-mangling")
elseif ( "MSVC" STREQUAL "${CMAKE_C_COMPILER_ID}")
    #
    # Unhelpful warnings, generated in particular by MSVC and Windows SDK header files
    #
    # Warning C4820: 'XXX': 'N' bytes padding added after data member 'YYY'
    # Warning C5045 Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
    # Warning C4668	'XXX' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
    # Warning C5105	macro expansion producing 'defined' has undefined behavior
    # Warning C4514: 'XXX': unreferenced inline function has been removed
    # warning C4623: 'XXX': default constructor was implicitly defined as deleted
    # warning C4625: 'XXX': copy constructor was implicitly defined as deleted
    # warning C4626: 'XXX': assignment operator was implicitly defined as deleted
    # warning C5026: 'XXX': move constructor was implicitly defined as deleted
    # warning C5027: 'XXX': move assignment operator was implicitly defined as deleted
    # warning C4571: Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
    # warning C4774: '_scprintf' : format string expected in argument 1 is not a string literal
    # warning C4255: 'XXX': no function prototype given: converting '()' to '(void)'
    # warning C4710: 'XXX': function not inlined
    # warning C5031: #pragma warning(pop): likely mismatch, popping warning state pushed in different file
	# warning C5032: detected #pragma warning(push) with no corresponding #pragma warning(pop)
    # Warning C4464 relative include path contains '..'
    # Warning C4100 'XXX': unreferenced formal parameter
    # Warning C4061	enumerator 'XXX' in switch of enum 'YYY' is not explicitly handled by a case label
    # Warning C5264	'XXX': 'const' variable is not used
    # Warning C5220	'XXX': a non-static data member with a volatile qualified type no longer implies that compiler generated copy/move constructors and copy/move assignment operators are not trivial
    set( DISABLED_WARNINGS_C "/wd4820 /wd5045 /wd4668 /wd5105 /wd4514 /wd4774 /wd4255 /wd4710 /wd5031 /wd5032 /wd4464 /wd4100 /wd4061")
    set( DISABLED_WARNINGS_CXX "/wd4623 /wd4625 /wd4626 /wd5026 /wd5027 /wd4571 /wd5264 /wd5220")

    # properly report the C++ version in __cplusplus (by default, always set to "199711L" !)
    add_compile_options("/Zc:__cplusplus")
endif()
set( CMAKE_C_FLAGS "-Wall ${CMAKE_C_FLAGS} ${DISABLED_WARNINGS_C}" )
set( CMAKE_CXX_FLAGS "-Wall ${CMAKE_CXX_FLAGS} ${DISABLED_WARNINGS_C} ${DISABLED_WARNINGS_CXX}" )

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

add_compile_definitions (PKGNAME=${OS}${BITS})

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

if( "bsd" STREQUAL ${OS} )
    include_directories(interfaces/os/bsd)
    include_directories(interfaces/os/unix)
elseif( "linux" STREQUAL ${OS} )
    include_directories(interfaces/os/linux)
    include_directories(interfaces/os/unix)
elseif ( "mac" STREQUAL ${OS} )
    include_directories(interfaces/os/mac)
    include_directories(interfaces/os/unix)
elseif( "windows" STREQUAL ${OS} )
    include_directories(interfaces/os/win)
endif()

if(NOT _NCBIVDB_CFG_PACKAGING)
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

if ( PYTHON_PATH )
    set( Python3_EXECUTABLE ${PYTHON_PATH} )
endif()
find_package( Python3 COMPONENTS Interpreter )
endif(NOT _NCBIVDB_CFG_PACKAGING)

# ===========================================================================
# Installation location
#

#message( CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX} )

# provide ability to override installation directories
if ( NOT INST_BINDIR )
    if ( CMAKE_INSTALL_BINDIR )
        set( INST_BINDIR ${CMAKE_INSTALL_BINDIR} )
    else()
        set( INST_BINDIR ${CMAKE_INSTALL_PREFIX}/bin )
    endif()
endif()

if ( NOT INST_LIBDIR )
    if ( CMAKE_INSTALL_LIBDIR )
        set( INST_LIBDIR ${CMAKE_INSTALL_LIBDIR} )
    else()
        set( INST_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib${BITS} )
    endif()
endif()

if ( NOT INST_INCDIR )
    if ( CMAKE_INSTALL_INCLUDEDIR )
        set( INST_INCDIR ${CMAKE_INSTALL_INCLUDEDIR} )
    else()
        set( INST_INCDIR ${CMAKE_INSTALL_PREFIX}/include )
    endif()
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

if(NOT _NCBIVDB_CFG_PACKAGING)
option(COVERAGE "Generate test coverage" OFF)

if( COVERAGE AND "GNU" STREQUAL "${CMAKE_C_COMPILER_ID}")
    message( COVERAGE=${COVERAGE} )

    SET(GCC_COVERAGE_COMPILE_FLAGS "-coverage -fprofile-arcs -ftest-coverage")
    SET(GCC_COVERAGE_LINK_FLAGS    "-coverage -lgcov")

    SET( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
    SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )
endif()

# ===========================================================================
# documentation
find_package(Doxygen)
if ( Doxygen_FOUND)
    # set DOXYGEN_* variables here
    doxygen_add_docs(docs interfaces)
endif()
endif(NOT _NCBIVDB_CFG_PACKAGING)

# ===========================================================================
# common functions
include( ${CMAKE_CURRENT_SOURCE_DIR}/build/common.cmake )

# ===========================================================================
# installation

if(NOT _NCBIVDB_CFG_PACKAGING)
if ( SINGLE_CONFIG )
    install( SCRIPT CODE
        "execute_process(COMMAND /bin/bash -c \"${CMAKE_SOURCE_DIR}/build/install-root.sh ${VERSION} ${INST_INCDIR} ${INST_LIBDIR} \" )"
    )
endif()

if( NOT SINGLE_CONFIG )
	if( RUN_SANITIZER_TESTS )
		message( "RUN_SANITIZER_TESTS (${RUN_SANITIZER_TESTS}) cannot be turned on in a non single config mode - overriding to OFF" )
	endif()
	set( RUN_SANITIZER_TESTS OFF )
endif()

if( RUN_SANITIZER_TESTS )
	find_program(LSB_RELEASE_EXEC lsb_release)
	execute_process(COMMAND ${LSB_RELEASE_EXEC} -is
		OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	message("LSB_RELEASE_ID_SHORT: ${LSB_RELEASE_ID_SHORT}")
	if( LSB_RELEASE_ID_SHORT STREQUAL "Ubuntu" )
		message("Disabling sanitizer tests on Ubuntu...")
		set( RUN_SANITIZER_TESTS OFF )
	endif()
endif()

if( RUN_SANITIZER_TESTS_OVERRIDE )
	message("Overriding sanitizer tests due to RUN_SANITIZER_TESTS_OVERRIDE: ${RUN_SANITIZER_TESTS_OVERRIDE}")
	set( RUN_SANITIZER_TESTS ON )
endif()
message( "RUN_SANITIZER_TESTS: ${RUN_SANITIZER_TESTS}" )
endif(NOT _NCBIVDB_CFG_PACKAGING)

if( NOT HAVE_MBEDTLS_F )
	message( "Using local mbedtls headers from interfaces/ext/mbedtls" )
	include_directories( interfaces/ext/mbedtls )
endif()
