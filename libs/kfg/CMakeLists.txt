add_library(kfg STATIC
    config-aws.c
    config-grammar.c
    config-lex.c
    config.c
    docker.c
    kart.c
    keystore.c
    ngc.c
    properties.c
    report-kfg.c
    repository.c
)

add_definitions (-D__mod__="libs/kfg")
add_definitions (-D_ARCH_BITS=${BITS})