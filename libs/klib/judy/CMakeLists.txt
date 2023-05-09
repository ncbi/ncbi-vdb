set( SRC_JUDY
    JudyHS
    JudyMalloc
    JudySL
    Judy_Bit
    Judy_Bit_Cascade
    Judy_Bit_Count
    Judy_Bit_Decascade
    Judy_Bit_Next
    Judy_Bit_NextEmpty
    Judy_Bit_Prev
    Judy_Word
    Judy_Word_Cascade
    Judy_Word_Count
    Judy_Word_Decascade
    Judy_Word_Next
    Judy_Word_NextEmpty
    Judy_Word_Prev
)

# the debug build of Judy is broken!
remove_definitions( -DDEBUG )

if ( "GNU" STREQUAL "${CMAKE_C_COMPILER_ID}")
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-array-bounds -Wno-aggressive-loop-optimizations" )
elseif ( CMAKE_CXX_COMPILER_ID MATCHES "^(Apple)?Clang$" )
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-array-bounds" )
elseif ( "MSVC" STREQUAL "${CMAKE_C_COMPILER_ID}" )
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W0")
endif()

add_compile_definitions (__mod__="libs/klib/judy" JU_64BIT)
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)

GenerateStaticLibsWithDefs( judy "${SRC_JUDY}" JU_64BIT )
