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

set ( UNIX_SRC unix/syskeyring.c unix/syspath.c )
set ( BSD_SRC ${UNIX_SRC} )
set ( LINUX_SRC ${UNIX_SRC} )
set ( MAC_SRC ${UNIX_SRC} )
set ( WIN_SRC win/syskeyring.c win/syspath.c )

if( "bsd" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${BSD_SRC} )
elseif( "linux" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${LINUX_SRC} )
elseif( "mac" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${MAC_SRC} )
elseif ( "windows" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${WIN_SRC} )
endif()

set( SRC
    jwt
    names4-response
    manager
    remote-services
    resolver
    resolver-3.0
    sdl-response
    services
    services-cache
    SraDesc
    srv-response
    path
    ${OS_SPECIFIC_SRC}
)

GenerateStaticLibs( vfs "${SRC}" )

add_compile_definitions (__mod__="libs/vfs")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
