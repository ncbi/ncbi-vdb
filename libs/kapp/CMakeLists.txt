set ( UNIX_SRC unix/args-conv-os.c unix/sysmain.c )
set ( LINUX_SRC ${UNIX_SRC} linux/ram.c )
set ( MAC_SRC ${UNIX_SRC} mac/ram.c )
set ( WIN_SRC win/args-conv-os.c win/ram.c win/sysmain.c )

if ( "windows" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${WIN_SRC} )
elseif( "linux" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${LINUX_SRC} )
elseif( "mac" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${MAC_SRC} )
endif()

set( KAPP_CMN
    args
    args-conv
    tokenizer
    log-xml
)

set( KAPP_SRC
    main
    ${KAPP_CMN}
)

set( LOADLIB_SRC
    progressbar
    loader-file
    loader-meta
    queue-file
)

set( SRC ${KAPP_SRC} ${OS_SPECIFIC_SRC} )
GenerateStaticLibs( kapp "${SRC}" )
GenerateStaticLibsWithDefs( kapp-norsrc "${SRC}" "NO_KRSRC=1" )

GenerateStaticLibs( load "${LOADLIB_SRC}" )

add_compile_definitions (__mod__="libs/kapp")
add_compile_definitions (PKGNAME="${OS}${BITS}")