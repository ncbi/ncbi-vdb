if( "linux" STREQUAL ${OS} )
    set (OS_SPECIFIC_SRC linux/sysendpoint.c linux/syspoll.c)
elseif ( "mac" STREQUAL ${OS} )
    set (OS_SPECIFIC_SRC mac/sysendpoint.c mac/syspoll.c)
elseif( "unix" STREQUAL ${OS} )
    set (OS_SPECIFIC_SRC unix/sysmgr.c unix/syssock.c unix/sysstream.c)
elseif ( "win" STREQUAL ${OS} )
    set (OS_SPECIFIC_SRC win/sysendpoint.c win/sysmgr.c win/syssock.c win/sysstream.c)
endif()

add_library(kns STATIC
    buffered-stream.c
    endpoint.c
    http-client.c
    http-file.c
    http-request.c
    http-retrier.c
    http.c
    kns_manager-ext.c
    kns_manager.c
    manager.c
    proxy.c
    stable-http-file.c
    stream-from-buffer.c
    stream-from-files.c
    stream-from-streams.c
    stream.c
    tls.c
    url-fetcher-curl.c
    url-fetcher.c
    ${OS_SPECIFIC_SRC}
)

add_definitions (-D__mod__="libs/kns")