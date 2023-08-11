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

set ( UNIX_SRC unix/sysmgr.c unix/syssock.c unix/sysstream.c )
set ( LINUX_SRC ${UNIX_SRC} linux/sysendpoint.c linux/syspoll.c )
set ( MAC_SRC ${UNIX_SRC} mac/sysendpoint.c mac/syspoll.c )
set ( WIN_SRC win/sysendpoint.c win/sysmgr.c win/syssock.c win/sysstream.c )

if ( "windows" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${WIN_SRC} )
    include_directories( win )
elseif( "linux" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${LINUX_SRC} )
    include_directories( unix )
elseif( "mac" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${MAC_SRC} )
    include_directories( unix )
endif()

set( KNS_NO_HTTP_SRC
    stream-from-buffer
    kns_manager-ext
    manager
    proxy
    stable-http-file
    buffered-stream
    stream
    stream-from-files
    stream-from-streams
    endpoint
    tls
)

set( KNS_SRC
    ${KNS_NO_HTTP_SRC}
    http-file
    http-client
    http-request
    http-retrier
    http
    ${OS_SPECIFIC_SRC}
)

GenerateStaticLibs( kns "${KNS_SRC}" )

add_compile_definitions (__mod__="libs/kns")
include_directories( BEFORE ${CMAKE_CURRENT_SOURCE_DIR} )

if(HAVE_GNU_LIBC_VERSION_H)
    target_compile_definitions(kns PRIVATE HAVE_GNU_LIBC_VERSION_H)
endif()
if(HAVE_GNU_GET_LIBC_VERSION_F)
    target_compile_definitions(kns PRIVATE HAVE_GNU_GET_LIBC_VERSION_F)
endif()
