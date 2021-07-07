set ( UNIX_SRC unix/args-conv-os.c unix/sysmain.c )
set ( LINUX_SRC ${UNIX_SRC} linux/ram.c )
set ( MAC_SRC ${UNIX_SRC} mac/ram.c )
set ( WIN_SRC win/args-conv-os.c win/ram.c win/sysmain.c )

if ( "win" STREQUAL ${OS} )
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
    queue-file
)

set( KAPP_SRC
    main
    ${KAPP_CMN}
)

set( LOADLIB_SRC
    progressbar
    loader-file
    loader-meta
)

add_library( kapp STATIC ${KAPP_SRC} ${OS_SPECIFIC_SRC} )

add_library( kapp-norsrc STATIC ${KAPPR_SRC} ${OS_SPECIFIC_SRC} )
target_compile_definitions(kapp-norsrc PRIVATE NO_KRSRC=1)

add_library( load STATIC ${LOADLIB_SRC} )

add_definitions (-D__mod__="libs/kapp")
add_definitions (-DPKGNAME="${OS}${BITS}")