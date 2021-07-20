set( UNIX_SRC unix/operations.c unix/platform.c unix/security.c )
set( LINUX_SRC ${UNIX_SRC} )
set( MAC_SRC ${UNIX_SRC} )
set( WIN_SRC win/operations.c win/platform.c win/security.c )

if ( "win" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${WIN_SRC} )
    include_directories( win )
elseif( "linux" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${LINUX_SRC} )
    include_directories( unix )
elseif( "mac" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${MAC_SRC} )
    include_directories( unix )
endif()

set( XFS_SRC
    xfs
    xlog
    model
    tree
    node
    path
    perm
    editors
    handle
    teleport
    foo
    ncon
    contnode
    docnode
    simcon
    kfs
    readme
    bogus
    owp
    doc
    xhttp
    http
    xtar
    tar
    xencro
    encro
    xencws
    encws
    spen
    xgap
    xgapf
    xgapk
    gapk
    gapks
    gapka
    gapc
    gap
    gapf
    lockdpt
    lreader
    schwarzschraube
    zehr
    mehr
    ${OS_SPECIFIC_SRC}
)

add_compile_definitions (__mod__="libs/xfs")