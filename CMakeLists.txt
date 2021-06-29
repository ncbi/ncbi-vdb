cmake_minimum_required(VERSION 3.16)
project(ncbi-vdb)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# determine OS
if ( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL  "Darwin" )
    set(OS "mac")
elseif ( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL  "Linux" )
    set(OS "linux")
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

add_definitions( -DARCHBITS=${BITS} -D${ARCH} -Wall )

if ( "${CMAKE_BUILD_TYPE}" STREQUAL "" )
    set ( CMAKE_BUILD_TYPE "Debug" )
endif()

if ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" )
    add_compile_definitions( DEBUG _DEBUGGING )
else()
    add_compile_definitions( NDEBUG )
endif()

#message( "OS=" ${OS} " ARCH=" ${ARCH} " CXX=" ${CMAKE_CXX_COMPILER} " LMCHECK=" ${LMCHECK} " BITS=" ${BITS} " CMAKE_C_COMPILER_ID=" ${CMAKE_C_COMPILER_ID} " CMAKE_CXX_COMPILER_ID=" ${CMAKE_CXX_COMPILER_ID} )

include_directories(interfaces)
include_directories(interfaces/os)
include_directories(interfaces/ext)

if ( "GNU" STREQUAL "${CMAKE_C_COMPILER_ID}")
    include_directories(interfaces/cc/gcc)
    include_directories(interfaces/cc/gcc/${ARCH})
elseif ( "Clang" STREQUAL "${CMAKE_C_COMPILER_ID}")
    include_directories(interfaces/cc/clang/${ARCH})
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

# Flex/Bison
find_package(FLEX) #TODO: specify minimal version
find_package(BISON) #TODO: specify minimal version

# testing
enable_testing()
#set(GTEST_LIBRARIES gtest )

add_subdirectory(libs)
add_subdirectory(test)

# documentation
find_package(Doxygen)
# set DOXYGEN_* variables here
doxygen_add_docs(docs interfaces)

