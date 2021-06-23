add_library(search STATIC
    agrep-dp.c
    agrep-myers.c
    agrep-myersunltd.c
    agrep-wumanber.c
    fgrep-aho.c
    fgrep-boyermoore.c
    fgrep-dumb.c
    nucstrstr.c
    search.c
    sw-search.c
)

add_definitions (-D__mod__="libs/search")