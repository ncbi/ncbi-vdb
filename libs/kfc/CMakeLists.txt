set ( UNIX_SRC unix/sysctx.c unix/sysrsrc.c )
set ( LINUX_SRC ${UNIX_SRC} )
set ( MAC_SRC ${UNIX_SRC} )
set ( WIN_SRC win/sysctx.c win/sysrsrc.c )

if ( "windows" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${WIN_SRC} )
elseif( "linux" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${LINUX_SRC} )
elseif( "mac" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${MAC_SRC} )
endif()

set( SRC
    except
    refcount
    rsrc
    tstate
    vtbl
    xc
    ${OS_SPECIFIC_SRC}
)

GenerateStaticLibs( kfc "${SRC}" )

add_compile_definitions (__mod__="libs/kfc")
include_directories( BEFORE ${CMAKE_CURRENT_SOURCE_DIR} )