set ( UNIX_SRC unix/syscond.c unix/systhread.c unix/systimeout.c )
set ( LINUX_SRC ${UNIX_SRC} linux/sysbarrier.c linux/syslock.c linux/sysmgr.c )
set ( MAC_SRC ${UNIX_SRC} bsd/syslock.c bsd/sysmgr.c)
set ( WIN_SRC win/syscond.c win/syslock.c win/sysmgr.c win/systhread.c win/systimeout.c )

if ( "windows" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${WIN_SRC} )
    include_directories( win )
elseif( "linux" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${LINUX_SRC} )
    include_directories( linux )
    include_directories( unix )
elseif( "mac" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${MAC_SRC} )
    include_directories( mac )
    include_directories( bsd )
    include_directories( unix )
endif()

set( PROC_CMN
    task
    procmgr
    ${OS_SPECIFIC_SRC}
)

set( PROC_SRC ${PROC_CMN} sem )

# set( SPROC_SRC
    # ${PROC_CMN}
    # sttimeout
    # stlock
    # stcond
    # stsem
    # stthread
    # stbarrier
# )



#GenerateStaticLibs( ksproc "${SPROC_SRC}" )
GenerateStaticLibs( kproc "${PROC_SRC}" )
GenerateStaticLibs( kq queue )

add_compile_definitions (__mod__="libs/kproc")
