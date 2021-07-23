set ( UNIX_SRC unix/sysmgr.c unix/syssock.c unix/sysstream.c )
set ( LINUX_SRC ${UNIX_SRC} linux/sysendpoint.c linux/syspoll.c )
set ( MAC_SRC ${UNIX_SRC} mac/sysendpoint.c mac/syspoll.c )
set ( WIN_SRC win/sysendpoint.c win/sysmgr.c win/syssock.c win/sysstream.c )

if ( "windows" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${WIN_SRC} )
    include_directories( win )
elseif( "linux" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${LINUX_SRC} )
    include_directories( unix )
elseif( "mac" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${MAC_SRC} )
    include_directories( unix )
endif()

set( KNS_NO_HTTP_SRC
    stream-from-buffer
    kns_manager-ext
    manager
    proxy
    stable-http-file
    buffered-stream
    stream
    stream-from-files
    stream-from-streams
    endpoint
    tls
)

set( KNS_SRC
    ${KNS_NO_HTTP_SRC}
    http-file
    http-client
    http-request
    http-retrier
    http
)

GenerateStaticLibs( kns ${KNS_SRC} ${OS_SPECIFIC_SRC} )

add_compile_definitions (__mod__="libs/kns")
add_compile_definitions (PKGNAME="${OS}${BITS}")
include_directories( BEFORE ${CMAKE_CURRENT_SOURCE_DIR} )
