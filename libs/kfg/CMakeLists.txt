find_package(FLEX)
if( FLEX_FOUND )
    FLEX_TARGET( KfgFlex config-lex.l ${CMAKE_CURRENT_SOURCE_DIR}/config-lex.c
        COMPILE_FLAGS "--debug --noline" )
    set( KfgScanner ${FLEX_KfgFlex_OUTPUTS} )
else()
    set( KfgScanner config-lex.c )
endif()

find_package(BISON)
if( BISON_FOUND )
    BISON_TARGET( KfgGrammar config-grammar.y ${CMAKE_CURRENT_SOURCE_DIR}/config-grammar.c
        COMPILE_FLAGS "-Wno-other --no-lines -r state" )
    set( KfgParser ${BISON_KfgGrammar_OUTPUT_SOURCE} )
    ADD_FLEX_BISON_DEPENDENCY(KfgFlex KfgGrammar)
else()
    set( KfgParser config-grammar.c )
endif()

add_library(kfg STATIC
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

add_definitions (-D__mod__="libs/kfg")
add_definitions (-D_ARCH_BITS=${BITS})