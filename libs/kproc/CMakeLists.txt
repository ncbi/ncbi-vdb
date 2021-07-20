set ( UNIX_SRC sem unix/syscond.c unix/systhread.c unix/systimeout.c )
set ( LINUX_SRC ${UNIX_SRC} linux/sysbarrier.c linux/syslock.c linux/sysmgr.c )
set ( MAC_SRC ${UNIX_SRC} )
set ( WIN_SRC win/syscond.c win/syslock.c win/sysmgr.c win/systhread.c win/systimeout.c )

if ( "win" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${WIN_SRC} )
    include_directories( win )
elseif( "linux" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${LINUX_SRC} )
    include_directories( unix )
    include_directories( linux )
elseif( "mac" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${MAC_SRC} )
    include_directories( unix )
    include_directories( mac )
endif()

set( PROC_CMN
    task
    procmgr
    ${OS_SPECIFIC_SRC}
)

set( PROC_SRC ${PROC_CMN} )

set( SPROC_SRC
    ${PROC_CMN}
    sttimeout
    stlock
    stcond
    stsem
    stthread
    stbarrier
)



add_library( ksproc STATIC ${SPROC_SRC} )
add_library( kproc STATIC ${PROC_SRC} )
add_library( kq STATIC queue )

add_compile_definitions (__mod__="libs/kproc")