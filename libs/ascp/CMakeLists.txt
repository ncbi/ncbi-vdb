set ( UNIX_SRC unix/connect.c )
set ( LINUX_SRC ${UNIX_SRC} linux/ascp-path.c )
set ( MAC_SRC ${UNIX_SRC} mac/ascp-path.c )
set ( WIN_SRC win/connect.c win/ascp-path.c )

if ( "win" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${WIN_SRC} )
elseif( "linux" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${LINUX_SRC} )
elseif( "mac" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${MAC_SRC} )
endif()

add_library(ascp STATIC
    ascp.c
    ${OS_SPECIFIC_SRC}
)

add_definitions( -D__mod__="libs/ascp" )
include_directories( BEFORE ${CMAKE_CURRENT_SOURCE_DIR} )