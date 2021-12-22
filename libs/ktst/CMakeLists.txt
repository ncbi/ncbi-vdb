set ( UNIX_SRC unix/systestenv )
set ( LINUX_SRC ${UNIX_SRC} )
set ( MAC_SRC ${UNIX_SRC} )
set ( WIN_SRC win/systestenv )

if ( "windows" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${WIN_SRC} )
elseif( "linux" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${LINUX_SRC} )
elseif( "mac" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${MAC_SRC} )
endif()

set( SRC
    testenv
    testcase
    testrunner
    ${OS_SPECIFIC_SRC}
)

GenerateStaticLibs( ktst "${SRC}" )

add_compile_definitions (__mod__="libs/ktst")
#include_directories( BEFORE ${CMAKE_CURRENT_SOURCE_DIR} )