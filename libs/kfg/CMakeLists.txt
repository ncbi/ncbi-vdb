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
    FLEX_TARGET( KfgFlex config-lex.l ${CMAKE_CURRENT_SOURCE_DIR}/zz_config-lex.c
        COMPILE_FLAGS "--debug --noline" )
    set( KfgScanner ${FLEX_KfgFlex_OUTPUTS} )
    BISON_TARGET( KfgGrammar config-grammar.y ${CMAKE_CURRENT_SOURCE_DIR}/zz_config-grammar.c
        COMPILE_FLAGS "-Wno-other --no-lines -r state --defines=${CMAKE_CURRENT_SOURCE_DIR}/zz_config-grammar.h" )
    set( KfgParser ${BISON_KfgGrammar_OUTPUT_SOURCE} )
    ADD_FLEX_BISON_DEPENDENCY(KfgFlex KfgGrammar)
    SET_DIRECTORY_PROPERTIES(PROPERTIES CLEAN_NO_CUSTOM 1)
else()
    set( KfgScanner zz_config-lex.c )
    set( KfgParser zz_config-grammar.c )
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
#set_source_files_properties(${KfgScanner} PROPERTIES COMPILE_FLAGS -D__STDC_VERSION__=199901L)

if ( SINGLE_CONFIG AND NOT _NCBIVDB_CFG_PACKAGING)
    install( SCRIPT CODE
        "execute_process(COMMAND /bin/bash -c \"${CMAKE_CURRENT_SOURCE_DIR}/install.sh ${CMAKE_SOURCE_DIR}/interfaces/kfg/ncbi ${CMAKE_INSTALL_PREFIX}/lib64/ncbi /etc/ncbi ${CMAKE_SOURCE_DIR}/interfaces/kfg/ncbi/kfgsums \" )"
    )
endif()
