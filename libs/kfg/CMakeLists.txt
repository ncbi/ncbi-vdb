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
        "execute_process(COMMAND /bin/bash -c \"${CMAKE_SOURCE_DIR}/build/install-kfg.sh default.kfg ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_INSTALL_PREFIX}/lib64/ncbi ${CMAKE_CURRENT_SOURCE_DIR}/kfgsums\" )"
    )
    install( SCRIPT CODE
        "execute_process(COMMAND /bin/bash -c \"${CMAKE_SOURCE_DIR}/build/install-kfg.sh certs.kfg ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_INSTALL_PREFIX}/lib64/ncbi ${CMAKE_CURRENT_SOURCE_DIR}/kfgsums\" )"
    )
endif()
