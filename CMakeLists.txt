cmake_minimum_required(VERSION 3.16)
project(ncbi-vdb)

message( CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR} )

include( ${CMAKE_CURRENT_SOURCE_DIR}/build/common.cmake )

# version, taken from the source by default
if ( "${VERSION}" STREQUAL "" )
    set( VERSION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/libs/ncbi-vdb/libncbi-vdb.vers")
    file( READ ${VERSION_FILE} VERSION )
    string( STRIP ${VERSION} VERSION )
endif()
message( VERSION=${VERSION} )
string( REGEX MATCH "^[0-9]+" MAJVERS ${VERSION} )

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# determine OS
if ( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL  "Darwin" )
    set(OS "mac")
elseif ( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL  "Linux" )
    set(OS "linux")
elseif ( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL  "Windows" )
    set(OS "windows")
elseif()
    message ( FATAL_ERROR "unknown OS " ${CMAKE_HOST_SYSTEM_NAME})
endif()

# determine architecture
if ( ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "armv7l")
	set(ARCH "armv7l")
elseif ( ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "aarch64")
    set(ARCH "aarch64")
elseif ( ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    set(ARCH "x86_64")
elseif ( ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64")
    set(ARCH "x86_64")
elseif()
    message ( FATAL_ERROR "unknown architecture " ${CMAKE_HOST_SYSTEM_PROCESSOR})
endif ()

# create variables based entirely upon OS
if ( "mac" STREQUAL ${OS} )
    add_definitions( -DMAC -DBSD -DUNIX )
elseif( "linux" STREQUAL ${OS} )
    add_definitions( -DLINUX -DUNIX )
    set( LMCHECK -lmcheck )
endif()

# create variables based entirely upon ARCH
if ("armv7l" STREQUAL ${ARCH})
	set( BITS 32 )
    add_definitions( -DRPI )
	add_compile_options( -mcpu=cortex-a53 -mfpu=neon-vfpv4 -Wno-psabi )
	set ( Z128SRC z128 )
	set ( Z128LSRC z128.nopt )
elseif ("aarch64" STREQUAL ${ARCH} )
	set ( BITS 64 )
	add_definitions( -DHAVE_Z128 )
elseif ("x86_64" STREQUAL ${ARCH} )
    set ( BITS 64 )
    add_definitions( -DHAVE_Z128 -DLONG_DOUBLE_IS_NOT_R128 )
endif()

# now any unique combinations of OS and ARCH
if     ( "mac-x86_84" STREQUAL ${OS}-${ARCH})
elseif ( "linux-x86_64" STREQUAL ${OS}-${ARCH})
    add_definitions( -DHAVE_R128 )
elseif ( "linux-armv7l" STREQUAL ${OS}-${ARCH})
elseif ( "linux-aarch64" STREQUAL ${OS}-${ARCH})
    add_definitions( -DHAVE_R128 -D__float128=_Float128 )
endif()

# combinations of OS and COMP
if ( ${OS}-${CMAKE_CXX_COMPILER_ID} STREQUAL "linux-GNU"  )
    add_definitions( -rdynamic )
    add_definitions( -D_GNU_SOURCE )
endif()

# combinations of OS, ARCH and COMP
if ( ${OS}-${ARCH}-${CMAKE_CXX_COMPILER_ID} STREQUAL "linux-x86_64-GNU"  )
    add_definitions( -DHAVE_QUADMATH )
	set ( LQUADMATH -lquadmath )
elseif ( ${OS}-${ARCH}-${CMAKE_CXX_COMPILER_ID} STREQUAL "linux-x86_64-Clang"  )
endif()

add_definitions( -D_ARCH_BITS=${BITS} -D${ARCH} -Wall )

if ( "${CMAKE_BUILD_TYPE}" STREQUAL "" )
    set ( CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE )
endif()

if ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" )
    add_compile_definitions( DEBUG _DEBUGGING )
else()
    add_compile_definitions( NDEBUG )
endif()
message("CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")

#message( "OS=" ${OS} " ARCH=" ${ARCH} " CXX=" ${CMAKE_CXX_COMPILER} " LMCHECK=" ${LMCHECK} " BITS=" ${BITS} " CMAKE_C_COMPILER_ID=" ${CMAKE_C_COMPILER_ID} " CMAKE_CXX_COMPILER_ID=" ${CMAKE_CXX_COMPILER_ID} )

include_directories(interfaces)
include_directories(interfaces/os)
include_directories(interfaces/ext)

if ( "GNU" STREQUAL "${CMAKE_C_COMPILER_ID}")
    include_directories(interfaces/cc/gcc)
    include_directories(interfaces/cc/gcc/${ARCH})
elseif ( "Clang" STREQUAL "${CMAKE_C_COMPILER_ID}")
    include_directories(interfaces/cc/clang/${ARCH})
elseif ( "MSVC" STREQUAL "${CMAKE_C_COMPILER_ID}")
    include_directories(interfaces/cc/vc++)
    include_directories(interfaces/cc/vc++/${ARCH})
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

#TODO: plug in proper variable og tthe location of NGS-SDK
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../ngs/ngs-sdk )

# Flex/Bison
find_package(FLEX) #TODO: specify minimal version
find_package(BISON) #TODO: specify minimal version

#libxml2
find_package(LibXml2)

# Build artefact locations
if( NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/ilib )
endif()
if( NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
endif()
if( NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
endif()
file(MAKE_DIRECTORY
    ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# testing
enable_testing()

add_subdirectory(libs)
add_subdirectory(test)
add_subdirectory(vdb3)

# documentation
find_package(Doxygen)
# set DOXYGEN_* variables here
doxygen_add_docs(docs interfaces)
