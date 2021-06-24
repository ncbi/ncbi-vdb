set ( UNIX_SRC unix/syskeyring.c unix/syspath.c )
set ( LINUX_SRC ${UNIX_SRC} )
set ( MAC_SRC ${UNIX_SRC} )
set ( WIN_SRC win/syskeyring.c win/syspath.c )

if ( "win" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${WIN_SRC} )
elseif( "linux" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${LINUX_SRC} )
elseif( "mac" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${MAC_SRC} )
endif()

add_library(vfs STATIC
    jwt
    names4-response
    manager
    remote-services
    resolver
    resolver-3.0
    sdl-response
    services
    services-cache
    SraDesc
    srv-response
)

add_definitions (-D__mod__="libs/vfs")