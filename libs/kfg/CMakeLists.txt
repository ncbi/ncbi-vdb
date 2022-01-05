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

if( FLEX_FOUND AND BISON_FOUND )
    FLEX_TARGET( KfgFlex config-lex.l ${CMAKE_CURRENT_SOURCE_DIR}/config-lex.c
        COMPILE_FLAGS "--debug --noline" )
    set( KfgScanner ${FLEX_KfgFlex_OUTPUTS} )
    BISON_TARGET( KfgGrammar config-grammar.y ${CMAKE_CURRENT_SOURCE_DIR}/config-grammar.c
        COMPILE_FLAGS "-Wno-other --no-lines -r state" )
    set( KfgParser ${BISON_KfgGrammar_OUTPUT_SOURCE} )
    ADD_FLEX_BISON_DEPENDENCY(KfgFlex KfgGrammar)
else()
    set( KfgScanner config-lex.c )
    set( KfgParser config-grammar.c )
endif()

set( SRC
    ${KfgParser}
    ${KfgScanner}
    config-aws
    config
    docker
    kart
    keystore
    ngc
    properties
    report-kfg
    repository
)

GenerateStaticLibs( kfg "${SRC}" )

add_compile_definitions (__mod__="libs/kfg")
add_compile_definitions (_ARCH_BITS=${BITS})

if ( SINGLE_CONFIG )
    install( SCRIPT CODE
        "execute_process(COMMAND /bin/bash -c \"${CMAKE_SOURCE_DIR}/build/install-kfg.sh default.kfg ${CMAKE_CURRENT_SOURCE_DIR} ${INST_LIBDIR}/ncbi ${CMAKE_CURRENT_SOURCE_DIR}/kfgsums\" )"
    )
    install( SCRIPT CODE
        "execute_process(COMMAND /bin/bash -c \"${CMAKE_SOURCE_DIR}/build/install-kfg.sh certs.kfg ${CMAKE_CURRENT_SOURCE_DIR} ${INST_LIBDIR}/ncbi ${CMAKE_CURRENT_SOURCE_DIR}/kfgsums\" )"
    )
endif()
