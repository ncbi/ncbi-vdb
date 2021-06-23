if ( "win" STREQUAL ${OS} )
    set (OS_SPECIFIC_SRC win/args-conv-os.c win/ram.c win/sysmain.c)
elseif( "unix" STREQUAL ${OS} )
    set (OS_SPECIFIC_SRC unix/args-conv-os.c unix/sysmain.c)
elseif( "linux" STREQUAL ${OS} )
    set (OS_SPECIFIC_SRC linux/ram.c)
elseif( "mac" STREQUAL ${OS} )
    set (OS_SPECIFIC_SRC mac/ram.c)
endif()

add_library(kapp STATIC
    args-conv.c
    args.c
    loader-file.c
    loader-meta.c
    log-xml.c
    main.c
    progressbar.c
    queue-file.c
    tokenizer.c
    ${OS_SPECIFIC_SRC}
)

add_definitions (-D__mod__="libs/kapp")
add_definitions (-DPKGNAME="${OS}${BITS}")