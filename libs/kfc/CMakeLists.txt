if ( "win" STREQUAL ${OS} )
    set (OS_SPECIFIC_DIR win)
elseif( "linux" STREQUAL ${OS} )
    set (OS_SPECIFIC_DIR unix)
elseif( "mac" STREQUAL ${OS} )
    set (OS_SPECIFIC_DIR unix)
endif()

add_library(kfc STATIC
    except.c
    refcount.c
    rsrc.c
    tstate.c
    vtbl.c
    xc.c
    ${OS_SPECIFIC_DIR}/sysctx.c
    ${OS_SPECIFIC_DIR}/sysrsrc.c
)

add_definitions (-D__mod__="libs/kfc")
