if( "linux" STREQUAL ${OS} )
    include_directories(unix) # TODO: unix/linux
    set (OS_SPECIFIC_SRC linux/sysbarrier.c linux/syslock.c linux/sysmgr.c)
elseif( "unix" STREQUAL ${OS} )
    include_directories(unix)
    set (OS_SPECIFIC_SRC unix/syscond.c unix/systhread.c unix/systimeout.c)
elseif ( "win" STREQUAL ${OS} )
    include_directories(win)
    set (OS_SPECIFIC_SRC win/syscond.c win/syslock.c win/sysmgr.c win/systhread.c win/systimeout.c)
endif()

add_library(kproc STATIC
    procmgr.c
    queue.c
    sem.c
    stbarrier.c
    stcond.c
    stlock.c
    stsem.c
    stthread.c
    sttimeout.c
    task.c
    ${OS_SPECIFIC_SRC}
)

add_definitions (-D__mod__="libs/kproc")