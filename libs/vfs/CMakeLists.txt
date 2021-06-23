if( "unix" STREQUAL ${OS} )
    set (OS_SPECIFIC_SRC unix/syskeyring.c unix/syspath.c)
elseif ( "win" STREQUAL ${OS} )
    set (OS_SPECIFIC_SRC win/syskeyring.c win/syspath.c)
endif()

add_library(vfs STATIC
    jwt.c
    keyring.c
    kurl-stub.c
    manager.c
    names4-response.c
    path.c
    remote-services.c
    resolver-3.0.c
    resolver.c
    sdl-response.c
    services-cache.c
    services.c
    SraDesc.c
    srapath-stub.c
    srapath.c
    srv-response.c
)

add_definitions (-D__mod__="libs/vfs")