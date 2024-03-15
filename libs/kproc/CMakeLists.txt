# ===========================================================================
#
#                            PUBLIC DOMAIN NOTICE
#               National Center for Biotechnology Information
#
#  This software/database is a "United States Government Work" under the
#  terms of the United States Copyright Act.  It was written as part of
#  the author's official duties as a United States Government employee and
#  thus cannot be copyrighted.  This software/database is freely available
#  to the public for use. The National Library of Medicine and the U.S.
#  Government have not placed any restriction on its use or reproduction.
#
#  Although all reasonable efforts have been taken to ensure the accuracy
#  and reliability of the software and data, the NLM and the U.S.
#  Government do not and cannot warrant the performance or results that
#  may be obtained by using this software or data. The NLM and the U.S.
#  Government disclaim all warranties, express or implied, including
#  warranties of performance, merchantability or fitness for any particular
#  purpose.
#
#  Please cite the author in any work or product based on this material.
#
# ===========================================================================

set ( UNIX_SRC unix/syscond.c unix/systhread.c unix/systimeout.c )
set ( BSD_SRC ${UNIX_SRC} bsd/syslock.c bsd/sysmgr.c)
set ( LINUX_SRC ${UNIX_SRC} linux/sysbarrier.c linux/syslock.c linux/sysmgr.c )
set ( MAC_SRC ${UNIX_SRC} bsd/syslock.c bsd/sysmgr.c)
set ( WIN_SRC win/syscond.c win/syslock.c win/sysmgr.c win/systhread.c win/systimeout.c )

if( "bsd" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${BSD_SRC} )
    include_directories( bsd )
    include_directories( unix )
elseif( "linux" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${LINUX_SRC} )
    include_directories( linux )
    include_directories( unix )
elseif( "mac" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${MAC_SRC} )
    include_directories( mac )
    include_directories( bsd )
    include_directories( unix )
elseif ( "windows" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${WIN_SRC} )
    include_directories( win )
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
