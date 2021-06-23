add_library(kxml STATIC
    xml.c
    xtoc-parsexml.c
    xtoc.c
)

add_definitions (-D__mod__="libs/kxml")